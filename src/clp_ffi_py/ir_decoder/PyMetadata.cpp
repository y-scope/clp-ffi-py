#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <clp_ffi_py/ir_decoder/PyMetadata.hpp>

#include <clp_ffi_py/ErrorMessage.hpp>
#include <clp_ffi_py/ExceptionFFI.hpp>
#include <clp_ffi_py/PyObjectPtr.hpp>
#include <clp_ffi_py/Py_utils.hpp>
#include <clp_ffi_py/ir_decoder/Metadata.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir_decoder {
auto PyMetadata::init(
        ffi::epoch_time_ms_t ref_timestamp,
        char const* input_timestamp_format,
        char const* input_timezone) -> bool {
    this->metadata = new Metadata(ref_timestamp, input_timestamp_format, input_timezone);
    if (nullptr == this->metadata) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        return false;
    }
    return this->init_py_timezone();
}

auto PyMetadata::init(nlohmann::json const& metadata, bool is_four_byte_encoding) -> bool {
    this->metadata = new Metadata(metadata, is_four_byte_encoding);
    if (nullptr == this->metadata) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        return false;
    }
    return this->init_py_timezone();
}

auto PyMetadata::init_py_timezone() -> bool {
    assert(this->metadata);
    this->py_timezone = Py_utils_get_timezone_from_timezone_id(this->metadata->get_timezone_id());
    if (nullptr == this->py_timezone) {
        return false;
    }
    Py_INCREF(this->py_timezone);
    return true;
}

