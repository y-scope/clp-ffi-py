from io import BytesIO
from pathlib import Path
from typing import Any, Dict, Optional

from test_ir.test_utils import JsonLinesFileReader, TestCLPBase

from clp_ffi_py.ir import Deserializer, KeyValuePairLogEvent, Serializer


class TestCaseKeyValuePairLogEvent(TestCLPBase):
    """
    Class for testing `clp_ffi_py.ir.KeyValuePairLogEvent`.
    """

    jsonl_test_data_dir: Path = Path("test_data") / "jsonl"

    def test_basic(self) -> None:
        """
        Tests the conversion between a Python dictionary and a `KeyValuePairLogEvent` instance,
        ensuring accurate serialization and deserialization in both directions.
        """
        current_dir: Path = Path(__file__).resolve().parent
        test_data_dir: Path = current_dir / TestCaseKeyValuePairLogEvent.jsonl_test_data_dir
        num_files_tested: int = 0
        empty_dict: Dict[str, Any] = {}
        for file_path in test_data_dir.rglob("*"):
            if not file_path.is_file():
                continue
            json_file_reader: JsonLinesFileReader = JsonLinesFileReader(file_path)
            for expected_user_gen_dict in json_file_reader.read_lines():
                self.assertIsInstance(expected_user_gen_dict, dict, "Input must be a dictionary")
                for expected_auto_gen_dict in (empty_dict, {"auto_gen": expected_user_gen_dict}):
                    actual: KeyValuePairLogEvent = KeyValuePairLogEvent(
                        auto_gen_kv_pairs=expected_auto_gen_dict,
                        user_gen_kv_pairs=expected_user_gen_dict,
                    )
                    serialized_auto_gen_dict, serialized_user_gen_dict = actual.to_dict()
                    self.assertEqual(expected_auto_gen_dict, serialized_auto_gen_dict)
                    self.assertEqual(expected_user_gen_dict, serialized_user_gen_dict)

            num_files_tested += 1
        self.assertNotEqual(
            num_files_tested, 0, f"No test files found in directory: {test_data_dir}"
        )

    def test_invalid_utf8_encoding(self) -> None:
        """
        Tests handling of invalid UTF-8 encoded strings.
        """
        encoding_type: str = "cp932"

        # msgpack map: {"key": 0x970x5c}, where "0x970x5c" is encoded using "cp932"
        msgpack_with_invalid_utf8_str: bytes = b"\x81\xa3\x6b\x65\x79\xa2\x97\x5c"
        expected_dict_with_proper_encoding: Dict[str, str] = {
            "key": str(b"\x97\x5c", encoding=encoding_type)
        }
        expected_dict_with_ignore: Dict[str, str] = {"key": str(b"\x97\x5c", errors="ignore")}

        byte_buffer: BytesIO = BytesIO()
        serializer: Serializer = Serializer(byte_buffer)
        serializer.serialize_log_event_from_msgpack_map(
            msgpack_with_invalid_utf8_str, msgpack_with_invalid_utf8_str
        )
        serializer.flush()

        byte_buffer.seek(0)
        deserializer: Deserializer = Deserializer(byte_buffer)
        log_event: Optional[KeyValuePairLogEvent] = deserializer.deserialize_log_event()
        self.assertNotEqual(log_event, None)
        assert log_event is not None

        with self.assertRaises(UnicodeDecodeError):
            _, _ = log_event.to_dict()

        with self.assertRaises(UnicodeDecodeError):
            _, _ = log_event.to_dict(encoding="big5")

        actual_auto_gen_dict, actual_user_gen_dict = log_event.to_dict(encoding=encoding_type)
        self.assertEqual(expected_dict_with_proper_encoding, actual_auto_gen_dict)
        self.assertEqual(expected_dict_with_proper_encoding, actual_user_gen_dict)

        actual_auto_gen_dict_with_ignore, actual_user_gen_dict_with_ignore = log_event.to_dict(
            errors="ignore"
        )
        self.assertEqual(expected_dict_with_ignore, actual_auto_gen_dict_with_ignore)
        self.assertEqual(expected_dict_with_ignore, actual_user_gen_dict_with_ignore)
