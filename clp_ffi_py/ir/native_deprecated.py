from typing import IO, Optional

from deprecated.sphinx import deprecated

from clp_ffi_py.ir.native import (
    DeserializerBuffer,
    FourByteDeserializer,
    FourByteSerializer,
    LogEvent,
    Metadata,
    Query,
)


@deprecated(
    version="0.0.13",
    reason=":class:`FourByteEncoder` is deprecated and has been renamed to"
    " :class:`~clp_ffi_py.ir.native.FourByteSerializer`.",
)
class FourByteEncoder:
    @staticmethod
    def encode_preamble(ref_timestamp: int, timestamp_format: str, timezone: str) -> bytearray:
        """
        This method is deprecated and has been renamed to
        :meth:`~clp_ffi_py.ir.native.FourByteSerializer.serialize_preamble`.
        """
        return FourByteSerializer.serialize_preamble(ref_timestamp, timestamp_format, timezone)

    @staticmethod
    def encode_message_and_timestamp_delta(timestamp_delta: int, msg: bytes) -> bytearray:
        """
        This method is deprecated and has been renamed to
        :meth:`~clp_ffi_py.ir.native.FourByteSerializer.serialize_message_and_timestamp_delta`.
        """
        return FourByteSerializer.serialize_message_and_timestamp_delta(
            timestamp_delta,
            msg,
        )

    @staticmethod
    def encode_message(msg: bytes) -> bytearray:
        """
        This method is deprecated and has been renamed to
        :meth:`~clp_ffi_py.ir.native.FourByteSerializer.serialize_message`.
        """
        return FourByteSerializer.serialize_message(msg)

    @staticmethod
    def encode_timestamp_delta(timestamp_delta: int) -> bytearray:
        """
        This method is deprecated and has been renamed to
        :meth:`~clp_ffi_py.ir.native.FourByteSerializer.serialize_timestamp_delta`.
        """
        return FourByteSerializer.serialize_timestamp_delta(timestamp_delta)

    @staticmethod
    def encode_end_of_ir() -> bytearray:
        """
        This method is deprecated and has been renamed to
        :meth:`~clp_ffi_py.ir.native.FourByteSerializer.serialize_end_of_ir`.
        """
        return FourByteSerializer.serialize_end_of_ir()


@deprecated(
    version="0.0.13",
    reason=":class:`DecoderBuffer` is deprecated and has been renamed to"
    " :class:`~clp_ffi_py.ir.native.DeserializerBuffer`.",
)
class DecoderBuffer:
    def __init__(self, input_stream: IO[bytes], initial_buffer_capacity: int = 4096):
        self._deserializer_buffer = DeserializerBuffer(
            input_stream=input_stream, initial_buffer_capacity=initial_buffer_capacity
        )

    def get_num_decoded_log_messages(self) -> int:
        """
        This method is deprecated and has been renamed to
        :meth:`~clp_ffi_py.ir.native.DeserializerBuffer.get_num_deserialized_log_messages`.
        """
        return self._deserializer_buffer.get_num_deserialized_log_messages()

    def _test_streaming(self, seed: int) -> bytearray:
        """
        See :meth:`~clp_ffi_py.ir.native.DeserializerBuffer._test_streaming`.
        """
        return self._deserializer_buffer._test_streaming(seed)


@deprecated(
    version="0.0.13",
    reason=":class:`Decoder` is deprecated and has been renamed to"
    " :class:`~clp_ffi_py.ir.native.FourByteDeserializer`.",
)
class Decoder:
    @staticmethod
    def decode_preamble(decoder_buffer: DecoderBuffer) -> Metadata:
        """
        This method is deprecated and has been renamed to
        :meth:`~clp_ffi_py.ir.native.FourByteDeserializer.deserialize_preamble`.
        """
        return FourByteDeserializer.deserialize_preamble(decoder_buffer._deserializer_buffer)

    @staticmethod
    def decode_next_log_event(
        decoder_buffer: DecoderBuffer,
        query: Optional[Query] = None,
        allow_incomplete_stream: bool = False,
    ) -> Optional[LogEvent]:
        """
        This method is deprecated and has been renamed to
        :meth:`~clp_ffi_py.ir.native.FourByteDeserializer.deserialize_next_log_event`.
        """
        return FourByteDeserializer.deserialize_next_log_event(
            deserializer_buffer=decoder_buffer._deserializer_buffer,
            query=query,
            allow_incomplete_stream=allow_incomplete_stream,
        )


# Delete `__new__` so that it will be ignored by Sphinx.
del Decoder.__new__
del FourByteEncoder.__new__
