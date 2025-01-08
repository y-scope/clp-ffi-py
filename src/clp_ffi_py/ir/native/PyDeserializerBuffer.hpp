#ifndef CLP_FFI_PY_IR_NATIVE_PYDESERIALIZERBUFFER_HPP
#define CLP_FFI_PY_IR_NATIVE_PYDESERIALIZERBUFFER_HPP

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include <cstddef>
#include <cstdint>
#include <span>

#include <clp/ir/types.hpp>

#include <clp_ffi_py/ir/native/PyMetadata.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>

namespace clp_ffi_py::ir::native {
/**
 * This Python class is designed to buffer serialized CLP IR bytes that are read from an input
 * stream. This object serves a dual purpose:
 * - It enables CLP IR deserialization methods to access the buffered bytes for the purpose of
 *   deserializing and decoding log events.
 * - It adheres to the Python buffer protocol, allowing for direct reading from an `IO[bytes]`-like
 *   input stream.
 * This class encompasses all essential attributes to hold the buffered bytes and monitor the state
 * of the buffer. It's meant to be utilized across various CLP IR deserialization method calls when
 * deserializing from the same IR stream.
 */
class PyDeserializerBuffer {
public:
    static constexpr Py_ssize_t cDefaultInitialCapacity{4096};

    // Static methods
    /**
     * CPython-level factory function.
     * @param input_stream
     * @param buf_capacity
     * @return a new reference of a `PyDeserializerBuffer` object that is initialized with the given
     * inputs.
     * @return nullptr on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto create(
            PyObject* input_stream,
            Py_ssize_t buf_capacity = PyDeserializerBuffer::cDefaultInitialCapacity
    ) -> PyDeserializerBuffer*;

    /**
     * Gets the PyTypeObject that represents PyDeserializerBuffer's Python type. This type is
     * dynamically created and initialized during the execution of
     * `PyDeserializerBuffer::module_level_init`.
     * @return Python type object associated with PyDeserializerBuffer.
     */
    [[nodiscard]] static auto get_py_type() -> PyTypeObject*;

    /**
     * Gets a PyObject that represents the incomplete stream error as a Python exception.
     */
    [[nodiscard]] static auto get_py_incomplete_stream_error() -> PyObject*;

    /**
     * Creates and initializes PyDeserializerBuffer as a Python type, and then incorporates this
     * type as a Python object into the py_module module.
     * @param py_module This is the Python module where the initialized PyDeserializerBuffer will be
     * incorporated.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto module_level_init(PyObject* py_module) -> bool;

    // Delete default constructor to disable direct instantiation.
    PyDeserializerBuffer() = delete;

    // Delete copy & move constructors and assignment operators
    PyDeserializerBuffer(PyDeserializerBuffer const&) = delete;
    PyDeserializerBuffer(PyDeserializerBuffer&&) = delete;
    auto operator=(PyDeserializerBuffer const&) -> PyDeserializerBuffer& = delete;
    auto operator=(PyDeserializerBuffer&&) -> PyDeserializerBuffer& = delete;

    // Destructor
    ~PyDeserializerBuffer() = default;

    // Methods
    /**
     * Since the memory allocation of PyDeserializerBuffer is handled by CPython's allocator, cpp
     * constructors will not be explicitly called. This function serves as the default constructor
     * to initialize the underlying input IR stream and read buffer. Other data members are assumed
     * to be zero-initialized by `default-init` method. It has to be manually called whenever
     * creating a new PyDeserializerBuffer object through CPython APIs.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto init(
            PyObject* input_stream,
            Py_ssize_t buf_capacity = PyDeserializerBuffer::cDefaultInitialCapacity
    ) -> bool;

    /**
     * Zero-initializes all the data members in PyDeserializerBuffer. Should be called once the
     * object is allocated.
     */
    auto default_init() -> void {
        m_read_buffer_mem_owner = nullptr;
        m_buffer_size = 0;
        m_num_current_bytes_consumed = 0;
        m_ref_timestamp = 0;
        m_num_deserialized_message = 0;
        m_py_buffer_protocol_enabled = false;
        m_input_ir_stream = nullptr;
        m_metadata = nullptr;
    }

    /**
     * Releases the memory allocated for the underlying metadata field and the reference held for
     * the Python object(s).
     */
    auto clean() -> void {
        Py_XDECREF(m_input_ir_stream);
        Py_XDECREF(m_metadata);
        PyMem_Free(m_read_buffer_mem_owner);
    }

