import io
import random
from pathlib import Path
from typing import Optional

from smart_open import open  # type: ignore
from test_ir.test_utils import TestCLPBase

from clp_ffi_py.ir import DeserializerBuffer


class TestCaseDeserializerBuffer(TestCLPBase):
    """
    Class for testing clp_ffi_py.ir.DeserializerBuffer.
    """

    deserializer_buffer_test_data_dir: Path = Path("test_data") / "unstructured_ir"

    def test_buffer_protocol(self) -> None:
        """
        Tests whether the buffer protocol is disabled by default.
        """
        byte_array: bytearray = bytearray(b"Hello, world!")
        byte_stream: io.BytesIO = io.BytesIO(byte_array)
        deserializer_buffer: DeserializerBuffer = DeserializerBuffer(byte_stream)
        exception_captured: bool = False
        try:
            byte_stream.readinto(deserializer_buffer)  # type: ignore
        except TypeError:
            exception_captured = True
        self.assertTrue(
            exception_captured, "The buffer protocol should not be enabled in Python layer."
        )

    def test_streaming_small_buffer(self) -> None:
        """
        Tests DeserializerBuffer's functionality using the small buffer capacity.
        """
        buffer_capacity: int = 1024
        self.__launch_test(buffer_capacity)

    def test_streaming_default_buffer(self) -> None:
        """
        Tests DeserializerBuffer's functionality using the default buffer capacity.
        """
        self.__launch_test(None)

    def test_streaming_large_buffer(self) -> None:
        """
        Tests DeserializerBuffer's functionality using the large buffer capacity.
        """
        buffer_capacity: int = 16384
        self.__launch_test(buffer_capacity)

    def __launch_test(self, buffer_capacity: Optional[int]) -> None:
        """
        Tests the DeserializerBuffer by streaming the files inside `test_src_dir`.

        :param self
        :param buffer_capacity: The buffer capacity used to initialize the deserializer buffer.
        """
        current_dir: Path = Path(__file__).resolve().parent
        test_data_dir: Path = (
            current_dir / TestCaseDeserializerBuffer.deserializer_buffer_test_data_dir
        )
        for file_path in test_data_dir.rglob("*"):
            if not file_path.is_file():
                continue
            streaming_result: bytearray
            deserializer_buffer: DeserializerBuffer
            random_seed: int
            # Run against 10 different seeds:
            for _ in range(10):
                random_seed = random.randint(1, 3190)
                with open(str(file_path), "rb") as istream:
                    try:
                        if None is buffer_capacity:
                            deserializer_buffer = DeserializerBuffer(istream)
                        else:
                            deserializer_buffer = DeserializerBuffer(
                                initial_buffer_capacity=buffer_capacity, input_stream=istream
                            )
                        streaming_result = deserializer_buffer._test_streaming(random_seed)
                    except Exception as e:
                        self.assertFalse(
                            True, f"Error on file {file_path} using seed {random_seed}: {e}"
                        )
                self.__assert_streaming_result(file_path, streaming_result, random_seed)

    def __assert_streaming_result(
        self, file_path: Path, streaming_result: bytearray, random_seed: int
    ) -> None:
        """
        Validates the streaming result read by the deserializer buffer.

        :param file_path: Input stream file Path.
        :param streaming_result: Result of DeserializerBuffer `_test_streaming` method.
        """
        with open(str(file_path), "rb") as istream:
            ref_result: bytearray = bytearray(istream.read())
            self.assertEqual(
                ref_result,
                streaming_result,
                f"Streaming result is different from the src: {file_path}. Random seed:"
                f" {random_seed}.",
            )
