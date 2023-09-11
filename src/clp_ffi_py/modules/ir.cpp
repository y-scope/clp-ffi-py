#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <clp_ffi_py/ir_ffi/PyDecoder.hpp>
#include <clp_ffi_py/ir_ffi/PyDecoderBuffer.hpp>
#include <clp_ffi_py/ir_ffi/PyFourByteEncoder.hpp>
#include <clp_ffi_py/ir_ffi/PyLogEvent.hpp>
#include <clp_ffi_py/ir_ffi/PyMetadata.hpp>
#include <clp_ffi_py/ir_ffi/PyQuery.hpp>
#include <clp_ffi_py/Py_utils.hpp>

namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(cModuleDoc, "Python interface to the CLP IR encoding and decoding methods.");

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyMethodDef Py_ir_ffi_method_table[]{{nullptr, nullptr, 0, nullptr}};

PyModuleDef Py_ir_ffi{
        PyModuleDef_HEAD_INIT,
        "ir_ffi",
        static_cast<char const*>(cModuleDoc),
        -1,
        static_cast<PyMethodDef*>(Py_ir_ffi_method_table)
};
}  // namespace

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
PyMODINIT_FUNC PyInit_ir_ffi() {
    PyObject* new_module{PyModule_Create(&Py_ir_ffi)};
    if (nullptr == new_module) {
        return nullptr;
    }

    if (false == clp_ffi_py::py_utils_init()) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir_ffi::PyDecoderBuffer::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir_ffi::PyMetadata::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir_ffi::PyLogEvent::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir_ffi::PyQuery::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir_ffi::PyDecoder::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir_ffi::PyFourByteEncoder::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    return new_module;
}
