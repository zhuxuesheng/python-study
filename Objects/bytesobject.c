/* bytes object implementation */

#define PY_SSIZE_T_CLEAN

#include "Python.h"

#include "bytes_methods.h"
#include "pystrhex.h"
#include <stddef.h>

/*[clinic input]
class bytes "PyBytesObject*" "&PyBytes_Type"
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=1a1d9102afc1b00c]*/

//#include "clinic/bytesobject.c.h"

#ifdef COUNT_ALLOCS
Py_ssize_t null_strings, one_strings;
#endif

static PyBytesObject *characters[UCHAR_MAX + 1];
static PyBytesObject *nullstring;

/* PyBytesObject_SIZE gives the basic size of a string; any memory allocation
   for a string of length n should request PyBytesObject_SIZE + n bytes.

   Using PyBytesObject_SIZE instead of sizeof(PyBytesObject) saves
   3 bytes per string allocation on a typical system.
*/
#define PyBytesObject_SIZE (offsetof(PyBytesObject, ob_sval) + 1)

/*
   For PyBytes_FromString(), the parameter `str' points to a null-terminated
   string containing exactly `size' bytes.

   For PyBytes_FromStringAndSize(), the parameter the parameter `str' is
   either NULL or else points to a string containing at least `size' bytes.
   For PyBytes_FromStringAndSize(), the string in the `str' parameter does
   not have to be null-terminated.  (Therefore it is safe to construct a
   substring by calling `PyBytes_FromStringAndSize(origstring, substrlen)'.)
   If `str' is NULL then PyBytes_FromStringAndSize() will allocate `size+1'
   bytes (setting the last byte to the null terminating character) and you can
   fill in the data yourself.  If `str' is non-NULL then the resulting
   PyBytes object must be treated as immutable and you must not fill in nor
   alter the data yourself, since the strings may be shared.

   The PyObject member `op->ob_size', which denotes the number of "extra
   items" in a variable-size object, will contain the number of bytes
   allocated for string data, not counting the null terminating character.
   It is therefore equal to the `size' parameter (for
   PyBytes_FromStringAndSize()) or the length of the string in the `str'
   parameter (for PyBytes_FromString()).
*/
static PyObject *
_PyBytes_FromSize(Py_ssize_t size, int use_calloc)
{
    PyBytesObject *op;
    assert(size >= 0);

    if (size == 0 && (op = nullstring) != NULL) {
#ifdef COUNT_ALLOCS
        null_strings++;
#endif
        Py_INCREF(op);
        return (PyObject *)op;
    }

    if ((size_t)size > (size_t)PY_SSIZE_T_MAX - PyBytesObject_SIZE) {
        PyErr_SetString(PyExc_OverflowError,
                        "byte string is too large");
        return NULL;
    }

    /* Inline PyObject_NewVar */
    if (use_calloc)
        op = (PyBytesObject *)PyObject_Calloc(1, PyBytesObject_SIZE + size);
    else
        op = (PyBytesObject *)PyObject_Malloc(PyBytesObject_SIZE + size);
    if (op == NULL)
        return PyErr_NoMemory();
    (void)PyObject_INIT_VAR(op, &PyBytes_Type, size);
    op->ob_shash = -1;
    if (!use_calloc)
        op->ob_sval[size] = '\0';
    /* empty byte string singleton */
    if (size == 0) {
        nullstring = op;
        Py_INCREF(op);
    }
    return (PyObject *) op;
}

PyObject *
PyBytes_FromStringAndSize(const char *str, Py_ssize_t size)
{
    PyBytesObject *op;
    if (size < 0) {
        PyErr_SetString(PyExc_SystemError,
            "Negative size passed to PyBytes_FromStringAndSize");
        return NULL;
    }
    if (size == 1 && str != NULL &&
        (op = characters[*str & UCHAR_MAX]) != NULL)
    {
#ifdef COUNT_ALLOCS
        one_strings++;
#endif
        Py_INCREF(op);
        return (PyObject *)op;
    }

    op = (PyBytesObject *)_PyBytes_FromSize(size, 0);
    if (op == NULL)
        return NULL;
    if (str == NULL)
        return (PyObject *) op;

    Py_MEMCPY(op->ob_sval, str, size);
    /* share short strings */
    if (size == 1) {
        characters[*str & UCHAR_MAX] = op;
        Py_INCREF(op);
    }
    return (PyObject *) op;
}

