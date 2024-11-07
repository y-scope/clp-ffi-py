#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include "DeserializerBufferReader.hpp"

#include <algorithm>
#include <cstdint>
#include <gsl/gsl>

#include <clp/ErrorCode.hpp>
#include <clp/type_utils.hpp>

#include <clp_ffi_py/ExceptionFFI.hpp>
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
    std::span<int8_t> dst_buf{clp::size_checked_pointer_cast<int8_t>(buf), num_bytes_to_read};
    num_bytes_read = 0;
    while (false == dst_buf.empty()) {
        if (is_deserializer_buffer_empty() && false == fill_deserializer_buffer()) {
            return clp::ErrorCode_EndOfFile;
        }
        auto const buffered_bytes_view{m_py_deserializer_buffer->get_unconsumed_bytes()};
        auto const bytes_to_copy{
                buffered_bytes_view.subspan(0, std::min(buffered_bytes_view.size(), dst_buf.size()))
        };
        std::copy(bytes_to_copy.begin(), bytes_to_copy.end(), dst_buf.begin());

        // Commit read
        auto const num_bytes_copied{bytes_to_copy.size()};
        m_py_deserializer_buffer->commit_read_buffer_consumption(
                static_cast<Py_ssize_t>(num_bytes_copied)
        );
        num_bytes_read += num_bytes_copied;
        dst_buf = dst_buf.last(dst_buf.size() - num_bytes_copied);
    }

    return clp::ErrorCode_Success;
}

auto DeserializerBufferReader::fill_deserializer_buffer() -> bool {
    auto const has_error{m_py_deserializer_buffer->try_read()};
    if (false == has_error) {
        return true;
    }
    if (static_cast<bool>(
                PyErr_ExceptionMatches(PyDeserializerBuffer::get_py_incomplete_stream_error())
        ))
    {
        PyErr_Clear();
        return false;
    }
    throw ExceptionFFI(
            clp::ErrorCode_Failure,
            __FILE__,
            __LINE__,
            "`DeserializerBufferReader::fill_deserializer_buffer` failed"
    );
}
}  // namespace clp_ffi_py::ir::native
