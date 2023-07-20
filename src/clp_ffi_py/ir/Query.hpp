#ifndef CLP_FFI_PY_QUERY_HPP
#define CLP_FFI_PY_QUERY_HPP

#include <limits>
#include <vector>

#include <clp/components/core/src/ffi/encoding_methods.hpp>

#include <clp_ffi_py/ir/LogEvent.hpp>

namespace clp_ffi_py::ir {
/**
 * This class represents a search query, utilized for eliminating unmatched log
 * events when decoding a CLP IR stream. The query could include a list of
 * wildcard searches aimed at identifying certain log messages, and a preset
 * timestamp interval specified by a lower bound and an upper bound timestamp.
 * This class provides an interface to set up a search query, as well as methods 
 * to validate whether the query can be matched by a log event.
 */
class Query {
public:
    static constexpr ffi::epoch_time_ms_t const cTimestampMin{0};
    static constexpr ffi::epoch_time_ms_t const cTimestampMax{
            std::numeric_limits<ffi::epoch_time_ms_t>::max()};
    
    /**
    * Ideally, when decoding an IR stream with a query, the decoding terminates 
    * once the timestamp exceeds the search upper bound. However, the
    * timestamp might not be monotonically increasing in a CLP IR stream. It can
    * be locally disordered due to the thread contention. To safely exit, we 
    * need to ensure that the timestamp has exceeded the upper bound by a 
    * reasonable margin. This margin is defined by the following constant. 
    * During the query initialization, it will be applied to the upper bound
    * timestamp to generate a timestamp which indicates the safe termination.
    */
    static constexpr ffi::epoch_time_ms_t const cTimestampTerminationMargin{
            static_cast<ffi::epoch_time_ms_t>(60 * 1000)};

    /**
     * Constructs a new query object. By default, the wildcard list is empty and
     * the timestamp range is set to include all the valid Unix epoch time.
     * @param case_sensitive Whether the wildcard match is case-sensitive.
     */
    explicit Query(bool case_sensitive)
            : m_case_sensitive{case_sensitive},
              m_lower_bound_ts{cTimestampMin},
              m_upper_bound_ts{cTimestampMax},
              m_termination_ts{cTimestampMax} {};

    /**
     * Constructs a new query object with the given timestamp range. By default,
     * the wildcard list is empty.
     * @param case_sensitive Whether the wildcard match is case-sensitive.
     * @param search_time_lower_bound Start of search time range (inclusive).
     * @param search_time_upper_bound End of search time range (inclusive).
     */
    explicit Query(
            bool case_sensitive,
            ffi::epoch_time_ms_t search_time_lower_bound,
            ffi::epoch_time_ms_t search_time_upper_bound
    )
            : m_case_sensitive{case_sensitive},
              m_lower_bound_ts{search_time_lower_bound},
              m_upper_bound_ts{search_time_upper_bound},
              m_termination_ts{
                      (cTimestampMax - cTimestampTerminationMargin > search_time_upper_bound)
                              ? search_time_upper_bound + cTimestampTerminationMargin
                              : cTimestampMax} {};

    [[nodiscard]] auto is_case_sensitive() const -> bool { return m_case_sensitive; }

    [[nodiscard]] auto get_lower_bound_ts() const -> ffi::epoch_time_ms_t {
        return m_lower_bound_ts;
    }

    [[nodiscard]] auto get_ts_upper_bound_ts() const -> ffi::epoch_time_ms_t {
        return m_upper_bound_ts;
    }

    /**
     * Adds a wildcard search query into the wildcard list.
     * @param wildcard
     */
    auto add_wildcard(std::string_view wildcard) -> void { m_wildcard_list.emplace_back(wildcard); }

    /**
     * @param ts Input timestamp.
     * @return true if the given timestamp is equal to or greater than the 
     * search lower bound.
     * @return false otherwise.
     */
    [[nodiscard]] auto ts_lower_bound_check(ffi::epoch_time_ms_t ts) const -> bool {
        return m_lower_bound_ts <= ts;
    }

    /**
     * @param ts Input timestamp.
     * @return true if the given timestamp is equal to or smaller than the 
     * search lower bound.
     * @return false otherwise.
     */
    [[nodiscard]] auto ts_upper_bound_check(ffi::epoch_time_ms_t ts) const -> bool {
        return ts <= m_upper_bound_ts;
    }

    /**
     * @param ts Input timestamp.
     * @return Whether the given timestamp is in the search time rage bounded by
     * the lower bound and the upper bound timestamp (inclusive).
     */
    [[nodiscard]] auto ts_in_range(ffi::epoch_time_ms_t ts) const -> bool {
        return ts_lower_bound_check(ts) && ts_upper_bound_check(ts);
    }

    /**
     * Determines whether the decoding can terminate by evaluating the input
     * timestamp.
     * @param ts Input timestamp.
     * @return true if the given timestamp is equal to or greater than the 
     * termination timestamp.
     * @return false otherwise.
     */
    [[nodiscard]] auto is_safe_to_terminate_decoding(ffi::epoch_time_ms_t ts) const -> bool {
        return m_termination_ts <= ts;
    }

    /**
     * Validates whether the input log message matches the query.
     * @param log_message Input log message.
     * @return true if the wildcard list is empty or has at least one match.
     * @return false otherwise.
     */
    [[nodiscard]] auto wildcard_matches(std::string_view log_message) -> bool;

    /**
     * Validates whether the input log event matches the query.
     * @param log_event Input log event.
     * @return true if the timestamp is in range, and the wildcard list is empty
     * or has at least one match.
     * @return false otherwise.
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
    ffi::epoch_time_ms_t m_lower_bound_ts;
    ffi::epoch_time_ms_t m_upper_bound_ts;
    ffi::epoch_time_ms_t m_termination_ts;
};
}  // namespace clp_ffi_py::ir
#endif
