#ifndef CLP_FFI_PY_IR_NATIVE_PYKEYVALUEPAIRLOGEVENT_HPP
#define CLP_FFI_PY_IR_NATIVE_PYKEYVALUEPAIRLOGEVENT_HPP

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include <concepts>
#include <cstdint>
#include <optional>
#include <stack>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <clp/ffi/KeyValuePairLogEvent.hpp>
#include <clp/ffi/SchemaTree.hpp>
#include <clp/ffi/Value.hpp>
#include <clp/TraceableException.hpp>
#include <gsl/gsl>

#include <clp_ffi_py/Py_utils.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
/**
 * Concept that defines the method to construct a `PyUnicode` object from a string view, which
 * allows user-defined behaviours of decoding a byte sequence into a Python unicode object.
 * @tparam StringViewToPyUnicodeMethod
 * @param sv_to_py_unicode_method
 * @param sv
 * @return A new reference of the newly constructed `PyUnicode` object on success.
 * @return std::nullptr on failures with relevant Python exception and error set.
 */
template <typename StringViewToPyUnicodeMethod>
concept StringViewToPyUnicodeMethodReq
        = requires(StringViewToPyUnicodeMethod sv_to_py_unicode_method, std::string_view sv) {
              {
                  sv_to_py_unicode_method(sv)
              } -> std::same_as<PyObject*>;
          };

/**
 * A PyObject structure functioning as a Python-compatible interface to retrieve a key-value pair
 * log event. The underlying data is pointed to by `m_kv_pair_log_event`.
 */
class PyKeyValuePairLogEvent {
public:
    /**
     * CPython-level factory function.
     * @param kv_log_event
     * @return a new reference of a `PyKeyValuePairLogEvent` object that is initialized with the
     * given kv log event.
     * @return nullptr on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto create(clp::ffi::KeyValuePairLogEvent kv_log_event)
            -> PyKeyValuePairLogEvent*;

    /**
     * Gets the `PyTypeObject` that represents `PyKeyValuePair`'s Python type. This type is
     * dynamically created and initialized during the execution of `module_level_init`.
     * @return Python type object associated with `PyKeyValuePairLogEvent`.
     */
    [[nodiscard]] static auto get_py_type() -> PyTypeObject*;

    /**
     * Creates and initializes `PyKeyValuePairLogEvent` as a Python type, and then incorporates this
     * type as a Python object into the py_module module.
     * @param py_module The Python module where the initialized `PyKeyValuePairLogEvent` will be
     * incorporated.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto module_level_init(PyObject* py_module) -> bool;

    // Delete default constructor to disable direct instantiation.
    PyKeyValuePairLogEvent() = delete;

    // Delete copy & move constructors and assignment operators
    PyKeyValuePairLogEvent(PyKeyValuePairLogEvent const&) = delete;
    PyKeyValuePairLogEvent(PyKeyValuePairLogEvent&&) = delete;
    auto operator=(PyKeyValuePairLogEvent const&) -> PyKeyValuePairLogEvent& = delete;
    auto operator=(PyKeyValuePairLogEvent&&) -> PyKeyValuePairLogEvent& = delete;

    // Destructor
    ~PyKeyValuePairLogEvent() = default;

    /**
     * Initializes the underlying data with the given inputs. Since the memory allocation of
     * `PyKeyValuePairLogEvent` is handled by CPython's allocator, cpp constructors will not be
     * explicitly called. This function serves as the default constructor to initialize the
     * underlying key-value pair log event. It has to be called manually to create a
     * `PyKeyValuePairLogEvent` object through CPython APIs.
     * @param kv_pair_log_event
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto init(clp::ffi::KeyValuePairLogEvent kv_pair_log_event) -> bool;

    /**
     * Initializes the pointers to nullptr by default. Should be called once the object is
     * allocated.
     */
    auto default_init() -> void { m_kv_pair_log_event = nullptr; }

    /**
     * Releases the memory allocated for underlying data fields.
     */
    auto clean() -> void {
        delete m_kv_pair_log_event;
        m_kv_pair_log_event = nullptr;
    }

    [[nodiscard]] auto get_kv_pair_log_event() const -> clp::ffi::KeyValuePairLogEvent const* {
        return static_cast<clp::ffi::KeyValuePairLogEvent const*>(m_kv_pair_log_event);
    }

    /**
     * Converts the underlying key-value pair log event into Python dictionaries.
     * @tparam StringViewToPyUnicodeMethod
     * @param string_view_to_py_unicode_method
     * @return A new reference to a Python tuple containing a pair of Python dictionaries on
     * success:
     * - A Python dictionary for auto-generated key-value pairs.
     * - A Python dictionary for user-generated key-value pairs.
     * @return nullptr on failure with the relevant Python exception and error set.
     */
    template <StringViewToPyUnicodeMethodReq StringViewToPyUnicodeMethod>
    [[nodiscard]] auto to_dict(StringViewToPyUnicodeMethod string_view_to_py_unicode_method)
            -> PyObject*;

private:
    static inline PyObjectStaticPtr<PyTypeObject> m_py_type{nullptr};

