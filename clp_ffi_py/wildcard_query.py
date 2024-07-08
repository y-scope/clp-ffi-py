import warnings

from deprecated.sphinx import deprecated


class WildcardQuery:
    """
    An abstract class defining a wildcard query. Users should instantiate a wildcard query through
    :class:`SubstringWildcardQuery` or :class:`FullStringWildcardQuery`.

    A wildcard string may contain the following types of wildcards:

    1. '*': match 0 or more characters.
    2. '?': match any single character.

    Each wildcard can be escaped using a preceding '\\\\' (a single backslash). Other characters
    that are escaped are treated as normal characters.
    """

    @deprecated(
        version="0.0.12",
        reason=":class:`WildcardQuery` will soon be made abstract and should"
        " not be used directly. To create a wildcard query, use"
        " :class:`SubstringWildcardQuery` or :class:`FullStringWildcardQuery`"
        " instead.",
    )
    def __init__(self, wildcard_query: str, case_sensitive: bool = False):
        """
        Initializes a wildcard query using the given parameters.

        :param wildcard_query: Wildcard query string.
        :param case_sensitive: Whether to perform case-sensitive matching.
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
    A wildcard query that can match a substring in a log event's message, in contrast with
    :class:`FullStringWildcardQuery` where the query needs to match the entire message.

    This class is derived from :class:`WildcardQuery` by adding both a prefix and a postfix wildcard
    ("*") to the input wildcard string.
    """

    def __init__(self, substring_wildcard_query: str, case_sensitive: bool = False):
        """
        Initializes a substring wildcard query using the given parameters.

        :param substring_wildcard_query: Wildcard query string.
        :param case_sensitive: Whether to perform case-sensitive matching.
        """
        substring_wildcard_query = "*" + substring_wildcard_query + "*"
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", DeprecationWarning)
            super().__init__(substring_wildcard_query, case_sensitive)


class FullStringWildcardQuery(WildcardQuery):
    """
    A wildcard query where the query must match the entirety of the log event's message, in contrast
    with :class:`SubstringWildcardQuery` where the query only needs to match a substring.

    This class is derived from :class:`WildcardQuery` as a more explicit interface for full-string
    matches.

    Users can create a match that's anchored to only one end of the message by adding a prefix OR
    postfix wildcard ("*").
    """

    def __init__(self, full_string_wildcard_query: str, case_sensitive: bool = False):
        """
        Initializes a full-string wildcard query using the given parameters.

        :param full_string_wildcard_query: Wildcard query string.
        :param case_sensitive: Whether to perform case-sensitive matching.
        """
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", DeprecationWarning)
            super().__init__(full_string_wildcard_query, case_sensitive)
