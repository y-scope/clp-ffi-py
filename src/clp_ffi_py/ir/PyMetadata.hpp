#ifndef CLP_FFI_PY_PY_METADATA_HPP
#define CLP_FFI_PY_PY_METADATA_HPP

#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include <clp/components/core/submodules/json/single_include/nlohmann/json.hpp>

#include <clp_ffi_py/ir/Metadata.hpp>

namespace clp_ffi_py::ir {
/**
 * A PyObject structure functioning as a Python-compatible interface to retrieve
 * CLP IR metadata. The underlying data is pointed by `metadata`. Additionally,
 * it retains a tzinfo object at the Python level that signifies the
 * corresponding timezone.
 */
class PyMetadata {
public:
    PyObject_HEAD;
    Metadata* metadata;
    PyObject* py_timezone;

    /**
     * Initializes the underlying data with the given inputs.
     * Since the memory allocation of PyMetadata is handled by CPython
     * allocator, cpp constructors will not be explicitly called. This function
     * serves as the default constructor to initialize the underlying metadata.
     * It has to be manually called whenever creating a new PyMetadata object
     * through CPython APIs.
     * @param ref_timestamp
     * @param input_timestamp_format
     * @param input_timezone
     * @return true on success.
     * @return false on failure with the relevant Python exception and error
     * set.
     */
    [[nodiscard]] auto init(
            ffi::epoch_time_ms_t ref_timestamp,
            char const* input_timestamp_format,
            char const* input_timezone
    ) -> bool;

    /**
     * Same as above, but takes inputs stored in the JSON format instead.
     * @param metadata
     * @param is_four_byte_encoding
     * @return true on success.
     * @return false on failure with the relevant Python exception and error
     * set.
     */
    [[nodiscard]] auto init(nlohmann::json const& metadata, bool is_four_byte_encoding = true)
            -> bool;

    /**
     * Resets pointers to nullptr.
     */
    auto reset() -> void {
        metadata = nullptr;
        py_timezone = nullptr;
    }

private:
    /**
     * Initializes py_timezone by setting the corresponded tzinfo object from
     * the timezone id. Should be called by `init` methods.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error
     * set.
     */
    [[nodiscard]] auto init_py_timezone() -> bool;
};

class PyWhatever {
public:
    PyObject_HEAD;

private:
    int cpp_layer_data_1;
    int cpp_layer_data_2;

public:
    int expose_to_python_1;
    int expose_to_python_2;
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
 * @return true on success.
 * @return false on failure with the relevant Python exception and error set.
 */
auto PyMetadata_module_level_init(PyObject* py_module) -> bool;

/**
 * Creates and initializes a new PyMetadata object with the metadata values
 * specified in the JSON format.
 * @param metadata CLP IR metadata stored in the JSON format.
 * @param is_four_byte_encoding Indicates whether the CLP IR uses 4-byte
 * encoding (true) or 8-byte encoding (false).
 * @return a new reference of a PyMetadata object that is initialized with the
 * given inputs.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
auto PyMetadata_new_from_json(nlohmann::json const& metadata, bool is_four_byte_encoding)
        -> PyMetadata*;
}  // namespace clp_ffi_py::ir
#endif
