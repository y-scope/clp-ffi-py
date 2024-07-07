import pickle
from datetime import tzinfo
from typing import Optional

import dateutil.tz
from test_ir.test_utils import TestCLPBase

from clp_ffi_py.ir import LogEvent, Metadata


class TestCaseLogEvent(TestCLPBase):
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
        self._check_log_event(log_event, log_message, timestamp, idx)

        log_event = LogEvent(log_message, timestamp)
        self._check_log_event(log_event, log_message, timestamp, 0)

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
        self._check_log_event(log_event, log_message, timestamp, idx)

        # Initialize with keyword (out-of-order)
        log_event = LogEvent(
            index=idx, timestamp=timestamp, log_message=log_message, metadata=metadata
        )
        self._check_log_event(log_event, log_message, timestamp, idx)

        # Initialize with keyword and default argument (out-of-order)
        log_event = LogEvent(timestamp=timestamp, log_message=log_message)
        self._check_log_event(log_event, log_message, timestamp, 0)

    def test_formatted_message(self) -> None:
        """
        Test the reconstruction of the raw message.

        In particular, it checks if the timestamp is properly formatted with the expected tzinfo
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
        self._check_log_event(log_event, log_message, timestamp, idx)
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
        self._check_log_event(log_event, log_message, timestamp, idx)
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

        For unpickled LogEvent object, even though the metadata is set to None, it should still
        format the timestamp with the original tz before pickling
        """
        log_message: str = " This is a test log message"
        timestamp: int = 932724000000
        idx: int = 3190
        metadata: Optional[Metadata] = Metadata(0, "yy/MM/dd HH:mm:ss", "Asia/Hong_Kong")
        log_event = LogEvent(
            log_message=log_message, timestamp=timestamp, index=idx, metadata=metadata
        )
        self._check_log_event(log_event, log_message, timestamp, idx)
        reconstructed_log_event: LogEvent = pickle.loads(pickle.dumps(log_event))
        self._check_log_event(reconstructed_log_event, log_message, timestamp, idx)

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
