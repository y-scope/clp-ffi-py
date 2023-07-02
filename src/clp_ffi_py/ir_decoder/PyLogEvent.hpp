#ifndef CLP_FFI_PY_PY_LOG_EVENT_HPP
#define CLP_FFI_PY_PY_LOG_EVENT_HPP

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <clp_ffi_py/ir_decoder/LogEvent.hpp>
#include <clp_ffi_py/ir_decoder/PyMetadata.hpp>

namespace clp_ffi_py::ir_decoder {
/**
 * A PyObject structure functioning as a Python-compatible interface to retrieve
 * an long event. The underlying data is pointed by `log_event`. The object may
 * bind with a PyMetadata object pointed by `Py_metadata` that specifies event
 * metadata such as timestamp format.
 */
struct PyLogEvent {
    PyObject_HEAD;
    LogEvent* log_event;
    PyMetadata* Py_metadata;

    /**
     * Validates whether the PyLogEvent has a PyMetadata object associated.
     * @return Whether `Py_metadata` points to nullptr.
     */
    [[nodiscard]] bool has_metadata() { return nullptr != Py_metadata; }

    /**
     * Resets pointers to nullptr. Since the memory allocation of PyLogEvent is
     * handled by CPython allocator, cpp constructors cannot be used to assign
     * initial values to the underlying pointers. This function serves as the
     * default constructor to reset the memory region. It has to be manually
     * called whenever receiving a newly created PyLogEvent object from CPython
     * APIs.
     */
    auto reset() -> void {
        log_event = nullptr;
        Py_metadata = nullptr;
    }

    /**
     * Binds the given PyMetadata and holds a reference. If `Py_metadata` has
     * been set already, decrement the reference to discard the old value.
     * @param metadata
     */
    auto set_metadata(PyMetadata* metadata) -> void {
        Py_XDECREF(Py_metadata);
        Py_metadata = metadata;
        if (nullptr != metadata) {
            Py_INCREF(Py_metadata);
        }
    }
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
 * @param new_object_append_list This vector is responsible for appending all
 * successfully created PyObjects during initialization for reference tracking
 * purposes.
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
auto PyLogEvent_module_level_init(
        PyObject* py_module,
        std::vector<PyObject*>& new_object_append_list) -> bool;

/**
 * Creates and initializes a new PyLogEvent using the given inputs.
 * @param log_message
 * @param timestamp
 * @param index
 * @param metadata A PyMetadata to bind with the log event (can be nullptr).
 * @return a new reference of a PyLogEvent object that is initialized with the
 * given inputs.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
auto PyLogEvent_create_new(
        std::string log_message,
        ffi::epoch_time_ms_t timestamp,
        size_t index,
        PyMetadata* metadata) -> PyLogEvent*;

/**
 * Constant keys used to serialize/deserialize PyLogEvent objects through
 * __getstate__ and __setstate__ methods.
 */
constexpr char cStateLogMessage[] = "log_message";
constexpr char cStateTimestamp[] = "timestamp";
constexpr char cStateFormattedTimestamp[] = "formatted_timestamp";
constexpr char cStateIndex[] = "index";
} // namespace clp_ffi_py::ir_decoder
#endif
