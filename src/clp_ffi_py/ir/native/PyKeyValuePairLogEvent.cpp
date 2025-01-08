#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include "PyKeyValuePairLogEvent.hpp"

#include <cstddef>
#include <cstdint>
#include <new>
#include <optional>
#include <span>
#include <stack>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <clp/BufferReader.hpp>
#include <clp/ffi/ir_stream/decoding_methods.hpp>
#include <clp/ffi/ir_stream/Deserializer.hpp>
#include <clp/ffi/ir_stream/IrUnitType.hpp>
#include <clp/ffi/ir_stream/Serializer.hpp>
#include <clp/ffi/KeyValuePairLogEvent.hpp>
#include <clp/ffi/SchemaTree.hpp>
#include <clp/ffi/Value.hpp>
#include <clp/ir/EncodedTextAst.hpp>
#include <clp/ir/types.hpp>
#include <clp/time_types.hpp>
#include <clp/TraceableException.hpp>
#include <clp/type_utils.hpp>
#include <wrapped_facade_headers/msgpack.hpp>

#include <clp_ffi_py/api_decoration.hpp>
#include <clp_ffi_py/error_messages.hpp>
#include <clp_ffi_py/ir/native/error_messages.hpp>
#include <clp_ffi_py/Py_utils.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
using clp::ffi::ir_stream::IRErrorCode;
using clp::ffi::KeyValuePairLogEvent;
using clp::ffi::SchemaTree;
using clp::ffi::Value;
using clp::ir::EightByteEncodedTextAst;
using clp::ir::FourByteEncodedTextAst;

namespace {
/**
 * Class that implements `clp::ffi::ir_stream::IrUnitHandlerInterface` for deserializing log events.
 */
class IrUnitHandler {
public:
    // Methods that implement the `clp::ffi::ir_stream::IrUnitHandlerInterface` interface
    [[nodiscard]] auto handle_log_event(clp::ffi::KeyValuePairLogEvent&& deserialized_log_event)
            -> IRErrorCode {
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

    // TODO: We should enable linting when clang-tidy config is up-to-date to allow simple classes.
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes,readability-identifier-naming)
    std::optional<clp::ffi::KeyValuePairLogEvent> log_event;
};

/**
 * Helper class for `PyKeyValuePairLogEvent::to_dict`.
 */
class PyDictSerializationIterator {
public:
    // Factory function
    /**
     * Creates an iterator with the given inputs.
     * @param schema_tree_node
     * @param schema_subtree_bitmap
     * @param parent
     * @return A newly created iterator that holds a new reference of a Python dictionary on
     * success.
     * @return std::nullopt on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto create(
            SchemaTree::Node const* schema_tree_node,
            std::vector<bool> const& schema_subtree_bitmap,
            PyDictObject* parent
    ) -> std::optional<PyDictSerializationIterator> {
        if (schema_tree_node->is_root() && nullptr != parent) {
            PyErr_SetString(
                    PyExc_RuntimeError,
                    "KeyValuePairLogEvent.to_dict(): Root node cannot have a parent"
            );
            return std::nullopt;
        }
        if (false == schema_tree_node->is_root() && nullptr == parent) {
            PyErr_SetString(
                    PyExc_RuntimeError,
                    "KeyValuePairLogEvent.to_dict(): Parent cannot be empty for non-root node"
            );
            return std::nullopt;
        }

        PyObjectPtr<PyDictObject> py_dict{py_reinterpret_cast<PyDictObject>(PyDict_New())};
        if (nullptr == py_dict) {
            return std::nullopt;
        }

        std::vector<SchemaTree::Node::id_t> child_schema_tree_nodes;
        for (auto const child_id : schema_tree_node->get_children_ids()) {
            if (schema_subtree_bitmap[child_id]) {
                child_schema_tree_nodes.push_back(child_id);
            }
        }

        return PyDictSerializationIterator{
                schema_tree_node,
                std::move(child_schema_tree_nodes),
                parent,
                std::move(py_dict)
        };
    }

    // Delete copy constructor and assignment
    PyDictSerializationIterator(PyDictSerializationIterator const&) = delete;
    auto operator=(PyDictSerializationIterator const&) -> PyDictSerializationIterator& = delete;

    // Default move constructor and assignment
    PyDictSerializationIterator(PyDictSerializationIterator&&) = default;
    auto operator=(PyDictSerializationIterator&&) -> PyDictSerializationIterator& = default;

    // Destructor
    ~PyDictSerializationIterator() = default;

    /**
     * @return Whether there are more child schema tree nodes to traverse.
     */
    [[nodiscard]] auto has_next_child_schema_tree_node() const -> bool {
        return m_child_schema_tree_node_it != m_child_schema_tree_nodes.end();
    }

