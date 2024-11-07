#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include "DeserializerBufferReader.hpp"

#include <gsl/gsl>

#include <clp_ffi_py/ir/native/PyDeserializerBuffer.hpp>

namespace clp_ffi_py::ir::native {
auto DeserializerBufferReader::create(PyObject* input_stream, Py_ssize_t buf_capacity)
        -> gsl::owner<DeserializerBufferReader*> {
    auto* py_deserializer_buffer{PyDeserializerBuffer::create(input_stream, buf_capacity)};
    if (nullptr == py_deserializer_buffer) {
        return nullptr;
    }
    return new DeserializerBufferReader{py_deserializer_buffer};
}

auto DeserializerBufferReader::try_read(char* buf, size_t num_bytes_to_read, size_t& num_bytes_read)
        -> clp::ErrorCode {
    return clp::ErrorCode_Unsupported;
}
}  // namespace clp_ffi_py::ir::native
