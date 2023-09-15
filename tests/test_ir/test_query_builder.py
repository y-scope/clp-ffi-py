from typing import List, Optional

from test_ir.test_utils import TestCLPBase

from clp_ffi_py.ir import (
    Query,
    QueryBuilder,
    QueryBuilderException,
)
from clp_ffi_py.wildcard_query import WildcardQuery


class TestCaseQueryBuilder(TestCLPBase):
    """
    Class for testing clp_ffi_py.ir.QueryBuilder.
    """

    def test_init(self) -> None:
        """
        Tests the default initialized Query Builder and its behavior.
        """
        query_builder: QueryBuilder = QueryBuilder()
        empty_query: Query = Query()
        attributes_exception_captured: bool

        self._check_query(
            query_builder.build(),
            empty_query.get_search_time_lower_bound(),
            empty_query.get_search_time_upper_bound(),
            empty_query.get_wildcard_queries(),
            empty_query.get_search_time_termination_margin(),
        )

        attributes_exception_captured = False
        try:
            query_builder.search_time_lower_bound = 0  # type: ignore
        except AttributeError:
            attributes_exception_captured = True
        except Exception:
            pass
        self.assertTrue(
            attributes_exception_captured, "search time lower bound should be read-only"
        )

        attributes_exception_captured = False
        try:
            query_builder.search_time_upper_bound = 0  # type: ignore
        except AttributeError:
            attributes_exception_captured = True
        except Exception:
            pass
        self.assertTrue(
            attributes_exception_captured, "search time upper bound should be read-only"
        )

        attributes_exception_captured = False
        try:
            query_builder.search_time_termination_margin = 0  # type: ignore
        except AttributeError:
            attributes_exception_captured = True
        except Exception:
            pass
        self.assertTrue(
            attributes_exception_captured, "search time termination margin should be read-only"
        )

        query_builder.wildcard_queries.append(WildcardQuery(""))
        self.assertEqual(
            len(query_builder.wildcard_queries), 0, "The query list should be size of 0"
        )

    def test_set_value(self) -> None:
        """
        Tests QueryBuilder by building different Query objects.
        """
        search_time_lower_bound: int
        search_time_upper_bound: int
        search_time_termination_margin: int
        wildcard_queries: Optional[List[WildcardQuery]]
        query_builder: QueryBuilder = QueryBuilder()

        search_time_lower_bound = Query.default_search_time_lower_bound()
        search_time_upper_bound = Query.default_search_time_upper_bound()
        search_time_termination_margin = Query.default_search_time_termination_margin()
        wildcard_queries = None
        query_builder.set_search_time_lower_bound(search_time_lower_bound)
        self._check_query(
            query_builder.build(),
            search_time_lower_bound,
            search_time_upper_bound,
            wildcard_queries,
            0,
        )

        search_time_upper_bound = 3270
        query_builder.set_search_time_upper_bound(search_time_upper_bound)
        self._check_query(
            query_builder.build(),
            search_time_lower_bound,
            search_time_upper_bound,
            wildcard_queries,
            search_time_termination_margin,
        )

        search_time_termination_margin = 123
        query_builder.set_search_time_termination_margin(search_time_termination_margin)
        self._check_query(
            query_builder.build(),
            search_time_lower_bound,
            search_time_upper_bound,
            wildcard_queries,
            search_time_termination_margin,
        )

        wildcard_queries = [WildcardQuery("aaa*aaa"), WildcardQuery("bbb*bbb", True)]
        for wildcard_query in wildcard_queries:
            query_builder.add_wildcard_query(
                wildcard_query.wildcard_query, wildcard_query.case_sensitive
            )
        extra_wildcard_queries = [WildcardQuery("ccc?ccc", True), WildcardQuery("ddd?ddd")]
        query_builder.add_wildcard_queries(extra_wildcard_queries)
        wildcard_queries.extend(extra_wildcard_queries)
        self._check_query(
            query_builder.build(),
            search_time_lower_bound,
            search_time_upper_bound,
            wildcard_queries,
            search_time_termination_margin,
        )

        search_time_lower_bound = 16384
        search_time_upper_bound = 16384
        query_builder = query_builder.set_search_time_lower_bound(
            search_time_lower_bound
        ).set_search_time_upper_bound(search_time_upper_bound)
        self._check_query(
            query_builder.build(),
            search_time_lower_bound,
            search_time_upper_bound,
            wildcard_queries,
            search_time_termination_margin,
        )

        search_time_termination_margin = 3670
        wildcard_query_string: str = "eee*?*eee"
        query_builder.set_search_time_termination_margin(
            search_time_termination_margin
        ).add_wildcard_query(wildcard_query_string)
        wildcard_queries.append(WildcardQuery(wildcard_query_string))
        self._check_query(
            query_builder.build(),
            search_time_lower_bound,
            search_time_upper_bound,
            wildcard_queries,
            search_time_termination_margin,
        )

        search_time_lower_bound = 12345
        search_time_upper_bound = 67890
        search_time_termination_margin = 0
        query_builder = (
            query_builder.reset_search_time_lower_bound()
            .set_search_time_lower_bound(search_time_lower_bound)
            .reset_search_time_termination_margin()
            .set_search_time_termination_margin(search_time_termination_margin)
            .reset_search_time_upper_bound()
            .set_search_time_upper_bound(search_time_upper_bound)
        )
        self._check_query(
            query_builder.build(),
            search_time_lower_bound,
            search_time_upper_bound,
            wildcard_queries,
            search_time_termination_margin,
        )

        query_builder.reset_wildcard_queries()
        query_builder.add_wildcard_query(wildcard_query_string)
        wildcard_queries = [WildcardQuery(wildcard_query_string)]
        self._check_query(
            query_builder.build(),
            search_time_lower_bound,
            search_time_upper_bound,
            wildcard_queries,
            search_time_termination_margin,
        )

        query_builder = query_builder.reset()
        self._check_query(
            query_builder.build(),
            Query.default_search_time_lower_bound(),
            Query.default_search_time_upper_bound(),
            None,
            0,
        )

    def test_exception(self) -> None:
        """
        Tests whether QueryBuilderException is triggered as expected.
        """
        query_builder: QueryBuilder = QueryBuilder()
        search_time_lower_bound: int = 3270
        search_time_upper_bound: int = 3190
        query_builder.set_search_time_lower_bound(search_time_lower_bound)
        query_builder.set_search_time_upper_bound(search_time_upper_bound)

        query_builder_exception_captured: bool = False
        try:
            query_builder.build()
        except QueryBuilderException:
            query_builder_exception_captured = True
        except Exception:
            pass
        self.assertTrue(
            query_builder_exception_captured,
            "QueryBuilderException is not triggered when the search time lower bound exceeds the"
            " search time upper bound",
        )
