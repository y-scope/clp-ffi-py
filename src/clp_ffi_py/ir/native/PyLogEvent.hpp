#ifndef CLP_FFI_PY_IR_NATIVE_PYLOGEVENT_HPP
#define CLP_FFI_PY_IR_NATIVE_PYLOGEVENT_HPP

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include <cstddef>
#include <optional>
#include <string_view>

#include <clp/ir/types.hpp>

#include <clp_ffi_py/ir/native/LogEvent.hpp>
#include <clp_ffi_py/ir/native/PyMetadata.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>

namespace clp_ffi_py::ir::native {
/**
 * A PyObject structure functioning as a Python-compatible interface to retrieve a log event. The
 * underlying data is pointed to by `m_log_event`. The object may reference a PyMetadata object
 * pointed to by `m_py_metadata` that specifies the event's metadata, such as timestamp format, from
 * the preamble.
 */
class PyLogEvent {
public:
    // Static methods
    /**
     * Gets the PyTypeObject that represents PyLogEvent's Python type. This type is dynamically
     * created and initialized during the execution of `PyLogEvent::module_level_init`.
     * @return Python type object associated with PyLogEvent.
     */
    [[nodiscard]] static auto get_py_type() -> PyTypeObject*;

    /**
     * Creates and initializes PyLogEvent as a Python type, and then incorporates this type as a
     * Python object into the py_module module.
     * @param py_module This is the Python module where the initialized PyLogEvent will be
     * incorporated.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto module_level_init(PyObject* py_module) -> bool;

    /**
     * Creates and initializes a new PyLogEvent using the given inputs.
     * @param log_message
     * @param timestamp
     * @param index
     * @param metadata A PyMetadata instance to bind with the log event (can be nullptr).
     * @return a new reference of a PyLogEvent object that is initialized with the given inputs.
     * @return nullptr on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto create_new_log_event(
            std::string_view log_message,
            clp::ir::epoch_time_ms_t timestamp,
            size_t index,
            PyMetadata* metadata
    ) -> PyLogEvent*;

    // Delete default constructor to disable direct instantiation.
    PyLogEvent() = delete;

    // Delete copy & move constructors and assignment operators
    PyLogEvent(PyLogEvent const&) = delete;
    PyLogEvent(PyLogEvent&&) = delete;
    auto operator=(PyLogEvent const&) -> PyLogEvent& = delete;
    auto operator=(PyLogEvent&&) -> PyLogEvent& = delete;

    // Destructor
    ~PyLogEvent() = default;

    // Methods
    /**
     * Initializes the underlying data with the given inputs. Since the memory allocation of
     * PyLogEvent is handled by CPython's allocator, cpp constructors will not be explicitly called.
     * This function serves as the default constructor to initialize the underlying metadata. It has
     * to be manually called whenever creating a new PyLogEvent object through CPython APIs.
     * @param log_message
     * @param timestamp
     * @param index
     * @param metadata A PyMetadata instance to bind with the log event (can be nullptr).
     * @param formatted_timestamp Formatted timestamp. This argument is not given by default. It
     * should be given when deserializing the object from a saved state.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto init(
            std::string_view log_message,
            clp::ir::epoch_time_ms_t timestamp,
            size_t index,
            PyMetadata* metadata,
            std::optional<std::string_view> formatted_timestamp = std::nullopt
    ) -> bool;

    /**
     * Validates whether the PyLogEvent has a PyMetadata object associated.
     * @return Whether `Py_metadata` points to nullptr.
     */
    [[nodiscard]] auto has_metadata() -> bool { return nullptr != m_py_metadata; }

    /**
     * Initializes the pointers to nullptr by default. Should be called once the object is
     * allocated.
     */
    auto default_init() -> void {
        m_log_event = nullptr;
        m_py_metadata = nullptr;
    }

    /**
     * Releases the memory allocated for underlying metadata field and the reference hold for the
     * Python object(s).
     */
    auto clean() -> void {
        Py_XDECREF(m_py_metadata);
        delete m_log_event;
    }

    /**
     * Binds the given PyMetadata and holds a reference. If `Py_metadata` has been set already,
     * decrement the reference to discard the old value.
     * @param metadata
     */
    auto set_metadata(PyMetadata* metadata) -> void {
        Py_XDECREF(m_py_metadata);
        m_py_metadata = metadata;
        if (nullptr != metadata) {
            Py_INCREF(m_py_metadata);
        }
    }

    /**
     * Gets the formatted log message represented by the underlying log event. If a specific
     * timezone is provided, this timezone is used to format the timestamp. If the timezone is not
     * provided (Py_None), the default formatted timestamp from the log event is used. In the case
     * where the log event does not have a cached formatted timestamp, it obtains one using the
     * default timezone from the metadata (if metadata is present), or defaults to UTC.
     * @param timezone Python tzinfo object that specifies a timezone.
     * @return Python string of the formatted log message.
     * @return nullptr on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto get_formatted_message(PyObject* timezone = Py_None) -> PyObject*;

    [[nodiscard]] auto get_log_event() -> LogEvent* { return m_log_event; }

    [[nodiscard]] auto get_py_metadata() -> PyMetadata* { return m_py_metadata; }

private:
    static inline PyObjectStaticPtr<PyTypeObject> m_py_type{nullptr};

    PyObject_HEAD;
    LogEvent* m_log_event;
    PyMetadata* m_py_metadata;
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_PYLOGEVENT_HPP
