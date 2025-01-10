#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include "PyQuery.hpp"

#include <new>
#include <string_view>
#include <type_traits>
#include <vector>

#include <clp/ir/types.hpp>
#include <clp/string_utils/string_utils.hpp>

#include <clp_ffi_py/api_decoration.hpp>
#include <clp_ffi_py/error_messages.hpp>
#include <clp_ffi_py/ExceptionFFI.hpp>
#include <clp_ffi_py/ir/native/PyLogEvent.hpp>
#include <clp_ffi_py/ir/native/Query.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
namespace {
/**
 * Constant keys used to serialize/deserialize PyQuery objects through `__getstate__` and
 * `__setstate__` methods.
 */
constexpr std::string_view cStateSearchTimeLowerBound{"search_time_lower_bound"};
constexpr std::string_view cStateSearchTimeUpperBound{"search_time_upper_bound"};
constexpr std::string_view cStateWildcardQueries{"wildcard_queries"};
constexpr std::string_view cStateSearchTimeTerminationMargin{"search_time_termination_margin"};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyQueryDoc,
        "This class represents a search query, utilized for filtering log events in a CLP IR "
        "stream. The query could include a list of wildcard queries aimed at identifying certain "
        "log messages, and a timestamp range with a lower and upper bound. This class provides an "
        "interface to set up a search query, as well as methods to validate whether the query can "
        "be matched by a log event. Note that an empty wildcard query list will match any log "
        "within the range.\n\n"
        "By default, the wildcard query list is empty and the timestamp range is set to include "
        "all the valid Unix epoch timestamps. To filter certain log messages, use customized "
        "wildcard queries to initialize the wildcard query list. For more details, check the "
        "documentation of the class `WildcardQuery`.\n\n"
        "NOTE: When searching an IR stream with a query, ideally, the search would terminate once "
        "the current log event's timestamp exceeds the upper bound of the query's time range. "
        "However, the timestamps in the IR stream might not be monotonically increasing; they can "
        "be locally disordered due to thread contention. To safely stop searching, the "
        "deserializer needs to ensure that the current timestamp in the IR stream exceeds the "
        "query's upper bound timestamp by a reasonable margin. This margin can be specified during "
        "the initialization. This margin is set to a default value specified by the static method "
        "`default_search_time_termination_margin()`. Users can customized this margin accordingly, "
        "for example, the margin can be set to 0 if the CLP IR stream is generated from a "
        "single-threaded program execution.\n\n"
        "The signature of `__init__` method is shown as following:\n\n"
        "__init__(self, search_time_lower_bound=Query.default_search_time_lower_bound(), "
        "search_time_upper_bound=Query.default_search_time_upper_bound(), "
        "wildcard_queries=None,search_time_termination_margin=Query.default_search_time_"
        "termination_margin())\n\n"
        "Initializes a Query object using the given inputs.\n\n"
        ":param search_time_lower_bound: Start of search time range (inclusive).\n"
        ":param search_time_upper_bound: End of search time range (inclusive).\n"
        ":param wildcard_queries: A list of wildcard queries.\n"
        ":param search_time_termination_margin: The margin used to determine the search "
        "termination timestamp.\n"
);
CLP_FFI_PY_METHOD auto PyQuery_init(PyQuery* self, PyObject* args, PyObject* keywords) -> int;

/**
 * Callback of `PyQuery` deallocator.
 * @param self
 */
