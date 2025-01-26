from pathlib import Path
from typing import Any, Dict, IO, List, Optional, Tuple

from smart_open import open  # type: ignore
from test_ir.test_utils import get_current_timestamp, JsonLinesFileReader, TestCLPBase

from clp_ffi_py.ir import Deserializer, IncompleteStreamError, KeyValuePairLogEvent, Serializer
from clp_ffi_py.utils import serialize_dict_to_msgpack

LOG_DIR: Path = Path("unittest-logs")


class TestCaseSerDerBase(TestCLPBase):
    """
    Class for testing serialization and deserialization of CLP key-value pair IR stream.
    """

    test_data_dir: Path = Path(__file__).resolve().parent / "test_data/jsonl"
    user_defined_metadata: Dict[str, Any] = {
        "int": 1,
        "float": 1.0,
        "bool": True,
        "str": "String",
        "null": None,
        "array": [1, 1.0, True, "String", None],
        "object": {
            "int": 1,
            "float": 1.0,
            "bool": True,
            "str": "String",
            "null": None,
            "array": [1, 1.0, True, "String", None],
        },
    }

    enable_compression: bool
    generate_incomplete_ir: bool

    ir_stream_path_prefix: str
    ir_stream_path_postfix: str

    # override
    @classmethod
    def setUpClass(cls) -> None:
        if not LOG_DIR.exists():
            LOG_DIR.mkdir(parents=True, exist_ok=True)
        assert LOG_DIR.is_dir()

    # override
    def setUp(self) -> None:
        self.ir_stream_path_prefix: str = f"{self.id()}"
        file_extension_name: str = "clp.zst" if self.enable_compression else "clp"
        self.ir_stream_path_postfix: str = (
            f"incomplete.{file_extension_name}"
            if self.generate_incomplete_ir
            else file_extension_name
        )

    def _serialize(
        self, inputs: List[Tuple[Dict[Any, Any], Dict[Any, Any]]], ir_stream_path: Path
    ) -> None:
        """
        Serializes the given JSON Lines file into CLP key-value pair IR stream.

        :param inputs: A list of dictionary tuples (auto-generated, user-generated) to serialize.
        :param ir_stream_path: Path to the output file that the serializer writes to.
        """
        file_stream: IO[bytes] = open(ir_stream_path, "wb")
        serializer: Serializer = Serializer(
            file_stream, user_defined_metadata=TestCaseSerDerBase.user_defined_metadata
        )
        for auto_gen_dict, user_gen_dict in inputs:
            serializer.serialize_log_event_from_msgpack_map(
                auto_gen_msgpack_map=serialize_dict_to_msgpack(auto_gen_dict),
                user_gen_msgpack_map=serialize_dict_to_msgpack(user_gen_dict),
            )
        serializer.flush()
        if self.generate_incomplete_ir:
            file_stream.close()
            return
        serializer.close()

    def _deserialize(
        self,
        ir_stream_path: Path,
        buffer_capacity: int,
        allow_incomplete_ir_stream: bool,
        expected_outputs: List[Tuple[Dict[Any, Any], Dict[Any, Any]]],
    ) -> None:
        """
        Deserializes the input CLP key-value pair IR stream and compare the deserialized log events
        with the given expected outputs.

        :param ir_stream_path: Path to the input file that the deserializers reads from.
        :param buffer_capacity: Buffer capacity used to create the deserializer.
        :param allow_incomplete_ir_stream: Whether to allow incomplete IR streams.
        :param expected_outputs: A list of dictionary tuples (auto-generated, user-generated) as the
            expected outputs.
        """
        input_stream: IO[bytes] = open(ir_stream_path, "rb")
        deserializer: Deserializer = Deserializer(
            input_stream,
            allow_incomplete_stream=allow_incomplete_ir_stream,
            buffer_capacity=buffer_capacity,
        )
        for expected_auto_gen_dict, expected_user_gen_dict in expected_outputs:
            deserialized_log_event: Optional[KeyValuePairLogEvent] = (
                deserializer.deserialize_log_event()
            )
            self.assertNotEqual(deserialized_log_event, None)
            assert deserialized_log_event is not None  # To silent mypy
            actual_auto_gen_dict, actual_user_gen_dict = deserialized_log_event.to_dict()
            self.assertEqual(expected_auto_gen_dict, actual_auto_gen_dict)
            self.assertEqual(expected_user_gen_dict, actual_user_gen_dict)

        if not self.generate_incomplete_ir or allow_incomplete_ir_stream:
            self.assertEqual(None, deserializer.deserialize_log_event())
            return

        with self.assertRaises(IncompleteStreamError):
            deserializer.deserialize_log_event()

        self.assertEqual(
            TestCaseSerDerBase.user_defined_metadata, deserializer.get_user_defined_metadata()
        )

    def _get_ir_stream_path(
        self,
        jsonl_path: Path,
    ) -> Path:
        ir_stream_path: Path = LOG_DIR / Path(
            f"{self.ir_stream_path_prefix}.{str(jsonl_path.stem)}.{self.ir_stream_path_postfix}"
        )
        if ir_stream_path.exists():
            ir_stream_path.unlink()
        return ir_stream_path

    def test_serder(self) -> None:
        """
        Tests serializing and deserializing CLP key-value pair IR streams.
        """
        jsonl_file_paths: List[Path] = []

        for file_path in TestCaseSerDerBase.test_data_dir.rglob("*"):
            if not file_path.is_file():
                continue
            jsonl_file_paths.append(file_path)
        self.assertNotEqual(len(jsonl_file_paths), 0, "No test files")

        for jsonl_file_path in jsonl_file_paths:
            expected: List[Tuple[Dict[Any, Any], Dict[Any, Any]]] = []
            for json_obj in JsonLinesFileReader(jsonl_file_path).read_lines():
                expected.append(({}, json_obj))
                expected.append(
                    ({"Timestamp": get_current_timestamp(), "Inner": json_obj}, json_obj)
                )
            ir_stream_path: Path = self._get_ir_stream_path(jsonl_file_path)
            self._serialize(expected, ir_stream_path)
            for buffer_capacity in [1, 16, 256, 4096, 65536]:
                self._deserialize(ir_stream_path, buffer_capacity, False, expected)
            if self.generate_incomplete_ir:
                self._deserialize(ir_stream_path, 65536, True, expected)


class TestCaseSerDerRaw(TestCaseSerDerBase):
    """
    Tests serialization and deserialization against raw IR stream.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = False
        self.generate_incomplete_ir = False
        super().setUp()


class TestCaseSerDerIncompleteRaw(TestCaseSerDerBase):
    """
    Tests serialization and deserialization against raw incomplete IR stream.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = False
        self.generate_incomplete_ir = True
        super().setUp()


class TestCaseSerDerZstd(TestCaseSerDerBase):
    """
    Tests serialization and deserialization against zstd-compressed IR stream.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = True
        self.generate_incomplete_ir = False
        super().setUp()


class TestCaseSerDerIncompleteZstd(TestCaseSerDerBase):
    """
    Tests serialization and deserialization against zstd-compressed incomplete IR stream.
    """

    # override
    def setUp(self) -> None:
        self.enable_compression = True
        self.generate_incomplete_ir = True
        super().setUp()
