#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include "PyDeserializer.hpp"

#include <system_error>
#include <type_traits>
#include <utility>

#include <clp/ffi/ir_stream/decoding_methods.hpp>
#include <clp/ffi/ir_stream/Deserializer.hpp>
#include <clp/ffi/ir_stream/IrUnitType.hpp>
#include <clp/ffi/KeyValuePairLogEvent.hpp>
#include <clp/ffi/SchemaTree.hpp>
#include <clp/time_types.hpp>
#include <clp/TraceableException.hpp>

#include <clp_ffi_py/api_decoration.hpp>
#include <clp_ffi_py/ir/native/DeserializerBufferReader.hpp>
#include <clp_ffi_py/ir/native/error_messages.hpp>
#include <clp_ffi_py/ir/native/PyKeyValuePairLogEvent.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
using clp::ffi::ir_stream::IRErrorCode;
using clp::ffi::ir_stream::IrUnitType;

namespace {
/**
 * Callback of `PyDeserializer`'s `__init__` method:
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyDeserializerDoc,
        "Deserializer for deserializing CLP key-value pair IR streams.\n"
        "This class deserializes a CLP key-value pair IR stream into log events.\n\n"
        "__init__(self, input_stream, buffer_capacity=65536, allow_incomplete_stream=False)\n\n"
        "Initializes a :class:`Deserializer` instance with the given inputs. Note that each"
        " object should only be initialized once. Double initialization will result in a memory"
        " leak.\n\n"
        ":param input_stream: Serialized CLP IR stream.\n"
        ":type input_stream: IO[bytes]\n"
        ":param buffer_capacity: The capacity of the underlying read buffer.\n"
        ":type buffer_capacity: int\n"
        ":param allow_incomplete_stream: If set to `True`, an incomplete CLP IR stream is not"
        " treated as an error.\n"
        ":type allow_incomplete_stream: bool\n"
);
CLP_FFI_PY_METHOD auto
PyDeserializer_init(PyDeserializer* self, PyObject* args, PyObject* keywords) -> int;

/**
 * Callback of `PyDeserializer`'s `deserialize_log_event`.
 */
PyDoc_STRVAR(
        cPyDeserializerDeserializeLogEventDoc,
        "deserialize_log_event(self)\n"
        "--\n\n"
        "Deserializes the next log event from the IR stream.\n\n"
        ":return:\n"
        "     - The next deserialized log event from the IR stream.\n"
        "     - None if there are no more log events in the stream.\n"
        ":rtype: :class:`KeyValuePairLogEvent` | None\n"
        ":raises: Appropriate exceptions with detailed information on any encountered failure.\n"
);
CLP_FFI_PY_METHOD auto PyDeserializer_deserialize_log_event(PyDeserializer* self) -> PyObject*;

/**
 * Callback of `PyDeserializer`'s deallocator.
 */
CLP_FFI_PY_METHOD auto PyDeserializer_dealloc(PyDeserializer* self) -> void;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyMethodDef PyDeserializer_method_table[]{
        {"deserialize_log_event",
         py_c_function_cast(PyDeserializer_deserialize_log_event),
         METH_NOARGS,
         static_cast<char const*>(cPyDeserializerDeserializeLogEventDoc)},

        {nullptr}
};

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-*-cast)
PyType_Slot PyDeserializer_slots[]{
        {Py_tp_alloc, reinterpret_cast<void*>(PyType_GenericAlloc)},
        {Py_tp_dealloc, reinterpret_cast<void*>(PyDeserializer_dealloc)},
        {Py_tp_new, reinterpret_cast<void*>(PyType_GenericNew)},
        {Py_tp_init, reinterpret_cast<void*>(PyDeserializer_init)},
        {Py_tp_methods, static_cast<void*>(PyDeserializer_method_table)},
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPyDeserializerDoc))},
        {0, nullptr}
};
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-*-cast)

/**
 * `PyDeserializer`'s Python type specifications.
 */
PyType_Spec PyDeserializer_type_spec{
        "clp_ffi_py.ir.native.Deserializer",
        sizeof(PyDeserializer),
        0,
        Py_TPFLAGS_DEFAULT,
        static_cast<PyType_Slot*>(PyDeserializer_slots)
};

CLP_FFI_PY_METHOD auto
PyDeserializer_init(PyDeserializer* self, PyObject* args, PyObject* keywords) -> int {
    static char keyword_input_stream[]{"input_stream"};
    static char keyword_buffer_capacity[]{"buffer_capacity"};
    static char keyword_allow_incomplete_stream[]{"allow_incomplete_stream"};
    static char* keyword_table[]{
            static_cast<char*>(keyword_input_stream),
            static_cast<char*>(keyword_buffer_capacity),
            static_cast<char*>(keyword_allow_incomplete_stream),
            nullptr
    };

    // If the argument parsing fails, `self` will be deallocated. We must reset all pointers to
    // nullptr in advance, otherwise the deallocator might trigger segmentation fault.
    self->default_init();

    PyObject* input_stream{};
    Py_ssize_t buffer_capacity{PyDeserializer::cDefaultBufferCapacity};
    int allow_incomplete_stream{0};
    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "O|np",
                static_cast<char**>(keyword_table),
                &input_stream,
                &buffer_capacity,
                &allow_incomplete_stream
        )))
    {
        return -1;
    }

    if (false
        == self->init(input_stream, buffer_capacity, static_cast<bool>(allow_incomplete_stream)))
    {
        return -1;
    }

    return 0;
}

CLP_FFI_PY_METHOD auto PyDeserializer_deserialize_log_event(PyDeserializer* self) -> PyObject* {
    return self->deserialize_log_event();
}

