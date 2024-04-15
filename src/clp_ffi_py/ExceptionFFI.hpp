#ifndef CLP_FFI_PY_EXCEPTION_FFI
#define CLP_FFI_PY_EXCEPTION_FFI

#include <string>

#include <clp/components/core/src/clp/TraceableException.hpp>

namespace clp_ffi_py {
/**
 * A class that represents a traceable exception during the native code
 * execution. Note: for exceptions of CPython execution, please use CPython
 * interface to set the exception instead.
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
              m_message{std::move(message)} {};

    [[nodiscard]] auto what() const noexcept -> char const* override { return m_message.c_str(); }

private:
    std::string m_message;
};
}  // namespace clp_ffi_py

#endif  // CLP_FFI_PY_EXCEPTION_FFI
