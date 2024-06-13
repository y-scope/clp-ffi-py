clp\_ffi\_py.ir.query\_builder module
=====================================

.. automodule:: clp_ffi_py.ir.query_builder
    :members:
    :undoc-members:
    :show-inheritance:
    :exclude-members: QueryBuilder

    .. autoclass:: QueryBuilder
        :members:
        :undoc-members:
        :show-inheritance:
        :exclude-members: __init__, __new__, add_wildcard_query

        .. method:: add_wildcard_query(wildcard_query: WildcardQuery) -> QueryBuilder

            Adds the given wildcard query to the wildcard query list.

            :param WildcardQuery wildcard_query: The wildcard query object to add.
            :returns: self.
            :rtype: QueryBuilder

        .. method:: add_wildcard_query(wildcard_query: str, case_sensitive: bool = False) \
            -> QueryBuilder
            :no-index:

            Constructs and adds a :class:`~clp_ffi_py.wildcard_query.WildcardQuery` to the wildcard
            query list.

            .. deprecated:: 0.0.12
                Use :meth:`add_wildcard_query` with either a
                :class:`~clp_ffi_py.wildcard_query.FullStringWildcardQuery` or
                :class:`~clp_ffi_py.wildcard_query.SubstringWildcardQuery`
                instead.

            :param str wildcard_query: The wildcard query string to add.
            :param bool case_sensitive: Whether to perform case-sensitive matching.
            :returns: self.
            :rtype: QueryBuilder
