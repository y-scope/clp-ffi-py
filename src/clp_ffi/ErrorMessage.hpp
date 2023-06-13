#ifndef CLP_FFI_PY_ERROR_MESSAGE
#define CLP_FFI_PY_ERROR_MESSAGE

namespace clp_ffi_py::error_messages {
namespace encoder {
    constexpr char const* cTimestampError =
            "Native encoder cannot encode the given timestamp delta";
    constexpr char const* cPreambleError = "Native encoder cannot encode the given preamble";
    constexpr char const* cMessageError = "Native encoder cannot encode the given message";
} // namespace encoder
} // namespace clp_ffi_py::error_messages
#endif
