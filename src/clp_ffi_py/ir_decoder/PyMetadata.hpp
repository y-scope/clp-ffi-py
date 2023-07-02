#ifndef CLP_FFI_PY_PY_METADATA_HPP
#define CLP_FFI_PY_PY_METADATA_HPP

#include <clp_ffi_py/Python.hpp> // Must always be included before any other header files

#include <clp/components/core/submodules/json/single_include/nlohmann/json.hpp>
#include <clp_ffi_py/ir_decoder/Metadata.hpp>

namespace clp_ffi_py::ir_decoder {
/**
 * A PyObject structure functioning as a Python-compatible interface to retrieve
 * CLP IR metadata. The underlying data is pointed by `metadata`. Additionally,
 * it retains a tzinfo object at the Python level that signifies the
 * corresponding timezone.
 */
struct PyMetadata {
    PyObject_HEAD;
    Metadata* metadata;
    PyObject* Py_timezone;
};

/**
 * Gets the PyTypeObject that represents PyMetadata's Python type. This type is
 * dynamically created and initialized during the execution of
 * `PyMetadata_module_level_init`.
 * @return Python type object associated with PyMetadata.
 */
auto PyMetadata_get_PyType() -> PyTypeObject*;

/**
 * Creates and initializes PyMetadata as a Python type, and then incorporates
 * this type as a Python object into the py_module module.
 * @param py_module This is the Python module where the initialized PyMetadata
 * will be incorporated.
 * @param new_object_append_list This vector is responsible for appending all
 * successfully created PyObjects during initialization for reference tracking
 * purposes.
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
auto PyMetadata_module_level_init(
        PyObject* py_module,
        std::vector<PyObject*>& new_object_append_list) -> bool;

/**
 * Creates and initializes a new PyMetadata object with the metadata values
 * specified in the JSON format.
 * @param metadata CLP IR metadata stored in the JSON format.
 * @param is_four_byte_encoding Indicates whether the CLP IR uses four-byte
 * encoding (true) or eight-byte encoding (false).
 * @return a new reference of a PyMetadata object that is initialized with the
 * given inputs.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
auto PyMetadata_new_from_json(nlohmann::json const& metadata, bool is_four_byte_encoding)
        -> PyMetadata*;
} // namespace clp_ffi_py::ir_decoder
#endif
