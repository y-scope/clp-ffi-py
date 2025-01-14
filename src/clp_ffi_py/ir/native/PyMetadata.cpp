#include <wrapped_facade_headers/Python.hpp>

#include "PyMetadata.hpp"

#include <new>
#include <type_traits>

#include <clp/ir/types.hpp>
#include <json/single_include/nlohmann/json.hpp>

#include <clp_ffi_py/api_decoration.hpp>
#include <clp_ffi_py/error_messages.hpp>
#include <clp_ffi_py/ExceptionFFI.hpp>
#include <clp_ffi_py/ir/native/Metadata.hpp>
#include <clp_ffi_py/Py_utils.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyMetadataDoc,
        "This class represents the IR stream preamble and provides ways to access the underlying "
        "metadata. Normally, this class will be instantiated by the FFI IR deserialization "
        "methods.\n"
        "However, with the `__init__` method provided below, direct instantiation is also "
        "possible.\n\n"
        "The signature of `__init__` method is shown as following:\n\n"
        "__init__(self, ref_timestamp, timestamp_format, timezone_id)\n\n"
        "Initializes an object that represents CLP IR metadata. "
        "Assumes `self` is uninitialized and will allocate the underlying memory. If "
        "`self` is already initialized this will result in memory leaks.\n\n"
        ":param ref_timestamp: the reference Unix epoch timestamp in milliseconds used to "
        "calculate the timestamp of the first log message in the IR stream.\n"
        ":param timestamp_format: the timestamp format to be use when generating the logs with a "
        "reader.\n"
        ":param timezone_id: the timezone id to be use when generating the timestamp from Unix "
        "epoch time.\n"
);
CLP_FFI_PY_METHOD auto PyMetadata_init(PyMetadata* self, PyObject* args, PyObject* keywords) -> int;

/**
 * Callback of PyMetadata deallocator.
 * @param self
 */
CLP_FFI_PY_METHOD auto PyMetadata_dealloc(PyMetadata* self) -> void;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyMetadataIsUsingFourByteEncodingDoc,
        "is_using_four_byte_encoding(self)\n"
        "--\n\n"
        "Checks whether the CLP IR is encoded using 4-byte or 8-byte encoding methods.\n\n"
        ":return: True for 4-byte encoding, and False for 8-byte encoding.\n"
);
CLP_FFI_PY_METHOD auto PyMetadata_is_using_four_byte_encoding(PyMetadata* self) -> PyObject*;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyMetadataGetRefTimestampDoc,
        "get_ref_timestamp(self)\n"
        "--\n\n"
        "Gets the reference Unix epoch timestamp in milliseconds used to calculate the timestamp "
        "of the first log message in the IR stream.\n\n"
        ":return: The reference timestamp.\n"
);
CLP_FFI_PY_METHOD auto PyMetadata_get_ref_timestamp(PyMetadata* self) -> PyObject*;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyMetadataGetTimestampFormatDoc,
        "get_timestamp_format(self)\n"
        "--\n\n"
        "Gets the timestamp format to be use when generating the logs with a reader.\n\n"
        ":return: The timestamp format.\n"
);
CLP_FFI_PY_METHOD auto PyMetadata_get_timestamp_format(PyMetadata* self) -> PyObject*;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyMetadataGetTimezoneIdDoc,
        "get_timezone_id(self)\n"
        "--\n\n"
        "Gets the timezone id to be use when generating the timestamp from Unix epoch time.\n\n"
        ":return: The timezone ID in TZID format.\n"
);
CLP_FFI_PY_METHOD auto PyMetadata_get_timezone_id(PyMetadata* self) -> PyObject*;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyMetadataGetTimezoneDoc,
        "get_timezone(self)\n"
        "--\n\n"
        "Gets the timezone represented as tzinfo to be use when generating the timestamp from Unix "
        "epoch time.\n\n"
        ":return: A new reference to the timezone as tzinfo.\n"
);
CLP_FFI_PY_METHOD auto PyMetadata_get_timezone(PyMetadata* self) -> PyObject*;

