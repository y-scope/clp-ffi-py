#include <chrono>

#include "../clp/components/core/src/ffi/encoding_methods.hpp"
#include "../clp/components/core/src/ffi/ir_stream/encoding_methods.hpp"
#include "encoding_method.hpp"

namespace clp_ffi_py::encoder::four_byte_encoding {
PyObject* encode_preamble (PyObject* self, PyObject* args) {
    ffi::epoch_time_ms_t ref_timestamp;
    char const* input_timestamp_format;
    char const* input_timezone;

    if (false ==
        PyArg_ParseTuple(args, "Lss", &ref_timestamp, &input_timestamp_format, &input_timezone)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Native preamble encoder failed to parse Python arguments.");
        Py_RETURN_NONE;
    }

    std::string timestamp_format(input_timestamp_format);
    std::string timezone(input_timezone);
    static const std::string timestamp_pattern_syntax = "";
    static std::vector<int8_t> ir_buf;
    if (false ==
        ffi::ir_stream::four_byte_encoding::encode_preamble(
                timestamp_format, timestamp_pattern_syntax, timezone, ref_timestamp, ir_buf)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Native preamble encoder failed to encode the preamble.");
        Py_RETURN_NONE;
    }

    PyObject* output_bytearray =
            PyByteArray_FromStringAndSize(reinterpret_cast<char*>(ir_buf.data()), ir_buf.size());

    // Release the input buffer
    return output_bytearray;
}

PyObject* encode_message_and_timestamp (PyObject* self, PyObject* args) {
    ffi::epoch_time_ms_t ref_timestamp;
    Py_buffer input_buffer;
    if (false == PyArg_ParseTuple(args, "Ly*", &ref_timestamp, &input_buffer)) {
        PyErr_SetString(PyExc_RuntimeError, "Native encoder failed to parse Python arguments.");
        Py_RETURN_NONE;
    }

    std::string logtype;
    std::vector<int8_t> ir_buf;

    std::string_view msg(static_cast<char const*>(input_buffer.buf), input_buffer.len);
    auto const ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::high_resolution_clock::now().time_since_epoch())
                            .count();
    const ffi::epoch_time_ms_t current_timestamp = static_cast<ffi::epoch_time_ms_t>(ms);
    auto const delta = current_timestamp - ref_timestamp;

    // Encode the message
    if (false == ffi::ir_stream::four_byte_encoding::encode_message(delta, msg, logtype, ir_buf)) {
        PyBuffer_Release(&input_buffer);
        PyErr_SetString(PyExc_RuntimeError, "Native message encoder failed to encode the message.");
        Py_RETURN_NONE;
    }

    PyObject* output_bytearray =
            PyByteArray_FromStringAndSize(reinterpret_cast<char*>(ir_buf.data()), ir_buf.size());

    PyBuffer_Release(&input_buffer);
    PyObject* py_timestamp = PyLong_FromLongLong(current_timestamp);
    PyObject* result_tuple = PyTuple_New(2);
    PyTuple_SetItem(result_tuple, 0, output_bytearray);
    PyTuple_SetItem(result_tuple, 1, py_timestamp);
    return result_tuple;
}

PyObject* encode_message (PyObject* self, PyObject* args) {
    Py_buffer input_buffer;
    if (false == PyArg_ParseTuple(args, "y*", &input_buffer)) {
        PyErr_SetString(PyExc_RuntimeError, "Native encoder failed to parse Python arguments.");
        Py_RETURN_NONE;
    }

    std::string logtype;
    std::vector<int8_t> ir_buf;

    std::string_view msg(static_cast<char const*>(input_buffer.buf), input_buffer.len);
    if (false == ffi::ir_stream::four_byte_encoding::encode_message(msg, logtype, ir_buf)) {
        PyErr_SetString(PyExc_RuntimeError, "Native encoder failed to encode the message.");
        Py_RETURN_NONE;
    }

    PyObject* output_bytearray =
            PyByteArray_FromStringAndSize(reinterpret_cast<char*>(ir_buf.data()), ir_buf.size());

    PyBuffer_Release(&input_buffer);
    return output_bytearray;
}

PyObject* encode_timestamp (PyObject* self, PyObject* args) {
    ffi::epoch_time_ms_t ref_timestamp;
    if (false == PyArg_ParseTuple(args, "L", &ref_timestamp)) {
        PyErr_SetString(PyExc_RuntimeError, "Native encoder failed to parse Python arguments.");
        Py_RETURN_NONE;
    }

    auto const ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::high_resolution_clock::now().time_since_epoch())
                            .count();
    const ffi::epoch_time_ms_t current_timestamp = static_cast<ffi::epoch_time_ms_t>(ms);
    auto const delta = current_timestamp - ref_timestamp;

    std::vector<int8_t> ir_buf;
    if (false == ffi::ir_stream::four_byte_encoding::encode_timestamp(delta, ir_buf)) {
        PyErr_SetString(PyExc_RuntimeError, "Native encoder failed to encode the timestamp delta.");
        Py_RETURN_NONE;
    }

    PyObject* output_bytearray =
            PyByteArray_FromStringAndSize(reinterpret_cast<char*>(ir_buf.data()), ir_buf.size());

    PyObject* py_timestamp = PyLong_FromLongLong(current_timestamp);
    PyObject* result_tuple = PyTuple_New(2);
    PyTuple_SetItem(result_tuple, 0, output_bytearray);
    PyTuple_SetItem(result_tuple, 1, py_timestamp);
    return result_tuple;
}
} // namespace clp_ffi_py::encoder::four_byte_encoding
