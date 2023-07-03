#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <clp_ffi_py/ir_decoder/PyLogEvent.hpp>

#include <clp_ffi_py/ErrorMessage.hpp>
#include <clp_ffi_py/PyObjectPtr.hpp>
#include <clp_ffi_py/Py_utils.hpp>
#include <clp_ffi_py/ir_decoder/LogEvent.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir_decoder {
auto PyLogEvent::get_formatted_message(PyObject* timezone) -> PyObject* {
    auto cache_formatted_timestamp{false};
    if (Py_None == timezone) {
        if (this->log_event->has_formatted_timestamp()) {
            // If the formatted timestamp exists, it constructs the raw message
            // without calling python level format function
            return PyUnicode_FromFormat(
                    "%s%s",
                    this->log_event->get_formatted_timestamp().c_str(),
                    this->log_event->get_log_message().c_str());
        } else if (this->has_metadata()) {
            timezone = this->py_metadata->py_timezone;
            cache_formatted_timestamp = true;
        }
    }

    PyObjectPtr<PyObject> formatted_timestamp_object{
            Py_utils_get_formatted_timestamp(this->log_event->get_timestamp(), timezone)};
    auto formatted_timestamp_ptr{formatted_timestamp_object.get()};
    if (nullptr == formatted_timestamp_ptr) {
        return nullptr;
    }
    std::string formatted_timestamp;
    if (false == parse_PyString(formatted_timestamp_ptr, formatted_timestamp)) {
        return nullptr;
    }

    if (cache_formatted_timestamp) {
        this->log_event->set_formatted_timestamp(formatted_timestamp);
    }
    return PyUnicode_FromFormat(
            "%s%s",
            formatted_timestamp.c_str(),
            this->log_event->get_log_message().c_str());
}

auto PyLogEvent::init(
        std::string_view log_message,
        ffi::epoch_time_ms_t timestamp,
        size_t index,
        PyMetadata* metadata,
        std::optional<std::string_view> formatted_timestamp) -> bool {
    this->log_event = new LogEvent(log_message, timestamp, index, formatted_timestamp);
    if (nullptr == this->log_event) {
        PyErr_SetString(PyExc_RuntimeError, clp_ffi_py::error_messages::out_of_memory_error);
        return false;
    }
    this->set_metadata(metadata);
    return true;
}

extern "C" {
/**
 * Callback of PyLogEvent `__init__` method:
 * __init__(log_message, timestamp, index=0, metadata=None)
 * Keyword argument parsing is supported.
 * @param self
 * @param args
 * @param keywords
 * @return 0 on success.
 * @return -1 on failure with the relevant Python exception and error set.
 */
static auto PyLogEvent_init(PyLogEvent* self, PyObject* args, PyObject* keywords) -> int {
    static char keyword_message[] = "log_message";
    static char keyword_timestamp[] = "timestamp";
    static char keyword_message_idx[] = "index";
    static char keyword_metadata[] = "metadata";
    static char* keyword_table[]{
            static_cast<char*>(keyword_message),
            static_cast<char*>(keyword_timestamp),
            static_cast<char*>(keyword_message_idx),
            static_cast<char*>(keyword_metadata),
            nullptr};

    // If the argument parsing fails, `self` will be deallocated. We must reset
    // all pointers to nullptr in advance, otherwise the deallocator might
    // trigger segmentation fault
    self->reset();

    char const* log_message{nullptr};
    ffi::epoch_time_ms_t timestamp{0};
    size_t index{0};
    PyObject* metadata{Py_None};
    if (false == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                         args,
                         keywords,
                         "sL|KO",
                         keyword_table,
                         &log_message,
                         &timestamp,
                         &index,
                         &metadata))) {
        return -1;
    }

    auto const has_metadata{Py_None != metadata};
    if (has_metadata && false == PyObject_TypeCheck(metadata, PyMetadata_get_PyType())) {
        PyErr_SetString(PyExc_TypeError, clp_ffi_py::error_messages::py_type_error);
        return -1;
    }

    if (false == self->init(
                         log_message,
                         timestamp,
                         index,
                         has_metadata ? reinterpret_cast<PyMetadata*>(metadata) : nullptr)) {
        return -1;
    }
    return 0;
}

