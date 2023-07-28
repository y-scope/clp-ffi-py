import dateutil.tz
import pickle
import unittest

from clp_ffi_py import FourByteEncoder, LogEvent, Metadata, Query, WildcardQuery
from datetime import tzinfo
from typing import Optional, List


class TestCaseMetadata(unittest.TestCase):
    """
    Class for testing clp_ffi_py.ir.Metadata.
    """

    def test_init(self) -> None:
        """
        Test the initialization of Metadata object without using keyword.
        """
        ref_timestamp: int = 2005689603190
        timestamp_format: str = "yy/MM/dd HH:mm:ss"
        timezone_id: str = "America/Chicago"
        metadata: Metadata = Metadata(ref_timestamp, timestamp_format, timezone_id)
        self.__check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

    def test_keyword_init(self) -> None:
        """
        Test the initialization of Metadata object using keyword.
        """
        ref_timestamp: int = 2005689603270
        timestamp_format: str = "MM/dd/yy HH:mm:ss"
        timezone_id: str = "America/New_York"
        metadata: Metadata

        metadata = Metadata(
            ref_timestamp=ref_timestamp, timestamp_format=timestamp_format, timezone_id=timezone_id
        )
        self.__check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

        metadata = Metadata(
            timestamp_format=timestamp_format, ref_timestamp=ref_timestamp, timezone_id=timezone_id
        )
        self.__check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

    def test_timezone_new_reference(self) -> None:
        """
        Test the timezone is a new reference returned.
        """
        ref_timestamp: int = 2005689603190
        timestamp_format: str = "yy/MM/dd HH:mm:ss"
        timezone_id: str = "America/Chicago"
        metadata: Metadata = Metadata(ref_timestamp, timestamp_format, timezone_id)
        self.__check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

        wrong_tz: Optional[tzinfo] = metadata.get_timezone()
        self.assertEqual(wrong_tz is metadata.get_timezone(), True)

        wrong_tz = dateutil.tz.gettz("America/New_York")
        self.assertEqual(wrong_tz is not metadata.get_timezone(), True)

        self.__check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

    def __check_metadata(
        self,
        metadata: Metadata,
        expected_ref_timestamp: int,
        expected_timestamp_format: str,
        expected_timezone_id: str,
    ) -> None:
        """
        Given a Metadata object, check if the content matches the reference.

        :param metadata: Metadata object to be checked.
        :param expected_ref_timestamp: Expected reference timestamp.
        :param expected_timestamp_format: Expected timestamp format.
        :param expected_timezone_id: Expected timezone ID.
        """
        ref_timestamp: int = metadata.get_ref_timestamp()
        timestamp_format: str = metadata.get_timestamp_format()
        timezone_id: str = metadata.get_timezone_id()
        timezone: tzinfo = metadata.get_timezone()

        self.assertEqual(
            ref_timestamp,
            expected_ref_timestamp,
            f'Reference Timestamp: "{ref_timestamp}", Expected: "{expected_ref_timestamp}"',
        )
        self.assertEqual(
            timestamp_format,
            expected_timestamp_format,
            f'Timestamp Format: "{timestamp_format}", Expected: "{expected_timestamp_format}"',
        )
        self.assertEqual(
            timezone_id,
            expected_timezone_id,
            f'Timezone ID: "{timezone_id}", Expected: "{expected_timezone_id}"',
        )

        expected_tzinfo: Optional[tzinfo] = dateutil.tz.gettz(expected_timezone_id)
        assert expected_tzinfo is not None
        is_the_same_tz: bool = expected_tzinfo is timezone
        self.assertEqual(
            is_the_same_tz,
            True,
            f"Timezone does not match timezone id. Timezone ID: {timezone_id}, Timezone:"
            f' {str(timezone)}"',
        )