CLP_FFI_PY_METHOD auto PyQuery_dealloc(PyQuery* self) -> void;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyQueryGetStateDoc,
        "__getstate__(self)\n"
        "--\n\n"
        "Serializes the Query object (should be called by the Python pickle module).\n\n"
        ":return: Serialized query in a Python dictionary.\n"
);
CLP_FFI_PY_METHOD auto PyQuery_getstate(PyQuery* self) -> PyObject*;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyQuerySetStateDoc,
        "__setstate__(self, state)\n"
        "--\n\n"
        "Deserializes the query from a state dictionary.\n"
        "Note: this function is exclusively designed for invocation by the Python pickle module. "
        "Assumes `self` is uninitialized and will allocate the underlying memory."
        "If `self` is already initialized this will result in memory leaks.\n\n"
        ":param state: Serialized query represented by a Python dictionary. It is anticipated "
        "to be the valid output of the `__getstate__` method.\n"
        ":return: None\n"
);
CLP_FFI_PY_METHOD auto PyQuery_setstate(PyQuery* self, PyObject* state) -> PyObject*;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyQueryMatchLogEventDoc,
        "match_log_event(self, log_event)\n"
        "--\n\n"
        "Validates whether the input log message matches the query.\n\n"
        ":param log_event: Input log event.\n"
        ":return:\n"
        "   - True if the timestamp is in range, and the wildcard query list is empty or has at "
        "     least one match.\n"
        "   - False otherwise.\n"
);
CLP_FFI_PY_METHOD auto PyQuery_match_log_event(PyQuery* self, PyObject* log_event) -> PyObject*;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyQueryGetSearchTimeLowerBoundDoc,
        "get_search_time_lower_bound(self)\n"
        "--\n\n"
        ":return: The search time lower bound.\n"
);
CLP_FFI_PY_METHOD auto PyQuery_get_search_time_lower_bound(PyQuery* self) -> PyObject*;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyQueryGetSearchTimeUpperBoundDoc,
        "get_search_time_upper_bound(self)\n"
        "--\n\n"
        ":return: The search time upper bound.\n"
);
CLP_FFI_PY_METHOD auto PyQuery_get_search_time_upper_bound(PyQuery* self) -> PyObject*;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyQueryGetWildcardQueriesDoc,
        "get_wildcard_queries(self)\n"
        "--\n\n"
        ":return: A new Python list of stored wildcard queries, presented as Wildcard Query "
        "objects.\n"
        ":return: None if the wildcard queries are empty.\n"
);
CLP_FFI_PY_METHOD auto PyQuery_get_wildcard_queries(PyQuery* self) -> PyObject*;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyQueryGetSearchTimeTerminationMarginDoc,
        "get_search_time_termination_margin(self)\n"
        "--\n\n"
        ":return: The search time termination margin.\n"
);
CLP_FFI_PY_METHOD auto PyQuery_get_search_time_termination_margin(PyQuery* self) -> PyObject*;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyQueryDefaultSearchTimeLowerBoundDoc,
        "default_search_time_lower_bound()\n"
        "--\n\n"
        ":return: The minimum valid timestamp from Unix epoch time.\n"
);
CLP_FFI_PY_METHOD auto PyQuery_default_search_time_lower_bound(PyObject* Py_UNUSED(self))
        -> PyObject*;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyQueryDefaultSearchTimeUpperBoundDoc,
        "default_search_time_upper_bound()\n"
        "--\n\n"
        ":return: The maximum valid timestamp from Unix epoch time.\n"
);
CLP_FFI_PY_METHOD auto PyQuery_default_search_time_upper_bound(PyObject* Py_UNUSED(self))
        -> PyObject*;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyQueryDefaultSearchTimeTerminationMargin,
        "default_search_time_termination_margin()\n"
        "--\n\n"
        ":return: The default search termination margin as Unix epoch time.\n"
);
CLP_FFI_PY_METHOD auto PyQuery_default_search_time_termination_margin(PyObject* Py_UNUSED(self))
        -> PyObject*;

/**
 * Callback of `PyQuery`'s `__str__` method.
 * @param self
 * @return Python string representation of the serialized `PyQuery` object.
 */
CLP_FFI_PY_METHOD auto PyQuery_str(PyQuery* self) -> PyObject*;

/**
 * Callback of `PyQuery`'s `__repr__` method.
 * @param self
 * @return __repr__ of the serialized `PyQuery` object.
 */
CLP_FFI_PY_METHOD auto PyQuery_repr(PyQuery* self) -> PyObject*;

