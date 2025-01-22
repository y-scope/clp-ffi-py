#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include "utils.hpp"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include <clp/TraceableException.hpp>
#include <outcome/single-header/outcome.hpp>
#include <wrapped_facade_headers/msgpack.hpp>

#include <clp_ffi_py/ExceptionFFI.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>

namespace clp_ffi_py {
namespace {
/**
 * Gets the underlying py_string byte data.
 * @param py_string PyObject that represents a Python level string. Only Python Unicode object or an
 * instance of a Python Unicode subtype will be considered as valid input.
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
    view = std::string_view{str};
    return true;
}

auto get_py_bool(bool is_true) -> PyObject* {
    if (is_true) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

auto unpack_msgpack(std::span<char const> msgpack_byte_sequence)
        -> outcome_v2::std_result<msgpack::object_handle, std::string> {
    msgpack::object_handle handle;
    try {
        msgpack::unpack(handle, msgpack_byte_sequence.data(), msgpack_byte_sequence.size());
    } catch (msgpack::unpack_error const& error) {
        return std::string{error.what()};
    }
    return handle;
}

auto unpack_msgpack_map(std::span<char const> msgpack_byte_sequence)
        -> std::optional<msgpack::object_handle> {
    auto unpack_result{unpack_msgpack(msgpack_byte_sequence)};
    if (unpack_result.has_error()) {
        PyErr_SetString(PyExc_RuntimeError, unpack_result.error().c_str());
        return std::nullopt;
    }

    auto& msgpack_obj_handle{unpack_result.value()};
    if (msgpack::type::MAP != msgpack_obj_handle.get().type) {
        PyErr_SetString(PyExc_TypeError, "Unpacked msgpack is not a map");
        return std::nullopt;
    }

    return std::move(msgpack_obj_handle);
}

auto handle_traceable_exception(clp::TraceableException& exception) noexcept -> void {
    if (auto* py_ffi_exception{dynamic_cast<ExceptionFFI*>(&exception)}) {
        auto& exception_context{py_ffi_exception->get_py_exception_context()};
        if (exception_context.has_exception()) {
            exception_context.restore();
            return;
        }
    }

    PyErr_Format(
            PyExc_RuntimeError,
            "%s:%d: ErrorCode: %d; Message: %s",
            exception.get_filename(),
            exception.get_line_number(),
            static_cast<int>(exception.get_error_code()),
            exception.what()
    );
}

auto construct_py_str_from_string_view(std::string_view sv) -> PyObject* {
    return PyUnicode_FromStringAndSize(sv.data(), static_cast<Py_ssize_t>(sv.size()));
}

auto get_new_ref_to_py_none() -> PyObject* {
    Py_INCREF(Py_None);
    return Py_None;
}
}  // namespace clp_ffi_py
