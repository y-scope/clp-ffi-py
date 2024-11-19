#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header file

#include "Py_utils.hpp"

#include <string_view>

#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>

namespace clp_ffi_py {
namespace {
constexpr char const* const cPyFuncNameGetFormattedTimestamp{"get_formatted_timestamp"};
PyObjectStaticPtr<PyObject> Py_func_get_formatted_timestamp{nullptr};

constexpr char const* const cPyFuncNameGetTimezoneFromTimezoneId{"get_timezone_from_timezone_id"};
PyObjectStaticPtr<PyObject> Py_func_get_timezone_from_timezone_id{nullptr};

constexpr std::string_view cPyFuncNameSerializeDictToMsgpack{"serialize_dict_to_msgpack"};
PyObjectStaticPtr<PyObject> Py_func_serialize_dict_to_msgpack{nullptr};

constexpr std::string_view cPyFuncNameParseJsonStr{"parse_json_str"};
PyObjectStaticPtr<PyObject> Py_func_parse_json_str{nullptr};

/**
 * Wrapper of PyObject_CallObject.
 * @param func PyObject that points to the calling function.
 * @param args Function arguments.
 * @return PyObject* returned from PyObject_CallObject.
 */
auto py_utils_function_call_wrapper(PyObject* func, PyObject* args) -> PyObject* {
    return PyObject_CallObject(func, args);
}
}  // namespace

auto py_utils_init() -> bool {
    PyObjectPtr<PyObject> const utils_module(PyImport_ImportModule("clp_ffi_py.utils"));
    auto* py_utils{utils_module.get()};
    if (nullptr == py_utils) {
        return false;
    }

    Py_func_get_timezone_from_timezone_id.reset(
            PyObject_GetAttrString(py_utils, cPyFuncNameGetTimezoneFromTimezoneId)
    );
    if (nullptr == Py_func_get_timezone_from_timezone_id.get()) {
        return false;
    }

    Py_func_get_formatted_timestamp.reset(
            PyObject_GetAttrString(py_utils, cPyFuncNameGetFormattedTimestamp)
    );
    if (nullptr == Py_func_get_formatted_timestamp.get()) {
        return false;
    }

    Py_func_serialize_dict_to_msgpack.reset(
            PyObject_GetAttrString(py_utils, cPyFuncNameSerializeDictToMsgpack.data())
    );
    if (nullptr == Py_func_serialize_dict_to_msgpack.get()) {
        return false;
    }

    Py_func_parse_json_str.reset(PyObject_GetAttrString(py_utils, cPyFuncNameParseJsonStr.data()));
    if (nullptr == Py_func_parse_json_str.get()) {
        return false;
    }

    return true;
}

auto py_utils_get_formatted_timestamp(clp::ir::epoch_time_ms_t timestamp, PyObject* timezone)
        -> PyObject* {
    PyObjectPtr<PyObject> const func_args_ptr{Py_BuildValue("LO", timestamp, timezone)};
    auto* func_args{func_args_ptr.get()};
    if (nullptr == func_args) {
        return nullptr;
    }
    return py_utils_function_call_wrapper(Py_func_get_formatted_timestamp.get(), func_args);
}

auto py_utils_get_timezone_from_timezone_id(std::string const& timezone_id) -> PyObject* {
    PyObjectPtr<PyObject> const func_args_ptr{Py_BuildValue("(s)", timezone_id.c_str())};
    auto* func_args{func_args_ptr.get()};
    if (nullptr == func_args) {
        return nullptr;
    }
    return py_utils_function_call_wrapper(Py_func_get_timezone_from_timezone_id.get(), func_args);
}

auto py_utils_serialize_dict_to_msgpack(PyDictObject* py_dict) -> PyBytesObject* {
    PyObjectPtr<PyObject> const func_args_ptr{
            Py_BuildValue("(O)", py_reinterpret_cast<PyObject>(py_dict))
    };
    auto* func_args{func_args_ptr.get()};
    if (nullptr == func_args) {
        return nullptr;
    }
    auto* result{py_utils_function_call_wrapper(Py_func_serialize_dict_to_msgpack.get(), func_args)
    };
    if (nullptr == result) {
        return nullptr;
    }
    if (false == static_cast<bool>(PyBytes_Check(result))) {
        PyErr_SetString(
                PyExc_TypeError,
                "`serialize_dict_to_msgpack` is supposed to return a `bytes` object"
        );
        return nullptr;
    }

    return py_reinterpret_cast<PyBytesObject>(result);
}

auto py_utils_parse_json_str(std::string_view json_str) -> PyObject* {
    PyObjectPtr<PyObject> const func_args_ptr{Py_BuildValue("(s)", json_str.data())};
    auto* func_args{func_args_ptr.get()};
    if (nullptr == func_args) {
        return nullptr;
    }
    return py_utils_function_call_wrapper(Py_func_parse_json_str.get(), func_args);
}
}  // namespace clp_ffi_py
