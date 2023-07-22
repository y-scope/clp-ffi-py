#ifndef CLP_FFI_PY_QUERY_HPP
#define CLP_FFI_PY_QUERY_HPP

#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include <clp/components/core/src/ErrorCode.hpp>
#include <clp/components/core/src/ffi/encoding_methods.hpp>

#include <clp_ffi_py/ExceptionFFI.hpp>
#include <clp_ffi_py/ir/LogEvent.hpp>

namespace clp_ffi_py::ir {
/**
 * This class defines a wildcard query, which includes a wildcard string and a
 * boolean value to indicate if the match is case-sensitive.
 */
class WildcardQuery {
public:
    /**
     * Initializes the wildcard query.
     * @param wildcard Wildcard string.
     * @param case_sensitive Case sensitive indicator.
     */
    WildcardQuery(std::string wildcard, bool case_sensitive)
            : m_wildcard(std::move(wildcard)),
              m_case_sensitive(case_sensitive){};

    [[nodiscard]] auto get_wildcard() const -> std::string_view { return m_wildcard; }

    [[nodiscard]] auto get_case_sensitive() const -> bool { return m_case_sensitive; }

private:
    std::string m_wildcard;
    bool m_case_sensitive;
};

/**
 * This class represents a search query, utilized for filtering log events in a
 * CLP IR stream. The query could include a list of wildcard searches aimed at
 * identifying certain log messages, and a timestamp interval with a lower and
 * upper bound.  This class provides an interface to set up a search query, as
 * well as methods to validate whether the query can be matched by a log event.
 */
class Query {
public:
    static constexpr ffi::epoch_time_ms_t const cTimestampMin{0};
    static constexpr ffi::epoch_time_ms_t const cTimestampMax{
            std::numeric_limits<ffi::epoch_time_ms_t>::max()};

    /**
     * Ideally, when searching an IR stream with a query, the search terminates
     * once the timestamp exceeds the search upper bound. However, the timestamp
     * might not be monotonically increasing in a CLP IR stream. It can be
     * locally disordered due to thread contention. To safely stop searching, we
     * need to ensure that the current timestamp has exceeded the search's upper
     * bound by a reasonable margin. This margin defaults to the following
     * constant. During the query initialization, it will be applied to the
     * upper bound timestamp to generate a timestamp which indicates the safe
     * termination.
     */
    static constexpr ffi::epoch_time_ms_t const cDefaultSearchTerminationMargin{
            static_cast<ffi::epoch_time_ms_t>(60 * 1000)};

    /**
     * Constructs an empty query object: the wildcard list is empty and the
     * timestamp range is set to include all the valid Unix epoch time.
     */
    explicit Query()
            : m_lower_bound_ts{cTimestampMin},
              m_upper_bound_ts{cTimestampMax},
              m_search_termination_ts{cTimestampMax} {};

    /**
     * Constructs a new query object with the given timestamp range with an
     * empty wildcard list.
     * @param search_time_lower_bound Start of search time range (inclusive).
     * @param search_time_upper_bound End of search time range (inclusive).
     * @param search_termination_margin The margin used to determine the search
     * termination timestamp.
     */
    explicit Query(
            ffi::epoch_time_ms_t search_time_lower_bound,
            ffi::epoch_time_ms_t search_time_upper_bound,
            ffi::epoch_time_ms_t search_termination_margin = cDefaultSearchTerminationMargin
    )
            : m_lower_bound_ts{search_time_lower_bound},
              m_upper_bound_ts{search_time_upper_bound},
              m_search_termination_ts{
                      (cTimestampMax - search_termination_margin > search_time_upper_bound)
                              ? search_time_upper_bound + search_termination_margin
                              : cTimestampMax} {
        throw_if_ts_range_invalid();
    }

    /**
     * Constructs a new query object with the given timestamp range and a
     * wildcard list.
     * @param search_time_lower_bound Start of search time range (inclusive).
     * @param search_time_upper_bound End of search time range (inclusive).
     * @param wildcard_list A reference to the wildcard queries vector, whose
     * data will be transferred using std::move to initialize m_wildcard_list.
     * @param search_termination_margin The margin used to determine the search
     * termination timestamp. By default, this value is set to
     * cDefaultSearchTerminationMargin.
     */
    explicit Query(
            ffi::epoch_time_ms_t search_time_lower_bound,
            ffi::epoch_time_ms_t search_time_upper_bound,
            std::vector<WildcardQuery> wildcard_list,
            ffi::epoch_time_ms_t search_termination_margin = cDefaultSearchTerminationMargin
    )
            : m_lower_bound_ts{search_time_lower_bound},
              m_upper_bound_ts{search_time_upper_bound},
              m_search_termination_ts{
                      (cTimestampMax - search_termination_margin > search_time_upper_bound)
                              ? search_time_upper_bound + search_termination_margin
                              : cTimestampMax},
              m_wildcard_list{std::move(wildcard_list)} {
        throw_if_ts_range_invalid();
    }

    [[nodiscard]] auto get_lower_bound_ts() const -> ffi::epoch_time_ms_t {
        return m_lower_bound_ts;
    }

    [[nodiscard]] auto get_ts_upper_bound_ts() const -> ffi::epoch_time_ms_t {
        return m_upper_bound_ts;
    }

    /**
     * @param ts Input timestamp.
     * @return true if the given timestamp is in the search time rage bounded by
     * the lower bound and the upper bound timestamp (inclusive).
     */
    [[nodiscard]] auto ts_in_range(ffi::epoch_time_ms_t ts) const -> bool {
        return m_lower_bound_ts <= ts && ts <= m_upper_bound_ts;
    }

    /**
     * Determines whether the search can terminate by evaluating the input
     * timestamp.
     * @param ts Input timestamp.
     * @return true if the given timestamp is equal to or greater than the
     * termination timestamp.
     * @return false otherwise.
     */
    [[nodiscard]] auto terminate_search(ffi::epoch_time_ms_t ts) const -> bool {
        return m_search_termination_ts <= ts;
    }

    /**
     * Validates whether the input log message matches any of the wildcard
     * conditions for the query.
     * @param log_message Input log message.
     * @return true if the wildcard list is empty or has at least one match.
     * @return false otherwise.
     */
    [[nodiscard]] auto wildcard_matches(std::string_view log_message) const -> bool;

    /**
     * Validates whether the input log event matches the query.
     * @param log_event Input log event.
     * @return true if the timestamp is in range, and the wildcard list is empty
     * or has at least one match.
     * @return false otherwise.
     */
    [[nodiscard]] auto matches(LogEvent const& log_event) const -> bool {
        if (false == ts_in_range(log_event.get_timestamp())) {
            return false;
        }
        return wildcard_matches(log_event.get_log_message_view());
    }

private:
    /**
     * Throws an exception if the lower bound ts exceeds the upper bound ts.
     */
    auto throw_if_ts_range_invalid() const -> void {
        if (m_lower_bound_ts > m_upper_bound_ts) {
            throw ExceptionFFI(
                    ErrorCode_Unsupported,
                    __FILE__,
                    __LINE__,
                    "Search query lower bound timestamp exceeds the upper bound timestamp."
            );
        }
    }

    ffi::epoch_time_ms_t m_lower_bound_ts;
    ffi::epoch_time_ms_t m_upper_bound_ts;
    ffi::epoch_time_ms_t m_search_termination_ts;
    std::vector<WildcardQuery> m_wildcard_list;
};
}  // namespace clp_ffi_py::ir
#endif
