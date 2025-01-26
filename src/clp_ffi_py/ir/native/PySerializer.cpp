#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include "PySerializer.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <new>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>

#include <clp/ffi/ir_stream/protocol_constants.hpp>
#include <json/single_include/nlohmann/json.hpp>
#include <wrapped_facade_headers/msgpack.hpp>

#include <clp_ffi_py/api_decoration.hpp>
#include <clp_ffi_py/error_messages.hpp>
#include <clp_ffi_py/ir/native/error_messages.hpp>
#include <clp_ffi_py/Py_utils.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
namespace {
/**
 * Callback of `PySerializer`'s `__init__` method:
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPySerializerDoc,
        "Serializer for serializing CLP key-value pair IR streams.\n"
        "This class serializes log events into the CLP key-value pair IR format and writes the"
        " serialized data to a specified byte stream object.\n\n"
        "__init__(self, output_stream, buffer_size_limit=65536, user_defined_metadata=None)\n\n"
        "Initializes a :class:`Serializer` instance with the given output stream. Note that each"
        " object should only be initialized once. Double initialization will result in a memory"
        " leak.\n\n"
        ":param output_stream: A writable byte output stream to which the serializer will write the"
        " serialized IR byte sequences.\n"
        ":type output_stream: IO[bytes]\n"
        ":param buffer_size_limit: The maximum amount of serialized data to buffer before flushing"
        " it to `output_stream`. Defaults to 64 KiB.\n"
        ":type buffer_size_limit: int\n"
        ":param user_defined_metadata: A dictionary representing user-defined stream-level"
        " metadata, or None to indicate the absence of such metadata. If a dictionary is provided,"
        " it must be valid for serialization as a string using the `Python Standard JSON library"
        " <https://docs.python.org/3/library/json.html>`_\.\n"
        ":type user_defined_metadata: dict | None\n"
);
CLP_FFI_PY_METHOD auto PySerializer_init(PySerializer* self, PyObject* args, PyObject* keywords)
        -> int;

/**
 * Callback of `PySerializer`'s `serialize_msgpack` method.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPySerializerSerializeLogEventFromMsgpackMapDoc,
        "serialize_log_event_from_msgpack_map(self, auto_gen_msgpack_map, user_gen_msgpack_map)\n"
        "--\n\n"
        "Serializes the given log event from the given packed msgpack maps.\n\n"
        ":param auto_gen_msgpack_map: The auto-generated key-value pairs of the log event as a"
        " packed msgpack map where all keys are strings.\n"
        ":type auto_gen_msgpack_map: bytes\n"
        ":param user_gen_msgpack_map: The user-generated key-value pairs of the log event as a"
        " packed msgpack map where all keys are strings.\n"
        ":type user_gen_msgpack_map: bytes\n"
        ":return: The number of bytes serialized.\n"
        ":rtype: int\n"
        ":raise IOError: If the serializer has already been closed.\n"
        ":raise TypeError: If `auto_gen_msgpack_map` or `user_gen_msgpack_map` is not a packed"
        " msgpack map.\n"
        ":raise RuntimeError: If `auto_gen_msgpack_map` or `user_gen_msgpack_map` couldn't be"
        " unpacked or serialization into the IR stream failed.\n"
);
CLP_FFI_PY_METHOD auto PySerializer_serialize_log_event_from_msgpack_map(
        PySerializer* self,
        PyObject* args,
        PyObject* keywords
) -> PyObject*;

/**
 * Callback of `PySerializer`'s `get_num_bytes_serialized` method.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPySerializerGetNumBytesSerializedDoc,
        "get_num_bytes_serialized(self)\n"
        "--\n\n"
        ":return: The total number of bytes serialized.\n"
        ":rtype: int\n"
        ":raise IOError: If the serializer has already been closed.\n"
);
CLP_FFI_PY_METHOD auto PySerializer_get_num_bytes_serialized(PySerializer* self) -> PyObject*;

/**
 * Callback of `PySerializer`'s `flush` method.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPySerializerFlushDoc,
        "flush(self)\n"
        "--\n\n"
        "Flushes any buffered data and the output stream.\n\n"
        ":raise IOError: If the serializer has already been closed.\n"
);
CLP_FFI_PY_METHOD auto PySerializer_flush(PySerializer* self) -> PyObject*;

/**
 * Callback of `PySerializer`'s `close` method.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPySerializerCloseDoc,
        "close(self)\n"
        "--\n\n"
        "Closes the serializer, writing any buffered data to the output stream and appending a byte"
        " sequence to mark the end of the CLP IR stream. The output stream is then flushed and"
        " closed.\n"
        "NOTE: This method must be called to properly terminate an IR stream. If it isn't called,"
        " the stream will be incomplete, and any buffered data may be lost.\n\n"
        ":raise IOError: If the serializer has already been closed.\n"
);
CLP_FFI_PY_METHOD auto PySerializer_close(PySerializer* self) -> PyObject*;

/**
 * Callback of `PySerializer`'s `__enter__` method.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPySerializerEnterDoc,
        "__enter__(self)\n"
        "--\n\n"
        "Enters the runtime context.\n\n"
        ":return: self.\n"
        ":rtype: :class:`Serializer`\n"
);
CLP_FFI_PY_METHOD auto PySerializer_enter(PySerializer* self) -> PyObject*;

/**
 * Callback of `PySerializer`'s `__exit__` method.
 */
PyDoc_STRVAR(
        cPySerializerExitDoc,
        "__exit__(self, exc_type, exc_value, traceback)\n"
        "--\n\n"
        "Exits the runtime context, automatically calling :meth:`close` to flush all buffered data"
        " into the output stream."
        ":param exc_type: The type of the exception that caused the exit. Unused.\n"
        ":param exc_value: The value of the exception that caused the exit. Unused.\n"
        ":param exc_traceable: The traceback. Unused.\n"
);
CLP_FFI_PY_METHOD auto PySerializer_exit(PySerializer* self, PyObject* args, PyObject* keywords)
        -> PyObject*;

/**
 * Callback of `PySerializer`'s deallocator.
 */
CLP_FFI_PY_METHOD auto PySerializer_dealloc(PySerializer* self) -> void;

// NOLINTNEXTLINE(*-avoid-c-arrays, cppcoreguidelines-avoid-non-const-global-variables)
PyMethodDef PySerializer_method_table[]{
        {"serialize_log_event_from_msgpack_map",
         py_c_function_cast(PySerializer_serialize_log_event_from_msgpack_map),
         METH_VARARGS | METH_KEYWORDS,
         static_cast<char const*>(cPySerializerSerializeLogEventFromMsgpackMapDoc)},

        {"get_num_bytes_serialized",
         py_c_function_cast(PySerializer_get_num_bytes_serialized),
         METH_NOARGS,
         static_cast<char const*>(cPySerializerGetNumBytesSerializedDoc)},

        {"flush",
         py_c_function_cast(PySerializer_flush),
         METH_NOARGS,
         static_cast<char const*>(cPySerializerFlushDoc)},

        {"close",
         py_c_function_cast(PySerializer_close),
         METH_NOARGS,
         static_cast<char const*>(cPySerializerCloseDoc)},

        {"__enter__",
         py_c_function_cast(PySerializer_enter),
         METH_NOARGS,
         static_cast<char const*>(cPySerializerEnterDoc)},

        {"__exit__",
         py_c_function_cast(PySerializer_exit),
         METH_VARARGS | METH_KEYWORDS,
         static_cast<char const*>(cPySerializerExitDoc)},

        {nullptr}
};

// NOLINTBEGIN(cppcoreguidelines-pro-type-*-cast)
// NOLINTNEXTLINE(*-avoid-c-arrays, cppcoreguidelines-avoid-non-const-global-variables)
PyType_Slot PySerializer_slots[]{
        {Py_tp_alloc, reinterpret_cast<void*>(PyType_GenericAlloc)},
        {Py_tp_dealloc, reinterpret_cast<void*>(PySerializer_dealloc)},
        {Py_tp_new, reinterpret_cast<void*>(PyType_GenericNew)},
        {Py_tp_init, reinterpret_cast<void*>(PySerializer_init)},
        {Py_tp_methods, static_cast<void*>(PySerializer_method_table)},
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPySerializerDoc))},
        {0, nullptr}
};
// NOLINTEND(cppcoreguidelines-pro-type-*-cast)

