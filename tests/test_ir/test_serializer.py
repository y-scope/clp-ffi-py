from test_ir.test_utils import TestCLPBase

from clp_ffi_py.ir import FourByteSerializer


class TestCaseFourByteSerializer(TestCLPBase):
    """
    Class for testing clp_ffi_py.ir.FourByteSerializer.

    The actual functionality should also be covered by the unittest of CLP Python logging library.
    """

    def test_init(self) -> None:
        type_error_exception_captured: bool = False
        four_byte_serializer: FourByteSerializer
        try:
            four_byte_serializer = FourByteSerializer()  # noqa
        except TypeError:
            type_error_exception_captured = True
        self.assertEqual(
            type_error_exception_captured, True, "FourByteSerializer should be non-instantiable."
        )

    def test_serialization_methods_consistency(self) -> None:
        """
        This test checks if the result of serialize_message_and_timestamp_delta is consistent with
        the combination of serialize_message and serialize_timestamp_delta.
        """
        timestamp_delta: int = -3190
        log_message: str = "This is a test message: Do NOT Reply!"
        serialized_message_and_ts_delta: bytearray = (
            FourByteSerializer.serialize_message_and_timestamp_delta(
                timestamp_delta, log_message.encode()
            )
        )
        serialized_message: bytearray = FourByteSerializer.serialize_message(log_message.encode())
        serialized_ts_delta: bytearray = FourByteSerializer.serialize_timestamp_delta(
            timestamp_delta
        )
        self.assertEqual(serialized_message_and_ts_delta, serialized_message + serialized_ts_delta)