extern "C" {
/**
 * PyMetadata `__init__` method:
 * __init__(ref_timestamp, timestamp_format, timezone_id)
 * Keyword argument parsing is supported.
 * @param self
 * @param args
 * @param keywords
 * @return 0 on success.
 * @return -1 on failure  with the relevant Python exception and error set.
 */
static auto PyMetadata_init(PyMetadata* self, PyObject* args, PyObject* keywords) -> int {
    static char keyword_ref_timestamp[]{"ref_timestamp"};
    static char keyword_timestamp_format[]{"timestamp_format"};
    static char keyword_timezone_id[]{"timezone_id"};
    static char* keyword_table[] = {
            static_cast<char*>(keyword_ref_timestamp),
            static_cast<char*>(keyword_timestamp_format),
            static_cast<char*>(keyword_timezone_id),
            nullptr};

    ffi::epoch_time_ms_t ref_timestamp;
    char const* input_timestamp_format;
    char const* input_timezone;

    // If the argument parsing fails, `self` will be deallocated. We must reset
    // all pointers to nullptr in advance, otherwise the deallocator might free
    // trigger segmentation faults.
    self->reset();

    if (false == PyArg_ParseTupleAndKeywords(
                         args,
                         keywords,
                         "Lss",
                         keyword_table,
                         &ref_timestamp,
                         &input_timestamp_format,
                         &input_timezone)) {
        return -1;
    }

    auto success{self->init(ref_timestamp, input_timestamp_format, input_timezone)};
    return success ? 0 : -1;
}

/**
 * PyMetadata deallocator.
 * @param self
 */
static auto PyMetadata_dealloc(PyMetadata* self) -> void {
    delete self->metadata;
    Py_XDECREF(self->py_timezone);
    PyObject_Del(self);
}

PyDoc_STRVAR(
        cPyMetadataIsUsingFourByteEncodingDoc,
        "is_using_four_byte_encoding(self)\n"
        "--\n\n"
        "Checks whether the CLP IR is encoded using four-byte or eight-byte encoding methods.\n"
        ":param self\n"
        ":return: True for four-byte encoding, and False for eight-byte encoding.\n");

static auto PyMetadata_is_using_four_byte_encoding(PyMetadata* self) -> PyObject* {
    assert(self->metadata);
    if (self->metadata->is_using_four_byte_encoding()) {
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

PyDoc_STRVAR(
        cPyMetadataGetRefTimestampDoc,
        "get_ref_timestamp(self)\n"
        "--\n\n"
        "Gets the reference timestamp used to calculate the timestamp of the first log message in "
        "the IR stream.\n"
        ":param self\n"
        ":return: The reference timestamp.\n");

static auto PyMetadata_get_ref_timestamp(PyMetadata* self) -> PyObject* {
    assert(self->metadata);
    return PyLong_FromLongLong(self->metadata->get_ref_timestamp());
}

PyDoc_STRVAR(
        cPyMetadataGetTimestampFormatDoc,
        "get_timestamp_format(self)\n"
        "--\n\n"
        "Gets the timestamp format to be use when generating the logs with a reader.\n"
        ":param self\n"
        ":return: The timestamp format.\n");

static auto PyMetadata_get_timestamp_format(PyMetadata* self) -> PyObject* {
    assert(self->metadata);
    return PyUnicode_FromString(self->metadata->get_timestamp_format().c_str());
}

PyDoc_STRVAR(
        cPyMetadataGetTimezoneIdDoc,
        "get_timezone_id(self)\n"
        "--\n\n"
        "Gets the timezone id to be use when generating the timestamp from Unix epoch time.\n"
        ":param self\n"
        ":return: The timezone in TZID format.\n");

static auto PyMetadata_get_timezone_id(PyMetadata* self) -> PyObject* {
    assert(self->metadata);
    return PyUnicode_FromString(self->metadata->get_timezone_id().c_str());
}
}

/**
 * PyMetadata method table.
 */
static PyMethodDef PyMetadata_method_table[]{
        {"is_using_four_byte_encoding",
         reinterpret_cast<PyCFunction>(PyMetadata_is_using_four_byte_encoding),
         METH_NOARGS,
         cPyMetadataIsUsingFourByteEncodingDoc},

        {"get_ref_timestamp",
         reinterpret_cast<PyCFunction>(PyMetadata_get_ref_timestamp),
         METH_NOARGS,
         cPyMetadataGetRefTimestampDoc},

        {"get_timestamp_format",
         reinterpret_cast<PyCFunction>(PyMetadata_get_timestamp_format),
         METH_NOARGS,
         cPyMetadataGetTimestampFormatDoc},

        {"get_timezone_id",
         reinterpret_cast<PyCFunction>(PyMetadata_get_timezone_id),
         METH_NOARGS,
         cPyMetadataGetTimezoneIdDoc},

        {nullptr}};

/**
 * PyMetadata member table.
 */
static PyMemberDef PyMetadata_members[] = {
        {"timezone",
         T_OBJECT,
         offsetof(PyMetadata, py_timezone),
         READONLY,
         "Read only timezone stored as tzinfo"},

        {nullptr}};

/**
 * PyMetadata Python type slots.
 */
static PyType_Slot PyMetadata_slots[]{
        {Py_tp_dealloc, reinterpret_cast<void*>(PyMetadata_dealloc)},
        {Py_tp_methods, PyMetadata_method_table},
        {Py_tp_init, reinterpret_cast<void*>(PyMetadata_init)},
        {Py_tp_new, reinterpret_cast<void*>(PyType_GenericNew)},
        {Py_tp_members, PyMetadata_members},
        {0, nullptr}};

/**
 * PyMetadata Python type specifications.
 */
static PyType_Spec PyMetadata_type_spec{
        "clp_ffi_py.CLPIRDecoder.Metadata",
        sizeof(PyMetadata),
        0,
        Py_TPFLAGS_DEFAULT,
        PyMetadata_slots};

/**
 * PyMetadata's Python type.
 */
static PyObjectPtr<PyTypeObject> PyMetadata_type;

auto PyMetadata_get_PyType() -> PyTypeObject* {
    return PyMetadata_type.get();
}

auto PyMetadata_init_from_json(nlohmann::json const& metadata, bool is_four_byte_encoding)
        -> PyMetadata* {
    PyMetadata* self{
            reinterpret_cast<PyMetadata*>(PyObject_New(PyMetadata, PyMetadata_get_PyType()))};
    if (nullptr == self) {
        return nullptr;
    }
    self->reset();
    if (false == self->init(metadata, is_four_byte_encoding)) {
        Py_DECREF(self);
        return nullptr;
    }
    return self;
}

auto PyMetadata_module_level_init(PyObject* py_module) -> bool {
    auto type{reinterpret_cast<PyTypeObject*>(PyType_FromSpec(&PyMetadata_type_spec))};
    PyMetadata_type.reset(type);
    if (nullptr == type) {
        return false;
    }
    return add_type(PyMetadata_get_PyType(), "Metadata", py_module);
}
}; // namespace clp_ffi_py::ir_decoder
