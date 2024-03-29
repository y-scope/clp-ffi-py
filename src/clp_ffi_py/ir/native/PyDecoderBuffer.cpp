#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include "PyDecoderBuffer.hpp"

#include <algorithm>
#include <random>
#include <span>

#include <clp_ffi_py/error_messages.hpp>
#include <clp_ffi_py/ir/native/error_messages.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir::native {
namespace {
extern "C" {
/**
 * Callback of PyDecoderBuffer `__init__` method:
 * __init__(self, input_stream: IO[bytes], initial_buffer_capacity: int = 4096)
 * Keyword argument parsing is supported.
 * Assumes `self` is uninitialized and will allocate the underlying memory. If
 * `self` is already initialized this will result in memory leaks.
 * @param self
 * @param args
 * @param keywords
 * @return 0 on success.
 * @return -1 on failure with the relevant Python exception and error set.
 */
auto PyDecoderBuffer_init(PyDecoderBuffer* self, PyObject* args, PyObject* keywords) -> int {
    static char keyword_input_stream[]{"input_stream"};
    static char keyword_initial_buffer_capacity[]{"initial_buffer_capacity"};
    static char* keyword_table[]{
            static_cast<char*>(keyword_input_stream),
            static_cast<char*>(keyword_initial_buffer_capacity),
            nullptr
    };

    // If the argument parsing fails, `self` will be deallocated. We must reset
    // all pointers to nullptr in advance, otherwise the deallocator might
    // trigger a segmentation fault.
    self->default_init();

    PyObject* input_stream{nullptr};
    Py_ssize_t initial_buffer_capacity{PyDecoderBuffer::cDefaultInitialCapacity};
    if (false
        == static_cast<bool>(PyArg_ParseTupleAndKeywords(
                args,
                keywords,
                "O|L",
                static_cast<char**>(keyword_table),
                &input_stream,
                &initial_buffer_capacity
        )))
    {
        return -1;
    }

    PyObjectPtr<PyObject> const readinto_method_obj{PyObject_GetAttrString(input_stream, "readinto")
    };
    auto* readinto_method{readinto_method_obj.get()};
    if (nullptr == readinto_method) {
        return -1;
    }

    if (false == static_cast<bool>(PyCallable_Check(readinto_method))) {
        PyErr_SetString(
                PyExc_TypeError,
                "The attribute `readinto` of the given input stream object is not callable."
        );
        return -1;
    }

    if (false == self->init(input_stream, initial_buffer_capacity)) {
        return -1;
    }

    return 0;
}

/**
 * Callback of PyDecoderBuffer deallocator.
 * @param self
 */
auto PyDecoderBuffer_dealloc(PyDecoderBuffer* self) -> void {
    self->clean();
    PyObject_Del(self);
}

/**
 * Callback of Python buffer protocol's `getbuffer` operation.
 * @param self
 * @param view
 * @param flags
 * @return 0 on success.
 * @return -1 on failure with the relevant Python exception and error set.
 */
auto PyDecoderBuffer_getbuffer(PyDecoderBuffer* self, Py_buffer* view, int flags) -> int {
    return self->py_getbuffer(view, flags);
}

/**
 * Callback of Python buffer protocol's `releasebuffer` operation.
 * This callback doesn't do anything, but it is set intentionally to avoid
 * unexpected behaviour.
 * @param self (unused).
 * @param view (unused).
 */
auto PyDecoderBuffer_releasebuffer(PyDecoderBuffer* Py_UNUSED(self), Py_buffer* Py_UNUSED(view))
        -> void {}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyDecoderBufferGetNumDecodedLogMessages,
        "get_num_decoded_log_messages(self)\n"
        "--\n\n"
        ":return: Total number of messages decoded so far.\n"
);

auto PyDecoderBuffer_get_num_decoded_log_messages(PyDecoderBuffer* self) -> PyObject* {
    return PyLong_FromLongLong(static_cast<long long>(self->get_num_decoded_message()));
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyDecoderBufferTestStreamingDoc,
        "_test_streaming(self, seed)\n"
        "--\n\n"
        "Tests the functionality of the DecoderBuffer by streaming the entire input stream into "
        "a Python bytearray. The stepping size from the read buffer is randomly generated, "
        "initialized by the given seed.\n\n"
        "Note: this function should only be used for testing purpose.\n\n"
        ":param seed_obj: Random seed.\n"
        ":return: The entire input stream stored in a Python bytearray.\n"
);

auto PyDecoderBuffer_test_streaming(PyDecoderBuffer* self, PyObject* seed_obj) -> PyObject* {
    unsigned seed{0};
    if (false == parse_py_int<uint32_t>(seed_obj, seed)) {
        return nullptr;
    }
    return self->test_streaming(seed);
}
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyMethodDef PyDecoderBuffer_method_table[]{
        {"get_num_decoded_log_messages",
         py_c_function_cast(PyDecoderBuffer_get_num_decoded_log_messages),
         METH_NOARGS,
         static_cast<char const*>(cPyDecoderBufferGetNumDecodedLogMessages)},

        {"_test_streaming",
         py_c_function_cast(PyDecoderBuffer_test_streaming),
         METH_O,
         static_cast<char const*>(cPyDecoderBufferTestStreamingDoc)},

        {nullptr}
};

/**
 * Declaration of Python buffer protocol.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyBufferProcs PyDecoderBuffer_as_buffer{
        .bf_getbuffer = py_getbufferproc_cast(PyDecoderBuffer_getbuffer),
        .bf_releasebuffer = py_releasebufferproc_cast(PyDecoderBuffer_releasebuffer),
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyDecoderBufferDoc,
        "This class represents a CLP IR Decoder Buffer corresponding to a CLP IR stream. "
        "It buffers encoded CLP IR data read from the input stream, which can be consumed by the "
        "CLP IR decoding methods to recover encoded log events. An instance of this class is "
        "expected to be passed across different calls of CLP IR decoding methods when decoding "
        "from the same IR stream.\n\n"
        "The signature of `__init__` method is shown as following:\n\n"
        "__init__(self, input_stream, initial_buffer_capacity=4096)\n\n"
        "Initializes a DecoderBuffer object for the given input IR stream.\n\n"
        ":param input_stream: Input stream that contains encoded CLP IR. It should be an instance "
        "of type `IO[bytes]` with the method `readinto` supported.\n"
        ":param initial_buffer_capacity: The initial capacity of the underlying byte buffer.\n"
);

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-*-cast)
PyType_Slot PyDecoderBuffer_slots[]{
        {Py_tp_alloc, reinterpret_cast<void*>(PyType_GenericAlloc)},
        {Py_tp_dealloc, reinterpret_cast<void*>(PyDecoderBuffer_dealloc)},
        {Py_tp_new, reinterpret_cast<void*>(PyType_GenericNew)},
        {Py_tp_init, reinterpret_cast<void*>(PyDecoderBuffer_init)},
        {Py_tp_methods, static_cast<void*>(PyDecoderBuffer_method_table)},
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPyDecoderBufferDoc))},
        {0, nullptr}
};
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-*-cast)

/**
 * PyDecoderBuffer Python type specifications.
 */
PyType_Spec PyDecoderBuffer_type_spec{
        "clp_ffi_py.ir.native.DecoderBuffer",
        sizeof(PyDecoderBuffer),
        0,
        Py_TPFLAGS_DEFAULT,
        static_cast<PyType_Slot*>(PyDecoderBuffer_slots)
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyIncompleteStreamErrorDoc,
        "This exception will be raised if the decoder buffer cannot read more data from the "
        "input stream while the decoding method expects more bytes.\n"
        "Typically, this error indicates the input stream has been truncated.\n"
);
}  // namespace