// NOLINTNEXTLINE(*-avoid-c-arrays, cppcoreguidelines-avoid-non-const-global-variables)
PyMethodDef PyMetadata_method_table[]{
        {"is_using_four_byte_encoding",
         py_c_function_cast(PyMetadata_is_using_four_byte_encoding),
         METH_NOARGS,
         static_cast<char const*>(cPyMetadataIsUsingFourByteEncodingDoc)},

        {"get_ref_timestamp",
         py_c_function_cast(PyMetadata_get_ref_timestamp),
         METH_NOARGS,
         static_cast<char const*>(cPyMetadataGetRefTimestampDoc)},

        {"get_timestamp_format",
         py_c_function_cast(PyMetadata_get_timestamp_format),
         METH_NOARGS,
         static_cast<char const*>(cPyMetadataGetTimestampFormatDoc)},

        {"get_timezone_id",
         py_c_function_cast(PyMetadata_get_timezone_id),
         METH_NOARGS,
         static_cast<char const*>(cPyMetadataGetTimezoneIdDoc)},

        {"get_timezone",
         py_c_function_cast(PyMetadata_get_timezone),
         METH_NOARGS,
         static_cast<char const*>(cPyMetadataGetTimezoneDoc)},

        {nullptr}
};

// NOLINTBEGIN(cppcoreguidelines-pro-type-*-cast)
// NOLINTNEXTLINE(*-avoid-c-arrays, cppcoreguidelines-avoid-non-const-global-variables)
PyType_Slot PyMetadata_slots[]{
        {Py_tp_alloc, reinterpret_cast<void*>(PyType_GenericAlloc)},
        {Py_tp_dealloc, reinterpret_cast<void*>(PyMetadata_dealloc)},
        {Py_tp_init, reinterpret_cast<void*>(PyMetadata_init)},
        {Py_tp_new, reinterpret_cast<void*>(PyType_GenericNew)},
        {Py_tp_methods, static_cast<void*>(PyMetadata_method_table)},
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPyMetadataDoc))},
        {0, nullptr}
};
// NOLINTEND(cppcoreguidelines-pro-type-*-cast)

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
PyType_Spec PyMetadata_type_spec{
        "clp_ffi_py.ir.native.Metadata",
        sizeof(PyMetadata),
        0,
        Py_TPFLAGS_DEFAULT,
        static_cast<PyType_Slot*>(PyMetadata_slots)
};

CLP_FFI_PY_METHOD auto PyMetadata_init(PyMetadata* self, PyObject* args, PyObject* keywords)
        -> int {
    static char keyword_ref_timestamp[]{"ref_timestamp"};
    static char keyword_timestamp_format[]{"timestamp_format"};
    static char keyword_timezone_id[]{"timezone_id"};
    static char* keyword_table[]{
            static_cast<char*>(keyword_ref_timestamp),
            static_cast<char*>(keyword_timestamp_format),
            static_cast<char*>(keyword_timezone_id),
            nullptr
    };

    clp::ir::epoch_time_ms_t ref_timestamp{0};
    char const* input_timestamp_format{nullptr};
    char const* input_timezone{nullptr};

    // If the argument parsing fails, `self` will be deallocated. We must reset all pointers to
    // nullptr in advance, otherwise the deallocator might trigger segmentation fault
    self->default_init();

    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "Lss",
                static_cast<char**>(keyword_table),
                &ref_timestamp,
                &input_timestamp_format,
                &input_timezone
        )))
    {
        return -1;
    }

    auto success{self->init(ref_timestamp, input_timestamp_format, input_timezone)};
    return success ? 0 : -1;
}

CLP_FFI_PY_METHOD auto PyMetadata_dealloc(PyMetadata* self) -> void {
    self->clean();
    PyObject_Del(self);
}

