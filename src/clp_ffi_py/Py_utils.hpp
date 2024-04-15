#ifndef CLP_FFI_PY_PY_UTILS_HPP
#define CLP_FFI_PY_PY_UTILS_HPP

#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <clp/components/core/src/clp/ffi/encoding_methods.hpp>

namespace clp_ffi_py {
/**
 * Initializes CPython interface to Python level utility functions implemented
 * in submodule `clp_ffi_py.utils`.
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
auto py_utils_init() -> bool;

/**
 * CPython wrapper of clp_ffi_py.utils.get_formatted_timestamp.
 * @param timestamp
 * @param tzinfo Python tzinfo object that specifies timezone information.
 * @return a new reference of a PyObject string that stores the formatted
 * timestamp.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
auto py_utils_get_formatted_timestamp(clp::ir::epoch_time_ms_t timestamp, PyObject* timezone)
        -> PyObject*;

/**
 * CPython wrapper of clp_ffi_py.utils.get_timezone_from_timezone_id
 * @param timezone_id
 * @return a new reference of a Python tzinfo object that matches the input
 * timezone id.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
auto py_utils_get_timezone_from_timezone_id(std::string const& timezone_id) -> PyObject*;
}  // namespace clp_ffi_py
#endif  // CLP_FFI_PY_PY_UTILS_HPP