/**
 * Callback of PyLogEvent deallocator.
 * @param self
 */
static auto PyLogEvent_dealloc(PyLogEvent* self) -> int {
    self->set_metadata(nullptr);
    delete self->log_event;
    PyObject_Del(self);
}

/**
 * Constant keys used to serialize/deserialize PyLogEvent objects through
 * __getstate__ and __setstate__ methods.
 */
static constexpr char cStateLogMessage[] = "log_message";
static constexpr char cStateTimestamp[] = "timestamp";
static constexpr char cStateFormattedTimestamp[] = "formatted_timestamp";
static constexpr char cStateIndex[] = "index";

PyDoc_STRVAR(
        cPyLogEventGetStateDoc,
        "__getstate__(self)\n"
        "--\n\n"
        "Serializes the log event.\n"
        ":param self\n"
        ":return: Serialized log event in a Python dictionary.\n");

/**
 * Callback of PyLogEvent `__getstate__` method.
 * @param self
 * @return Python dictionary that contains the serialized object.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
static auto PyLogEvent_getstate(PyLogEvent* self) -> PyObject* {
    assert(self->log_event);
    if (false == self->log_event->has_formatted_timestamp()) {
        PyObjectPtr<PyObject> formatted_timestamp_object{Py_utils_get_formatted_timestamp(
                self->log_event->get_timestamp(),
                self->has_metadata() ? self->py_metadata->py_timezone : Py_None)};
        auto formatted_timestamp_ptr{formatted_timestamp_object.get()};
        if (nullptr == formatted_timestamp_ptr) {
            return nullptr;
        }
        std::string formatted_timestamp;
        if (false == parse_PyString(formatted_timestamp_ptr, formatted_timestamp)) {
            return nullptr;
        }
        self->log_event->set_formatted_timestamp(formatted_timestamp);
    }

    return Py_BuildValue(
            "{sssssLsK}",
            cStateLogMessage,
            self->log_event->get_log_message().c_str(),
            cStateFormattedTimestamp,
            self->log_event->get_formatted_timestamp().c_str(),
            cStateTimestamp,
            self->log_event->get_timestamp(),
            cStateIndex,
            self->log_event->get_index());
}

PyDoc_STRVAR(
        cPyLogEventSetStateDoc,
        "__setstate__(self, state)\n"
        "--\n\n"
        "Deserializes the log event from a state dictionary.\n"
        ":param self\n"
        ":param state: Serialized log event represented by a Python dictionary.\n"
        ":return: None\n");

/**
 * Callback of PyLogEvent `__setstate__` method.
 * @param self
 * @param state Python dictionary that contains the serialized object info.
 * @return Py_None on success
 * @return nullptr on failure with the relevant Python exception and error set.
 */
