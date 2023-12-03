import io
import random
from pathlib import Path
from typing import IO, List, Optional, Tuple

from smart_open import open  # type: ignore
from test_ir.test_utils import get_current_timestamp, LogGenerator, TestCLPBase

from clp_ffi_py.ir import (
    Decoder,
    DecoderBuffer,
    FourByteEncoder,
    LogEvent,
    Metadata,
    Query,
)
from clp_ffi_py.wildcard_query import WildcardQuery

LOG_DIR: Path = Path("unittest-logs")


class TestCaseDecoderSkip(TestCLPBase):
    """
    Class for testing the skip method(s) of clp_ffi_py.ir.Decoder.
    """

    def _create_simple_encoded_IR_stream(self, num_events: int) -> IO[bytes]:
        """
        Creates a simple IR stream that contains `num_events` of log events. The
        content of each message is set to "Message".

        :param num_events
        :return: A byte stream that contains the encoded CLP IR stream.
        """
        ir_stream: IO[bytes] = io.BytesIO()
        ir_stream.write(FourByteEncoder.encode_preamble(0, "", "America/New_York"))
        for _ in range(num_events):
            ir_stream.write(
                FourByteEncoder.encode_message_and_timestamp_delta(0, "Message".encode())
            )
        ir_stream.write(FourByteEncoder.encode_end_of_ir())
        ir_stream.seek(0)
        return ir_stream

    def test_skip_negative_num_events(self) -> None:
        """
        Tests skipping with a negative number of events.
        """
        ir_stream: IO[bytes] = self._create_simple_encoded_IR_stream(1)
        decoder_buffer: DecoderBuffer = DecoderBuffer(ir_stream)
        _ = Decoder.decode_preamble(decoder_buffer)
        error_captured: bool = False
        try:
            Decoder.skip_next_n_log_events(decoder_buffer, -1)
        except NotImplementedError:
            error_captured = True
        except Exception as e:
            self.assertTrue(False, f"Unexpected exception captured: {e}")
        self.assertTrue(error_captured, "Negative input should raise an `NotImplemented` exception")

    def test_skip_zero_event(self) -> None:
        """
        Tests skipping with a `0` as the input.
        """
        num_events: int = 10
        ir_stream: IO[bytes] = self._create_simple_encoded_IR_stream(10)
        decoder_buffer: DecoderBuffer = DecoderBuffer(ir_stream)
        _ = Decoder.decode_preamble(decoder_buffer)
        for i in range(num_events):
            Decoder.skip_next_n_log_events(decoder_buffer, 0)
            log_event: Optional[LogEvent] = Decoder.decode_next_log_event(decoder_buffer)
            if None is log_event:
                self.assertTrue(False, "EOF shouldn't be reached before all log events are decoded")
            assert None is not log_event  # Silence mypy check
            decoded_idx: int = log_event.get_index()
            self.assertEqual(i, decoded_idx, f"Expected idx: {i}, Decoded idx: {decoded_idx}")
        eof: Optional[LogEvent] = Decoder.decode_next_log_event(decoder_buffer)
        self.assertEqual(None, eof, "EOF should be reached since all the log events are decoded")

    def test_skip_to_eof(self) -> None:
        """
        Tests skipping with a number larger than the total number of events in
        the stream.
        """
        ir_stream: IO[bytes] = self._create_simple_encoded_IR_stream(1)
        decoder_buffer: DecoderBuffer = DecoderBuffer(ir_stream)
        _ = Decoder.decode_preamble(decoder_buffer)
        Decoder.skip_next_n_log_events(decoder_buffer, 1000)
        eof: Optional[LogEvent] = Decoder.decode_next_log_event(decoder_buffer)
        self.assertEqual(None, eof, "EOF should be reached since all the log events are decoded")

    def test_skip_general(self) -> None:
        """
        Tests skipping in general.
        """
        num_arithmetic_sequence_term: int = 10
        arithmetic_sequence_sum: int = 0
        for i in range(num_arithmetic_sequence_term):
            arithmetic_sequence_sum += i + 1
        num_events = arithmetic_sequence_sum * 2

        ir_stream: IO[bytes] = self._create_simple_encoded_IR_stream(num_events)
        decoder_buffer: DecoderBuffer = DecoderBuffer(ir_stream)
        _ = Decoder.decode_preamble(decoder_buffer)
        num_events_skipped: int = 0
        num_events_decoded: int = 0
        for i in range(num_arithmetic_sequence_term):
            term_idx = i + 1
            Decoder.skip_next_n_log_events(decoder_buffer, term_idx)
            num_events_skipped += term_idx
            for _ in range(term_idx):
                log_event: Optional[LogEvent] = Decoder.decode_next_log_event(decoder_buffer)
                if None is log_event:
                    self.assertTrue(
                        False, "EOF shouldn't be reached before all log events are decoded"
                    )
                assert None is not log_event  # Silence mypy check
                decoded_idx: int = log_event.get_index()
                expected_idx: int = num_events_decoded + num_events_skipped
                self.assertEqual(
                    expected_idx,
                    decoded_idx,
                    f"Expected idx: {expected_idx}, Decoded idx: {decoded_idx}",
                )
                num_events_decoded += 1
        eof: Optional[LogEvent] = Decoder.decode_next_log_event(decoder_buffer)
        self.assertEqual(None, eof, "EOF should be reached since all the log events are decoded")
        self.assertTrue(arithmetic_sequence_sum, num_events_decoded)
        self.assertTrue(arithmetic_sequence_sum, num_events_skipped)


class TestCaseDecoderBase(TestCLPBase):
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
            ostream.write(FourByteEncoder.encode_end_of_ir())

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


class TestCaseDecoderDecompressZstd(TestCaseDecoderBase):
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


class TestCaseDecoderDecompressZstdDefaultQuery(TestCaseDecoderBase):
    """
    Tests encoding/decoding methods against zstd compressed IR stream with the
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


class TestCaseDecoderTimeRangeQueryZstd(TestCaseDecoderTimeRangeQueryBase):
    """
    Tests encoding/decoding methods against zstd compressed IR stream with the
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


class TestCaseDecoderWildcardQueryZstd(TestCaseDecoderWildcardQueryBase):
    """
    Tests encoding/decoding methods against zstd compressed IR stream with the
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


class TestCaseDecoderTimeRangeWildcardQueryZstd(TestCaseDecoderTimeRangeWildcardQueryBase):
    """
    Tests encoding/decoding methods against zstd compressed IR stream with the
    query that specifies both search time range and wildcard queries.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = True
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()
