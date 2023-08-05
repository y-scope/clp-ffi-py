import dateutil.tz
import io
import pickle
import random
import time
import unittest

from clp_ffi_py import (
    Decoder,
    DecoderBuffer,
    FourByteEncoder,
    LogEvent,
    Metadata,
    Query,
    WildcardQuery,
)
from datetime import tzinfo
from math import floor
from pathlib import Path
from smart_open import open, register_compressor  # type: ignore
from typing import IO, List, Optional, Set, Tuple, Union
from zstandard import (
    ZstdCompressionWriter,
    ZstdCompressor,
    ZstdDecompressionReader,
    ZstdDecompressor,
)


def _zstd_compressions_handler(
    file_obj: IO[bytes], mode: str
) -> Union[ZstdCompressionWriter, ZstdDecompressionReader]:
    if "wb" == mode:
        cctx = ZstdCompressor()
        return cctx.stream_writer(file_obj)
    elif "rb" == mode:
        dctx = ZstdDecompressor()
        return dctx.stream_reader(file_obj)
    else:
        raise RuntimeError(f"Zstd handler: Unexpected Mode {mode}")


# Register .zst with zstandard library compressor
register_compressor(".zst", _zstd_compressions_handler)


def _get_current_timestamp() -> int:
    """
    :return: the current Unix epoch time in milliseconds.
    """
    timestamp_ms: int = floor(time.time() * 1000)
    return timestamp_ms


LOG_DIR: Path = Path("unittest-logs")


class TestBase(unittest.TestCase):
    """
    Base class for all the testers.

    Helper functions should be defined here.
    """

    def _check_metadata(
        self,
        metadata: Metadata,
        expected_ref_timestamp: int,
        expected_timestamp_format: str,
        expected_timezone_id: str,
        extra_test_info: str = "",
    ) -> None:
        """
        Given a Metadata object, check if the content matches the reference.

        :param metadata: Metadata object to be checked.
        :param expected_ref_timestamp: Expected reference timestamp.
        :param expected_timestamp_format: Expected timestamp format.
        :param expected_timezone_id: Expected timezone ID.
        :param extra_test_info: Extra test information appended to the assert
            message.
        """
        ref_timestamp: int = metadata.get_ref_timestamp()
        timestamp_format: str = metadata.get_timestamp_format()
        timezone_id: str = metadata.get_timezone_id()
        timezone: tzinfo = metadata.get_timezone()

        self.assertEqual(
            ref_timestamp,
            expected_ref_timestamp,
            f'Reference Timestamp: "{ref_timestamp}", Expected: "{expected_ref_timestamp}"\n'
            + extra_test_info,
        )
        self.assertEqual(
            timestamp_format,
            expected_timestamp_format,
            f'Timestamp Format: "{timestamp_format}", Expected: "{expected_timestamp_format}"\n'
            + extra_test_info,
        )
        self.assertEqual(
            timezone_id,
            expected_timezone_id,
            f'Timezone ID: "{timezone_id}", Expected: "{expected_timezone_id}"\n',
        )

        expected_tzinfo: Optional[tzinfo] = dateutil.tz.gettz(expected_timezone_id)
        assert expected_tzinfo is not None
        is_the_same_tz: bool = expected_tzinfo is timezone
        self.assertEqual(
            is_the_same_tz,
            True,
            f"Timezone does not match timezone id. Timezone ID: {timezone_id}, Timezone:"
            f' {str(timezone)}"\n'
            + extra_test_info,
        )

    def _check_log_event(
        self,
        log_event: LogEvent,
        expected_log_message: str,
        expected_timestamp: int,
        expected_idx: int,
        extra_test_info: str = "",
    ) -> None:
        """
        Given a LogEvent object, check if the content matches the reference.

        :param log_event: LogEvent object to be checked.
        :param expected_log_message: Expected log message.
        :param expected_timestamp: Expected timestamp.
        :param expected_idx: Expected log event index.
        :param extra_test_info: Extra test information appended to the assert
            message.
        """
        log_message: str = log_event.get_log_message()
        timestamp: int = log_event.get_timestamp()
        idx: int = log_event.get_index()
        self.assertEqual(
            log_message,
            expected_log_message,
            f'Log message: "{log_message}", Expected: "{expected_log_message}"\n' + extra_test_info,
        )
        self.assertEqual(
            timestamp,
            expected_timestamp,
            f'Timestamp: "{timestamp}", Expected: "{expected_log_message}"\n' + extra_test_info,
        )
        self.assertEqual(
            idx, expected_idx, f"Message idx: {idx}, Expected: {expected_idx}\n" + extra_test_info
        )

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


