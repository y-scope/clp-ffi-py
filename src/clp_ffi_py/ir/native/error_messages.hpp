#ifndef CLP_FFI_PY_IR_NATIVE_ERROR_MESSAGES
#define CLP_FFI_PY_IR_NATIVE_ERROR_MESSAGES

namespace clp_ffi_py::ir::native {
constexpr char const* cDeserializerBufferOverflowError
        = "DeserializerBuffer internal read buffer overflows.";
constexpr char const* cDeserializerIncompleteIRError = "The IR stream is incomplete.";
constexpr char const* cDeserializerErrorCodeFormatStr
        = "IR deserialization method failed with error code: %d.";
constexpr char const* cSerializeTimestampError
        = "Native serializer cannot serialize the given timestamp delta";
constexpr char const* cSerializePreambleError
        = "Native serializer cannot serialize the given preamble";
constexpr char const* cSerializeMessageError
        = "Native serializer cannot serialize the given message";
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_ERROR_MESSAGES