CLP_FFI_PY_METHOD auto PyMetadata_is_using_four_byte_encoding(PyMetadata* self) -> PyObject* {
    if (self->get_metadata()->is_using_four_byte_encoding()) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

CLP_FFI_PY_METHOD auto PyMetadata_get_ref_timestamp(PyMetadata* self) -> PyObject* {
    return PyLong_FromLongLong(self->get_metadata()->get_ref_timestamp());
}

CLP_FFI_PY_METHOD auto PyMetadata_get_timestamp_format(PyMetadata* self) -> PyObject* {
    return PyUnicode_FromString(self->get_metadata()->get_timestamp_format().c_str());
}

CLP_FFI_PY_METHOD auto PyMetadata_get_timezone_id(PyMetadata* self) -> PyObject* {
    return PyUnicode_FromString(self->get_metadata()->get_timezone_id().c_str());
}

CLP_FFI_PY_METHOD auto PyMetadata_get_timezone(PyMetadata* self) -> PyObject* {
    auto* timezone{self->get_py_timezone()};
    if (nullptr == timezone) {
        PyErr_SetString(
                PyExc_RuntimeError,
                get_c_str_from_constexpr_string_view(clp_ffi_py::cTimezoneObjectNotInitialized)
        );
        return nullptr;
    }
    Py_INCREF(timezone);
    return timezone;
}
}  // namespace

auto PyMetadata::get_py_type() -> PyTypeObject* {
    return m_py_type.get();
}

auto PyMetadata::module_level_init(PyObject* py_module) -> bool {
    static_assert(std::is_trivially_destructible<PyMetadata>());
    auto* type{py_reinterpret_cast<PyTypeObject>(PyType_FromSpec(&PyMetadata_type_spec))};
    m_py_type.reset(type);
    if (nullptr == type) {
        return false;
    }
    return add_python_type(get_py_type(), "Metadata", py_module);
}

auto PyMetadata::create_new_from_json(nlohmann::json const& metadata, bool is_four_byte_encoding)
        -> PyMetadata* {
    PyMetadata* self{PyObject_New(PyMetadata, get_py_type())};
    if (nullptr == self) {
        return nullptr;
    }
    self->default_init();
    if (false == self->init(metadata, is_four_byte_encoding)) {
        Py_DECREF(self);
        return nullptr;
    }
    return self;
}

auto PyMetadata::init(
        clp::ir::epoch_time_ms_t ref_timestamp,
        char const* input_timestamp_format,
        char const* input_timezone
) -> bool {
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    m_metadata = new (std::nothrow) Metadata(ref_timestamp, input_timestamp_format, input_timezone);
    if (nullptr == m_metadata) {
        PyErr_SetString(
                PyExc_RuntimeError,
                get_c_str_from_constexpr_string_view(clp_ffi_py::cOutOfMemoryError)
        );
        return false;
    }
    return init_py_timezone();
}

auto PyMetadata::init(nlohmann::json const& metadata, bool is_four_byte_encoding) -> bool {
    try {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        m_metadata = new (std::nothrow) Metadata(metadata, is_four_byte_encoding);
        if (nullptr == m_metadata) {
            PyErr_SetString(
                    PyExc_RuntimeError,
                    get_c_str_from_constexpr_string_view(clp_ffi_py::cOutOfMemoryError)
            );
            return false;
        }
    } catch (clp_ffi_py::ExceptionFFI& ex) {
        handle_traceable_exception(ex);
        m_metadata = nullptr;
        return false;
    }
    return init_py_timezone();
}

auto PyMetadata::init_py_timezone() -> bool {
    m_py_timezone = py_utils_get_timezone_from_timezone_id(m_metadata->get_timezone_id());
    if (nullptr == m_py_timezone) {
        return false;
    }
    Py_INCREF(m_py_timezone);
    return true;
}
}  // namespace clp_ffi_py::ir::native