class TestCaseMetadata(TestBase):
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
        self._check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

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
        self._check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

        metadata = Metadata(
            timestamp_format=timestamp_format, ref_timestamp=ref_timestamp, timezone_id=timezone_id
        )
        self._check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

    def test_timezone_new_reference(self) -> None:
        """
        Test the timezone is a new reference returned.
        """
        ref_timestamp: int = 2005689603190
        timestamp_format: str = "yy/MM/dd HH:mm:ss"
        timezone_id: str = "America/Chicago"
        metadata: Metadata = Metadata(ref_timestamp, timestamp_format, timezone_id)
        self._check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

        wrong_tz: Optional[tzinfo] = metadata.get_timezone()
        self.assertEqual(wrong_tz is metadata.get_timezone(), True)

        wrong_tz = dateutil.tz.gettz("America/New_York")
        self.assertEqual(wrong_tz is not metadata.get_timezone(), True)

        self._check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)


class TestCaseLogEvent(TestBase):
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
        self._check_log_event(log_event, log_message, timestamp, idx)

        log_event = LogEvent(log_message, timestamp)
        self._check_log_event(log_event, log_message, timestamp, 0)

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
        self._check_log_event(log_event, log_message, timestamp, idx)

        # Initialize with keyword (out-of-order)
        log_event = LogEvent(
            index=idx, timestamp=timestamp, log_message=log_message, metadata=metadata
        )
        self._check_log_event(log_event, log_message, timestamp, idx)

        # Initialize with keyword and default argument (out-of-order)
        log_event = LogEvent(timestamp=timestamp, log_message=log_message)
        self._check_log_event(log_event, log_message, timestamp, 0)

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
        self._check_log_event(log_event, log_message, timestamp, idx)
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
        self._check_log_event(log_event, log_message, timestamp, idx)
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
        self._check_log_event(log_event, log_message, timestamp, idx)
        reconstructed_log_event: LogEvent = pickle.loads(pickle.dumps(log_event))
        self._check_log_event(reconstructed_log_event, log_message, timestamp, idx)

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


class TestCaseFourByteEncoder(TestBase):
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


