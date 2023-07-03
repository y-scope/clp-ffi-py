from typing import Iterable, Optional, Union
from tests.TestCase import TestCaseBase
import unittest


def add_tests(suite: unittest.TestSuite, loader: unittest.TestLoader, test_class: type) -> None:
    """
    Recursively collect tests from concrete classes. Although TestCLPBase*
    classes are functionally abstract to the user we cannot properly make them
    abstract as `unittest` will still create instances of these classes.

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

    for test_class in TestCaseBase.__subclasses__():
        add_tests(suite, loader, test_class)

    return suite
