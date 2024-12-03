#ifndef CLP_FFI_PY_PYEXCEPTIONCONTEXT_HPP
#define CLP_FFI_PY_PYEXCEPTIONCONTEXT_HPP

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

namespace clp_ffi_py {
/**
 * Class to get/set Python exception context, designed to capture the current exception state upon
 * instantiation and provide a restore API that allows users to reinstate the exception context when
 * needed. Doc: https://docs.python.org/3/c-api/exceptions.html#c.PyErr_Fetch
 */
class PyExceptionContext {
public:
    // Constructor
    /**
     * Constructs the context by fetching the current raised exception (if any).
     */
    PyExceptionContext() { PyErr_Fetch(&m_type, &m_value, &m_traceback); }

    // Delete copy/move constructors and assignments
    PyExceptionContext(PyExceptionContext const&) = delete;
    PyExceptionContext(PyExceptionContext&&) = delete;
    auto operator=(PyExceptionContext const&) -> PyExceptionContext& = delete;
    auto operator=(PyExceptionContext&&) -> PyExceptionContext& = delete;

    // Destructor
    ~PyExceptionContext() {
        Py_XDECREF(m_type);
        Py_XDECREF(m_value);
        Py_XDECREF(m_traceback);
    }

    // Methods
    /**
     * @return Whether the context stores an exception.
     */
    [[nodiscard]] auto has_exception() const noexcept -> bool { return nullptr != m_value; }

    /**
     * Restores the exception from the context.
     * NOTE:
     * - This method will clear the existing exception if one is set.
     * - The stored context will be cleared after restoration.
     * - If there's no exception in the stored context, the error indicator will be cleared.
     * - This method should be called strictly once, otherwise the error indicator will be cleared.
     * @return Whether an exception has been set by restoring the context.
     */
    [[maybe_unused]] auto restore() noexcept -> bool {
        auto const exception_has_been_set{has_exception()};
        PyErr_Restore(m_type, m_value, m_traceback);
        m_type = nullptr;
        m_value = nullptr;
        m_traceback = nullptr;
        return exception_has_been_set;
    }

    [[nodiscard]] auto get_type() const -> PyObject* { return m_type; }

    [[nodiscard]] auto get_value() const -> PyObject* { return m_value; }

    [[nodiscard]] auto get_traceback() const -> PyObject* { return m_traceback; }

private:
    PyObject* m_type{nullptr};
    PyObject* m_value{nullptr};
    PyObject* m_traceback{nullptr};
};
}  // namespace clp_ffi_py

#endif  // CLP_FFI_PY_PYEXCEPTIONCONTEXT_HPP
