#ifndef CLP_FFI_PY_ERROR_MESSAGES
#define CLP_FFI_PY_ERROR_MESSAGES

namespace clp_ffi_py {
constexpr char const* cOutofMemoryError = "Failed to allocate memory.";
constexpr char const* cPyTypeError = "Wrong Python Type received.";
constexpr char const* cSetstateInputError
        = "Python dictionary is expected to be the input of __setstate__ method.";
constexpr char const* cSetstateKeyErrorTemplate = "\"%s\" not found in the state dictionary.";
constexpr char const* cTimezoneObjectNotInitialzed
        = "Timezone (tzinfo) object is not yet initialized.";
}  // namespace clp_ffi_py

#endif  // CLP_FFI_PY_ERROR_MESSAGES
