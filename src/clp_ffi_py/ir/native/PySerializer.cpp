#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include "PySerializer.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>

#include <clp/ffi/ir_stream/protocol_constants.hpp>

#include <clp_ffi_py/api_decoration.hpp>
#include <clp_ffi_py/error_messages.hpp>
#include <clp_ffi_py/ir/native/error_messages.hpp>
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
        "Serializer for CLP key-value pair IR stream.\n"
        "This class serializes log events into CLP IR format (using four-byte-encoding) and writes"
        " the serialized data to a specified byte stream.\n\n"
        "__init__(self, output_stream)\n\n"
        "Initializes a `Serializer` instance with the given output stream. Notice that each object"
        " should be strictly initialized only once. Double initialization will result in memory"
        " leak.\n\n"
        ":param output_stream: A writable byte stream to which the serializer will write serialized"
        " IR byte sequence.\n"
);
CLP_FFI_PY_METHOD auto
PySerializer_init(PySerializer* self, PyObject* args, PyObject* keywords) -> int;

/**
 * Callback of `PySerializer`'s `serialize_msgpack` method.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPySerializerSerializeMsgpackDoc,
        "serialize_msgpack(self, msgpack_byte_sequence)\n"
        "--\n\n"
        "Serializes the given msgpack byte sequence into as a log event.\n"
        "NOTE: the serialization results will be buffered inside the serializer. `write_to_stream`"
        " must be called to write buffered bytes into the output stream.\n\n"
        ":param msgpack_byte_sequence: A byte sequence encoded in msgpack as the input log event."
        " The unpacked result must be a msgpack map with all keys as strings.\n"
        ":raise IOError: If the serializer has already been closed.\n"
        ":raise TypeError: If the unpacked result is not a msgpack map.\n"
        ":raise RuntimeError: If it fails to unpack the given msgpack byte sequence, or the"
        " serialization method returns failure.\n"
);
CLP_FFI_PY_METHOD auto
PySerializer_serialize_msgpack(PySerializer* self, PyObject* msgpack_byte_sequence) -> PyObject*;

/**
 * Callback of `PySerializer`'s `write_to_stream` method.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPySerializerWriteToStreamDoc,
        "write_to_stream(self)\n"
        "--\n\n"
        "Writes the buffered results to the output stream and clears the buffer.\n\n"
        ":return: Number of bytes written.\n"
        ":raise IOError: If the serializer has already been closed.\n"
);
CLP_FFI_PY_METHOD auto PySerializer_write_to_stream(PySerializer* self) -> PyObject*;

/**
 * Callback of `PySerializer`'s `get_buffer_size` method.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPySerializerGetBufferSizeDoc,
        "get_buffer_size(self)\n"
        "--\n\n"
        "Gets the size of the result buffer.\n\n"
        ":return: The buffer size in bytes.\n"
        ":raise IOError: If the serializer has already been closed.\n"
);
CLP_FFI_PY_METHOD auto PySerializer_get_buffer_size(PySerializer* self) -> PyObject*;

/**
 * Callback of `PySerializer`'s `flush_stream` method.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPySerializerFlushStreamDoc,
        "flush_stream(self)\n"
        "--\n\n"
        "Flushes the output stream.\n\n"
        ":raise IOError: If the serializer has already been closed.\n"
);
CLP_FFI_PY_METHOD auto PySerializer_flush_stream(PySerializer* self) -> PyObject*;

/**
 * Callback of `PySerializer`'s `close` method.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPySerializerCloseDoc,
        "close(self, flush_stream=True)\n"
        "--\n\n"
        "Closes the serializer by writing the results into the output stream with the byte sequence"
        " in the end that indicates the end of a CLP IR stream. This method must be called to"
        " terminate an IR stream. Otherwise, the stream will be considered incomplete.\n\n"
        ":param flush_stream: Whether to flush the output stream.\n"
        ":return: Forwards :meth:`write_to_stream`'s return value.\n"
        ":raise IOError: If the serializer has already been closed.\n"
);
CLP_FFI_PY_METHOD auto
PySerializer_close(PySerializer* self, PyObject* args, PyObject* keywords) -> PyObject*;

/**
 * Callback of `PySerializer`'s deallocator.
 */
