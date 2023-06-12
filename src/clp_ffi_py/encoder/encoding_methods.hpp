#ifndef CLP_FFI_PY_ENCODING_METHOD
#define CLP_FFI_PY_ENCODING_METHOD

#include <clp_ffi_py/Python.hpp>

namespace clp_ffi_py::encoder::four_byte_encoding {
/**
 * Encodes the preamble
 * @param self
 * @param args the reference timestamp, the timestamp format, and the time
 * zone are expected in sequence
 * @return A Python bytearray containing the encoded preamble
 */
auto encode_preamble (PyObject* self, PyObject* args) -> PyObject*;

/**
 * Encodes the message and timestamp delta using 4-byte encoding
 * @param self
 * @param args timestamp delta and input byte buffer of the current log
 * message are expected in sequence
 * @return A Python bytearray containing the encoded message and timestamp
 * delta
 */
auto encode_message_and_timestamp_delta (PyObject* self, PyObject* args) -> PyObject*;

/**
 * Encodes the message using 4-byte encoding
 * @param self
 * @param args a log message in byte array is expected
 * @return A Python bytearray containing the encoded message
 */
auto encode_message (PyObject* self, PyObject* args) -> PyObject*;

/**
 * Encodes the timestamp delta using 4-byte encoding
 * @param self
 * @param args timestamp delta
 * @return A Python bytearray containing the encoded timestamp delta
 */
auto encode_timestamp_delta (PyObject* self, PyObject* args) -> PyObject*;
} // namespace clp_ffi_py::encoder::four_byte_encoding

#endif
