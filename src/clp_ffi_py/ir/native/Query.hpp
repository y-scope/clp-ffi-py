#ifndef CLP_FFI_PY_IR_NATIVE_QUERY_HPP
#define CLP_FFI_PY_IR_NATIVE_QUERY_HPP

#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <clp/ErrorCode.hpp>
#include <clp/ir/types.hpp>

#include <clp_ffi_py/ExceptionFFI.hpp>
#include <clp_ffi_py/ir/native/LogEvent.hpp>

namespace clp_ffi_py::ir::native {
/**
 * This class defines a wildcard query, which includes a wildcard string and a boolean value to
 * indicate if the match is case-sensitive.
 */
class WildcardQuery {
public:
    /**
     * Initializes the wildcard query.
     * @param wildcard_query Wildcard query.
     * @param case_sensitive Case sensitive indicator.
     */
    WildcardQuery(std::string wildcard_query, bool case_sensitive)
            : m_wildcard_query(std::move(wildcard_query)),
              m_case_sensitive(case_sensitive) {}

    [[nodiscard]] auto get_wildcard_query() const -> std::string const& { return m_wildcard_query; }

    [[nodiscard]] auto is_case_sensitive() const -> bool { return m_case_sensitive; }

private:
    std::string m_wildcard_query;
    bool m_case_sensitive;
};

/**
 * This class represents a search query, utilized for filtering log events in a CLP IR stream. The
 * query could include a list of wildcard queries aimed at identifying certain log messages, and a
 * timestamp range with a lower and upper bound. This class provides an interface to set up a search
 * query, as well as methods to validate whether the query can be matched by a log event. Note that
 * an empty wildcard query list will match any log within the range.
 * <p>
 * NOTE: When searching an IR stream with a query, ideally, the search would terminate once the
 * current log event's timestamp exceeds the upper bound of the query's time range. However, the
 * timestamps in the IR stream might not be monotonically increasing; they can be locally disordered
 * due to thread contention. So to safely stop searching, we need to ensure that the current
 * timestamp in the IR stream exceeds the query's upper bound timestamp by a reasonable margin.
 * This margin can be specified by the user or it will default to
 * `cDefaultSearchTimeTerminationMargin`.
 */
class Query {
public:
    static constexpr clp::ir::epoch_time_ms_t const cTimestampMin{0};
    static constexpr clp::ir::epoch_time_ms_t const cTimestampMax{
            std::numeric_limits<clp::ir::epoch_time_ms_t>::max()
    };
    static constexpr clp::ir::epoch_time_ms_t const cDefaultSearchTimeTerminationMargin{
            static_cast<clp::ir::epoch_time_ms_t>(60 * 1000)
    };

    /**
     * Constructs an empty query object that will match all logs. The wildcard query list is empty
     * and the timestamp range is set to include all the valid Unix epoch timestamps.
     */
    explicit Query()
            : m_lower_bound_ts{cTimestampMin},
              m_upper_bound_ts{cTimestampMax},
              m_search_termination_ts{cTimestampMax} {}

    /**
     * Constructs a new query object with the given timestamp range with an empty wildcard list.
     * @param search_time_lower_bound Start of search time range (inclusive).
     * @param search_time_upper_bound End of search time range (inclusive).
     * @param search_time_termination_margin The margin used to determine the search termination
     * timestamp (see note in the class' docstring).
     */
    explicit Query(
            clp::ir::epoch_time_ms_t search_time_lower_bound,
            clp::ir::epoch_time_ms_t search_time_upper_bound,
            clp::ir::epoch_time_ms_t search_time_termination_margin
            = cDefaultSearchTimeTerminationMargin
    )
            : m_lower_bound_ts{search_time_lower_bound},
              m_upper_bound_ts{search_time_upper_bound},
              m_search_termination_ts{
                      (cTimestampMax - search_time_termination_margin > search_time_upper_bound)
                              ? search_time_upper_bound + search_time_termination_margin
                              : cTimestampMax
              } {
        throw_if_ts_range_invalid();
    }

