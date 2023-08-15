#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include "decoding_methods.hpp"

#include <clp/components/core/src/ffi/ir_stream/decoding_methods.hpp>
#include <clp/components/core/src/type_utils.hpp>
#include <gsl/span>
#include <json/single_include/nlohmann/json.hpp>

#include <clp_ffi_py/error_messages.hpp>
#include <clp_ffi_py/ir/error_messages.hpp>
#include <clp_ffi_py/ir/PyDecoderBuffer.hpp>
#include <clp_ffi_py/ir/PyLogEvent.hpp>
#include <clp_ffi_py/ir/PyMetadata.hpp>
#include <clp_ffi_py/ir/PyQuery.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir {
namespace {
/**
 * Decodes the next log event from the CLP IR buffer `decoder_buffer`. If
 * `py_query` is non-null decode until finding a log event that matches the
 * query.
 * @param decoder_buffer IR decoder buffer of the input IR stream.
 * @param py_metadata The metadata associated with the input IR stream.
 * @param py_query Search query to filter log events.
 * @return Log event represented as PyLogEvent on success.
 * @return PyNone on termination.
 * @return nullptr on failure with the relevant Python exception and error set.
 */
auto decode(PyDecoderBuffer* decoder_buffer, PyMetadata* py_metadata, PyQuery* py_query)
        -> PyObject* {
    std::string decoded_message;
    ffi::epoch_time_ms_t timestamp_delta{0};
    auto timestamp{decoder_buffer->get_ref_timestamp()};
    size_t current_log_event_idx{0};
    bool reached_eof{false};
    while (true) {
        auto const unconsumed_bytes{decoder_buffer->get_unconsumed_bytes()};
        ffi::ir_stream::IrBuffer ir_buffer{unconsumed_bytes.data(), unconsumed_bytes.size()};
        auto const err{ffi::ir_stream::four_byte_encoding::decode_next_message(
                ir_buffer,
                decoded_message,
                timestamp_delta
        )};
        if (ffi::ir_stream::IRErrorCode_Incomplete_IR == err) {
            if (false == decoder_buffer->try_read()) {
                return nullptr;
            }
            continue;
        }
        if (ffi::ir_stream::IRErrorCode_Eof == err) {
            reached_eof = true;
            break;
        }
        if (ffi::ir_stream::IRErrorCode_Success != err) {
            PyErr_Format(PyExc_RuntimeError, cDecoderErrorCodeFormatStr, err);
            return nullptr;
        }

        timestamp += timestamp_delta;
        current_log_event_idx = decoder_buffer->get_and_increment_decoded_message_count();
        decoder_buffer->commit_read_buffer_consumption(
                static_cast<Py_ssize_t>(ir_buffer.get_cursor_pos())
        );

        if (nullptr == py_query) {
            break;
        }

        auto* query{py_query->get_query()};
        if (query->ts_safely_outside_time_range(timestamp)) {
            Py_RETURN_NONE;
        }
        if (query->matches_time_range(timestamp)
            && query->matches_wildcard_queries(decoded_message))
        {
            break;
        }
    }

    if (reached_eof) {
        Py_RETURN_NONE;
    }

    decoder_buffer->set_ref_timestamp(timestamp);
    return py_reinterpret_cast<PyObject>(PyLogEvent::create_new_log_event(
            decoded_message,
            timestamp,
            current_log_event_idx,
            py_metadata
    ));
}
}  // namespace

