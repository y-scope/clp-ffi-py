#ifndef CLP_FFI_PY_ERROR_MESSAGE
#define CLP_FFI_PY_ERROR_MESSAGE

namespace clp_ffi_py::ErrorMessage {
constexpr char arg_parsing_error[] = "Native preamble encoder failed to parse Python arguments.";
constexpr char out_of_memory_error[] = "Failed to allocate memory.";

namespace Encoding {
    constexpr char timestamp_error[] = "Timestamp delta > signed int32 currently unsupported";
    constexpr char preamble_error[] = "Metadata length > unsigned short currently unsupported";
    constexpr char message_error[] = "Native encoder cannot handle the given message";
} // namespace Encoding
} // namespace clp_ffi_py::ErrorMessage
#endif