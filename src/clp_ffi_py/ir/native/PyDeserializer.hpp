#ifndef CLP_FFI_PY_IR_NATIVE_PYDESERIALIZER_HPP
#define CLP_FFI_PY_IR_NATIVE_PYDESERIALIZER_HPP

#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <clp_ffi_py/PyObjectUtils.hpp>

namespace clp_ffi_py::ir::native {
/**
 * This class provides a Python-level namespace for IR deserialization methods.
 */
class PyDeserializer {
public:
    /**
     * Creates and initializes PyDeserializer as a Python type, and then incorporates this type as a
     * Python object into py_module.
     * @param py_module This is the Python module where the initialized PyDeserializer will be
     * incorporated.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto module_level_init(PyObject* py_module) -> bool;

private:
    PyObject_HEAD;

    static PyObjectStaticPtr<PyTypeObject> m_py_type;
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_PYDESERIALIZER_HPP
