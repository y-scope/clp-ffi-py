import dateutil.tz
import pickle
import unittest

from clp_ffi_py.ir import FourByteEncoder, LogEvent, Metadata
from datetime import tzinfo
from typing import Optional


class TestCaseMetadata(unittest.TestCase):
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
        self.__check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

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
        self.__check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

        metadata = Metadata(
            timestamp_format=timestamp_format, ref_timestamp=ref_timestamp, timezone_id=timezone_id
        )
        self.__check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

    def test_timezone_new_reference(self) -> None:
        """
        Test the timezone is a new reference returned.
        """
        ref_timestamp: int = 2005689603190
        timestamp_format: str = "yy/MM/dd HH:mm:ss"
        timezone_id: str = "America/Chicago"
        metadata: Metadata = Metadata(ref_timestamp, timestamp_format, timezone_id)
        self.__check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

        wrong_tz: Optional[tzinfo] = metadata.get_timezone()
        self.assertEqual(wrong_tz is metadata.get_timezone(), True)

        wrong_tz = dateutil.tz.gettz("America/New_York")
        self.assertEqual(wrong_tz is not metadata.get_timezone(), True)

        self.__check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

    def __check_metadata(
        self,
        metadata: Metadata,
        expected_ref_timestamp: int,
        expected_timestamp_format: str,
        expected_timezone_id: str,
    ) -> None:
        """
        Given a Metadata object, check if the content matches the reference.

        :param metadata: Metadata object to be checked.
        :param expected_ref_timestamp: Expected reference timestamp.
        :param expected_timestamp_format: Expected timestamp format.
        :param expected_timezone_id: Expected timezone ID.
        """
        ref_timestamp: int = metadata.get_ref_timestamp()
        timestamp_format: str = metadata.get_timestamp_format()
        timezone_id: str = metadata.get_timezone_id()
        timezone: tzinfo = metadata.get_timezone()

        self.assertEqual(
            ref_timestamp,
            expected_ref_timestamp,
            f'Reference Timestamp: "{ref_timestamp}", Expected: "{expected_ref_timestamp}"',
        )
        self.assertEqual(
            timestamp_format,
            expected_timestamp_format,
            f'Timestamp Format: "{timestamp_format}", Expected: "{expected_timestamp_format}"',
        )
        self.assertEqual(
            timezone_id,
            expected_timezone_id,
            f'Timezone ID: "{timezone_id}", Expected: "{expected_timezone_id}"',
        )

        expected_tzinfo: Optional[tzinfo] = dateutil.tz.gettz(expected_timezone_id)
        assert expected_tzinfo is not None
        is_the_same_tz: bool = expected_tzinfo is timezone
        self.assertEqual(
            is_the_same_tz,
            True,
            f"Timezone does not match timezone id. Timezone ID: {timezone_id}, Timezone:"
            f' {str(timezone)}"',
        )


class TestCaseLogEvent(unittest.TestCase):
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
        self.__check_log_event(log_event, log_message, timestamp, idx)

        log_event = LogEvent(log_message, timestamp)
        self.__check_log_event(log_event, log_message, timestamp, 0)

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
        self.__check_log_event(log_event, log_message, timestamp, idx)

        # Initialize with keyword (out-of-order)
        log_event = LogEvent(
            index=idx, timestamp=timestamp, log_message=log_message, metadata=metadata
        )
        self.__check_log_event(log_event, log_message, timestamp, idx)

        # Initialize with keyword and default argument (out-of-order)
        log_event = LogEvent(timestamp=timestamp, log_message=log_message)
        self.__check_log_event(log_event, log_message, timestamp, 0)

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
        self.__check_log_event(log_event, log_message, timestamp, idx)
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
        self.__check_log_event(log_event, log_message, timestamp, idx)
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
        self.__check_log_event(log_event, log_message, timestamp, idx)
        reconstructed_log_event: LogEvent = pickle.loads(pickle.dumps(log_event))
        self.__check_log_event(reconstructed_log_event, log_message, timestamp, idx)

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

    def __check_log_event(
        self,
        log_event: LogEvent,
        expected_log_message: str,
        expected_timestamp: int,
        expected_idx: int,
    ) -> None:
        """
        Given a LogEvent object, check if the content matches the reference.

        :param log_event: LogEvent object to be checked.
        :param expected_log_message: Expected log message.
        :param expected_timestamp: Expected timestamp.
        :param expected_idx: Expected log event index.
        """
        log_message: str = log_event.get_log_message()
        timestamp: int = log_event.get_timestamp()
        idx: int = log_event.get_index()
        self.assertEqual(
            log_message,
            expected_log_message,
            f'Log message: "{log_message}", Expected: "{expected_log_message}"',
        )
        self.assertEqual(
            timestamp,
            expected_timestamp,
            f'Timestamp: "{timestamp}", Expected: "{expected_log_message}"',
        )
        self.assertEqual(idx, expected_idx, f"Message idx: {idx}, Expected: {expected_idx}")


class TestCaseFourByteEncoder(unittest.TestCase):
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


if __name__ == "__main__":
    unittest.main()
