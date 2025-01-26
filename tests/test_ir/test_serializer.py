from io import BytesIO
from pathlib import Path
from typing import List, Optional

from test_ir.test_utils import JsonLinesFileReader, TestCLPBase

from clp_ffi_py.ir import FourByteSerializer, Serializer
from clp_ffi_py.utils import serialize_dict_to_msgpack


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


class TestCaseSerializer(TestCLPBase):
    """
    Class for testing `clp_ffi_py.ir.Serializer`.
    """

    jsonl_test_data_dir: Path = Path("test_data") / "jsonl"
    current_dir: Path = Path(__file__).resolve().parent
    test_data_dir: Path = current_dir / jsonl_test_data_dir

    def test_serialize_json(self) -> None:
        """
        Tests serializing JSON files.

        The JSON parser will parse the file into Python dictionaries,
        and then convert them into msgpack and feed into `clp_ffi_py.ir.Serializer`.
        """

        test_files: List[Path] = self.__get_test_files()
        byte_buffer: BytesIO
        num_bytes_serialized: int
        serializer: Serializer
        msgpack_byte_sequence: bytes

        # Test with context manager
        for file_path in test_files:
            byte_buffer = BytesIO()
            with Serializer(byte_buffer) as serializer:
                num_bytes_serialized = serializer.get_num_bytes_serialized()
                for json_obj in JsonLinesFileReader(file_path).read_lines():
                    msgpack_byte_sequence = serialize_dict_to_msgpack(json_obj)
                    num_bytes_serialized += serializer.serialize_log_event_from_msgpack_map(
                        auto_gen_msgpack_map=msgpack_byte_sequence,
                        user_gen_msgpack_map=msgpack_byte_sequence,
                    )
                    serializer.flush()
                    self.assertEqual(
                        len(byte_buffer.getvalue()), serializer.get_num_bytes_serialized()
                    )
                    self.assertEqual(num_bytes_serialized, serializer.get_num_bytes_serialized())

        # Test without context manager
        for file_path in test_files:
            byte_buffer = BytesIO()
            serializer = Serializer(byte_buffer)
            num_bytes_serialized = serializer.get_num_bytes_serialized()
            for json_obj in JsonLinesFileReader(file_path).read_lines():
                msgpack_byte_sequence = serialize_dict_to_msgpack(json_obj)
                num_bytes_serialized += serializer.serialize_log_event_from_msgpack_map(
                    auto_gen_msgpack_map=msgpack_byte_sequence,
                    user_gen_msgpack_map=msgpack_byte_sequence,
                )
                serializer.flush()
                self.assertEqual(len(byte_buffer.getvalue()), serializer.get_num_bytes_serialized())
                self.assertEqual(num_bytes_serialized, serializer.get_num_bytes_serialized())
            serializer.close()
            self.assertEqual(
                num_bytes_serialized + 1,
                serializer.get_num_bytes_serialized(),
                "End-of-stream byte is missing",
            )

    def test_serialize_with_customized_buffer_size_limit(self) -> None:
        """
        Tests serializing with customized buffer size limit.
        """
        buffer_size_limit: int = 3000
        for file_path in self.__get_test_files():
            byte_buffer: BytesIO = BytesIO()
            with Serializer(
                buffer_size_limit=buffer_size_limit, output_stream=byte_buffer
            ) as serializer:
                serializer.flush()
                num_bytes_in_ir_buffer: int = 0
                for json_obj in JsonLinesFileReader(file_path).read_lines():
                    msgpack_byte_sequence: bytes = serialize_dict_to_msgpack(json_obj)
                    num_bytes_serialized: int = serializer.serialize_log_event_from_msgpack_map(
                        auto_gen_msgpack_map=msgpack_byte_sequence,
                        user_gen_msgpack_map=msgpack_byte_sequence,
                    )
                    self.assertNotEqual(0, num_bytes_serialized)
                    num_bytes_in_ir_buffer += num_bytes_serialized
                    if num_bytes_in_ir_buffer > buffer_size_limit:
                        # The IR buffer should already be written to the stream
                        self.assertEqual(
                            serializer.get_num_bytes_serialized(), len(byte_buffer.getvalue())
                        )
                        num_bytes_in_ir_buffer = 0
                    else:
                        self.assertEqual(
                            serializer.get_num_bytes_serialized(),
                            num_bytes_in_ir_buffer + len(byte_buffer.getvalue()),
                        )

    def test_closing_empty(self) -> None:
        """
        Tests closing an empty serializer.
        """
        byte_buffer: BytesIO
        serializer: Serializer

        byte_buffer = BytesIO()
        preamble_size: int
        with Serializer(byte_buffer, 0) as serializer:
            self.assertTrue(len(byte_buffer.getvalue()) > 0)
            self.assertEqual(len(byte_buffer.getvalue()), serializer.get_num_bytes_serialized())
            preamble_size = serializer.get_num_bytes_serialized()

        byte_buffer = BytesIO()
        serializer = Serializer(byte_buffer)
        serializer.close()
        self.assertEqual(
            serializer.get_num_bytes_serialized(),
            preamble_size + 1,
            "End-of-stream byte is missing",
        )

    def test_not_closed(self) -> None:
        serializer: Optional[Serializer] = Serializer(BytesIO())
        with self.assertWarns(ResourceWarning) as _:
            serializer = None  # noqa

    def test_invalid_user_defined_metadata(self) -> None:
        byte_buffer: BytesIO = BytesIO()
        with self.assertRaises(TypeError):
            _ = Serializer(byte_buffer, user_defined_metadata="str")  # type: ignore
        with self.assertRaises(TypeError):
            _ = Serializer(byte_buffer, user_defined_metadata=[1, "str"])  # type: ignore

    def __get_test_files(self) -> List[Path]:
        test_files: List[Path] = []
        for file_path in TestCaseSerializer.test_data_dir.rglob("*"):
            if not file_path.is_file():
                continue
            test_files.append(file_path)
        self.assertFalse(0 == len(test_files), "No test files found")
        return test_files
