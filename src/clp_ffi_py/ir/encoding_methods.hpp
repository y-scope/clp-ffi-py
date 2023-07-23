#ifndef CLP_FFI_PY_ENCODING_METHODS
#define CLP_FFI_PY_ENCODING_METHODS

#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

// Documentation for these methods is in clp_ffi_py/ir/PyFourByteEncoder.cpp,
// as it also serves as the documentation for python.
namespace clp_ffi_py::ir {
auto encode_four_byte_preamble(PyObject* self, PyObject* args) -> PyObject*;
auto encode_four_byte_message_and_timestamp_delta(PyObject* self, PyObject* args) -> PyObject*;
auto encode_four_byte_message(PyObject* self, PyObject* args) -> PyObject*;
auto encode_four_byte_timestamp_delta(PyObject* self, PyObject* args) -> PyObject*;
}  // namespace clp_ffi_py::ir

#endif
