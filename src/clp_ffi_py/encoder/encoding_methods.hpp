#ifndef CLP_FFI_PY_ENCODING_METHOD
#define CLP_FFI_PY_ENCODING_METHOD

#include <clp_ffi_py/Python.hpp>

namespace clp_ffi_py::encoder::four_byte_encoding {
/**
 * Encodes the preamble using 4-byte encoding
 * @param[in] self
 * @param[in] args the reference timestamp, the timestamp format, and the time
 * zone are expected in sequence
 * @return A Python bytearray containing the encoded preamble on success.
 *  If there is a failure in argument parsing or
 *  ffi::ir_stream::four_byte_encoding::encode_preamble, it returns nullptr and
 *  set PyErr accordingly
 */
auto encode_preamble(PyObject* self, PyObject* args) -> PyObject*;

/**
 * Encodes the message and timestamp delta using 4-byte encoding
 * @param[in] self
 * @param[in] args timestamp delta and input byte buffer of the current log
 * message are expected in sequence
 * @return A Python bytearray containing the encoded message and timestamp
 *  delta on success. If there is a failure in argument parsing,
 *  ffi::ir_stream::four_byte_encoding::encode_message,
 *  or ffi::ir_stream::four_byte_encoding::encode_timestamp, it returns nullptr
 *  and sets PyErr accordingly
 */
auto encode_message_and_timestamp_delta(PyObject* self, PyObject* args) -> PyObject*;

/**
 * Encodes the message using 4-byte encoding
 * @param[in] self
 * @param[in] args a log message in byte array is expected
 * @return A Python bytearray containing the encoded message on success.
 *  If there is a failure in argument parsing or
 *  ffi::ir_stream::four_byte_encoding::encode_message, it returns nullptr
 *  and sets PyErr accordingly
 */
auto encode_message(PyObject* self, PyObject* args) -> PyObject*;

/**
 * Encodes the timestamp delta using 4-byte encoding
 * @param[in] self
 * @param[in] args timestamp delta
 * @return A Python bytearray containing the encoded timestamp delta on success.
 *  If there is a failure in argument parsing or
 *  ffi::ir_stream::four_byte_encoding::encode_timestamp, it returns nullptr
 *  and sets PyErr accordingly
 */
auto encode_timestamp_delta(PyObject* self, PyObject* args) -> PyObject*;
} // namespace clp_ffi_py::encoder::four_byte_encoding

#endif