CLP_FFI_PY_METHOD auto PySerializer_dealloc(PySerializer* self) -> void;

/**
 * Callback of `PySerializer`'s finalization (destructor).
 */
CLP_FFI_PY_METHOD auto PySerializer_finalize(PySerializer* self) -> void;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyMethodDef PySerializer_method_table[]{
        {"serialize_msgpack",
         py_c_function_cast(PySerializer_serialize_msgpack),
         METH_O,
         static_cast<char const*>(cPySerializerSerializeMsgpackDoc)},

        {"write_to_stream",
         py_c_function_cast(PySerializer_write_to_stream),
         METH_NOARGS,
         static_cast<char const*>(cPySerializerWriteToStreamDoc)},

        {"get_buffer_size",
         py_c_function_cast(PySerializer_get_buffer_size),
         METH_NOARGS,
         static_cast<char const*>(cPySerializerGetBufferSizeDoc)},

        {"flush_stream",
         py_c_function_cast(PySerializer_flush_stream),
         METH_NOARGS,
         static_cast<char const*>(cPySerializerFlushStreamDoc)},

        {"close",
         py_c_function_cast(PySerializer_close),
         METH_VARARGS | METH_KEYWORDS,
         static_cast<char const*>(cPySerializerCloseDoc)},

        {nullptr}
};

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-*-cast)
PyType_Slot PySerializer_slots[]{
        {Py_tp_alloc, reinterpret_cast<void*>(PyType_GenericAlloc)},
        {Py_tp_dealloc, reinterpret_cast<void*>(PySerializer_dealloc)},
        {Py_tp_new, reinterpret_cast<void*>(PyType_GenericNew)},
        {Py_tp_init, reinterpret_cast<void*>(PySerializer_init)},
        {Py_tp_finalize, reinterpret_cast<void*>(PySerializer_finalize)},
        {Py_tp_methods, static_cast<void*>(PySerializer_method_table)},
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPySerializerDoc))},
        {0, nullptr}
};
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-*-cast)

/**
 * `PySerializer`'s Python type specifications.
 */
PyType_Spec PySerializer_type_spec{
        "clp_ffi_py.ir.native.Serializer",
        sizeof(PySerializer),
        0,
        Py_TPFLAGS_DEFAULT,
        static_cast<PyType_Slot*>(PySerializer_slots)
};

CLP_FFI_PY_METHOD auto
PySerializer_init(PySerializer* self, PyObject* args, PyObject* keywords) -> int {
    static char keyword_output_stream[]{"output_stream"};
    static char* keyword_table[]{static_cast<char*>(keyword_output_stream), nullptr};

    // If the argument parsing fails, `self` will be deallocated. We must reset all pointers to
    // nullptr in advance, otherwise the deallocator might trigger segmentation fault.
    self->default_init();

    PyObject* output_stream{Py_None};
    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "O",
                static_cast<char**>(keyword_table),
                &output_stream
        )))
    {
        return -1;
    }

    // Ensure the `output_stream` has `write` and `flush` methods
    PyObjectPtr<PyObject> const write_method{PyObject_GetAttrString(output_stream, "write")};
    if (nullptr == write_method) {
        return -1;
    }
    if (false == static_cast<bool>(PyCallable_Check(write_method.get()))) {
        PyErr_SetString(
                PyExc_TypeError,
                "The attribute `write` of the given output stream object is not callable."
        );
        return -1;
    }

    PyObjectPtr<PyObject> const flush_method{PyObject_GetAttrString(output_stream, "flush")};
    if (nullptr == flush_method) {
        return -1;
    }
    if (false == static_cast<bool>(PyCallable_Check(flush_method.get()))) {
        PyErr_SetString(
                PyExc_TypeError,
                "The attribute `flush` of the given output stream object is not callable."
        );
        return -1;
    }

    auto serializer_result{PySerializer::ClpIrSerializer::create()};
    if (serializer_result.has_error()) {
        PyErr_Format(
                PyExc_RuntimeError,
                cSerializerCreateErrorFormatStr.data(),
                serializer_result.error().message().c_str()
        );
        return -1;
    }

    return self->init(output_stream, std::move(serializer_result.value())) ? 0 : -1;
}

