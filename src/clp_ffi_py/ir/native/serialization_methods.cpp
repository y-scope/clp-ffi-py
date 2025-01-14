#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include "serialization_methods.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <clp/ffi/ir_stream/encoding_methods.hpp>
#include <clp/ffi/ir_stream/protocol_constants.hpp>
#include <clp/ir/types.hpp>
#include <clp/type_utils.hpp>

#include <clp_ffi_py/api_decoration.hpp>
#include <clp_ffi_py/ir/native/error_messages.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
CLP_FFI_PY_METHOD auto serialize_four_byte_preamble(PyObject* Py_UNUSED(self), PyObject* args)
        -> PyObject* {
    clp::ir::epoch_time_ms_t ref_timestamp{};
    char const* input_timestamp_format{};
    char const* input_timezone{};
    Py_ssize_t input_timestamp_format_size{};
    Py_ssize_t input_timezone_size{};

    if (0
        == PyArg_ParseTuple(
                args,
                "Ls#s#",
                &ref_timestamp,
                &input_timestamp_format,
                &input_timestamp_format_size,
                &input_timezone,
                &input_timezone_size
        ))
    {
        return nullptr;
    }

    std::string_view const timestamp_format{
            input_timestamp_format,
            static_cast<size_t>(input_timestamp_format_size)
    };
    std::string_view const timezone{input_timezone, static_cast<size_t>(input_timezone_size)};
    std::vector<int8_t> ir_buf;

    if (false
        == clp::ffi::ir_stream::four_byte_encoding::serialize_preamble(
                timestamp_format,
                {},
                timezone,
                ref_timestamp,
                ir_buf
        ))
    {
        PyErr_SetString(
                PyExc_NotImplementedError,
                get_c_str_from_constexpr_string_view(clp_ffi_py::ir::native::cSerializePreambleError
                )
        );
        return nullptr;
    }

    return PyByteArray_FromStringAndSize(
            clp::size_checked_pointer_cast<char>(ir_buf.data()),
            static_cast<Py_ssize_t>(ir_buf.size())
    );
}

CLP_FFI_PY_METHOD auto
serialize_four_byte_message_and_timestamp_delta(PyObject* Py_UNUSED(self), PyObject* args)
        -> PyObject* {
    clp::ir::epoch_time_ms_t delta{};
    char const* input_buffer{};
    Py_ssize_t input_buffer_size{};
    if (0 == PyArg_ParseTuple(args, "Ly#", &delta, &input_buffer, &input_buffer_size)) {
        return nullptr;
    }

    std::string logtype;
    std::vector<int8_t> ir_buf;
    std::string_view const msg{input_buffer, static_cast<size_t>(input_buffer_size)};

    // To avoid the frequent expansion of ir_buf, allocate sufficient space in advance
    ir_buf.reserve(input_buffer_size * 2);

    if (false == clp::ffi::ir_stream::four_byte_encoding::serialize_message(msg, logtype, ir_buf)) {
        PyErr_SetString(
                PyExc_NotImplementedError,
                get_c_str_from_constexpr_string_view(clp_ffi_py::ir::native::cSerializeMessageError)
        );
        return nullptr;
    }

    if (false == clp::ffi::ir_stream::four_byte_encoding::serialize_timestamp(delta, ir_buf)) {
        PyErr_SetString(
                PyExc_NotImplementedError,
                get_c_str_from_constexpr_string_view(
                        clp_ffi_py::ir::native::cSerializeTimestampError
                )
        );
        return nullptr;
    }

    return PyByteArray_FromStringAndSize(
            clp::size_checked_pointer_cast<char>(ir_buf.data()),
            static_cast<Py_ssize_t>(ir_buf.size())
    );
}

CLP_FFI_PY_METHOD auto serialize_four_byte_message(PyObject* Py_UNUSED(self), PyObject* args)
        -> PyObject* {
    char const* input_buffer{};
    Py_ssize_t input_buffer_size{};
    if (0 == PyArg_ParseTuple(args, "y#", &input_buffer, &input_buffer_size)) {
        return nullptr;
    }

    std::string log_type;
    std::vector<int8_t> ir_buf;
    std::string_view const msg{input_buffer, static_cast<size_t>(input_buffer_size)};

    // To avoid frequent resize of ir_buf, allocate sufficient space in advance
    ir_buf.reserve(input_buffer_size * 2);

    if (false == clp::ffi::ir_stream::four_byte_encoding::serialize_message(msg, log_type, ir_buf))
    {
        PyErr_SetString(
                PyExc_NotImplementedError,
                get_c_str_from_constexpr_string_view(clp_ffi_py::ir::native::cSerializeMessageError)
        );
        return nullptr;
    }

    return PyByteArray_FromStringAndSize(
            clp::size_checked_pointer_cast<char>(ir_buf.data()),
            static_cast<Py_ssize_t>(ir_buf.size())
    );
}

CLP_FFI_PY_METHOD auto
serialize_four_byte_timestamp_delta(PyObject* Py_UNUSED(self), PyObject* args) -> PyObject* {
    clp::ir::epoch_time_ms_t delta{};
    if (0 == PyArg_ParseTuple(args, "L", &delta)) {
        return nullptr;
    }

    std::vector<int8_t> ir_buf;
    if (false == clp::ffi::ir_stream::four_byte_encoding::serialize_timestamp(delta, ir_buf)) {
        PyErr_SetString(
                PyExc_NotImplementedError,
                get_c_str_from_constexpr_string_view(
                        clp_ffi_py::ir::native::cSerializeTimestampError
                )
        );
        return nullptr;
    }

    return PyByteArray_FromStringAndSize(
            clp::size_checked_pointer_cast<char>(ir_buf.data()),
            static_cast<Py_ssize_t>(ir_buf.size())
    );
}

CLP_FFI_PY_METHOD auto serialize_end_of_ir(PyObject* Py_UNUSED(self)) -> PyObject* {
    constexpr char cEof{clp::ffi::ir_stream::cProtocol::Eof};
    return PyByteArray_FromStringAndSize(&cEof, sizeof(cEof));
}
}  // namespace clp_ffi_py::ir::native
