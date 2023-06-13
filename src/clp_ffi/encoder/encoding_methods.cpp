#include "../Python.hpp"

#include "../../clp/components/core/src/ffi/encoding_methods.hpp"
#include "../../clp/components/core/src/ffi/ir_stream/encoding_methods.hpp"
#include "../../clp/components/core/src/type_utils.hpp"
#include "../ErrorMessage.hpp"
#include "encoding_methods.hpp"

// NOLINTBEGIN(cppcoreguidelines-init-variables)
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace clp_ffi_py::encoder::four_byte_encoding {
auto encode_preamble (PyObject* Py_UNUSED(self), PyObject* args) -> PyObject* {
    ffi::epoch_time_ms_t ref_timestamp;
    char const* input_timestamp_format;
    char const* input_timezone;
    Py_ssize_t input_timestamp_format_size;
    Py_ssize_t input_timezone_size;

    if (0 == PyArg_ParseTuple(
                     args,
                     "Ls#s#",
                     &ref_timestamp,
                     &input_timestamp_format,
                     &input_timestamp_format_size,
                     &input_timezone,
                     &input_timezone_size)) {
        return nullptr;
    }

    const std::string timestamp_format{
            input_timestamp_format,
            static_cast<size_t>(input_timestamp_format_size)};
    const std::string timezone{input_timezone, static_cast<size_t>(input_timezone_size)};
    const std::string timestamp_pattern_syntax{};
    std::vector<int8_t> ir_buf;

    if (false == ffi::ir_stream::four_byte_encoding::encode_preamble(
                         timestamp_format,
                         timestamp_pattern_syntax,
                         timezone,
                         ref_timestamp,
                         ir_buf)) {
        PyErr_SetString(
                PyExc_NotImplementedError,
                clp_ffi_py::error_messages::encoder::cPreambleError);
        return nullptr;
    }

    return PyByteArray_FromStringAndSize(
            size_checked_pointer_cast<char>(ir_buf.data()),
            static_cast<Py_ssize_t>(ir_buf.size()));
}

auto encode_message_and_timestamp_delta (PyObject* Py_UNUSED(self), PyObject* args) -> PyObject* {
    ffi::epoch_time_ms_t delta;
    char const* input_buffer;
    Py_ssize_t input_buffer_size;
    if (0 == PyArg_ParseTuple(args, "Ly#", &delta, &input_buffer, &input_buffer_size)) {
        return nullptr;
    }

    std::string logtype;
    std::vector<int8_t> ir_buf;
    std::string_view msg{input_buffer, static_cast<size_t>(input_buffer_size)};

    // To avoid the frequent expansion of ir_buf, allocate sufficient space in advance
    ir_buf.reserve(input_buffer_size * 2);

    // Encode the message
    if (false == ffi::ir_stream::four_byte_encoding::encode_message(msg, logtype, ir_buf)) {
        PyErr_SetString(
                PyExc_NotImplementedError,
                clp_ffi_py::error_messages::encoder::cMessageError);
        return nullptr;
    }

    // Encode the timestamp
    if (false == ffi::ir_stream::four_byte_encoding::encode_timestamp(delta, ir_buf)) {
        PyErr_SetString(
                PyExc_NotImplementedError,
                clp_ffi_py::error_messages::encoder::cTimestampError);
        return nullptr;
    }

    return PyByteArray_FromStringAndSize(
            size_checked_pointer_cast<char>(ir_buf.data()),
            static_cast<Py_ssize_t>(ir_buf.size()));
}

auto encode_message (PyObject* Py_UNUSED(self), PyObject* args) -> PyObject* {
    char const* input_buffer;
    Py_ssize_t input_buffer_size;
    if (0 == PyArg_ParseTuple(args, "y#", &input_buffer, &input_buffer_size)) {
        return nullptr;
    }

    std::string logtype;
    std::vector<int8_t> ir_buf;
    std::string_view msg{input_buffer, static_cast<size_t>(input_buffer_size)};

    // To avoid the frequent expansion of ir_buf, allocate sufficient space in advance
    ir_buf.reserve(input_buffer_size * 2);

    if (false == ffi::ir_stream::four_byte_encoding::encode_message(msg, logtype, ir_buf)) {
        PyErr_SetString(
                PyExc_NotImplementedError,
                clp_ffi_py::error_messages::encoder::cMessageError);
        return nullptr;
    }

    return PyByteArray_FromStringAndSize(
            size_checked_pointer_cast<char>(ir_buf.data()),
            static_cast<Py_ssize_t>(ir_buf.size()));
}

auto encode_timestamp_delta (PyObject* Py_UNUSED(self), PyObject* args) -> PyObject* {
    ffi::epoch_time_ms_t delta;
    if (0 == PyArg_ParseTuple(args, "L", &delta)) {
        return nullptr;
    }

    std::vector<int8_t> ir_buf;
    if (false == ffi::ir_stream::four_byte_encoding::encode_timestamp(delta, ir_buf)) {
        PyErr_SetString(
                PyExc_NotImplementedError,
                clp_ffi_py::error_messages::encoder::cTimestampError);
        return nullptr;
    }

    return PyByteArray_FromStringAndSize(
            size_checked_pointer_cast<char>(ir_buf.data()),
            static_cast<Py_ssize_t>(ir_buf.size()));
}
} // namespace clp_ffi_py::encoder::four_byte_encoding

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
// NOLINTEND(cppcoreguidelines-init-variables)
