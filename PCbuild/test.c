#include "Python.h"
#include <stdarg.h>
#include <ctype.h>

void
Py_FatalError(const char *msg)
{
    fprintf(stderr, "FATAL ERROR: %s\n", msg);
    exit(1);
}

int Py_VerboseFlag;
int Py_BytesWarningFlag;

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
    z = PyLong_Type.tp_as_number->nb_remainder(x, y);
    result = PyLong_AsLong(z);
    printf("1+2=%d\n", result);
    
    PyObject *t, *f, *zz;
    t = PyBool_FromLong(1);
    f = PyBool_FromLong(0);
    zz = PyBool_Type.tp_as_number->nb_and(t, f);
    printf("True and False is %s\n", (zz==Py_True)?("True"):("False"));
    
    PyObject *f1, *f2, *f3;
    f1 = PyFloat_FromDouble(10.5);
    f2 = PyFloat_FromDouble(15.4);
    f3 = PyFloat_Type.tp_as_number->nb_multiply(f1, f2);
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
_PySys_GetObjectId(_Py_Identifier *key)
{
}

void
PySys_WriteStderr(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    vfprintf(stderr, format, va);
    va_end(va);
}

void
PySys_FormatStderr(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    //sys_format(&PyId_stderr, stderr, format, va);
    va_end(va);
}

int
PyErr_CheckSignals(void)
{
    return 0;
}

void
PyErr_Print(void)
{
}

PyObject *PyExc_OverflowError, *PyExc_TypeError, *PyExc_ValueError, *PyExc_ZeroDivisionError, *PyExc_DeprecationWarning;
PyObject *PyExc_IndexError, *PyExc_SystemError, *PyExc_BufferError, *PyExc_StopIteration, *PyExc_AttributeError;
PyObject *PyExc_KeyError, *PyExc_MemoryError, *PyExc_RuntimeError, *PyExc_ImportError, *PyExc_NotImplementedError;
PyObject *PyExc_RuntimeWarning;

PyGC_Head *_PyGC_generation0;

void
PyObject_GC_Track(void *op)
{
    _PyObject_GC_TRACK(op);
}

void
PyObject_GC_UnTrack(void *op)
{
    /* Obscure:  the Py_TRASHCAN mechanism requires that we be able to
     * call PyObject_GC_UnTrack twice on an object.
     */
    //if (IS_TRACKED(op))
        _PyObject_GC_UNTRACK(op);
}

PyObject *
_PyObject_GC_New(PyTypeObject *tp)
{
    PyObject *op = PyObject_Malloc(_PyObject_SIZE(tp));
    if (op != NULL)
        op = PyObject_INIT(op, tp);
    return op;
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

PyObject *
_PyObject_GC_Malloc(size_t basicsize)
{
    PyObject_Malloc(basicsize);
}

void
PyObject_GC_Del(void *op)
{
    PyObject_FREE(op);
}

PyVarObject *
_PyObject_GC_Resize(PyVarObject *op, Py_ssize_t nitems)
{
}

PyObject *
_Py_strhex(const char* argbuf, const Py_ssize_t arglen)
{
}

PyObject *
PyImport_ImportModule(const char *name)
{
}

PyObject *
PyImport_Import(PyObject *module_name)
{
}

int
_PyComplex_FormatAdvancedWriter(_PyUnicodeWriter *writer,
                                PyObject *obj,
                                PyObject *format_spec,
                                Py_ssize_t start, Py_ssize_t end)
{
}

int
_PyLong_FormatAdvancedWriter(_PyUnicodeWriter *writer,
                             PyObject *obj,
                             PyObject *format_spec,
                             Py_ssize_t start, Py_ssize_t end)
{
}

int
_PyFloat_FormatAdvancedWriter(_PyUnicodeWriter *writer,
                              PyObject *obj,
                              PyObject *format_spec,
                              Py_ssize_t start, Py_ssize_t end)
{
}

int
_PyUnicode_FormatAdvancedWriter(_PyUnicodeWriter *writer,
                                PyObject *obj,
                                PyObject *format_spec,
                                Py_ssize_t start, Py_ssize_t end)
{
}

char *
PyTokenizer_FindEncodingFilename(int fd, PyObject *filename)
{
}

PyObject *
PySys_GetObject(const char *name)
{
}

PyObject *
PyImport_ImportModuleNoBlock(const char *name)
{
    return NULL;
}

PyObject *
PyImport_GetModuleDict(void)
{
}

PyObject *
_Py_Mangle(PyObject *privateobj, PyObject *ident)
{
}
