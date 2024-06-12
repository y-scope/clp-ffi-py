import warnings

from deprecated.sphinx import deprecated


class WildcardQuery:
    """
    This class defines an abstract wildcard query. It includes a wildcard string
    and a boolean value to indicate if the match is case-sensitive.

    A wildcard string may contain the following types of supported wildcards:

    1. '*': match 0 or more characters.
    2. '?': match any single character.

    Each wildcard can be escaped using a preceding '\\\\' (a single backslash).
    Other characters which are escaped are treated as normal characters.
    """

    @deprecated(
        version="0.0.12",
        reason="`clp_ffi_py.wildcard_query.WildcardQuery` is supposed to be an abstract class and"
        " should not be used directly. To create a wildcard query, please explicit instantiate"
        " `clp_ffi_py.wildcard_query.SubstringWildcardQuery` or"
        " `clp_ffi_py.wildcard_query.FullStringWildcardQuery`.",
    )
    def __init__(self, wildcard_query: str, case_sensitive: bool = False):
        """
        Initializes a wildcard query using the given parameters.

        :param wildcard_query: Wildcard query string.
        :param case_sensitive: Case sensitive indicator.
        """
        self._wildcard_query: str = wildcard_query
        self._case_sensitive: bool = case_sensitive

    def __str__(self) -> str:
        """
        :return: The string representation of the WildcardQuery object.
        """
        return (
            f'{self.__class__.__name__}(wildcard_query="{self._wildcard_query}",'
            f" case_sensitive={self._case_sensitive})"
        )

    def __repr__(self) -> str:
        """
        :return: Same as `__str__` method.
        """
        return self.__str__()

    @property
    def wildcard_query(self) -> str:
        return self._wildcard_query

    @property
    def case_sensitive(self) -> bool:
        return self._case_sensitive


class SubstringWildcardQuery(WildcardQuery):
    """
    This class defines a substring wildcard query.

    It is derived from
    :class:`~clp_ffi_py.WildcardQuery`, adding both a prefix and a postfix
    wildcard ("*") to the input wildcard string. This allows the query to match
    any substring within a log message.
    """

    def __init__(self, substring_wildcard_query: str, case_sensitive: bool = False):
        """
        Initializes a substring wildcard query using the given parameters.

        :param substring_wildcard_query: Wildcard query string.
        :param case_sensitive: Case sensitive indicator.
        """
        substring_wildcard_query = "*" + substring_wildcard_query + "*"
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", DeprecationWarning)
            super().__init__(substring_wildcard_query, case_sensitive)


class FullStringWildcardQuery(WildcardQuery):
    """
    This class defines a full string wildcard query.

    It is derived from
    :class:`~clp_ffi_py.WildcardQuery`, and uses the input wildcard string
    directly to create the query. This ensures that the query matches only the
    entire log message.
    """

    def __init__(self, full_string_wildcard_query: str, case_sensitive: bool = False):
        """
        Initializes a full-string wildcard query using the given parameters.

        :param full_string_wildcard_query: Wildcard query string.
        :param case_sensitive: Case sensitive indicator.
        """
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", DeprecationWarning)
            super().__init__(full_string_wildcard_query, case_sensitive)