    /**
     * Gets the id of the next child schema tree node and advances the iterator.
     * @return The id of the next child schema tree node.
     */
    [[nodiscard]] auto get_next_child_schema_tree_node_id() -> SchemaTree::Node::id_t {
        return *(m_child_schema_tree_node_it++);
    }

    /**
     * Adds the underlying Python dictionary into the parent dictionary.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto add_to_parent_dict() -> bool {
        if (is_root()) {
            PyErr_SetString(
                    PyExc_RuntimeError,
                    "KeyValuePairLogEvent.to_dict(): root has no parent to add"
            );
            return false;
        }
        PyObjectPtr<PyObject> const py_key{
                construct_py_str_from_string_view(m_schema_tree_node->get_key_name())
        };
        if (nullptr == py_key) {
            return false;
        }
        return 0
               == PyDict_SetItem(
                       py_reinterpret_cast<PyObject>(m_parent_py_dict),
                       py_key.get(),
                       py_reinterpret_cast<PyObject>(m_py_dict.get())
               );
    }

    /**
     * Releases the underlying Python dictionary as the root dictionary to return.
     * @return The released Python dictionary on success.
     * @return nullptr on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto release_root() -> PyDictObject* {
        if (false == is_root()) {
            PyErr_SetString(
                    PyExc_RuntimeError,
                    "KeyValuePairLogEvent.to_dict(): only root can be released"
            );
            return nullptr;
        }
        return m_py_dict.release();
    }

    [[nodiscard]] auto get_py_dict() -> PyDictObject* { return m_py_dict.get(); }

    [[nodiscard]] auto is_root() const -> bool { return m_schema_tree_node->is_root(); }

private:
    // Constructor
    PyDictSerializationIterator(
            SchemaTree::Node const* schema_tree_node,
            std::vector<SchemaTree::Node::id_t> child_schema_tree_nodes,
            PyDictObject* parent,
            PyObjectPtr<PyDictObject> py_dict
    )
            : m_schema_tree_node{schema_tree_node},
              m_child_schema_tree_nodes{std::move(child_schema_tree_nodes)},
              m_child_schema_tree_node_it{m_child_schema_tree_nodes.cbegin()},
              m_parent_py_dict{parent},
              m_py_dict{std::move(py_dict)} {}

    SchemaTree::Node const* m_schema_tree_node;
    std::vector<SchemaTree::Node::id_t> m_child_schema_tree_nodes;
    std::vector<SchemaTree::Node::id_t>::const_iterator m_child_schema_tree_node_it;
    PyDictObject* m_parent_py_dict;
    PyObjectPtr<PyDictObject> m_py_dict;
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
);
CLP_FFI_PY_METHOD auto
PyKeyValuePairLogEvent_init(PyKeyValuePairLogEvent* self, PyObject* args, PyObject* keywords)
        -> int;

/**
 * Callback of `PyKeyValuePairLogEvent`'s `to_dict` method.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyKeyValuePairLogEventToDictDoc,
        "to_dict(self)\n"
        "--\n\n"
        "Converts the log event into a Python dictionary.\n\n"
        ":return: The log event as a Python dictionary.\n"
        ":rtype: dict[str, Any]\n"
);
CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_to_dict(PyKeyValuePairLogEvent* self) -> PyObject*;

/**
 * Callback of `PyKeyValuePairLogEvent`'s deallocator.
 */
CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_dealloc(PyKeyValuePairLogEvent* self) -> void;

// NOLINTNEXTLINE(*-avoid-c-arrays, cppcoreguidelines-avoid-non-const-global-variables)
PyMethodDef PyKeyValuePairLogEvent_method_table[]{
        {"to_dict",
         py_c_function_cast(PyKeyValuePairLogEvent_to_dict),
         METH_NOARGS,
         static_cast<char const*>(cPyKeyValuePairLogEventToDictDoc)},

        {nullptr}
};

// NOLINTBEGIN(*-avoid-c-arrays, cppcoreguidelines-pro-type-*-cast)
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
PyType_Slot PyKeyValuePairLogEvent_slots[]{
        {Py_tp_alloc, reinterpret_cast<void*>(PyType_GenericAlloc)},
        {Py_tp_dealloc, reinterpret_cast<void*>(PyKeyValuePairLogEvent_dealloc)},
        {Py_tp_new, reinterpret_cast<void*>(PyType_GenericNew)},
        {Py_tp_init, reinterpret_cast<void*>(PyKeyValuePairLogEvent_init)},
        {Py_tp_methods, static_cast<void*>(PyKeyValuePairLogEvent_method_table)},
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPyKeyValuePairLogEventDoc))},
        {0, nullptr}
};
// NOLINTEND(*-avoid-c-arrays, cppcoreguidelines-pro-type-*-cast)