// NOLINTNEXTLINE(*-avoid-c-arrays, cppcoreguidelines-avoid-non-const-global-variables)
PyMethodDef PyQuery_method_table[]{
        {"match_log_event",
         py_c_function_cast(PyQuery_match_log_event),
         METH_O,
         static_cast<char const*>(cPyQueryMatchLogEventDoc)},

        {"__getstate__",
         py_c_function_cast(PyQuery_getstate),
         METH_NOARGS,
         static_cast<char const*>(cPyQueryGetStateDoc)},

        {"__setstate__",
         py_c_function_cast(PyQuery_setstate),
         METH_O,
         static_cast<char const*>(cPyQuerySetStateDoc)},

        {"get_search_time_lower_bound",
         py_c_function_cast(PyQuery_get_search_time_lower_bound),
         METH_NOARGS,
         static_cast<char const*>(cPyQueryGetSearchTimeLowerBoundDoc)},

        {"get_search_time_upper_bound",
         py_c_function_cast(PyQuery_get_search_time_upper_bound),
         METH_NOARGS,
         static_cast<char const*>(cPyQueryGetSearchTimeUpperBoundDoc)},

        {"get_wildcard_queries",
         py_c_function_cast(PyQuery_get_wildcard_queries),
         METH_NOARGS,
         static_cast<char const*>(cPyQueryGetWildcardQueriesDoc)},

        {"get_search_time_termination_margin",
         py_c_function_cast(PyQuery_get_search_time_termination_margin),
         METH_NOARGS,
         static_cast<char const*>(cPyQueryGetSearchTimeTerminationMarginDoc)},

        {"default_search_time_lower_bound",
         py_c_function_cast(PyQuery_default_search_time_lower_bound),
         METH_NOARGS | METH_STATIC,
         static_cast<char const*>(cPyQueryDefaultSearchTimeLowerBoundDoc)},

        {"default_search_time_upper_bound",
         py_c_function_cast(PyQuery_default_search_time_upper_bound),
         METH_NOARGS | METH_STATIC,
         static_cast<char const*>(cPyQueryDefaultSearchTimeUpperBoundDoc)},

        {"default_search_time_termination_margin",
         py_c_function_cast(PyQuery_default_search_time_termination_margin),
         METH_NOARGS | METH_STATIC,
         static_cast<char const*>(cPyQueryDefaultSearchTimeTerminationMargin)},

        {nullptr}
};

// NOLINTBEGIN(cppcoreguidelines-pro-type-*-cast)
// NOLINTNEXTLINE(*-avoid-c-arrays, cppcoreguidelines-avoid-non-const-global-variables)
PyType_Slot PyQuery_slots[]{
        {Py_tp_alloc, reinterpret_cast<void*>(PyType_GenericAlloc)},
        {Py_tp_dealloc, reinterpret_cast<void*>(PyQuery_dealloc)},
        {Py_tp_new, reinterpret_cast<void*>(PyType_GenericNew)},
        {Py_tp_init, reinterpret_cast<void*>(PyQuery_init)},
        {Py_tp_str, reinterpret_cast<void*>(PyQuery_str)},
        {Py_tp_repr, reinterpret_cast<void*>(PyQuery_repr)},
        {Py_tp_methods, static_cast<void*>(PyQuery_method_table)},
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPyQueryDoc))},
        {0, nullptr}
};
// NOLINTEND(cppcoreguidelines-pro-type-*-cast)

/**
 * `PyQuery` Python type specifications.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
PyType_Spec PyQuery_type_spec{
        "clp_ffi_py.ir.native.Query",
        sizeof(Query),
        0,
        Py_TPFLAGS_DEFAULT,
        static_cast<PyType_Slot*>(PyQuery_slots)
};

/**
 * Deserializes the wildcard queries from a list of Python wildcard queries into a WildcardQuery
 * std::vector. If there is no wildcard queries given, the given py_wildcard_queries will be set to
 * Py_None.
 * @param py_wildcard_queries A Python list that contains Python wildcard queries. Each element
 * inside this list should be an instance of WildcardQuery defined in clp_ffi_py Python module.
 * @param wildcard_queries The output std::vector that contains cleaned wildcard queries from the
 * input. If py_wildcard_queries is Py_None, wildcard_queries will be set to empty.
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
auto deserialize_wildcard_queries(
        PyObject* py_wildcard_queries,
        std::vector<WildcardQuery>& wildcard_queries
) -> bool;

/**
 * Serializes the std::vector of WildcardQuery into a Python list. Serves as a helper function to
 * serialize the underlying wildcard queries of the PyQuery object.
 * @param wildcard_queries A std::vector of WildcardQuery.
 * @return The Python list that consists of Python wildcard query objects.
 * @return Py_None if the wildcard_queries are empty.
 */
