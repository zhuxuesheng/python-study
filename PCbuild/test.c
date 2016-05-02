#include "Python.h"
#include <stdarg.h>
#include <ctype.h>

void
Py_FatalError(const char *msg)
{
    fprintf(stderr, "FATAL ERROR: %s\n", msg);
    exit(1);
}

const char *Py_hexdigits = "0123456789abcdef";
int Py_VerboseFlag;

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

void
_PyErr_SetKeyError(PyObject *arg)
{
}

void
_PyErr_ChainExceptions(PyObject *exc, PyObject *val, PyObject *tb)
{
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

void
PyErr_SetObject(PyObject *exception, PyObject *value)
{
}

int
PyErr_ExceptionMatches(PyObject *exc)
{
}

int
PyErr_GivenExceptionMatches(PyObject *err, PyObject *exc)
{
    return err == exc;
}

void
PyErr_Fetch(PyObject **p_type, PyObject **p_value, PyObject **p_traceback)
{
}

void
PyErr_Clear(void)
{
}

void
PyErr_Restore(PyObject *type, PyObject *value, PyObject *traceback)
{
}

PyObject *
PyUnicode_New(Py_ssize_t size, Py_UCS4 maxchar)
{
}

PyObject *
PyUnicode_FromString(const char *u)
{
    return NULL;
}

PyObject *
PyUnicode_FromStringAndSize(const char *u, Py_ssize_t size)
{
    return NULL;
}

PyObject *PyExc_OverflowError, *PyExc_TypeError, *PyExc_ValueError, *PyExc_ZeroDivisionError, *PyExc_DeprecationWarning;
PyObject *PyExc_IndexError, *PyExc_SystemError, *PyExc_BufferError, *PyExc_StopIteration, *PyExc_AttributeError;
PyObject *PyExc_KeyError, *PyExc_MemoryError, *PyExc_RuntimeError, *PyExc_ImportError, *PyExc_NotImplementedError;
PyObject *PyExc_RuntimeWarning;

PyTypeObject PyType_Type; // wait type module
PyGC_Head *_PyGC_generation0;

int
PyType_IsSubtype(PyTypeObject *a, PyTypeObject *b)
{
    printf("PyType_IsSubtype is not implemented yet, always return 1 ...\n");
    return 1;
}

PyObject *
PyType_GenericAlloc(PyTypeObject *type, Py_ssize_t nitems)
{
}

PyObject *
PyType_GenericNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    return type->tp_alloc(type, 0);
}

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
_PyObject_LookupSpecial(PyObject *self, _Py_Identifier *attrid)
{
    return NULL;
}

int
PyArg_ParseTuple(PyObject *args, const char *format, ...)
{
}

int
PyArg_ParseTupleAndKeywords(PyObject *args,
                            PyObject *keywords,
                            const char *format,
                            char **kwlist, ...)
{
}

int
PyArg_UnpackTuple(PyObject *args, const char *name, Py_ssize_t min, Py_ssize_t max, ...)
{
}

int
PyArg_ValidateKeywordArguments(PyObject *kwargs)
{
}

int
_PyArg_NoKeywords(const char *funcname, PyObject *kw)
{
}

int
_PyEval_SliceIndex(PyObject *v, Py_ssize_t *pi)
{
}

PyObject *
PyEval_CallObjectWithKeywords(PyObject *func, PyObject *arg, PyObject *kw)
{
}

PyObject *
PyEval_GetLocals(void)
{
}

PyObject *
PySeqIter_New(PyObject *seq)
{
}

PyTypeObject PyUnicode_Type;

int
_PyUnicode_Ready(PyObject *unicode)
{
}

PyObject *
PyUnicode_FromFormat(const char *format, ...)
{
}

void
PyUnicode_InternInPlace(PyObject **p)
{
}

PyObject *
_PyUnicode_FromId(_Py_Identifier *id)
{
}

PyObject*
PyUnicode_Substring(PyObject *self, Py_ssize_t start, Py_ssize_t end)
{
}

PyObject *
PyUnicode_AsASCIIString(PyObject *unicode)
{
}

PyObject *
PyUnicode_InternFromString(const char *cp)
{
}

void
_PyUnicodeWriter_Init(_PyUnicodeWriter *writer)
{
}

PyObject *
_PyUnicodeWriter_Finish(_PyUnicodeWriter *writer)
{
}

void
_PyUnicodeWriter_Dealloc(_PyUnicodeWriter *writer)
{
    Py_CLEAR(writer->buffer);
}

int
_PyUnicodeWriter_WriteChar(_PyUnicodeWriter *writer, Py_UCS4 ch)
{
}

int
_PyUnicodeWriter_WriteStr(_PyUnicodeWriter *writer, PyObject *str)
{
}

int
_PyUnicodeWriter_WriteASCIIString(_PyUnicodeWriter *writer,
                                  const char *ascii, Py_ssize_t len)
{
}

char*
PyUnicode_AsUTF8(PyObject *unicode)
{
}

char*
PyUnicode_AsUTF8AndSize(PyObject *unicode, Py_ssize_t *psize)
{
}

PyObject *
_PyUnicode_TransformDecimalAndSpaceToASCII(PyObject *unicode)
{
}

PyObject*
_PyUnicode_FromASCII(const char *buffer, Py_ssize_t size)
{
}

int
PyUnicode_CompareWithASCIIString(PyObject* uni, const char* str)
{
}

PyObject *
PyUnicode_FromUnicode(const Py_UNICODE *u, Py_ssize_t size)
{
}

PyObject *
PyUnicode_FromOrdinal(int ordinal)
{
}

size_t
Py_UNICODE_strlen(const Py_UNICODE *u)
{
    int res = 0;
    while(*u++)
        res++;
    return res;
}

void
PyObject_ClearWeakRefs(PyObject *object)
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
PyType_Ready(PyTypeObject *type)
{
}

PyObject *
_PyType_Lookup(PyTypeObject *type, PyObject *name)
{
}

PyObject *
PyCFunction_NewEx(PyMethodDef *ml, PyObject *self, PyObject *module)
{
}
