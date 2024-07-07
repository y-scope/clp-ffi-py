from __future__ import annotations

import warnings
from copy import deepcopy
from typing import List, Optional, overload, Union

from deprecated.sphinx import deprecated

from clp_ffi_py.ir.native import Query
from clp_ffi_py.wildcard_query import FullStringWildcardQuery, WildcardQuery

_add_wildcard_query_deprecation_warning_message: str = "Instead, use :meth:`add_wildcard_query`"
" with either a :class:`~clp_ffi_py.wildcard_query.FullStringWildcardQuery` or "
" :class:`~clp_ffi_py.wildcard_query.SubstringWildcardQuery`."


class QueryBuilderException(Exception):
    """
    Exception raised when building a :class:`~clp_ffi_py.ir.native.Query` fails.
    """

    pass


class QueryBuilder:
    """
    This class serves as an interface for conveniently constructing Query objects utilized in CLP IR
    streaming search. It provides methods for configuring and resetting search parameters.

    For more details about the search query CLP IR stream supports, see
    :class:`~clp_ffi_py.ir.native.Query` and :class:`~clp_ffi_py.wildcard_query.WildcardQuery`.
    """

    def __init__(self) -> None:
        self._search_time_lower_bound: int = Query.default_search_time_lower_bound()
        self._search_time_upper_bound: int = Query.default_search_time_upper_bound()
        self._search_time_termination_margin: int = Query.default_search_time_termination_margin()
        self._wildcard_queries: List[WildcardQuery] = []

    @property
    def search_time_lower_bound(self) -> int:
        return self._search_time_lower_bound

    @property
    def search_time_upper_bound(self) -> int:
        return self._search_time_upper_bound

    @property
    def search_time_termination_margin(self) -> int:
        return self._search_time_termination_margin

    @property
    def wildcard_queries(self) -> List[WildcardQuery]:
        """
        :return: A deep copy of the underlying wildcard query list.
        """
        return deepcopy(self._wildcard_queries)

    def set_search_time_lower_bound(self, ts: int) -> QueryBuilder:
        """
        :param ts: Start of the search time range (inclusive) as a UNIX epoch timestamp in
            milliseconds.
        :return: self.
        """
        self._search_time_lower_bound = ts
        return self

    def set_search_time_upper_bound(self, ts: int) -> QueryBuilder:
        """
        :param ts: End of the search time range (inclusive) as a UNIX epoch timestamp in
            milliseconds.
        :return: self.
        """
        self._search_time_upper_bound = ts
        return self

    def set_search_time_termination_margin(self, ts: int) -> QueryBuilder:
        """
        :param ts: The search time termination margin as a UNIX epoch timestamp in milliseconds.
        :return: self.
        """
        self._search_time_termination_margin = ts
        return self

    @overload
    @deprecated(
        version="0.0.12",
        reason=_add_wildcard_query_deprecation_warning_message,
    )
    def add_wildcard_query(self, wildcard_query: str, case_sensitive: bool = False) -> QueryBuilder:
        """
        Constructs and adds a :class:`~clp_ffi_py.wildcard_query.WildcardQuery` to the wildcard
        query list.

        :param wildcard_query: The wildcard query string to add.
        :param case_sensitive: Whether to perform case-sensitive matching.
        :return: self.
        """
        ...

    @overload
    def add_wildcard_query(self, wildcard_query: WildcardQuery) -> QueryBuilder:
        """
        Adds the given wildcard query to the wildcard query list.

        :param wildcard_query: The wildcard query to add. It can be any derived class of
            :class:`~clp_ffi_py.wildcard_query.WildcardQuery`.
        :return: self.
        """
        ...

    def add_wildcard_query(
        self, wildcard_query: Union[str, WildcardQuery], case_sensitive: bool = False
    ) -> QueryBuilder:
        if isinstance(wildcard_query, WildcardQuery):
            self._wildcard_queries.append(wildcard_query)
            return self
        elif isinstance(wildcard_query, str):
            warnings.warn(
                _add_wildcard_query_deprecation_warning_message,
                DeprecationWarning,
            )
            self._wildcard_queries.append(FullStringWildcardQuery(wildcard_query, case_sensitive))
            return self

        raise NotImplementedError

    def add_wildcard_queries(self, wildcard_queries: List[WildcardQuery]) -> QueryBuilder:
        """
        Adds a list of wildcard queries to the wildcard query list.

        :param wildcard_queries: The list of wildcard queries to add.
        :return: self.
        """
        self._wildcard_queries.extend(wildcard_queries)
        return self

    def reset_search_time_lower_bound(self) -> QueryBuilder:
        """
        Resets the search time lower bound to the default value.

        :return: self.
        """
        self._search_time_lower_bound = Query.default_search_time_lower_bound()
        return self

    def reset_search_time_upper_bound(self) -> QueryBuilder:
        """
        Resets the search time upper bound to the default value.

        :return: self.
        """
        self._search_time_upper_bound = Query.default_search_time_upper_bound()
        return self

    def reset_search_time_termination_margin(self) -> QueryBuilder:
        """
        Resets the search time termination margin to the default value.

        :return: self.
        """
        self._search_time_termination_margin = Query.default_search_time_termination_margin()
        return self

    def reset_wildcard_queries(self) -> QueryBuilder:
        """
        Clears the wildcard query list.

        :return: self.
        """
        self._wildcard_queries.clear()
        return self

    def reset(self) -> QueryBuilder:
        """
        Resets all settings to their defaults.

        :return: self.
        """
        return (
            self.reset_wildcard_queries()
            .reset_search_time_termination_margin()
            .reset_search_time_upper_bound()
            .reset_search_time_lower_bound()
        )

    def build(self) -> Query:
        """
        :raises QueryBuilderException: If the search time range lower bound exceeds the search time
            range upper bound.
        :return: A :class:`~clp_ffi_py.ir.native.Query` object initialized with the parameters set
            by the builder.
        """
        if self._search_time_lower_bound > self._search_time_upper_bound:
            raise QueryBuilderException(
                "The search time lower bound exceeds the search time upper bound."
            )
        wildcard_queries: Optional[List[WildcardQuery]] = None
        if 0 != len(self._wildcard_queries):
            wildcard_queries = self._wildcard_queries
        return Query(
            search_time_lower_bound=self._search_time_lower_bound,
            search_time_upper_bound=self._search_time_upper_bound,
            search_time_termination_margin=self._search_time_termination_margin,
            wildcard_queries=wildcard_queries,
        )
