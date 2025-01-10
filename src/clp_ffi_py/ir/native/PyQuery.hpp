#ifndef CLP_FFI_PY_IR_NATIVE_PYQUERY_HPP
#define CLP_FFI_PY_IR_NATIVE_PYQUERY_HPP

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include <vector>

#include <clp/ir/types.hpp>

#include <clp_ffi_py/ir/native/Query.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>

namespace clp_ffi_py::ir::native {
/**
 * A PyObject structure functioning as a Python-compatible interface to retrieve a search query that
 * is used to filter log events in a CLP IR stream. The underlying data is pointed to by `m_query`.
 * A detailed description can be found in the PyQuery Python doc strings.
 */
class PyQuery {
public:
    // Static methods
    /**
     * Gets the PyTypeObject that represents PyQuery's Python type. This type is dynamically created
     * and initialized during the execution of `PyQuery::module_level_init`.
     * @return Python type object associated with PyQuery.
     */
    [[nodiscard]] static auto get_py_type() -> PyTypeObject*;

    /**
     * Creates and initializes PyQuery as a Python type, and then incorporates this type as a Python
     * object into the py_module module.
     * @param py_module This is the Python module where the initialized PyQuery will be
     * incorporated.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto module_level_init(PyObject* py_module) -> bool;

    /**
     * @return PyObject that represents the Python level class `WildcardQuery`.
     */
    [[nodiscard]] static auto get_py_wildcard_query_type() -> PyObject*;

    /**
     * @return PyObject that represents the Python level class `FullStringWildcardQuery`.
     */
    [[nodiscard]] static auto get_py_full_string_wildcard_query_type() -> PyObject*;

    // Delete default constructor to disable direct instantiation.
    PyQuery() = delete;

    // Delete copy & move constructors and assignment operators
    PyQuery(PyQuery const&) = delete;
    PyQuery(PyQuery&&) = delete;
    auto operator=(PyQuery const&) -> PyQuery& = delete;
    auto operator=(PyQuery&&) -> PyQuery& = delete;

    // Destructor
    ~PyQuery() = default;

    // Methods
    /**
     * Initializes the underlying data with the given input. Since the memory allocation of PyQuery
     * is handled by CPython's allocator, any cpp constructor will not be explicitly called. This
     * function serves as the default constructor to initialize the underlying query. It has to be
     * manually called whenever creating a new PyQuery object through CPython AIPs.
     * @param search_time_lower_bound Start of search time range (inclusive).
     * @param search_time_upper_bound End of search time range (inclusive).
     * @param wildcard_queries A list of wildcard queries. Each wildcard query must be valid (see
     * `wildcard_match_unsafe`).
     * @param search_time_termination_margin The margin used to determine the search termination
     * timestamp (see note in the Query class' docstring).
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto init(
            clp::ir::epoch_time_ms_t search_time_lower_bound,
            clp::ir::epoch_time_ms_t search_time_upper_bound,
            std::vector<WildcardQuery> const& wildcard_queries,
            clp::ir::epoch_time_ms_t search_time_termination_margin
    ) -> bool;

    /**
     * Initializes the pointers to nullptr by default. Should be called once the object is
     * allocated.
     */
    auto default_init() -> void { m_query = nullptr; }

    /*
     * Releases the memory allocated for underlying query object.
     */
    auto clean() -> void { delete m_query; }

    [[nodiscard]] auto get_query() -> Query* { return m_query; }

private:
    static inline PyObjectStaticPtr<PyTypeObject> m_py_type{nullptr};
    static inline PyObjectStaticPtr<PyObject> m_py_wildcard_query_type{nullptr};
    static inline PyObjectStaticPtr<PyObject> m_py_full_string_wildcard_query_type{nullptr};

    PyObject_HEAD;
    Query* m_query;
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_PYQUERY_HPP
