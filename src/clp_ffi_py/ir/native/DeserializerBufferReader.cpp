#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include "DeserializerBufferReader.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <new>
#include <span>

#include <clp/ErrorCode.hpp>
#include <clp/type_utils.hpp>
#include <gsl/gsl>

#include <clp_ffi_py/error_messages.hpp>
#include <clp_ffi_py/ExceptionFFI.hpp>
#include <clp_ffi_py/ir/native/PyDeserializerBuffer.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
auto DeserializerBufferReader::create(PyObject* input_stream, Py_ssize_t buf_capacity)
        -> gsl::owner<DeserializerBufferReader*> {
    PyObjectPtr<PyDeserializerBuffer> const py_deserializer_buffer{
            PyDeserializerBuffer::create(input_stream, buf_capacity)
    };
    if (nullptr == py_deserializer_buffer) {
        return nullptr;
    }
    gsl::owner<DeserializerBufferReader*> reader{new (std::nothrow
    ) DeserializerBufferReader{py_deserializer_buffer.get()}};
    if (nullptr == reader) {
        PyErr_SetString(
                PyExc_RuntimeError,
                get_c_str_from_constexpr_string_view(cOutOfMemoryError)
        );
        return nullptr;
    }
    return reader;
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
        std::ranges::copy(bytes_to_copy.begin(), bytes_to_copy.end(), dst_buf.begin());

        // Commit read
        auto const num_bytes_copied{bytes_to_copy.size()};
        if (false
            == m_py_deserializer_buffer->commit_read_buffer_consumption(
                    static_cast<Py_ssize_t>(num_bytes_copied)
            ))
        {
            throw ExceptionFFI(
                    clp::ErrorCode_Failure,
                    __FILE__,
                    __LINE__,
                    "`commit_read_buffer_consumption` failed"
            );
        }
        num_bytes_read += num_bytes_copied;
        dst_buf = dst_buf.subspan(num_bytes_copied);
    }

    return clp::ErrorCode_Success;
}

auto DeserializerBufferReader::fill_deserializer_buffer() -> bool {
    if (m_py_deserializer_buffer->try_read()) {
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
