#include "Python.hpp"

#include "encoding_method.hpp"

PyDoc_STRVAR(
        encode_preamble_doc,
        "encode_preamble(ref_timestamp: int, timestamp_format: str, timezone: str) -> bytearray\n"
        "--\n\n"
        "Create the encoded CLP preamble for a stream of encoded log messages.\n"
        "Arguments: (ref_timestamp, timestamp_format, timezone)\n"
        ":param ref_timestamp: Reference timestamp used to calculate deltas emitted with each "
        "message.\n"
        ":param timestamp_format: Timestamp format to be use when generating the logs with a "
        "reader.\n"
        ":param timezone: Timezone in TZID format to be use when generating the timestamp "
        "from Unix epoch time.\n"
        ":raises NotImplementedError: If metadata length too large.\n"
        ":return: The encoded preamble.\n");

/**
 * Method table
 */
static PyMethodDef encoder_method[] = {
        {"encode_preamble",
         clp_ffi_py::encoder::four_byte_encoding::encode_preamble,
         METH_VARARGS,
         encode_preamble_doc},

        {"encode_message_and_timestamp_delta",
         clp_ffi_py::encoder::four_byte_encoding::encode_message_and_timestamp_delta,
         METH_VARARGS,
         "encode the log message with the message content and the timestamp delta."},

        {"encode_message",
         clp_ffi_py::encoder::four_byte_encoding::encode_message,
         METH_VARARGS,
         "encode the log message content"},

        {"encode_timestamp_delta",
         clp_ffi_py::encoder::four_byte_encoding::encode_timestamp_delta,
         METH_VARARGS,
         "encode the timestamp delta."},

        {nullptr, nullptr, 0, nullptr}};

/**
 * Module definition
 */
static struct PyModuleDef clp_four_byte_encoder = {
        PyModuleDef_HEAD_INIT, "CLPFourByteEncoder", NULL, -1, encoder_method};

/**
 * Module initialization
 */
PyMODINIT_FUNC PyInit_CLPFourByteEncoder (void) {
    return PyModule_Create(&clp_four_byte_encoder);
}