auto serialize_wildcard_queries(std::vector<WildcardQuery> const& wildcard_queries) -> PyObject*;

CLP_FFI_PY_METHOD auto PyQuery_init(PyQuery* self, PyObject* args, PyObject* keywords) -> int {
    static char keyword_search_time_lower_bound[]{"search_time_lower_bound"};
    static char keyword_search_time_upper_bound[]{"search_time_upper_bound"};
    static char keyword_wildcard_queries[]{"wildcard_queries"};
    static char keyword_search_time_termination_margin[]{"search_time_termination_margin"};
    static char* keyword_table[]{
            static_cast<char*>(keyword_search_time_lower_bound),
            static_cast<char*>(keyword_search_time_upper_bound),
            static_cast<char*>(keyword_wildcard_queries),
            static_cast<char*>(keyword_search_time_termination_margin),
            nullptr
    };

    // If the argument parsing fails, `self` will be deallocated. We must reset all pointers to
    // nullptr in advance, otherwise the deallocator might trigger a segmentation fault.
    self->default_init();

    auto search_time_lower_bound{Query::cTimestampMin};
    auto search_time_upper_bound{Query::cTimestampMax};
    auto* py_wildcard_queries{Py_None};
    auto search_time_termination_margin{Query::cDefaultSearchTimeTerminationMargin};

    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "|LLOL",
                static_cast<char**>(keyword_table),
                &search_time_lower_bound,
                &search_time_upper_bound,
                &py_wildcard_queries,
                &search_time_termination_margin
        )))
    {
        return -1;
    }

    std::vector<WildcardQuery> wildcard_queries;
    if (false == deserialize_wildcard_queries(py_wildcard_queries, wildcard_queries)) {
        return -1;
    }

    if (false
        == self->init(
                search_time_lower_bound,
                search_time_upper_bound,
                wildcard_queries,
                search_time_termination_margin
        ))
    {
        return -1;
    }
    return 0;
}

CLP_FFI_PY_METHOD auto PyQuery_dealloc(PyQuery* self) -> void {
    self->clean();
    PyObject_Del(self);
}

CLP_FFI_PY_METHOD auto PyQuery_getstate(PyQuery* self) -> PyObject* {
    auto* query{self->get_query()};
    auto* py_wildcard_queries{serialize_wildcard_queries(query->get_wildcard_queries())};
    if (nullptr == py_wildcard_queries) {
        return nullptr;
    }
    return Py_BuildValue(
            "{sLsLsOsL}",
            get_c_str_from_constexpr_string_view(cStateSearchTimeLowerBound),
            query->get_lower_bound_ts(),
            get_c_str_from_constexpr_string_view(cStateSearchTimeUpperBound),
            query->get_upper_bound_ts(),
            get_c_str_from_constexpr_string_view(cStateWildcardQueries),
            py_wildcard_queries,
            get_c_str_from_constexpr_string_view(cStateSearchTimeTerminationMargin),
            query->get_search_time_termination_margin()
    );
}

