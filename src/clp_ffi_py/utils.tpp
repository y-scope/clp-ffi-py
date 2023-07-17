#ifndef CLP_FFI_PY_UTILS_TPP
#define CLP_FFI_PY_UTILS_TPP

#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <type_traits>

#include <clp/components/core/src/ffi/encoding_methods.hpp>

namespace clp_ffi_py {
namespace {
template <typename T>
[[maybe_unused]] constexpr bool cAlwaysFalse{false};
}  // namespace

template <typename int_type>
auto parse_py_int(PyObject* py_int, int_type& val) -> bool {
    if (false == static_cast<bool>(PyLong_Check(py_int))) {
        PyErr_SetString(PyExc_TypeError, "parse_py_int receives none-integer argument.");
        return false;
    }

    if constexpr (std::is_same_v<int_type, size_t>) {
        val = PyLong_AsSize_t(py_int);
    } else if constexpr (std::is_same_v<int_type, ffi::epoch_time_ms_t>) {
        val = PyLong_AsLongLong(py_int);
    } else if constexpr (std::is_same_v<int_type, Py_ssize_t>) {
        val = PyLong_AsSsize_t(py_int);
    } else {
        static_assert(cAlwaysFalse<int_type>, "Given integer type not supported.");
    }

    return (nullptr == PyErr_Occurred());
}
}  // namespace clp_ffi_py
#endif
