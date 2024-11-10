/**
 * Python Header
 * Must be included before any other headers.
 * Check the following for reference:
 * 1. https://docs.python.org/3/c-api/intro.html#include-files
 * 2. https://docs.python.org/3/c-api/arg.html#strings-and-buffers
 */
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

// clang-format off
#ifdef CLP_FFI_PY_ENABLE_LINTING
// The following headers are added to export Python headers to get rid of clang-tidy warnings.
// IWYU pragma: begin_exports
#include <bytesobject.h>
#include <boolobject.h>
#include <dictobject.h>
#include <floatobject.h>
#include <longobject.h>
#include <methodobject.h>
#include <modsupport.h>
#include <object.h>
#include <objimpl.h>
#include <pyerrors.h>
#include <pymacro.h>
#include <pyport.h>
#include <typeslots.h>
#include <unicodeobject.h>
// IWYU pragma: end_exports
#endif
// clang-format on
