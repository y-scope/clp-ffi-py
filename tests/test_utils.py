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
            "Dict": {"Int": 1, "Dict": {"String": "This is a string"}},
        }
        serialized_dict: bytes = clp_ffi_py.utils.serialize_dict_to_msgpack(expected)
        actual: Dict[Any, Any] = msgpack.unpackb(serialized_dict)
        self.assertEqual(expected, actual)

        # TODO: add more testing
