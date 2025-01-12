#ifndef CLP_FFI_PY_IR_NATIVE_LOG_EVENT_HPP
#define CLP_FFI_PY_IR_NATIVE_LOG_EVENT_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

#include <clp/ir/types.hpp>

namespace clp_ffi_py::ir::native {
/**
 * A class that represents a deserialized IR log event. Contains ways to access (get or set) the log
 * message, the timestamp, and the log event index.
 */
class LogEvent {
public:
    LogEvent() = delete;

    /**
     * Constructs a new log event and leaves the formatted timestamp empty by default.
     * @param log_message
     * @param timestamp
     * @param index
     * @param formatted_timestamp
     */
    explicit LogEvent(
            std::string_view log_message,
            clp::ir::epoch_time_ms_t timestamp,
            size_t index,
            std::optional<std::string_view> formatted_timestamp = std::nullopt
    )
            : m_log_message{log_message},
              m_timestamp{timestamp},
              m_index{index} {
        if (formatted_timestamp.has_value()) {
            m_formatted_timestamp = std::string(formatted_timestamp.value());
        }
    }

    [[nodiscard]] auto get_log_message() const -> std::string { return m_log_message; }

    [[nodiscard]] auto get_log_message_view() const -> std::string_view {
        return std::string_view{m_log_message};
    }

    [[nodiscard]] auto get_timestamp() const -> clp::ir::epoch_time_ms_t { return m_timestamp; }

    [[nodiscard]] auto get_formatted_timestamp() const -> std::string {
        return m_formatted_timestamp;
    }

    [[nodiscard]] auto get_index() const -> size_t { return m_index; }

    /**
     * @return Whether the log event has the formatted timestamp buffered.
     */
    [[nodiscard]] auto has_formatted_timestamp() const -> bool {
        return (false == m_formatted_timestamp.empty());
    }

    auto set_log_message(std::string_view log_message) -> void { m_log_message = log_message; }

    auto set_timestamp(clp::ir::epoch_time_ms_t timestamp) -> void { m_timestamp = timestamp; }

    auto set_formatted_timestamp(std::string const& formatted_timestamp) -> void {
        m_formatted_timestamp = formatted_timestamp;
    }

    auto set_index(size_t index) -> void { m_index = index; }

private:
    std::string m_log_message;
    clp::ir::epoch_time_ms_t m_timestamp;
    size_t m_index;
    std::string m_formatted_timestamp;
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_LOG_EVENT_HPP
