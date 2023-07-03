#ifndef CLP_FFI_PY_ERROR_MESSAGE
#define CLP_FFI_PY_ERROR_MESSAGE

namespace clp_ffi_py::error_messages {
// Note: each every message is defined using pointer instead of an array to
// address clang-tidy warnings.
constexpr char const* out_of_memory_error = "Failed to allocate memory.";
constexpr char const* py_type_error = "Wrong Python Type received.";
constexpr char const* setstate_input_error =
        "Python dictionary is expected to be the input of __setstate__ method.";
constexpr char const* setstate_key_error_template = "\"%s\" not found in the state dictionary.";

namespace encoder {
    constexpr char const* cTimestampError =
            "Native encoder cannot encode the given timestamp delta";
    constexpr char const* cPreambleError = "Native encoder cannot encode the given preamble";
    constexpr char const* cMessageError = "Native encoder cannot encode the given message";
} // namespace encoder
} // namespace clp_ffi_py::error_messages
#endif