class TestCaseWildcardQuery(TestBase):
    """
    Class for testing clp_ffi_py.wildcard_query.WildcardQuery.
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


class TestCaseQuery(TestBase):
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


class TestCaseDecoderBuffer(TestBase):
    """
    Class for testing clp_ffi_py.ir.DecoderBuffer.
    """

    input_src_dir = "test_data"

    def test_buffer_protocol(self) -> None:
        """
        Tests whether the buffer protocol is disabled by default.
        """
        byte_array: bytearray = bytearray(b"Hello, world!")
        byte_stream: io.BytesIO = io.BytesIO(byte_array)
        decoder_buffer: DecoderBuffer = DecoderBuffer(byte_stream)
        exception_captured: bool = False
        try:
            byte_stream.readinto(decoder_buffer)  # type: ignore
        except TypeError:
            exception_captured = True
        self.assertTrue(
            exception_captured, "The buffer protocol should not be enabled in Python layer."
        )

    def test_streaming_small_buffer(self) -> None:
        """
        Tests DecoderBuffer's functionality using the small buffer capacity.
        """
        buffer_capacity: int = 1024
        self.__launch_test(buffer_capacity)

    def test_streaming_default_buffer(self) -> None:
        """
        Tests DecoderBuffer's functionality using the default buffer capacity.
        """
        self.__launch_test(None)

    def test_streaming_large_buffer(self) -> None:
        """
        Tests DecoderBuffer's functionality using the large buffer capacity.
        """
        buffer_capacity: int = 16384
        self.__launch_test(buffer_capacity)

    def __launch_test(self, buffer_capacity: Optional[int]) -> None:
        """
        Tests the DecoderBuffer by streaming the files inside `test_src_dir`.

        :param self
        :param buffer_capacity: The buffer capacity used to initialize the
            decoder buffer.
        """
        current_dir: Path = Path(__file__).resolve().parent
        test_src_dir: Path = current_dir / TestCaseDecoderBuffer.input_src_dir
        for file_path in test_src_dir.rglob("*"):
            if not file_path.is_file():
                continue
            streaming_result: bytearray
            decoder_buffer: DecoderBuffer
            random_seed: int
            # Run against 10 different seeds:
            for _ in range(10):
                random_seed = random.randint(1, 3190)
                with open(str(file_path), "rb") as istream:
                    try:
                        if None is buffer_capacity:
                            decoder_buffer = DecoderBuffer(istream)
                        else:
                            decoder_buffer = DecoderBuffer(
                                initial_buffer_capacity=buffer_capacity, input_stream=istream
                            )
                        streaming_result = decoder_buffer._test_streaming(random_seed)
                    except Exception as e:
                        self.assertFalse(
                            True, f"Error on file {file_path} using seed {random_seed}: {e}"
                        )
                self.__assert_streaming_result(file_path, streaming_result, random_seed)

    def __assert_streaming_result(
        self, file_path: Path, streaming_result: bytearray, random_seed: int
    ) -> None:
        """
        Validates the streaming result read by the decoder buffer.

        :param file_path: Input stream file Path.
        :param streaming_result: Result of DecoderBuffer `_test_streaming` method.
        """
        with open(str(file_path), "rb") as istream:
            ref_result: bytearray = bytearray(istream.read())
            self.assertEqual(
                ref_result,
                streaming_result,
                f"Streaming result is different from the src: {file_path}. Random seed:"
                f" {random_seed}.",
            )


class LogGenerator:
    """
    Generates random logs from a list of log types and dictionary words.
    """

    log_type_list: List[str] = [
        (
            "org.apache.hadoop.yarn.event.AsyncDispatcher: Registering class"
            " org.apache.hadoop.yarn.server.nodemanager.NodeManagerEventType for class"
            " org.apache.hadoop.yarn.server.nodemanager.NodeManager"
        ),
        "\d: Scheduled snapshot period at \i second(s).",
        (
            "org.apache.hadoop.ipc.Client: Retrying connect to server: \d:\d Already tried \i"
            " time(s); retry policy is RetryUpToMaximumCountWithFixedSleep(maxRetries=\i,"
            " sleepTime=\i MILLISECONDS)"
        ),
        (
            "org.apache.hadoop.yarn.server.nodemanager.containermanager.monitor.ContainersMonitor:"
            " Memory usage of ProcessTree \i for container-id \d: \f MB of \i GB physical memory"
            " used; \f MB of \f GB virtual memory used"
        ),
        (
            "org.apache.hadoop.hdfs.server.datanode.DataNode: Namenode Block pool \d (Datanode Uuid"
            " \d) service to \d:\i trying to claim ACTIVE state with txid=\i"
        ),
        " org.apache.hadoop.mapred.MapTask: (RESET) equator \i kv \i(\i) kvi \i(\i)",
        " org.apache.hadoop.mapred.TaskAttemptListenerImpl: Progress of TaskAttempt \d is : \f",
        (
            " \d: Final Stats: PendingReds:\i ScheduledMaps:\i ScheduledReds:\i AssignedMaps:\i"
            " AssignedReds:\i CompletedMaps:\i CompletedReds:\i ContAlloc:\i ContRel:\i"
            " HostLocal:\i RackLocal:\i"
        ),
        (
            "org.apache.hadoop.yarn.server.resourcemanager.scheduler.capacity.LeafQueue: Reserved"
            " container  application=\d resource=<memory:\i, vCores:\i> queue=\d: capacity=\f,"
            " absoluteCapacity=\f, usedResources=<memory:\i, vCores:\i>, usedCapacity=\f,"
            " absoluteUsedCapacity=\f, numApps=\i, numContainers=\i usedCapacity=\f"
            " absoluteUsedCapacity=\f used=<memory:\i, vCores:\i> cluster=<memory:\i, vCores:\i>"
        ),
        "org.apache.hadoop.hdfs.server.namenode.TransferFsImage: Transfer took \d at \f KB/s",
    ]

    dict_words: List[str] = [
        "attempt_1427088391284_0029_r_000026_0",
        "blk_1073742594_1770",
        "container_1427088391284_0034_01_000074",
        "DFSClient_attempt_1427088391284_0015_r_000003_0_-1867302407_1",
        "ip-172-31-17-96",
        "jvm_1427088391284_0034_m_000018",
        "jvm_1427088391284_0032_m_000200",
        "task_1427088391284_0034_m_000005",
        (
            "/tmp/hadoop-ubuntu/nm-local-dir/usercache/ubuntu/appcache/application_1427088391284_72"
            "/container_1427088391284_0072_01_000031/default_container_executor.sh"
        ),
        (
            "/tmp/hadoop-ubuntu/nm-local-dir/usercache/ubuntu/appcache/application_1427088391284_27"
            "/container_1427088391284_0027_01_000008.tokens"
        ),
        (
            "/tmp/hadoop-ubuntu/nm-local-dir/usercache/ubuntu/appcache/application_1427088391284_87"
            "/container_1427088391284_0087_01_000043/default_container_executor.sh"
        ),
        "/tmp/hadoop-yarn/staging/ubuntu/.staging/job_1427088391284_0005",
        (
            "/tmp/hadoop-ubuntu/dfs/data/current/BP-1121897155-172.31.17.135-1427088167814/current"
            "/finalized/subdir0/subdir18/blk_1073746642"
        ),
        "328418859ns",
        "3154ms",
    ]

    @staticmethod
    def generate_random_logs(num_log_events: int) -> Tuple[Metadata, List[LogEvent]]:
        ref_timestamp: int = _get_current_timestamp()
        timestamp_format: str = "yy/MM/dd HH:mm:ss"
        timezone_id: str = "America/Chicago"
        metadata: Metadata = Metadata(ref_timestamp, timestamp_format, timezone_id)
        log_events: List[LogEvent] = []
        timestamp: int = ref_timestamp
        for idx in range(num_log_events):
            log_message: str = random.choice(LogGenerator.log_type_list)
            log_message = log_message.replace("\d", random.choice(LogGenerator.dict_words))
            log_message = log_message.replace("\i", str(random.randint(-999999999, 999999999)))
            log_message = log_message.replace("\f", str(random.uniform(-999999, 9999999)))
            timestamp += random.randint(0, 10)
            log_event: LogEvent = LogEvent(log_message + "\n", timestamp, idx)
            log_events.append(log_event)
        return metadata, log_events

    @staticmethod
    def generate_random_log_type_wildcard_queries(num_wildcard_queries: int) -> List[WildcardQuery]:
        num_log_types: int = len(LogGenerator.log_type_list)
        num_wildcard_queries = min(num_log_types, num_wildcard_queries)
        selected_log_type_idx: Set[int] = set()
        max_log_type_idx: int = num_log_types - 1
        wildcard_queries: List[WildcardQuery] = []
        for _ in range(num_wildcard_queries):
            idx: int
            while True:
                idx = random.randint(0, max_log_type_idx)
                if idx in selected_log_type_idx:
                    continue
                break
            selected_log_type_idx.add(idx)
            wildcard_query_str: str = LogGenerator.log_type_list[idx]

            # Replace the placeholders by the wildcard `*`
            wildcard_query_str = wildcard_query_str.replace("\d", "*")
            wildcard_query_str = wildcard_query_str.replace("\i", "*")
            wildcard_query_str = wildcard_query_str.replace("\f", "*")

            # Replace a random character that is not `*` by `?`
            str_idx_max: int = len(wildcard_query_str) - 1
            while True:
                str_idx: int = random.randint(0, str_idx_max)
                if "*" == wildcard_query_str[str_idx] or "/" == wildcard_query_str[str_idx]:
                    continue
                wildcard_query_str = (
                    wildcard_query_str[:str_idx] + "?" + wildcard_query_str[str_idx + 1 :]
                )
                break

            wildcard_queries.append(
                WildcardQuery(wildcard_query=wildcard_query_str, case_sensitive=True)
            )

        return wildcard_queries


class TestCaseDecoderBase(TestBase):
    """
    Class for testing clp_ffi_py.ir.Decoder.
    """

    encoded_log_path_prefix: str
    encoded_log_path_postfix: str
    num_test_iterations: int
    enable_compression: bool
    has_query: bool

    # override
    @classmethod
    def setUpClass(cls) -> None:
        if not LOG_DIR.exists():
            LOG_DIR.mkdir(parents=True, exist_ok=True)
        assert LOG_DIR.is_dir()

    # override
    def setUp(self) -> None:
        self.encoded_log_path_prefix: str = f"{self.id()}"
        self.encoded_log_path_postfix: str = "clp.zst" if self.enable_compression else "clp"
        for i in range(self.num_test_iterations):
            log_path = self._get_log_path(i)
            if log_path.exists():
                log_path.unlink()

    def _get_log_path(self, iter: int) -> Path:
        log_path: Path = LOG_DIR / Path(
            f"{self.encoded_log_path_prefix}.{iter}.{self.encoded_log_path_postfix}"
        )
        return log_path

    def _encode_random_log_stream(
        self, log_path: Path, num_log_events_to_generate: int, seed: int
    ) -> Tuple[Metadata, List[LogEvent]]:
        """
        Writes a randomly generated log stream into the local path `log_path`.

        :param log_path: Path on the local file system to write the stream.
        :param num_log_events_to_generate: Number of log events to generate.
        :param seed: Random seed used to generate the log stream.
        :return: A tuple containing the generated log events and the metadata.
        """
        metadata: Metadata
        log_events: List[LogEvent]
        metadata, log_events = LogGenerator.generate_random_logs(num_log_events_to_generate)
        try:
            with open(str(log_path), "wb") as ostream:
                ref_timestamp: int = metadata.get_ref_timestamp()
                ostream.write(
                    FourByteEncoder.encode_preamble(
                        ref_timestamp, metadata.get_timestamp_format(), metadata.get_timezone_id()
                    )
                )
                for log_event in log_events:
                    curr_ts: int = log_event.get_timestamp()
                    delta: int = curr_ts - ref_timestamp
                    ref_timestamp = curr_ts
                    log_message: str = log_event.get_log_message()
                    ostream.write(
                        FourByteEncoder.encode_message_and_timestamp_delta(
                            delta, log_message.encode()
                        )
                    )
                ostream.write(b"\x00")
        except Exception as e:
            self.assertTrue(
                False, f"Failed to encode random log stream generated using seed {seed}: {e}"
            )
        return metadata, log_events

    def _generate_random_query(
        self, ref_log_events: List[LogEvent]
    ) -> Tuple[Query, List[LogEvent]]:
        """
        Generates a random query and return all the log events in the given
        `log_events` that matches this random query.

        The derived class might overwrite this method to generate a random query
        using customized algorithm. By default, this function returns an empty
        query and `ref_log_events`.
        :param log_events: reference log events.
        :return: A tuple that contains the randomly generated query, and a list
        of log events filtered from `ref_log_events` by the query.
        """
        return Query(), ref_log_events

    def _decode_log_stream(
        self, log_path: Path, query: Optional[Query]
    ) -> Tuple[Metadata, List[LogEvent]]:
        """
        Decodes the log stream specified by `log_path`, using decoding methods
        provided in clp_ffi_py.ir.Decoder.

        :param log_path: The path to the log stream.
        :param query: Optional search query.
        :return: A tuple that contains the decoded metadata and log events
            returned from decoding methods.
        """
        with open(str(log_path), "rb") as istream:
            decoder_buffer: DecoderBuffer = DecoderBuffer(istream)
            metadata: Metadata = Decoder.decode_preamble(decoder_buffer)
            log_events: List[LogEvent] = []
            while True:
                log_event: Optional[LogEvent] = Decoder.decode_next_log_event(decoder_buffer, query)
                if None is log_event:
                    break
                log_events.append(log_event)
        return metadata, log_events

    def _validate_decoded_logs(
        self,
        ref_metadata: Metadata,
        ref_log_events: List[LogEvent],
        decoded_metadata: Metadata,
        decoded_log_events: List[LogEvent],
        log_path: Path,
        seed: int,
    ) -> None:
        """
        Validates decoded logs from the IR stream specified by `log_path`.

        :param ref_metadata: Reference metadata.
        :param ref_log_events: A list of reference log events sequence (order
            sensitive).
        :param decoded_metadata: Metadata decoded from the IR stream.
        :param decoded_log_events: A list of log events decoded from the IR
            stream in sequence.
        :param log_path: Local path of the IR stream.
        :param seed: Random seed used to generate the log events sequence.
        """
        test_info: str = f"Seed: {seed}, Log Path: {log_path}"
        self._check_metadata(
            decoded_metadata,
            ref_metadata.get_ref_timestamp(),
            ref_metadata.get_timestamp_format(),
            ref_metadata.get_timezone_id(),
            test_info,
        )

        ref_num_log_events: int = len(ref_log_events)
        decoded_num_log_events: int = len(decoded_log_events)
        self.assertEqual(
            ref_num_log_events,
            decoded_num_log_events,
            "Number of log events decoded does not match.\n" + test_info,
        )
        for ref_log_event, decoded_log_event in zip(ref_log_events, decoded_log_events):
            self._check_log_event(
                decoded_log_event,
                ref_log_event.get_log_message(),
                ref_log_event.get_timestamp(),
                ref_log_event.get_index(),
                test_info,
            )

    def test_decoder_with_random_logs(self) -> None:
        """
        Tests encoding/decoding methods.

        Check the TestCase class doc string for more details.
        """
        for i in range(self.num_test_iterations):
            seed: int = _get_current_timestamp()
            random.seed(seed)
            num_log_events: int = 100 * (i + 1)
            log_path: Path = self._get_log_path(i)

            ref_metadata: Metadata
            ref_log_events: List[LogEvent]
            ref_metadata, ref_log_events = self._encode_random_log_stream(
                log_path, num_log_events, seed
            )

            query: Optional[Query] = None
            if self.has_query:
                query, ref_log_events = self._generate_random_query(ref_log_events)

            metadata: Metadata
            log_events: List[LogEvent]
            try:
                metadata, log_events = self._decode_log_stream(log_path, query)
            except Exception as e:
                self.assertTrue(
                    False, f"Failed to decode random log stream generated using seed {seed}: {e}"
                )

            self._validate_decoded_logs(
                ref_metadata, ref_log_events, metadata, log_events, log_path, seed
            )


class TestCaseDecoderDecompress(TestCaseDecoderBase):
    """
    Tests encoding/decoding methods against uncompressed IR stream.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = False
        self.has_query = False
        self.num_test_iterations = 10
        super().setUp()


