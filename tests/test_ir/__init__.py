import unittest
from typing import Iterable, Optional, Union

from test_ir.test_decoder import *  # noqa
from test_ir.test_decoder_buffer import *  # noqa
from test_ir.test_deserializer import *  # noqa
from test_ir.test_deserializer_buffer import *  # noqa
from test_ir.test_encoder import *  # noqa
from test_ir.test_key_value_pair_log_event import *  # noqa
from test_ir.test_log_event import *  # noqa
from test_ir.test_metadata import *  # noqa
from test_ir.test_query import *  # noqa
from test_ir.test_query_builder import *  # noqa
from test_ir.test_readers import *  # noqa
from test_ir.test_serder import *  # noqa
from test_ir.test_serializer import *  # noqa
from test_ir.test_utils import TestCLPBase


def add_tests(suite: unittest.TestSuite, loader: unittest.TestLoader, test_class: type) -> None:
    """
    Recursively collect tests from concrete classes. Although Test*Base classes are functionally
    abstract to the user we cannot properly make them abstract as `unittest` will still create
    instances of these classes.

    :param suite: test suite to add found tests to
    :param loader: load test from `unittest.TestCase` class
    :param test_class: current class to search for tests in
    """
    if "Base" not in str(test_class):
        tests = loader.loadTestsFromTestCase(test_class)
        suite.addTests(tests)

    for subclass in test_class.__subclasses__():
        add_tests(suite, loader, subclass)


def load_tests(
    loader: unittest.TestLoader,
    tests: Iterable[Union[unittest.TestCase, unittest.TestSuite]],
    pattern: Optional[str],
) -> unittest.TestSuite:
    suite: unittest.TestSuite = unittest.TestSuite()

    for test_class in TestCLPBase.__subclasses__():
        add_tests(suite, loader, test_class)

    return suite
