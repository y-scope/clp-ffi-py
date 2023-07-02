#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <clp_ffi_py/utils.hpp>

#include <iostream>

#include <clp_ffi_py/ErrorMessage.hpp>

namespace clp_ffi_py {
/**
 * Gets the underlying py_string byte data.
 * @param py_string PyObject that represents a Python level string. Only Python
 * Unicode object or an instance of a Python Unicode subtype will be considered
 * as valid input.
 * @return pointer to the byte data on success.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
static inline auto get_Py_string_data(PyObject* py_string) -> char const* {
    if (false == static_cast<bool>(PyUnicode_Check(py_string))) {
        PyErr_SetString(PyExc_TypeError, "parse_PyString receives none-string argument.");
        return nullptr;
    }
    return PyUnicode_AsUTF8(py_string);
}

auto add_type(
        PyTypeObject* new_type,
        char const* type_name,
        PyObject* module) -> bool {
    assert(new_type);
    auto* type_object{reinterpret_cast<PyObject*>(new_type)};
    if (PyModule_AddObject(module, type_name, type_object) < 0) {
        return false;
    }
    return true;
}

auto parse_PyString(PyObject* py_string, std::string& out) -> bool {
    char const* str{get_Py_string_data(py_string)};
    if (nullptr == str) {
        return false;
    }
    out = std::string(str);
    return true;
}

auto parse_PyString_as_string_view(PyObject* py_string, std::string_view& view) -> bool {
    char const* str{get_Py_string_data(py_string)};
    if (nullptr == str) {
        return false;
    }
    view = std::string_view(str);
    return true;
}
} // namespace clp_ffi_py
