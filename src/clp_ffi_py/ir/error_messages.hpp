#ifndef CLP_FFI_PY_IR_ERROR_MESSAGES
#define CLP_FFI_PY_IR_ERROR_MESSAGES

namespace clp_ffi_py::ir {
constexpr char const* cDecoderBufferCursorOverflowError
        = "DecoderBuffer receives an operand that overflows the internal read buffer.";
constexpr char const* cDecoderBufferFullError
        = "DecoderBuffer is full and cannot read more from the IR stream.";
constexpr char const* cEncodeTimestampError
        = "Native encoder cannot encode the given timestamp delta";
constexpr char const* cEncodePreambleError = "Native encoder cannot encode the given preamble";
constexpr char const* cEncodeMessageError = "Native encoder cannot encode the given message";
}  // namespace clp_ffi_py::ir

#endif  // CLP_FFI_PY_IR_ERROR_MESSAGES
