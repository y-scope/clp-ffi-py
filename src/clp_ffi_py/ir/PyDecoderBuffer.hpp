#ifndef CLP_FFI_PY_PYDECODERBUFFER_HPP
#define CLP_FFI_PY_PYDECODERBUFFER_HPP

#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <GSL/include/gsl/span>

#include <clp/components/core/src/ffi/ir_stream/decoding_methods.hpp>

#include <clp_ffi_py/PyObjectUtils.hpp>

namespace clp_ffi_py::ir {
/**
 * This Python Object represents a DecoderBuffer that buffers encoded CLP IR
 * bytes read from the input stream. On the one hand, CLP IR decoding methods
 * can consume the buffered bytes to decode log events. On the other hand, it
 * follows the Python buffer protocol so that it can directly read from an
 * `IO[bytes]` like input stream. This class contains all the necessary data
 * members to store the buffered bytes and track the buffer states. It is
 * expected to be passed across different calls of CLP IR decoding methods when
 * decoding from the same IR stream.
 */
class PyDecoderBuffer {
public:
    static constexpr Py_ssize_t cDefaultInitialCapacity{4096};

    /**
     * Since the memory allocation of PyDecoderBuffer is handled by CPython's
     * allocator, cpp constructors will not be explicitly called. This function
     * serves as the default constructor to initialize the underlying input IR
     * stream and read buffer. Other data members are assumed to be
     * zero-initialized by `default-init` method. It has to be manually called
     * whenever creating a new PyDecoderBuffer object through CPython APIs.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error
     * set.
     */
    [[nodiscard]] auto
    init(PyObject* input_stream, Py_ssize_t buf_capacity = PyDecoderBuffer::cDefaultInitialCapacity)
            -> bool;

    /**
     * Zero-initializes all the data members in PyDecoderBuffer. Should be
     * called once the object is allocated.
     */
    auto default_init() -> void {
        m_read_buffer_mem_owner = nullptr;
        m_buffer_size = 0;
        m_num_current_bytes_consumed = 0;
        m_num_decoded_message = 0;
        m_py_buffer_protocol_enabled = false;
        m_input_ir_stream = nullptr;
    }

    /**
     * Releases the memory allocated for underlying metadata field and the
     * reference hold for the Python object(s).
     */
    auto clean() -> void {
        Py_XDECREF(m_input_ir_stream);
        PyMem_Free(m_read_buffer_mem_owner);
    }

    /**
     * Cleans the consumed bytes by shifting the unconsumed bytes to the
     * beginning of the buffer, and fills the read buffer by reading from the
     * input IR stream. If more than half of the bytes are unconsumed in the
     * read buffer, the buffer will be doubled before reading.
     * @param num_bytes_read Number of bytes read from the input IR stream to
     * populate the read buffer.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error
     * set.
     */
    [[nodiscard]] auto populate_read_buffer(Py_ssize_t& num_bytes_read) -> bool;

    /**
     * Commits the bytes consumed in the read buffer by incrementing the
     * underlying cursor position.
     * @param num_bytes_consumed Total number of bytes consumed.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error
     * set.
     */
    auto commit_read_buffer_consumption(Py_ssize_t num_bytes_consumed) -> bool;

    [[nodiscard]] auto get_num_decoded_message() const -> size_t { return m_num_decoded_message; }

    auto increment_num_decoded_message() -> void { ++m_num_decoded_message; }

    /**
     * @return Number of unconsumed bytes stored in the current read buffer.
     */
    [[nodiscard]] auto get_num_unconsumed_bytes() const -> Py_ssize_t {
        return m_buffer_size - m_num_current_bytes_consumed;
    }

    /**
     * @return A span containing unconsumed bytes.
     */
    [[nodiscard]] auto get_unconsumed_bytes() const -> gsl::span<int8_t> {
        return m_read_buffer.subspan(m_num_current_bytes_consumed, get_num_unconsumed_bytes());
    }

    /**
     * Handles the Python buffer protocol's `getbuffer` operation.
     * This function should fail unless the buffer protocol is enabled.
     * @param view Python Buffer view.
     * @param flags Python Buffer flags.
     * @return 0 on success.
     * @return -1 on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto py_getbuffer(Py_buffer* view, int flags) -> int;

    /**
     * @return Whether the buffer protocol is enabled.
     */
    [[nodiscard]] auto is_py_buffer_protocol_enabled() const -> bool {
        return m_py_buffer_protocol_enabled;
    }

    /**
     * Tests the functionality of the DecoderBuffer by stepping through the
     * input stream and stream it as random sized byte sequences. It will
     * attempt to read a random number of bytes from the read buffer and grow
     * the buffer when necessary until the the entire input stream is consumed.
     * All the read bytes will be returned as a Python bytearray.
     * @param seed Random seed passing from the tester.
     * @return Python bytearray that contains all the read bytes read from the
     * input stream in sequence.
     * @return nullptr on failure with the relevant Python exception and error
     * set.
     */
    [[nodiscard]] auto test_streaming(uint32_t seed) -> PyObject*;

    /**
     * Gets the PyTypeObject that represents PyDecoderBuffer's Python type. This
     * type is dynamically created and initialized during the execution of
     * `PyDecoderBuffer::module_level_init`.
     * @return Python type object associated with PyDecoderBuffer.
     */
    [[nodiscard]] static auto get_py_type() -> PyTypeObject*;

    /**:w
     *
     * Creates and initializes PyDecoderBuffer as a Python type, and then
     * incorporates this type as a Python object into the py_module module.
     * @param py_module This is the Python module where the initialized
     * PyDecoderBuffer will be incorporated.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error
     * set.
     */
    [[nodiscard]] static auto module_level_init(PyObject* py_module) -> bool;

private:
    PyObject_HEAD;
    int8_t* m_read_buffer_mem_owner;
    gsl::span<int8_t> m_read_buffer;
    Py_ssize_t m_buffer_size;
    Py_ssize_t m_num_current_bytes_consumed;
    size_t m_num_decoded_message;
    bool m_py_buffer_protocol_enabled;
    PyObject* m_input_ir_stream;

    /**
     * Enable the buffer protocol.
     */
    auto enable_py_buffer_protocol() -> void { m_py_buffer_protocol_enabled = true; }

    /**
     * Disable the buffer protocol.
     */
    auto disable_py_buffer_protocol() -> void { m_py_buffer_protocol_enabled = false; }

    static PyObjectPtr<PyTypeObject> m_py_type;
};
}  // namespace clp_ffi_py::ir
#endif
