import unittest
from typing import Any, Dict

import msgpack

import clp_ffi_py.utils


class TestUtils(unittest.TestCase):
    """
    Class for testing `clp_ffi_py.utils`.
    """

    def test_serialize_dict_to_msgpack(self) -> None:
        expected: Dict[str, Any] = {
            "Int": 1,
            "Float": 1.3,
            "Bool": True,
            "String": "This is a string",
            "Array": [1, 1.3, True, "String"],
            "None": None,
            "Dict": {"Int": 1, "Dict": {"String": "This is a string"}},
        }
        actual: Dict[Any, Any] = msgpack.unpackb(
            clp_ffi_py.utils.serialize_dict_to_msgpack(expected)
        )
        self.assertEqual(expected, actual)

        with self.assertRaises(TypeError):
            clp_ffi_py.utils.serialize_dict_to_msgpack(1)  # type: ignore

        with self.assertRaises(TypeError):
            clp_ffi_py.utils.serialize_dict_to_msgpack(1.1)  # type: ignore

        with self.assertRaises(TypeError):
            clp_ffi_py.utils.serialize_dict_to_msgpack(True)  # type: ignore

        with self.assertRaises(TypeError):
            clp_ffi_py.utils.serialize_dict_to_msgpack(None)  # type: ignore
