#include "Query.hpp"

#include <algorithm>

#include <clp/components/core/src/string_utils.hpp>

namespace clp_ffi_py::ir {
auto Query::wildcard_matches(std::string_view log_message) -> bool {
    if (m_wildcard_list.empty()) {
        return true;
    }
    return std::any_of(
            m_wildcard_list.begin(),
            m_wildcard_list.end(),
            [&](auto const& wildcard_query) {
                return wildcard_match_unsafe(log_message, wildcard_query, m_case_sensitive);
            }
    );
}
}  // namespace clp_ffi_py::ir
