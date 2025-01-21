#include <wrapped_facade_headers/Python.hpp>  // Must be included before any other header files

#include "deserialization_methods.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include <clp/BufferReader.hpp>
#include <clp/ffi/ir_stream/decoding_methods.hpp>
#include <clp/ffi/ir_stream/protocol_constants.hpp>
#include <clp/ir/types.hpp>
#include <clp/type_utils.hpp>
#include <json/single_include/nlohmann/json.hpp>

#include <clp_ffi_py/api_decoration.hpp>
#include <clp_ffi_py/error_messages.hpp>
#include <clp_ffi_py/ir/native/error_messages.hpp>
#include <clp_ffi_py/ir/native/PyDeserializerBuffer.hpp>
#include <clp_ffi_py/ir/native/PyLogEvent.hpp>
#include <clp_ffi_py/ir/native/PyMetadata.hpp>
#include <clp_ffi_py/ir/native/PyQuery.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
using clp::ffi::ir_stream::encoded_tag_t;
using clp::ffi::ir_stream::IRErrorCode;
using clp::ffi::ir_stream::IRProtocolErrorCode;

namespace {
/**
 * This template defines the function signature of a termination handler required by
 * `deserialize_log_events`. Signature: (
 *         ffi::epoch_timestamp_ms timestamp,
 *         std::string_view deserialized_log_msg,
 *         size_t deserialized_log_event_idx,
 *         PyObject*& return_value
 * ) -> bool;
 * @tparam TerminateHandler
 */
template <typename TerminateHandler>
concept TerminateHandlerSignature = requires(TerminateHandler handler) {
    {
        handler(std::declval<clp::ir::epoch_time_ms_t>(),
                std::declval<std::string_view>(),
                std::declval<size_t>(),
                std::declval<PyObject*&>())
    } -> std::same_as<bool>;
};

/**
 * Handles the error when IRErrorCode::IRErrorCode_Incomplete_IR is seen. The handler will first
 * try to load more data into `deserializer_buffer`. If it fails, `allow_incomplete_stream` will be
 * used to determine whether to swallow the incomplete IR exception.
 * @param deserializer_buffer
 * @param allow_incomplete_stream A flag to indicate whether the incomplete stream error should be
 * ignored. If it is set to true, incomplete stream error.
 * @param std::nullopt if more data is loaded.
 * @param PyNone if the IR stream is incomplete and allowed.
 * @param nullptr if the IR stream is incomplete not allowed, with the relevant Python exceptions
 * and error set.
 */
[[nodiscard]] auto
handle_incomplete_ir_error(PyDeserializerBuffer* deserializer_buffer, bool allow_incomplete_stream)
        -> std::optional<PyObject*>;

/**
 * Deserializes the next log event from the CLP IR buffer `deserializer_buffer` until terminate
 * handler returns true.
 * @tparam TerminateHandler Method to determine if the deserialization should terminate, and set the
 * return value for termination.
 * @param deserializer_buffer IR deserializer buffer of the input IR stream.
 * @param allow_incomplete_stream A flag to indicate whether the incomplete stream error should be
 * ignored. If it is set to true, incomplete stream error should be treated as the IR stream is
 * terminated.
 * @param terminate_handler
 * @return The return value set by `terminate_handler`.
 * @return PyNone if the IR stream is terminated.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
template <TerminateHandlerSignature TerminateHandler>
[[nodiscard]] auto deserialize_log_events(
        PyDeserializerBuffer* deserializer_buffer,
        bool allow_incomplete_stream,
        TerminateHandler terminate_handler
) -> PyObject*;

auto
handle_incomplete_ir_error(PyDeserializerBuffer* deserializer_buffer, bool allow_incomplete_stream)
        -> std::optional<PyObject*> {
    if (deserializer_buffer->try_read()) {
        return std::nullopt;
    }
    if (allow_incomplete_stream
        && static_cast<bool>(
                PyErr_ExceptionMatches(PyDeserializerBuffer::get_py_incomplete_stream_error())
        ))
    {
        PyErr_Clear();
        Py_RETURN_NONE;
    }
    return nullptr;
}

template <TerminateHandlerSignature TerminateHandler>
auto deserialize_log_events(
        PyDeserializerBuffer* deserializer_buffer,
        bool allow_incomplete_stream,
        TerminateHandler terminate_handler
) -> PyObject* {
    std::string deserialized_message;
    clp::ir::epoch_time_ms_t timestamp_delta{0};
    auto timestamp{deserializer_buffer->get_ref_timestamp()};
    size_t current_log_event_idx{0};
    PyObject* return_value{nullptr};
    clp::ffi::ir_stream::encoded_tag_t tag{};

    while (true) {
        auto const unconsumed_bytes{deserializer_buffer->get_unconsumed_bytes()};
        clp::BufferReader ir_buffer{
                clp::size_checked_pointer_cast<char const>(unconsumed_bytes.data()),
                unconsumed_bytes.size()
        };

        if (auto const err{clp::ffi::ir_stream::deserialize_tag(ir_buffer, tag)};
            IRErrorCode::IRErrorCode_Success != err)
        {
            if (IRErrorCode::IRErrorCode_Incomplete_IR != err) {
                PyErr_Format(
                        PyExc_RuntimeError,
                        get_c_str_from_constexpr_string_view(cDeserializerErrorCodeFormatStr),
                        err
                );
                return nullptr;
            }
            if (auto const ret_val{
                        handle_incomplete_ir_error(deserializer_buffer, allow_incomplete_stream)
                };
                ret_val.has_value())
            {
                return ret_val.value();
            }
            continue;
        }
        if (clp::ffi::ir_stream::cProtocol::Eof == tag) {
            Py_RETURN_NONE;
        }

        auto const err{clp::ffi::ir_stream::four_byte_encoding::deserialize_log_event(
                ir_buffer,
                tag,
                deserialized_message,
                timestamp_delta
        )};
        if (IRErrorCode::IRErrorCode_Incomplete_IR == err) {
            if (auto const ret_val{
                        handle_incomplete_ir_error(deserializer_buffer, allow_incomplete_stream)
                };
                ret_val.has_value())
            {
                return ret_val.value();
            }
            continue;
        }
        if (IRErrorCode::IRErrorCode_Success != err) {
            PyErr_Format(
                    PyExc_RuntimeError,
                    get_c_str_from_constexpr_string_view(cDeserializerErrorCodeFormatStr),
                    err
            );
            return nullptr;
        }

        timestamp += timestamp_delta;
        current_log_event_idx = deserializer_buffer->get_and_increment_deserialized_message_count();
        auto const num_bytes_consumed{static_cast<Py_ssize_t>(ir_buffer.get_pos())};
        deserializer_buffer->commit_read_buffer_consumption(num_bytes_consumed);

        if (terminate_handler(timestamp, deserialized_message, current_log_event_idx, return_value))
        {
            deserializer_buffer->set_ref_timestamp(timestamp);
            break;
        }
    }

    return return_value;
}
}  // namespace

CLP_FFI_PY_METHOD auto
deserialize_preamble(PyObject* Py_UNUSED(self), PyObject* py_deserializer_buffer) -> PyObject* {
    if (false
        == static_cast<bool>(
                PyObject_TypeCheck(py_deserializer_buffer, PyDeserializerBuffer::get_py_type())
        ))
    {
        PyErr_SetString(PyExc_TypeError, get_c_str_from_constexpr_string_view(cPyTypeError));
        return nullptr;
    }

    auto* deserializer_buffer{py_reinterpret_cast<PyDeserializerBuffer>(py_deserializer_buffer)};
    bool is_four_byte_encoding{false};
    size_t ir_buffer_cursor_pos{0};
    while (true) {
        auto const unconsumed_bytes{deserializer_buffer->get_unconsumed_bytes()};
        clp::BufferReader ir_buffer{
                clp::size_checked_pointer_cast<char const>(unconsumed_bytes.data()),
                unconsumed_bytes.size()
        };
        auto const err{clp::ffi::ir_stream::get_encoding_type(ir_buffer, is_four_byte_encoding)};
        if (IRErrorCode::IRErrorCode_Success == err) {
            ir_buffer_cursor_pos = ir_buffer.get_pos();
            break;
        }
        if (IRErrorCode::IRErrorCode_Incomplete_IR != err) {
            PyErr_Format(
                    PyExc_RuntimeError,
                    get_c_str_from_constexpr_string_view(cDeserializerErrorCodeFormatStr),
                    err
            );
            return nullptr;
        }
        if (false == deserializer_buffer->try_read()) {
            return nullptr;
        }
    }
    deserializer_buffer->commit_read_buffer_consumption(static_cast<Py_ssize_t>(ir_buffer_cursor_pos
    ));
    if (false == is_four_byte_encoding) {
        PyErr_SetString(PyExc_NotImplementedError, "8-byte IR encoding is not supported yet.");
        return nullptr;
    }

    clp::ffi::ir_stream::encoded_tag_t metadata_type_tag{0};
    size_t metadata_pos{0};
    uint16_t metadata_size{0};
    while (true) {
        auto const unconsumed_bytes = deserializer_buffer->get_unconsumed_bytes();
        clp::BufferReader ir_buffer{
                clp::size_checked_pointer_cast<char const>(unconsumed_bytes.data()),
                unconsumed_bytes.size()
        };
        auto const err{clp::ffi::ir_stream::deserialize_preamble(
                ir_buffer,
                metadata_type_tag,
                metadata_pos,
                metadata_size
        )};
        if (IRErrorCode::IRErrorCode_Success == err) {
            ir_buffer_cursor_pos = ir_buffer.get_pos();
            break;
        }
        if (IRErrorCode ::IRErrorCode_Incomplete_IR != err) {
            PyErr_Format(
                    PyExc_RuntimeError,
                    get_c_str_from_constexpr_string_view(cDeserializerErrorCodeFormatStr),
                    err
            );
            return nullptr;
        }
        if (false == deserializer_buffer->try_read()) {
            return nullptr;
        }
    }

    auto const unconsumed_bytes = deserializer_buffer->get_unconsumed_bytes();
    auto const metadata_buffer{
            unconsumed_bytes.subspan(metadata_pos, static_cast<size_t>(metadata_size))
    };
    deserializer_buffer->commit_read_buffer_consumption(static_cast<Py_ssize_t>(ir_buffer_cursor_pos
    ));
    PyMetadata* metadata{nullptr};
    try {
        // Initialization list should not be used in this case:
        // https://github.com/nlohmann/json/discussions/4096
        nlohmann::json const metadata_json(
                nlohmann::json::parse(metadata_buffer.begin(), metadata_buffer.end())
        );
        std::string const version{metadata_json.at(
                static_cast<char const*>(clp::ffi::ir_stream::cProtocol::Metadata::VersionKey)
        )};
        auto const error_code{clp::ffi::ir_stream::validate_protocol_version(version)};
        if (IRProtocolErrorCode::BackwardCompatible != error_code) {
            switch (error_code) {
                case IRProtocolErrorCode::Supported:
                    // This represents a key-value pair IR stream, which is not supported by these
                    // old deserialization methods.
                    PyErr_Format(PyExc_RuntimeError, "Version too new: %s", version.c_str());
                    break;
                case IRProtocolErrorCode::Unsupported:
                    PyErr_Format(PyExc_RuntimeError, "Version unsupported: %s", version.c_str());
                    break;
                default:
                    PyErr_Format(
                            PyExc_NotImplementedError,
                            "Unrecognized return code %d with version: %s",
                            error_code,
                            version.c_str()
                    );
                    break;
            }
            return nullptr;
        }
        metadata = PyMetadata::create_new_from_json(metadata_json, is_four_byte_encoding);
    } catch (nlohmann::json::exception& ex) {
        PyErr_Format(PyExc_RuntimeError, "Json Parsing Error: %s", ex.what());
        return nullptr;
    }
    if (false == deserializer_buffer->metadata_init(metadata)) {
        return nullptr;
    }
    return py_reinterpret_cast<PyObject>(metadata);
}

CLP_FFI_PY_METHOD auto
deserialize_next_log_event(PyObject* Py_UNUSED(self), PyObject* args, PyObject* keywords)
        -> PyObject* {
    static char keyword_deserializer_buffer[]{"deserializer_buffer"};
    static char keyword_query[]{"query"};
    static char keyword_allow_incomplete_stream[]{"allow_incomplete_stream"};
    static char* keyword_table[]{
            static_cast<char*>(keyword_deserializer_buffer),
            static_cast<char*>(keyword_query),
            static_cast<char*>(keyword_allow_incomplete_stream),
            nullptr
    };

    PyDeserializerBuffer* deserializer_buffer{nullptr};
    PyObject* query_obj{Py_None};
    int allow_incomplete_stream{0};

    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "O!|Op",
                static_cast<char**>(keyword_table),
                PyDeserializerBuffer::get_py_type(),
                &deserializer_buffer,
                &query_obj,
                &allow_incomplete_stream
        )))
    {
        return nullptr;
    }

    bool const is_query_given{Py_None != query_obj};
    if (is_query_given
        && false == static_cast<bool>(PyObject_TypeCheck(query_obj, PyQuery::get_py_type())))
    {
        PyErr_SetString(PyExc_TypeError, get_c_str_from_constexpr_string_view(cPyTypeError));
        return nullptr;
    }

    if (false == deserializer_buffer->has_metadata()) {
        PyErr_SetString(
                PyExc_RuntimeError,
                "The given deserializerBuffer does not have a valid CLP IR metadata deserialized."
        );
        return nullptr;
    }
    auto* metadata{deserializer_buffer->get_metadata()};

    if (false == is_query_given) {
        auto terminate_handler{
                [metadata](
                        clp::ir::epoch_time_ms_t timestamp,
                        std::string_view log_message,
                        size_t log_event_idx,
                        PyObject*& return_value
                ) -> bool {
                    return_value = py_reinterpret_cast<PyObject>(PyLogEvent::create_new_log_event(
                            log_message,
                            timestamp,
                            log_event_idx,
                            metadata
                    ));
                    return true;
                }
        };
        return deserialize_log_events(
                deserializer_buffer,
                static_cast<bool>(allow_incomplete_stream),
                terminate_handler
        );
    }

    auto* py_query{py_reinterpret_cast<PyQuery>(query_obj)};
    auto const* query{py_query->get_query()};
    auto query_terminate_handler{
            [query, metadata](
                    clp::ir::epoch_time_ms_t timestamp,
                    std::string_view log_message,
                    size_t log_event_idx,
                    PyObject*& return_value
            ) -> bool {
                if (query->ts_safely_outside_time_range(timestamp)) {
                    return_value = get_new_ref_to_py_none();
                    return true;
                }
                if (false == query->matches_time_range(timestamp)
                    || false == query->matches_wildcard_queries(log_message))
                {
                    return false;
                }
                return_value = py_reinterpret_cast<PyObject>(PyLogEvent::create_new_log_event(
                        log_message,
                        timestamp,
                        log_event_idx,
                        metadata
                ));
                return true;
            }
    };
    return deserialize_log_events(
            deserializer_buffer,
            static_cast<bool>(allow_incomplete_stream),
            query_terminate_handler
    );
}
}  // namespace clp_ffi_py::ir::native