class TestCaseLogEvent(unittest.TestCase):
    """
    Class for testing clp_ffi_py.ir.LogEvent.
    """

    def test_init(self) -> None:
        """
        Test the initialization of LogEvent object without using keyword.
        """
        log_message: str = " This is a test log message"
        timestamp: int = 2005689603190
        idx: int = 3270
        metadata: Optional[Metadata] = None

        log_event: LogEvent

        log_event = LogEvent(log_message, timestamp, idx, metadata)
        self.__check_log_event(log_event, log_message, timestamp, idx)

        log_event = LogEvent(log_message, timestamp)
        self.__check_log_event(log_event, log_message, timestamp, 0)

    def test_keyword_init(self) -> None:
        """
        Test the initialization of LogEvent object using keyword.
        """
        log_message: str = " This is a test log message"
        timestamp: int = 932724000000
        idx: int = 14111813
        metadata: Optional[Metadata] = None

        # Initialize with keyword (in-order)
        log_event = LogEvent(
            log_message=log_message, timestamp=timestamp, index=idx, metadata=metadata
        )
        self.__check_log_event(log_event, log_message, timestamp, idx)

        # Initialize with keyword (out-of-order)
        log_event = LogEvent(
            index=idx, timestamp=timestamp, log_message=log_message, metadata=metadata
        )
        self.__check_log_event(log_event, log_message, timestamp, idx)

        # Initialize with keyword and default argument (out-of-order)
        log_event = LogEvent(timestamp=timestamp, log_message=log_message)
        self.__check_log_event(log_event, log_message, timestamp, 0)

    def test_formatted_message(self) -> None:
        """
        Test the reconstruction of the raw message.

        In particular, it checks if the timestamp is properly formatted with the
        expected tzinfo
        """
        log_message: str = " This is a test log message"
        timestamp: int = 932724000000
        idx: int = 3190
        metadata: Optional[Metadata] = Metadata(0, "yy/MM/dd HH:mm:ss", "Asia/Hong_Kong")
        log_event: LogEvent
        expected_formatted_message: str
        formatted_message: str

        # If metadata is given, use the metadata's timezone as default
        log_event = LogEvent(
            log_message=log_message, timestamp=timestamp, index=idx, metadata=metadata
        )
        self.__check_log_event(log_event, log_message, timestamp, idx)
        expected_formatted_message = f"1999-07-23 18:00:00.000+08:00{log_message}"
        formatted_message = log_event.get_formatted_message()

        self.assertEqual(
            formatted_message,
            expected_formatted_message,
            f"Raw message: {formatted_message}; Expected: {expected_formatted_message}",
        )

        # If metadata is given but another timestamp is specified, use the given
        # timestamp
        test_tz: Optional[tzinfo] = dateutil.tz.gettz("America/New_York")
        assert test_tz is not None
        expected_formatted_message = f"1999-07-23 06:00:00.000-04:00{log_message}"
        formatted_message = log_event.get_formatted_message(test_tz)
        self.assertEqual(
            formatted_message,
            expected_formatted_message,
            f"Raw message: {formatted_message}; Expected: {expected_formatted_message}",
        )

        # If the metadata is initialized as None, and no tzinfo passed in, UTC
        # will be used as default
        log_event = LogEvent(log_message=log_message, timestamp=timestamp, index=idx, metadata=None)
        self.__check_log_event(log_event, log_message, timestamp, idx)
        expected_formatted_message = f"1999-07-23 10:00:00.000+00:00{log_message}"
        formatted_message = log_event.get_formatted_message()
        self.assertEqual(
            formatted_message,
            expected_formatted_message,
            f"Raw message: {formatted_message}; Expected: {expected_formatted_message}",
        )

    def test_pickle(self) -> None:
        """
        Test the reconstruction of LogEvent object from pickling data.

        For unpickled LogEvent object, even though the metadata is set to None,
        it should still format the timestamp with the original tz before
        pickling
        """
        log_message: str = " This is a test log message"
        timestamp: int = 932724000000
        idx: int = 3190
        metadata: Optional[Metadata] = Metadata(0, "yy/MM/dd HH:mm:ss", "Asia/Hong_Kong")
        log_event = LogEvent(
            log_message=log_message, timestamp=timestamp, index=idx, metadata=metadata
        )
        self.__check_log_event(log_event, log_message, timestamp, idx)
        reconstructed_log_event: LogEvent = pickle.loads(pickle.dumps(log_event))
        self.__check_log_event(reconstructed_log_event, log_message, timestamp, idx)

        # For unpickled LogEvent object, even though the metadata is set to
        # None, it should still format the timestamp with the original tz before
        # pickling
        expected_formatted_message = f"1999-07-23 18:00:00.000+08:00{log_message}"
        formatted_message = reconstructed_log_event.get_formatted_message()
        self.assertEqual(
            formatted_message,
            expected_formatted_message,
            f"Raw message: {formatted_message}; Expected: {expected_formatted_message}",
        )

        # If we pickle it again, we should still have the same results
        reconstructed_log_event = pickle.loads(pickle.dumps(reconstructed_log_event))
        formatted_message = reconstructed_log_event.get_formatted_message()
        self.assertEqual(
            formatted_message,
            expected_formatted_message,
            f"Raw message: {formatted_message}; Expected: {expected_formatted_message}",
        )

    def __check_log_event(
        self,
        log_event: LogEvent,
        expected_log_message: str,
        expected_timestamp: int,
        expected_idx: int,
    ) -> None:
        """
        Given a LogEvent object, check if the content matches the reference.

        :param log_event: LogEvent object to be checked.
        :param expected_log_message: Expected log message.
        :param expected_timestamp: Expected timestamp.
        :param expected_idx: Expected log event index.
        """
        log_message: str = log_event.get_log_message()
        timestamp: int = log_event.get_timestamp()
        idx: int = log_event.get_index()
        self.assertEqual(
            log_message,
            expected_log_message,
            f'Log message: "{log_message}", Expected: "{expected_log_message}"',
        )
        self.assertEqual(
            timestamp,
            expected_timestamp,
            f'Timestamp: "{timestamp}", Expected: "{expected_log_message}"',
        )
        self.assertEqual(idx, expected_idx, f"Message idx: {idx}, Expected: {expected_idx}")


