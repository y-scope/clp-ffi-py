#ifndef CLP_FFI_PY_IR_NATIVE_DESERIALIZATION_METHODS
#define CLP_FFI_PY_IR_NATIVE_DESERIALIZATION_METHODS

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include <clp_ffi_py/api_decoration.hpp>

// Documentation for these methods is in clp/ir/native/PyDeserializer.cpp, as it also serves as the
// documentation for Python.
namespace clp_ffi_py::ir::native {
CLP_FFI_PY_METHOD auto deserialize_preamble(PyObject* self, PyObject* py_deserializer_buffer)
        -> PyObject*;

CLP_FFI_PY_METHOD auto
deserialize_next_log_event(PyObject* self, PyObject* args, PyObject* keywords) -> PyObject*;
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_DESERIALIZATION_METHODS