PyObject *
PyBytes_FromString(const char *str)
{
    size_t size;
    PyBytesObject *op;

    assert(str != NULL);
    size = strlen(str);
    if (size > PY_SSIZE_T_MAX - PyBytesObject_SIZE) {
        PyErr_SetString(PyExc_OverflowError,
            "byte string is too long");
        return NULL;
    }
    if (size == 0 && (op = nullstring) != NULL) {
#ifdef COUNT_ALLOCS
        null_strings++;
#endif
        Py_INCREF(op);
        return (PyObject *)op;
    }
    if (size == 1 && (op = characters[*str & UCHAR_MAX]) != NULL) {
#ifdef COUNT_ALLOCS
        one_strings++;
#endif
        Py_INCREF(op);
        return (PyObject *)op;
    }

    /* Inline PyObject_NewVar */
    op = (PyBytesObject *)PyObject_MALLOC(PyBytesObject_SIZE + size);
    if (op == NULL)
        return PyErr_NoMemory();
    (void)PyObject_INIT_VAR(op, &PyBytes_Type, size);
    op->ob_shash = -1;
    Py_MEMCPY(op->ob_sval, str, size+1);
    /* share short strings */
    if (size == 0) {
        nullstring = op;
        Py_INCREF(op);
    } else if (size == 1) {
        characters[*str & UCHAR_MAX] = op;
        Py_INCREF(op);
    }
    return (PyObject *) op;
}

