#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include "utils.hpp"

#include <iostream>

#include <clp_ffi_py/error_messages.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>

namespace clp_ffi_py {
namespace {
/**
 * Gets the underlying py_string byte data.
 * @param py_string PyObject that represents a Python level string. Only Python
 * Unicode object or an instance of a Python Unicode subtype will be considered
 * as valid input.
 * @return pointer to the byte data on success.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
auto get_py_string_data(PyObject* py_string) -> char const* {
    if (false == static_cast<bool>(PyUnicode_Check(py_string))) {
        PyErr_SetString(PyExc_TypeError, "parse_py_string receives none-string argument.");
        return nullptr;
    }
    return PyUnicode_AsUTF8(py_string);
}
}  // namespace

auto add_python_type(PyTypeObject* new_type, char const* type_name, PyObject* module) -> bool {
    assert(new_type);
    if (PyType_Ready(new_type) < 0) {
        return false;
    }
    if (PyModule_AddObject(module, type_name, py_reinterpret_cast<PyObject>(new_type)) < 0) {
        return false;
    }
    return true;
}

auto parse_py_string(PyObject* py_string, std::string& out) -> bool {
    char const* str{get_py_string_data(py_string)};
    if (nullptr == str) {
        return false;
    }
    out = std::string(str);
    return true;
}

auto parse_py_string_as_string_view(PyObject* py_string, std::string_view& view) -> bool {
    char const* str{get_py_string_data(py_string)};
    if (nullptr == str) {
        return false;
    }
    view = std::string_view(str);
    return true;
}

auto get_py_bool(bool is_true) -> PyObject* {
    if (is_true) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}
}  // namespace clp_ffi_py