auto PyDecoderBuffer::init(PyObject* input_stream, Py_ssize_t buf_capacity) -> bool {
    m_read_buffer_mem_owner = static_cast<int8_t*>(PyMem_Malloc(buf_capacity));
    if (nullptr == m_read_buffer_mem_owner) {
        PyErr_NoMemory();
        return false;
    }
    m_read_buffer = std::span<int8_t>{m_read_buffer_mem_owner, static_cast<size_t>(buf_capacity)};
    m_input_ir_stream = input_stream;
    Py_INCREF(m_input_ir_stream);
    return true;
}

auto PyDecoderBuffer::populate_read_buffer(Py_ssize_t& num_bytes_read) -> bool {
    auto const unconsumed_bytes_in_curr_read_buffer{get_unconsumed_bytes()};
    auto const num_unconsumed_bytes{
            static_cast<Py_ssize_t>(unconsumed_bytes_in_curr_read_buffer.size())
    };
    auto const buffer_capacity{static_cast<Py_ssize_t>(m_read_buffer.size())};

    if (num_unconsumed_bytes > (buffer_capacity / 2)) {
        // PyMem_Realloc is not used to avoid redundant memory copy
        auto const new_capacity{buffer_capacity * 2};
        auto* new_buf{static_cast<int8_t*>(PyMem_Malloc(new_capacity))};
        if (nullptr == new_buf) {
            PyErr_NoMemory();
            return false;
        }
        std::span<int8_t> const new_read_buffer{new_buf, static_cast<size_t>(new_capacity)};
        std::copy(
                unconsumed_bytes_in_curr_read_buffer.begin(),
                unconsumed_bytes_in_curr_read_buffer.end(),
                new_read_buffer.begin()
        );
        PyMem_Free(m_read_buffer_mem_owner);
        m_read_buffer_mem_owner = new_buf;
        m_read_buffer = new_read_buffer;
    } else if (0 < num_unconsumed_bytes) {
        std::copy(
                unconsumed_bytes_in_curr_read_buffer.begin(),
                unconsumed_bytes_in_curr_read_buffer.end(),
                m_read_buffer.begin()
        );
    }
    m_num_current_bytes_consumed = 0;
    m_buffer_size = num_unconsumed_bytes;

    enable_py_buffer_protocol();
    PyObjectPtr<PyObject> const num_read_byte_obj{PyObject_CallMethod(
            m_input_ir_stream,
            "readinto",
            "O",
            py_reinterpret_cast<PyObject>(this)
    )};
    disable_py_buffer_protocol();

    if (nullptr == num_read_byte_obj.get()) {
        return false;
    }
    num_bytes_read = PyLong_AsSsize_t(num_read_byte_obj.get());
    if (0 > num_bytes_read) {
        return false;
    }
    m_buffer_size += num_bytes_read;
    return true;
}