    /**
     * Constructs a new query object with the given timestamp range and a wildcard query list.
     * @param search_time_lower_bound Start of search time range (inclusive).
     * @param search_time_upper_bound End of search time range (inclusive).
     * @param wildcard_queries A list of wildcard queries. Each wildcard query must be valid (see
     * `wildcard_match_unsafe`).
     * @param search_time_termination_margin The margin used to determine the search termination
     * timestamp (see note in the class' docstring).
     */
    Query(clp::ir::epoch_time_ms_t search_time_lower_bound,
          clp::ir::epoch_time_ms_t search_time_upper_bound,
          std::vector<WildcardQuery> wildcard_queries,
          clp::ir::epoch_time_ms_t search_time_termination_margin
          = cDefaultSearchTimeTerminationMargin)
            : m_lower_bound_ts{search_time_lower_bound},
              m_upper_bound_ts{search_time_upper_bound},
              m_search_termination_ts{
                      (cTimestampMax - search_time_termination_margin > search_time_upper_bound)
                              ? search_time_upper_bound + search_time_termination_margin
                              : cTimestampMax
              },
              m_wildcard_queries{std::move(wildcard_queries)} {
        throw_if_ts_range_invalid();
    }

    [[nodiscard]] auto get_lower_bound_ts() const -> clp::ir::epoch_time_ms_t {
        return m_lower_bound_ts;
    }

    [[nodiscard]] auto get_upper_bound_ts() const -> clp::ir::epoch_time_ms_t {
        return m_upper_bound_ts;
    }

    [[nodiscard]] auto get_wildcard_queries() const -> std::vector<WildcardQuery> const& {
        return m_wildcard_queries;
    }

    /**
     * @return The search time termination margin by calculating the difference between
     * `m_search_termination_ts` and `m_upper_bound_ts`.
     */
    [[nodiscard]] auto get_search_time_termination_margin() const -> clp::ir::epoch_time_ms_t {
        return m_search_termination_ts - m_upper_bound_ts;
    }

    /**
     * @param ts Input timestamp.
     * @return true if the given timestamp is in the search time range bounded by the lower bound
     * and the upper bound timestamp (inclusive).
     */
    [[nodiscard]] auto matches_time_range(clp::ir::epoch_time_ms_t ts) const -> bool {
        return m_lower_bound_ts <= ts && ts <= m_upper_bound_ts;
    }

    /**
     * @param ts Input timestamp.
     * @return Whether the given timestamp is safely outside this query's time range (see note in
     * the class' docstring).
     */
    [[nodiscard]] auto ts_safely_outside_time_range(clp::ir::epoch_time_ms_t ts) const -> bool {
        return m_search_termination_ts < ts;
    }

    /**
     * Validates whether the input log message matches any of the wildcard queries in the query.
     * @param log_message Input log message.
     * @return true if the wildcard query list is empty or at least one wildcard query matches.
     * @return false otherwise.
     */
    [[nodiscard]] auto matches_wildcard_queries(std::string_view log_message) const -> bool;

    /**
     * Validates whether the input log event matches the query.
     * @param log_event Input log event.
     * @return true if the timestamp is in range, and the wildcard list is empty or has at least one
     * match.
     * @return false otherwise.
     */
    [[nodiscard]] auto matches(LogEvent const& log_event) const -> bool {
        return matches_time_range(log_event.get_timestamp())
               && matches_wildcard_queries(log_event.get_log_message_view());
    }

private:
    /**
     * Throws an exception if the lower bound ts exceeds the upper bound ts.
     */
    auto throw_if_ts_range_invalid() const -> void {
        if (m_lower_bound_ts > m_upper_bound_ts) {
            throw ExceptionFFI(
                    clp::ErrorCode_Unsupported,
                    __FILE__,
                    __LINE__,
                    "Search query lower bound timestamp exceeds the upper bound timestamp."
            );
        }
    }

    clp::ir::epoch_time_ms_t m_lower_bound_ts;
    clp::ir::epoch_time_ms_t m_upper_bound_ts;
    clp::ir::epoch_time_ms_t m_search_termination_ts;
    std::vector<WildcardQuery> m_wildcard_queries;
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_QUERY_HPP