class TestCaseFourByteEncoder(unittest.TestCase):
    """
    Class for testing clp_ffi_py.ir.FourByteEncoder.

    The actual functionality should also be covered by the unittest of CLP
    Python logging library.
    TODO: When the decoder is implemented, add some more tests to ensure the
    encoded bytes can be successfully decoded to recover the original log event.
    """

    def test_init(self) -> None:
        type_error_exception_captured: bool = False
        four_byte_encoder: FourByteEncoder
        try:
            four_byte_encoder = FourByteEncoder()  # noqa
        except TypeError:
            type_error_exception_captured = True
        self.assertEqual(
            type_error_exception_captured, True, "FourByteEncoder should be non-instantiable."
        )

    def test_encoding_methods_consistency(self) -> None:
        """
        This test checks if the result of encode_message_and_timestamp_delta is
        consistent with the combination of encode_message and
        encode_timestamp_delta.
        """
        timestamp_delta: int = -3190
        log_message: str = "This is a test message: Do NOT Reply!"
        encoded_message_and_ts_delta: bytearray = (
            FourByteEncoder.encode_message_and_timestamp_delta(
                timestamp_delta, log_message.encode()
            )
        )
        encoded_message: bytearray = FourByteEncoder.encode_message(log_message.encode())
        encoded_ts_delta: bytearray = FourByteEncoder.encode_timestamp_delta(timestamp_delta)
        self.assertEqual(encoded_message_and_ts_delta, encoded_message + encoded_ts_delta)


