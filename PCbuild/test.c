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

void
PyErr_SetObject(PyObject *exception, PyObject *value)
{
}

int
PyErr_ExceptionMatches(PyObject *exc)
{
}

void
PyErr_Clear(void)
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
PyObject *PyExc_IndexError, *PyExc_SystemError, *PyExc_BufferError, *PyExc_StopIteration;

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

PyObject *
Py_BuildValue(const char *format, ...)
{
    printf("Py_BuildValue is not supported...\n");
    return NULL;
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

int
PyObject_GetBuffer(PyObject *obj, Py_buffer *view, int flags)
{
    PyBufferProcs *pb = obj->ob_type->tp_as_buffer;

    if (pb == NULL || pb->bf_getbuffer == NULL) {
        PyErr_Format(PyExc_TypeError,
                     "a bytes-like object is required, not '%.100s'",
                     Py_TYPE(obj)->tp_name);
        return -1;
    }
    return (*pb->bf_getbuffer)(obj, view, flags);
}

void
PyBuffer_Release(Py_buffer *view)
{
    PyObject *obj = view->obj;
    PyBufferProcs *pb;
    if (obj == NULL)
        return;
    pb = Py_TYPE(obj)->tp_as_buffer;
    if (pb && pb->bf_releasebuffer)
        pb->bf_releasebuffer(obj, view);
    view->obj = NULL;
    Py_DECREF(obj);
}

PyObject *
PyObject_CallFunctionObjArgs(PyObject *callable, ...)
{
    return NULL;
}

PyObject *
_PyObject_LookupSpecial(PyObject *self, _Py_Identifier *attrid)
{
    return NULL;
}

PyObject *
PyObject_GetIter(PyObject *o)
{    
}

Py_ssize_t
PyObject_LengthHint(PyObject *o, Py_ssize_t defaultvalue)
{
}

Py_ssize_t
PyDict_Size(PyObject *mp)
{
    return -1;
}

PyObject *
PySequence_Fast(PyObject *v, const char *m)
{
}

Py_ssize_t
PyNumber_AsSsize_t(PyObject *item, PyObject *err)
{
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

int PySlice_Check(PyObject *x)
{
    return 0;
}
int
PySlice_GetIndicesEx(PyObject *_r, Py_ssize_t length,
                     Py_ssize_t *start, Py_ssize_t *stop, Py_ssize_t *step,
                     Py_ssize_t *slicelength)
{
}

int
_PyEval_SliceIndex(PyObject *v, Py_ssize_t *pi)
{
}
