#ifndef CLP_FFI_PY_ERROR_MESSAGES
#define CLP_FFI_PY_ERROR_MESSAGES

#include <string_view>

namespace clp_ffi_py {
constexpr std::string_view cOutOfMemoryError{"Failed to allocate memory."};
constexpr std::string_view cPyTypeError{"Wrong Python Type received."};
constexpr std::string_view cSetstateInputError{
        "Python dictionary is expected to be the input of __setstate__ method."
};
constexpr std::string_view cSetstateKeyErrorTemplate{"\"%s\" not found in the state dictionary."};
constexpr std::string_view cTimezoneObjectNotInitialized{
        "Timezone (tzinfo) object is not yet initialized."
};
}  // namespace clp_ffi_py

#endif  // CLP_FFI_PY_ERROR_MESSAGES
