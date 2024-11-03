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
    // Implements `clp::ffi::ir_stream::IrUnitHandlerInterface` interface
    [[nodiscard]] auto handle_log_event(clp::ffi::KeyValuePairLogEvent&& log_event) -> IRErrorCode {
        m_log_event.emplace(std::move(log_event));
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

    // NOLINTNEXTLINE(*)
    std::optional<clp::ffi::KeyValuePairLogEvent> m_log_event;
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyKeyValuePairLogEventDoc,
        "This class represents a key-value pair log event and provides ways to access the"
        " underlying log data. This class is designed to be instantiated by the IR deserializer."
        " However, direct instantiation using the `__init__` method is also supported for testing"
        " purposes, although this may not be as efficient as emitting from the IR deserializer.\n\n"
        "The signature of `__init__` method is shown as following:\n\n"
        "__init__(self, dictionary)\n\n"
        "Initializes an object that represents a key-value pair log event from the given Python"
        " dictionary. Notice that each object should be strictly initialized only once. Double"
        " initialization will result in memory leak.\n\n"
        ":param dictionary: A dictionary representing the key-value log event, where all keys are"
        " expected to be of string type, including keys inside any sub-dictionaries.\n"
);

/**
 * Callback of `PyKeyValuePairLogEvent`'s `__init__` method:
 * @param self
 * @param args
 * @param keywords
 * @return 0 on success.
 * @return -1 on failure with the relevant Python exception and error set.
 */
CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_init(
        PyKeyValuePairLogEvent* self,
        PyObject* args,
        PyObject* keywords
) -> int;

/**
 * Callback of `PyKeyValuePairLogEvent`'s deallocator.
 * @param self
 */
CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_dealloc(PyKeyValuePairLogEvent* self) -> void;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyKeyValuePairLogEventToDictDoc,
        "to_dict(self)\n"
        "--\n\n"
        "Converts the underlying key-value pair log event into a Python dictionary.\n\n"
        ":return: Serialized log event in a Python dictionary.\n"
);

CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_to_dict(PyKeyValuePairLogEvent* self) -> PyObject*;

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
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPyKeyValuePairLogEventToDictDoc))},
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
 * instance creation without a full IR stream. Future implementations should replace this method
 * with a more efficient conversion once a direct utility is available.
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

    // If the argument parsing fails, `self` will be deallocated. We must reset all pointers to
    // nullptr in advance, otherwise the deallocator might trigger segmentation fault.
    self->default_init();

    if (false == static_cast<bool>(PyDict_Check(dictionary))) {
        PyErr_SetString(PyExc_TypeError, "`dictionary` must be a Python dictionary object");
        return -1;
    }
    PyDictObject* py_dict{py_reinterpret_cast<PyDictObject>(dictionary)};

    auto optional_kv_pair_log_event{convert_py_dict_to_key_value_pair_log_event(py_dict)};
    if (false == optional_kv_pair_log_event.has_value()) {
        return -1;
    }

    if (false == self->init(std::move(optional_kv_pair_log_event.value()))) {
        return -1;
    }

    return 0;
}

CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_dealloc(PyKeyValuePairLogEvent* self) -> void {
    self->clean();
    PyObject_Del(self);
}

CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_to_dict(PyKeyValuePairLogEvent* self) -> PyObject* {
    // TODO: use an efficient algorithm to turn the underlying log event to a Python dictionary
    auto const& kv_pair_log_event{self->get_kv_pair_log_event()};
    auto const serialized_json_result{kv_pair_log_event.serialize_to_json()};
    if (serialized_json_result.has_error()) {
        PyErr_Format(
                PyExc_RuntimeError,
                cKeyValuePairLogEventSerializeToStringErrorFormatStr.data(),
                serialized_json_result.error().message().c_str()
        );
        return nullptr;
    }
    auto const json_str{serialized_json_result.value().dump()};
    auto* parsed_json{py_utils_parse_json_str(json_str)};
    if (nullptr == parsed_json) {
        return nullptr;
    }
    if (false == static_cast<bool>(PyDict_Check(parsed_json))) {
        PyErr_SetString(PyExc_TypeError, "Serialized JSON object is not a dictionary");
    }
    return parsed_json;
}

auto convert_py_dict_to_key_value_pair_log_event(PyDictObject* py_dict
) -> std::optional<clp::ffi::KeyValuePairLogEvent> {
    auto* serialized_msgpack_byte_sequence{py_utils_serialize_dict_to_msgpack(py_dict)};
    if (nullptr == serialized_msgpack_byte_sequence) {
        return std::nullopt;
    }

    // Since the type is already checked, we can use the macro to avoid duplicated type checking.
    std::span<char const> const data_view{
            PyBytes_AS_STRING(serialized_msgpack_byte_sequence),
            static_cast<size_t>(PyBytes_GET_SIZE(serialized_msgpack_byte_sequence))
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
        if (ir_unit_type == clp::ffi::ir_stream::IrUnitType::SchemaTreeNodeInsertion) {
            continue;
        }
        if (ir_unit_type != clp::ffi::ir_stream::IrUnitType::LogEvent) {
            PyErr_SetString(PyExc_RuntimeError, "Unexpected Ir Unit Type");
            return std::nullopt;
        }
        break;
    }

    if (false == ir_unit_handler.m_log_event.has_value()) {
        PyErr_SetString(PyExc_RuntimeError, "No log event has been deserialized");
        return std::nullopt;
    }

    return std::move(ir_unit_handler.m_log_event);
}
}  // namespace

auto PyKeyValuePairLogEvent::init(clp::ffi::KeyValuePairLogEvent kv_pair_log_event) -> bool {
    m_kv_pair_log_event = new clp::ffi::KeyValuePairLogEvent{std::move(kv_pair_log_event)};
    return nullptr != m_kv_pair_log_event;
}

auto PyKeyValuePairLogEvent::get_py_type() -> PyTypeObject* {
    return m_py_type.get();
}

auto PyKeyValuePairLogEvent::module_level_init(PyObject* py_module) -> bool {
    // TODO: complete this function
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