class TestCaseDecoderDecompressZst(TestCaseDecoderBase):
    """
    Tests encoding/decoding methods against zstd compressed IR stream.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = True
        self.has_query = False
        self.num_test_iterations = 10
        super().setUp()


class TestCaseDecoderDecompressDefaultQuery(TestCaseDecoderBase):
    """
    Tests encoding/decoding methods against uncompressed IR stream with the
    default empty query.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = False
        self.has_query = True
        self.num_test_iterations = 1
        super().setUp()


class TestCaseDecoderDecompressZstDefaultQuery(TestCaseDecoderBase):
    """
    Tests encoding/decoding methods against zst compressed IR stream with the
    default empty query.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = True
        self.has_query = True
        self.num_test_iterations = 1
        super().setUp()


class TestCaseDecoderTimeRangeQueryBase(TestCaseDecoderBase):
    # override
    def _generate_random_query(
        self, ref_log_events: List[LogEvent]
    ) -> Tuple[Query, List[LogEvent]]:
        self.assertGreater(len(ref_log_events), 0, "The reference log event list is empty.")
        ts_min: int = ref_log_events[0].get_timestamp()
        ts_max: int = ref_log_events[-1].get_timestamp()
        search_time_lower_bound: int = random.randint(ts_min, ts_max)
        search_time_upper_bound: int = random.randint(search_time_lower_bound, ts_max)
        query: Query = Query(
            search_time_lower_bound=search_time_lower_bound,
            search_time_upper_bound=search_time_upper_bound,
            search_time_termination_margin=0,
        )
        matched_log_events: List[LogEvent] = []
        for log_event in ref_log_events:
            if not log_event.match_query(query):
                continue
            matched_log_events.append(log_event)
        return query, matched_log_events


class TestCaseDecoderTimeRangeQuery(TestCaseDecoderTimeRangeQueryBase):
    """
    Tests encoding/decoding methods against uncompressed IR stream with the
    query that specifies a search timestamp.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = False
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestCaseDecoderTimeRangeQueryZst(TestCaseDecoderTimeRangeQueryBase):
    """
    Tests encoding/decoding methods against zst compressed IR stream with the
    query that specifies a search timestamp.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = True
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestCaseDecoderWildcardQueryBase(TestCaseDecoderBase):
    # override
    def _generate_random_query(
        self, ref_log_events: List[LogEvent]
    ) -> Tuple[Query, List[LogEvent]]:
        wildcard_queries: List[WildcardQuery] = (
            LogGenerator.generate_random_log_type_wildcard_queries(3)
        )
        query: Query = Query(wildcard_queries=wildcard_queries)
        matched_log_events: List[LogEvent] = []
        for log_event in ref_log_events:
            if not log_event.match_query(query):
                continue
            matched_log_events.append(log_event)
        return query, matched_log_events


class TestCaseDecoderWildcardQuery(TestCaseDecoderWildcardQueryBase):
    """
    Tests encoding/decoding methods against uncompressed IR stream with the
    query that specifies wildcard queries.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = False
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestCaseDecoderWildcardQueryZst(TestCaseDecoderWildcardQueryBase):
    """
    Tests encoding/decoding methods against zst compressed IR stream with the
    query that specifies a wildcard queries.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = True
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestCaseDecoderTimeRangeWildcardQueryBase(TestCaseDecoderBase):
    # override
    def _generate_random_query(
        self, ref_log_events: List[LogEvent]
    ) -> Tuple[Query, List[LogEvent]]:
        self.assertGreater(len(ref_log_events), 0, "The reference log event list is empty.")
        ts_min: int = ref_log_events[0].get_timestamp()
        ts_max: int = ref_log_events[-1].get_timestamp()
        search_time_lower_bound: int = random.randint(ts_min, ts_max)
        search_time_upper_bound: int = random.randint(search_time_lower_bound, ts_max)
        wildcard_queries: List[WildcardQuery] = (
            LogGenerator.generate_random_log_type_wildcard_queries(3)
        )
        query: Query = Query(
            search_time_lower_bound=search_time_lower_bound,
            search_time_upper_bound=search_time_upper_bound,
            wildcard_queries=wildcard_queries,
            search_time_termination_margin=0,
        )
        matched_log_events: List[LogEvent] = []
        for log_event in ref_log_events:
            if not log_event.match_query(query):
                continue
            matched_log_events.append(log_event)
        return query, matched_log_events


class TestCaseDecoderTimeRangeWildcardQuery(TestCaseDecoderTimeRangeWildcardQueryBase):
    """
    Tests encoding/decoding methods against uncompressed IR stream with the
    query that specifies both search time range and wildcard queries.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = False
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestCaseDecoderTimeRangeWildcardQueryZst(TestCaseDecoderTimeRangeWildcardQueryBase):
    """
    Tests encoding/decoding methods against zst compressed IR stream with the
    query that specifies both search time range and wildcard queries.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = True
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


if __name__ == "__main__":
    unittest.main()