    // Variables
    PyObject_HEAD;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    gsl::owner<clp::ffi::KeyValuePairLogEvent*> m_kv_pair_log_event;
};

// NOLINTNEXTLINE(readability-identifier-naming)
namespace PyKeyValuePairLogEvent_internal {
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
            clp::ffi::SchemaTree::Node const* schema_tree_node,
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

        std::vector<clp::ffi::SchemaTree::Node::id_t> child_schema_tree_nodes;
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
    [[nodiscard]] auto get_next_child_schema_tree_node_id() -> clp::ffi::SchemaTree::Node::id_t {
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
            clp::ffi::SchemaTree::Node const* schema_tree_node,
            std::vector<clp::ffi::SchemaTree::Node::id_t> child_schema_tree_nodes,
            PyDictObject* parent,
            PyObjectPtr<PyDictObject> py_dict
    )
            : m_schema_tree_node{schema_tree_node},
              m_child_schema_tree_nodes{std::move(child_schema_tree_nodes)},
              m_child_schema_tree_node_it{m_child_schema_tree_nodes.cbegin()},
              m_parent_py_dict{parent},
              m_py_dict{std::move(py_dict)} {}

    clp::ffi::SchemaTree::Node const* m_schema_tree_node;
    std::vector<clp::ffi::SchemaTree::Node::id_t> m_child_schema_tree_nodes;
    std::vector<clp::ffi::SchemaTree::Node::id_t>::const_iterator m_child_schema_tree_node_it;
    PyDictObject* m_parent_py_dict;
    PyObjectPtr<PyDictObject> m_py_dict;
};

