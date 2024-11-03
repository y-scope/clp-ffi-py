#ifndef CLP_FFI_PY_API_DECORATION_HPP
#define CLP_FFI_PY_API_DECORATION_HPP

/**
 * `CLP_FFI_PY_METHOD` should be added at the beginning of a function's declaration/implementation
 * to decorate any APIs that are directly invoked by Python's interpreter. The macro expands to
 * `extern "C"` to ensure C linkage.
 */
#define CLP_FFI_PY_METHOD extern "C"

#endif  // CLP_FFI_PY_API_DECORATION_HPP
