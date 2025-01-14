#ifndef CLP_FFI_PY_IR_NATIVE_PYFOURBYTEDESERIALIZER_HPP
#define CLP_FFI_PY_IR_NATIVE_PYFOURBYTEDESERIALIZER_HPP

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include <clp_ffi_py/PyObjectUtils.hpp>

namespace clp_ffi_py::ir::native {
/**
 * This class provides a Python-level namespace for four-byte encoded IR deserialization methods.
 */
class PyFourByteDeserializer {
public:
    // Static methods
    /**
     * Creates and initializes PyFourByteDeserializer as a Python type, and then incorporates this
     * type as a Python object into py_module.
     * @param py_module This is the Python module where the initialized PyFourByteDeserializer will
     * be incorporated.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto module_level_init(PyObject* py_module) -> bool;

    // Delete default constructor to disable direct instantiation.
    PyFourByteDeserializer() = delete;

    // Delete copy & move constructors and assignment operators
    PyFourByteDeserializer(PyFourByteDeserializer const&) = delete;
    PyFourByteDeserializer(PyFourByteDeserializer&&) = delete;
    auto operator=(PyFourByteDeserializer const&) -> PyFourByteDeserializer& = delete;
    auto operator=(PyFourByteDeserializer&&) -> PyFourByteDeserializer& = delete;

    // Destructor
    ~PyFourByteDeserializer() = default;

private:
    static inline PyObjectStaticPtr<PyTypeObject> m_py_type{nullptr};

    PyObject_HEAD;
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_PYFOURBYTEDESERIALIZER_HPP
