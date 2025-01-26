#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include "PyKeyValuePairLogEvent.hpp"

#include <cstddef>
#include <new>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

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
            [[maybe_unused]] bool is_auto_generated,
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
 * Callback of `PyKeyValuePairLogEvent`'s `__init__` method.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyKeyValuePairLogEventDoc,
        "This class represents a key-value pair log event and provides methods to access the"
        " key-value pairs. This class is designed to be instantiated by the IR deserializer."
        " However, direct instantiation using the `__init__` method is also supported for testing"
        " purposes, although this may not be as efficient as emission from the IR deserializer.\n\n"
        "__init__(self, auto_gen_kv_pairs, user_gen_kv_pairs)\n\n"
        "Initializes a :class:`KeyValuePairLogEvent` from the given Python dictionary. Note that"
        " each object should only be initialized once. Double initialization will result in a"
        " memory leak.\n\n"
        ":param auto_gen_kv_pairs: A dictionary representing the auto-generated key-value pairs of"
        " the given log event, where all keys must be strings, including keys inside any"
        " sub-dictionaries.\n"
        ":type auto_gen_kv_pairs: dict[str, Any]\n"
        ":param user_gen_kv_pairs: A dictionary representing the user-generated key-value pairs of"
        " the given log event, where all keys must be strings, including keys inside any"
        " sub-dictionaries.\n"
        ":type user_gen_kv_pairs: dict[str, Any]\n"
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
        "to_dict(self, encoding=\"utf-8\", errors=\"strict\")\n"
        "--\n\n"
        "Converts the log event into Python dictionaries.\n\n"
        "The `encoding` and `errors` parameters have the same meaning as the parameters of the same"
        " name in Python's `str()` built-in function, allowing users to custom the behaviour of"
        " converting C strings into Python Unicode objects.\n\n"
        ":param encoding: The encoding type used to convert C strings into Python Unicode objects."
        " See `Python Standard Encodings documentation"
        " <https://docs.python.org/3/library/codecs.html#standard-encodings>`_ for supported"
        " encodings.\n"
        ":type encoding: str\n"
        ":param errors: Specifies how decoding errors are handled. See `Python Error Handlers"
        " documentation <https://docs.python.org/3/library/codecs.html#error-handlers>`_ for"
        " available options.\n"
        ":type errors: str\n"
        ":return: A tuple of Python dictionaries:\n\n"
        "   - A dictionary for auto-generated key-value pairs.\n"
        "   - A dictionary for user-generated key-value pairs.\n"
        ":rtype: tuple[dict[str, Any], dict[str, Any]]\n"
);
CLP_FFI_PY_METHOD auto
PyKeyValuePairLogEvent_to_dict(PyKeyValuePairLogEvent* self, PyObject* args, PyObject* keywords)
        -> PyObject*;

/**
 * Callback of `PyKeyValuePairLogEvent`'s deallocator.
 */
CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_dealloc(PyKeyValuePairLogEvent* self) -> void;

// NOLINTNEXTLINE(*-avoid-c-arrays, cppcoreguidelines-avoid-non-const-global-variables)
PyMethodDef PyKeyValuePairLogEvent_method_table[]{
        {"to_dict",
         py_c_function_cast(PyKeyValuePairLogEvent_to_dict),
         METH_VARARGS | METH_KEYWORDS,
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
 * @param py_auto_gen_kv_pairs_dict
 * @param py_user_gen_kv_pairs_dict
 * @return The converted key-value log event of the given dictionary on success.
 * @return std::nullopt on failure with the relevant Python exception and error set.
 */
[[nodiscard]] auto convert_py_dict_to_key_value_pair_log_event(
        PyDictObject* py_auto_gen_kv_pairs_dict,
        PyDictObject* py_user_gen_kv_pairs_dict
) -> std::optional<clp::ffi::KeyValuePairLogEvent>;

CLP_FFI_PY_METHOD auto
PyKeyValuePairLogEvent_init(PyKeyValuePairLogEvent* self, PyObject* args, PyObject* keywords)
        -> int {
    static char keyword_auto_gen_kv_pairs[]{"auto_gen_kv_pairs"};
    static char keyword_user_gen_kv_pairs[]{"user_gen_kv_pairs"};
    static char* keyword_table[]{
            static_cast<char*>(keyword_auto_gen_kv_pairs),
            static_cast<char*>(keyword_user_gen_kv_pairs),
            nullptr
    };

    // If the argument parsing fails, `self` will be deallocated. We must reset all pointers to
    // nullptr in advance, otherwise the deallocator might trigger segmentation fault.
    self->default_init();

    PyObject* py_auto_gen_kv_pairs{Py_None};
    PyObject* py_user_gen_kv_pairs{Py_None};
    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "OO",
                static_cast<char**>(keyword_table),
                &py_auto_gen_kv_pairs,
                &py_user_gen_kv_pairs
        )))
    {
        return -1;
    }

    if (false == static_cast<bool>(PyDict_Check(py_auto_gen_kv_pairs))) {
        PyErr_SetString(PyExc_TypeError, "`auto_gen_kv_pairs` must be a Python dictionary object");
        return -1;
    }
    if (false == static_cast<bool>(PyDict_Check(py_user_gen_kv_pairs))) {
        PyErr_SetString(PyExc_TypeError, "`user_gen_kv_pairs` must be a Python dictionary object");
        return -1;
    }

    auto optional_kv_pair_log_event{convert_py_dict_to_key_value_pair_log_event(
            py_reinterpret_cast<PyDictObject>(py_auto_gen_kv_pairs),
            py_reinterpret_cast<PyDictObject>(py_user_gen_kv_pairs)
    )};
    if (false == optional_kv_pair_log_event.has_value()) {
        return -1;
    }

    return self->init(std::move(optional_kv_pair_log_event.value())) ? 0 : -1;
}

