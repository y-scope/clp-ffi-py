#ifndef CLP_FFI_PY_IR_NATIVE_DESERIALIZERBUFFERREADER_HPP
#define CLP_FFI_PY_IR_NATIVE_DESERIALIZERBUFFERREADER_HPP

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include <cstddef>

#include <clp/ErrorCode.hpp>
#include <clp/ReaderInterface.hpp>
#include <gsl/gsl>

#include <clp_ffi_py/ir/native/PyDeserializerBuffer.hpp>

namespace clp_ffi_py::ir::native {
/**
 * This class implements `clp::ReaderInterface`  to consume data from a Python byte stream object.
 * It uses `PyDeserializerBuffer` as the underlying buffer storage to read data from a Python
 * stream object.
 */
class DeserializerBufferReader : public clp::ReaderInterface {
public:
    // Factory function
    /**
     * Creates a reader with the given input stream object.
     * @param input_stream A Python IO[byte] object, which must have `readinto` method provided.
     * @param buf_capacity The capacity of the underlying `PyDeserializerBuffer`.
     * @return The transferred ownership of a created object on success.
     * @return nullptr on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto create(PyObject* input_stream, Py_ssize_t buf_capacity)
            -> gsl::owner<DeserializerBufferReader*>;

    // Delete copy & move constructors and assignment operators
    DeserializerBufferReader(DeserializerBufferReader const&) = delete;
    DeserializerBufferReader(DeserializerBufferReader&&) = delete;
    auto operator=(DeserializerBufferReader const&) -> DeserializerBufferReader& = delete;
    auto operator=(DeserializerBufferReader&&) -> DeserializerBufferReader& = delete;

    // Destructor
    ~DeserializerBufferReader() override { Py_XDECREF(m_py_deserializer_buffer); }

    // Methods implementing `clp::ReaderInterface`
    /**
     * Tries to read up to a given number of bytes from the buffered data.
     * @param buf
     * @param num_bytes_to_read
     * @param num_bytes_read Returns the number of bytes read.
     * @return ErrorCode_Success on success.
     * @return ErrorCode_EndOfFile if there is no more data to read.
     * @throw ExceptionFFI that forwards `fill_deserializer_buffer`'s throw.
     */
    [[nodiscard]] auto try_read(char* buf, size_t num_bytes_to_read, size_t& num_bytes_read)
            -> clp::ErrorCode override;

    /**
     * Tries to seek to the given position, relative to the beginning of the data.
     * TODO: Implement this method when needed.
     * @param pos
     * @return clp::ErrorCode_Unsupported always.
     */
    [[nodiscard]] auto try_seek_from_begin([[maybe_unused]] size_t pos) -> clp::ErrorCode override {
        return clp::ErrorCode_Unsupported;
    }

    /**
     * @param pos Returns the position of the read head in the buffer.
     * @return ErrorCode_Success always.
     */
    [[nodiscard]] auto try_get_pos(size_t& pos) -> clp::ErrorCode override {
        pos = m_pos;
        return clp::ErrorCode_Success;
    }

private:
    // Constructor
    /**
     * Constructs a `DeserializerBufferReader` by holding a new reference of the given Python
     * objects. Must be called from the factory function.
     * @param py_deserializer_buffer
     */
    explicit DeserializerBufferReader(PyDeserializerBuffer* py_deserializer_buffer)
            : m_py_deserializer_buffer{py_deserializer_buffer} {
        Py_INCREF(py_deserializer_buffer);
    }

    // Methods
    /**
     * Fills the underlying deserializer buffer by calling its `try_read`.
     * @return true on success.
     * @return false if `try_read` returns false and `IncompleteStreamError` has been set.
     * @throw ExceptionFFI on any other failure.
     */
    [[nodiscard]] auto fill_deserializer_buffer() -> bool;

    [[nodiscard]] auto is_deserializer_buffer_empty() const -> bool {
        return m_py_deserializer_buffer->get_unconsumed_bytes().empty();
    }

    // Variables
    PyDeserializerBuffer* m_py_deserializer_buffer{nullptr};
    size_t m_pos{0};
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_DESERIALIZERBUFFERREADER_HPP