static auto PyLogEvent_setstate(PyLogEvent* self, PyObject* state) -> PyObject* {
    self->reset();

    if (false == PyDict_CheckExact(state)) {
        PyErr_SetString(PyExc_ValueError, clp_ffi_py::error_messages::setstate_input_error);
        return nullptr;
    }

    auto* log_message_obj{PyDict_GetItemString(state, cStateLogMessage)};
    if (nullptr == log_message_obj) {
        PyErr_Format(
                PyExc_KeyError,
                clp_ffi_py::error_messages::setstate_key_error_template,
                cStateLogMessage);
        return nullptr;
    }
    std::string log_message;
    if (false == parse_PyString(log_message_obj, log_message)) {
        return nullptr;
    }

    auto* formatted_timestamp_obj{PyDict_GetItemString(state, cStateFormattedTimestamp)};
    if (nullptr == formatted_timestamp_obj) {
        PyErr_Format(
                PyExc_KeyError,
                clp_ffi_py::error_messages::setstate_key_error_template,
                cStateFormattedTimestamp);
        return nullptr;
    }
    std::string formatted_timestamp;
    if (false == parse_PyString(formatted_timestamp_obj, formatted_timestamp)) {
        return nullptr;
    }

    auto* timestamp_obj{PyDict_GetItemString(state, cStateTimestamp)};
    if (nullptr == timestamp_obj) {
        PyErr_Format(
                PyExc_KeyError,
                clp_ffi_py::error_messages::setstate_key_error_template,
                cStateTimestamp);
        return nullptr;
    }
    ffi::epoch_time_ms_t timestamp;
    if (false == parse_PyInt<ffi::epoch_time_ms_t>(timestamp_obj, timestamp)) {
        return nullptr;
    }

    auto* index_obj{PyDict_GetItemString(state, cStateIndex)};
    if (nullptr == index_obj) {
        PyErr_Format(
                PyExc_KeyError,
                clp_ffi_py::error_messages::setstate_key_error_template,
                cStateIndex);
        return nullptr;
    }
    size_t index;
    if (false == parse_PyInt<size_t>(index_obj, index)) {
        return nullptr;
    }

    if (false == self->init(log_message, timestamp, index, nullptr, formatted_timestamp)) {
        return nullptr;
    }

    Py_RETURN_NONE;
}

/**
 * Callback of PyLogEvent `__str__` method.
 * @param self
 * @return PyLogEvent::get_formatted_log_message
 */
static auto PyLogEvent_str(PyLogEvent* self) -> PyObject* {
    return self->get_formatted_message();
}

/**
 * Callback of PyLogEvent `__repr__` method.
 * @param self
 * @return Python string representation of PyLogEvent state.
 */
static auto PyLogEvent_repr(PyLogEvent* self) -> PyObject* {
    return PyObject_Repr(PyLogEvent_getstate(self));
}

PyDoc_STRVAR(
        cPyLogEventGetLogMessageDoc,
        "get_log_message(self)\n"
        "--\n\n"
        "Gets the log message of the log event.\n"
        ":param self\n"
        ":return: The log message.\n");

static auto PyLogEvent_get_log_message(PyLogEvent* self) -> PyObject* {
    assert(self->log_event);
    return PyUnicode_FromString(self->log_event->get_log_message().c_str());
}

PyDoc_STRVAR(
        cPyLogEventGetTimestampDoc,
        "get_timestamp(self)\n"
        "--\n\n"
        "Gets the Unix epoch timestamp in milliseconds of the log event.\n"
        ":param self\n"
        ":return: The timestamp in milliseconds.\n");

static auto PyLogEvent_get_timestamp(PyLogEvent* self) -> PyObject* {
    assert(self->log_event);
    return PyLong_FromLongLong(self->log_event->get_timestamp());
}

PyDoc_STRVAR(
        cPyLogEventGetIndexDoc,
        "get_index(self)\n"
        "--\n\n"
        "Gets the message index (relative to the source CLP IR stream) of the log event. This "
        "message is set to 0 by default.\n"
        ":param self\n"
        ":return: The log event index.\n");

static auto PyLogEvent_get_index(PyLogEvent* self) -> PyObject* {
    assert(self->log_event);
    return PyLong_FromLongLong(self->log_event->get_index());
}

PyDoc_STRVAR(
        cPyLogEventGetFormattedMessageDoc,
        "get_formatted_message(self, timezone=None)\n"
        "--\n\n"
        "Gets the formatted log message of the log event.\n"
        "If a specific timezone is provided, it will be used to format the timestamp. Otherwise, "
        "the timestamp will be formatted using the timezone from the originating CLP IR stream.\n"
        ":param self\n"
        ":param timezone: Python tzinfo object that specifies a timezone."
        ":return: The formatted message.\n");

