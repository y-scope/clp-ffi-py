#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include "PyDecoder.hpp"

#include <clp_ffi_py/ir/decoding_methods.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir {
namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(cDecodePreambleDoc, "\n");

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(cDecodeNextLogEventDoc, "\n");

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyMethodDef PyDecoder_method_table[]{
        {"decode_preamble",
         decode_preamble,
         METH_O | METH_STATIC,
         static_cast<char const*>(cDecodePreambleDoc)},

        {"decode_next_log_event",
         py_c_function_cast(decode_next_log_event),
         METH_VARARGS | METH_KEYWORDS | METH_STATIC,
         static_cast<char const*>(cDecodePreambleDoc)},

        {nullptr, nullptr, 0, nullptr}};

PyDoc_STRVAR(
        cPyDecoderDoc,
        "Namespace for all CLP IR decoding methods.\n\n"
        "Methods decode log events from encoded CLP IR stream. This class should never be "
        "instantiated "
        "since it only contains static methods.\n"
);

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-const-cast)
PyType_Slot PyDecoder_slots[]{
        {Py_tp_methods, static_cast<void*>(PyDecoder_method_table)},
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPyDecoderDoc))},
        {0, nullptr}};
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-const-cast)

/**
 * PyDecoder Python type specifications.
 */
PyType_Spec PyDecoder_type_spec{
        "clp_ffi_py.ir.Decoder",
        sizeof(PyDecoder),
        0,
        Py_TPFLAGS_DEFAULT,
        static_cast<PyType_Slot*>(PyDecoder_slots)};
}  // namespace

PyObjectPtr<PyTypeObject> PyDecoder::m_py_type{nullptr};

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
}  // namespace clp_ffi_py::ir
