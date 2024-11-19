from pathlib import Path
from typing import List, Optional, Tuple

from test_ir.test_deserializer import (
    TestCaseFourByteDeserializerBase,
    TestCaseFourByteDeserializerTimeRangeQueryBase,
    TestCaseFourByteDeserializerTimeRangeWildcardQueryBase,
    TestCaseFourByteDeserializerWildcardQueryBase,
)
from test_ir.test_utils import TestCLPBase

from clp_ffi_py.ir import (
    ClpIrFileReader,
    ClpIrStreamReader,
    IncompleteStreamError,
    LogEvent,
    Metadata,
    Query,
)


def read_log_stream(
    log_path: Path, query: Optional[Query], enable_compression: bool
) -> Tuple[Metadata, List[LogEvent]]:
    metadata: Metadata
    log_events: List[LogEvent] = []
    with open(str(log_path), "rb") as fin:
        reader = ClpIrStreamReader(fin, enable_compression=enable_compression)
        if None is query:
            for log_event in reader:
                log_events.append(log_event)
        else:
            for log_event in reader.search(query):
                log_events.append(log_event)
        metadata = reader.get_metadata()
        reader.close()
    return metadata, log_events


class TestCaseReaderBase(TestCaseFourByteDeserializerBase):
    # override
    def _deserialize_log_stream(
        self, log_path: Path, query: Optional[Query]
    ) -> Tuple[Metadata, List[LogEvent]]:
        return read_log_stream(log_path, query, self.enable_compression)


class TestCaseReaderTimeRangeQueryBase(TestCaseFourByteDeserializerTimeRangeQueryBase):
    # override
    def _deserialize_log_stream(
        self, log_path: Path, query: Optional[Query]
    ) -> Tuple[Metadata, List[LogEvent]]:
        return read_log_stream(log_path, query, self.enable_compression)


class TestCaseReaderWildcardQueryBase(TestCaseFourByteDeserializerWildcardQueryBase):
    # override
    def _deserialize_log_stream(
        self, log_path: Path, query: Optional[Query]
    ) -> Tuple[Metadata, List[LogEvent]]:
        return read_log_stream(log_path, query, self.enable_compression)


class TestCaseReaderTimeRangeWildcardQueryBase(
    TestCaseFourByteDeserializerTimeRangeWildcardQueryBase
):
    # override
    def _deserialize_log_stream(
        self, log_path: Path, query: Optional[Query]
    ) -> Tuple[Metadata, List[LogEvent]]:
        return read_log_stream(log_path, query, self.enable_compression)


class TestCaseReaderDecompress(TestCaseReaderBase):
    """
    Tests stream reader methods against uncompressed IR stream.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = False
        self.has_query = False
        self.num_test_iterations = 10
        super().setUp()


class TestCaseReaderDecompressZstd(TestCaseReaderBase):
    """
    Tests stream reader against zstd compressed IR stream.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = True
        self.has_query = False
        self.num_test_iterations = 10
        super().setUp()


class TestCaseReaderTimeRangeQuery(TestCaseReaderTimeRangeQueryBase):
    """
    Tests stream reader against uncompressed IR stream with the query that specifies a search
    timestamp.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = False
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestCaseReaderTimeRangeQueryZstd(TestCaseReaderTimeRangeQueryBase):
    """
    Tests stream reader against zstd compressed IR stream with the query that specifies a search
    timestamp.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = True
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestCaseReaderWildcardQuery(TestCaseReaderWildcardQueryBase):
    """
    Tests stream reader against uncompressed IR stream with the query that specifies wildcard
    queries.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = False
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestCaseReaderWildcardQueryZstd(TestCaseReaderWildcardQueryBase):
    """
    Tests stream reader against zstd compressed IR stream with the query that specifies a wildcard
    queries.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = True
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestCaseReaderTimeRangeWildcardQuery(TestCaseReaderTimeRangeWildcardQueryBase):
    """
    Tests stream reader against uncompressed IR stream with the query that specifies both search
    time range and wildcard queries.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = False
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestCaseReaderTimeRangeWildcardQueryZstd(TestCaseReaderTimeRangeWildcardQueryBase):
    """
    Tests stream reader against zstd compressed IR stream with the query that specifies both search
    time range and wildcard queries.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = True
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestIncompleteIRStream(TestCLPBase):
    """
    Tests on reading an incomplete stream.
    """

    test_src: Path = (
        Path(__file__).resolve().parent / "test_data/unstructured_ir/incomplete_ir.log.zst"
    )

    def test_incomplete_ir_stream_error(self) -> None:
        """
        Tests the reader against an incomplete IR file with `allow_incomplete_stream` disabled.
        """
        incomplete_stream_error_captured: bool = False
        other_exception_captured: bool = False
        log_counter: int = 0
        with ClpIrFileReader(TestIncompleteIRStream.test_src) as clp_reader:
            try:
                for _ in clp_reader.search(Query()):
                    log_counter += 1
            except IncompleteStreamError:
                incomplete_stream_error_captured = True
            except Exception:
                other_exception_captured = True
        self.assertTrue(
            incomplete_stream_error_captured, "Incomplete Stream Error is not properly set."
        )
        self.assertFalse(other_exception_captured, "No other exception should be set.")
        self.assertTrue(0 != log_counter, "No logs are deserialized.")

    def test_allow_incomplete_ir_stream_error(self) -> None:
        """
        Tests the reader against an incomplete IR file with `allow_incomplete_stream` enabled.
        """
        incomplete_stream_error_captured: bool = False
        other_exception_captured: bool = False
        log_event: Optional[LogEvent] = None
        with ClpIrFileReader(
            TestIncompleteIRStream.test_src, allow_incomplete_stream=True
        ) as clp_reader:
            try:
                while None is not log_event:
                    log_event = clp_reader.read_next_log_event()
            except IncompleteStreamError:
                incomplete_stream_error_captured = True
            except Exception:
                other_exception_captured = True
        self.assertFalse(
            incomplete_stream_error_captured, "Incomplete Stream Error shouldn't be set."
        )
        self.assertFalse(other_exception_captured, "No other exception should be set.")
        self.assertTrue(None is log_event, "None is not reached.")
