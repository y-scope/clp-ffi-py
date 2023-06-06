#include "../Python.hpp"

#include "../encoder/encoding_methods.hpp"

PyDoc_STRVAR(
        encode_preamble_doc,
        "encode_preamble(ref_timestamp, timestamp_format, timezone)\n"
        "--\n\n"
        "Creates the encoded CLP preamble for a stream of encoded log messages.\n"
        ":param ref_timestamp: Reference timestamp used to calculate deltas emitted with each "
        "message.\n"
        ":param timestamp_format: Timestamp format to be use when generating the logs with a "
        "reader.\n"
        ":param timezone: Timezone in TZID format to be use when generating the timestamp "
        "from Unix epoch time.\n"
        ":raises NotImplementedError: If metadata length too large.\n"
        ":return: The encoded preamble.\n");

PyDoc_STRVAR(
        encode_message_and_timestamp_delta_doc,
        "encode_message_and_timestamp_delta(timestamp_delta, msg)\n"
        "--\n\n"
        "Encodes the log `msg` along with the timestamp delta.\n"
        ":param timestamp_delta: Timestamp difference in miliseconds between the current log "
        "message and the previous log message.\n"
        ":param msg: Log message to encode.\n"
        ":raises NotImplementedError: If the log message failed to encode, or the timestamp delta "
        "exceeds the supported size.\n"
        ":return: The encoded message and timestamp.\n");

PyDoc_STRVAR(
        encode_message_doc,
        "encode_message(msg)\n"
        "--\n\n"
        "Encodes the log `msg`.\n"
        ":param msg: Log message to encode.\n"
        ":raises NotImplementedError: If the log message failed to encode.\n"
        ":return: The encoded message.\n");

PyDoc_STRVAR(
        encode_timestamp_delta_doc,
        "encode_timestamp_delta(timestamp_delta)\n"
        "--\n\n"
        "Encodes the timestamp.\n"
        ":param timestamp_delta: Timestamp difference in miliseconds between the current log "
        "message and the previous log message.\n"
        ":raises NotImplementedError: If the timestamp failed to encode.\n"
        ":return: The encoded timestamp.\n");

PyDoc_STRVAR(module_doc, "Python interface to the CLP IR four byte encoding methods.");

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
         encode_message_and_timestamp_delta_doc},

        {"encode_message",
         clp_ffi_py::encoder::four_byte_encoding::encode_message,
         METH_VARARGS,
         encode_message_doc},

        {"encode_timestamp_delta",
         clp_ffi_py::encoder::four_byte_encoding::encode_timestamp_delta,
         METH_VARARGS,
         encode_timestamp_delta_doc},

        {nullptr, nullptr, 0, nullptr}};

/**
 * Module definition
 */
static PyModuleDef clp_four_byte_encoder = {
        PyModuleDef_HEAD_INIT, "CLPFourByteEncoder", module_doc, 0, encoder_method};

/**
 * Module initialization
 */
PyMODINIT_FUNC PyInit_CLPFourByteEncoder () {
    return PyModule_Create(&clp_four_byte_encoder);
}
