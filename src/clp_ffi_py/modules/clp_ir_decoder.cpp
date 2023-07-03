#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <clp_ffi_py/ir_decoder/PyLogEvent.hpp>
#include <clp_ffi_py/ir_decoder/PyMetadata.hpp>
#include <clp_ffi_py/Py_utils.hpp>

static PyMethodDef decoder_methods[] = {{NULL, NULL, 0, NULL}};

static struct PyModuleDef clp_ir_decoder =
        {PyModuleDef_HEAD_INIT, "CLPIRDecoder", nullptr, -1, decoder_methods};

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