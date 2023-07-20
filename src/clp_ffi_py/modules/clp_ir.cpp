#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <clp_ffi_py/ir/encoding_methods.hpp>
#include <clp_ffi_py/ir/PyFourByteEncoder.hpp>
#include <clp_ffi_py/ir/PyLogEvent.hpp>
#include <clp_ffi_py/ir/PyMetadata.hpp>
#include <clp_ffi_py/Py_utils.hpp>

namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(cModuleDoc, "Python interface to the CLP IR encoding and decoding methods.");

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyMethodDef PyCLPIR_method_table[]{{nullptr, nullptr, 0, nullptr}};

struct PyModuleDef PyCLPIR {
    PyModuleDef_HEAD_INIT, "CLPIR", static_cast<char const*>(cModuleDoc), -1,
            static_cast<PyMethodDef*>(PyCLPIR_method_table)
};
}  // namespace

extern "C" {
// NOLINTNEXTLINE(modernize-use-trailing-return-type)
PyMODINIT_FUNC PyInit_CLPIR() {
    PyObject* new_module{PyModule_Create(&PyCLPIR)};
    if (nullptr == new_module) {
        return nullptr;
    }

    if (false == clp_ffi_py::py_utils_init()) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir::PyMetadata::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir::PyLogEvent::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir::PyFourByteEncoder::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    return new_module;
}
}
