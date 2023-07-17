#ifndef CLP_FFI_PY_PY_OBJECT_UTILS_HPP
#define CLP_FFI_PY_PY_OBJECT_UTILS_HPP

#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <memory>

namespace clp_ffi_py {
/**
 * A specialized deleter for PyObjectPtr which decrements the pointed PyObject
 * reference count when it is destroyed.
 * @tparam PyObjectType
 */
template <typename PyObjectType>
class PyObjectDeleter {
public:
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    void operator()(PyObjectType* ptr) { Py_XDECREF(reinterpret_cast<PyObject*>(ptr)); }
};

/**
 * A type of smart pointer that maintains a reference to a Python object for the
 * duration of its lifetime.
 * @tparam PyObjectType
 */
template <typename PyObjectType>
using PyObjectPtr = std::unique_ptr<PyObjectType, PyObjectDeleter<PyObjectType>>;
}  // namespace clp_ffi_py
#endif  // CLP_FFI_PY_PY_OBJECT_PTR_HPP
