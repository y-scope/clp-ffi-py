#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <clp_ffi_py/Py_utils.hpp>
#include <clp_ffi_py/ir_decoder/PyLogEvent.hpp>
#include <clp_ffi_py/ir_decoder/PyMetadata.hpp>

namespace {
PyMethodDef decoder_methods[] = {{nullptr, nullptr, 0, nullptr}};

PyDoc_STRVAR(cModuleDoc, "Python interface to the CLP IR Decoder.");

struct PyModuleDef clp_ir_decoder = {
        PyModuleDef_HEAD_INIT,
        "CLPIRDecoder",
        static_cast<char const*>(cModuleDoc),
        -1,
        decoder_methods};
} // namespace

extern "C" {
PyMODINIT_FUNC PyInit_CLPIRDecoder() {
    PyObject* new_module{PyModule_Create(&clp_ir_decoder)};
    if (nullptr == new_module) {
        return nullptr;
    }

    if (false == clp_ffi_py::Py_utils_init()) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir_decoder::PyLogEvent_module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir_decoder::PyMetadata_module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    return new_module;
}
}
