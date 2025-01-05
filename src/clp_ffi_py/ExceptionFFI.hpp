#ifndef CLP_FFI_PY_EXCEPTION_FFI
#define CLP_FFI_PY_EXCEPTION_FFI

#include <string>
#include <utility>

#include <clp/ErrorCode.hpp>
#include <clp/TraceableException.hpp>

#include <clp_ffi_py/PyExceptionContext.hpp>

namespace clp_ffi_py {
/**
 * A class that represents a traceable exception during the native code execution. It captures any
 * Python exceptions set, allowing the handler at the catch site to either restore or discard the
 * exception as needed.
 */
class ExceptionFFI : public clp::TraceableException {
public:
    ExceptionFFI(
            clp::ErrorCode error_code,
            char const* const filename,
            int line_number,
            std::string message
    )
            : TraceableException{error_code, filename, line_number},
              m_message{std::move(message)} {}

    [[nodiscard]] auto what() const noexcept -> char const* override { return m_message.c_str(); }

    [[nodiscard]] auto get_py_exception_context() -> PyExceptionContext& {
        return m_py_exception_context;
    }

private:
    std::string m_message;
    PyExceptionContext m_py_exception_context;
};
}  // namespace clp_ffi_py

#endif  // CLP_FFI_PY_EXCEPTION_FFI
