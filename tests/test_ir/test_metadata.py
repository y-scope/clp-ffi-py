from datetime import tzinfo
from typing import Optional

import dateutil.tz
from test_ir.test_utils import TestCLPBase

from clp_ffi_py.ir import Metadata


class TestCaseMetadata(TestCLPBase):
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
        self._check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

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
        self._check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

        metadata = Metadata(
            timestamp_format=timestamp_format, ref_timestamp=ref_timestamp, timezone_id=timezone_id
        )
        self._check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

    def test_timezone_new_reference(self) -> None:
        """
        Test the timezone is a new reference returned.
        """
        ref_timestamp: int = 2005689603190
        timestamp_format: str = "yy/MM/dd HH:mm:ss"
        timezone_id: str = "America/Chicago"
        metadata: Metadata = Metadata(ref_timestamp, timestamp_format, timezone_id)
        self._check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)

        wrong_tz: Optional[tzinfo] = metadata.get_timezone()
        self.assertEqual(wrong_tz is metadata.get_timezone(), True)

        wrong_tz = dateutil.tz.gettz("America/New_York")
        self.assertEqual(wrong_tz is not metadata.get_timezone(), True)

        self._check_metadata(metadata, ref_timestamp, timestamp_format, timezone_id)
