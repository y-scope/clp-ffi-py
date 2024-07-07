import pickle
from typing import List

from test_ir.test_utils import TestCLPBase

from clp_ffi_py.ir import (
    LogEvent,
    Query,
)
from clp_ffi_py.wildcard_query import FullStringWildcardQuery, SubstringWildcardQuery, WildcardQuery


class TestCaseWildcardQuery(TestCLPBase):
    """
    Class for testing clp_ffi_py.wildcard_query.WildcardQuery.
    """

    def test_init(self) -> None:
        """
        Test the initialization of WildcardQuery object.
        """
        wildcard_string: str
        expected_wildcard_string: str
        wildcard_query: WildcardQuery

        wildcard_string = "Are you the lord of *Pleiades*?"
        expected_wildcard_string = wildcard_string

        wildcard_query = FullStringWildcardQuery(wildcard_string)
        self._check_wildcard_query(wildcard_query, expected_wildcard_string, False)

        wildcard_query = FullStringWildcardQuery(wildcard_string, True)
        self._check_wildcard_query(wildcard_query, expected_wildcard_string, True)

        wildcard_query = FullStringWildcardQuery(wildcard_string, case_sensitive=True)
        self._check_wildcard_query(wildcard_query, expected_wildcard_string, True)

        wildcard_query = FullStringWildcardQuery(
            case_sensitive=True, full_string_wildcard_query=wildcard_string
        )
        self._check_wildcard_query(wildcard_query, expected_wildcard_string, True)

        expected_wildcard_string = "*" + wildcard_string + "*"

        wildcard_query = SubstringWildcardQuery(wildcard_string)
        self._check_wildcard_query(wildcard_query, expected_wildcard_string, False)

        wildcard_query = SubstringWildcardQuery(wildcard_string, True)
        self._check_wildcard_query(wildcard_query, expected_wildcard_string, True)

        wildcard_query = SubstringWildcardQuery(wildcard_string, case_sensitive=True)
        self._check_wildcard_query(wildcard_query, expected_wildcard_string, True)

        wildcard_query = SubstringWildcardQuery(
            case_sensitive=True, substring_wildcard_query=wildcard_string
        )
        self._check_wildcard_query(wildcard_query, expected_wildcard_string, True)


