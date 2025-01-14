#ifndef CLP_FFI_PY_IR_NATIVE_PYMETADATA_HPP
#define CLP_FFI_PY_IR_NATIVE_PYMETADATA_HPP

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include <clp/ir/types.hpp>
#include <json/single_include/nlohmann/json.hpp>

#include <clp_ffi_py/ir/native/Metadata.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>

namespace clp_ffi_py::ir::native {
/**
 * A PyObject structure functioning as a Python-compatible interface to retrieve CLP IR metadata.
 * The underlying data is pointed to by `m_metadata`. Additionally, it retains a tzinfo object at
 * the Python level that signifies the corresponding timezone.
 */
class PyMetadata {
public:
    // Static methods
    /**
     * Gets the PyTypeObject that represents PyMetadata's Python type. This type is dynamically
     * created and initialized during the execution of `PyMetadata::module_level_init`.
     * @return Python type object associated with PyMetadata.
     */
    [[nodiscard]] static auto get_py_type() -> PyTypeObject*;

    /**
     * Creates and initializes PyMetadata as a Python type, and then incorporates this type as a
     * Python object into the py_module module.
     * @param py_module This is the Python module where the initialized PyMetadata will be
     * incorporated.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto module_level_init(PyObject* py_module) -> bool;

    /**
     * Creates and initializes a new PyMetadata object with the metadata values specified in the
     * JSON format.
     * @param metadata CLP IR metadata stored in the JSON format.
     * @param is_four_byte_encoding Indicates whether the CLP IR uses 4-byte encoding (true) or
     * 8-byte encoding (false).
     * @return a new reference of a PyMetadata object that is initialized with the given inputs.
     * @return nullptr on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto
    create_new_from_json(nlohmann::json const& metadata, bool is_four_byte_encoding) -> PyMetadata*;

    // Delete default constructor to disable direct instantiation.
    PyMetadata() = delete;

    // Delete copy & move constructors and assignment operators
    PyMetadata(PyMetadata const&) = delete;
    PyMetadata(PyMetadata&&) = delete;
    auto operator=(PyMetadata const&) -> PyMetadata& = delete;
    auto operator=(PyMetadata&&) -> PyMetadata& = delete;

    // Destructor
    ~PyMetadata() = default;

    // Methods
    /**
     * Initializes the underlying data with the given inputs. Since the memory allocation of
     * PyMetadata is handled by CPython allocator, cpp constructors will not be explicitly called.
     * This function serves as the default constructor to initialize the underlying metadata.
     * It has to be manually called whenever creating a new PyMetadata object through CPython APIs.
     * @param ref_timestamp
     * @param input_timestamp_format
     * @param input_timezone
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto init(
            clp::ir::epoch_time_ms_t ref_timestamp,
            char const* input_timestamp_format,
            char const* input_timezone
    ) -> bool;

    /**
     * Same as above, but takes inputs stored in the JSON format instead.
     * @param metadata
     * @param is_four_byte_encoding
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto init(nlohmann::json const& metadata, bool is_four_byte_encoding = true)
            -> bool;

    /**
     * Releases the memory allocated for underlying metadata field and the reference hold for the
     * Python object(s).
     */
    auto clean() -> void {
        delete m_metadata;
        Py_XDECREF(m_py_timezone);
    }

    /**
     * Initializes the pointers to nullptr by default. Should be called once the object is
     * allocated.
     */
    auto default_init() -> void {
        m_metadata = nullptr;
        m_py_timezone = nullptr;
    }

    [[nodiscard]] auto get_metadata() -> Metadata* { return m_metadata; }

    [[nodiscard]] auto get_py_timezone() -> PyObject* { return m_py_timezone; }

private:
    static inline PyObjectStaticPtr<PyTypeObject> m_py_type{nullptr};

    /**
     * Initializes py_timezone by setting the corresponded tzinfo object from the timezone id.
     * Should be called by `init` methods.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto init_py_timezone() -> bool;

    PyObject_HEAD;
    Metadata* m_metadata;
    PyObject* m_py_timezone;
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_PYMETADATA_HPP
