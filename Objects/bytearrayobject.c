/* PyByteArray (bytearray) implementation */

#define PY_SSIZE_T_CLEAN
#include "Python.h"
#include "structmember.h"
#include "bytes_methods.h"
#include "bytesobject.h"
#include "pystrhex.h"

/*[clinic input]
class bytearray "PyByteArrayObject *" "&PyByteArray_Type"
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=5535b77c37a119e0]*/

char _PyByteArray_empty_string[] = "";

void
PyByteArray_Fini(void)
{
}

int
PyByteArray_Init(void)
{
    return 1;
}

/* end nullbytes support */

/* Helpers */

static int
_getbytevalue(PyObject* arg, int *value)
{
    long face_value;

    if (PyLong_Check(arg)) {
        face_value = PyLong_AsLong(arg);
    } else {
        PyObject *index = PyNumber_Index(arg);
        if (index == NULL) {
            PyErr_Format(PyExc_TypeError, "an integer is required");
            *value = -1;
            return 0;
        }
        face_value = PyLong_AsLong(index);
        Py_DECREF(index);
    }

    if (face_value < 0 || face_value >= 256) {
        /* this includes the OverflowError in case the long is too large */
        PyErr_SetString(PyExc_ValueError, "byte must be in range(0, 256)");
        *value = -1;
        return 0;
    }

    *value = face_value;
    return 1;
}

static int
bytearray_getbuffer(PyByteArrayObject *obj, Py_buffer *view, int flags)
{
    void *ptr;
    if (view == NULL) {
        PyErr_SetString(PyExc_BufferError,
            "bytearray_getbuffer: view==NULL argument is obsolete");
        return -1;
    }
    ptr = (void *) PyByteArray_AS_STRING(obj);
    /* cannot fail if view != NULL and readonly == 0 */
    (void)PyBuffer_FillInfo(view, (PyObject*)obj, ptr, Py_SIZE(obj), 0, flags);
    obj->ob_exports++;
    return 0;
}

static void
bytearray_releasebuffer(PyByteArrayObject *obj, Py_buffer *view)
{
    obj->ob_exports--;
}

static int
_canresize(PyByteArrayObject *self)
{
    if (self->ob_exports > 0) {
        PyErr_SetString(PyExc_BufferError,
                "Existing exports of data: object cannot be re-sized");
        return 0;
    }
    return 1;
}

//#include "clinic/bytearrayobject.c.h"

/* Direct API functions */

PyObject *
PyByteArray_FromObject(PyObject *input)
{
    return PyObject_CallFunctionObjArgs((PyObject *)&PyByteArray_Type,
                                        input, NULL);
}

PyObject *
PyByteArray_FromStringAndSize(const char *bytes, Py_ssize_t size)
{
    PyByteArrayObject *new;
    Py_ssize_t alloc;

    if (size < 0) {
        PyErr_SetString(PyExc_SystemError,
            "Negative size passed to PyByteArray_FromStringAndSize");
        return NULL;
    }

    /* Prevent buffer overflow when setting alloc to size+1. */
    if (size == PY_SSIZE_T_MAX) {
        return PyErr_NoMemory();
    }

    new = PyObject_New(PyByteArrayObject, &PyByteArray_Type);
    if (new == NULL)
        return NULL;

    if (size == 0) {
        new->ob_bytes = NULL;
        alloc = 0;
    }
    else {
        alloc = size + 1;
        new->ob_bytes = PyObject_Malloc(alloc);
        if (new->ob_bytes == NULL) {
            Py_DECREF(new);
            return PyErr_NoMemory();
        }
        if (bytes != NULL && size > 0)
            memcpy(new->ob_bytes, bytes, size);
        new->ob_bytes[size] = '\0';  /* Trailing null byte */
    }
    Py_SIZE(new) = size;
    new->ob_alloc = alloc;
    new->ob_start = new->ob_bytes;
    new->ob_exports = 0;

    return (PyObject *)new;
}

Py_ssize_t
PyByteArray_Size(PyObject *self)
{
    assert(self != NULL);
    assert(PyByteArray_Check(self));

    return PyByteArray_GET_SIZE(self);
}

char  *
PyByteArray_AsString(PyObject *self)
{
    assert(self != NULL);
    assert(PyByteArray_Check(self));

    return PyByteArray_AS_STRING(self);
}

