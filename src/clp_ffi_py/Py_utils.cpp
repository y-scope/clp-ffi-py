#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header file

#include "Py_utils.hpp"

#include <string>
#include <string_view>

#include <clp/ir/types.hpp>

#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py {
namespace {
constexpr std::string_view cPyFuncNameGetFormattedTimestamp{"get_formatted_timestamp"};
constexpr std::string_view cPyFuncNameGetTimezoneFromTimezoneId{"get_timezone_from_timezone_id"};
constexpr std::string_view cPyFuncNameSerializeDictToMsgpack{"serialize_dict_to_msgpack"};
constexpr std::string_view cPyFuncNameSerializeDictToJsonStr{"serialize_dict_to_json_str"};
constexpr std::string_view cPyFuncNameParseJsonStr{"parse_json_str"};

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
PyObjectStaticPtr<PyObject> Py_func_get_formatted_timestamp{nullptr};
PyObjectStaticPtr<PyObject> Py_func_get_timezone_from_timezone_id{nullptr};
PyObjectStaticPtr<PyObject> Py_func_serialize_dict_to_msgpack{nullptr};
PyObjectStaticPtr<PyObject> Py_func_serialize_dict_to_json_str{nullptr};
PyObjectStaticPtr<PyObject> Py_func_parse_json_str{nullptr};

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

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

    Py_func_get_timezone_from_timezone_id.reset(PyObject_GetAttrString(
            py_utils,
            get_c_str_from_constexpr_string_view(cPyFuncNameGetTimezoneFromTimezoneId)
    ));
    if (nullptr == Py_func_get_timezone_from_timezone_id.get()) {
        return false;
    }

    Py_func_get_formatted_timestamp.reset(PyObject_GetAttrString(
            py_utils,
            get_c_str_from_constexpr_string_view(cPyFuncNameGetFormattedTimestamp)
    ));
    if (nullptr == Py_func_get_formatted_timestamp.get()) {
        return false;
    }

    Py_func_serialize_dict_to_msgpack.reset(PyObject_GetAttrString(
            py_utils,
            get_c_str_from_constexpr_string_view(cPyFuncNameSerializeDictToMsgpack)
    ));
    if (nullptr == Py_func_serialize_dict_to_msgpack.get()) {
        return false;
    }

    Py_func_serialize_dict_to_json_str.reset(PyObject_GetAttrString(
            py_utils,
            get_c_str_from_constexpr_string_view(cPyFuncNameSerializeDictToJsonStr)
    ));
    if (nullptr == Py_func_serialize_dict_to_json_str.get()) {
        return false;
    }

    Py_func_parse_json_str.reset(PyObject_GetAttrString(
            py_utils,
            get_c_str_from_constexpr_string_view(cPyFuncNameParseJsonStr)
    ));
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

auto py_utils_serialize_dict_to_json_str(PyDictObject* py_dict) -> PyUnicodeObject* {
    PyObjectPtr<PyObject> const func_args_ptr{
            Py_BuildValue("(O)", py_reinterpret_cast<PyObject>(py_dict))
    };
    auto* func_args{func_args_ptr.get()};
    if (nullptr == func_args) {
        return nullptr;
    }
    auto* result{py_utils_function_call_wrapper(Py_func_serialize_dict_to_json_str.get(), func_args)
    };
    if (nullptr == result) {
        return nullptr;
    }
    if (false == static_cast<bool>(PyUnicode_Check(result))) {
        PyErr_Format(
                PyExc_TypeError,
                "`%s` is supposed to return a `str` object",
                cPyFuncNameSerializeDictToJsonStr
        );
        return nullptr;
    }

    return py_reinterpret_cast<PyUnicodeObject>(result);
}

auto py_utils_parse_json_str(std::string_view json_str) -> PyObject* {
    PyObjectPtr<PyObject> const func_args_ptr{
            Py_BuildValue("(s#)", json_str.data(), static_cast<Py_ssize_t>(json_str.size()))
    };
    auto* func_args{func_args_ptr.get()};
    if (nullptr == func_args) {
        return nullptr;
    }
    return py_utils_function_call_wrapper(Py_func_parse_json_str.get(), func_args);
}
}  // namespace clp_ffi_py
