#ifndef CLP_FFI_PY_PY_OBJECT_PTR_HPP
#define CLP_FFI_PY_PY_OBJECT_PTR_HPP

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

namespace ir {
class PyMetadata;
class PyLogEvent;
}  // namespace ir

template <typename Dst, typename Src, typename PyT>
constexpr bool is_valid_py_object_cast{
        (std::is_same_v<Src, PyObject*> && std::is_same_v<Dst, PyT>)
        || (std::is_same_v<Src, PyT> && std::is_same_v<Dst, PyObject*>)};

template <typename Dst, typename Src>
auto py_reinterpret_cast(Src src) noexcept -> Dst {
    constexpr bool is_valid_cast{
            std::is_same_v<Dst, PyCFunction> || is_valid_py_object_cast<Src, Dst, ir::PyMetadata*>
            || is_valid_py_object_cast<Src, Dst, ir::PyLogEvent*>
            || is_valid_py_object_cast<Src, Dst, PyTypeObject*>};
    static_assert(is_valid_cast, "Invalid Cast");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<Dst>(src);
}

}  // namespace clp_ffi_py
#endif  // CLP_FFI_PY_PY_OBJECT_PTR_HPP
