
from io import BytesIO
from pathlib import Path

from test_ir.test_utils import JsonFileReader, TestCLPBase

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

    input_src_dir: str = "test_json"

    def test_serialize_json(self) -> None:
        """
        Tests serializing JSON files.

        The JSON parser will parse the file into Python dictionaries,
        and then convert them into msgpack and feed into `clp_ffi_py.ir.Serializer`.
        """
        current_dir: Path = Path(__file__).resolve().parent
        test_src_dir: Path = current_dir / Path(TestCaseSerializer.input_src_dir)

        byte_buffer: BytesIO
        num_bytes_serialized: int
        serializer: Serializer

        # Test with context manager
        for file_path in test_src_dir.rglob("*"):
            if not file_path.is_file():
                continue
            byte_buffer = BytesIO()
            num_bytes_serialized = 0
            with Serializer(byte_buffer) as serializer:
                for json_obj in JsonFileReader(file_path).read_json_lines():
                    serializer.serialize_msgpack(serialize_dict_to_msgpack(json_obj))
                    num_bytes_serialized += serializer.write_to_stream()
                    self.assertEqual(0, serializer.write_to_stream())
                    serializer.flush_stream()
            self.assertEqual(num_bytes_serialized + 1, len(byte_buffer.getvalue()))
            byte_buffer.close()

        # Test without context manager
        for file_path in test_src_dir.rglob("*"):
            if not file_path.is_file():
                continue
            byte_buffer = BytesIO()
            num_bytes_serialized = 0
            serializer = Serializer(byte_buffer)
            for json_obj in JsonFileReader(file_path).read_json_lines():
                serializer.serialize_msgpack(serialize_dict_to_msgpack(json_obj))
                num_bytes_serialized += serializer.write_to_stream()
                self.assertEqual(0, serializer.write_to_stream())
                serializer.flush_stream()
            self.assertEqual(num_bytes_serialized, len(byte_buffer.getvalue()))
            serializer.close()
            byte_buffer.close()

    def test_closing_empty(self) -> None:
        """
        Tests closing an empty serializer.
        """
        byte_buffer: BytesIO
        serializer: Serializer

        byte_buffer = BytesIO()
        with Serializer(byte_buffer) as serializer:
            serializer.flush_stream()
        self.assertTrue(1 < len(byte_buffer.getvalue()))
        byte_buffer.close()

        byte_buffer = BytesIO()
        serializer = Serializer(byte_buffer)
        serializer.close()
        self.assertTrue(1 < len(byte_buffer.getvalue()))
        byte_buffer.close()
