#ifndef CLP_FFI_PY_IR_NATIVE_PYSERIALIZER_HPP
#define CLP_FFI_PY_IR_NATIVE_PYSERIALIZER_HPP

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include <cstddef>
#include <optional>
#include <span>

#include <clp/ffi/ir_stream/Serializer.hpp>
#include <clp/ir/types.hpp>
#include <gsl/gsl>

#include <clp_ffi_py/PyObjectUtils.hpp>

namespace clp_ffi_py::ir::native {
/**
 * A PyObject structure for CLP key-value pair IR format serialization (using four-byte encoding).
 * The underlying serializer is pointed by `m_serializer`, and the serialized IR stream is written
 * into an `IO[byte]` stream pointed by `m_output_stream`.
 */
class PySerializer {
public:
    using ClpIrSerializer = clp::ffi::ir_stream::Serializer<clp::ir::four_byte_encoded_variable_t>;
    using BufferView = ClpIrSerializer::BufferView;

    /**
     * The default buffer size limit. Any change to the value should also be applied to `__init__`'s
     * doc string and Python stub file.
     */
    static constexpr size_t cDefaultBufferSizeLimit{65'536};

    /**
     * Gets the `PyTypeObject` that represents `PySerializer`'s Python type. This type is
     * dynamically created and initialized during the execution of `module_level_init`.
     * @return Python type object associated with `PySerializer`.
     */
    [[nodiscard]] static auto get_py_type() -> PyTypeObject* { return m_py_type.get(); }

    /**
     * Creates and initializes `PySerializer` as a Python type, and then incorporates this
     * type as a Python object into the py_module module.
     * @param py_module The Python module where the initialized `PySerializer` will be incorporated.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto module_level_init(PyObject* py_module) -> bool;

    // Delete default constructor to disable direct instantiation.
    PySerializer() = delete;

    // Delete copy & move constructors and assignment operators
    PySerializer(PySerializer const&) = delete;
    PySerializer(PySerializer&&) = delete;
    auto operator=(PySerializer const&) -> PySerializer& = delete;
    auto operator=(PySerializer&&) -> PySerializer& = delete;

    // Destructor
    ~PySerializer() = default;

    /**
     * Initializes the underlying data with the given inputs. Since the memory allocation of
     * `PySerializer` is handled by CPython's allocator, cpp constructors will not be explicitly
     * called. This function serves as the default constructor initialize the underlying serializer.
     * It has to be called manually to create a `PySerializer` object through CPython APIs.
     * @param output_stream
     * @param serializer
     * @param buffer_size_limit
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto
    init(PyObject* output_stream, ClpIrSerializer serializer, Py_ssize_t buffer_size_limit) -> bool;

    /**
     * Initializes the pointers to nullptr by default. Should be called once the object is
     * allocated.
     */
    auto default_init() -> void {
        m_output_stream = nullptr;
        m_serializer = nullptr;
        m_num_total_bytes_serialized = 0;
        m_buffer_size_limit = 0;
    }

    /**
     * Releases the memory allocated for underlying data fields.
     */
    auto clean() -> void {
        close_serializer();
        Py_XDECREF(m_output_stream);
    }

    [[nodiscard]] auto is_closed() const -> bool { return nullptr == m_serializer; }

    /**
     * Serializes the log event from the given msgpack maps into IR format.
     * @param auto_gen_msgpack_map Auto-generated key-value pairs, serialized as a msgpack map.
     * @param user_gen_msgpack_map User-generated key-value pairs, serialized as a msgpack map.
     * @return the number of bytes serialized on success.
     * @return std::nullptr on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto serialize_log_event_from_msgpack_map(
            std::span<char const> auto_gen_msgpack_map,
            std::span<char const> user_gen_msgpack_map
    ) -> std::optional<Py_ssize_t>;

    [[nodiscard]] auto get_num_bytes_serialized() const -> Py_ssize_t {
        return m_num_total_bytes_serialized;
    }

    /**
     * Flushes the underlying IR buffer and `m_output_stream`.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto flush() -> bool;

    /**
     * Closes the serializer by writing the buffered results into the output stream with
     * end-of-stream IR Unit appended in the end.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto close() -> bool;

private:
    static inline PyObjectStaticPtr<PyTypeObject> m_py_type{nullptr};

    /**
     * Asserts the serializer has not been closed.
     * @return true on success, false if it's already been closed with `IOError` set.
     */
    [[nodiscard]] auto assert_is_not_closed() const -> bool;

    [[nodiscard]] auto get_ir_buf_size() const -> Py_ssize_t {
        return static_cast<Py_ssize_t>(m_serializer->get_ir_buf_view().size());
    }

    /**
     * Writes the underlying IR buffer into `m_output_stream`.
     * NOTE: the serializer must not be closed to call this method.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto write_ir_buf_to_output_stream() -> bool;

    /**
     * Closes `m_serializer` by releasing the allocated memory.
     * NOTE: it is safe to call this method more than once as it resets `m_serializer` to nullptr.
     */
    auto close_serializer() -> void {
        delete m_serializer;
        m_serializer = nullptr;
    }

    /**
     * Wrapper of `output_stream`'s `write` method.
     * @param buf
     * @return The number of bytes written on success.
     * @return std::nullopt on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto write_to_output_stream(BufferView buf) -> std::optional<Py_ssize_t>;

    /**
     * Wrapper of `output_stream`'s `flush` method.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto flush_output_stream() -> bool;

    /**
     * Wrapper of `output_stream`'s `close` method.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto close_output_stream() -> bool;

    // Variables
    PyObject_HEAD;
    PyObject* m_output_stream;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    gsl::owner<ClpIrSerializer*> m_serializer;
    Py_ssize_t m_num_total_bytes_serialized;
    Py_ssize_t m_buffer_size_limit;
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_PYSERIALIZER_HPP