int
PyByteArray_Resize(PyObject *self, Py_ssize_t requested_size)
{
    void *sval;
    PyByteArrayObject *obj = ((PyByteArrayObject *)self);
    /* All computations are done unsigned to avoid integer overflows
       (see issue #22335). */
    size_t alloc = (size_t) obj->ob_alloc;
    size_t logical_offset = (size_t) (obj->ob_start - obj->ob_bytes);
    size_t size = (size_t) requested_size;

    assert(self != NULL);
    assert(PyByteArray_Check(self));
    assert(logical_offset <= alloc);
    assert(requested_size >= 0);

    if (requested_size == Py_SIZE(self)) {
        return 0;
    }
    if (!_canresize(obj)) {
        return -1;
    }

    if (size + logical_offset + 1 <= alloc) {
        /* Current buffer is large enough to host the requested size,
           decide on a strategy. */
        if (size < alloc / 2) {
            /* Major downsize; resize down to exact size */
            alloc = size + 1;
        }
        else {
            /* Minor downsize; quick exit */
            Py_SIZE(self) = size;
            PyByteArray_AS_STRING(self)[size] = '\0'; /* Trailing null */
            return 0;
        }
    }
    else {
        /* Need growing, decide on a strategy */
        if (size <= alloc * 1.125) {
            /* Moderate upsize; overallocate similar to list_resize() */
            alloc = size + (size >> 3) + (size < 9 ? 3 : 6);
        }
        else {
            /* Major upsize; resize up to exact size */
            alloc = size + 1;
        }
    }
    if (alloc > PY_SSIZE_T_MAX) {
        PyErr_NoMemory();
        return -1;
    }

    if (logical_offset > 0) {
        sval = PyObject_Malloc(alloc);
        if (sval == NULL) {
            PyErr_NoMemory();
            return -1;
        }
        memcpy(sval, PyByteArray_AS_STRING(self),
               Py_MIN(requested_size, Py_SIZE(self)));
        PyObject_Free(obj->ob_bytes);
    }
    else {
        sval = PyObject_Realloc(obj->ob_bytes, alloc);
        if (sval == NULL) {
            PyErr_NoMemory();
            return -1;
        }
    }

    obj->ob_bytes = obj->ob_start = sval;
    Py_SIZE(self) = size;
    obj->ob_alloc = alloc;
    obj->ob_bytes[size] = '\0'; /* Trailing null byte */

    return 0;
}

PyObject *
PyByteArray_Concat(PyObject *a, PyObject *b)
{
    Py_ssize_t size;
    Py_buffer va, vb;
    PyByteArrayObject *result = NULL;

    va.len = -1;
    vb.len = -1;
    if (PyObject_GetBuffer(a, &va, PyBUF_SIMPLE) != 0 ||
        PyObject_GetBuffer(b, &vb, PyBUF_SIMPLE) != 0) {
            PyErr_Format(PyExc_TypeError, "can't concat %.100s to %.100s",
                         Py_TYPE(a)->tp_name, Py_TYPE(b)->tp_name);
            goto done;
    }

    size = va.len + vb.len;
    if (size < 0) {
            PyErr_NoMemory();
            goto done;
    }

    result = (PyByteArrayObject *) PyByteArray_FromStringAndSize(NULL, size);
    if (result != NULL) {
        memcpy(result->ob_bytes, va.buf, va.len);
        memcpy(result->ob_bytes + va.len, vb.buf, vb.len);
    }

  done:
    if (va.len != -1)
        PyBuffer_Release(&va);
    if (vb.len != -1)
        PyBuffer_Release(&vb);
    return (PyObject *)result;
}

static PyObject *
bytearray_format(PyByteArrayObject *self, PyObject *args)
{
    PyObject *bytes_in, *bytes_out, *res;
    char *bytestring;

    if (self == NULL || !PyByteArray_Check(self) || args == NULL) {
        PyErr_BadInternalCall();
        return NULL;
    }
    bytestring = PyByteArray_AS_STRING(self);
    bytes_in = PyBytes_FromString(bytestring);
    if (bytes_in == NULL)
        return NULL;
    bytes_out = _PyBytes_Format(bytes_in, args);
    Py_DECREF(bytes_in);
    if (bytes_out == NULL)
        return NULL;
    res = PyByteArray_FromObject(bytes_out);
    Py_DECREF(bytes_out);
    if (res == NULL)
        return NULL;
    return res;
}


static PyNumberMethods bytearray_as_number = {
    0,              /*nb_add*/
    0,              /*nb_subtract*/
    0,              /*nb_multiply*/
    0,  /*nb_remainder*/
};


PyTypeObject PyByteArray_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "bytearray",
    sizeof(PyByteArrayObject),
    0,
    0,       /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_reserved */
    0,           /* tp_repr */
    &bytearray_as_number,               /* tp_as_number */
    0,             /* tp_as_sequence */
    0,              /* tp_as_mapping */
    0,                                  /* tp_hash */
    0,                                  /* tp_call */
    0,                      /* tp_str */
    0,            /* tp_getattro */
    0,                                  /* tp_setattro */
    0,               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                      /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0, /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                                  /* tp_iternext */
    0,                  /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0,           /* tp_init */
    0,                /* tp_alloc */
    0,                  /* tp_new */
    PyObject_Del,                       /* tp_free */
};
