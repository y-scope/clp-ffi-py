#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files
#include <clp_ffi_py/encoder/encoding_methods.hpp>

// NOLINTBEGIN(*-avoid-c-arrays)
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
// NOLINTBEGIN(modernize-use-trailing-return-type)
// NOLINTBEGIN(readability-identifier-naming)

PyDoc_STRVAR(
        cEncodePreambleDoc,
        "encode_preamble(ref_timestamp, timestamp_format, timezone)\n"
        "--\n\n"
        "Creates the encoded CLP preamble for a stream of encoded log messages"
        " using the 4-byte encoding.\n"
        ":param ref_timestamp: Reference timestamp used to calculate deltas emitted with each "
        "message.\n"
        ":param timestamp_format: Timestamp format to be use when generating the logs with a "
        "reader.\n"
        ":param timezone: Timezone in TZID format to be use when generating the timestamp "
        "from Unix epoch time.\n"
        ":raises NotImplementedError: If metadata length too large.\n"
        ":return: The encoded preamble.\n");

PyDoc_STRVAR(
        cEncodeMessageAndTimestampDeltaDoc,
        "encode_message_and_timestamp_delta(timestamp_delta, msg)\n"
        "--\n\n"
        "Encodes the log `msg` along with the timestamp delta using the 4-byte encoding.\n"
        ":param timestamp_delta: Timestamp difference in milliseconds between the current log "
        "message and the previous log message.\n"
        ":param msg: Log message to encode.\n"
        ":raises NotImplementedError: If the log message failed to encode, or the timestamp delta "
        "exceeds the supported size.\n"
        ":return: The encoded message and timestamp.\n");

PyDoc_STRVAR(
        cEncodeMessageDoc,
        "encode_message(msg)\n"
        "--\n\n"
        "Encodes the log `msg` using the 4-byte encoding.\n"
        ":param msg: Log message to encode.\n"
        ":raises NotImplementedError: If the log message failed to encode.\n"
        ":return: The encoded message.\n");

PyDoc_STRVAR(
        cEncodeTimestampDeltaDoc,
        "encode_timestamp_delta(timestamp_delta)\n"
        "--\n\n"
        "Encodes the timestamp using the 4-byte encoding.\n"
        ":param timestamp_delta: Timestamp difference in milliseconds between the current log "
        "message and the previous log message.\n"
        ":raises NotImplementedError: If the timestamp failed to encode.\n"
        ":return: The encoded timestamp.\n");

PyDoc_STRVAR(cModuleDoc, "Python interface to the CLP IR 4-byte encoding methods.");

/**
 * Method table
 */
static PyMethodDef encoder_method_table[]{
        {"encode_preamble",
         clp_ffi_py::encoder::four_byte_encoding::encode_preamble,
         METH_VARARGS,
         static_cast<char const*>(cEncodePreambleDoc)},

        {"encode_message_and_timestamp_delta",
         clp_ffi_py::encoder::four_byte_encoding::encode_message_and_timestamp_delta,
         METH_VARARGS,
         static_cast<char const*>(cEncodeMessageAndTimestampDeltaDoc)},

        {"encode_message",
         clp_ffi_py::encoder::four_byte_encoding::encode_message,
         METH_VARARGS,
         static_cast<char const*>(cEncodeMessageDoc)},

        {"encode_timestamp_delta",
         clp_ffi_py::encoder::four_byte_encoding::encode_timestamp_delta,
         METH_VARARGS,
         static_cast<char const*>(cEncodeTimestampDeltaDoc)},

        {nullptr, nullptr, 0, nullptr}};

/**
 * Module definition
 */
static PyModuleDef clp_four_byte_encoder{
        PyModuleDef_HEAD_INIT,
        "CLPFourByteEncoder",
        static_cast<char const*>(cModuleDoc),
        0,
        static_cast<PyMethodDef*>(encoder_method_table)};

/**
 * Module initialization
 */
PyMODINIT_FUNC PyInit_CLPFourByteEncoder() {
    return PyModule_Create(&clp_four_byte_encoder);
}

// NOLINTEND(readability-identifier-naming)
// NOLINTEND(modernize-use-trailing-return-type)
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
// NOLINTEND(*-avoid-c-arrays)
