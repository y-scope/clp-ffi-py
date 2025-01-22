#ifndef CLP_FFI_PY_UTILS_HPP
#define CLP_FFI_PY_UTILS_HPP

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

#include <clp/ir/types.hpp>
#include <clp/TraceableException.hpp>
#include <clp/type_utils.hpp>
#include <outcome/single-header/outcome.hpp>
#include <wrapped_facade_headers/msgpack.hpp>

namespace clp_ffi_py {
/**
 * Adds the given Python type to the given Python module.
 * @param new_type Type object to add.
 * @param type_name Type name identifier.
 * @param module Python module to hold the new type.
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto add_python_type(PyTypeObject* new_type, char const* type_name, PyObject* module)
        -> bool;

/**
 * Parses a Python string into std::string.
 * @param py_string PyObject that represents a Python level string. Only Python Unicode object or an
 * instance of a Python Unicode subtype will be considered as valid input.
 * @param out The string parsed.
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto parse_py_string(PyObject* py_string, std::string& out) -> bool;

/**
 * Parses a Python string into std::string_view.
 * @param py_string PyObject that represents a Python level string. Only Python Unicode object or an
 * instance of a Python Unicode subtype will be considered as valid input.
 * @param view The string_view of the underlying byte data of py_string.
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto parse_py_string_as_string_view(PyObject* py_string, std::string_view& view)
        -> bool;

/**
 * Gets the Python True/False object from a given `bool` value/expression.
 * @param is_true A boolean value/expression.
 * @return PyObject that is either Python True or Python False.
 */
[[nodiscard]] auto get_py_bool(bool is_true) -> PyObject*;

/**
 * Parses a Python integer into an int_type variable.
 * @tparam IntType Output integer type (size and signed/unsigned).
 * @param py_int PyObject that represents a Python level integer. Only PyLongObject or an instance
 * of a subtype of PyLongObject will be considered as valid input.
 * @param val The integer parsed.
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
template <clp::IntegerType IntType>
[[nodiscard]] auto parse_py_int(PyObject* py_int, IntType& val) -> bool;

/**
 * Unpacks the given msgpack byte sequence.
 * @param msgpack_byte_sequence
 * @return A result containing the unpacked msgpack object handle on success or an error string
 * indicating the unpack failure (forwarded from the thrown `msgpack::unpack_error`).
 */
[[nodiscard]] auto unpack_msgpack(std::span<char const> msgpack_byte_sequence)
        -> outcome_v2::std_result<msgpack::object_handle, std::string>;

/**
 * Unpacks a msgpack map from the given byte sequence.
 * @param msgpack_byte_sequence
 * @return A unpacked msgpack object handle on success.
 * @return std::optional with the relevant Python exception and error set on the following failures:
 * - Forward `unpack_msgpack`'s error.
 * - The unpacked msgpack object is not a map.
 */
[[nodiscard]] auto unpack_msgpack_map(std::span<char const> msgpack_byte_sequence)
        -> std::optional<msgpack::object_handle>;

/*
 * Handles a `clp::TraceableException` by setting a Python exception accordingly.
 * @param exception
 */
auto handle_traceable_exception(clp::TraceableException& exception) noexcept -> void;

/**
 * @param sv
 * @return A new reference to the constructed Python string object from the given string view `sv`.
 * @Return nullptr on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto construct_py_str_from_string_view(std::string_view sv) -> PyObject*;

/**
 * A template that always evaluates as false.
 */
template <typename T>
[[maybe_unused]] constexpr bool cAlwaysFalse{false};

/**
 * @param sv
 * @return The underlying C-string of the given constexpr string view.
 */
[[nodiscard]] consteval auto get_c_str_from_constexpr_string_view(std::string_view const& sv)
        -> char const*;

/**
 * @return A new reference to `Py_None`.
 */
[[nodiscard]] auto get_new_ref_to_py_none() -> PyObject*;

template <clp::IntegerType IntType>
auto parse_py_int(PyObject* py_int, IntType& val) -> bool {
    if (false == static_cast<bool>(PyLong_Check(py_int))) {
        PyErr_SetString(PyExc_TypeError, "parse_py_int receives none-integer argument.");
        return false;
    }

    if constexpr (std::is_same_v<IntType, size_t>) {
        val = PyLong_AsSize_t(py_int);
    } else if constexpr (std::is_same_v<IntType, clp::ir::epoch_time_ms_t>) {
        val = PyLong_AsLongLong(py_int);
    } else if constexpr (std::is_same_v<IntType, Py_ssize_t>) {
        val = PyLong_AsSsize_t(py_int);
    } else if constexpr (std::is_same_v<IntType, uint32_t>) {
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
        static_assert(cAlwaysFalse<IntType>, "Given integer type not supported.");
    }

    return (nullptr == PyErr_Occurred());
}

consteval auto get_c_str_from_constexpr_string_view(std::string_view const& sv) -> char const* {
    return sv.data();
}
}  // namespace clp_ffi_py

#endif  // CLP_FFI_PY_UTILS_HPP
