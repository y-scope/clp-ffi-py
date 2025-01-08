#ifndef CLP_FFI_PY_IR_NATIVE_PYKEYVALUEPAIRLOGEVENT_HPP
#define CLP_FFI_PY_IR_NATIVE_PYKEYVALUEPAIRLOGEVENT_HPP

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include <clp/ffi/KeyValuePairLogEvent.hpp>
#include <gsl/gsl>

#include <clp_ffi_py/PyObjectUtils.hpp>

namespace clp_ffi_py::ir::native {
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
     * Converts the underlying key-value pair log event into a Python dictionary.
     * @return A new reference to the created dictionary on success.
     * @return nullptr on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto to_dict() -> PyDictObject*;

private:
    static inline PyObjectStaticPtr<PyTypeObject> m_py_type{nullptr};

    // Variables
    PyObject_HEAD;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    gsl::owner<clp::ffi::KeyValuePairLogEvent*> m_kv_pair_log_event;
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_PYKEYVALUEPAIRLOGEVENT_HPP
