#include <clp_ffi_py/Python.hpp>  // Must always be included before any other header files

#include "PyDecoderBuffer.hpp"

#include <clp_ffi_py/error_messages.hpp>
#include <clp_ffi_py/ir/error_messages.hpp>
#include <clp_ffi_py/PyObjectCast.hpp>
#include <clp_ffi_py/PyObjectUtils.hpp>
#include <clp_ffi_py/utils.hpp>

namespace clp_ffi_py::ir {
namespace {
extern "C" {
/**
 * Callback of PyDecoderBuffer `__init__` method:
 * __init__(TODO)
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
    return -1;
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
auto PyDecoderBuffer_releasebuffer(PyDecoderBuffer* Py_UNUSED(self), Py_buffer* Py_UNUSED(view)) -> void {
    return;
}
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyMethodDef PyDecoderBuffer_method_table[]{
        {nullptr}};

/**
 * Declaration of Python buffer protocol.
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
static PyBufferProcs PyDecoderBuffer_as_buffer{
        .bf_getbuffer = py_getbufferproc_cast(PyDecoderBuffer_getbuffer),
        .bf_releasebuffer = py_releasebufferproc_cast(PyDecoderBuffer_releasebuffer),
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays)
PyDoc_STRVAR(
        cPyDecoderBufferDoc,
        "TODO\n"
);

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-*-cast)
PyType_Slot PyDecoderBuffer_slots[]{
        {Py_tp_alloc, reinterpret_cast<void*>(PyType_GenericAlloc)},
        {Py_tp_dealloc, reinterpret_cast<void*>(PyDecoderBuffer_dealloc)},
        {Py_tp_new, reinterpret_cast<void*>(PyType_GenericNew)},
        {Py_tp_init, reinterpret_cast<void*>(PyDecoderBuffer_init)},
        {Py_tp_methods, static_cast<void*>(PyDecoderBuffer_method_table)},
        {Py_tp_doc, const_cast<void*>(static_cast<void const*>(cPyDecoderBufferDoc))},
        {0, nullptr}};
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, cppcoreguidelines-pro-type-*-cast)

/**
 * PyDecoderBuffer Python type specifications.
 */
PyType_Spec PyDecoderBuffer_type_spec{
        "clp_ffi_py.ir.DecoderBuffer",
        sizeof(PyDecoderBuffer),
        0,
        Py_TPFLAGS_DEFAULT,
        static_cast<PyType_Slot*>(PyDecoderBuffer_slots)};
}

auto PyDecoderBuffer::init(PyObject* input_stream, Py_ssize_t buf_capacity) -> bool {
    m_buffer_capacity = buf_capacity;
    m_read_buffer = static_cast<int8_t*>(PyMem_Malloc(buf_capacity));
    if (nullptr == m_read_buffer) {
        return false;
    }
    m_buffer_size = 0;
    m_cursor_pos = 0;
    m_num_decoded_message = 0;
    m_input_ir_stream = input_stream;
    Py_INCREF(m_input_ir_stream);
    return true;
}

auto PyDecoderBuffer::populate_read_buffer(Py_ssize_t& num_bytes_read) -> bool {
    auto const num_unconsumed_bytes{get_num_unconsumed_bytes()};
    auto const* unconsumed_bytes{get_unconsumed_bytes()};

    if (num_unconsumed_bytes > (m_buffer_capacity / 2)) {
        // PyMem_Realloc is not used to avoid redundant memory copy
        auto const new_capacity{m_buffer_capacity * 2};
        auto* new_buf{static_cast<int8_t*>(PyMem_Malloc(new_capacity))};
        if (nullptr == new_buf) {
            return false;
        }
        memcpy(new_buf, unconsumed_bytes, static_cast<size_t>(num_unconsumed_bytes));
        PyMem_Free(m_read_buffer);
        m_read_buffer = new_buf;
        m_buffer_capacity = new_capacity;
    } else if (0 < num_unconsumed_bytes) {
        memcpy(m_read_buffer, unconsumed_bytes, static_cast<size_t>(num_unconsumed_bytes));
    }
    m_cursor_pos = 0;
    m_buffer_size = num_unconsumed_bytes;

    PyObjectPtr<PyObject> num_read_byte_obj{PyObject_CallMethod(
        m_input_ir_stream, 
        "readinto", 
        "O", 
        py_reinterpret_cast<PyObject>(this))
    };
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

auto PyDecoderBuffer::py_getbuffer(Py_buffer* view, int flags) -> int {
    auto const buffer_size{m_buffer_capacity - m_buffer_size};
    if (0 >= buffer_size) {
        PyErr_SetString(PyExc_RuntimeError, cDecoderBufferFullError);
        return -1;
    }
    auto* buffer{m_read_buffer + m_buffer_size};
    return PyBuffer_FillInfo(
        view,
        py_reinterpret_cast<PyObject>(this),
        buffer,
        buffer_size,
        0,
        flags
    );
}

auto PyDecoderBuffer::commit_read_buffer_consumption(Py_ssize_t num_bytes_consumed) -> bool {
    if (get_num_unconsumed_bytes() < num_bytes_consumed) {
        PyErr_SetString(PyExc_OverflowError, cDecoderBufferCursorOverflowError);
        return false;
    }
    m_cursor_pos += num_bytes_consumed;
    return true;
}

auto PyDecoderBuffer::get_py_type() -> PyTypeObject* {
    return m_py_type.get();
}

auto PyDecoderBuffer::module_level_init(PyObject* py_module) -> bool {
    static_assert(std::is_trivially_destructible<PyDecoderBuffer>());
    auto* type{py_reinterpret_cast<PyTypeObject>(PyType_FromSpec(&PyDecoderBuffer_type_spec))};
    m_py_type.reset(type);
    if (nullptr == type) {
        return false;
    }
    type->tp_as_buffer = &PyDecoderBuffer_as_buffer;
    return add_python_type(get_py_type(), "DecoderBuffer", py_module);
}
}