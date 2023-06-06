#ifndef CLP_FFI_PY_ERROR_MESSAGE
#define CLP_FFI_PY_ERROR_MESSAGE

namespace clp_ffi_py::error_messages {
namespace Encoding {
    constexpr char timestamp_error[] = "Native encoder cannot encode the given timestamp delta";
    constexpr char preamble_error[] = "Native encoder cannot encode the given preamble";
    constexpr char message_error[] = "Native encoder cannot encode the given message";
} // namespace Encoding
} // namespace clp_ffi_py::error_messages
#endif