    /**
     * Commits the bytes consumed in the read buffer by incrementing the underlying cursor position.
     * @param num_bytes_consumed Total number of bytes consumed.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    auto commit_read_buffer_consumption(Py_ssize_t num_bytes_consumed) -> bool;

    [[nodiscard]] auto get_num_deserialized_message() const -> size_t {
        return m_num_deserialized_message;
    }

    /**
     * Increments the number of deserialized message counter, and returns the value before
     * increment.
     */
    [[maybe_unused]] auto get_and_increment_deserialized_message_count() -> size_t {
        auto current_num_deserialized_message{m_num_deserialized_message};
        ++m_num_deserialized_message;
        return current_num_deserialized_message;
    }

    [[nodiscard]] auto get_ref_timestamp() const -> clp::ir::epoch_time_ms_t {
        return m_ref_timestamp;
    }

    auto set_ref_timestamp(clp::ir::epoch_time_ms_t timestamp) -> void {
        m_ref_timestamp = timestamp;
    }

    /**
     * @return Number of unconsumed bytes stored in the current read buffer.
     */
    [[nodiscard]] auto get_num_unconsumed_bytes() const -> Py_ssize_t {
        return m_buffer_size - m_num_current_bytes_consumed;
    }

    /**
     * @return A span containing unconsumed bytes.
     */
    [[nodiscard]] auto get_unconsumed_bytes() const -> std::span<int8_t> {
        return m_read_buffer.subspan(m_num_current_bytes_consumed, get_num_unconsumed_bytes());
    }

    /**
     * @return true if metadata has been set.
     */
    [[nodiscard]] auto has_metadata() const -> bool { return (nullptr != m_metadata); }

    /**
     * Initializes the metadata and the initial reference timestamp.
     * @param metadata Input metadata.
     * @return true on success.
     * @return false if the metadata has already been initialized. The relevant Python exception and
     * error will be set.
     */
    [[nodiscard]] auto metadata_init(PyMetadata* metadata) -> bool;

    [[nodiscard]] auto get_metadata() const -> PyMetadata* { return m_metadata; }

    /**
     * Handles the Python buffer protocol's `getbuffer` operation. This function should fail unless
     * the buffer protocol is enabled.
     * @param view Python Buffer view.
     * @param flags Python Buffer flags.
     * @return 0 on success.
     * @return -1 on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto py_getbuffer(Py_buffer* view, int flags) -> int;

    /**
     * @return true if the buffer protocol is enabled.
     */
    [[nodiscard]] auto is_py_buffer_protocol_enabled() const -> bool {
        return m_py_buffer_protocol_enabled;
    }

    /**
     * Attempts to populate the deserializer buffer. When this function is called, it is expected to
     * have more bytes to read from the IR stream.
     * @return true on success.
     * @return false on failure. The Python exception and error will be properly set if the error.
     */
    [[nodiscard]] auto try_read() -> bool;

    /**
     * Tests the functionality of PyDeserializerBuffer by sequentially reading through the input
     * stream with randomly sized reads. It will grow the read buffer when necessary until the the
     * entire input stream is consumed. All the read bytes will be returned as a Python bytearray.
     * @param seed Random seed passing from the tester.
     * @return Python bytearray that contains all the read bytes read from the input stream in
     * sequence.
     * @return nullptr on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto test_streaming(uint32_t seed) -> PyObject*;

private:
    static inline PyObjectStaticPtr<PyTypeObject> m_py_type{nullptr};
    static inline PyObjectStaticPtr<PyObject> m_py_incomplete_stream_error{nullptr};

    /**
     * Cleans the consumed bytes by shifting the unconsumed bytes to the beginning of the buffer,
     * and fills the read buffer by reading from the input IR stream. If more than half of the bytes
     * are unconsumed in the read buffer, the buffer will be doubled before reading.
     * @param num_bytes_read Number of bytes read from the input IR stream to populate the read
     * buffer.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto populate_read_buffer(Py_ssize_t& num_bytes_read) -> bool;

    /**
     * Enable the buffer protocol.
     */
    auto enable_py_buffer_protocol() -> void { m_py_buffer_protocol_enabled = true; }

    /**
     * Disable the buffer protocol.
     */
    auto disable_py_buffer_protocol() -> void { m_py_buffer_protocol_enabled = false; }

    PyObject_HEAD;
    PyObject* m_input_ir_stream;
    PyMetadata* m_metadata;
    int8_t* m_read_buffer_mem_owner;
    std::span<int8_t> m_read_buffer;
    clp::ir::epoch_time_ms_t m_ref_timestamp;
    Py_ssize_t m_buffer_size;
    Py_ssize_t m_num_current_bytes_consumed;
    size_t m_num_deserialized_message;
    bool m_py_buffer_protocol_enabled;
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_PYDESERIALIZERBUFFER_HPP
