/**
 * Python Header
 * Must be included before any other headers.
 * Check the following for reference:
 * 1. https://docs.python.org/3/c-api/intro.html#include-files
 * 2. https://docs.python.org/3/c-api/arg.html#strings-and-buffers
 */
// IWYU pragma: begin_exports
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
// IWYU pragma: end_exports

// clang-format off
#ifdef CLP_FFI_PY_ENABLE_LINTING
// Inform IWYU of the headers that we use that are exported by Python.h
// IWYU pragma: begin_exports
#include <abstract.h>
#include <boolobject.h>
#include <bytearrayobject.h>
#include <bytesobject.h>
#include <dictobject.h>
#include <floatobject.h>
#include <import.h>
#include <listobject.h>
#include <longobject.h>
#include <memoryobject.h>
#include <methodobject.h>
#include <modsupport.h>
#include <moduleobject.h>
#include <object.h>
#include <objimpl.h>
#include <pyerrors.h>
#include <pymacro.h>
#include <pymem.h>
#include <pyport.h>
#include <tupleobject.h>
#include <typeslots.h>
#include <unicodeobject.h>
#include <warnings.h>
// IWYU pragma: end_exports
#endif
// clang-format on
