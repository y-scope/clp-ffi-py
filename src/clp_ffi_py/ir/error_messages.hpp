#ifndef CLP_FFI_PY_IR_ERROR_MESSAGES
#define CLP_FFI_PY_IR_ERROR_MESSAGES

namespace clp_ffi_py::ir {
constexpr char const* cDecoderBufferFullError
        = "DecoderBuffer is full and cannot read more from the IR stream.";
constexpr char const* cDecoderBufferOverflowError = "DecoderBuffer internal read buffer overflows.";
constexpr char const* cDecoderBufferPyBufferProtocolNotEnabledError
        = "DecoderBuffer Python buffer protocol is not enabled.";
constexpr char const* cEncodeTimestampError
        = "Native encoder cannot encode the given timestamp delta";
constexpr char const* cEncodePreambleError = "Native encoder cannot encode the given preamble";
constexpr char const* cEncodeMessageError = "Native encoder cannot encode the given message";
}  // namespace clp_ffi_py::ir

#endif  // CLP_FFI_PY_IR_ERROR_MESSAGES