CLP_FFI_PY_METHOD auto
PySerializer_serialize_msgpack(PySerializer* self, PyObject* msgpack_byte_sequence) -> PyObject* {
    if (false == static_cast<bool>(PyBytes_Check(msgpack_byte_sequence))) {
        PyErr_SetString(
                PyExc_TypeError,
                "`msgpack_byte_sequence` is supposed to return a `bytes` object"
        );
        return nullptr;
    }

    auto* py_bytes_msgpack_byte_sequence{py_reinterpret_cast<PyBytesObject>(msgpack_byte_sequence)};
    // Since the type is already checked, we can use the macro to avoid duplicated type checking.
    if (false
        == self->serialize_msgpack_map(
                {PyBytes_AS_STRING(py_bytes_msgpack_byte_sequence),
                 static_cast<size_t>(PyBytes_GET_SIZE(py_bytes_msgpack_byte_sequence))}
        ))
    {
        return nullptr;
    }

    Py_RETURN_NONE;
}

CLP_FFI_PY_METHOD auto PySerializer_write_to_stream(PySerializer* self) -> PyObject* {
    auto const optional_num_bytes_write{self->write_ir_buf_to_output_stream()};
    if (false == optional_num_bytes_write.has_value()) {
        return nullptr;
    }
    return PyLong_FromSsize_t(optional_num_bytes_write.value());
}

CLP_FFI_PY_METHOD auto PySerializer_get_buffer_size(PySerializer* self) -> PyObject* {
    if (false == self->assert_is_not_closed()) {
        return nullptr;
    }
    return PyLong_FromSsize_t(self->get_ir_buf_size());
}

CLP_FFI_PY_METHOD auto PySerializer_flush_stream(PySerializer* self) -> PyObject* {
    if (false == self->flush_output_stream()) {
        return nullptr;
    }
    Py_RETURN_NONE;
}

CLP_FFI_PY_METHOD auto
PySerializer_close(PySerializer* self, PyObject* args, PyObject* keywords) -> PyObject* {
    static char keyword_flush_stream[]{"flush_stream"};
    static char* keyword_table[]{static_cast<char*>(keyword_flush_stream), nullptr};

    int flush_stream{1};

    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "|p",
                static_cast<char**>(keyword_table),
                &flush_stream
        )))
    {
        return nullptr;
    }

    auto const optional_num_bytes_written{self->close(static_cast<bool>(flush_stream))};
    if (false == optional_num_bytes_written.has_value()) {
        return nullptr;
    }

    return PyLong_FromSsize_t(optional_num_bytes_written.value());
}

CLP_FFI_PY_METHOD auto PySerializer_dealloc(PySerializer* self) -> void {
    self->clean();
}

CLP_FFI_PY_METHOD auto PySerializer_finalize(PySerializer* self) -> void {
    PyErrGuard const err_guard;
    if (self->is_closed()) {
        return;
    }

    if (0
        != PyErr_WarnEx(
                PyExc_RuntimeWarning,
                "`Serializer.close()` is not called before object destruction",
                1
        ))
    {
        PyErr_Clear();
    }
}
}  // namespace

auto PySerializer::init(PyObject* output_stream, PySerializer::ClpIrSerializer serializer) -> bool {
    m_output_stream = output_stream;
    Py_INCREF(output_stream);
    m_serializer = new PySerializer::ClpIrSerializer{std::move(serializer)};
    if (nullptr == m_serializer) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::cOutofMemoryError);
        return false;
    }
    return true;
}

