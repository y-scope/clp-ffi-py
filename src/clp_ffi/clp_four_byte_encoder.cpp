#include "encoding_method.hpp"

/**
 * Method table
*/
static PyMethodDef EncoderMethods[] = {
        {"encode_preamble",
         clp_ffi_py::encoder::four_byte_encoding::encode_preamble,
         METH_VARARGS,
         "encode the preamble with a reference timestamp, a timestamp format, and a time zone."},

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

        {NULL, NULL, 0, NULL}};

/**
 * Module definition
*/
static struct PyModuleDef clp_four_byte_encoder = {
        PyModuleDef_HEAD_INIT, "CLPFourByteEncoder", NULL, -1, EncoderMethods};

/**
 * Module initialization
*/
PyMODINIT_FUNC PyInit_CLPFourByteEncoder (void) {
    return PyModule_Create(&clp_four_byte_encoder);
}