PyObject *
PyBytes_FromFormatV(const char *format, va_list vargs)
{
    va_list count;
    Py_ssize_t n = 0;
    const char* f;
    char *s;
    PyObject* string;

    Py_VA_COPY(count, vargs);
    /* step 1: figure out how large a buffer we need */
    for (f = format; *f; f++) {
        if (*f == '%') {
            const char* p = f;
            while (*++f && *f != '%' && !Py_ISALPHA(*f))
                ;

            /* skip the 'l' or 'z' in {%ld, %zd, %lu, %zu} since
             * they don't affect the amount of space we reserve.
             */
            if ((*f == 'l' || *f == 'z') &&
                            (f[1] == 'd' || f[1] == 'u'))
                ++f;

            switch (*f) {
            case 'c':
            {
                int c = va_arg(count, int);
                if (c < 0 || c > 255) {
                    PyErr_SetString(PyExc_OverflowError,
                                    "PyBytes_FromFormatV(): %c format "
                                    "expects an integer in range [0; 255]");
                    return NULL;
                }
                n++;
                break;
            }
            case '%':
                n++;
                break;
            case 'd': case 'u': case 'i': case 'x':
                (void) va_arg(count, int);
                /* 20 bytes is enough to hold a 64-bit
                   integer.  Decimal takes the most space.
                   This isn't enough for octal. */
                n += 20;
                break;
            case 's':
                s = va_arg(count, char*);
                n += strlen(s);
                break;
            case 'p':
                (void) va_arg(count, int);
                /* maximum 64-bit pointer representation:
                 * 0xffffffffffffffff
                 * so 19 characters is enough.
                 * XXX I count 18 -- what's the extra for?
                 */
                n += 19;
                break;
            default:
                /* if we stumble upon an unknown
                   formatting code, copy the rest of
                   the format string to the output
                   string. (we cannot just skip the
                   code, since there's no way to know
                   what's in the argument list) */
                n += strlen(p);
                goto expand;
            }
        } else
            n++;
    }
 expand:
    /* step 2: fill the buffer */
    /* Since we've analyzed how much space we need for the worst case,
       use sprintf directly instead of the slower PyOS_snprintf. */
    string = PyBytes_FromStringAndSize(NULL, n);
    if (!string)
        return NULL;

    s = PyBytes_AsString(string);

    for (f = format; *f; f++) {
        if (*f == '%') {
            const char* p = f++;
            Py_ssize_t i;
            int longflag = 0;
            int size_tflag = 0;
            /* parse the width.precision part (we're only
               interested in the precision value, if any) */
            n = 0;
            while (Py_ISDIGIT(*f))
                n = (n*10) + *f++ - '0';
            if (*f == '.') {
                f++;
                n = 0;
                while (Py_ISDIGIT(*f))
                    n = (n*10) + *f++ - '0';
            }
            while (*f && *f != '%' && !Py_ISALPHA(*f))
                f++;
            /* handle the long flag, but only for %ld and %lu.
               others can be added when necessary. */
            if (*f == 'l' && (f[1] == 'd' || f[1] == 'u')) {
                longflag = 1;
                ++f;
            }
            /* handle the size_t flag. */
            if (*f == 'z' && (f[1] == 'd' || f[1] == 'u')) {
                size_tflag = 1;
                ++f;
            }

            switch (*f) {
            case 'c':
            {
                int c = va_arg(vargs, int);
                /* c has been checked for overflow in the first step */
                *s++ = (unsigned char)c;
                break;
            }
            case 'd':
                if (longflag)
                    sprintf(s, "%ld", va_arg(vargs, long));
                else if (size_tflag)
                    sprintf(s, "%" PY_FORMAT_SIZE_T "d",
                        va_arg(vargs, Py_ssize_t));
                else
                    sprintf(s, "%d", va_arg(vargs, int));
                s += strlen(s);
                break;
            case 'u':
                if (longflag)
                    sprintf(s, "%lu",
                        va_arg(vargs, unsigned long));
                else if (size_tflag)
                    sprintf(s, "%" PY_FORMAT_SIZE_T "u",
                        va_arg(vargs, size_t));
                else
                    sprintf(s, "%u",
                        va_arg(vargs, unsigned int));
                s += strlen(s);
                break;
            case 'i':
                sprintf(s, "%i", va_arg(vargs, int));
                s += strlen(s);
                break;
            case 'x':
                sprintf(s, "%x", va_arg(vargs, int));
                s += strlen(s);
                break;
            case 's':
                p = va_arg(vargs, char*);
                i = strlen(p);
                if (n > 0 && i > n)
                    i = n;
                Py_MEMCPY(s, p, i);
                s += i;
                break;
            case 'p':
                sprintf(s, "%p", va_arg(vargs, void*));
                /* %p is ill-defined:  ensure leading 0x. */
                if (s[1] == 'X')
                    s[1] = 'x';
                else if (s[1] != 'x') {
                    memmove(s+2, s, strlen(s)+1);
                    s[0] = '0';
                    s[1] = 'x';
                }
                s += strlen(s);
                break;
            case '%':
                *s++ = '%';
                break;
            default:
                strcpy(s, p);
                s += strlen(s);
                goto end;
            }
        } else
            *s++ = *f;
    }

 end:
    _PyBytes_Resize(&string, s - PyBytes_AS_STRING(string));
    return string;
}

PyObject *
PyBytes_FromFormat(const char *format, ...)
{
    PyObject* ret;
    va_list vargs;

#ifdef HAVE_STDARG_PROTOTYPES
    va_start(vargs, format);
#else
    va_start(vargs);
#endif
    ret = PyBytes_FromFormatV(format, vargs);
    va_end(vargs);
    return ret;
}

/* =-= */

static void
bytes_dealloc(PyObject *op)
{
    Py_TYPE(op)->tp_free(op);
}

/* -------------------------------------------------------------------- */
/* object api */