/**
 * `PySerializer`'s Python type specifications.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
PyType_Spec PySerializer_type_spec{
        "clp_ffi_py.ir.native.Serializer",
        sizeof(PySerializer),
        0,
        Py_TPFLAGS_DEFAULT,
        static_cast<PyType_Slot*>(PySerializer_slots)
};

CLP_FFI_PY_METHOD auto PySerializer_init(PySerializer* self, PyObject* args, PyObject* keywords)
        -> int {
    static char keyword_output_stream[]{"output_stream"};
    static char keyword_buffer_size_limit[]{"buffer_size_limit"};
    static char keyword_user_defined_metadata[]{"user_defined_metadata"};
    static char* keyword_table[]{
            static_cast<char*>(keyword_output_stream),
            static_cast<char*>(keyword_buffer_size_limit),
            static_cast<char*>(keyword_user_defined_metadata),
            nullptr
    };

    // If the argument parsing fails, `self` will be deallocated. We must reset all pointers to
    // nullptr in advance, otherwise the deallocator might trigger segmentation fault.
    self->default_init();

    PyObject* output_stream{Py_None};
    PyObject* py_user_defined_metadata{Py_None};
    Py_ssize_t buffer_size_limit{PySerializer::cDefaultBufferSizeLimit};
    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "O|nO",
                static_cast<char**>(keyword_table),
                &output_stream,
                &buffer_size_limit,
                &py_user_defined_metadata
        )))
    {
        return -1;
    }

    // Ensure the `output_stream` has `write`, `flush`, and `close` methods
    auto output_stream_has_method = [&](char const* method_name) -> bool {
        PyObjectPtr<PyObject> const method{PyObject_GetAttrString(output_stream, method_name)};
        if (nullptr == method) {
            return false;
        }
        if (false == static_cast<bool>(PyCallable_Check(method.get()))) {
            PyErr_SetString(PyExc_TypeError, "");
            PyErr_Format(
                    PyExc_TypeError,
                    "The attribute `%s` of the given output stream object is not callable.",
                    method_name
            );
            return false;
        }
        return true;
    };

    if (false == output_stream_has_method("write")) {
        return -1;
    }
    if (false == output_stream_has_method("flush")) {
        return -1;
    }
    if (false == output_stream_has_method("close")) {
        return -1;
    }

    if (0 > buffer_size_limit) {
        PyErr_SetString(PyExc_ValueError, "The buffer size limit cannot be negative");
        return -1;
    }

    std::optional<nlohmann::json> optional_user_defined_metadata;
    if (Py_None != py_user_defined_metadata) {
        if (false == static_cast<bool>(PyDict_Check(py_user_defined_metadata))) {
            PyErr_Format(
                    PyExc_TypeError,
                    "`%s` must be a dictionary, if not None.",
                    static_cast<char const*>(keyword_user_defined_metadata)
            );
            return -1;
        }
        auto* py_serialized_json_str{py_utils_serialize_dict_to_json_str(
                py_reinterpret_cast<PyDictObject>(py_user_defined_metadata)
        )};
        if (nullptr == py_serialized_json_str) {
            return -1;
        }
        Py_ssize_t json_str_size{};
        auto const* json_str_data{PyUnicode_AsUTF8AndSize(
                py_reinterpret_cast<PyObject>(py_serialized_json_str),
                &json_str_size
        )};
        if (nullptr == json_str_data) {
            return -1;
        }
        auto parsed_user_defined_metadata = nlohmann::json::parse(
                std::string_view{json_str_data, static_cast<size_t>(json_str_size)},
                nullptr,
                false
        );
        if (parsed_user_defined_metadata.is_discarded()) {
            PyErr_Format(
                    PyExc_RuntimeError,
                    "Failed to parse `%s`: %s",
                    static_cast<char const*>(keyword_user_defined_metadata),
                    json_str_data
            );
            return -1;
        }
        optional_user_defined_metadata = std::move(parsed_user_defined_metadata);
    }

    auto serializer_result{PySerializer::ClpIrSerializer::create(optional_user_defined_metadata)};
    if (serializer_result.has_error()) {
        PyErr_Format(
                PyExc_RuntimeError,
                get_c_str_from_constexpr_string_view(cSerializerCreateErrorFormatStr),
                serializer_result.error().message().c_str()
        );
        return -1;
    }

    if (false == self->init(output_stream, std::move(serializer_result.value()), buffer_size_limit))
    {
        return -1;
    }

    return 0;
}

CLP_FFI_PY_METHOD auto PySerializer_serialize_log_event_from_msgpack_map(
        PySerializer* self,
        PyObject* args,
        PyObject* keywords
) -> PyObject* {
    static char keyword_auto_gen_msgpack_map[]{"auto_gen_msgpack_map"};
    static char keyword_user_gen_msgpack_map[]{"user_gen_msgpack_map"};
    static char* keyword_table[]{
            static_cast<char*>(keyword_auto_gen_msgpack_map),
            static_cast<char*>(keyword_user_gen_msgpack_map),
            nullptr
    };

    char const* auto_gen_msgpack_map{};
    Py_ssize_t auto_gen_msgpack_map_size{};
    char const* user_gen_msgpack_map{};
    Py_ssize_t user_gen_msgpack_map_size{};
    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "y#y#",
                static_cast<char**>(keyword_table),
                &auto_gen_msgpack_map,
                &auto_gen_msgpack_map_size,
                &user_gen_msgpack_map,
                &user_gen_msgpack_map_size
        )))
    {
        return nullptr;
    }

    auto const num_byte_serialized{self->serialize_log_event_from_msgpack_map(
            {auto_gen_msgpack_map, static_cast<size_t>(auto_gen_msgpack_map_size)},
            {user_gen_msgpack_map, static_cast<size_t>(user_gen_msgpack_map_size)}
    )};
    if (false == num_byte_serialized.has_value()) {
        return nullptr;
    }

    return PyLong_FromSsize_t(num_byte_serialized.value());
}

CLP_FFI_PY_METHOD auto PySerializer_get_num_bytes_serialized(PySerializer* self) -> PyObject* {
    return PyLong_FromSsize_t(self->get_num_bytes_serialized());
}

CLP_FFI_PY_METHOD auto PySerializer_flush(PySerializer* self) -> PyObject* {
    if (false == self->flush()) {
        return nullptr;
    }
    Py_RETURN_NONE;
}

CLP_FFI_PY_METHOD auto PySerializer_close(PySerializer* self) -> PyObject* {
    if (false == self->close()) {
        return nullptr;
    }
    Py_RETURN_NONE;
}

CLP_FFI_PY_METHOD auto PySerializer_enter(PySerializer* self) -> PyObject* {
    Py_INCREF(self);
    return py_reinterpret_cast<PyObject>(self);
}

CLP_FFI_PY_METHOD auto PySerializer_exit(PySerializer* self, PyObject* args, PyObject* keywords)
        -> PyObject* {
    static char keyword_exc_type[]{"exc_type"};
    static char keyword_exc_value[]{"exc_value"};
    static char keyword_traceback[]{"traceback"};
    static char* keyword_table[]{
            static_cast<char*>(keyword_exc_type),
            static_cast<char*>(keyword_exc_value),
            static_cast<char*>(keyword_traceback),
            nullptr
    };

    PyObject* py_exc_type{};
    PyObject* py_exc_value{};
    PyObject* py_traceback{};
    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "|OOO",
                static_cast<char**>(keyword_table),
                &py_exc_type,
                &py_exc_value,
                &py_traceback
        )))
    {
        return nullptr;
    }

    // We don't do anything with the given exception. It is the caller's responsibility to raise
    // the exceptions: https://docs.python.org/3/reference/datamodel.html#object.__exit__
    if (false == self->close()) {
        return nullptr;
    }

    Py_RETURN_NONE;
}

CLP_FFI_PY_METHOD auto PySerializer_dealloc(PySerializer* self) -> void {
    PyErrGuard const err_guard;

    if (false == self->is_closed()) {
        if (0
            != PyErr_WarnEx(
                    PyExc_ResourceWarning,
                    "`Serializer.close()` is not called before object destruction, which will leave"
                    " the stream incomplete, and potentially resulting in data"
                    " loss due to data buffering",
                    1
            ))
        {
            PyErr_Clear();
        }
    }

    self->clean();
    Py_TYPE(self)->tp_free(py_reinterpret_cast<PyObject>(self));
}
}  // namespace

auto PySerializer::module_level_init(PyObject* py_module) -> bool {
    static_assert(std::is_trivially_destructible<PySerializer>());
    auto* type{py_reinterpret_cast<PyTypeObject>(PyType_FromSpec(&PySerializer_type_spec))};
    m_py_type.reset(type);
    if (nullptr == type) {
        return false;
    }
    return add_python_type(get_py_type(), "Serializer", py_module);
}

auto PySerializer::init(
        PyObject* output_stream,
        PySerializer::ClpIrSerializer serializer,
        Py_ssize_t buffer_size_limit
) -> bool {
    m_output_stream = output_stream;
    Py_INCREF(output_stream);
    m_buffer_size_limit = buffer_size_limit;
    m_serializer = new (std::nothrow) PySerializer::ClpIrSerializer{std::move(serializer)};
    if (nullptr == m_serializer) {
        PyErr_SetString(
                PyExc_RuntimeError,
                get_c_str_from_constexpr_string_view(clp_ffi_py::cOutOfMemoryError)
        );
        return false;
    }
    auto const preamble_size{get_ir_buf_size()};
    if (preamble_size > m_buffer_size_limit && false == write_ir_buf_to_output_stream()) {
        return false;
    }
    m_num_total_bytes_serialized += preamble_size;
    return true;
}

auto PySerializer::assert_is_not_closed() const -> bool {
    if (is_closed()) {
        PyErr_SetString(PyExc_IOError, "Serializer has already been closed.");
        return false;
    }
    return true;
}

auto PySerializer::serialize_log_event_from_msgpack_map(
        std::span<char const> auto_gen_msgpack_map,
        std::span<char const> user_gen_msgpack_map
) -> std::optional<Py_ssize_t> {
    if (false == assert_is_not_closed()) {
        return std::nullopt;
    }

    auto const optional_auto_gen_msgpack_map_handle{unpack_msgpack_map(auto_gen_msgpack_map)};
    if (false == optional_auto_gen_msgpack_map_handle.has_value()) {
        return std::nullopt;
    }

    auto const optional_user_gen_msgpack_map_handle{unpack_msgpack_map(user_gen_msgpack_map)};
    if (false == optional_user_gen_msgpack_map_handle.has_value()) {
        return std::nullopt;
    }

    auto const buffer_size_before_serialization{get_ir_buf_size()};
    // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
    if (false
        == m_serializer->serialize_msgpack_map(
                optional_auto_gen_msgpack_map_handle.value().get().via.map,
                optional_user_gen_msgpack_map_handle.value().get().via.map
        ))
    // NOLINTEND(cppcoreguidelines-pro-type-union-access)
    {
        PyErr_SetString(
                PyExc_RuntimeError,
                get_c_str_from_constexpr_string_view(cSerializerSerializeMsgpackMapError)
        );
        return std::nullopt;
    }
    auto const buffer_size_after_serialization{get_ir_buf_size()};
    auto const num_bytes_serialized{
            buffer_size_after_serialization - buffer_size_before_serialization
    };
    m_num_total_bytes_serialized += num_bytes_serialized;

    if (buffer_size_after_serialization > m_buffer_size_limit
        && false == write_ir_buf_to_output_stream())
    {
        return std::nullopt;
    }
    return num_bytes_serialized;
}

auto PySerializer::flush() -> bool {
    if (false == assert_is_not_closed()) {
        return false;
    }
    if (false == write_ir_buf_to_output_stream()) {
        return false;
    }
    return flush_output_stream();
}

auto PySerializer::close() -> bool {
    if (false == assert_is_not_closed()) {
        return false;
    }

    if (false == write_ir_buf_to_output_stream()) {
        return false;
    }

    // Write end-of-stream
    constexpr std::array<int8_t, 1> cEndOfStreamBuf{clp::ffi::ir_stream::cProtocol::Eof};
    if (false == write_to_output_stream({cEndOfStreamBuf.cbegin(), cEndOfStreamBuf.cend()})) {
        return false;
    }
    m_num_total_bytes_serialized += cEndOfStreamBuf.size();

    if (false == (flush_output_stream() && close_output_stream())) {
        return false;
    }

    close_serializer();
    return true;
}

auto PySerializer::write_ir_buf_to_output_stream() -> bool {
    if (false == assert_is_not_closed()) {
        return false;
    }

    auto const optional_num_bytes_written{write_to_output_stream(m_serializer->get_ir_buf_view())};
    if (false == optional_num_bytes_written.has_value()) {
        return false;
    }
    if (optional_num_bytes_written.value() != get_ir_buf_size()) {
        PyErr_SetString(
                PyExc_RuntimeError,
                "The number of bytes written to the output stream doesn't match the size of the "
                "internal buffer"
        );
        return false;
    }

    m_serializer->clear_ir_buf();
    return true;
}

auto PySerializer::write_to_output_stream(PySerializer::BufferView buf)
        -> std::optional<Py_ssize_t> {
    if (buf.empty()) {
        return 0;
    }

    // `PyBUF_READ` ensures the buffer is read-only, so it should be safe to cast `char const*` to
    // `char*`
    PyObjectPtr<PyObject> const ir_buf_mem_view{PyMemoryView_FromMemory(
            // NOLINTNEXTLINE(bugprone-casting-through-void, cppcoreguidelines-pro-type-*-cast)
            static_cast<char*>(const_cast<void*>(static_cast<void const*>(buf.data()))),
            static_cast<Py_ssize_t>(buf.size()),
            PyBUF_READ
    )};
    if (nullptr == ir_buf_mem_view) {
        return std::nullopt;
    }

    PyObjectPtr<PyObject> const py_num_bytes_written{
            PyObject_CallMethod(m_output_stream, "write", "O", ir_buf_mem_view.get())
    };
    if (nullptr == py_num_bytes_written) {
        return std::nullopt;
    }

    Py_ssize_t num_bytes_written{};
    if (false == parse_py_int(py_num_bytes_written.get(), num_bytes_written)) {
        return std::nullopt;
    }
    return num_bytes_written;
}

auto PySerializer::flush_output_stream() -> bool {
    PyObjectPtr<PyObject> const ret_val{PyObject_CallMethod(m_output_stream, "flush", "")};
    if (nullptr == ret_val) {
        return false;
    }
    return true;
}

auto PySerializer::close_output_stream() -> bool {
    PyObjectPtr<PyObject> const ret_val{PyObject_CallMethod(m_output_stream, "close", "")};
    if (nullptr == ret_val) {
        return false;
    }
    return true;
}
}  // namespace clp_ffi_py::ir::native