extern "C" {
auto decode_preamble(PyObject* Py_UNUSED(self), PyObject* py_decoder_buffer) -> PyObject* {
    if (false
        == static_cast<bool>(PyObject_TypeCheck(py_decoder_buffer, PyDecoderBuffer::get_py_type())))
    {
        PyErr_SetString(PyExc_TypeError, cPyTypeError);
        return nullptr;
    }

    auto* decoder_buffer{py_reinterpret_cast<PyDecoderBuffer>(py_decoder_buffer)};
    bool is_four_byte_encoding{false};
    size_t ir_buffer_cursor_pos{0};
    while (true) {
        auto const unconsumed_bytes{decoder_buffer->get_unconsumed_bytes()};
        ffi::ir_stream::IrBuffer ir_buffer{unconsumed_bytes.data(), unconsumed_bytes.size()};
        auto const err{ffi::ir_stream::get_encoding_type(ir_buffer, is_four_byte_encoding)};
        if (ffi::ir_stream::IRErrorCode_Success == err) {
            ir_buffer_cursor_pos = ir_buffer.get_cursor_pos();
            break;
        }
        if (ffi::ir_stream::IRErrorCode_Incomplete_IR != err) {
            PyErr_Format(PyExc_RuntimeError, cDecoderErrorCodeFormatStr, err);
            return nullptr;
        }
        if (false == decoder_buffer->try_read()) {
            return nullptr;
        }
    }
    decoder_buffer->commit_read_buffer_consumption(static_cast<Py_ssize_t>(ir_buffer_cursor_pos));
    if (false == is_four_byte_encoding) {
        PyErr_SetString(PyExc_NotImplementedError, "8-byte IR decoding is not supported yet.");
        return nullptr;
    }

    ffi::ir_stream::encoded_tag_t metadata_type_tag{0};
    size_t metadata_pos{0};
    uint16_t metadata_size{0};
    while (true) {
        auto const unconsumed_bytes = decoder_buffer->get_unconsumed_bytes();
        ffi::ir_stream::IrBuffer ir_buffer{unconsumed_bytes.data(), unconsumed_bytes.size()};
        auto const err{ffi::ir_stream::decode_preamble(
                ir_buffer,
                metadata_type_tag,
                metadata_pos,
                metadata_size
        )};
        if (ffi::ir_stream::IRErrorCode_Success == err) {
            ir_buffer_cursor_pos = ir_buffer.get_cursor_pos();
            break;
        }
        if (ffi::ir_stream::IRErrorCode_Incomplete_IR != err) {
            PyErr_Format(PyExc_RuntimeError, cDecoderErrorCodeFormatStr, err);
            return nullptr;
        }
        if (false == decoder_buffer->try_read()) {
            return nullptr;
        }
    }

    auto const unconsumed_bytes = decoder_buffer->get_unconsumed_bytes();
    auto const metadata_buffer{
            unconsumed_bytes.subspan(metadata_pos, static_cast<size_t>(metadata_size))};
    decoder_buffer->commit_read_buffer_consumption(static_cast<Py_ssize_t>(ir_buffer_cursor_pos));
    PyMetadata* metadata{nullptr};
    try {
        // Initialization list should not be used in this case:
        // https://github.com/nlohmann/json/discussions/4096
        nlohmann::json const metadata_json(
                nlohmann::json::parse(metadata_buffer.begin(), metadata_buffer.end())
        );
        metadata = PyMetadata::create_new_from_json(metadata_json, is_four_byte_encoding);
    } catch (nlohmann::json::exception& ex) {
        PyErr_Format(PyExc_RuntimeError, "Json Parsing Error: %s", ex.what());
        return nullptr;
    }
    if (false == decoder_buffer->metadata_init(metadata)) {
        return nullptr;
    }
    return py_reinterpret_cast<PyObject>(metadata);
}

auto decode_next_log_event(PyObject* Py_UNUSED(self), PyObject* args, PyObject* keywords)
        -> PyObject* {
    static char keyword_decoder_buffer[]{"decoder_buffer"};
    static char keyword_query[]{"query"};
    static char* keyword_table[]{
            static_cast<char*>(keyword_decoder_buffer),
            static_cast<char*>(keyword_query),
            nullptr};

    PyDecoderBuffer* decoder_buffer{nullptr};
    PyObject* query{Py_None};

    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "O!|O",
                static_cast<char**>(keyword_table),
                PyDecoderBuffer::get_py_type(),
                &decoder_buffer,
                &query
        )))
    {
        return nullptr;
    }

    bool const is_query_given{Py_None != query};
    if (is_query_given
        && false == static_cast<bool>(PyObject_TypeCheck(query, PyQuery::get_py_type())))
    {
        PyErr_SetString(PyExc_TypeError, cPyTypeError);
        return nullptr;
    }

    if (false == decoder_buffer->has_metadata()) {
        PyErr_SetString(
                PyExc_RuntimeError,
                "The given DecoderBuffer does not have a valid CLP IR metadata decoded."
        );
        return nullptr;
    }

    return decode(
            decoder_buffer,
            decoder_buffer->get_metadata(),
            is_query_given ? py_reinterpret_cast<PyQuery>(query) : nullptr
    );
}
}
}  // namespace clp_ffi_py::ir
