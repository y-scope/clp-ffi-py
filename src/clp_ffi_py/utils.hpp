#ifndef CLP_FFI_PY_UTILS_HPP
#define CLP_FFI_PY_UTILS_HPP

#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <iostream>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

#include <clp/ffi/encoding_methods.hpp>
#include <clp/TraceableException.hpp>
#include <msgpack.hpp>
#include <outcome/single-header/outcome.hpp>

namespace clp_ffi_py {
/**
 * Adds the given Python type to the given Python module.
 * @param new_type Type object to add.
 * @param type_name Type name identifier.
 * @param module Python module to hold the new type.
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
auto add_python_type(PyTypeObject* new_type, char const* type_name, PyObject* module) -> bool;

/**
 * Parses a Python string into std::string.
 * @param py_string PyObject that represents a Python level string. Only Python Unicode object or an
 * instance of a Python Unicode subtype will be considered as valid input.
 * @param out The string parsed.
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
auto parse_py_string(PyObject* py_string, std::string& out) -> bool;

/**
 * Parses a Python string into std::string_view.
 * @param py_string PyObject that represents a Python level string. Only Python Unicode object or an
 * instance of a Python Unicode subtype will be considered as valid input.
 * @param view The string_view of the underlying byte data of py_string.
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
auto parse_py_string_as_string_view(PyObject* py_string, std::string_view& view) -> bool;

/**
 * Gets the Python True/False object from a given `bool` value/expression.
 * @param is_true A boolean value/expression.
 * @return PyObject that is either Python True or Python False.
 */
auto get_py_bool(bool is_true) -> PyObject*;

/**
 * Parses a Python integer into an int_type variable.
 * @tparam int_type Output integer type (size and signed/unsigned).
 * @param py_int PyObject that represents a Python level integer. Only PyLongObject or an instance
 * of a subtype of PyLongObject will be considered as valid input.
 * @param val The integer parsed.
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
template <typename int_type>
auto parse_py_int(PyObject* py_int, int_type& val) -> bool;

/**
 * Unpacks the given msgpack byte sequence.
 * @param msgpack_byte_sequence
 * @return A result containing the unpacked msgpack object handle on success or an error string
 * indicating the unpack failure (forwarded from the thrown `msgpack::unpack_error`).
 */
[[nodiscard]] auto unpack_msgpack(std::span<char const> msgpack_byte_sequence
) -> outcome_v2::std_result<msgpack::object_handle, std::string>;

/*
 * Handles a `clp::TraceableException` by setting a Python exception accordingly.
 * @param exception
 */
auto handle_traceable_exception(clp::TraceableException& exception) noexcept -> void;

/**
 * A template that always evaluates as false.
 */
template <typename T>
[[maybe_unused]] constexpr bool cAlwaysFalse{false};

template <typename int_type>
auto parse_py_int(PyObject* py_int, int_type& val) -> bool {
    if (false == static_cast<bool>(PyLong_Check(py_int))) {
        PyErr_SetString(PyExc_TypeError, "parse_py_int receives none-integer argument.");
        return false;
    }

    if constexpr (std::is_same_v<int_type, size_t>) {
        val = PyLong_AsSize_t(py_int);
    } else if constexpr (std::is_same_v<int_type, clp::ir::epoch_time_ms_t>) {
        val = PyLong_AsLongLong(py_int);
    } else if constexpr (std::is_same_v<int_type, Py_ssize_t>) {
        val = PyLong_AsSsize_t(py_int);
    } else if constexpr (std::is_same_v<int_type, uint32_t>) {
        uint64_t const val_as_unsigned_long{PyLong_AsUnsignedLong(py_int)};
        if (nullptr != PyErr_Occurred()) {
            return false;
        }
        if (std::numeric_limits<uint32_t>::max() < val_as_unsigned_long) {
            PyErr_Format(
                    PyExc_OverflowError,
                    "The input integer %lu overflows the range of type `uint32_t`",
                    val_as_unsigned_long
            );
            return false;
        }
        val = static_cast<uint32_t>(val_as_unsigned_long);
    } else {
        static_assert(cAlwaysFalse<int_type>, "Given integer type not supported.");
    }

    return (nullptr == PyErr_Occurred());
}
}  // namespace clp_ffi_py

#endif  // CLP_FFI_PY_UTILS_HPP