CLP_FFI_PY_METHOD auto PyDeserializer_dealloc(PyDeserializer* self) -> void {
    self->clean();
    Py_TYPE(self)->tp_free(py_reinterpret_cast<PyObject>(self));
}
}  // namespace

auto PyDeserializer::init(
        PyObject* input_stream,
        Py_ssize_t buffer_capacity,
        bool allow_incomplete_stream
) -> bool {
    m_allow_incomplete_stream = allow_incomplete_stream;
    m_deserializer_buffer_reader = DeserializerBufferReader::create(input_stream, buffer_capacity);
    if (nullptr == m_deserializer_buffer_reader) {
        return false;
    }

    try {
        PyDeserializer::IrUnitHandler::LogEventHandle log_event_handle
                = [this](clp::ffi::KeyValuePairLogEvent&& kv_log_event) -> IRErrorCode {
            return this->handle_log_event(std::move(kv_log_event));
        };

        PyDeserializer::IrUnitHandler::UtcOffsetChangeHandle trivial_utc_offset_handle
                = []([[maybe_unused]] clp::UtcOffset, [[maybe_unused]] clp::UtcOffset
                  ) -> IRErrorCode { return IRErrorCode::IRErrorCode_Success; };

        PyDeserializer::IrUnitHandler::SchemaTreeNodeInsertionHandle
                trivial_schema_tree_node_insertion_handle
                = []([[maybe_unused]] clp::ffi::SchemaTree::NodeLocator) -> IRErrorCode {
            return IRErrorCode::IRErrorCode_Success;
        };

        PyDeserializer::IrUnitHandler::EndOfStreamHandle end_of_stream_handle
                = [this]() -> IRErrorCode { return this->handle_end_of_stream(); };

        auto deserializer_result{Deserializer::create(
                *m_deserializer_buffer_reader,
                {std::move(log_event_handle),
                 std::move(trivial_utc_offset_handle),
                 std::move(trivial_schema_tree_node_insertion_handle),
                 std::move(end_of_stream_handle)}
        )};
        if (deserializer_result.has_error()) {
            PyErr_Format(
                    PyExc_RuntimeError,
                    cDeserializerCreateErrorFormatStr.data(),
                    deserializer_result.error().message().c_str()
            );
            return false;
        }
        m_deserializer = new clp::ffi::ir_stream::Deserializer<PyDeserializer::IrUnitHandler>{
                std::move(deserializer_result.value())
        };
    } catch (clp::TraceableException& exception) {
        handle_traceable_exception(exception);
        return false;
    }

    return true;
}

auto PyDeserializer::deserialize_log_event() -> PyObject* {
    try {
        while (false == is_stream_completed()) {
            auto const ir_unit_type_result{
                    m_deserializer->deserialize_next_ir_unit(*m_deserializer_buffer_reader)
            };
            if (ir_unit_type_result.has_error()) {
                auto const err{ir_unit_type_result.error()};
                if (std::errc::result_out_of_range != err) {
                    PyErr_Format(
                            PyExc_RuntimeError,
                            cDeserializerDeserializeNextIrUnitErrorFormatStr.data(),
                            err.message().c_str()
                    );
                    return nullptr;
                }
                if (false == handle_incomplete_stream_error()) {
                    return nullptr;
                }
                break;
            }
            if (IrUnitType::LogEvent != ir_unit_type_result.value()) {
                continue;
            }
            if (false == has_unreleased_deserialized_log_event()) {
                // TODO: after native query is implemented, this branch could indicate the
                // deserialized log event IR unit doesn't match the query and thus not set as a
                // buffered deserialization result. But before that, this check is added to ensure
                // we check the ownership is valid before calling `release_deserialized_log_event`.
                PyErr_SetString(
                        PyExc_RuntimeError,
                        "Deserializer failed to set the underlying deserialized log event properly"
                        " after successfully deserializing a log event IR unit."
                );
                return nullptr;
            }
            return py_reinterpret_cast<PyObject>(
                    PyKeyValuePairLogEvent::create(release_deserialized_log_event())
            );
        }
    } catch (clp::TraceableException& exception) {
        handle_traceable_exception(exception);
        return nullptr;
    }

    Py_RETURN_NONE;
}

auto PyDeserializer::module_level_init(PyObject* py_module) -> bool {
    static_assert(std::is_trivially_destructible<PyDeserializer>());
    auto* type{py_reinterpret_cast<PyTypeObject>(PyType_FromSpec(&PyDeserializer_type_spec))};
    m_py_type.reset(type);
    if (nullptr == type) {
        return false;
    }
    return add_python_type(get_py_type(), "Deserializer", py_module);
}

auto PyDeserializer::handle_log_event(clp::ffi::KeyValuePairLogEvent&& log_event) -> IRErrorCode {
    if (has_unreleased_deserialized_log_event()) {
        // This situation may occur if the deserializer methods return an error after the last
        // successful call to `handle_log_event`. If the user resolves the error and invokes the
        // deserializer methods again, the underlying deserialized log event from the previous
        // failed calls remains unreleased.
        // To prevent a memory leak, we must free the associated memory by clearing the last
        // deserialized log event.
        clear_deserialized_log_event();
    }
    m_deserialized_log_event = new clp::ffi::KeyValuePairLogEvent{std::move(log_event)};
    return IRErrorCode::IRErrorCode_Success;
}

auto PyDeserializer::handle_incomplete_stream_error() -> bool {
    if (m_allow_incomplete_stream) {
        handle_end_of_stream();
        return true;
    }
    PyErr_SetString(
            PyDeserializerBuffer::get_py_incomplete_stream_error(),
            cDeserializerIncompleteIRError
    );
    return false;
}
}  // namespace clp_ffi_py::ir::native
