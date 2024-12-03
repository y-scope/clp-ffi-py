#ifndef CLP_FFI_PY_PY_OBJECT_UTILS_HPP
#define CLP_FFI_PY_PY_OBJECT_UTILS_HPP

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include <memory>

#include <clp_ffi_py/PyExceptionContext.hpp>

namespace clp_ffi_py {
/**
 * A specialized deleter for PyObjectPtr which decrements the pointed PyObject reference count when
 * it is destroyed.
 * @tparam PyObjectType
 */
template <typename PyObjectType>
class PyObjectDeleter {
public:
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    void operator()(PyObjectType* ptr) { Py_XDECREF(reinterpret_cast<PyObject*>(ptr)); }
};

/**
 * An empty deleter that has an empty implementation to ensure object trivial destruction.
 * @tparam PyObjectType
 */
template <typename PyObjectType>
class PyObjectTrivialDeleter {
public:
    void operator()(PyObjectType* ptr) {}
};

/**
 * A type of smart pointer that maintains a reference to a Python object for the duration of its
 * lifetime.
 * @tparam PyObjectType
 */
template <typename PyObjectType>
using PyObjectPtr = std::unique_ptr<PyObjectType, PyObjectDeleter<PyObjectType>>;

/**
 * A smart pointer to be used for raw Python object pointers that have static storage duration. It
 * holds a reference of the underlying Python object. Compared to PyObjectPtr, when destructed, this
 * smart pointer does not decrease the reference count of the contained Python object. Since this
 * pointer has static storage duration, it's possible that the Python interpreter exits before this
 * pointer's destructor is called; attempting to decrement the reference count (maintained by the
 * interpreter) in this situation would lead to undefined behaviour.
 * @tparam PyObjectType
 */
template <typename PyObjectType>
using PyObjectStaticPtr = std::unique_ptr<PyObjectType, PyObjectTrivialDeleter<PyObjectType>>;

/**
 * A guard class for Python exceptions. In certain CPython methods, such as `tp_finalize`,
 * the exception state must remain unchanged throughout execution. This class saves the current
 * exception state upon initialization and restores it upon destruction, ensuring the exception
 * status is preserved.
 * Docs: https://docs.python.org/3/c-api/typeobj.html#c.PyTypeObject.tp_finalize
 */
class PyErrGuard {
public:
    // Constructor
    PyErrGuard() = default;

    // Destructor
    ~PyErrGuard() { m_exception_context.restore(); }

    // Delete copy/move constructor and assignment
    PyErrGuard(PyErrGuard const&) = delete;
    PyErrGuard(PyErrGuard&&) = delete;
    auto operator=(PyErrGuard const&) -> PyErrGuard& = delete;
    auto operator=(PyErrGuard&&) -> PyErrGuard& = delete;

private:
    // Variables
    PyExceptionContext m_exception_context;
};
}  // namespace clp_ffi_py

#endif  // CLP_FFI_PY_PY_OBJECT_UTILS_HPP