/**
 * Serializes the given node id value pairs into a Python dictionary object.
 * @tparam StringViewToUnicodeMethod
 * @param schema_tree
 * @param schema_subtree_bitmap
 * @param node_id_value_pairs
 * @param string_view_to_py_unicode_method
 * @return A new reference to the serialized dictionary object on success.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
template <StringViewToPyUnicodeMethodReq StringViewToPyUnicodeMethod>
[[nodiscard]] auto serialize_node_id_value_pair_to_py_dict(
        clp::ffi::SchemaTree const& schema_tree,
        std::vector<bool> const& schema_subtree_bitmap,
        clp::ffi::KeyValuePairLogEvent::NodeIdValuePairs const& node_id_value_pairs,
        StringViewToPyUnicodeMethod string_view_to_py_unicode_method
) -> PyDictObject*;

/**
 * Inserts the given key-value pair into the JSON object (map).
 * @tparam StringViewToPyUnicodeMethod
 * @param node The schema tree node of the key to insert.
 * @param optional_val The value to insert.
 * @param dict The Python dictionary to insert the kv-pair into.
 * @param string_view_to_py_unicode_method
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
template <StringViewToPyUnicodeMethodReq StringViewToPyUnicodeMethod>
[[nodiscard]] auto insert_kv_pair_into_py_dict(
        clp::ffi::SchemaTree::Node const& node,
        std::optional<clp::ffi::Value> const& optional_val,
        PyDictObject* dict,
        StringViewToPyUnicodeMethod string_view_to_py_unicode_method
) -> bool;

/**
 * Decodes a value as an `EncodedTextAst` according to the encoding type.
 * NOTE: This function assumes that `val` is either a `FourByteEncodedTextAst` or
 * `EightByteEncodedTextAst`.
 * @param val
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto decode_as_encoded_text_ast(clp::ffi::Value const& val)
        -> std::optional<std::string>;

template <StringViewToPyUnicodeMethodReq StringViewToPyUnicodeMethod>
auto serialize_node_id_value_pair_to_py_dict(
        clp::ffi::SchemaTree const& schema_tree,
        std::vector<bool> const& schema_subtree_bitmap,
        clp::ffi::KeyValuePairLogEvent::NodeIdValuePairs const& node_id_value_pairs,
        StringViewToPyUnicodeMethod string_view_to_py_unicode_method
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
                    dfs_stack_top.get_py_dict(),
                    string_view_to_py_unicode_method
            ))
        {
            return nullptr;
        }
    }

    return root_dict.release();
}

template <StringViewToPyUnicodeMethodReq StringViewToPyUnicodeMethod>
auto insert_kv_pair_into_py_dict(
        clp::ffi::SchemaTree::Node const& node,
        std::optional<clp::ffi::Value> const& optional_val,
        PyDictObject* dict,
        StringViewToPyUnicodeMethod string_view_to_py_unicode_method
) -> bool {
    PyObjectPtr<PyObject> const py_key{string_view_to_py_unicode_method(node.get_key_name())};
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
        case clp::ffi::SchemaTree::Node::Type::Int:
            py_value.reset(PyLong_FromLongLong(val.get_immutable_view<clp::ffi::value_int_t>()));
            break;
        case clp::ffi::SchemaTree::Node::Type::Float:
            py_value.reset(PyFloat_FromDouble(val.get_immutable_view<clp::ffi::value_float_t>()));
            break;
        case clp::ffi::SchemaTree::Node::Type::Bool:
            py_value.reset(PyBool_FromLong(
                    static_cast<long>(val.get_immutable_view<clp::ffi::value_bool_t>())
            ));
            break;
        case clp::ffi::SchemaTree::Node::Type::Str:
            if (val.is<std::string>()) {
                std::string_view const val_str{val.get_immutable_view<std::string>()};
                py_value.reset(string_view_to_py_unicode_method(val_str));
            } else {
                auto const decoded_result{decode_as_encoded_text_ast(val)};
                if (false == decoded_result.has_value()) {
                    return false;
                }
                std::string_view const decoded_str{decoded_result.value()};
                py_value.reset(string_view_to_py_unicode_method(decoded_str));
            }
            break;
        case clp::ffi::SchemaTree::Node::Type::UnstructuredArray: {
            auto const decoded_result{decode_as_encoded_text_ast(val)};
            if (false == decoded_result.has_value()) {
                return false;
            }
            py_value.reset(py_utils_parse_json_str(decoded_result.value()));
            break;
        }
        case clp::ffi::SchemaTree::Node::Type::Obj:
            py_value.reset(get_new_ref_to_py_none());
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
}  // namespace PyKeyValuePairLogEvent_internal

template <StringViewToPyUnicodeMethodReq StringViewToPyUnicodeMethod>
auto PyKeyValuePairLogEvent::to_dict(StringViewToPyUnicodeMethod string_view_to_py_unicode_method)
        -> PyObject* {
    try {
        auto const& auto_gen_node_id_value_pairs{
                m_kv_pair_log_event->get_auto_gen_node_id_value_pairs()
        };
        auto const& auto_gen_keys_schema_tree{m_kv_pair_log_event->get_auto_gen_keys_schema_tree()};
        auto const auto_gen_keys_schema_subtree_bitmap_result{
                m_kv_pair_log_event->get_auto_gen_keys_schema_subtree_bitmap()
        };
        if (auto_gen_keys_schema_subtree_bitmap_result.has_error()) {
            PyErr_Format(
                    PyExc_RuntimeError,
                    "Failed to get auto-generated keys schema subtree bitmap: %s",
                    auto_gen_keys_schema_subtree_bitmap_result.error().message().c_str()
            );
            return nullptr;
        }
        PyObjectPtr<PyDictObject> const auto_gen_kv_pairs_dict{
                PyKeyValuePairLogEvent_internal::serialize_node_id_value_pair_to_py_dict(
                        auto_gen_keys_schema_tree,
                        auto_gen_keys_schema_subtree_bitmap_result.value(),
                        auto_gen_node_id_value_pairs,
                        string_view_to_py_unicode_method
                )
        };
        if (nullptr == auto_gen_kv_pairs_dict) {
            return nullptr;
        }

        auto const& user_gen_node_id_value_pairs{
                m_kv_pair_log_event->get_user_gen_node_id_value_pairs()
        };
        auto const& user_gen_keys_schema_tree{m_kv_pair_log_event->get_user_gen_keys_schema_tree()};
        auto const user_gen_keys_schema_subtree_bitmap_result{
                m_kv_pair_log_event->get_user_gen_keys_schema_subtree_bitmap()
        };
        if (user_gen_keys_schema_subtree_bitmap_result.has_error()) {
            PyErr_Format(
                    PyExc_RuntimeError,
                    "Failed to get user-generated keys schema subtree bitmap: %s",
                    user_gen_keys_schema_subtree_bitmap_result.error().message().c_str()
            );
            return nullptr;
        }
        PyObjectPtr<PyDictObject> const user_gen_kv_pairs_dict{
                PyKeyValuePairLogEvent_internal::serialize_node_id_value_pair_to_py_dict(
                        user_gen_keys_schema_tree,
                        user_gen_keys_schema_subtree_bitmap_result.value(),
                        user_gen_node_id_value_pairs,
                        string_view_to_py_unicode_method
                )
        };
        if (nullptr == user_gen_kv_pairs_dict) {
            return nullptr;
        }

        return PyTuple_Pack(2, auto_gen_kv_pairs_dict.get(), user_gen_kv_pairs_dict.get());
    } catch (clp::TraceableException& ex) {
        handle_traceable_exception(ex);
        return nullptr;
    }
}
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_PYKEYVALUEPAIRLOGEVENT_HPP