auto PyDecoderBuffer::metadata_init(PyMetadata* metadata) -> bool {
    if (has_metadata()) {
        PyErr_SetString(PyExc_RuntimeError, "Metadata has already been initialized.");
        return false;
    }
    if (nullptr == metadata) {
        return false;
    }
    Py_INCREF(metadata);
    m_metadata = metadata;
    set_ref_timestamp(metadata->get_metadata()->get_ref_timestamp());
    return true;
}

auto PyDecoderBuffer::py_getbuffer(Py_buffer* view, int flags) -> int {
    // Don't need to set the exception message during the failure.
    // The Python level caller will set the exception and thus overwrite it.
    if (false == is_py_buffer_protocol_enabled()) {
        return -1;
    }
    auto const buffer{m_read_buffer.subspan(m_buffer_size, m_read_buffer.size() - m_buffer_size)};
    return PyBuffer_FillInfo(
            view,
            py_reinterpret_cast<PyObject>(this),
            buffer.data(),
            static_cast<Py_ssize_t>(buffer.size()),
            0,
            flags
    );
}

auto PyDecoderBuffer::commit_read_buffer_consumption(Py_ssize_t num_bytes_consumed) -> bool {
    if (get_num_unconsumed_bytes() < num_bytes_consumed) {
        PyErr_SetString(PyExc_OverflowError, cDecoderBufferOverflowError);
        return false;
    }
    m_num_current_bytes_consumed += num_bytes_consumed;
    return true;
}

