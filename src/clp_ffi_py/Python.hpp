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
// Inform IWYU of the headers that we use that are exported by Python.h
// IWYU pragma: begin_exports
#include <abstract.h>
#include <boolobject.h>
#include <bytesobject.h>
#include <dictobject.h>
#include <floatobject.h>
#include <longobject.h>
#include <longobject.h>
#include <memoryobject.h>
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
