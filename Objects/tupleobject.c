
/* Tuple object implementation */

#include "Python.h"
//#include "accu.h"

/* Speed optimization to avoid frequent malloc/free of small tuples */
#ifndef PyTuple_MAXSAVESIZE
#define PyTuple_MAXSAVESIZE     20  /* Largest tuple to save on free list */
#endif
#ifndef PyTuple_MAXFREELIST
#define PyTuple_MAXFREELIST  2000  /* Maximum number of tuples of each size to save */
#endif

#if PyTuple_MAXSAVESIZE > 0
/* Entries 1 up to PyTuple_MAXSAVESIZE are free lists, entry 0 is the empty
   tuple () of which at most one instance will be allocated.
*/
static PyTupleObject *free_list[PyTuple_MAXSAVESIZE];
static int numfree[PyTuple_MAXSAVESIZE];
#endif
#ifdef COUNT_ALLOCS
Py_ssize_t fast_tuple_allocs;
Py_ssize_t tuple_zero_allocs;
#endif

/* Debug statistic to count GC tracking of tuples.
   Please note that tuples are only untracked when considered by the GC, and
   many of them will be dead before. Therefore, a tracking rate close to 100%
   does not necessarily prove that the heuristic is inefficient.
*/
#ifdef SHOW_TRACK_COUNT
static Py_ssize_t count_untracked = 0;
static Py_ssize_t count_tracked = 0;

static void
show_track(void)
{
    fprintf(stderr, "Tuples created: %" PY_FORMAT_SIZE_T "d\n",
        count_tracked + count_untracked);
    fprintf(stderr, "Tuples tracked by the GC: %" PY_FORMAT_SIZE_T
        "d\n", count_tracked);
    fprintf(stderr, "%.2f%% tuple tracking rate\n\n",
        (100.0*count_tracked/(count_untracked+count_tracked)));
}
#endif

/* Print summary info about the state of the optimized allocator */
void
_PyTuple_DebugMallocStats(FILE *out)
{
#if PyTuple_MAXSAVESIZE > 0
    int i;
    char buf[128];
    for (i = 1; i < PyTuple_MAXSAVESIZE; i++) {
        PyOS_snprintf(buf, sizeof(buf),
                      "free %d-sized PyTupleObject", i);
        _PyDebugAllocatorStats(out,
                               buf,
                               numfree[i], _PyObject_VAR_SIZE(&PyTuple_Type, i));
    }
#endif
}

PyObject *
PyTuple_New(Py_ssize_t size)
{
    PyTupleObject *op;
    Py_ssize_t i;
    if (size < 0) {
        PyErr_BadInternalCall();
        return NULL;
    }
#if PyTuple_MAXSAVESIZE > 0
    if (size == 0 && free_list[0]) {
        op = free_list[0];
        Py_INCREF(op);
#ifdef COUNT_ALLOCS
        tuple_zero_allocs++;
#endif
        return (PyObject *) op;
    }
    if (size < PyTuple_MAXSAVESIZE && (op = free_list[size]) != NULL) {
        free_list[size] = (PyTupleObject *) op->ob_item[0];
        numfree[size]--;
#ifdef COUNT_ALLOCS
        fast_tuple_allocs++;
#endif
        /* Inline PyObject_InitVar */
#ifdef Py_TRACE_REFS
        Py_SIZE(op) = size;
        Py_TYPE(op) = &PyTuple_Type;
#endif
        _Py_NewReference((PyObject *)op);
    }
    else
#endif
    {
        /* Check for overflow */
        if ((size_t)size > ((size_t)PY_SSIZE_T_MAX - sizeof(PyTupleObject) -
                    sizeof(PyObject *)) / sizeof(PyObject *)) {
            return PyErr_NoMemory();
        }
        op = PyObject_GC_NewVar(PyTupleObject, &PyTuple_Type, size);
        if (op == NULL)
            return NULL;
    }
    for (i=0; i < size; i++)
        op->ob_item[i] = NULL;
#if PyTuple_MAXSAVESIZE > 0
    if (size == 0) {
        free_list[0] = op;
        ++numfree[0];
        Py_INCREF(op);          /* extra INCREF so that this is never freed */
    }
#endif
#ifdef SHOW_TRACK_COUNT
    count_tracked++;
#endif
    //_PyObject_GC_TRACK(op);
    return (PyObject *) op;
}

Py_ssize_t
PyTuple_Size(PyObject *op)
{
    if (!PyTuple_Check(op)) {
        PyErr_BadInternalCall();
        return -1;
    }
    else
        return Py_SIZE(op);
}