CLP_FFI_PY_METHOD auto PyQuery_setstate(PyQuery* self, PyObject* state) -> PyObject* {
    self->default_init();

    if (false == static_cast<bool>(PyDict_CheckExact(state))) {
        PyErr_SetString(
                PyExc_ValueError,
                get_c_str_from_constexpr_string_view(clp_ffi_py::cSetstateInputError)
        );
        return nullptr;
    }

    auto* search_time_lower_bound_obj{PyDict_GetItemString(
            state,
            get_c_str_from_constexpr_string_view(cStateSearchTimeLowerBound)
    )};
    if (nullptr == search_time_lower_bound_obj) {
        PyErr_Format(
                PyExc_KeyError,
                get_c_str_from_constexpr_string_view(clp_ffi_py::cSetstateKeyErrorTemplate),
                cStateSearchTimeLowerBound
        );
        return nullptr;
    }
    clp::ir::epoch_time_ms_t search_time_lower_bound{0};
    if (false
        == parse_py_int<clp::ir::epoch_time_ms_t>(
                search_time_lower_bound_obj,
                search_time_lower_bound
        ))
    {
        return nullptr;
    }

    auto* search_time_upper_bound_obj{PyDict_GetItemString(
            state,
            get_c_str_from_constexpr_string_view(cStateSearchTimeUpperBound)
    )};
    if (nullptr == search_time_upper_bound_obj) {
        PyErr_Format(
                PyExc_KeyError,
                get_c_str_from_constexpr_string_view(clp_ffi_py::cSetstateKeyErrorTemplate),
                cStateSearchTimeUpperBound
        );
        return nullptr;
    }
    clp::ir::epoch_time_ms_t search_time_upper_bound{0};
    if (false
        == parse_py_int<clp::ir::epoch_time_ms_t>(
                search_time_upper_bound_obj,
                search_time_upper_bound
        ))
    {
        return nullptr;
    }

    auto* py_wildcard_queries{
            PyDict_GetItemString(state, get_c_str_from_constexpr_string_view(cStateWildcardQueries))
    };
    if (nullptr == py_wildcard_queries) {
        PyErr_Format(
                PyExc_KeyError,
                get_c_str_from_constexpr_string_view(clp_ffi_py::cSetstateKeyErrorTemplate),
                cStateWildcardQueries
        );
        return nullptr;
    }
    std::vector<WildcardQuery> wildcard_queries;
    if (false == deserialize_wildcard_queries(py_wildcard_queries, wildcard_queries)) {
        return nullptr;
    }

    auto* search_time_termination_margin_obj{PyDict_GetItemString(
            state,
            get_c_str_from_constexpr_string_view(cStateSearchTimeTerminationMargin)
    )};
    if (nullptr == search_time_termination_margin_obj) {
        PyErr_Format(
                PyExc_KeyError,
                get_c_str_from_constexpr_string_view(clp_ffi_py::cSetstateKeyErrorTemplate),
                cStateSearchTimeTerminationMargin
        );
        return nullptr;
    }
    clp::ir::epoch_time_ms_t search_time_termination_margin{0};
    if (false
        == parse_py_int<clp::ir::epoch_time_ms_t>(
                search_time_termination_margin_obj,
                search_time_termination_margin
        ))
    {
        return nullptr;
    }

    if (false
        == self->init(
                search_time_lower_bound,
                search_time_upper_bound,
                wildcard_queries,
                search_time_termination_margin
        ))
    {
        return nullptr;
    }

    Py_RETURN_NONE;
}

CLP_FFI_PY_METHOD auto PyQuery_match_log_event(PyQuery* self, PyObject* log_event) -> PyObject* {
    if (false == static_cast<bool>(PyObject_TypeCheck(log_event, PyLogEvent::get_py_type()))) {
        PyErr_SetString(PyExc_TypeError, get_c_str_from_constexpr_string_view(cPyTypeError));
        return nullptr;
    }
    auto* py_log_event{py_reinterpret_cast<PyLogEvent>(log_event)};
    return get_py_bool(self->get_query()->matches(*py_log_event->get_log_event()));
}

CLP_FFI_PY_METHOD auto PyQuery_get_search_time_lower_bound(PyQuery* self) -> PyObject* {
    return PyLong_FromLongLong(self->get_query()->get_lower_bound_ts());
}

