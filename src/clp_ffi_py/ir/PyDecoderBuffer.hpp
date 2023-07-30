#ifndef CLP_FFI_PY_PYDECODERBUFFER_HPP
#define CLP_FFI_PY_PYDECODERBUFFER_HPP

#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <clp/components/core/src/ffi/ir_stream/decoding_methods.hpp>

#include <clp_ffi_py/PyObjectUtils.hpp>

namespace clp_ffi_py::ir {
class PyDecoderBuffer {
    /**
     * TODO: add class description to this class.
     */
public:
    static constexpr Py_ssize_t cDefaultInitialCapacity{4096};

    /**
     * Since the memory allocation of PyDecoderBuffer is handled by CPython's
     * allocator, cpp constructors will not be explicitly called. This function
     * serves as the default constructor to initialize the underlying input IR
     * stream, read buffer, and variables used to track reading states. It has
     * to be manually called whenever creating a new PyDecoderBuffer object
     * through CPython APIs.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error
     * set.
     */
    [[nodiscard]] auto
    init(PyObject* input_stream, Py_ssize_t buf_capacity = PyDecoderBuffer::cDefaultInitialCapacity)
            -> bool;

    /**
     * Initializes the pointers to nullptr by default. Should be called once
     * the object is allocated.
     */
    auto default_init() -> void {
        m_read_buffer = nullptr;
        m_input_ir_stream = nullptr;
    }

    /**
     * Releases the memory allocated for underlying metadata field and the
     * reference hold for the Python object(s).
     */
    auto clean() -> void {
        Py_XDECREF(m_input_ir_stream);
        PyMem_Free(m_read_buffer);
    }

    /**
     * Cleans the consumed bytes by shifting the unconsumed bytes to the 
     * beginning of the buffer, and fills the read buffer by reading from the
     * input IR stream. If more than half of the bytes are unconsumed in the
     * read buffer, the buffer capacity will be doubled before reading.
     * @param num_bytes_read Number of bytes read from the input IR stream to
     * populate the read buffer.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error
     * set.
     */
    [[nodiscard]] auto populate_read_buffer(Py_ssize_t& num_bytes_read) -> bool;

    /**
     * Creates a CLP FFI IRBuffer as a wrapper so that CLP FFI decoding methods
     * can decode from bytes stored in `m_read_buffer`.
     * @return ffi::ir_stream::IrBuffer wrapping the unconsumed bytes.
     */
    [[nodiscard]] auto create_clp_ir_buffer_wrapper() const -> ffi::ir_stream::IrBuffer {
        return {get_unconsumed_bytes(), static_cast<size_t>(get_num_unconsumed_bytes())};
    }

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
        return m_buffer_size - m_current_num_bytes_consumed;
    }

    /**
     * @return The pointer to the first unconsumed bytes stored in the current
     * read buffer.
     */
    [[nodiscard]] auto get_unconsumed_bytes() const -> int8_t* {
        return m_read_buffer + m_current_num_bytes_consumed;
    }

    /**
     * Handles the Python buffer protocol's `getbuffer` operation.
     * @param view Python Buffer view.
     * @param flags Python Buffer flags.
     * @return 0 on success.
     * @return -1 on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto py_getbuffer(Py_buffer* view, int flags) -> int;

    /**
     * Gets the PyTypeObject that represents PyDecoderBuffer's Python type. This
     * type is dynamically created and initialized during the execution of
     * `PyDecoderBuffer::module_level_init`.
     * @return Python type object associated with PyDecoderBuffer.
     */
    [[nodiscard]] static auto get_py_type() -> PyTypeObject*;

    /**
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
    int8_t* m_read_buffer;
    Py_ssize_t m_buffer_size;
    Py_ssize_t m_buffer_capacity;
    Py_ssize_t m_current_num_bytes_consumed;
    size_t m_num_decoded_message;
    PyObject* m_input_ir_stream;

    static PyObjectPtr<PyTypeObject> m_py_type;
};
}  // namespace clp_ffi_py::ir
#endif
