#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include "PyKeyValuePairLogEvent.hpp"

#include <cstddef>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>

#include <clp/BufferReader.hpp>
#include <clp/ffi/ir_stream/decoding_methods.hpp>
#include <clp/ffi/ir_stream/Deserializer.hpp>
#include <clp/ffi/ir_stream/IrUnitType.hpp>
#include <clp/ffi/ir_stream/Serializer.hpp>
#include <clp/ffi/KeyValuePairLogEvent.hpp>
#include <clp/ffi/SchemaTree.hpp>
#include <clp/ir/types.hpp>
#include <clp/time_types.hpp>
#include <clp/type_utils.hpp>

#include <clp_ffi_py/api_decoration.hpp>
#include <clp_ffi_py/error_messages.hpp>
#include <clp_ffi_py/ir/native/error_messages.hpp>
#include <clp_ffi_py/Py_utils.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>
#include <clp_ffi_py/utils.hpp>

using clp::ffi::ir_stream::IRErrorCode;

namespace clp_ffi_py::ir::native {
namespace {
/**
 * Class that implements `clp::ffi::ir_stream::IrUnitHandlerInterface` for deserializing log events.
 */
class IrUnitHandler {
public:
    // Methods that implement the `clp::ffi::ir_stream::IrUnitHandlerInterface` interface
    [[nodiscard]] auto handle_log_event(clp::ffi::KeyValuePairLogEvent&& deserialized_log_event
    ) -> IRErrorCode {
        log_event.emplace(std::move(deserialized_log_event));
        return IRErrorCode::IRErrorCode_Success;
    }

    [[nodiscard]] static auto handle_utc_offset_change(
            [[maybe_unused]] clp::UtcOffset utc_offset_old,
            [[maybe_unused]] clp::UtcOffset utc_offset_new
    ) -> IRErrorCode {
        return IRErrorCode::IRErrorCode_Success;
    }

    [[nodiscard]] static auto handle_schema_tree_node_insertion(
            [[maybe_unused]] clp::ffi::SchemaTree::NodeLocator schema_tree_node_locator
    ) -> IRErrorCode {
        return IRErrorCode::IRErrorCode_Success;
    }

    [[nodiscard]] static auto handle_end_of_stream() -> IRErrorCode {
        return IRErrorCode::IRErrorCode_Success;
    }

    // TODO: we should enable linting when clang-tidy config is up-to-date to allow simple classes.
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes,readability-identifier-naming)
    std::optional<clp::ffi::KeyValuePairLogEvent> log_event;
};

/**
 * Callback of `PyKeyValuePairLogEvent`'s `__init__` method.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyKeyValuePairLogEventDoc,
        "This class represents a key-value pair log event and provides methods to access the"
        " key-value pairs. This class is designed to be instantiated by the IR deserializer."
        " However, direct instantiation using the `__init__` method is also supported for testing"
        " purposes, although this may not be as efficient as emission from the IR deserializer.\n\n"
        "__init__(self, dictionary)\n\n"
        "Initializes a :class:`KeyValuePairLogEvent` from the given Python dictionary. Note that"
        " each object should only be initialized once. Double initialization will result in a"
        " memory leak.\n\n"
        ":param dictionary: A dictionary representing the key-value pair log event, where all keys"
        " must be strings, including keys inside any sub-dictionaries.\n"
        ":type dictionary: dict[str, Any]\n"
        ":rtype: int\n"
);
CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_init(
        PyKeyValuePairLogEvent* self,
        PyObject* args,
        PyObject* keywords
) -> int;

/**
 * Callback of `PyKeyValuePairLogEvent`'s `to_dict` method.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyKeyValuePairLogEventToDictDoc,
        "to_dict(self)\n"
        "--\n\n"
        "Converts the underlying key-value pair log event into a Python dictionary.\n\n"
        ":return: Serialized log event in a Python dictionary.\n"
        ":rtype: dict[str, Any]"
);
CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_to_dict(PyKeyValuePairLogEvent* self) -> PyObject*;

/**
 * Callback of `PyKeyValuePairLogEvent`'s deallocator.
 */
CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_dealloc(PyKeyValuePairLogEvent* self) -> void;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyMethodDef PyKeyValuePairLogEvent_method_table[]{
        {"to_dict",
         py_c_function_cast(PyKeyValuePairLogEvent_to_dict),
         METH_NOARGS,
         static_cast<char const*>(cPyKeyValuePairLogEventToDictDoc)},

        {nullptr}
};

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-*-cast)
PyType_Slot PyKeyValuePairLogEvent_slots[]{
        {Py_tp_alloc, reinterpret_cast<void*>(PyType_GenericAlloc)},
        {Py_tp_dealloc, reinterpret_cast<void*>(PyKeyValuePairLogEvent_dealloc)},
        {Py_tp_new, reinterpret_cast<void*>(PyType_GenericNew)},
        {Py_tp_init, reinterpret_cast<void*>(PyKeyValuePairLogEvent_init)},
        {Py_tp_methods, static_cast<void*>(PyKeyValuePairLogEvent_method_table)},
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPyKeyValuePairLogEventDoc))},
        {0, nullptr}
};
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-*-cast)

