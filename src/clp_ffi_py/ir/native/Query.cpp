#include "Query.hpp"

#include <algorithm>
#include <string_view>

#include <clp/components/core/src/clp/string_utils/string_utils.hpp>

namespace clp_ffi_py::ir::native {
auto Query::matches_wildcard_queries(std::string_view log_message) const -> bool {
    if (m_wildcard_queries.empty()) {
        return true;
    }
    return std::any_of(
            m_wildcard_queries.begin(),
            m_wildcard_queries.end(),
            [&](auto const& wildcard_query) {
                return clp::string_utils::wildcard_match_unsafe(
                        log_message,
                        wildcard_query.get_wildcard_query(),
                        wildcard_query.is_case_sensitive()
                );
            }
    );
}
}  // namespace clp_ffi_py::ir::native
