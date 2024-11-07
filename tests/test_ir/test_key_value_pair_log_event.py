from pathlib import Path
from typing import Any, Dict

from test_ir.test_utils import JsonLinesFileReader, TestCLPBase

from clp_ffi_py.ir import KeyValuePairLogEvent


class TestCaseKeyValuePairLogEvent(TestCLPBase):
    """
    Class for testing `clp_ffi_py.ir.KeyValuePairLogEvent`.
    """

    input_src_dir: str = "test_json"

    def test_basic(self) -> None:
        """
        Tests the conversion between a Python dictionary and a `KeyValuePairLogEvent` instance,
        ensuring accurate serialization and deserialization in both directions.
        """
        current_dir: Path = Path(__file__).resolve().parent
        test_src_dir: Path = current_dir / Path(TestCaseKeyValuePairLogEvent.input_src_dir)
        for file_path in test_src_dir.rglob("*"):
            if not file_path.is_file():
                continue
            json_file_reader: JsonLinesFileReader = JsonLinesFileReader(file_path)
            for expected in json_file_reader.read_lines():
                self.assertIsInstance(expected, dict, "Input must be a dictionary")
                actual: KeyValuePairLogEvent = KeyValuePairLogEvent(expected)
                serialized_py_dict: Dict[Any, Any] = actual.to_dict()
                self.assertEqual(expected, serialized_py_dict)
