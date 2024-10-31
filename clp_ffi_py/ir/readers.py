from __future__ import annotations

from pathlib import Path
from sys import stderr
from types import TracebackType
from typing import Generator, IO, Iterator, Optional, Type, Union
from warnings import warn

from zstandard import ZstdDecompressionReader, ZstdDecompressor

from clp_ffi_py.ir.native import DeserializerBuffer, FourByteDeserializer, LogEvent, Metadata, Query


class ClpIrStreamReader(Iterator[LogEvent]):
    """
    This class represents a stream reader used to read/deserialize log events from a CLP IR stream.
    It also provides method(s) to instantiate a log event generator with a customized search query.

    :param istream: Input stream that contains CLP IR byte sequence.
    :param deserializer_buffer_size: Initial size of the deserializer buffer.
    :param enable_compression: A flag indicating whether the istream is compressed using `zstd`.
    :param allow_incomplete_stream: If set to `True`, an incomplete CLP IR stream is not treated as
        an error. Instead, encountering such a stream is seen as reaching its end without raising
        any exceptions.
    :param decoder_buffer_size: Deprecated since 0.0.13. Use `deserializer_buffer_size` instead.
        This argument is provided for backward compatibility and if set, will overwrite
        `deserializer_buffer_size`'s value.
    """

    DEFAULT_DESERIALIZER_BUFFER_SIZE: int = 65536
    DEFAULT_DECODER_BUFFER_SIZE: int = DEFAULT_DESERIALIZER_BUFFER_SIZE

    def __init__(
        self,
        istream: IO[bytes],
        deserializer_buffer_size: int = DEFAULT_DESERIALIZER_BUFFER_SIZE,
        enable_compression: bool = True,
        allow_incomplete_stream: bool = False,
        decoder_buffer_size: Optional[int] = None,
    ):
        if decoder_buffer_size is not None:
            deserializer_buffer_size = decoder_buffer_size
            warn(
                "Argument `decoder_buffer_size` has been renamed to `deserializer_buffer_size`"
                " since 0.0.13",
                category=DeprecationWarning,
                stacklevel=2,
            )

        self.__istream: Union[IO[bytes], ZstdDecompressionReader]
        if enable_compression:
            dctx: ZstdDecompressor = ZstdDecompressor()
            self.__istream = dctx.stream_reader(istream, read_across_frames=True)
        else:
            self.__istream = istream
        self._deserializer_buffer: DeserializerBuffer = DeserializerBuffer(
            self.__istream, deserializer_buffer_size
        )
        self._metadata: Optional[Metadata] = None
        self._allow_incomplete_stream: bool = allow_incomplete_stream

    def read_next_log_event(self) -> Optional[LogEvent]:
        """
        Reads and deserializes the next log event from the IR stream.

        :return:
            - Next unread log event represented as an instance of LogEvent.
            - None if the end of IR stream is reached.
        :raise Exception:
            If :meth:`~clp_ffi_py.ir.native.FourByteDeserializer.deserialize_next_log_event` fails.
        """
        return FourByteDeserializer.deserialize_next_log_event(
            self._deserializer_buffer, allow_incomplete_stream=self._allow_incomplete_stream
        )

    def read_preamble(self) -> None:
        """
        Try to deserialize the preamble and set `metadata`. If `metadata` has been set already, it
        will instantly return. It is separated from `__init__` so that the input stream does not
        need to be readable on a reader's construction, but until the user starts to iterate logs.

        :raise Exception:
            If :meth:`~clp_ffi_py.ir.native.FourByteDeserializer.deserialize_preamble` fails.
        """
        if self.has_metadata():
            return
        self._metadata = FourByteDeserializer.deserialize_preamble(self._deserializer_buffer)

    def get_metadata(self) -> Metadata:
        if None is self._metadata:
            raise RuntimeError("The metadata has not been successfully deserialized yet.")
        return self._metadata

    def has_metadata(self) -> bool:
        return None is not self._metadata

    def search(self, query: Query) -> Generator[LogEvent, None, None]:
        """
        Searches and yields log events that match a specific search query.

        :param query: The input query object used to match log events. Check the document of
            :class:`~clp_ffi_py.ir.Query` for more details.
        :yield: The next unread log event that matches the given search query from the IR stream.
        """
        if False is self.has_metadata():
            self.read_preamble()
        while True:
            log_event: Optional[LogEvent] = FourByteDeserializer.deserialize_next_log_event(
                self._deserializer_buffer,
                query=query,
                allow_incomplete_stream=self._allow_incomplete_stream,
            )
            if None is log_event:
                break
            yield log_event

    def close(self) -> None:
        self.__istream.close()

    def __iter__(self) -> ClpIrStreamReader:
        if False is self.has_metadata():
            self.read_preamble()
        return self

    def __enter__(self) -> ClpIrStreamReader:
        if False is self.has_metadata():
            self.read_preamble()
        return self

    def __next__(self) -> LogEvent:
        next_log_event: Optional[LogEvent] = self.read_next_log_event()
        if None is next_log_event:
            raise StopIteration
        return next_log_event

    def __exit__(
        self,
        exc_type: Optional[Type[BaseException]],
        exc_value: Optional[BaseException],
        traceback: Optional[TracebackType],
    ) -> None:
        self.close()


class ClpIrFileReader(ClpIrStreamReader):
    """
    Wrapper class of `ClpIrStreamReader` that calls `open` for convenience.
    """

    def __init__(
        self,
        fpath: Path,
        deserializer_buffer_size: int = ClpIrStreamReader.DEFAULT_DESERIALIZER_BUFFER_SIZE,
        enable_compression: bool = True,
        allow_incomplete_stream: bool = False,
        decoder_buffer_size: Optional[int] = None,
    ):
        if decoder_buffer_size is not None:
            deserializer_buffer_size = decoder_buffer_size
            warn(
                "Argument `decoder_buffer_size` has been renamed to `deserializer_buffer_size`"
                " since 0.0.13",
                category=DeprecationWarning,
                stacklevel=2,
            )
        self._path: Path = fpath
        super().__init__(
            open(fpath, "rb"),
            deserializer_buffer_size=deserializer_buffer_size,
            enable_compression=enable_compression,
            allow_incomplete_stream=allow_incomplete_stream,
        )

    def dump(self, ostream: IO[str] = stderr) -> None:
        for log_event in self:
            ostream.write(str(log_event))
