#ifndef CLP_PY_ENCODING_METHOD
#define CLP_PY_ENCODING_METHOD

#include <Python.h>

namespace clp_ffi_py::encoder::four_byte_encoding {
/**
 * Given the reference timestamp, the timestamp format, and the time zone,
 * encode the preamble
 * @param self
 * @param args, the reference timestamp, the timestamp format, and the time
 * zone are expected in sequence
 * @return python bytearray that contains the encoded preamble
 */
PyObject* encode_preamble (PyObject* self, PyObject* args);

/**
 * Given the timestamp delta of the last log message and the content of the
 * current log message, encode the message using 4-byte encoding
 * @param self
 * @param args, timestamp delta and input byte buffer of the current log
 * message are expected in sequence
 * @return python bytearray that contains the encoded message
 */
PyObject* encode_message_and_timestamp_delta (PyObject* self, PyObject* args);

/**
 * Encode the given message into IR and write it into the buffer using
 * 4-byte encoding
 * @param self
 * @param args, a log message in byte array is expected
 * @return python bytearray that contains the encoded message
 */
PyObject* encode_message (PyObject* self, PyObject* args);

/**
 * Encode the given timestamp delta into IR and write it into the buffer
 * using 4-byte encoding
 * @param self
 * @param args, timestamp delta
 * @return python bytearray that contains the encoded message
 */
PyObject* encode_timestamp_delta (PyObject* self, PyObject* args);
} // namespace clp_ffi_py::encoder::four_byte_encoding

#endif
