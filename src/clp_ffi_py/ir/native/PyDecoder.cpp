#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include "PyDecoder.hpp"

#include <clp_ffi_py/ir/native/decoding_methods.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cDecodePreambleDoc,
        "decode_preamble(decoder_buffer)\n"
        "--\n\n"
        "Decodes the encoded preamble from the IR stream buffered in the given decoder buffer.\n\n"
        ":param decoder_buffer: The decoder buffer of the encoded CLP IR stream.\n"
        ":raises: Appropriate exceptions with detailed information on any encountered failure.\n"
        ":return: The decoded preamble presented as a new instance of Metadata.\n"
);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cDecodeNextLogEventDoc,
        "decode_next_log_event(decoder_buffer, query=None, allow_incomplete_stream=False)\n"
        "--\n\n"
        "Decodes the next encoded log event from the IR stream buffered in the given decoder "
        "buffer. `decoder_buffer` must have been returned by a successfully invocation of "
        "`decode_preamble`. If `query` is provided, only the next log event matching the query "
        "will be returned.\n\n"
        ":param decoder_buffer: The decoder buffer of the encoded CLP IR stream.\n"
        ":param query: A Query object that filters log events. See `Query` documents for more "
        "details.\n"
        ":param allow_incomplete_stream: If set to `True`, an incomplete CLP IR stream is not "
        "treated as an error. Instead, encountering such a stream is seen as reaching its end, and "
        "the function will return None without raising any exceptions.\n"
        ":raises: Appropriate exceptions with detailed information on any encountered failure.\n"
        ":return:\n"
        "     - A newly created LogEvent instance representing the next decoded log event from "
        "       the IR stream (if the query is `None`).\n"
        "     - A newly created LogEvent instance representing the next decoded log event "
        "       matched with the given query in the IR stream (if the query is given).\n"
        "     - None when the end of IR stream is reached or the query search terminates.\n"
);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyMethodDef PyDecoder_method_table[]{
        {"decode_preamble",
         decode_preamble,
         METH_O | METH_STATIC,
         static_cast<char const*>(cDecodePreambleDoc)},

        {"decode_next_log_event",
         py_c_function_cast(decode_next_log_event),
         METH_VARARGS | METH_KEYWORDS | METH_STATIC,
         static_cast<char const*>(cDecodeNextLogEventDoc)},

        {nullptr, nullptr, 0, nullptr}
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyDecoderDoc,
        "Namespace for all CLP IR decoding methods.\n\n"
        "Methods decode log events from encoded CLP IR streams. This class should never be "
        "instantiated since it only contains static methods.\n"
);

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-const-cast)
PyType_Slot PyDecoder_slots[]{
        {Py_tp_methods, static_cast<void*>(PyDecoder_method_table)},
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPyDecoderDoc))},
        {0, nullptr}
};
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-const-cast)

/**
 * PyDecoder Python type specifications.
 */
PyType_Spec PyDecoder_type_spec{
        "clp_ffi_py.ir.native.Decoder",
        sizeof(PyDecoder),
        0,
        Py_TPFLAGS_DEFAULT,
        static_cast<PyType_Slot*>(PyDecoder_slots)
};
}  // namespace

PyObjectStaticPtr<PyTypeObject> PyDecoder::m_py_type{nullptr};

auto PyDecoder::module_level_init(PyObject* py_module) -> bool {
    static_assert(std::is_trivially_destructible<PyDecoder>());
    auto* type{py_reinterpret_cast<PyTypeObject>(PyType_FromSpec(&PyDecoder_type_spec))};
    m_py_type.reset(type);
    if (nullptr == type) {
        return false;
    }
    // Explicitly set the tp_new to nullptr to mark this type non-instantiable.
    type->tp_new = nullptr;
    return add_python_type(type, "Decoder", py_module);
}
}  // namespace clp_ffi_py::ir::native