static auto PyLogEvent_get_formatted_message(PyLogEvent* self, PyObject* args, PyObject* keywords)
        -> PyObject* {
    static char keyword_timezone[] = "timezone";
    static char* key_table[] = {static_cast<char*>(keyword_timezone), nullptr};

    PyObject* timezone{Py_None};
    if (false == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                         args,
                         keywords,
                         "|O",
                         static_cast<char**>(key_table),
                         &timezone))) {
        return nullptr;
    }

    return self->get_formatted_message(timezone);
}
}

/**
 * PyLogEvent method table.
 */
static PyMethodDef PyLogEvent_method_table[]{
        {"get_log_message",
         reinterpret_cast<PyCFunction>(PyLogEvent_get_log_message),
         METH_NOARGS,
         static_cast<char const*>(cPyLogEventGetLogMessageDoc)},

        {"get_timestamp",
         reinterpret_cast<PyCFunction>(PyLogEvent_get_timestamp),
         METH_NOARGS,
         static_cast<char const*>(cPyLogEventGetTimestampDoc)},

        {"get_index",
         reinterpret_cast<PyCFunction>(PyLogEvent_get_index),
         METH_NOARGS,
         static_cast<char const*>(cPyLogEventGetIndexDoc)},

        {"get_formatted_message",
         reinterpret_cast<PyCFunction>(PyLogEvent_get_formatted_message),
         METH_KEYWORDS | METH_VARARGS,
         static_cast<char const*>(cPyLogEventGetFormattedMessageDoc)},

        {"__getstate__",
         reinterpret_cast<PyCFunction>(PyLogEvent_getstate),
         METH_NOARGS,
         cPyLogEventGetStateDoc},

        {"__setstate__",
         reinterpret_cast<PyCFunction>(PyLogEvent_setstate),
         METH_O,
         cPyLogEventSetStateDoc},

        {nullptr}};

/**
 * PyLogEvent Python type slots.
 */
static PyType_Slot PyLogEvent_slots[]{
        {Py_tp_dealloc, reinterpret_cast<void*>(PyLogEvent_dealloc)},
        {Py_tp_new, reinterpret_cast<void*>(PyType_GenericNew)},
        {Py_tp_init, reinterpret_cast<void*>(PyLogEvent_init)},
        {Py_tp_str, reinterpret_cast<void*>(PyLogEvent_str)},
        {Py_tp_repr, reinterpret_cast<void*>(PyLogEvent_repr)},
        {Py_tp_methods, static_cast<void*>(PyLogEvent_method_table)},
        {0, nullptr}};

/**
 * PyLogEvent Python type specifications.
 */
static PyType_Spec PyLogEvent_type_spec{
        "clp_ffi_py.CLPIRDecoder.LogEvent",
        sizeof(PyLogEvent),
        0,
        Py_TPFLAGS_DEFAULT,
        PyLogEvent_slots};

/**
 * PyLogEvent's Python type.
 */
static PyObjectPtr<PyTypeObject> PyLogEvent_type;

auto PyLogEvent_get_PyType() -> PyTypeObject* {
    return PyLogEvent_type.get();
}

auto PyLogEvent_module_level_init(PyObject* py_module) -> bool {
    auto* type{reinterpret_cast<PyTypeObject*>(PyType_FromSpec(&PyLogEvent_type_spec))};
    PyLogEvent_type.reset(type);
    if (nullptr == type) {
        return false;
    }
    return add_type(PyLogEvent_get_PyType(), "LogEvent", py_module);
}

auto PyLogEvent_create_new(
        std::string log_message,
        ffi::epoch_time_ms_t timestamp,
        size_t index,
        PyMetadata* metadata) -> PyLogEvent* {
    PyLogEvent* self{
            reinterpret_cast<PyLogEvent*>(PyObject_New(PyLogEvent, PyLogEvent_get_PyType()))};
    if (nullptr == self) {
        PyErr_SetString(PyExc_MemoryError, clp_ffi_py::error_messages::out_of_memory_error);
        return nullptr;
    }
    self->reset();
    if (false == self->init(log_message, timestamp, index, metadata)) {
        return nullptr;
    }
    return self;
}
} // namespace clp_ffi_py::ir_decoder
