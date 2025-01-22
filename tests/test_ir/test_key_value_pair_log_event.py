from pathlib import Path
from typing import Any, Dict

from test_ir.test_utils import JsonLinesFileReader, TestCLPBase

from clp_ffi_py.ir import KeyValuePairLogEvent


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
