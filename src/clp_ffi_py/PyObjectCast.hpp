#ifndef CLP_FFI_PY_PY_OBJECT_CAST_HPP
#define CLP_FFI_PY_PY_OBJECT_CAST_HPP

#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <type_traits>

namespace clp_ffi_py {

/**
 * Casts a given function pointer to a PyCFunction.
 * The main purpose of this function is to silence clang-tidy checks on using
 * `reinterpret_cast`. It does not perform any further function type checking.
 * @tparam Src The source function pointer type.
 * @param src The source function pointer.
 * @return PyCFunction using reinterpret_cast.
 */
template <typename Src>
auto py_c_function_cast(Src src) noexcept -> PyCFunction {
    static_assert(std::is_pointer_v<Src>);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<PyCFunction>(src);
}

/**
 * This template struct is used as a compile-time flag that indicates whether
 * type T is a PyObject or not. By default, `cValue` is set to false.
 * @tparam T
 */
template <typename T>
struct is_python_object {
    static constexpr bool cValue = false;
};

/**
 * This template const expression is a wrapper of underlying `cValue` stored in
 * `is_python_object`, which is used to determine whether a type T is a valid
 * Python object type.
 * @tparam T
 */
template <typename T>  // NOLINTNEXTLINE(readability-identifier-naming)
constexpr bool is_python_object_v{is_python_object<T>::cValue};

/**
 * The macro to create a specialization of is_python_object for a given type T.
 * Only types that are marked with this macro will be considered as a valid
 * Python object type.
 */
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CLP_FFI_PY_MARK_AS_PYOBJECT(T) \
    template <> \
    struct is_python_object<T> { \
        static constexpr bool cValue = true; \
    }

/**
 * Casts a given Src pointer to a Dst pointer. It should behave as followed:
 * 1. If Src and Dst are the same type, return `src` if their type is a valid
 * Python object type, or they are both PyObject.
 * 2. If Dst is PyObject, the cast is valid if Src is a PyObject. A pointer,
 * Python*, should be returned.
 * 3. If Src is PyObject, the cast is valid if Dst is a valid Python object
 * type. A pointer, Dst*, should be returned.
 * 4. Any other cases are considered as invalid cast.
 * @tparam Dst The destination type. Must be given explicitly.
 * @tparam Src The source type. Can be given implicitly.
 * @param src The source pointer.
 * @return Dst* The casted pointer.
 */
template <typename Dst, typename Src>
auto py_reinterpret_cast(Src* src) noexcept -> Dst* {
    if constexpr (std::is_same_v<Src*, Dst*>) {
        static_assert(is_python_object_v<Src> || std::is_same_v<Src*, PyObject*>);
        return src;
    } else if constexpr (std::is_same_v<Dst*, PyObject*>) {
        static_assert(is_python_object_v<Src>);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<PyObject*>(src);
    } else {
        static_assert(std::is_same_v<Src*, PyObject*>);
        static_assert(is_python_object_v<Dst>);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Dst*>(src);
    }
}

namespace ir {
class PyMetadata;
class PyLogEvent;
}  // namespace ir

CLP_FFI_PY_MARK_AS_PYOBJECT(ir::PyLogEvent);
CLP_FFI_PY_MARK_AS_PYOBJECT(ir::PyMetadata);
CLP_FFI_PY_MARK_AS_PYOBJECT(PyTypeObject);
}  // namespace clp_ffi_py
#endif
