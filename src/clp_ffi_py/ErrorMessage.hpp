#ifndef CLP_FFI_PY_ERROR_MESSAGE
#define CLP_FFI_PY_ERROR_MESSAGE

namespace clp_ffi_py::error_messages {
// Note: each every message is defined using pointer instead of an array to
// address clang-tidy warnings.
constexpr char const* cOutofMemoryError = "Failed to allocate memory.";
constexpr char const* cPyTypeError = "Wrong Python Type received.";
constexpr char const* cSetstateInputError
        = "Python dictionary is expected to be the input of __setstate__ method.";
constexpr char const* cSetstateKeyErrorTemplate = "\"%s\" not found in the state dictionary.";
constexpr char const* cTimezoneObjectNotInitialzed
        = "Timezone (tzinfo) object is not yet initialized.";

namespace ir {
constexpr char const* cEncodeTimestampError
        = "Native encoder cannot encode the given timestamp delta";
constexpr char const* cEncodePreambleError = "Native encoder cannot encode the given preamble";
constexpr char const* cEncodeMessageError = "Native encoder cannot encode the given message";
}  // namespace ir
}  // namespace clp_ffi_py::error_messages
#endif
