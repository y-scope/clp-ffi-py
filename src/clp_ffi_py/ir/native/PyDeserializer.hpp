#ifndef CLP_FFI_PY_IR_NATIVE_PYDESERIALIZER_HPP
#define CLP_FFI_PY_IR_NATIVE_PYDESERIALIZER_HPP

#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include <functional>
#include <utility>

#include <clp/ffi/ir_stream/decoding_methods.hpp>
#include <clp/ffi/ir_stream/Deserializer.hpp>
#include <clp/ffi/KeyValuePairLogEvent.hpp>
#include <clp/ffi/SchemaTree.hpp>
#include <clp/time_types.hpp>
#include <gsl/gsl>
#include <json/single_include/nlohmann/json.hpp>

#include <clp_ffi_py/ir/native/DeserializerBufferReader.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>

namespace clp_ffi_py::ir::native {
/**
 * A PyObject structure for deserializing CLP key-value pair IR stream. The underlying deserializer
 * is pointed by `m_deserializer`, which reads the IR stream from a Python `IO[byte]` object via
 * `DeserializerBufferReader`.
 */
class PyDeserializer {
public:
    /**
     * The default buffer capacity for the underlying deserializer buffer reader. Any change to the
     * value should also be applied to `__init__`'s doc string and Python stub file.
     */
    static constexpr Py_ssize_t cDefaultBufferCapacity{65'536};

    /**
     * Gets the `PyTypeObject` that represents `PyDeserializer`'s Python type. This type is
     * dynamically created and initialized during the execution of
     * `PyDeserializer::module_level_init`.
     * @return Python type object associated with `PyDeserializer`.
     */
    [[nodiscard]] static auto get_py_type() -> PyTypeObject* { return m_py_type.get(); }

    /**
     * Creates and initializes `PyDeserializer` as a Python type, and then incorporates this
     * type as a Python object into the py_module module.
     * @param py_module This is the Python module where the initialized `PyDeserializer` will be
     * incorporated.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] static auto module_level_init(PyObject* py_module) -> bool;

    // Delete default constructor to disable direct instantiation.
    PyDeserializer() = delete;

    // Delete copy & move constructors and assignment operators
    PyDeserializer(PyDeserializer const&) = delete;
    PyDeserializer(PyDeserializer&&) = delete;
    auto operator=(PyDeserializer const&) -> PyDeserializer& = delete;
    auto operator=(PyDeserializer&&) -> PyDeserializer& = delete;

    // Destructor
    ~PyDeserializer() = default;

    // Methods
    /**
     * Since the memory allocation of `PyDeserializer` is handled by CPython's allocator, cpp
     * constructors will not be explicitly called. This function serves as the default constructor
     * to initialize the underlying deserializer and deserializer buffer reader. Other data members
     * are assumed to be zero-initialized by `default-init` method. It has to be manually called
     * whenever creating a new `PyDeserializer` object through CPython APIs.
     * @param input_stream The input IR stream. Must be a Python `IO[byte]` object.
     * @param buffer_capacity The buffer capacity used to initialize the underlying
     * `PyDeserializerBufferReader`.
     * @param allow_incomplete_stream Whether to treat an incomplete CLP IR stream as an error. When
     * set to `true`, an incomplete stream is interpreted as the end of the stream without raising
     * an exception.
     * @return true on success.
     * @return false on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto
    init(PyObject* input_stream, Py_ssize_t buffer_capacity, bool allow_incomplete_stream) -> bool;

    /**
     * Zero-initializes all the data members in `PyDeserializer`. Should be called once the
     * object is allocated.
     */
    auto default_init() -> void {
        m_end_of_stream_reached = false;
        m_allow_incomplete_stream = false;
        m_deserializer_buffer_reader = nullptr;
        m_deserializer = nullptr;
        m_deserialized_log_event = nullptr;
    }

    /**
     * Releases the memory allocated for the underlying data fields.
     */
    auto clean() -> void {
        delete m_deserializer;
        delete m_deserializer_buffer_reader;
        clear_deserialized_log_event();
    }

    /**
     * Deserializes the next key value pair log event from the IR stream.
     * @return A new reference to a `KeyValuePairLogEvent` object representing the deserialized log
     * event on success.
     * @return A new reference to `Py_None` when the end of IR stream is reached.
     * @return nullptr on failure with the relevant Python exception and error set.
     */
    [[nodiscard]] auto deserialize_log_event() -> PyObject*;

    /**
     * @return A pointer to the user-defined stream-level metadata, deserialized from the stream's
     * preamble, if defined.
     * @return std::nullptr if the user-defined stream-level metadata is not defined.
     */
    [[nodiscard]] auto get_user_defined_metadata() const -> nlohmann::json const*;

private:
    /**
     * Class that implements `clp::ffi::ir_stream::IrUnitHandlerInterface` for deserializing
     * key-value pair log events using user-defined handlers.
     */
    class IrUnitHandler {
    public:
        // Types
        using LogEventHandle
                = std::function<clp::ffi::ir_stream::IRErrorCode(clp::ffi::KeyValuePairLogEvent&&)>;
        using UtcOffsetChangeHandle
                = std::function<clp::ffi::ir_stream::IRErrorCode(clp::UtcOffset, clp::UtcOffset)>;
        using SchemaTreeNodeInsertionHandle = std::function<clp::ffi::ir_stream::IRErrorCode(
                bool is_auto_generated,
                clp::ffi::SchemaTree::NodeLocator
        )>;
        using EndOfStreamHandle = std::function<clp::ffi::ir_stream::IRErrorCode()>;

        // Constructor
        IrUnitHandler(
                LogEventHandle log_event_handle,
                UtcOffsetChangeHandle utc_offset_handle,
                SchemaTreeNodeInsertionHandle schema_tree_insertion_handle,
                EndOfStreamHandle end_of_stream_handle
        )
                : m_log_event_handle{std::move(log_event_handle)},
                  m_utc_offset_change_handle{std::move(utc_offset_handle)},
                  m_schema_tree_node_insertion_handle{std::move(schema_tree_insertion_handle)},
                  m_end_of_stream_handle{std::move(end_of_stream_handle)} {}

        // Delete copy constructor and assignment
        IrUnitHandler(IrUnitHandler const&) = delete;
        auto operator=(IrUnitHandler const&) -> IrUnitHandler& = delete;

        // Default move constructor and assignment
        IrUnitHandler(IrUnitHandler&&) = default;
        auto operator=(IrUnitHandler&&) -> IrUnitHandler& = default;

        // Destructor
        ~IrUnitHandler() = default;

        // Implements `clp::ffi::ir_stream::IrUnitHandlerInterface` interface
        [[nodiscard]] auto handle_log_event(clp::ffi::KeyValuePairLogEvent&& log_event)
                -> clp::ffi::ir_stream::IRErrorCode {
            return m_log_event_handle(std::move(log_event));
        }

        [[nodiscard]] auto
        handle_utc_offset_change(clp::UtcOffset utc_offset_old, clp::UtcOffset utc_offset_new)
                -> clp::ffi::ir_stream::IRErrorCode {
            return m_utc_offset_change_handle(utc_offset_old, utc_offset_new);
        }

        [[nodiscard]] auto handle_schema_tree_node_insertion(
                bool is_auto_generated,
                clp::ffi::SchemaTree::NodeLocator schema_tree_node_locator
        ) -> clp::ffi::ir_stream::IRErrorCode {
            return m_schema_tree_node_insertion_handle(is_auto_generated, schema_tree_node_locator);
        }

        [[nodiscard]] auto handle_end_of_stream() -> clp::ffi::ir_stream::IRErrorCode {
            return m_end_of_stream_handle();
        }

    private:
        // Variables
        LogEventHandle m_log_event_handle;
        UtcOffsetChangeHandle m_utc_offset_change_handle;
        SchemaTreeNodeInsertionHandle m_schema_tree_node_insertion_handle;
        EndOfStreamHandle m_end_of_stream_handle;
    };

    using Deserializer = clp::ffi::ir_stream::Deserializer<IrUnitHandler>;

    static inline PyObjectStaticPtr<PyTypeObject> m_py_type{nullptr};

    // Methods
    /**
     * Implements `IrUnitHandler::EndOfStreamHandle`.
     * This handle function sets the underlying `m_end_of_stream_reached` to true.
     * @return IRErrorCode::IRErrorCode_Success on success.
     */
    [[maybe_unused]] auto handle_end_of_stream() -> clp::ffi::ir_stream::IRErrorCode {
        m_end_of_stream_reached = true;
        return clp::ffi::ir_stream::IRErrorCode::IRErrorCode_Success;
    }

    /**
     * Implements `IrUnitHandler::LogEventHandle`.
     * This handle function sets the underlying `m_deserialized_log_event` with the given input.
     * @param kv_log_event
     * @return IRErrorCode::IRErrorCode_Success on success.
     *
     */
    [[nodiscard]] auto handle_log_event(clp::ffi::KeyValuePairLogEvent&& log_event)
            -> clp::ffi::ir_stream::IRErrorCode;

    /**
     * @return Whether `m_deserialized_log_event` has been set.
     */
    [[nodiscard]] auto has_unreleased_deserialized_log_event() const -> bool {
        return nullptr != m_deserialized_log_event;
    }

    /**
     * Releases the ownership of the underlying deserialized log event as a rvalue reference.
     * NOTE: this method doesn't check whether the ownership is empty (nullptr). The caller must
     * ensure the ownership is legal.
     * @return The released ownership of the deserialized log event.
     */
    [[nodiscard]] auto release_deserialized_log_event() -> clp::ffi::KeyValuePairLogEvent {
        auto released{std::move(*m_deserialized_log_event)};
        clear_deserialized_log_event();
        return released;
    }

    /**
     * Handles the incomplete stream error returned from `Deserializer::deserialize_next_ir_unit`.
     * @return true if incomplete stream is allowed.
     * @return false with the relevant Python exception and error set otherwise.
     */
    [[nodiscard]] auto handle_incomplete_stream_error() -> bool;

    [[nodiscard]] auto is_stream_completed() const -> bool { return m_end_of_stream_reached; }

    auto clear_deserialized_log_event() -> void {
        delete m_deserialized_log_event;
        m_deserialized_log_event = nullptr;
    }

    // Variables
    PyObject_HEAD;
    bool m_end_of_stream_reached;
    bool m_allow_incomplete_stream;
    // NOLINTBEGIN(cppcoreguidelines-owning-memory)
    gsl::owner<DeserializerBufferReader*> m_deserializer_buffer_reader;
    gsl::owner<Deserializer*> m_deserializer;
    gsl::owner<clp::ffi::KeyValuePairLogEvent*> m_deserialized_log_event;
    // NOLINTEND(cppcoreguidelines-owning-memory)
};
}  // namespace clp_ffi_py::ir::native

#endif  // CLP_FFI_PY_IR_NATIVE_PYDESERIALIZER_HPP