CLP_FFI_PY_METHOD auto
PyKeyValuePairLogEvent_to_dict(PyKeyValuePairLogEvent* self, PyObject* args, PyObject* keywords)
        -> PyObject* {
    static char keyword_encoding[]{"encoding"};
    static char keyword_errors[]{"errors"};
    static char* keyword_table[]{
            static_cast<char*>(keyword_encoding),
            static_cast<char*>(keyword_errors),
            nullptr
    };

    constexpr std::string_view cDefaultEncoding{"utf-8"};
    constexpr std::string_view cDefaultErrors{"strict"};

    char const* encoding_c_str{cDefaultEncoding.data()};
    Py_ssize_t encoding_size{static_cast<Py_ssize_t>(cDefaultEncoding.size())};
    char const* errors_c_str{cDefaultErrors.data()};
    Py_ssize_t errors_size{static_cast<Py_ssize_t>(cDefaultErrors.size())};
    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "|s#s#",
                static_cast<char**>(keyword_table),
                &encoding_c_str,
                &encoding_size,
                &errors_c_str,
                &errors_size
        )))
    {
        return nullptr;
    }

    if (cDefaultEncoding != std::string_view{encoding_c_str, static_cast<size_t>(encoding_size)}) {
        // The default encoding is not used
        return self->to_dict([&](std::string_view sv) -> PyObject* {
            return PyUnicode_Decode(
                    sv.data(),
                    static_cast<Py_ssize_t>(sv.size()),
                    encoding_c_str,
                    errors_c_str
            );
        });
    }

    if (cDefaultErrors != std::string_view{errors_c_str, static_cast<size_t>(errors_size)}) {
        // The default encoding is used, but not the default error handling
        return self->to_dict([&](std::string_view sv) -> PyObject* {
            return PyUnicode_DecodeUTF8(
                    sv.data(),
                    static_cast<Py_ssize_t>(sv.size()),
                    errors_c_str
            );
        });
    }

    return self->to_dict([](std::string_view sv) -> PyObject* {
        return PyUnicode_FromStringAndSize(sv.data(), static_cast<Py_ssize_t>(sv.size()));
    });
}

CLP_FFI_PY_METHOD auto PyKeyValuePairLogEvent_dealloc(PyKeyValuePairLogEvent* self) -> void {
    self->clean();
    Py_TYPE(self)->tp_free(py_reinterpret_cast<PyObject>(self));
}

auto convert_py_dict_to_key_value_pair_log_event(
        PyDictObject* py_auto_gen_kv_pairs_dict,
        PyDictObject* py_user_gen_kv_pairs_dict
) -> std::optional<clp::ffi::KeyValuePairLogEvent> {
    PyObjectPtr<PyBytesObject> const msgpack_serialized_auto_gen_kv_pairs{
            py_utils_serialize_dict_to_msgpack(py_auto_gen_kv_pairs_dict)
    };
    if (nullptr == msgpack_serialized_auto_gen_kv_pairs) {
        return std::nullopt;
    }
    PyObjectPtr<PyBytesObject> const msgpack_serialized_user_gen_kv_pairs{
            py_utils_serialize_dict_to_msgpack(py_user_gen_kv_pairs_dict)
    };
    if (nullptr == msgpack_serialized_user_gen_kv_pairs) {
        return std::nullopt;
    }

    // Since the type is already checked, we can use the macro to avoid duplicated type checking.
    auto const optional_auto_gen_msgpack_map_handle{unpack_msgpack_map(
            {PyBytes_AS_STRING(msgpack_serialized_auto_gen_kv_pairs.get()),
             static_cast<size_t>(PyBytes_GET_SIZE(msgpack_serialized_auto_gen_kv_pairs.get()))}
    )};
    if (false == optional_auto_gen_msgpack_map_handle.has_value()) {
        return std::nullopt;
    }
    auto const optional_user_gen_msgpack_map_handle{unpack_msgpack_map(
            {PyBytes_AS_STRING(msgpack_serialized_user_gen_kv_pairs.get()),
             static_cast<size_t>(PyBytes_GET_SIZE(msgpack_serialized_user_gen_kv_pairs.get()))}
    )};
    if (false == optional_user_gen_msgpack_map_handle.has_value()) {
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
    // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
    if (false
        == serializer.serialize_msgpack_map(
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
}  // namespace

namespace PyKeyValuePairLogEvent_internal {
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
}  // namespace PyKeyValuePairLogEvent_internal

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
}  // namespace clp_ffi_py::ir::native
