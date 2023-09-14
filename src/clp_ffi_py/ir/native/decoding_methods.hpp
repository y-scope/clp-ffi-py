#ifndef CLP_FFI_PY_DECODING_METHODS
#define CLP_FFI_PY_DECODING_METHODS

#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

// Documentation for these methods is in clp/ir/native/PyDecoder.cpp, as it also
// serves as the documentation for Python.
namespace clp_ffi_py::ir::native {
extern "C" {
auto decode_preamble(PyObject* self, PyObject* py_decoder_buffer) -> PyObject*;
auto decode_next_log_event(PyObject* self, PyObject* args, PyObject* keywords) -> PyObject*;
}
}  // namespace clp_ffi_py::ir::native

#endif