auto PySerializer::assert_is_not_closed() const -> bool {
    if (is_closed()) {
        PyErr_SetString(PyExc_IOError, "Serializer has already been closed.");
        return false;
    }
    return true;
}

auto PySerializer::serialize_msgpack_map(std::span<char const> msgpack_byte_sequence) -> bool {
    if (false == assert_is_not_closed()) {
        return false;
    }

    auto const unpack_result{unpack_msgpack(msgpack_byte_sequence)};
    if (unpack_result.has_error()) {
        PyErr_SetString(PyExc_RuntimeError, unpack_result.error().c_str());
        return false;
    }

    auto const& msgpack_obj{unpack_result.value().get()};
    if (msgpack::type::MAP != msgpack_obj.type) {
        PyErr_SetString(PyExc_TypeError, "Unpacked msgpack is not a map");
        return false;
    }

    if (false == m_serializer->serialize_msgpack_map(msgpack_obj.via.map)) {
        PyErr_SetString(PyExc_RuntimeError, cSerializerSerializeMsgpackMapError.data());
        return false;
    }

    return true;
}

auto PySerializer::write_ir_buf_to_output_stream() -> std::optional<Py_ssize_t> {
    if (false == assert_is_not_closed()) {
        return std::nullopt;
    }

    auto const optional_num_bytes_written{write_to_output_stream(m_serializer->get_ir_buf_view())};
    if (false == optional_num_bytes_written.has_value()) {
        return std::nullopt;
    }

    m_serializer->clear_ir_buf();
    return optional_num_bytes_written;
}

auto PySerializer::flush_output_stream() -> bool {
    if (false == assert_is_not_closed()) {
        return false;
    }
    if (nullptr == PyObject_CallMethod(m_output_stream, "flush", "")) {
        return false;
    }
    return true;
}

auto PySerializer::close(bool flush_stream) -> std::optional<Py_ssize_t> {
    if (false == assert_is_not_closed()) {
        return std::nullopt;
    }

    // Write the IR buffer into the stream.
    auto const optional_num_bytes_written_from_ir_buf{write_ir_buf_to_output_stream()};
    if (false == optional_num_bytes_written_from_ir_buf.has_value()) {
        return std::nullopt;
    }

    // Write end-of-stream
    constexpr std::array<int8_t, 1> cEndOfStreamBuf{clp::ffi::ir_stream::cProtocol::Eof};
    auto const optional_num_bytes_written_from_end_of_stream_buf{
            write_to_output_stream({cEndOfStreamBuf.cbegin(), cEndOfStreamBuf.cend()})
    };
    if (false == optional_num_bytes_written_from_end_of_stream_buf.has_value()) {
        return std::nullopt;
    }

    // Flush the output stream if needed
    if (flush_stream && false == flush_output_stream()) {
        return std::nullopt;
    }

    close_serializer();
    return optional_num_bytes_written_from_ir_buf.value()
           + optional_num_bytes_written_from_end_of_stream_buf.value();
}

auto PySerializer::module_level_init(PyObject* py_module) -> bool {
    static_assert(std::is_trivially_destructible<PySerializer>());
    auto* type{py_reinterpret_cast<PyTypeObject>(PyType_FromSpec(&PySerializer_type_spec))};
    m_py_type.reset(type);
    if (nullptr == type) {
        return false;
    }
    return add_python_type(get_py_type(), "Serializer", py_module);
}

auto PySerializer::write_to_output_stream(PySerializer::BufferView buf
) -> std::optional<Py_ssize_t> {
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

    PyObject* py_num_bytes_written{
            PyObject_CallMethod(m_output_stream, "write", "O", ir_buf_mem_view.get())
    };
    if (nullptr == py_num_bytes_written) {
        return std::nullopt;
    }

    Py_ssize_t num_bytes_written{};
    if (false == parse_py_int(py_num_bytes_written, num_bytes_written)) {
        return std::nullopt;
    }
    return num_bytes_written;
}
}  // namespace clp_ffi_py::ir::native
