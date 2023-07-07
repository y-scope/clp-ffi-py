#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <clp_ffi_py/ir/encoding_methods.hpp>
#include <clp_ffi_py/ir/PyLogEvent.hpp>
#include <clp_ffi_py/ir/PyMetadata.hpp>
#include <clp_ffi_py/Py_utils.hpp>

namespace {
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
        ":return: The encoded preamble.\n"
);

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
        ":return: The encoded message and timestamp.\n"
);

PyDoc_STRVAR(
        cEncodeMessageDoc,
        "encode_message(msg)\n"
        "--\n\n"
        "Encodes the log `msg` using the 4-byte encoding.\n"
        ":param msg: Log message to encode.\n"
        ":raises NotImplementedError: If the log message failed to encode.\n"
        ":return: The encoded message.\n"
);

PyDoc_STRVAR(
        cEncodeTimestampDeltaDoc,
        "encode_timestamp_delta(timestamp_delta)\n"
        "--\n\n"
        "Encodes the timestamp using the 4-byte encoding.\n"
        ":param timestamp_delta: Timestamp difference in milliseconds between the current log "
        "message and the previous log message.\n"
        ":raises NotImplementedError: If the timestamp failed to encode.\n"
        ":return: The encoded timestamp.\n"
);

/**
 * Method table
 */
PyMethodDef method_table[]{
        {"encode_preamble",
         clp_ffi_py::ir::four_byte_encoding::encode_preamble,
         METH_VARARGS,
         static_cast<char const*>(cEncodePreambleDoc)},

        {"encode_message_and_timestamp_delta",
         clp_ffi_py::ir::four_byte_encoding::encode_message_and_timestamp_delta,
         METH_VARARGS,
         static_cast<char const*>(cEncodeMessageAndTimestampDeltaDoc)},

        {"encode_message",
         clp_ffi_py::ir::four_byte_encoding::encode_message,
         METH_VARARGS,
         static_cast<char const*>(cEncodeMessageDoc)},

        {"encode_timestamp_delta",
         clp_ffi_py::ir::four_byte_encoding::encode_timestamp_delta,
         METH_VARARGS,
         static_cast<char const*>(cEncodeTimestampDeltaDoc)},

        {nullptr, nullptr, 0, nullptr}};

PyDoc_STRVAR(cModuleDoc, "Python interface to the CLP IR encoding and decoding methods.");

struct PyModuleDef clp_ir {
    PyModuleDef_HEAD_INIT, "CLPIR", static_cast<char const*>(cModuleDoc), -1, method_table
};
}  // namespace

extern "C" {
PyMODINIT_FUNC PyInit_CLPIR() {
    PyObject* new_module{PyModule_Create(&clp_ir)};
    if (nullptr == new_module) {
        return nullptr;
    }

    if (false == clp_ffi_py::Py_utils_init()) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir::PyLogEvent_module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir::PyMetadata_module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    return new_module;
}
}
