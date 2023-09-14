#ifndef CLP_FFI_PY_IR_ERROR_MESSAGES
#define CLP_FFI_PY_IR_ERROR_MESSAGES

namespace clp_ffi_py::ir::native {
constexpr char const* cDecoderBufferOverflowError = "DecoderBuffer internal read buffer overflows.";
constexpr char const* cDecoderIncompleteIRError = "The IR stream is incomplete.";
constexpr char const* cDecoderErrorCodeFormatStr = "IR decoding method failed with error code: %d.";
constexpr char const* cEncodeTimestampError
        = "Native encoder cannot encode the given timestamp delta";
constexpr char const* cEncodePreambleError = "Native encoder cannot encode the given preamble";
constexpr char const* cEncodeMessageError = "Native encoder cannot encode the given message";
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_ERROR_MESSAGES
