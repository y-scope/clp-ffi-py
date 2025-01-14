#ifndef CLP_FFI_PY_IR_NATIVE_ERROR_MESSAGES
#define CLP_FFI_PY_IR_NATIVE_ERROR_MESSAGES

#include <string_view>

namespace clp_ffi_py::ir::native {
constexpr std::string_view cDeserializerBufferOverflowError{
        "DeserializerBuffer internal read buffer overflows."
};
constexpr std::string_view cDeserializerCreateErrorFormatStr{
        "Native `Deserializer::create` failed: %s"
};
constexpr std::string_view cDeserializerDeserializeNextIrUnitErrorFormatStr{
        "Native `Deserializer::deserialize_next_ir_unit` failed: %s"
};
constexpr std::string_view cDeserializerErrorCodeFormatStr{
        "IR deserialization method failed with error code: %d."
};
constexpr std::string_view cDeserializerIncompleteIRError{"The IR stream is incomplete."};
constexpr std::string_view cKeyValuePairLogEventSerializeToStringErrorFormatStr{
        "Native `KeyValuePairLogEvent::serialize_to_json` failed: %s"
};
constexpr std::string_view cSerializerCreateErrorFormatStr{"Native `Serializer::create` failed: %s"
};
constexpr std::string_view cSerializerSerializeMsgpackMapError{
        "Native `Serializer::serialize_msgpack_map` failed"
};
constexpr std::string_view cSerializeMessageError{
        "Native serializer cannot serialize the given message"
};
constexpr std::string_view cSerializePreambleError{
        "Native serializer cannot serialize the given preamble"
};
constexpr std::string_view cSerializeTimestampError{
        "Native serializer cannot serialize the given timestamp delta"
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_ERROR_MESSAGES
