#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include "PyFourByteDeserializer.hpp"

#include <type_traits>

#include <clp_ffi_py/ir/native/deserialization_methods.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyFourByteDeserializerDoc,
        "Namespace for all CLP four-byte encoded IR deserialization methods.\n\n"
        "Methods deserialize log events from serialized CLP IR streams. This class should never be "
        "instantiated since it only contains static methods.\n"
);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cDeserializePreambleDoc,
        "deserialize_preamble(deserializer_buffer)\n"
        "--\n\n"
        "Deserializes the preamble from the IR stream buffered in the given deserializer "
        "buffer.\n\n"
        ":param deserializer_buffer: The deserializer buffer of the serialized CLP IR stream.\n"
        ":raises: Appropriate exceptions with detailed information on any encountered failure.\n"
        ":return: The deserialized preamble presented as a new instance of Metadata.\n"
);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cDeserializeNextLogEventDoc,
        "deserialize_next_log_event(deserializer_buffer, query=None, allow_incomplete_stream=False)"
        "\n--\n\n"
        "Deserializes the next serialized log event from the IR stream buffered in the given "
        "deserializer buffer. `deserializer_buffer` must have been returned by a successfully "
        "invocation of `deserialize_preamble`. If `query` is provided, only the next log event "
        "matching the query will be returned.\n\n"
        ":param deserializer_buffer: The deserializer buffer of the serialized CLP IR stream.\n"
        ":param query: A Query object that filters log events. See `Query` documents for more "
        "details.\n"
        ":param allow_incomplete_stream: If set to `True`, an incomplete CLP IR stream is not "
        "treated as an error. Instead, encountering such a stream is seen as reaching its end, and "
        "the function will return None without raising any exceptions.\n"
        ":raises: Appropriate exceptions with detailed information on any encountered failure.\n"
        ":return:\n"
        "     - A newly created LogEvent instance representing the next deserialized log event "
        "       from the IR stream (if the query is `None`).\n"
        "     - A newly created LogEvent instance representing the next deserialized log event "
        "       matched with the given query in the IR stream (if the query is given).\n"
        "     - None when the end of IR stream is reached or the query search terminates.\n"
);

// NOLINTNEXTLINE(*-avoid-c-arrays, cppcoreguidelines-avoid-non-const-global-variables)
PyMethodDef PyFourByteDeserializer_method_table[]{
        {"deserialize_preamble",
         deserialize_preamble,
         METH_O | METH_STATIC,
         static_cast<char const*>(cDeserializePreambleDoc)},

        {"deserialize_next_log_event",
         py_c_function_cast(deserialize_next_log_event),
         METH_VARARGS | METH_KEYWORDS | METH_STATIC,
         static_cast<char const*>(cDeserializeNextLogEventDoc)},

        {nullptr, nullptr, 0, nullptr}
};

// NOLINTBEGIN(cppcoreguidelines-pro-type-*-cast)
// NOLINTNEXTLINE(*-avoid-c-arrays, cppcoreguidelines-avoid-non-const-global-variables)
PyType_Slot PyFourByteDeserializer_slots[]{
        {Py_tp_methods, static_cast<void*>(PyFourByteDeserializer_method_table)},
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPyFourByteDeserializerDoc))},
        {0, nullptr}
};
// NOLINTEND(cppcoreguidelines-pro-type-*-cast)

/**
 * `PyFourByteDeserializer`'s Python type specifications.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
PyType_Spec PyFourByteDeserializer_type_spec{
        "clp_ffi_py.ir.native.FourByteDeserializer",
        sizeof(PyFourByteDeserializer),
        0,
        Py_TPFLAGS_DEFAULT,
        static_cast<PyType_Slot*>(PyFourByteDeserializer_slots)
};
}  // namespace

auto PyFourByteDeserializer::module_level_init(PyObject* py_module) -> bool {
    static_assert(std::is_trivially_destructible<PyFourByteDeserializer>());
    auto* type{py_reinterpret_cast<PyTypeObject>(PyType_FromSpec(&PyFourByteDeserializer_type_spec))
    };
    m_py_type.reset(type);
    if (nullptr == type) {
        return false;
    }
    // Explicitly set the tp_new to nullptr to mark this type non-instantiable.
    type->tp_new = nullptr;
    return add_python_type(type, "FourByteDeserializer", py_module);
}
}  // namespace clp_ffi_py::ir::native