class TestCaseQuery(TestCLPBase):
    """
    Class for testing clp_ffi_py.ir.Query.
    """

    def test_init_search_time(self) -> None:
        """
        Test the construction of Query object with the different search time range.
        """
        query: Query
        search_time_lower_bound: int
        search_time_upper_bound: int
        search_time_termination_margin: int
        exception_captured: bool

        # Note: for the default initialization, the actual search time
        # termination margin should have been set to 0, otherwise it will
        # overflow the underlying integer that stores the search termination
        # timestamp.
        query = Query()
        self._check_query(
            query,
            Query.default_search_time_lower_bound(),
            Query.default_search_time_upper_bound(),
            None,
            0,
        )

        search_time_lower_bound = 3190
        query = Query(search_time_lower_bound=search_time_lower_bound)
        self._check_query(
            query,
            search_time_lower_bound,
            Query.default_search_time_upper_bound(),
            None,
            0,
        )

        search_time_upper_bound = 3270
        query = Query(search_time_upper_bound=search_time_upper_bound)
        self._check_query(
            query,
            Query.default_search_time_lower_bound(),
            search_time_upper_bound,
            None,
            Query.default_search_time_termination_margin(),
        )

        query = Query(search_time_lower_bound, search_time_upper_bound)
        self._check_query(
            query,
            search_time_lower_bound,
            search_time_upper_bound,
            None,
            Query.default_search_time_termination_margin(),
        )

        search_time_termination_margin = 2887
        query = Query(
            search_time_upper_bound=search_time_upper_bound,
            search_time_lower_bound=search_time_lower_bound,
            search_time_termination_margin=search_time_termination_margin,
        )
        self._check_query(
            query,
            search_time_lower_bound,
            search_time_upper_bound,
            None,
            search_time_termination_margin,
        )

        search_time_lower_bound = 3270
        search_time_upper_bound = 3190
        exception_captured = False
        try:
            query = Query(search_time_lower_bound, search_time_upper_bound)  # noqa
        except RuntimeError:
            exception_captured = True
        self.assertEqual(
            exception_captured,
            True,
            "Search time lower bound is larger than the search time upper bound.",
        )

        search_time_lower_bound = 1234
        search_time_upper_bound = search_time_upper_bound
        exception_captured = False
        try:
            query = Query(search_time_lower_bound, search_time_lower_bound)
        except Exception:
            exception_captured = True
        self.assertEqual(
            exception_captured,
            False,
            "Same search time lower bound and upper bound should not trigger any exception.",
        )

    def test_init_wildcard_queries(self) -> None:
        """
        Test the construction of Query object with wildcard queries.
        """
        wildcard_queries: List[WildcardQuery]
        query: Query

        wildcard_queries = []
        query = Query(wildcard_queries=wildcard_queries)
        self._check_query(
            query,
            Query.default_search_time_lower_bound(),
            Query.default_search_time_upper_bound(),
            None,
            0,
        )

        wildcard_queries.append(WildcardQuery("who is pleiades ? I * am!", True))
        query = Query(wildcard_queries=wildcard_queries)
        self._check_query(
            query,
            Query.default_search_time_lower_bound(),
            Query.default_search_time_upper_bound(),
            wildcard_queries,
            0,
        )

        wildcard_queries.append(WildcardQuery("who is pleiades * I am ?"))
        query = Query(wildcard_queries=wildcard_queries)
        self._check_query(
            query,
            Query.default_search_time_lower_bound(),
            Query.default_search_time_upper_bound(),
            wildcard_queries,
            0,
        )

        wildcard_queries = [
            WildcardQuery("who is \*** pleiades??\\"),
            WildcardQuery("a\?m********I?\\"),
            WildcardQuery("\g\%\*\??***"),
        ]
        ref_wildcard_queries = [
            WildcardQuery("who is \** pleiades??"),
            WildcardQuery("a\?m*I?"),
            WildcardQuery("g%\*\??*"),
        ]
        query = Query(wildcard_queries=wildcard_queries)
        self._check_query(
            query,
            Query.default_search_time_lower_bound(),
            Query.default_search_time_upper_bound(),
            ref_wildcard_queries,
            0,
        )

        search_time_lower_bound: int = 3190
        search_time_upper_bound: int = 3270
        search_time_termination_margin: int = 9974
        query = Query(
            search_time_lower_bound=search_time_lower_bound,
            search_time_upper_bound=search_time_upper_bound,
            search_time_termination_margin=search_time_termination_margin,
            wildcard_queries=wildcard_queries,
        )
        self._check_query(
            query,
            search_time_lower_bound,
            search_time_upper_bound,
            ref_wildcard_queries,
            search_time_termination_margin,
        )

    def test_pickle(self) -> None:
        """
        Test the reconstruction of Query object from pickling data.
        """
        wildcard_queries: List[WildcardQuery]
        query: Query
        reconstructed_query: Query

        query = Query()
        reconstructed_query = pickle.loads(pickle.dumps(query))
        self._check_query(
            reconstructed_query,
            query.get_search_time_lower_bound(),
            query.get_search_time_upper_bound(),
            query.get_wildcard_queries(),
            query.get_search_time_termination_margin(),
        )

        wildcard_queries = [
            WildcardQuery("who is \*** pleiades??\\"),
            WildcardQuery("a\?m********I?\\"),
            WildcardQuery("\g\%\*\??***"),
        ]
        query = Query(
            search_time_lower_bound=3190,
            search_time_upper_bound=3270,
            search_time_termination_margin=9974,
            wildcard_queries=wildcard_queries,
        )
        reconstructed_query = pickle.loads(pickle.dumps(query))
        self._check_query(
            reconstructed_query,
            query.get_search_time_lower_bound(),
            query.get_search_time_upper_bound(),
            query.get_wildcard_queries(),
            query.get_search_time_termination_margin(),
        )

    def test_log_event_match(self) -> None:
        """
        Test the match between a Query object and a LogEvent object.
        """
        query: Query
        log_event: LogEvent
        description: str
        wildcard_query_string: str

        description = "Any log event should match the empty query."
        log_event = LogEvent("whatever", 1234567890)
        query = Query()
        self.assertEqual(query.match_log_event(log_event), True, description)
        self.assertEqual(log_event.match_query(query), True, description)
        query = Query(wildcard_queries=[])
        self.assertEqual(query.match_log_event(log_event), True, description)
        self.assertEqual(log_event.match_query(query), True, description)

        description = "Only log events whose timestamp within the query's should match the query."
        query = Query(
            search_time_lower_bound=19990723,
            search_time_upper_bound=20310723,
            wildcard_queries=[WildcardQuery("*")],
        )
        log_event = LogEvent("whatever you want: in range", 20131102)
        self.assertEqual(query.match_log_event(log_event), True, description)
        self.assertEqual(log_event.match_query(query), True, description)
        log_event = LogEvent("whatever you want: lower than the lower bound", 10131102)
        self.assertEqual(query.match_log_event(log_event), False, description)
        self.assertEqual(log_event.match_query(query), False, description)
        log_event = LogEvent("whatever you want: higher than the higher bound", 31131102)
        self.assertEqual(query.match_log_event(log_event), False, description)
        self.assertEqual(log_event.match_query(query), False, description)
        query = Query(search_time_lower_bound=1024, search_time_upper_bound=1024)
        log_event = LogEvent("whatever you want: exact in bound (inclusive)", 1024)
        self.assertEqual(query.match_log_event(log_event), True, description)
        self.assertEqual(log_event.match_query(query), True, description)

        description = (
            "Only log events whose message matches the wildcard query should match the query."
        )
        log_event = LogEvent("fhakjhLFISHfashfShfiuSLSZkfSUSFS", 0)
        wildcard_query_string = "*JHlfish*SH?IU*s"
        query = Query(wildcard_queries=[WildcardQuery(wildcard_query_string)])
        self.assertEqual(query.match_log_event(log_event), True, description)
        self.assertEqual(log_event.match_query(query), True, description)
        query = Query(wildcard_queries=[WildcardQuery(wildcard_query_string, True)])
        self.assertEqual(query.match_log_event(log_event), False, description)
        self.assertEqual(log_event.match_query(query), False, description)
        log_event = LogEvent("j:flJo;jsf:LSJDFoiASFoasjzFZA", 0)
        wildcard_query_string = "*flJo*s?*AS*A"
        query = Query(wildcard_queries=[WildcardQuery(wildcard_query_string)])
        self.assertEqual(query.match_log_event(log_event), True, description)
        self.assertEqual(log_event.match_query(query), True, description)
        query = Query(wildcard_queries=[WildcardQuery(wildcard_query_string, True)])
        self.assertEqual(query.match_log_event(log_event), True, description)
        self.assertEqual(log_event.match_query(query), True, description)

        description = (
            "Log event whose messages matches any one of the wildcard queries should be considered"
            " as a match of the query."
        )
        wildcard_queries: List[WildcardQuery] = [WildcardQuery("*b&A*"), WildcardQuery("*A|a*")]
        log_event = LogEvent("-----a-A-----", 0)
        query = Query(wildcard_queries=wildcard_queries)
        self.assertEqual(query.match_log_event(log_event), False, description)
        self.assertEqual(log_event.match_query(query), False, description)
        wildcard_queries.append(WildcardQuery("*a?a*"))
        query = Query(wildcard_queries=wildcard_queries)
        self.assertEqual(query.match_log_event(log_event), True, description)
        self.assertEqual(log_event.match_query(query), True, description)
        log_event = LogEvent("-----B&a_____", 0)
        query = Query(wildcard_queries=wildcard_queries)
        self.assertEqual(query.match_log_event(log_event), True, description)
        self.assertEqual(log_event.match_query(query), True, description)

        description = (
            "The match of query requires both timestamp in range and log message matching any one"
            " of the wildcard queries."
        )
        query = Query(
            search_time_lower_bound=3190,
            search_time_upper_bound=3270,
            wildcard_queries=[WildcardQuery("*q?Q*"), WildcardQuery("*t?t*", True)],
        )
        log_event = LogEvent("I'm not matching anything...", 3213)
        self.assertEqual(query.match_log_event(log_event), False, description)
        self.assertEqual(log_event.match_query(query), False, description)
        log_event = LogEvent("I'm not matching anything... T.T", 3213)
        self.assertEqual(query.match_log_event(log_event), False, description)
        self.assertEqual(log_event.match_query(query), False, description)
        log_event = LogEvent("I'm not matching anything... QAQ", 2887)
        self.assertEqual(query.match_log_event(log_event), False, description)
        self.assertEqual(log_event.match_query(query), False, description)
        log_event = LogEvent("I'm finally matching something... QAQ", 3213)
        self.assertEqual(query.match_log_event(log_event), True, description)
        self.assertEqual(log_event.match_query(query), True, description)
