#ifndef CLP_FFI_PY_PYFOURBYTEENCODER_HPP
#define CLP_FFI_PY_PYFOURBYTEENCODER_HPP

#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <clp_ffi_py/PyObjectUtils.hpp>

namespace clp_ffi_py::ir::native {
/**
 * This class provides a Python-level namespace for CLP 4-byte IR encoding
 * methods.
 */
class PyFourByteEncoder {
public:
    /**
     * Creates and initializes PyFourByteEncoder as a Python type, and then
     * incorporates this type as a Python object into py_module.
     * @param py_module This is the Python module where the initialized
     * PyFourByteEncoder will be incorporated.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error
     * set.
     */
    [[nodiscard]] static auto module_level_init(PyObject* py_module) -> bool;

private:
    PyObject_HEAD;

    static PyObjectStaticPtr<PyTypeObject> m_py_type;
};
}  // namespace clp_ffi_py::ir::native
#endif