CLP_FFI_PY_METHOD auto PyQuery_get_search_time_upper_bound(PyQuery* self) -> PyObject* {
    return PyLong_FromLongLong(self->get_query()->get_upper_bound_ts());
}

CLP_FFI_PY_METHOD auto PyQuery_get_wildcard_queries(PyQuery* self) -> PyObject* {
    return serialize_wildcard_queries(self->get_query()->get_wildcard_queries());
}

CLP_FFI_PY_METHOD auto PyQuery_get_search_time_termination_margin(PyQuery* self) -> PyObject* {
    return PyLong_FromLongLong(self->get_query()->get_search_time_termination_margin());
}

CLP_FFI_PY_METHOD auto PyQuery_default_search_time_lower_bound(PyObject* Py_UNUSED(self))
        -> PyObject* {
    return PyLong_FromLongLong(Query::cTimestampMin);
}

CLP_FFI_PY_METHOD auto PyQuery_default_search_time_upper_bound(PyObject* Py_UNUSED(self))
        -> PyObject* {
    return PyLong_FromLongLong(Query::cTimestampMax);
}

CLP_FFI_PY_METHOD auto PyQuery_default_search_time_termination_margin(PyObject* Py_UNUSED(self))
        -> PyObject* {
    return PyLong_FromLongLong(Query::cDefaultSearchTimeTerminationMargin);
}

CLP_FFI_PY_METHOD auto PyQuery_str(PyQuery* self) -> PyObject* {
    return PyObject_Str(PyQuery_getstate(self));
}

CLP_FFI_PY_METHOD auto PyQuery_repr(PyQuery* self) -> PyObject* {
    return PyObject_Repr(PyQuery_getstate(self));
}

auto deserialize_wildcard_queries(
        PyObject* py_wildcard_queries,
        std::vector<WildcardQuery>& wildcard_queries
) -> bool {
    wildcard_queries.clear();
    if (Py_None == py_wildcard_queries) {
        return true;
    }

    if (false == static_cast<bool>(PyObject_TypeCheck(py_wildcard_queries, &PyList_Type))) {
        PyErr_SetString(
                PyExc_TypeError,
                get_c_str_from_constexpr_string_view(clp_ffi_py::cPyTypeError)
        );
        return false;
    }

    auto const wildcard_queries_size{PyList_Size(py_wildcard_queries)};
    wildcard_queries.reserve(wildcard_queries_size);
    for (Py_ssize_t idx{0}; idx < wildcard_queries_size; ++idx) {
        auto* wildcard_query{PyList_GetItem(py_wildcard_queries, idx)};
        if (1 != PyObject_IsInstance(wildcard_query, PyQuery::get_py_wildcard_query_type())) {
            PyErr_SetString(
                    PyExc_TypeError,
                    get_c_str_from_constexpr_string_view(clp_ffi_py::cPyTypeError)
            );
            return false;
        }
        auto* wildcard_query_py_str{PyObject_GetAttrString(wildcard_query, "wildcard_query")};
        if (nullptr == wildcard_query_py_str) {
            return false;
        }
        auto* case_sensitive_py_bool{PyObject_GetAttrString(wildcard_query, "case_sensitive")};
        if (nullptr == case_sensitive_py_bool) {
            return false;
        }
        std::string_view wildcard_query_view;
        if (false == parse_py_string_as_string_view(wildcard_query_py_str, wildcard_query_view)) {
            return false;
        }
        int const is_case_sensitive{PyObject_IsTrue(case_sensitive_py_bool)};
        if (-1 == is_case_sensitive && nullptr != PyErr_Occurred()) {
            return false;
        }
        wildcard_queries.emplace_back(
                clp::string_utils::clean_up_wildcard_search_string(wildcard_query_view),
                static_cast<bool>(is_case_sensitive)
        );
    }
    return true;
}