/**
 * `PyKeyValuePairLogEvent`'s Python type specifications.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
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
[[nodiscard]] auto convert_py_dict_to_key_value_pair_log_event(PyDictObject* py_dict)
        -> std::optional<clp::ffi::KeyValuePairLogEvent>;

/**
 * Serializes the given node id value pairs into a Python dictionary object.
 * @param schema_tree
 * @param schema_subtree_bitmap
 * @param node_id_value_pairs
 * @return A new reference to the serialized dictionary object on success.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto serialize_node_id_value_pair_to_py_dict(
        SchemaTree const& schema_tree,
        std::vector<bool> const& schema_subtree_bitmap,
        KeyValuePairLogEvent::NodeIdValuePairs const& node_id_value_pairs
) -> PyDictObject*;

/**
 * Inserts the given key-value pair into the JSON object (map).
 * @param node The schema tree node of the key to insert.
 * @param optional_val The value to insert.
 * @param dict The Python dictionary to insert the kv-pair into.
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto insert_kv_pair_into_py_dict(
        SchemaTree::Node const& node,
        std::optional<Value> const& optional_val,
        PyDictObject* dict
) -> bool;

/**
 * Decodes a value as an `EncodedTextAst` according to the encoding type.
 * NOTE: This function assumes that `val` is either a `FourByteEncodedTextAst` or
 * `EightByteEncodedTextAst`.
 * @param val
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto decode_as_encoded_text_ast(Value const& val) -> std::optional<std::string>;

CLP_FFI_PY_METHOD auto
PyKeyValuePairLogEvent_init(PyKeyValuePairLogEvent* self, PyObject* args, PyObject* keywords)
        -> int {
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
    return py_reinterpret_cast<PyObject>(self->to_dict());
}

CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_dealloc(PyKeyValuePairLogEvent* self) -> void {
    self->clean();
    Py_TYPE(self)->tp_free(py_reinterpret_cast<PyObject>(self));
}

auto convert_py_dict_to_key_value_pair_log_event(PyDictObject* py_dict)
        -> std::optional<clp::ffi::KeyValuePairLogEvent> {
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
                get_c_str_from_constexpr_string_view(cSerializerCreateErrorFormatStr),
                serializer_result.error().message().c_str()
        );
        return std::nullopt;
    }

    auto& serializer{serializer_result.value()};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    if (false == serializer.serialize_msgpack_map(msgpack_obj.via.map)) {
        PyErr_SetString(
                PyExc_RuntimeError,
                get_c_str_from_constexpr_string_view(cSerializerSerializeMsgpackMapError)
        );
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
                get_c_str_from_constexpr_string_view(cDeserializerCreateErrorFormatStr),
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
                    get_c_str_from_constexpr_string_view(
                            cDeserializerDeserializeNextIrUnitErrorFormatStr
                    ),
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

auto serialize_node_id_value_pair_to_py_dict(
        SchemaTree const& schema_tree,
        std::vector<bool> const& schema_subtree_bitmap,
        KeyValuePairLogEvent::NodeIdValuePairs const& node_id_value_pairs
) -> PyDictObject* {
    PyObjectPtr<PyDictObject> root_dict;
    using DfsIterator = PyDictSerializationIterator;

    std::stack<DfsIterator> dfs_stack;
    auto optional_root_iterator
            = DfsIterator::create(&schema_tree.get_root(), schema_subtree_bitmap, nullptr);
    if (false == optional_root_iterator.has_value()) {
        return nullptr;
    }
    dfs_stack.emplace(std::move(optional_root_iterator.value()));

    while (false == dfs_stack.empty()) {
        auto& dfs_stack_top{dfs_stack.top()};
        if (false == dfs_stack_top.has_next_child_schema_tree_node()) {
            if (dfs_stack_top.is_root()) {
                root_dict.reset(dfs_stack_top.release_root());
            } else {
                if (false == dfs_stack_top.add_to_parent_dict()) {
                    return nullptr;
                }
            }
            dfs_stack.pop();
            continue;
        }
        auto const child_schema_tree_node_id{dfs_stack_top.get_next_child_schema_tree_node_id()};
        auto const& child_schema_tree_node{schema_tree.get_node(child_schema_tree_node_id)};
        if (false == node_id_value_pairs.contains(child_schema_tree_node_id)) {
            auto optional_iterator{DfsIterator::create(
                    &child_schema_tree_node,
                    schema_subtree_bitmap,
                    dfs_stack_top.get_py_dict()
            )};
            if (false == optional_iterator.has_value()) {
                return nullptr;
            }
            dfs_stack.emplace(std::move(optional_iterator.value()));
            continue;
        }
        if (false
            == insert_kv_pair_into_py_dict(
                    child_schema_tree_node,
                    node_id_value_pairs.at(child_schema_tree_node_id),
                    dfs_stack_top.get_py_dict()
            ))
        {
            return nullptr;
        }
    }

    return root_dict.release();
}

[[nodiscard]] auto insert_kv_pair_into_py_dict(
        SchemaTree::Node const& node,
        std::optional<Value> const& optional_val,
        PyDictObject* dict
) -> bool {
    PyObjectPtr<PyObject> const py_key{construct_py_str_from_string_view(node.get_key_name())};
    if (nullptr == py_key) {
        return false;
    }

    if (false == optional_val.has_value()) {
        PyObjectPtr<PyObject> const empty_dict{PyDict_New()};
        return 0
               == PyDict_SetItem(
                       py_reinterpret_cast<PyObject>(dict),
                       py_key.get(),
                       empty_dict.get()
               );
    }

    auto const type{node.get_type()};
    auto const& val{optional_val.value()};
    PyObjectPtr<PyObject> py_value;
    switch (type) {
        case SchemaTree::Node::Type::Int:
            py_value.reset(PyLong_FromLongLong(val.get_immutable_view<clp::ffi::value_int_t>()));
            break;
        case SchemaTree::Node::Type::Float:
            py_value.reset(PyFloat_FromDouble(val.get_immutable_view<clp::ffi::value_float_t>()));
            break;
        case SchemaTree::Node::Type::Bool:
            py_value.reset(PyBool_FromLong(
                    static_cast<long>(val.get_immutable_view<clp::ffi::value_bool_t>())
            ));
            break;
        case SchemaTree::Node::Type::Str:
            if (val.is<std::string>()) {
                auto const val_str{val.get_immutable_view<std::string>()};
                py_value.reset(PyUnicode_FromStringAndSize(
                        val_str.data(),
                        static_cast<Py_ssize_t>(val_str.size())
                ));
            } else {
                auto const decoded_result{decode_as_encoded_text_ast(val)};
                if (false == decoded_result.has_value()) {
                    return false;
                }
                std::string_view const decoded_str{decoded_result.value()};
                py_value.reset(PyUnicode_FromStringAndSize(
                        decoded_str.data(),
                        static_cast<Py_ssize_t>(decoded_str.size())
                ));
            }
            break;
        case SchemaTree::Node::Type::UnstructuredArray: {
            auto const decoded_result{decode_as_encoded_text_ast(val)};
            if (false == decoded_result.has_value()) {
                return false;
            }
            py_value.reset(py_utils_parse_json_str(decoded_result.value()));
            break;
        }
        case SchemaTree::Node::Type::Obj:
            py_value.reset(Py_None);
            break;
        default:
            PyErr_Format(
                    PyExc_RuntimeError,
                    "Unknown schema tree node type: %d",
                    static_cast<uint32_t>(type)
            );
            return false;
    }

    if (nullptr == py_value) {
        return false;
    }

    return 0 == PyDict_SetItem(py_reinterpret_cast<PyObject>(dict), py_key.get(), py_value.get());
}

auto decode_as_encoded_text_ast(Value const& val) -> std::optional<std::string> {
    auto const result{
            val.is<FourByteEncodedTextAst>()
                    ? val.get_immutable_view<FourByteEncodedTextAst>().decode_and_unparse()
                    : val.get_immutable_view<EightByteEncodedTextAst>().decode_and_unparse()
    };
    if (false == result.has_value()) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to deserialize CLP encoded text AST");
    }
    return result;
}
}  // namespace

auto PyKeyValuePairLogEvent::create(clp::ffi::KeyValuePairLogEvent kv_log_event)
        -> PyKeyValuePairLogEvent* {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    PyKeyValuePairLogEvent* self{PyObject_New(PyKeyValuePairLogEvent, get_py_type())};
    if (nullptr == self) {
        return nullptr;
    }
    self->default_init();
    if (false == self->init(std::move(kv_log_event))) {
        return nullptr;
    }
    return self;
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

auto PyKeyValuePairLogEvent::init(clp::ffi::KeyValuePairLogEvent kv_pair_log_event) -> bool {
    m_kv_pair_log_event
            = new (std::nothrow) clp::ffi::KeyValuePairLogEvent{std::move(kv_pair_log_event)};
    if (nullptr == m_kv_pair_log_event) {
        PyErr_SetString(
                PyExc_RuntimeError,
                get_c_str_from_constexpr_string_view(clp_ffi_py::cOutOfMemoryError)
        );
        return false;
    }
    return true;
}

[[nodiscard]] auto PyKeyValuePairLogEvent::to_dict() -> PyDictObject* {
    try {
        auto const& node_id_value_pairs{m_kv_pair_log_event->get_node_id_value_pairs()};
        auto const& schema_tree{m_kv_pair_log_event->get_schema_tree()};
        auto const schema_subtree_bitmap_result{m_kv_pair_log_event->get_schema_subtree_bitmap()};
        if (schema_subtree_bitmap_result.has_error()) {
            PyErr_Format(
                    PyExc_RuntimeError,
                    "Failed to get schema subtree bitmap: %s",
                    schema_subtree_bitmap_result.error().message().c_str()
            );
            return nullptr;
        }
        return serialize_node_id_value_pair_to_py_dict(
                schema_tree,
                schema_subtree_bitmap_result.value(),
                node_id_value_pairs
        );
    } catch (clp::TraceableException& ex) {
        handle_traceable_exception(ex);
        return nullptr;
    }
}
}  // namespace clp_ffi_py::ir::native