/**
 * `PyKeyValuePairLogEvent`'s Python type specifications.
 */
PyType_Spec PyKeyValuePairLogEvent_type_spec{
        "clp_ffi_py.ir.native.KeyValuePairLogEvent",
        sizeof(PyKeyValuePairLogEvent),
        0,
        Py_TPFLAGS_DEFAULT,
        static_cast<PyType_Slot*>(PyKeyValuePairLogEvent_slots)
};

/**
 * Converts the given Python dictionary to `clp::ffi::KeyValuePairLogEvent`.
 * NOTE: Currently, there is no direct utility for constructing a `clp::ffi::KeyValuePairLogEvent`
 * from a dictionary-like structure. As a workaround, this function serializes the dictionary
 * using the key-value pair IR format, then deserializes it to create a `KeyValuePairLogEvent`
 * instance. This approach is inefficient and intended solely for testing purposes, as it allows
 * instance creation without a full IR stream. TODO: Replace this method with a more efficient
 * conversion once a direct utility is available.
 * @param py_dict
 * @return The converted key-value log event of the given dictionary on success.
 * @return std::nullopt on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto convert_py_dict_to_key_value_pair_log_event(PyDictObject* py_dict
) -> std::optional<clp::ffi::KeyValuePairLogEvent>;

CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_init(
        PyKeyValuePairLogEvent* self,
        PyObject* args,
        PyObject* keywords
) -> int {
    static char keyword_dictionary[]{"dictionary"};
    static char* keyword_table[]{static_cast<char*>(keyword_dictionary), nullptr};

    // If the argument parsing fails, `self` will be deallocated. We must reset all pointers to
    // nullptr in advance, otherwise the deallocator might trigger segmentation fault.
    self->default_init();

    PyObject* dictionary{Py_None};
    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "O",
                static_cast<char**>(keyword_table),
                &dictionary
        )))
    {
        return -1;
    }

    if (false == static_cast<bool>(PyDict_Check(dictionary))) {
        PyErr_SetString(PyExc_TypeError, "`dictionary` must be a Python dictionary object");
        return -1;
    }
    PyDictObject* py_dict{py_reinterpret_cast<PyDictObject>(dictionary)};

    auto optional_kv_pair_log_event{convert_py_dict_to_key_value_pair_log_event(py_dict)};
    if (false == optional_kv_pair_log_event.has_value()) {
        return -1;
    }

    return self->init(std::move(optional_kv_pair_log_event.value())) ? 0 : -1;
}

CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_to_dict(PyKeyValuePairLogEvent* self) -> PyObject* {
    // TODO: Use an efficient algorithm to turn the underlying log event into a Python dictionary
    auto const* kv_pair_log_event{self->get_kv_pair_log_event()};
    auto const serialized_json_result{kv_pair_log_event->serialize_to_json()};
    if (serialized_json_result.has_error()) {
        PyErr_Format(
                PyExc_RuntimeError,
                cKeyValuePairLogEventSerializeToStringErrorFormatStr.data(),
                serialized_json_result.error().message().c_str()
        );
        return nullptr;
    }
    auto const json_str{serialized_json_result.value().dump()};
    PyObjectPtr<PyObject> parsed_json{py_utils_parse_json_str(json_str)};
    if (nullptr == parsed_json) {
        return nullptr;
    }
    if (false == static_cast<bool>(PyDict_Check(parsed_json.get()))) {
        PyErr_SetString(PyExc_TypeError, "Serialized JSON object is not a dictionary");
        return nullptr;
    }
    return parsed_json.release();
}

auto convert_py_dict_to_key_value_pair_log_event(PyDictObject* py_dict
) -> std::optional<clp::ffi::KeyValuePairLogEvent> {
    PyObjectPtr<PyBytesObject> const serialized_msgpack_byte_sequence{
            py_utils_serialize_dict_to_msgpack(py_dict)
    };
    if (nullptr == serialized_msgpack_byte_sequence) {
        return std::nullopt;
    }

    // Since the type is already checked, we can use the macro to avoid duplicated type checking.
    std::span<char const> const data_view{
            PyBytes_AS_STRING(serialized_msgpack_byte_sequence.get()),
            static_cast<size_t>(PyBytes_GET_SIZE(serialized_msgpack_byte_sequence.get()))
    };
    auto const unpack_result{unpack_msgpack(data_view)};
    if (unpack_result.has_error()) {
        PyErr_SetString(PyExc_RuntimeError, unpack_result.error().c_str());
        return std::nullopt;
    }
    auto const& msgpack_obj{unpack_result.value().get()};
    if (msgpack::type::MAP != msgpack_obj.type) {
        PyErr_SetString(PyExc_TypeError, "Unpacked msgpack is not a map");
        return std::nullopt;
    }

    auto serializer_result{
            clp::ffi::ir_stream::Serializer<clp::ir::four_byte_encoded_variable_t>::create()
    };
    if (serializer_result.has_error()) {
        PyErr_Format(
                PyExc_RuntimeError,
                cSerializerCreateErrorFormatStr.data(),
                serializer_result.error().message().c_str()
        );
        return std::nullopt;
    }

    auto& serializer{serializer_result.value()};
    if (false == serializer.serialize_msgpack_map(msgpack_obj.via.map)) {
        PyErr_SetString(PyExc_RuntimeError, cSerializerSerializeMsgpackMapError.data());
        return std::nullopt;
    }

    auto const ir_buf{serializer.get_ir_buf_view()};
    clp::BufferReader buf_reader{
            clp::size_checked_pointer_cast<char const>(ir_buf.data()),
            ir_buf.size()
    };

    auto deserializer_result{
            clp::ffi::ir_stream::Deserializer<IrUnitHandler>::create(buf_reader, IrUnitHandler{})
    };
    if (deserializer_result.has_error()) {
        PyErr_Format(
                PyExc_RuntimeError,
                cDeserializerCreateErrorFormatStr.data(),
                deserializer_result.error().message().c_str()
        );
        return std::nullopt;
    }

    auto& deserializer{deserializer_result.value()};
    auto& ir_unit_handler{deserializer.get_ir_unit_handler()};
    while (true) {
        auto const result{deserializer.deserialize_next_ir_unit(buf_reader)};
        if (result.has_error()) {
            PyErr_Format(
                    PyExc_RuntimeError,
                    cDeserializerDeserializeNextIrUnitErrorFormatStr.data(),
                    result.error().message().c_str()
            );
            return std::nullopt;
        }
        auto const ir_unit_type{result.value()};
        if (clp::ffi::ir_stream::IrUnitType::LogEvent == ir_unit_type) {
            break;
        }
        if (clp::ffi::ir_stream::IrUnitType::SchemaTreeNodeInsertion != ir_unit_type) {
            PyErr_SetString(PyExc_RuntimeError, "Unexpected Ir Unit Type");
            return std::nullopt;
        }
    }

    if (false == ir_unit_handler.log_event.has_value()) {
        PyErr_SetString(PyExc_RuntimeError, "No log event has been deserialized");
        return std::nullopt;
    }

    return std::move(ir_unit_handler.log_event);
}

CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_dealloc(PyKeyValuePairLogEvent* self) -> void {
    self->clean();
    Py_TYPE(self)->tp_free(py_reinterpret_cast<PyObject>(self));
}
}  // namespace

auto PyKeyValuePairLogEvent::init(clp::ffi::KeyValuePairLogEvent kv_pair_log_event) -> bool {
    m_kv_pair_log_event = new clp::ffi::KeyValuePairLogEvent{std::move(kv_pair_log_event)};
    if (nullptr == m_kv_pair_log_event) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::cOutofMemoryError);
        return false;
    }
    return true;
}

auto PyKeyValuePairLogEvent::get_py_type() -> PyTypeObject* {
    return m_py_type.get();
}

auto PyKeyValuePairLogEvent::module_level_init(PyObject* py_module) -> bool {
    static_assert(std::is_trivially_destructible<PyKeyValuePairLogEvent>());
    auto* type{py_reinterpret_cast<PyTypeObject>(PyType_FromSpec(&PyKeyValuePairLogEvent_type_spec))
    };
    m_py_type.reset(type);
    if (nullptr == type) {
        return false;
    }
    return add_python_type(get_py_type(), "KeyValuePairLogEvent", py_module);
}
}  // namespace clp_ffi_py::ir::native
