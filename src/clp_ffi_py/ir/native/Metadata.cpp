#include "Metadata.hpp"

#include <exception>
#include <string>

#include <clp/ErrorCode.hpp>
#include <clp/ffi/ir_stream/protocol_constants.hpp>
#include <clp/ir/types.hpp>
#include <json/single_include/nlohmann/json.hpp>

#include <clp_ffi_py/ExceptionFFI.hpp>

namespace clp_ffi_py::ir::native {
namespace {
/**
 * Validates whether the JSON object contains the given key and has a string data associated to this
 * particular key.
 * @param json_data JSON object to be validated.
 * @param key The key to access the data field.
 * @return true if the data is valid.
 */
[[nodiscard]] auto is_valid_json_string_data(nlohmann::json const& json_data, char const* key)
        -> bool;

auto is_valid_json_string_data(nlohmann::json const& json_data, char const* key) -> bool {
    return json_data.contains(key) && json_data.at(key).is_string();
}
}  // namespace

Metadata::Metadata(nlohmann::json const& metadata, bool is_four_byte_encoding) {
    if (false == is_four_byte_encoding) {
        throw ExceptionFFI(
                clp::ErrorCode_Unsupported,
                __FILE__,
                __LINE__,
                "Eight Byte Preamble is not yet supported."
        );
    }
    m_is_four_byte_encoding = is_four_byte_encoding;

    auto const* ref_timestamp_key{static_cast<char const*>(
            clp::ffi::ir_stream::cProtocol::Metadata::ReferenceTimestampKey
    )};
    if (false == is_valid_json_string_data(metadata, ref_timestamp_key)) {
        throw ExceptionFFI(
                clp::ErrorCode_MetadataCorrupted,
                __FILE__,
                __LINE__,
                "Valid Reference Timestamp cannot be found in the metadata."
        );
    }
    try {
        std::string const ref_timestamp_str{metadata.at(ref_timestamp_key)};
        m_ref_timestamp = static_cast<clp::ir::epoch_time_ms_t>(std::stoull(ref_timestamp_str));
    } catch (std::exception const& ex) {
        throw ExceptionFFI(clp::ErrorCode_Unsupported, __FILE__, __LINE__, ex.what());
    }

    auto const* timestamp_format_key{
            static_cast<char const*>(clp::ffi::ir_stream::cProtocol::Metadata::TimestampPatternKey)
    };
    if (false == is_valid_json_string_data(metadata, timestamp_format_key)) {
        throw ExceptionFFI(
                clp::ErrorCode_MetadataCorrupted,
                __FILE__,
                __LINE__,
                "Valid Timestamp Format cannot be found in the metadata."
        );
    }
    m_timestamp_format = metadata.at(timestamp_format_key);

    auto const* timezone_id_key{
            static_cast<char const*>(clp::ffi::ir_stream::cProtocol::Metadata::TimeZoneIdKey)
    };
    if (false == is_valid_json_string_data(metadata, timezone_id_key)) {
        throw ExceptionFFI(
                clp::ErrorCode_MetadataCorrupted,
                __FILE__,
                __LINE__,
                "Valid Timezone ID cannot be found in the metadata."
        );
    }
    m_timezone_id = metadata.at(timezone_id_key);
}
}  // namespace clp_ffi_py::ir::native
