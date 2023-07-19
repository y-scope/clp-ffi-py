#ifndef CLP_FFI_PY_QUERY_HPP
#define CLP_FFI_PY_QUERY_HPP

#include <limits>
#include <optional>
#include <vector>

#include <clp/components/core/src/ffi/encoding_methods.hpp>

#include <clp_ffi_py/ir/LogEvent.hpp>

namespace clp_ffi_py::ir {
/**
 * This class represents a search query, utilized for eliminating unmatched log
 * events when decoding an encoded IR stream. The query could include a list of
 * wildcard searches aimed at identifying certain log messages, and a preset
 * timestamp interval specified by a lower bound and an upper bound timestamp,
 * which are defined as query conditions. This class provides an interface to
 * set up a query with customized conditions, as well as methods to validate
 * whether a log event matches the query conditions.
 */
class Query {
public:
    static constexpr ffi::epoch_time_ms_t const cTimestampMin{0};
    static constexpr ffi::epoch_time_ms_t const cTimestampMax{
            std::numeric_limits<ffi::epoch_time_ms_t>::max()};
    static constexpr ffi::epoch_time_ms_t const cTimestampUpperBoundSafeRange{
            static_cast<ffi::epoch_time_ms_t>(60 * 1000)};

    /**
     * Constructs a new query object. By default, the wildcard list is empty and
     * the timestamp range is set to include all the valid Unix epoch time.
     * @param case_sensitive Whether the wildcard match is case-sensitive.
     */
    explicit Query(bool case_sensitive)
            : m_case_sensitive{case_sensitive},
              m_ts_lower_bound{cTimestampMin},
              m_ts_upper_bound{cTimestampMax},
              m_ts_safe_upper_bound{cTimestampMax} {};

    /**
     * Constructs a new query object with the given timestamp range. By default,
     * the wildcard list is empty.
     * @param case_sensitive Whether the wildcard match is case-sensitive.
     * @param ts_lower_bound Start of search time range (inclusive).
     * @param ts_upper_bound End of search time range (inclusive).
     */
    explicit Query(
            bool case_sensitive,
            ffi::epoch_time_ms_t ts_lower_bound,
            ffi::epoch_time_ms_t ts_upper_bound
    )
            : m_case_sensitive{case_sensitive},
              m_ts_lower_bound{ts_lower_bound},
              m_ts_upper_bound{ts_upper_bound},
              m_ts_safe_upper_bound{
                      (cTimestampMax - cTimestampUpperBoundSafeRange > ts_upper_bound)
                              ? ts_upper_bound + cTimestampUpperBoundSafeRange
                              : cTimestampMax} {};

    [[nodiscard]] auto is_case_sensitive() const -> bool { return m_case_sensitive; }

    [[nodiscard]] auto get_ts_lower_bound() const -> ffi::epoch_time_ms_t {
        return m_ts_lower_bound;
    }

    [[nodiscard]] auto get_ts_upper_bound() const -> ffi::epoch_time_ms_t {
        return m_ts_upper_bound;
    }

    /**
     * Adds a wildcard search query into the wildcard list.
     * @param wildcard
     */
    auto add_wildcard(std::string_view wildcard) -> void { m_wildcard_list.emplace_back(wildcard); }

    /**
     * Checks the given timestamp against the timestamp search lower bound.
     * @param ts Given timestamp.
     * @return whether the given timestamp is equal to or larger than the
     * timestamp search lower bound.
     */
    [[nodiscard]] auto ts_lower_bound_check(ffi::epoch_time_ms_t ts) const -> bool {
        return m_ts_lower_bound <= ts;
    }

    /**
     * Checks the given timestamp against the timestamp search upper bound.
     * @param ts Given timestamp.
     * @return whether the given timestamp is equal to or smaller than the
     * timestamp search lower bound.
     */
    [[nodiscard]] auto ts_upper_bound_check(ffi::epoch_time_ms_t ts) const -> bool {
        return ts <= m_ts_upper_bound;
    }

    /**
     * Checks whether the given timestamp is in the search range: it should pass
     * both the lower bound check and the upper bound check.
     * @param ts Given timestamp.
     * @return Whether the given timestamp is in range.
     */
    [[nodiscard]] auto ts_in_range(ffi::epoch_time_ms_t ts) const -> bool {
        return ts_lower_bound_check(ts) && ts_upper_bound_check(ts);
    }

    /**
     * Checks whether the given timestamp exceeds the timestamp safe upper
     * bound. When decoding an IR stream with a query, the decoding can
     * terminate once the timestamp exceeds the upper bound. However, the
     * timestamp might not be monotonically increasing. It can be locally
     * disordered due to the thread contention. To safely exit, we need to
     * ensure that the timestamp has exceeds the upper bound with a reasonable
     * range.
     * @param ts Given timestamp.
     * @return Whether the given timestamp strictly exceeds the safe upper
     * bound timestamp.
     */
    [[nodiscard]] auto ts_exceed_safe_upper_bound(ffi::epoch_time_ms_t ts) const -> bool {
        return m_ts_safe_upper_bound < ts;
    }

    /**
     * Validates whether the given log message matches the underlying wildcard
     * query list. If the wildcard list is empty, this function will always
     * return true.
     * @param log_message Input log message.
     * @return true if the wildcard query list is empty, or there is at least
     * one wildcard query matches the given message.
     * @return false if there is no matches found.
     */
    [[nodiscard]] auto wildcard_matches(std::string_view log_message) -> bool;

    /**
     * Validates whether the given log event matches the given query.
     * @param log_event Input log event.
     * @return Whether the timestamp is in range, and the message represented by
     * the log event matches at least one of the wildcard query if the wildcard
     * query list is not empty.
     */
    [[nodiscard]] auto matches(LogEvent const& log_event) -> bool {
        if (false == ts_in_range(log_event.get_timestamp())) {
            return false;
        }
        return wildcard_matches(log_event.get_log_message_view());
    }

private:
    std::vector<std::string> m_wildcard_list;
    bool m_case_sensitive;
    ffi::epoch_time_ms_t m_ts_lower_bound;
    ffi::epoch_time_ms_t m_ts_upper_bound;
    ffi::epoch_time_ms_t m_ts_safe_upper_bound;
};
}  // namespace clp_ffi_py::ir
#endif