auto serialize_wildcard_queries(std::vector<WildcardQuery> const& wildcard_queries) -> PyObject* {
    Py_ssize_t const wildcard_queries_size{static_cast<Py_ssize_t>(wildcard_queries.size())};
    if (0 == wildcard_queries_size) {
        Py_RETURN_NONE;
    }

    auto* py_wildcard_queries{PyList_New(wildcard_queries_size)};
    if (nullptr == py_wildcard_queries) {
        return nullptr;
    }

    // In case of failure, we only need to decrement the reference for the Python list. CPython will
    // decrement the reference count of all the objects it contains.
    Py_ssize_t idx{0};
    for (auto const& wildcard_query : wildcard_queries) {
        PyObjectPtr<PyObject> const wildcard_py_str_ptr{
                PyUnicode_FromString(wildcard_query.get_wildcard_query().c_str())
        };
        auto* wildcard_py_str{wildcard_py_str_ptr.get()};
        if (nullptr == wildcard_py_str) {
            Py_DECREF(py_wildcard_queries);
            return nullptr;
        }
        PyObjectPtr<PyObject> const is_case_sensitive{get_py_bool(wildcard_query.is_case_sensitive()
        )};
        PyObject* py_wildcard_query{PyObject_CallFunction(
                PyQuery::get_py_full_string_wildcard_query_type(),
                "OO",
                wildcard_py_str,
                is_case_sensitive.get()
        )};
        if (nullptr == py_wildcard_query) {
            Py_DECREF(py_wildcard_queries);
            return nullptr;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        PyList_SET_ITEM(py_wildcard_queries, idx, py_wildcard_query);
        ++idx;
    }
    return py_wildcard_queries;
}
}  // namespace

auto PyQuery::get_py_type() -> PyTypeObject* {
    return m_py_type.get();
}

auto PyQuery::get_py_wildcard_query_type() -> PyObject* {
    return m_py_wildcard_query_type.get();
}

auto PyQuery::get_py_full_string_wildcard_query_type() -> PyObject* {
    return m_py_full_string_wildcard_query_type.get();
}

auto PyQuery::module_level_init(PyObject* py_module) -> bool {
    static_assert(std::is_trivially_destructible<PyQuery>());
    auto* type{py_reinterpret_cast<PyTypeObject>(PyType_FromSpec(&PyQuery_type_spec))};
    m_py_type.reset(type);
    if (nullptr == type) {
        return false;
    }
    if (false == add_python_type(get_py_type(), "Query", py_module)) {
        return false;
    }

    PyObjectPtr<PyObject> const query_module(PyImport_ImportModule("clp_ffi_py.wildcard_query"));
    auto* py_query{query_module.get()};
    if (nullptr == py_query) {
        return false;
    }
    auto* py_wildcard_query_type{PyObject_GetAttrString(py_query, "WildcardQuery")};
    if (nullptr == py_wildcard_query_type) {
        return false;
    }
    m_py_wildcard_query_type.reset(py_wildcard_query_type);
    auto* py_full_string_wildcard_query_type{
            PyObject_GetAttrString(py_query, "FullStringWildcardQuery")
    };
    if (nullptr == py_full_string_wildcard_query_type) {
        return false;
    }
    m_py_full_string_wildcard_query_type.reset(py_full_string_wildcard_query_type);
    return true;
}

auto PyQuery::init(
        clp::ir::epoch_time_ms_t search_time_lower_bound,
        clp::ir::epoch_time_ms_t search_time_upper_bound,
        std::vector<WildcardQuery> const& wildcard_queries,
        clp::ir::epoch_time_ms_t search_time_termination_margin
) -> bool {
    try {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        m_query = new (std::nothrow)
                Query(search_time_lower_bound,
                      search_time_upper_bound,
                      wildcard_queries,
                      search_time_termination_margin);
        if (nullptr == m_query) {
            PyErr_SetString(
                    PyExc_RuntimeError,
                    get_c_str_from_constexpr_string_view(clp_ffi_py::cOutOfMemoryError)
            );
            return false;
        }
    } catch (clp_ffi_py::ExceptionFFI& ex) {
        handle_traceable_exception(ex);
        m_query = nullptr;
        return false;
    }
    return true;
}
}  // namespace clp_ffi_py::ir::native
