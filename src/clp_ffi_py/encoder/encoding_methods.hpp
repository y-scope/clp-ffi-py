#ifndef CLP_FFI_PY_ENCODING_METHODS
#define CLP_FFI_PY_ENCODING_METHODS

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

namespace clp_ffi_py::encoder::four_byte_encoding {
auto encode_preamble(PyObject* self, PyObject* args) -> PyObject*;
auto encode_message_and_timestamp_delta(PyObject* self, PyObject* args) -> PyObject*;
auto encode_message(PyObject* self, PyObject* args) -> PyObject*;
auto encode_timestamp_delta(PyObject* self, PyObject* args) -> PyObject*;
} // namespace clp_ffi_py::encoder::four_byte_encoding

#endif
