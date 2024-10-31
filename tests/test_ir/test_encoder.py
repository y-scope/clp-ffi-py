from test_ir.test_utils import TestCLPBase

from clp_ffi_py.ir import FourByteEncoder


class TestCaseFourByteEncoder(TestCLPBase):
    """
    Class for testing clp_ffi_py.ir.FourByteEncoder.

    The actual functionality should also be covered by the unittest of CLP Python logging library.
    """

    def test_encoding_methods_consistency(self) -> None:
        """
        This test checks if the result of encode_message_and_timestamp_delta is consistent with the
        combination of encode_message and encode_timestamp_delta.
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