class TestCaseQueryBase(unittest.TestCase):
    """
    Base class for Query related testing.

    Helper functions should be defined in this class. This base class shouldn't
    be launched during the testing.
    """

    def _check_wildcard_query(
        self, wildcard_query: WildcardQuery, ref_wildcard_string: str, ref_is_case_sensitive: bool
    ) -> None:
        """
        Given a WildcardQuery object, check if the stored data matches the input
        reference.

        :param self
        :param wildcard_query: Input WildcardQuery object.
        :param ref_wildcard_string: Reference wildcard string.
        :param ref_is_case_sensitive: Reference case-sensitive indicator.
        """
        wildcard_string: str = wildcard_query.wildcard_query
        is_case_sensitive: bool = wildcard_query.case_sensitive
        self.assertEqual(
            wildcard_string,
            ref_wildcard_string,
            f'Wildcard string: "{wildcard_string}"; Expected: "{ref_wildcard_string}"',
        )
        self.assertEqual(
            is_case_sensitive,
            ref_is_case_sensitive,
            f"Expected case-sensitive indicator: {ref_is_case_sensitive}",
        )

    def _check_query(
        self,
        query: Query,
        ref_search_time_lower_bound: int,
        ref_search_time_upper_bound: int,
        ref_wildcard_queries: Optional[List[WildcardQuery]],
        ref_search_time_termination_margin: int,
    ) -> None:
        """
        Given a Query object, check if the stored data matches the input
        references.

        :param self
        :param query: Input Query object to validate.
        :param ref_search_time_lower_bound: Reference search time lower bound.
        :param ref_search_time_upper_bound: Reference search time upper bound.
        :param ref_wildcard_queries: Reference wildcard query list.
        :param ref_search_time_termination_margin: Reference search time
            termination margin.
        """
        search_time_lower_bound: int = query.get_search_time_lower_bound()
        search_time_upper_bound: int = query.get_search_time_upper_bound()
        wildcard_queries: Optional[List[WildcardQuery]] = query.get_wildcard_queries()
        search_time_termination_margin: int = query.get_search_time_termination_margin()
        self.assertEqual(
            search_time_lower_bound,
            ref_search_time_lower_bound,
            f"Search time lower bound: {search_time_lower_bound}; Expected:"
            f" {ref_search_time_lower_bound}",
        )
        self.assertEqual(
            search_time_upper_bound,
            ref_search_time_upper_bound,
            f"Search time upper bound: {search_time_upper_bound}; Expected:"
            f" {ref_search_time_upper_bound}",
        )
        self.assertEqual(
            search_time_termination_margin,
            ref_search_time_termination_margin,
            f"Search time lower bound: {search_time_termination_margin}; Expected:"
            f" {ref_search_time_termination_margin}",
        )

        if None is ref_wildcard_queries:
            self.assertEqual(wildcard_queries, None, "The wildcard query list should be empty.")
            return

        assert None is not wildcard_queries
        wildcard_query_list_size: int = len(wildcard_queries)
        self.assertEqual(
            wildcard_query_list_size,
            len(wildcard_queries),
            "Wildcard query list size doesn't match.",
        )

        for i in range(wildcard_query_list_size):
            self._check_wildcard_query(
                wildcard_queries[i],
                ref_wildcard_queries[i].wildcard_query,
                ref_wildcard_queries[i].case_sensitive,
            )


class TestCaseWildcardQuery(TestCaseQueryBase):
    """
    Class for testing clp_ffi_py.query.WildcardQuery.
    """

    def test_init(self) -> None:
        """
        Test the initialization of WildcardQuery object.
        """
        wildcard_string: str
        wildcard_query: WildcardQuery

        wildcard_string = "Are you the lord of *Pleiades*?"
        wildcard_query = WildcardQuery(wildcard_string)
        self._check_wildcard_query(wildcard_query, wildcard_string, False)

        wildcard_query = WildcardQuery(wildcard_string, True)
        self._check_wildcard_query(wildcard_query, wildcard_string, True)

        wildcard_query = WildcardQuery(wildcard_string, case_sensitive=True)
        self._check_wildcard_query(wildcard_query, wildcard_string, True)

        wildcard_query = WildcardQuery(case_sensitive=True, wildcard_query=wildcard_string)
        self._check_wildcard_query(wildcard_query, wildcard_string, True)


class TestCaseQuery(TestCaseQueryBase):
    """
    Class for testing clp_ffi_py.ir.Query.
    """

    def test_init_search_time(self) -> None:
        """
        Test the construction of Query object with the different search time
        range.
        """
        query: Query
        search_time_lower_bound: int
        search_time_upper_bound: int
        search_time_termination_margin: int
        exception_captured: bool

        # Note: for the default initialization, the actual search time
        # termination margin should be set to 0, otherwise it will overflow
        # the underlying integer that stores the search termination timestamp.
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


if __name__ == "__main__":
    unittest.main()
