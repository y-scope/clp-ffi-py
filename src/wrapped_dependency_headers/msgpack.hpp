// IWYU pragma: begin_exports
#define PY_SSIZE_T_CLEAN
#include <msgpack.hpp>
// IWYU pragma: end_exports

// clang-format off
#ifdef CLP_FFI_PY_ENABLE_LINTING
// Inform IWYU of the headers that we use that are exported by msgpack.hpp
// IWYU pragma: begin_exports
// TODO: add necessary msgpack internal headers to silent clang-tidy warnings
// IWYU pragma: end_exports
#endif
// clang-format on
