#ifndef Py_ERRORS_H
#define Py_ERRORS_H
#ifdef __cplusplus
extern "C" {
#endif



/* Predefined exceptions */

PyAPI_DATA(PyObject *) PyExc_BaseException;
PyAPI_DATA(PyObject *) PyExc_Exception;
PyAPI_DATA(PyObject *) PyExc_StopAsyncIteration;
PyAPI_DATA(PyObject *) PyExc_StopIteration;
PyAPI_DATA(PyObject *) PyExc_GeneratorExit;
PyAPI_DATA(PyObject *) PyExc_ArithmeticError;
PyAPI_DATA(PyObject *) PyExc_LookupError;

PyAPI_DATA(PyObject *) PyExc_AssertionError;
PyAPI_DATA(PyObject *) PyExc_AttributeError;
PyAPI_DATA(PyObject *) PyExc_BufferError;
PyAPI_DATA(PyObject *) PyExc_EOFError;
PyAPI_DATA(PyObject *) PyExc_FloatingPointError;
PyAPI_DATA(PyObject *) PyExc_OSError;
PyAPI_DATA(PyObject *) PyExc_ImportError;
PyAPI_DATA(PyObject *) PyExc_IndexError;
PyAPI_DATA(PyObject *) PyExc_KeyError;
PyAPI_DATA(PyObject *) PyExc_KeyboardInterrupt;
PyAPI_DATA(PyObject *) PyExc_MemoryError;
PyAPI_DATA(PyObject *) PyExc_NameError;
PyAPI_DATA(PyObject *) PyExc_OverflowError;
PyAPI_DATA(PyObject *) PyExc_RuntimeError;
PyAPI_DATA(PyObject *) PyExc_RecursionError;
PyAPI_DATA(PyObject *) PyExc_NotImplementedError;
PyAPI_DATA(PyObject *) PyExc_SyntaxError;
PyAPI_DATA(PyObject *) PyExc_IndentationError;
PyAPI_DATA(PyObject *) PyExc_TabError;
PyAPI_DATA(PyObject *) PyExc_ReferenceError;
PyAPI_DATA(PyObject *) PyExc_SystemError;
PyAPI_DATA(PyObject *) PyExc_SystemExit;
PyAPI_DATA(PyObject *) PyExc_TypeError;
PyAPI_DATA(PyObject *) PyExc_UnboundLocalError;
PyAPI_DATA(PyObject *) PyExc_UnicodeError;
PyAPI_DATA(PyObject *) PyExc_UnicodeEncodeError;
PyAPI_DATA(PyObject *) PyExc_UnicodeDecodeError;
PyAPI_DATA(PyObject *) PyExc_UnicodeTranslateError;
PyAPI_DATA(PyObject *) PyExc_ValueError;
PyAPI_DATA(PyObject *) PyExc_ZeroDivisionError;

PyAPI_DATA(PyObject *) PyExc_BlockingIOError;
PyAPI_DATA(PyObject *) PyExc_BrokenPipeError;
PyAPI_DATA(PyObject *) PyExc_ChildProcessError;
PyAPI_DATA(PyObject *) PyExc_ConnectionError;
PyAPI_DATA(PyObject *) PyExc_ConnectionAbortedError;
PyAPI_DATA(PyObject *) PyExc_ConnectionRefusedError;
PyAPI_DATA(PyObject *) PyExc_ConnectionResetError;
PyAPI_DATA(PyObject *) PyExc_FileExistsError;
PyAPI_DATA(PyObject *) PyExc_FileNotFoundError;
PyAPI_DATA(PyObject *) PyExc_InterruptedError;
PyAPI_DATA(PyObject *) PyExc_IsADirectoryError;
PyAPI_DATA(PyObject *) PyExc_NotADirectoryError;
PyAPI_DATA(PyObject *) PyExc_PermissionError;
PyAPI_DATA(PyObject *) PyExc_ProcessLookupError;
PyAPI_DATA(PyObject *) PyExc_TimeoutError;


/* Compatibility aliases */
PyAPI_DATA(PyObject *) PyExc_EnvironmentError;
PyAPI_DATA(PyObject *) PyExc_IOError;
#ifdef MS_WINDOWS
PyAPI_DATA(PyObject *) PyExc_WindowsError;
#endif

PyAPI_DATA(PyObject *) PyExc_RecursionErrorInst;

/* Predefined warning categories */
PyAPI_DATA(PyObject *) PyExc_Warning;
PyAPI_DATA(PyObject *) PyExc_UserWarning;
PyAPI_DATA(PyObject *) PyExc_DeprecationWarning;
PyAPI_DATA(PyObject *) PyExc_PendingDeprecationWarning;
PyAPI_DATA(PyObject *) PyExc_SyntaxWarning;
PyAPI_DATA(PyObject *) PyExc_RuntimeWarning;
PyAPI_DATA(PyObject *) PyExc_FutureWarning;
PyAPI_DATA(PyObject *) PyExc_ImportWarning;
PyAPI_DATA(PyObject *) PyExc_UnicodeWarning;
PyAPI_DATA(PyObject *) PyExc_BytesWarning;
PyAPI_DATA(PyObject *) PyExc_ResourceWarning;



/* These APIs aren't really part of the error implementation, but
   often needed to format error messages; the native C lib APIs are
   not available on all platforms, which is why we provide emulations
   for those platforms in Python/mysnprintf.c,
   WARNING:  The return value of snprintf varies across platforms; do
   not rely on any particular behavior; eventually the C99 defn may
   be reliable.
*/
#if defined(MS_WIN32) && !defined(HAVE_SNPRINTF)
# define HAVE_SNPRINTF
# define snprintf _snprintf
# define vsnprintf _vsnprintf
#endif

#include <stdarg.h>
PyAPI_FUNC(int) PyOS_snprintf(char *str, size_t size, const char  *format, ...)
                        Py_GCC_ATTRIBUTE((format(printf, 3, 4)));
PyAPI_FUNC(int) PyOS_vsnprintf(char *str, size_t size, const char  *format, va_list va)
                        Py_GCC_ATTRIBUTE((format(printf, 3, 0)));

#ifdef __cplusplus
}
#endif
#endif /* !Py_ERRORS_H */
