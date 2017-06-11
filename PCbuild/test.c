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
    printf("Python version: %s, hex: %08X\n", PY_VERSION, PY_VERSION_HEX); //patchlevel.h

    if (!_PyLong_Init())
        Py_FatalError("Py_Initialize: can't init longs");

    {
        PyObject *n1 = PyLong_FromLong(2);
        PyObject *res = long_as_number.nb_add(n1, n1);
        printf(">>> 2 + 2 \n%d\n", PyLong_AsLong(res));
    }

    {
        PyObject *n1 = PyLong_FromLong(50);
        PyObject *n2 = PyLong_FromLong(5);
        PyObject *n3 = PyLong_FromLong(6);
        PyObject *res = long_as_number.nb_subtract(n1,
                                                   long_as_number.nb_multiply(n2, n3));
        printf(">>> 50 - 5*6 \n%d\n", PyLong_AsLong(res));

        PyObject *n4 = PyLong_FromLong(4);
        PyObject *res2 = long_as_number.nb_true_divide(res, n4);
        printf(">>> (50 - 5*6) / 4 \n%f\n", PyFloat_AsDouble(res2));

        PyObject *n5 = PyLong_FromLong(8);
        PyObject *n6 = PyLong_FromLong(5);
        PyObject *res3 = long_as_number.nb_true_divide(n5, n6);
        printf(">>> 8 / 5 \n%f\n", PyFloat_AsDouble(res3));
    }

    {
        PyObject *n1 = PyLong_FromLong(17);
        PyObject *n2 = PyLong_FromLong(3);
        PyObject *res1 = long_as_number.nb_true_divide(n1, n2);
        printf(">>> 17 / 3 \n%f\n", PyFloat_AsDouble(res1));

        PyObject *res2 = long_as_number.nb_floor_divide(n1, n2);
        printf(">>> 17 // 3 \n%d\n", PyLong_AsLong(res2));

        PyObject *res3 = long_as_number.nb_remainder(n1, n2);
        printf(">>> 17 % 3 \n%d\n", PyLong_AsLong(res3));

        // 5 * 3 + 2
    }

    {
        PyObject *n1 = PyLong_FromLong(5);
        PyObject *n2 = PyLong_FromLong(2);
        PyObject *n3 = PyLong_FromLong(7);

        PyObject *res1 = long_as_number.nb_power(n1, n2, Py_None);
        PyObject *res2 = long_as_number.nb_power(n2, n3, Py_None);

        printf(">>> 5 ** 2 \n%d\n", PyLong_AsLong(res1));
        printf(">>> 2 ** 7 \n%d\n", PyLong_AsLong(res2));
    }

    {
        PyObject *n1 = PyFloat_FromDouble(3);
        PyObject *n2 = PyFloat_FromDouble(3.75);
        PyObject *n3 = PyFloat_FromDouble(1.5);
        PyObject *res = float_as_number.nb_true_divide(float_as_number.nb_multiply(n1, n2),
                                                        n3);
        printf(">>> 3 * 3.75 / 1.5 \n%f\n", PyFloat_AsDouble(res));
    }

    {
        PyObject *n1 = PyFloat_FromDouble(7.0);
        PyObject *n2 = PyFloat_FromDouble(2);
        PyObject *res = float_as_number.nb_true_divide(n1, n2);
        printf(">>> 7.0 / 2 \n%f\n", PyFloat_AsDouble(res));
    }

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

int Py_ISSPACE(char c)
{
    return isspace(c);
}

PyObject *
PyUnicode_FromStringAndSize(const char *u, Py_ssize_t size)
{
    return NULL;
}

PyObject *PyExc_OverflowError, *PyExc_TypeError, *PyExc_ValueError, *PyExc_ZeroDivisionError, *PyExc_DeprecationWarning;
PyObject *PyExc_IndexError;

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