auto PyDecoderBuffer::try_read() -> bool {
    Py_ssize_t num_bytes_read{0};
    if (false == populate_read_buffer(num_bytes_read)) {
        return false;
    }
    if (0 == num_bytes_read) {
        PyErr_SetString(get_py_incomplete_stream_error(), cDecoderIncompleteIRError);
        return false;
    }
    return true;
}

auto PyDecoderBuffer::test_streaming(uint32_t seed) -> PyObject* {
    std::default_random_engine rand_generator(seed);
    std::vector<uint8_t> read_bytes;
    bool reach_istream_end{false};
    while (false == reach_istream_end) {
        std::uniform_int_distribution<Py_ssize_t> distribution(
                1,
                static_cast<Py_ssize_t>(m_read_buffer.size())
        );
        auto num_bytes_to_read{distribution(rand_generator)};
        if (get_num_unconsumed_bytes() < num_bytes_to_read) {
            Py_ssize_t num_bytes_read_from_istream{0};
            if (false == populate_read_buffer(num_bytes_read_from_istream)) {
                return nullptr;
            }
            if (0 == num_bytes_read_from_istream) {
                reach_istream_end = true;
            }
            num_bytes_to_read = std::min<Py_ssize_t>(num_bytes_to_read, m_buffer_size);
        }
        auto const unconsumed_bytes{get_unconsumed_bytes()};
        auto const bytes_to_consume{unconsumed_bytes.subspan(0, num_bytes_to_read)};
        read_bytes.insert(read_bytes.end(), bytes_to_consume.begin(), bytes_to_consume.end());
        commit_read_buffer_consumption(num_bytes_to_read);
    }
    return PyByteArray_FromStringAndSize(
            clp::size_checked_pointer_cast<char>(read_bytes.data()),
            static_cast<Py_ssize_t>(read_bytes.size())
    );
}

PyObjectStaticPtr<PyTypeObject> PyDecoderBuffer::m_py_type{nullptr};
PyObjectStaticPtr<PyObject> PyDecoderBuffer::m_py_incomplete_stream_error{nullptr};

auto PyDecoderBuffer::get_py_type() -> PyTypeObject* {
    return m_py_type.get();
}

auto PyDecoderBuffer::get_py_incomplete_stream_error() -> PyObject* {
    return m_py_incomplete_stream_error.get();
}

auto PyDecoderBuffer::module_level_init(PyObject* py_module) -> bool {
    static_assert(std::is_trivially_destructible<PyDecoderBuffer>());
    auto* py_incomplete_stream_error{PyErr_NewExceptionWithDoc(
            "clp_ffi_py.native.IncompleteStreamError",
            static_cast<char const*>(cPyIncompleteStreamErrorDoc),
            nullptr,
            nullptr
    )};
    m_py_incomplete_stream_error.reset(py_incomplete_stream_error);
    if (nullptr == py_incomplete_stream_error) {
        return false;
    }
    if (0 > PyModule_AddObject(py_module, "IncompleteStreamError", py_incomplete_stream_error)) {
        return false;
    }

    auto* type{py_reinterpret_cast<PyTypeObject>(PyType_FromSpec(&PyDecoderBuffer_type_spec))};
    m_py_type.reset(type);
    if (nullptr == type) {
        return false;
    }
    type->tp_as_buffer = &PyDecoderBuffer_as_buffer;
    return add_python_type(get_py_type(), "DecoderBuffer", py_module);
}
}  // namespace clp_ffi_py::ir::native
