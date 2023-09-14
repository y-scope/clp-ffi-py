#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <clp_ffi_py/ir/native/PyDecoder.hpp>
#include <clp_ffi_py/ir/native/PyDecoderBuffer.hpp>
#include <clp_ffi_py/ir/native/PyFourByteEncoder.hpp>
#include <clp_ffi_py/ir/native/PyLogEvent.hpp>
#include <clp_ffi_py/ir/native/PyMetadata.hpp>
#include <clp_ffi_py/ir/native/PyQuery.hpp>
#include <clp_ffi_py/Py_utils.hpp>

namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(cModuleDoc, "Python interface to the CLP IR encoding and decoding methods.");

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyMethodDef Py_native_method_table[]{{nullptr, nullptr, 0, nullptr}};

PyModuleDef Py_native{
        PyModuleDef_HEAD_INIT,
        "native",
        static_cast<char const*>(cModuleDoc),
        -1,
        static_cast<PyMethodDef*>(Py_native_method_table)
};
}  // namespace

// NOLINTNEXTLINE(modernize-use-trailing-return-type)
PyMODINIT_FUNC PyInit_native() {
    PyObject* new_module{PyModule_Create(&Py_native)};
    if (nullptr == new_module) {
        return nullptr;
    }

    if (false == clp_ffi_py::py_utils_init()) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir::native::PyDecoderBuffer::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir::native::PyMetadata::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir::native::PyLogEvent::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir::native::PyQuery::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir::native::PyDecoder::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    if (false == clp_ffi_py::ir::native::PyFourByteEncoder::module_level_init(new_module)) {
        Py_DECREF(new_module);
        return nullptr;
    }

    return new_module;
}
