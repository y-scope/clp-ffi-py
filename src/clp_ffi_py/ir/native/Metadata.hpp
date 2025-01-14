#ifndef CLP_FFI_PY_IR_NATIVE_METADATA_HPP
#define CLP_FFI_PY_IR_NATIVE_METADATA_HPP

#include <string>
#include <utility>

#include <clp/ir/types.hpp>
#include <json/single_include/nlohmann/json.hpp>

namespace clp_ffi_py::ir::native {
/**
 * A class that represents a deserialized IR preamble. Contains ways to access (get) metadata such
 * as the timestamp format. After construction, the metadata is readonly.
 */
class Metadata {
public:
    /**
     * Constructs a new Metadata object by reading values from a JSON object deserialized from the
     * preamble. This constructor will validate the JSON data and throw exceptions when failing to
     * extract required values.
     * @param metadata JSON data that contains the metadata.
     * @param is_four_byte_encoding
     */
    explicit Metadata(nlohmann::json const& metadata, bool is_four_byte_encoding);

    /**
     * Constructs a new Metadata object from the provided fields. Currently,
     * `m_is_four_byte_encoding` is set to true by default since it is the only format supported.
     * @param ref_timestamp The reference timestamp used to calculate the timestamp of the first log
     * message in the IR stream.
     * @param timestamp_format Timestamp format to use when generating the logs with a reader.
     * @param timezone Timezone in TZID format to use when generating the timestamp from Unix epoch
     * time.
     */
    explicit Metadata(
            clp::ir::epoch_time_ms_t ref_timestamp,
            std::string timestamp_format,
            std::string timezone
    )
            : m_is_four_byte_encoding{true},
              m_ref_timestamp{ref_timestamp},
              m_timestamp_format{std::move(timestamp_format)},
              m_timezone_id{std::move(timezone)} {}

    [[nodiscard]] auto is_using_four_byte_encoding() const -> bool {
        return m_is_four_byte_encoding;
    }

    [[nodiscard]] auto get_ref_timestamp() const -> clp::ir::epoch_time_ms_t {
        return m_ref_timestamp;
    }

    [[nodiscard]] auto get_timestamp_format() const -> std::string const& {
        return m_timestamp_format;
    }

    [[nodiscard]] auto get_timezone_id() const -> std::string const& { return m_timezone_id; }

private:
    bool m_is_four_byte_encoding;
    clp::ir::epoch_time_ms_t m_ref_timestamp;
    std::string m_timestamp_format;
    std::string m_timezone_id;
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_METADATA_HPP
