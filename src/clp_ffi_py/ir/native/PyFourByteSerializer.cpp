#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include "PyFourByteSerializer.hpp"

#include <type_traits>

#include <clp_ffi_py/ir/native/serialization_methods.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyFourByteSerializerDoc,
        "Namespace for all CLP four byte IR serialization methods.\n\n"
        "Methods serialize bytes from the log record to create a CLP log message. This class "
        "should never be instantiated since it only contains static methods.\n"
);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cSerializePreambleDoc,
        "serialize_preamble(ref_timestamp, timestamp_format, timezone)\n"
        "--\n\n"
        "Serializes the preamble for a 4-byte encoded CLP IR stream.\n\n"
        ":param ref_timestamp: Reference timestamp used to calculate deltas emitted with each "
        "message.\n"
        ":param timestamp_format: Timestamp format to be use when generating the logs with a "
        "reader.\n"
        ":param timezone: Timezone in TZID format to be use when generating the timestamp "
        "from Unix epoch time.\n"
        ":raises NotImplementedError: If metadata length too large.\n"
        ":return: The serialized preamble.\n"
);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cSerializeMessageAndTimestampDeltaDoc,
        "serialize_message_and_timestamp_delta(timestamp_delta, msg)\n"
        "--\n\n"
        "Serializes the log `msg` along with the timestamp delta using the 4-byte encoding.\n\n"
        ":param timestamp_delta: Timestamp difference in milliseconds between the current log "
        "message and the previous log message.\n"
        ":param msg: Log message to serialize.\n"
        ":raises NotImplementedError: If the log message failed to serialize."
        ":return: The serialized message and timestamp.\n"
);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cSerializeMessageDoc,
        "serialize_message(msg)\n"
        "--\n\n"
        "Serializes the log `msg` using the 4-byte encoding.\n\n"
        ":param msg: Log message to serialize.\n"
        ":raises NotImplementedError: If the log message failed to serialize.\n"
        ":return: The serialized message.\n"
);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cSerializeTimestampDeltaDoc,
        "serialize_timestamp_delta(timestamp_delta)\n"
        "--\n\n"
        "Serializes the timestamp using the 4-byte encoding.\n\n"
        ":param timestamp_delta: Timestamp difference in milliseconds between the current log "
        "message and the previous log message.\n"
        ":raises NotImplementedError: If the timestamp failed to serialize.\n"
        ":return: The serialized timestamp.\n"
);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cSerializeEndOfIrDoc,
        "serialize_end_of_ir()\n"
        "--\n\n"
        "Serializes the byte sequence that indicates the end of a CLP IR stream. A stream that "
        "does not contain this will be considered as an incomplete IR stream.\n"
);

// NOLINTNEXTLINE(*-avoid-c-arrays, cppcoreguidelines-avoid-non-const-global-variables)
PyMethodDef PyFourByteSerializer_method_table[]{
        {"serialize_preamble",
         clp_ffi_py::ir::native::serialize_four_byte_preamble,
         METH_VARARGS | METH_STATIC,
         static_cast<char const*>(cSerializePreambleDoc)},

        {"serialize_message_and_timestamp_delta",
         clp_ffi_py::ir::native::serialize_four_byte_message_and_timestamp_delta,
         METH_VARARGS | METH_STATIC,
         static_cast<char const*>(cSerializeMessageAndTimestampDeltaDoc)},

        {"serialize_message",
         clp_ffi_py::ir::native::serialize_four_byte_message,
         METH_VARARGS | METH_STATIC,
         static_cast<char const*>(cSerializeMessageDoc)},

        {"serialize_timestamp_delta",
         clp_ffi_py::ir::native::serialize_four_byte_timestamp_delta,
         METH_VARARGS | METH_STATIC,
         static_cast<char const*>(cSerializeTimestampDeltaDoc)},

        {"serialize_end_of_ir",
         py_c_function_cast(clp_ffi_py::ir::native::serialize_end_of_ir),
         METH_NOARGS | METH_STATIC,
         static_cast<char const*>(cSerializeEndOfIrDoc)},

        {nullptr, nullptr, 0, nullptr}
};

// NOLINTBEGIN(cppcoreguidelines-pro-type-*-cast)
// NOLINTNEXTLINE(*-avoid-c-arrays, cppcoreguidelines-avoid-non-const-global-variables)
PyType_Slot PyFourByteSerializer_slots[]{
        {Py_tp_methods, static_cast<void*>(PyFourByteSerializer_method_table)},
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPyFourByteSerializerDoc))},
        {0, nullptr}
};
// NOLINTEND(cppcoreguidelines-pro-type-*-cast)

/**
 * `PyFourByteSerializer`'s Python type specifications.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
PyType_Spec PyFourByteSerializer_type_spec{
        "clp_ffi_py.ir.native.FourByteSerializer",
        sizeof(PyFourByteSerializer),
        0,
        Py_TPFLAGS_DEFAULT,
        static_cast<PyType_Slot*>(PyFourByteSerializer_slots)
};
}  // namespace

auto PyFourByteSerializer::module_level_init(PyObject* py_module) -> bool {
    static_assert(std::is_trivially_destructible<PyFourByteSerializer>());
    auto* type{py_reinterpret_cast<PyTypeObject>(PyType_FromSpec(&PyFourByteSerializer_type_spec))};
    m_py_type.reset(type);
    if (nullptr == type) {
        return false;
    }
    // Explicitly set the tp_new to nullptr to mark this type non-instantiable.
    type->tp_new = nullptr;
    return add_python_type(type, "FourByteSerializer", py_module);
}
}  // namespace clp_ffi_py::ir::native
