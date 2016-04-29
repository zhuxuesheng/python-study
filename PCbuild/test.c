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
    z = long_as_number.nb_multiply(x, y);
    result = PyLong_AsLong(z);
    printf("1+2=%d\n", result);
    
    PyObject *t, *f, *zz;
    t = PyBool_FromLong(1);
    f = PyBool_FromLong(0);
    zz = bool_as_number.nb_and(x, y);
    result = PyLong_AsLong(zz);
    printf("True and False is %d\n", result);
    
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

int Py_ISSPACE(char c)
{
    return isspace(c);
}

PyObject *
PyUnicode_FromStringAndSize(const char *u, Py_ssize_t size)
{
    return NULL;
}

PyObject *
PyTuple_New(Py_ssize_t size)
{
    return NULL;
}

int
PyTuple_SetItem(PyObject *op, Py_ssize_t i, PyObject *newitem)
{
    return 0;
}

PyObject *PyExc_OverflowError, *PyExc_TypeError, *PyExc_ValueError, *PyExc_ZeroDivisionError, *PyExc_DeprecationWarning;

PyObject *
Py_BuildValue(const char *format, ...)
{
    printf("Py_BuildValue is not supported...\n");
    return NULL;
}