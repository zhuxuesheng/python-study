#include "Python.h"
#include <stdarg.h>
#include <ctype.h>

void
Py_FatalError(const char *msg)
{
    fprintf(stderr, "FATAL ERROR: %s\n", msg);
    exit(1);
}

extern PyNumberMethods long_as_number;
extern PyNumberMethods bool_as_number;
extern PyNumberMethods float_as_number;

const char *Py_hexdigits = "0123456789abcdef";

int main()
{
    #ifdef _DEBUG
    printf("This is debug version...\n");
    #endif
    printf("Python version: %s, hex: %08X\n", PY_VERSION, PY_VERSION_HEX); //patchlevel.h
    void *ptr = PyMem_Malloc(1024);
    PyMem_Free(ptr);
    
    if (!_PyLong_Init())
        Py_FatalError("Py_Initialize: can't init longs");
    
    PyObject *x, *y, *z;
    long result;
    x = PyLong_FromLong(500);
    y = PyLong_FromLong(60);
    z = long_as_number.nb_remainder(x, y);
    result = PyLong_AsLong(z);
    printf("1+2=%d\n", result);
    
    PyObject *t, *f, *zz;
    t = PyBool_FromLong(1);
    f = PyBool_FromLong(0);
    zz = bool_as_number.nb_and(t, f);
    printf("True and False is %s\n", (zz==Py_True)?("True"):("False"));
    
    PyObject *f1, *f2, *f3;
    f1 = PyFloat_FromDouble(10.5);
    f2 = PyFloat_FromDouble(15.4);
    f3 = float_as_number.nb_multiply(f1, f2);
    double d = PyFloat_AsDouble(f3);
    printf("10.5*15.4 = %f\n", d);
    
    return 0;
}

// missed functions
int
PyOS_snprintf(char *str, size_t size, const  char  *format, ...)
{
    int rc;
    va_list va;

    va_start(va, format);
    rc = vsnprintf(str, size, format, va);
    va_end(va);
    return rc;
}

PyObject *
PyErr_NoMemory(void)
{
    return NULL;
}

PyObject *
PyErr_Format(PyObject *exception, const char *format, ...)
{
    va_list va;
    
    va_start(va, format);
    printf(format, va);
    va_end(va);
    return NULL;
}

int
PyErr_WarnFormat(PyObject *category, Py_ssize_t stack_level,
                 const char *format, ...)
{
    va_list va;
    
    va_start(va, format);
    printf(format, va);
    va_end(va);
    return 0;
}

void
PyErr_SetString(PyObject *exception, const char *string)
{
    printf("Error: %s\n", string);
}

void
PyErr_BadInternalCall(void)
{
}

PyObject*
PyErr_Occurred()
{
    return 0;
}

int
PyErr_CheckSignals(void)
{
    return 0;
}

int
PyErr_BadArgument(void)
{
    return 0;
}

PyObject *
PyErr_SetFromErrno(PyObject *exc)
{
    return 0;
}

PyObject *
PyUnicode_FromStringAndSize(const char *u, Py_ssize_t size)
{
    return NULL;
}

PyObject *PyExc_OverflowError, *PyExc_TypeError, *PyExc_ValueError, *PyExc_ZeroDivisionError, *PyExc_DeprecationWarning;
PyObject *PyExc_IndexError, *PyExc_SystemError;

PyTypeObject PyType_Type; // wait type module

int
PyType_IsSubtype(PyTypeObject *a, PyTypeObject *b)
{
    printf("PyType_IsSubtype is not implemented yet, always return 1 ...\n");
    return 1;
}

PyObject *
Py_BuildValue(const char *format, ...)
{
    printf("Py_BuildValue is not supported...\n");
    return NULL;
}

PyVarObject *
_PyObject_GC_NewVar(PyTypeObject *tp, Py_ssize_t nitems)
{
    size_t size;
    PyVarObject *op;

    if (nitems < 0) {
        PyErr_BadInternalCall();
        return NULL;
    }
    size = _PyObject_VAR_SIZE(tp, nitems);
    op = (PyVarObject *) PyObject_Malloc(size);
    if (op != NULL)
        op = PyObject_INIT_VAR(op, tp, nitems);
    return op;
}