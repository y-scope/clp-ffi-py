from __future__ import annotations

from typing import List, Optional
from clp_ffi_py.ir import Query
from clp_ffi_py.wildcard_query import WildcardQuery


class QueryBuilderException(Exception):
    """
    This exception will be raised when illegal parameters are used to build a
    Query object.
    """

    pass


class QueryBuilder:
    """
    This class serves as an interface for conveniently constructing Query
    objects utilized in CLP IR streaming search. It provides methods for
    configuring and resetting search parameters, which encompass:
        - search_time_lower_bound: Start of search time range (inclusive).
        - search_time_upper_bound: End of search time range (inclusive).
        - search_time_termination_margin: The margin used to determine the
          search termination timestamp.
        - wildcard_queries: A list of wildcard search queries.

    Timestamps are specified using Unix epoch timestamps in milliseconds. By
    default, the lower and upper bounds for search timestamps encompass the
    entire valid range of Unix epoch timestamps, while the wildcard queries list
    is empty.

    For more details about the search query CLP IR stream supports, please read
    the documents of `clp_ffi_py.ir.Query` and
    `clp_ffi_py.wildcard_query.WildcardQuery`.
    """

    def __init__(self):
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
    def wild_card_queries(self) -> List[WildcardQuery]:
        return self._wildcard_queries

    def set_search_time_lower_bound(self, ts: int) -> QueryBuilder:
        """
        Sets the search time lower bound.

        :param ts: The timestamp of search time lower bound.
        :return: self.
        """
        self._search_time_lower_bound = ts
        return self

    def set_search_time_upper_bound(self, ts: int) -> QueryBuilder:
        """
        Sets the search time upper bound.

        :param ts: The timestamp of search time upper bound.
        :return: self.
        """
        self._search_time_upper_bound = ts
        return self

    def set_search_time_termination_margin(self, ts: int) -> QueryBuilder:
        """
        Sets the search time termination margin.

        :param ts: The timestamp of search time termination margin.
        :return: self.
        """
        self._search_time_termination_margin = ts
        return self

    def add_wildcard_query(self, wildcard_query: str, case_sensitive: bool = False) -> QueryBuilder:
        """
        Adds a wildcard query to the builder's wildcard query list.

        :param wildcard_query: The wildcard query string to add.
        :param case_sensitive: Case sensitive indicator.
        :return: self.
        """
        self._wildcard_queries.append(WildcardQuery(wildcard_query, case_sensitive))
        return self

    def add_wildcard_queries(self, wildcard_queries: List[WildcardQuery]) -> QueryBuilder:
        """
        Adds a list wildcard queries to the builder's wildcard query list.

        :param wildcard_queries: The list of wildcard queries to be added.
        :return: self.
        """
        self._wildcard_queries.extend(wildcard_queries)
        return self

    def reset_search_time_lower_bound(self) -> QueryBuilder:
        """
        Resets the search time lower bound to the default value (the smallest
        legal Unix epoch timestamp).

        :return: self.
        """
        self._search_time_lower_bound = Query.default_search_time_lower_bound()
        return self

    def reset_search_time_upper_bound(self) -> QueryBuilder:
        """
        Resets the search time upper bound to the default value (the largest
        legal Unix epoch timestamp).

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
        Resets the wildcard queries to an empty list.

        :return: self.
        """
        self._wildcard_queries.clear()
        return self

    def reset(self) -> QueryBuilder:
        """
        Resets all the settings to the default.

        :return: self.
        """
        return (
            self.reset_wildcard_queries()
            .reset_search_time_termination_margin()
            .reset_search_time_upper_bound()
            .reset_search_time_lower_bound()
        )

    def build_query(self) -> Query:
        """
        :return: A new reference of a Query object initialized with the search
            parameters set by the builder.
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
