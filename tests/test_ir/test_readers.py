from pathlib import Path
from typing import List, Optional, Tuple

from test_ir.test_decoder import (
    TestCaseDecoderBase,
    TestCaseDecoderTimeRangeQueryBase,
    TestCaseDecoderTimeRangeWildcardQueryBase,
    TestCaseDecoderWildcardQueryBase,
)

from clp_ffi_py import LogEvent, Metadata, Query
from clp_ffi_py.readers import ClpIrStreamReader


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


class TestCaseReaderBase(TestCaseDecoderBase):
    # override
    def _decode_log_stream(
        self, log_path: Path, query: Optional[Query]
    ) -> Tuple[Metadata, List[LogEvent]]:
        return read_log_stream(log_path, query, self.enable_compression)


class TestCaseReaderTimeRangeQueryBase(TestCaseDecoderTimeRangeQueryBase):
    # override
    def _decode_log_stream(
        self, log_path: Path, query: Optional[Query]
    ) -> Tuple[Metadata, List[LogEvent]]:
        return read_log_stream(log_path, query, self.enable_compression)


class TestCaseReaderWildcardQueryBase(TestCaseDecoderWildcardQueryBase):
    # override
    def _decode_log_stream(
        self, log_path: Path, query: Optional[Query]
    ) -> Tuple[Metadata, List[LogEvent]]:
        return read_log_stream(log_path, query, self.enable_compression)


class TestCaseReaderTimeRangeWildcardQueryBase(TestCaseDecoderTimeRangeWildcardQueryBase):
    # override
    def _decode_log_stream(
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
    Tests stream reader against uncompressed IR stream with the query that
    specifies a search timestamp.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = False
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestCaseReaderTimeRangeQueryZstd(TestCaseReaderTimeRangeQueryBase):
    """
    Tests stream reader against zstd compressed IR stream with the query that
    specifies a search timestamp.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = True
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestCaseReaderWildcardQuery(TestCaseReaderWildcardQueryBase):
    """
    Tests stream reader against uncompressed IR stream with the query that
    specifies wildcard queries.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = False
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestCaseReaderWildcardQueryZstd(TestCaseReaderWildcardQueryBase):
    """
    Tests stream reader against zstd compressed IR stream with the query that
    specifies a wildcard queries.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = True
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestCaseReaderTimeRangeWildcardQuery(TestCaseReaderTimeRangeWildcardQueryBase):
    """
    Tests stream reader against uncompressed IR stream with the query that
    specifies both search time range and wildcard queries.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = False
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()


class TestCaseReaderTimeRangeWildcardQueryZstd(TestCaseReaderTimeRangeWildcardQueryBase):
    """
    Tests stream reader against zstd compressed IR stream with the query that
    specifies both search time range and wildcard queries.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = True
        self.has_query = True
        self.num_test_iterations = 10
        super().setUp()
