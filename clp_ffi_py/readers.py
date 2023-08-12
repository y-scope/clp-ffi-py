from abc import ABCMeta, abstractmethod
from pathlib import Path
from sys import stderr
from types import TracebackType
from typing import IO, Iterator, Optional, Type, Union

from zstandard import ZstdDecompressionReader, ZstdDecompressor

from clp_ffi_py import Decoder, DecoderBuffer, LogEvent, Metadata, Query


class CLPIRBaseReader(metaclass=ABCMeta):
    def __init__(self, istream: IO[bytes], decoder_buffer_size: int, enable_compression: bool):
        """
        Constructor.

        :param istream: Input stream that contains encoded CLP IR.
        :param decoder_buffer_size: Initial size of the decoder buffer.
        :param enable_compression: A flag indicating whether the istream is
        compressed using `zstd`.
        """
        self.__istream: Union[IO[bytes], ZstdDecompressionReader]
        if enable_compression:
            dctx: ZstdDecompressor = ZstdDecompressor()
            self.__istream = dctx.stream_reader(istream, read_across_frames=True)
        else:
            self.__istream = istream
        self._decoder_buffer: DecoderBuffer = DecoderBuffer(self.__istream, decoder_buffer_size)
        self._metadata: Optional[Metadata] = None

    def get_metadata(self) -> Metadata:
        if None is self._metadata:
            raise RuntimeError("The metadata has not been successfully decoded yet.")
        return self._metadata

    def has_metadata(self) -> bool:
        return None is not self._metadata

    def read_preamble(self) -> None:
        if self.has_metadata():
            raise RuntimeError("The preamble has already been decoded.")
        self._metadata = Decoder.decode_preamble(self._decoder_buffer)

    def close(self) -> None:
        self.__istream.close()

    def __iter__(self) -> Iterator[LogEvent]:
        if False is self.has_metadata():
            self.read_preamble()
        return self

    def __enter__(self) -> Iterator[LogEvent]:
        if False is self.has_metadata():
            self.read_preamble()
        return self

    @abstractmethod
    def __next__(self) -> LogEvent:
        raise NotImplementedError("__next__ method is not implemented.")

    def __exit__(
        self,
        exc_type: Optional[Type[BaseException]],
        exc_value: Optional[BaseException],
        traceback: Optional[TracebackType],
    ) -> None:
        self.close()


class CLPIRStreamReader(CLPIRBaseReader):
    def __init__(
        self, istream: IO[bytes], decoder_buffer_size: int = 4096, enable_compression: bool = True
    ):
        self._buffered_log_event: Optional[LogEvent] = None
        super().__init__(istream, decoder_buffer_size, enable_compression)

    def read_next_log_event(self) -> Optional[LogEvent]:
        """
        Reads the next log event by decoding the encoded IR stream.

        :return: Next unread log event represented as an instance of LogEvent.
        :return: None if the end of IR stream is reached.
        """
        if None is not self._buffered_log_event:
            buffered_log_event: Optional[LogEvent] = self._buffered_log_event
            self._buffered_log_event = None
            return buffered_log_event
        return Decoder.decode_next_log_event(self._decoder_buffer)

    def skip_to_time(self, timestamp: int) -> int:
        """
        Skip all logs with Unix epoch timestamp before `time_ms`.

        After being called, the next LogEvent returned by the reader (e.g.
        calling `__next__` or `read_next_log_event`) will return the first
        unread log event in the stream whose timestamp is greater or equal to
        the given timestamp.
        :return: Number of logs skipped.
        """
        if False is self.has_metadata():
            raise RuntimeError("The preamble hasn't been successfully decoded.")

        num_log_events_decoded_before_skip: int = (
            self._decoder_buffer.get_num_decoded_log_messages()
        )
        query_timestamp_lower_bound: Query = Query(search_time_lower_bound=timestamp)
        self._buffered_log_event = Decoder.decode_next_log_event(
            self._decoder_buffer, query_timestamp_lower_bound
        )
        num_log_events_skipped: int = (
            self._decoder_buffer.get_num_decoded_log_messages() - num_log_events_decoded_before_skip
        )
        if None is not self._buffered_log_event:
            num_log_events_skipped -= 1
        return num_log_events_skipped

    def __next__(self) -> LogEvent:
        next_log_event: Optional[LogEvent] = self.read_next_log_event()
        if None is next_log_event:
            raise StopIteration
        return next_log_event


class CLPIRFileReader(CLPIRStreamReader):
    def __init__(
        self, fpath: Path, decoder_buffer_size: int = 4096, enable_compression: bool = True
    ):
        self._path: Path = fpath
        super().__init__(open(fpath, "rb"), decoder_buffer_size, enable_compression)

    def dump(self, ostream: IO[str] = stderr) -> None:
        for log_event in self:
            ostream.write(str(log_event))


class CLPIRQueryReader(CLPIRBaseReader):
    def __init__(
        self,
        istream: IO[bytes],
        query: Query,
        decoder_buffer_size: int = 4096,
        enable_compression: bool = True,
    ):
        self._query: Query = query
        super().__init__(istream, decoder_buffer_size, enable_compression)

    def read_next_matched_log_event(self) -> Optional[LogEvent]:
        return Decoder.decode_next_log_event(self._decoder_buffer, self._query)

    def __next__(self) -> LogEvent:
        next_matched_log_event: Optional[LogEvent] = self.read_next_matched_log_event()
        if None is next_matched_log_event:
            raise StopIteration
        return next_matched_log_event


class CLPIRTimeRangeReader(CLPIRQueryReader):
    def __init__(
        self,
        istream: IO[bytes],
        search_time_lower_bound: int,
        search_time_upper_bound: int,
        search_time_termination_margin: int = Query.default_search_time_termination_margin(),
        decoder_buffer_size: int = 4096,
        enable_compression: bool = True,
    ):
        search_time_query: Query = Query(
            search_time_lower_bound=search_time_lower_bound,
            search_time_upper_bound=search_time_upper_bound,
            search_time_termination_margin=search_time_termination_margin,
        )
        super().__init__(istream, search_time_query, decoder_buffer_size, enable_compression)
