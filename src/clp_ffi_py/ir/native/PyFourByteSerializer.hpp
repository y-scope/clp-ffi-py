#ifndef CLP_FFI_PY_IR_NATIVE_PYFOURBYTESERIALIZER_HPP
#define CLP_FFI_PY_IR_NATIVE_PYFOURBYTESERIALIZER_HPP

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include <clp_ffi_py/PyObjectUtils.hpp>

namespace clp_ffi_py::ir::native {
/**
 * This class provides a Python-level namespace for CLP 4-byte IR serialization methods.
 */
class PyFourByteSerializer {
public:
    /**
     * Creates and initializes PyFourByteSerializer as a Python type, and then incorporates this
     * type as a Python object into py_module.
     * @param py_module This is the Python module where the initialized PyFourByteSerializer will be
     * incorporated.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto module_level_init(PyObject* py_module) -> bool;

private:
    static inline PyObjectStaticPtr<PyTypeObject> m_py_type{nullptr};

    PyObject_HEAD;
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_PYFOURBYTESERIALIZER_HPP