Py_ssize_t
PyBytes_Size(PyObject *op)
{
    if (!PyBytes_Check(op)) {
        PyErr_Format(PyExc_TypeError,
             "expected bytes, %.200s found", Py_TYPE(op)->tp_name);
        return -1;
    }
    return Py_SIZE(op);
}

char *
PyBytes_AsString(PyObject *op)
{
    if (!PyBytes_Check(op)) {
        PyErr_Format(PyExc_TypeError,
             "expected bytes, %.200s found", Py_TYPE(op)->tp_name);
        return NULL;
    }
    return ((PyBytesObject *)op)->ob_sval;
}

int
PyBytes_AsStringAndSize(PyObject *obj,
                         char **s,
                         Py_ssize_t *len)
{
    if (s == NULL) {
        PyErr_BadInternalCall();
        return -1;
    }

    if (!PyBytes_Check(obj)) {
        PyErr_Format(PyExc_TypeError,
             "expected bytes, %.200s found", Py_TYPE(obj)->tp_name);
        return -1;
    }

    *s = PyBytes_AS_STRING(obj);
    if (len != NULL)
        *len = PyBytes_GET_SIZE(obj);
    else if (strlen(*s) != (size_t)PyBytes_GET_SIZE(obj)) {
        PyErr_SetString(PyExc_ValueError,
                        "embedded null byte");
        return -1;
    }
    return 0;
}

PyTypeObject PyBytes_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "bytes",
    PyBytesObject_SIZE,
    sizeof(char),
    bytes_dealloc,                      /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    0,                       /* tp_repr */
    0,                           /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                       /* tp_hash */
    0,                                          /* tp_call */
    0,                                  /* tp_str */
    0,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE |
        Py_TPFLAGS_BYTES_SUBCLASS,              /* tp_flags */
    0,                                  /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,             /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                 /* tp_iter */
    0,                                          /* tp_iternext */
    0,                              /* tp_methods */
    0,                                          /* tp_members */
    0,                                          /* tp_getset */
    0,                         /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    0,                                  /* tp_new */
    PyObject_Del,                               /* tp_free */
};

/* The following function breaks the notion that bytes are immutable:
   it changes the size of a bytes object.  We get away with this only if there
   is only one module referencing the object.  You can also think of it
   as creating a new bytes object and destroying the old one, only
   more efficiently.  In any case, don't use this if the bytes object may
   already be known to some other part of the code...
   Note that if there's not enough memory to resize the bytes object, the
   original bytes object at *pv is deallocated, *pv is set to NULL, an "out of
   memory" exception is set, and -1 is returned.  Else (on success) 0 is
   returned, and the value in *pv may or may not be the same as on input.
   As always, an extra byte is allocated for a trailing \0 byte (newsize
   does *not* include that), and a trailing \0 byte is stored.
*/

int
_PyBytes_Resize(PyObject **pv, Py_ssize_t newsize)
{
    PyObject *v;
    PyBytesObject *sv;
    v = *pv;
    if (!PyBytes_Check(v) || Py_REFCNT(v) != 1 || newsize < 0) {
        *pv = 0;
        Py_DECREF(v);
        PyErr_BadInternalCall();
        return -1;
    }
    /* XXX UNREF/NEWREF interface should be more symmetrical */
    _Py_DEC_REFTOTAL;
    _Py_ForgetReference(v);
    *pv = (PyObject *)
        PyObject_REALLOC(v, PyBytesObject_SIZE + newsize);
    if (*pv == NULL) {
        PyObject_Del(v);
        PyErr_NoMemory();
        return -1;
    }
    _Py_NewReference(*pv);
    sv = (PyBytesObject *) *pv;
    Py_SIZE(sv) = newsize;
    sv->ob_sval[newsize] = '\0';
    sv->ob_shash = -1;          /* invalidate cached hash value */
    return 0;
}

void
PyBytes_Fini(void)
{
    int i;
    for (i = 0; i < UCHAR_MAX + 1; i++)
        Py_CLEAR(characters[i]);
    Py_CLEAR(nullstring);
}