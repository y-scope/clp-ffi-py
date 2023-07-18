#ifndef CLP_FFI_PY_IR_ERROR_MESSAGES
#define CLP_FFI_PY_IR_ERROR_MESSAGES

namespace clp_ffi_py::ir {
constexpr char const* const cEncodeTimestampError
        = "Native encoder cannot encode the given timestamp delta";
constexpr char const* const cEncodePreambleError
        = "Native encoder cannot encode the given preamble";
constexpr char const* const cEncodeMessageError = "Native encoder cannot encode the given message";
}  // namespace clp_ffi_py::ir

#endif  // CLP_FFI_PY_IR_ERROR_MESSAGES
