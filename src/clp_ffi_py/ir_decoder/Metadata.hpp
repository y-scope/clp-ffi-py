#ifndef CLP_FFI_PY_METADATA_HPP
#define CLP_FFI_PY_METADATA_HPP

#include <clp/components/core/src/ffi/encoding_methods.hpp>
#include <clp/components/core/submodules/json/single_include/nlohmann/json.hpp>

namespace clp_ffi_py::ir_decoder {
/**
 * A class that represents a decoded IR preamble. Contains ways to access (get)
 * metadata such as timestamp format. The metadata should be readonly,
 * meaning that there is no way to change the underlying values after
 * initialization.
 */
class Metadata {
public:
    /**
     * Constructs a new Metadata object by reading values from a JSON object
     * decoded from the preamble. This constructor will validate the JSON data
     * and throw exceptions when failing to extract required values.
     * @param metadata JSON data that contains the metadata.
     * @param is_four_byte_encoding
     */
    explicit Metadata(nlohmann::json const& metadata, bool is_four_byte_encoding);

    /**
     * Constructs a new Metadata object with values of underlying fields
     * explicitly given. Currently, `m_is_four_byte_encoding` is set to true by
     * default since it is the only format supported.
     * @param ref_timestamp
     * @param timestamp_format
     * @param timezone
     */
    explicit Metadata(
            ffi::epoch_time_ms_t ref_timestamp,
            std::string const& timestamp_format,
            std::string const& timezone)
        : m_is_four_byte_encoding{true},
          m_ref_timestamp{ref_timestamp},
          m_timestamp_format{timestamp_format},
          m_timezone_id{timezone} {};

    [[nodiscard]] auto is_using_four_byte_encoding() const -> bool {
        return m_is_four_byte_encoding;
    }

    [[nodiscard]] auto get_ref_timestamp() const -> ffi::epoch_time_ms_t { return m_ref_timestamp; }

    [[nodiscard]] auto get_timestamp_format() const -> std::string const& {
        return m_timestamp_format;
    }

    [[nodiscard]] auto get_timezone_id() const -> std::string const& { return m_timezone_id; }

private:
    bool m_is_four_byte_encoding;
    ffi::epoch_time_ms_t m_ref_timestamp;
    std::string m_timestamp_format;
    std::string m_timezone_id;
};
} // namespace clp_ffi_py::ir_decoder
#endif
