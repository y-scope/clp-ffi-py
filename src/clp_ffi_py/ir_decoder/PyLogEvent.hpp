#ifndef CLP_FFI_PY_PY_LOG_EVENT_HPP
#define CLP_FFI_PY_PY_LOG_EVENT_HPP

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <optional>

#include <clp_ffi_py/ir_decoder/LogEvent.hpp>
#include <clp_ffi_py/ir_decoder/PyMetadata.hpp>

namespace clp_ffi_py::ir_decoder {
/**
 * A PyObject structure functioning as a Python-compatible interface to retrieve
 * a log event. The underlying data is pointed by `log_event`. The object may
 * bind with a PyMetadata object pointed by `Py_metadata` that specifies event
 * metadata such as timestamp format.
 */
struct PyLogEvent {
    PyObject_HEAD;
    LogEvent* log_event;
    PyMetadata* py_metadata;

    /**
     * Initializes the underlying data with the given inputs.
     * Since the memory allocation of PyLogEvent is handled by CPython
     * allocator, cpp constructors will not be explicitly called. This function
     * serves as the default constructor to initialize the underlying metadata.
     * It has to be manually called whenever creating a new PyLogEvent object
     * through CPython APIs.
     * @param log_message
     * @param timestamp
     * @param index
     * @param metadata A PyMetadata instance to bind with the log event (can be
     * nullptr).
     * @param formatted_timestamp Formatted timestamp. This argument is not
     * given by default. It should be given when deserializing the object from
     * a saved state.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error
     * set.
     */
    [[nodiscard]] auto
    init(std::string_view log_message,
         ffi::epoch_time_ms_t timestamp,
         size_t index,
         PyMetadata* metadata,
         std::optional<std::string_view> formatted_timestamp = std::nullopt) -> bool;

    /**
     * Validates whether the PyLogEvent has a PyMetadata object associated.
     * @return Whether `Py_metadata` points to nullptr.
     */
    [[nodiscard]] bool has_metadata() { return nullptr != py_metadata; }

    /**
     * Resets pointers to nullptr.
     */
    auto reset() -> void {
        log_event = nullptr;
        py_metadata = nullptr;
    }

    /**
     * Binds the given PyMetadata and holds a reference. If `Py_metadata` has
     * been set already, decrement the reference to discard the old value.
     * @param metadata
     */
    auto set_metadata(PyMetadata* metadata) -> void {
        Py_XDECREF(py_metadata);
        py_metadata = metadata;
        if (nullptr != metadata) {
            Py_INCREF(py_metadata);
        }
    }

    /**
     * Gets the formatted log message represented by the underlying log event.
     * If a specific timezone is provided, this timezone is used to format the
     * timestamp. If the timezone is not provided (Py_None), the default
     * formatted timestamp from the log event is used. In the case where
     * the log event does not have a cached formatted timestamp, it obtains one
     * using the default timezone from the metadata (if metadata is present),
     * or defaults to UTC.
     * @param timezone Python tzinfo object that specifies a timezone.
     * @return Python string of the formatted log message.
     * @return nullptr on failure with the relevant Python exception and error
     * set.
     */
    [[nodiscard]] auto get_formatted_message(PyObject* timezone = Py_None) -> PyObject*;
};

/**
 * Gets the PyTypeObject that represents PyLogEvent's Python type. This type is
 * dynamically created and initialized during the execution of
 * `PyLogEvent_module_level_init`.
 * @return Python type object associated with PyLogEvent.
 */
auto PyLogEvent_get_PyType() -> PyTypeObject*;

/**
 * Creates and initializes PyLogEvent as a Python type, and then incorporates
 * this type as a Python object into the py_module module.
 * @param py_module This is the Python module where the initialized PyLogEvent
 * will be incorporated.
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
auto PyLogEvent_module_level_init(PyObject* py_module) -> bool;

/**
 * Creates and initializes a new PyLogEvent using the given inputs.
 * @param log_message
 * @param timestamp
 * @param index
 * @param metadata A PyMetadata instance to bind with the log event (can be
 * nullptr).
 * @return a new reference of a PyLogEvent object that is initialized with the
 * given inputs.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
auto PyLogEvent_create_new(
        std::string const& log_message,
        ffi::epoch_time_ms_t timestamp,
        size_t index,
        PyMetadata* metadata) -> PyLogEvent*;
} // namespace clp_ffi_py::ir_decoder
#endif