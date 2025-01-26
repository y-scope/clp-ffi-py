#ifndef CLP_FFI_PY_PY_UTILS_HPP
#define CLP_FFI_PY_PY_UTILS_HPP

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include <string>
#include <string_view>

#include <clp/ir/types.hpp>

namespace clp_ffi_py {
/**
 * Initializes CPython interface to Python level utility functions implemented in submodule
 * `clp_ffi_py.utils`.
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto py_utils_init() -> bool;

/**
 * CPython wrapper of `clp_ffi_py.utils.get_formatted_timestamp`.
 * @param timestamp
 * @param tzinfo Python tzinfo object that specifies timezone information.
 * @return a new reference of a PyObject string that stores the formatted timestamp.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto
py_utils_get_formatted_timestamp(clp::ir::epoch_time_ms_t timestamp, PyObject* timezone)
        -> PyObject*;

/**
 * CPython wrapper of `clp_ffi_py.utils.get_timezone_from_timezone_id`.
 * @param timezone_id
 * @return a new reference of a Python tzinfo object that matches the input timezone id.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto py_utils_get_timezone_from_timezone_id(std::string const& timezone_id)
        -> PyObject*;

/**
 * CPython wrapper of `clp_ffi_py.utils.serialize_dict_to_msgpack`.
 * @param py_dict
 * @return a new reference of a `PyByteObject` containing msgpack-serialized dictionary.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto py_utils_serialize_dict_to_msgpack(PyDictObject* py_dict) -> PyBytesObject*;

/**
 * CPython wrapper of `clp_ffi_py.utils.serialize_dict_to_json_str`.
 * @param py_dict
 * @return a new reference of a Python Unicode object containing JSON string representation of the
 * dictionary.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto py_utils_serialize_dict_to_json_str(PyDictObject* py_dict) -> PyUnicodeObject*;

/**
 * CPython wrapper of `clp_ffi_py.utils.parse_json_str_to_dict`.
 * @param json_str
 * @return a new reference of the parsed JSON object.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto py_utils_parse_json_str(std::string_view json_str) -> PyObject*;
}  // namespace clp_ffi_py

#endif  // CLP_FFI_PY_PY_UTILS_HPP
