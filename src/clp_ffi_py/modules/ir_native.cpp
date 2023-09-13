#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <clp_ffi_py/ir_native/PyDecoder.hpp>
#include <clp_ffi_py/ir_native/PyDecoderBuffer.hpp>
#include <clp_ffi_py/ir_native/PyFourByteEncoder.hpp>
#include <clp_ffi_py/ir_native/PyLogEvent.hpp>
#include <clp_ffi_py/ir_native/PyMetadata.hpp>
#include <clp_ffi_py/ir_native/PyQuery.hpp>
#include <clp_ffi_py/Py_utils.hpp>

namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(cModuleDoc, "Python interface to the CLP IR encoding and decoding methods.");

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyMethodDef Py_ir_native_method_table[]{{nullptr, nullptr, 0, nullptr}};

PyModuleDef Py_ir_native{
        PyModuleDef_HEAD_INIT,
        "ir_native",
        static_cast<char const*>(cModuleDoc),
        -1,
        static_cast<PyMethodDef*>(Py_ir_native_method_table)
};
}  // namespace

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
PyMODINIT_FUNC PyInit_ir_native() {
    PyObject* new_module{PyModule_Create(&Py_ir_native)};
    if (nullptr == new_module) {
        return nullptr;
    }

    if (false == clp_ffi_py::py_utils_init()) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir_native::PyDecoderBuffer::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir_native::PyMetadata::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir_native::PyLogEvent::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir_native::PyQuery::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir_native::PyDecoder::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir_native::PyFourByteEncoder::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    return new_module;
}
