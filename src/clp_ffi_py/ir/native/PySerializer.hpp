#ifndef CLP_FFI_PY_IR_NATIVE_PYSERIALIZER_HPP
#define CLP_FFI_PY_IR_NATIVE_PYSERIALIZER_HPP

#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <gsl/gsl>
#include <optional>
#include <span>

#include <clp/ffi/ir_stream/Serializer.hpp>
#include <clp/ir/types.hpp>

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
     * Initializes the underlying data with the given inputs. Since the memory allocation of
     * `PySerializer` is handled by CPython's allocator, cpp constructors will not be explicitly
     * called. This function serves as the default constructor initialize the underlying serializer.
     * It has to be manually called whenever creating a new `PySerializer` object through
     * CPython APIs.
     * @param output_stream
     * @param serializer
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto init(PyObject* output_stream, ClpIrSerializer serializer) -> bool;

    /**
     * Initializes the pointers to nullptr by default. Should be called once the object is
     * allocated.
     */
    auto default_init() -> void {
        m_output_stream = nullptr;
        m_serializer = nullptr;
    }

    /**
     * Releases the memory allocated for underlying data fields.
     */
    auto clean() -> void {
        Py_XDECREF(m_output_stream);
        close_serializer();
    }

    [[nodiscard]] auto is_closed() const -> bool { return nullptr == m_serializer; }

    /**
     * Asserts the serializer has not been closed.
     * @return true on success, false if it's already been closed with `IOError` set.
     */
    [[nodiscard]] auto assert_is_not_closed() const -> bool;

    /**
     * Serializes the given msgpack byte sequence as a msgpack map into IR format.
     * NOTE: the serializer must not be closed to call this method.
     * @param msgpack_byte_sequence
     * @return true on success.
     * @return false on error with the relevant Python exception and error set.
     */
    [[nodiscard]] auto serialize_msgpack_map(std::span<char const> msgpack_byte_sequence) -> bool;

    /**
     * Writes the underlying IR buffer into `m_output_stream`.
     * NOTE: the serializer must not be closed to call this method.
     * @return The number of bytes written on success.
     * @return std::nullopt on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto write_ir_buf_to_output_stream() -> std::optional<Py_ssize_t>;

    [[nodiscard]] auto get_ir_buf_size() const -> Py_ssize_t {
        return static_cast<Py_ssize_t>(m_serializer->get_ir_buf_view().size());
    }

    /**
     * Flushes `m_output_stream`.
     * NOTE: the serializer must not be closed to call this method.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto flush_output_stream() -> bool;

    /**
     * Closes the serializer by writing the buffered results into the output stream with
     * end-of-stream IR Unit appended in the end.
     * NOTE: the serializer must not be closed to call this method.
     * @param flush_stream Whether to flush `output_stream`.
     * @return number of bytes written on success.
     * @return std::nullopt on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto close(bool flush_stream) -> std::optional<Py_ssize_t>;

    /**
     * Gets the `PyTypeObject` that represents `PySerializer`'s Python type. This type is
     * dynamically created and initialized during the execution of `module_level_init`.
     * @return Python type object associated with `PySerializer`.
     */
    [[nodiscard]] static auto get_py_type() -> PyTypeObject* { return m_py_type.get(); }

    /**
     * Creates and initializes `PySerializer` as a Python type, and then incorporates this
     * type as a Python object into the py_module module.
     * @param py_module This is the Python module where the initialized `PySerializer` will be
     * incorporated.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto module_level_init(PyObject* py_module) -> bool;

private:
    /**
     * Writes the data from given buffer to `m_output_stream` by calling `m_output_stream`'s `write`
     * Python method.
     * @param buf
     * @return The number of bytes written on success.
     * @return std::nullopt on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto write_to_output_stream(BufferView buf) -> std::optional<Py_ssize_t>;

    /**
     * Closes `m_serializer` by releasing the allocated memory.
     * NOTE: it is safe to call this method more than once as it resets `m_serializer` to nullptr.
     */
    auto close_serializer() -> void {
        delete m_serializer;
        m_serializer = nullptr;
    }

    // Variables
    PyObject_HEAD;
    PyObject* m_output_stream;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    gsl::owner<ClpIrSerializer*> m_serializer;

    static inline PyObjectStaticPtr<PyTypeObject> m_py_type{nullptr};
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_PYSERIALIZER_HPP
