class WildcardQuery:
    """
    This class defines a wildcard query, which includes a wildcard string and a
    boolean value to indicate if the match is case-sensitive.

    A wildcard string may contain the following types of supported wildcards:

    1. '*': match 0 or more characters.
    2. '?': match any single character.

    Each wildcard can be escaped using a preceding '\\\\' (a single backslash).
    Other characters which are escaped are treated as normal characters.
    """

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
            f'WildcardQuery(wildcard_query="{self._wildcard_query}",'
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
