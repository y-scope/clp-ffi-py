import random
from pathlib import Path
from typing import List, Optional, Tuple

from smart_open import open  # type: ignore

from clp_ffi_py import (
    Decoder,
    DecoderBuffer,
    FourByteEncoder,
    LogEvent,
    Metadata,
    Query,
    WildcardQuery,
)
from tests.test_utils import get_current_timestamp, LogGenerator, TestCLPBase

LOG_DIR: Path = Path("unittest-logs")


class TestCaseDecoderBase(TestCLPBase):
    """
    Class for testing clp_ffi_py.ir.Decoder.
    """

    encoded_log_path_prefix: str
    encoded_log_path_postfix: str
    num_test_iterations: int
    enable_compression: bool
    has_query: bool
    test_skip: bool

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

    def _encode_log_stream(
        self, log_path: Path, metadata: Metadata, log_events: List[LogEvent]
    ) -> None:
        """
        Encodes the log stream into the given path.

        :param log_path: Path on the local file system to write the stream.
        :param metadata: Metadata of the log stream.
        :param log_events: A list of log events to encode.
        """
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
                    FourByteEncoder.encode_message_and_timestamp_delta(delta, log_message.encode())
                )
            ostream.write(b"\x00")

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
            self._encode_log_stream(log_path, metadata, log_events)
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
            seed: int = get_current_timestamp()
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


class TestCaseDecoderWithSkipBase(TestCaseDecoderBase):
    """
    Tests encoding/decoding methods against uncompressed IR stream with skip
    method(s).
    """

    def _skip_to_time_testing(self, log_path: Path, n: int) -> None:
        """
        Helper function for testing `skip_to_time`.

        :param log_path: Local file system path that contains the encoded IR
            stream.
        :param n: Testing parameter ranges from 0 to 50 (exclusive).
        """
        self.assertTrue(0 < n and n < 50, "The testing parameter must be within the range.")

        with open(str(log_path), "rb") as fin:
            decoder_buffer: DecoderBuffer = DecoderBuffer(fin)
            num_log_events_skipped: int
            log_event: Optional[LogEvent]
            metadata: Metadata = Decoder.decode_preamble(decoder_buffer)
            base_timestamp: int = metadata.get_ref_timestamp()

            num_log_events_skipped = Decoder.skip_to_time(decoder_buffer, 0)
            self.assertEqual(num_log_events_skipped, 0, "No log event should be skipped.")
            log_event = Decoder.decode_next_log_event(decoder_buffer)
            self.assertTrue(log_event is not None, "Log event should be successfully decoded.")
            assert None is not log_event
            self.assertEqual(
                log_event.get_index(),
                0,
                "The next decoded log event should be the first one in the stream",
            )

            # Iterate next n log messages
            for _ in range(n):
                log_event = Decoder.decode_next_log_event(decoder_buffer)

            num_log_events_skipped = Decoder.skip_to_time(decoder_buffer, base_timestamp + n // 2)
            self.assertEqual(num_log_events_skipped, 0, "No log event should be skipped.")
            log_event = Decoder.decode_next_log_event(decoder_buffer)
            self.assertTrue(log_event is not None, "Log event should be successfully decoded.")
            assert None is not log_event
            self.assertEqual(
                log_event.get_index(),
                n + 1,
                "The next decoded log event should be the first one in the stream",
            )

            num_log_events_skipped = Decoder.skip_to_time(decoder_buffer, base_timestamp + 2 * n)
            self.assertEqual(
                num_log_events_skipped, 4 * n - (n + 2), "No log event should be skipped."
            )
            log_event = Decoder.decode_next_log_event(decoder_buffer)
            self.assertTrue(log_event is not None, "Log event should be successfully decoded.")
            assert None is not log_event
            self.assertEqual(
                log_event.get_index(),
                4 * n,
                "The next decoded log event should be the first one in the stream",
            )

            num_log_events_skipped = Decoder.skip_to_time(decoder_buffer, base_timestamp + 1000000)
            log_event = Decoder.decode_next_log_event(decoder_buffer)
            self.assertTrue(log_event is None, "All the logs should be escaped.")

    def test_skip_to_time(self) -> None:
        """
        Tests `skip_to_time` method in Decoder.
        """
        skip_to_time_path: Path = LOG_DIR / Path(
            f"{self.encoded_log_path_prefix}.{iter}.{self.encoded_log_path_postfix}"
        )
        if skip_to_time_path.exists():
            skip_to_time_path.unlink()
        base_timestamp: int = 100
        ref_log_events: List[LogEvent] = []
        for i in range(200):
            log_message = (
                "This is a log message with one unique id per line:"
                f" app-20230813{i % 100}-{i % 20} from clp-node-{i % 31}"
            )
            ref_log_events.append(LogEvent(log_message, base_timestamp + i // 2, i))
        metadata: Metadata = Metadata(base_timestamp, "", "America/Chicago")
        try:
            self._encode_log_stream(skip_to_time_path, metadata, ref_log_events)
        except Exception:
            self.assertFalse(True, "Failed to encode log events.")

        self._skip_to_time_testing(skip_to_time_path, 1)
        self._skip_to_time_testing(skip_to_time_path, 10)
        self._skip_to_time_testing(skip_to_time_path, 31)
        self._skip_to_time_testing(skip_to_time_path, 49)


class TestCaseDecoderDecompress(TestCaseDecoderWithSkipBase):
    """
    Tests encoding/decoding methods against uncompressed IR stream.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = False
        self.has_query = False
        self.num_test_iterations = 10
        super().setUp()


class TestCaseDecoderDecompressZst(TestCaseDecoderWithSkipBase):
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
