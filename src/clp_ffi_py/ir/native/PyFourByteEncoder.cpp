#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include "PyFourByteEncoder.hpp"

#include <clp_ffi_py/ir/native/encoding_methods.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cEncodePreambleDoc,
        "encode_preamble(ref_timestamp, timestamp_format, timezone)\n"
        "--\n\n"
        "Creates the encoded CLP preamble for a stream of encoded log messages"
        " using the 4-byte encoding.\n\n"
        ":param ref_timestamp: Reference timestamp used to calculate deltas emitted with each "
        "message.\n"
        ":param timestamp_format: Timestamp format to be use when generating the logs with a "
        "reader.\n"
        ":param timezone: Timezone in TZID format to be use when generating the timestamp "
        "from Unix epoch time.\n"
        ":raises NotImplementedError: If metadata length too large.\n"
        ":return: The encoded preamble.\n"
);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cEncodeMessageAndTimestampDeltaDoc,
        "encode_message_and_timestamp_delta(timestamp_delta, msg)\n"
        "--\n\n"
        "Encodes the log `msg` along with the timestamp delta using the 4-byte encoding.\n\n"
        ":param timestamp_delta: Timestamp difference in milliseconds between the current log "
        "message and the previous log message.\n"
        ":param msg: Log message to encode.\n"
        ":raises NotImplementedError: If the log message failed to encode, or the timestamp delta "
        "exceeds the supported size.\n"
        ":return: The encoded message and timestamp.\n"
);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cEncodeMessageDoc,
        "encode_message(msg)\n"
        "--\n\n"
        "Encodes the log `msg` using the 4-byte encoding.\n\n"
        ":param msg: Log message to encode.\n"
        ":raises NotImplementedError: If the log message failed to encode.\n"
        ":return: The encoded message.\n"
);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cEncodeTimestampDeltaDoc,
        "encode_timestamp_delta(timestamp_delta)\n"
        "--\n\n"
        "Encodes the timestamp using the 4-byte encoding.\n\n"
        ":param timestamp_delta: Timestamp difference in milliseconds between the current log "
        "message and the previous log message.\n"
        ":raises NotImplementedError: If the timestamp failed to encode.\n"
        ":return: The encoded timestamp.\n"
);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cEncodeEndOfIrDoc,
        "encode_end_of_ir()\n"
        "--\n\n"
        "Encodes the byte sequence that indicates the end of a CLP IR stream. A stream that does "
        "not contain this will be considered as an incomplete IR stream.\n"
);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyMethodDef PyFourByteEncoder_method_table[]{
        {"encode_preamble",
         clp_ffi_py::ir::native::encode_four_byte_preamble,
         METH_VARARGS | METH_STATIC,
         static_cast<char const*>(cEncodePreambleDoc)},

        {"encode_message_and_timestamp_delta",
         clp_ffi_py::ir::native::encode_four_byte_message_and_timestamp_delta,
         METH_VARARGS | METH_STATIC,
         static_cast<char const*>(cEncodeMessageAndTimestampDeltaDoc)},

        {"encode_message",
         clp_ffi_py::ir::native::encode_four_byte_message,
         METH_VARARGS | METH_STATIC,
         static_cast<char const*>(cEncodeMessageDoc)},

        {"encode_timestamp_delta",
         clp_ffi_py::ir::native::encode_four_byte_timestamp_delta,
         METH_VARARGS | METH_STATIC,
         static_cast<char const*>(cEncodeTimestampDeltaDoc)},

        {"encode_end_of_ir",
         py_c_function_cast(clp_ffi_py::ir::native::encode_end_of_ir),
         METH_NOARGS | METH_STATIC,
         static_cast<char const*>(cEncodeEndOfIrDoc)},

        {nullptr, nullptr, 0, nullptr}
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyFourByteEncoderDoc,
        "Namespace for all CLP four byte IR encoding methods.\n\n"
        "Methods encode bytes from the log record to create a CLP log message. This class should "
        "never be instantiated since it only contains static methods.\n"
);

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-const-cast)
PyType_Slot PyFourByteEncoder_slots[]{
        {Py_tp_methods, static_cast<void*>(PyFourByteEncoder_method_table)},
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPyFourByteEncoderDoc))},
        {0, nullptr}
};
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-const-cast)

/**
 * PyFourByteEncoder Python type specifications.
 */
PyType_Spec PyFourByteEncoder_type_spec{
        "clp_ffi_py.ir.native.FourByteEncoder",
        sizeof(PyFourByteEncoder),
        0,
        Py_TPFLAGS_DEFAULT,
        static_cast<PyType_Slot*>(PyFourByteEncoder_slots)
};
}  // namespace

PyObjectStaticPtr<PyTypeObject> PyFourByteEncoder::m_py_type{nullptr};

auto PyFourByteEncoder::module_level_init(PyObject* py_module) -> bool {
    static_assert(std::is_trivially_destructible<PyFourByteEncoder>());
    auto* type{py_reinterpret_cast<PyTypeObject>(PyType_FromSpec(&PyFourByteEncoder_type_spec))};
    m_py_type.reset(type);
    if (nullptr == type) {
        return false;
    }
    // Explicitly set the tp_new to nullptr to mark this type non-instantiable.
    type->tp_new = nullptr;
    return add_python_type(type, "FourByteEncoder", py_module);
}
}  // namespace clp_ffi_py::ir::native
