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
        for file_path in test_data_dir.rglob("*"):
            if not file_path.is_file():
                continue
            json_file_reader: JsonLinesFileReader = JsonLinesFileReader(file_path)
            for expected in json_file_reader.read_lines():
                self.assertIsInstance(expected, dict, "Input must be a dictionary")
                actual: KeyValuePairLogEvent = KeyValuePairLogEvent(expected)
                serialized_py_dict: Dict[Any, Any] = actual.to_dict()
                self.assertEqual(expected, serialized_py_dict)
            num_files_tested += 1
        self.assertNotEqual(
            num_files_tested, 0, f"No test files found in directory: {test_data_dir}"
        )
