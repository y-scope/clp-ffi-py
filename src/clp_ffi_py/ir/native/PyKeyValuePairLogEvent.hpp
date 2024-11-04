#ifndef CLP_FFI_PY_IR_NATIVE_PYKEYVALUEPAIRLOGEVENT_HPP
#define CLP_FFI_PY_IR_NATIVE_PYKEYVALUEPAIRLOGEVENT_HPP

#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <gsl/gsl>

#include <clp/ffi/KeyValuePairLogEvent.hpp>

#include <clp_ffi_py/PyObjectUtils.hpp>

namespace clp_ffi_py::ir::native {
/**
 * A PyObject structure functioning as a Python-compatible interface to retrieve a key-value pair
 * log event. The underlying data is pointed by `m_kv_pair_log_event`.
 */
class PyKeyValuePairLogEvent {
public:
    /**
     * Initializes the underlying data with the given inputs. Since the memory allocation of
     * `PyKeyValuePairLogEvent` is handled by CPython's allocator, cpp constructors will not be
     * explicitly called. This function serves as the default constructor initialize the underlying
     * key-value pair log event. It has to be manually called whenever creating a new
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
        return m_kv_pair_log_event;
    }

    /**
     * Gets the `PyTypeObject` that represents `PyKeyValuePair`'s Python type. This type is
     * dynamically created and initialized during the execution of `module_level_init`.
     * @return Python type object associated with `PyKeyValuePairLogEvent`.
     */
    [[nodiscard]] static auto get_py_type() -> PyTypeObject*;

    /**
     * Creates and initializes `PyKeyValuePairLogEvent` as a Python type, and then incorporates this
     * type as a Python object into the py_module module.
     * @param py_module This is the Python module where the initialized `PyKeyValuePairLogEvent`
     * will be incorporated.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto module_level_init(PyObject* py_module) -> bool;

private:
    PyObject_HEAD;

    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    gsl::owner<clp::ffi::KeyValuePairLogEvent*> m_kv_pair_log_event;

    static inline PyObjectStaticPtr<PyTypeObject> m_py_type{nullptr};
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_PYKEYVALUEPAIRLOGEVENT_HPP