PyObject *
PyTuple_GetItem(PyObject *op, Py_ssize_t i)
{
    if (!PyTuple_Check(op)) {
        PyErr_BadInternalCall();
        return NULL;
    }
    if (i < 0 || i >= Py_SIZE(op)) {
        PyErr_SetString(PyExc_IndexError, "tuple index out of range");
        return NULL;
    }
    return ((PyTupleObject *)op) -> ob_item[i];
}

int
PyTuple_SetItem(PyObject *op, Py_ssize_t i, PyObject *newitem)
{
    PyObject *olditem;
    PyObject **p;
    if (!PyTuple_Check(op) || op->ob_refcnt != 1) {
        Py_XDECREF(newitem);
        PyErr_BadInternalCall();
        return -1;
    }
    if (i < 0 || i >= Py_SIZE(op)) {
        Py_XDECREF(newitem);
        PyErr_SetString(PyExc_IndexError,
                        "tuple assignment index out of range");
        return -1;
    }
    p = ((PyTupleObject *)op) -> ob_item + i;
    olditem = *p;
    *p = newitem;
    Py_XDECREF(olditem);
    return 0;
}

PyTypeObject PyTuple_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "tuple",
    sizeof(PyTupleObject) - sizeof(PyObject *),
    sizeof(PyObject *),
    0,                   /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    0,                        /* tp_repr */
    0,                                          /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                        /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    0,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    0, /* tp_flags */
    0,                                  /* tp_doc */
    0,                /* tp_traverse */
    0,                                          /* tp_clear */
    0,                           /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                 /* tp_iter */
    0,                                          /* tp_iternext */
    0,                              /* tp_methods */
    0,                                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    0,                                  /* tp_new */
    PyObject_Del,                            /* tp_free */
};

/* The following function breaks the notion that tuples are immutable:
   it changes the size of a tuple.  We get away with this only if there
   is only one module referencing the object.  You can also think of it
   as creating a new tuple object and destroying the old one, only more
   efficiently.  In any case, don't use this if the tuple may already be
   known to some other part of the code. */

int
_PyTuple_Resize(PyObject **pv, Py_ssize_t newsize)
{
    PyTupleObject *v;
    PyTupleObject *sv;
    Py_ssize_t i;
    Py_ssize_t oldsize;

    v = (PyTupleObject *) *pv;
    if (v == NULL || Py_TYPE(v) != &PyTuple_Type ||
        (Py_SIZE(v) != 0 && Py_REFCNT(v) != 1)) {
        *pv = 0;
        Py_XDECREF(v);
        PyErr_BadInternalCall();
        return -1;
    }
    oldsize = Py_SIZE(v);
    if (oldsize == newsize)
        return 0;

    if (oldsize == 0) {
        /* Empty tuples are often shared, so we should never
           resize them in-place even if we do own the only
           (current) reference */
        Py_DECREF(v);
        *pv = PyTuple_New(newsize);
        return *pv == NULL ? -1 : 0;
    }

    /* XXX UNREF/NEWREF interface should be more symmetrical */
    _Py_DEC_REFTOTAL;
    if (_PyObject_GC_IS_TRACKED(v))
        _PyObject_GC_UNTRACK(v);
    _Py_ForgetReference((PyObject *) v);
    /* DECREF items deleted by shrinkage */
    for (i = newsize; i < oldsize; i++) {
        Py_CLEAR(v->ob_item[i]);
    }
    sv = PyObject_GC_Resize(PyTupleObject, v, newsize);
    if (sv == NULL) {
        *pv = NULL;
        PyObject_GC_Del(v);
        return -1;
    }
    _Py_NewReference((PyObject *) sv);
    /* Zero out items added by growing */
    if (newsize > oldsize)
        memset(&sv->ob_item[oldsize], 0,
               sizeof(*sv->ob_item) * (newsize - oldsize));
    *pv = (PyObject *) sv;
    _PyObject_GC_TRACK(sv);
    return 0;
}

int
PyTuple_ClearFreeList(void)
{
    int freelist_size = 0;
#if PyTuple_MAXSAVESIZE > 0
    int i;
    for (i = 1; i < PyTuple_MAXSAVESIZE; i++) {
        PyTupleObject *p, *q;
        p = free_list[i];
        freelist_size += numfree[i];
        free_list[i] = NULL;
        numfree[i] = 0;
        while (p) {
            q = p;
            p = (PyTupleObject *)(p->ob_item[0]);
            PyObject_GC_Del(q);
        }
    }
#endif
    return freelist_size;
}

void
PyTuple_Fini(void)
{
#if PyTuple_MAXSAVESIZE > 0
    /* empty tuples are used all over the place and applications may
     * rely on the fact that an empty tuple is a singleton. */
    Py_CLEAR(free_list[0]);

    (void)PyTuple_ClearFreeList();
#endif
#ifdef SHOW_TRACK_COUNT
    show_track();
#endif
}

/*********************** Tuple Iterator **************************/