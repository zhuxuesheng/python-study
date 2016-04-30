
/* Complex object implementation */

/* Borrows heavily from floatobject.c */

/* Submitted by Jim Hugunin */

#include "Python.h"
#include "structmember.h"

/* elementary operations on complex numbers */

static Py_complex c_1 = {1., 0.};

Py_complex
_Py_c_sum(Py_complex a, Py_complex b)
{
    Py_complex r;
    r.real = a.real + b.real;
    r.imag = a.imag + b.imag;
    return r;
}

Py_complex
_Py_c_diff(Py_complex a, Py_complex b)
{
    Py_complex r;
    r.real = a.real - b.real;
    r.imag = a.imag - b.imag;
    return r;
}

Py_complex
_Py_c_neg(Py_complex a)
{
    Py_complex r;
    r.real = -a.real;
    r.imag = -a.imag;
    return r;
}

Py_complex
_Py_c_prod(Py_complex a, Py_complex b)
{
    Py_complex r;
    r.real = a.real*b.real - a.imag*b.imag;
    r.imag = a.real*b.imag + a.imag*b.real;
    return r;
}

Py_complex
_Py_c_quot(Py_complex a, Py_complex b)
{
    /******************************************************************
    This was the original algorithm.  It's grossly prone to spurious
    overflow and underflow errors.  It also merrily divides by 0 despite
    checking for that(!).  The code still serves a doc purpose here, as
    the algorithm following is a simple by-cases transformation of this
    one:

    Py_complex r;
    double d = b.real*b.real + b.imag*b.imag;
    if (d == 0.)
        errno = EDOM;
    r.real = (a.real*b.real + a.imag*b.imag)/d;
    r.imag = (a.imag*b.real - a.real*b.imag)/d;
    return r;
    ******************************************************************/

    /* This algorithm is better, and is pretty obvious:  first divide the
     * numerators and denominator by whichever of {b.real, b.imag} has
     * larger magnitude.  The earliest reference I found was to CACM
     * Algorithm 116 (Complex Division, Robert L. Smith, Stanford
     * University).  As usual, though, we're still ignoring all IEEE
     * endcases.
     */
     Py_complex r;      /* the result */
     const double abs_breal = b.real < 0 ? -b.real : b.real;
     const double abs_bimag = b.imag < 0 ? -b.imag : b.imag;

    if (abs_breal >= abs_bimag) {
        /* divide tops and bottom by b.real */
        if (abs_breal == 0.0) {
            errno = EDOM;
            r.real = r.imag = 0.0;
        }
        else {
            const double ratio = b.imag / b.real;
            const double denom = b.real + b.imag * ratio;
            r.real = (a.real + a.imag * ratio) / denom;
            r.imag = (a.imag - a.real * ratio) / denom;
        }
    }
    else if (abs_bimag >= abs_breal) {
        /* divide tops and bottom by b.imag */
        const double ratio = b.real / b.imag;
        const double denom = b.real * ratio + b.imag;
        assert(b.imag != 0.0);
        r.real = (a.real * ratio + a.imag) / denom;
        r.imag = (a.imag * ratio - a.real) / denom;
    }
    else {
        /* At least one of b.real or b.imag is a NaN */
        r.real = r.imag = Py_NAN;
    }
    return r;
}

Py_complex
_Py_c_pow(Py_complex a, Py_complex b)
{
    Py_complex r;
    double vabs,len,at,phase;
    if (b.real == 0. && b.imag == 0.) {
        r.real = 1.;
        r.imag = 0.;
    }
    else if (a.real == 0. && a.imag == 0.) {
        if (b.imag != 0. || b.real < 0.)
            errno = EDOM;
        r.real = 0.;
        r.imag = 0.;
    }
    else {
        vabs = hypot(a.real,a.imag);
        len = pow(vabs,b.real);
        at = atan2(a.imag, a.real);
        phase = at*b.real;
        if (b.imag != 0.0) {
            len /= exp(at*b.imag);
            phase += b.imag*log(vabs);
        }
        r.real = len*cos(phase);
        r.imag = len*sin(phase);
    }
    return r;
}

static Py_complex
c_powu(Py_complex x, long n)
{
    Py_complex r, p;
    long mask = 1;
    r = c_1;
    p = x;
    while (mask > 0 && n >= mask) {
        if (n & mask)
            r = _Py_c_prod(r,p);
        mask <<= 1;
        p = _Py_c_prod(p,p);
    }
    return r;
}

static Py_complex
c_powi(Py_complex x, long n)
{
    Py_complex cn;

    if (n > 100 || n < -100) {
        cn.real = (double) n;
        cn.imag = 0.;
        return _Py_c_pow(x,cn);
    }
    else if (n > 0)
        return c_powu(x,n);
    else
        return _Py_c_quot(c_1, c_powu(x,-n));

}

double
_Py_c_abs(Py_complex z)
{
    /* sets errno = ERANGE on overflow;  otherwise errno = 0 */
    double result;

    if (!Py_IS_FINITE(z.real) || !Py_IS_FINITE(z.imag)) {
        /* C99 rules: if either the real or the imaginary part is an
           infinity, return infinity, even if the other part is a
           NaN. */
        if (Py_IS_INFINITY(z.real)) {
            result = fabs(z.real);
            errno = 0;
            return result;
        }
        if (Py_IS_INFINITY(z.imag)) {
            result = fabs(z.imag);
            errno = 0;
            return result;
        }
        /* either the real or imaginary part is a NaN,
           and neither is infinite. Result should be NaN. */
        return Py_NAN;
    }
    result = hypot(z.real, z.imag);
    if (!Py_IS_FINITE(result))
        errno = ERANGE;
    else
        errno = 0;
    return result;
}

static PyObject *
complex_subtype_from_c_complex(PyTypeObject *type, Py_complex cval)
{
    PyObject *op;

    op = type->tp_alloc(type, 0);
    if (op != NULL)
        ((PyComplexObject *)op)->cval = cval;
    return op;
}

PyObject *
PyComplex_FromCComplex(Py_complex cval)
{
    PyComplexObject *op;

    /* Inline PyObject_New */
    op = (PyComplexObject *) PyObject_MALLOC(sizeof(PyComplexObject));
    if (op == NULL)
        return PyErr_NoMemory();
    (void)PyObject_INIT(op, &PyComplex_Type);
    op->cval = cval;
    return (PyObject *) op;
}

static PyObject *
complex_subtype_from_doubles(PyTypeObject *type, double real, double imag)
{
    Py_complex c;
    c.real = real;
    c.imag = imag;
    return complex_subtype_from_c_complex(type, c);
}

PyObject *
PyComplex_FromDoubles(double real, double imag)
{
    Py_complex c;
    c.real = real;
    c.imag = imag;
    return PyComplex_FromCComplex(c);
}

double
PyComplex_RealAsDouble(PyObject *op)
{
    if (PyComplex_Check(op)) {
        return ((PyComplexObject *)op)->cval.real;
    }
    else {
        return PyFloat_AsDouble(op);
    }
}

double
PyComplex_ImagAsDouble(PyObject *op)
{
    if (PyComplex_Check(op)) {
        return ((PyComplexObject *)op)->cval.imag;
    }
    else {
        return 0.0;
    }
}

static PyObject *
try_complex_special_method(PyObject *op) {
    PyObject *f;
    _Py_IDENTIFIER(__complex__);

    f = _PyObject_LookupSpecial(op, &PyId___complex__);
    if (f) {
        PyObject *res = PyObject_CallFunctionObjArgs(f, NULL);
        Py_DECREF(f);
        if (res != NULL && !PyComplex_Check(res)) {
            PyErr_SetString(PyExc_TypeError,
                "__complex__ should return a complex object");
            Py_DECREF(res);
            return NULL;
        }
        return res;
    }
    return NULL;
}

Py_complex
PyComplex_AsCComplex(PyObject *op)
{
    Py_complex cv;
    PyObject *newop = NULL;

    assert(op);
    /* If op is already of type PyComplex_Type, return its value */
    if (PyComplex_Check(op)) {
        return ((PyComplexObject *)op)->cval;
    }
    /* If not, use op's __complex__  method, if it exists */

    /* return -1 on failure */
    cv.real = -1.;
    cv.imag = 0.;

    newop = try_complex_special_method(op);

    if (newop) {
        cv = ((PyComplexObject *)newop)->cval;
        Py_DECREF(newop);
        return cv;
    }
    else if (PyErr_Occurred()) {
        return cv;
    }
    /* If neither of the above works, interpret op as a float giving the
       real part of the result, and fill in the imaginary part as 0. */
    else {
        /* PyFloat_AsDouble will return -1 on failure */
        cv.real = PyFloat_AsDouble(op);
        return cv;
    }
}

static void
complex_dealloc(PyObject *op)
{
    op->ob_type->tp_free(op);
}

/* This macro may return! */
#define TO_COMPLEX(obj, c) \
    if (PyComplex_Check(obj)) \
        c = ((PyComplexObject *)(obj))->cval; \
    else if (to_complex(&(obj), &(c)) < 0) \
        return (obj)

static int
to_complex(PyObject **pobj, Py_complex *pc)
{
    PyObject *obj = *pobj;

    pc->real = pc->imag = 0.0;
    if (PyLong_Check(obj)) {
        pc->real = PyLong_AsDouble(obj);
        if (pc->real == -1.0 && PyErr_Occurred()) {
            *pobj = NULL;
            return -1;
        }
        return 0;
    }
    if (PyFloat_Check(obj)) {
        pc->real = PyFloat_AsDouble(obj);
        return 0;
    }
    Py_INCREF(Py_NotImplemented);
    *pobj = Py_NotImplemented;
    return -1;
}


static PyObject *
complex_add(PyObject *v, PyObject *w)
{
    Py_complex result;
    Py_complex a, b;
    TO_COMPLEX(v, a);
    TO_COMPLEX(w, b);
    PyFPE_START_PROTECT("complex_add", return 0)
    result = _Py_c_sum(a, b);
    PyFPE_END_PROTECT(result)
    return PyComplex_FromCComplex(result);
}

static PyObject *
complex_sub(PyObject *v, PyObject *w)
{
    Py_complex result;
    Py_complex a, b;
    TO_COMPLEX(v, a);
    TO_COMPLEX(w, b);
    PyFPE_START_PROTECT("complex_sub", return 0)
    result = _Py_c_diff(a, b);
    PyFPE_END_PROTECT(result)
    return PyComplex_FromCComplex(result);
}

static PyObject *
complex_mul(PyObject *v, PyObject *w)
{
    Py_complex result;
    Py_complex a, b;
    TO_COMPLEX(v, a);
    TO_COMPLEX(w, b);
    PyFPE_START_PROTECT("complex_mul", return 0)
    result = _Py_c_prod(a, b);
    PyFPE_END_PROTECT(result)
    return PyComplex_FromCComplex(result);
}

static PyObject *
complex_div(PyObject *v, PyObject *w)
{
    Py_complex quot;
    Py_complex a, b;
    TO_COMPLEX(v, a);
    TO_COMPLEX(w, b);
    PyFPE_START_PROTECT("complex_div", return 0)
    errno = 0;
    quot = _Py_c_quot(a, b);
    PyFPE_END_PROTECT(quot)
    if (errno == EDOM) {
        PyErr_SetString(PyExc_ZeroDivisionError, "complex division by zero");
        return NULL;
    }
    return PyComplex_FromCComplex(quot);
}

static PyObject *
complex_remainder(PyObject *v, PyObject *w)
{
    PyErr_SetString(PyExc_TypeError,
                    "can't mod complex numbers.");
    return NULL;
}


static PyObject *
complex_divmod(PyObject *v, PyObject *w)
{
    PyErr_SetString(PyExc_TypeError,
                    "can't take floor or mod of complex number.");
    return NULL;
}

static PyObject *
complex_pow(PyObject *v, PyObject *w, PyObject *z)
{
    Py_complex p;
    Py_complex exponent;
    long int_exponent;
    Py_complex a, b;
    TO_COMPLEX(v, a);
    TO_COMPLEX(w, b);

    if (z != Py_None) {
        PyErr_SetString(PyExc_ValueError, "complex modulo");
        return NULL;
    }
    PyFPE_START_PROTECT("complex_pow", return 0)
    errno = 0;
    exponent = b;
    int_exponent = (long)exponent.real;
    if (exponent.imag == 0. && exponent.real == int_exponent)
        p = c_powi(a, int_exponent);
    else
        p = _Py_c_pow(a, exponent);

    PyFPE_END_PROTECT(p)
    Py_ADJUST_ERANGE2(p.real, p.imag);
    if (errno == EDOM) {
        PyErr_SetString(PyExc_ZeroDivisionError,
                        "0.0 to a negative or complex power");
        return NULL;
    }
    else if (errno == ERANGE) {
        PyErr_SetString(PyExc_OverflowError,
                        "complex exponentiation");
        return NULL;
    }
    return PyComplex_FromCComplex(p);
}

static PyObject *
complex_int_div(PyObject *v, PyObject *w)
{
    PyErr_SetString(PyExc_TypeError,
                    "can't take floor of complex number.");
    return NULL;
}

static PyObject *
complex_neg(PyComplexObject *v)
{
    Py_complex neg;
    neg.real = -v->cval.real;
    neg.imag = -v->cval.imag;
    return PyComplex_FromCComplex(neg);
}

static PyObject *
complex_pos(PyComplexObject *v)
{
    if (PyComplex_CheckExact(v)) {
        Py_INCREF(v);
        return (PyObject *)v;
    }
    else
        return PyComplex_FromCComplex(v->cval);
}

static PyObject *
complex_abs(PyComplexObject *v)
{
    double result;

    PyFPE_START_PROTECT("complex_abs", return 0)
    result = _Py_c_abs(v->cval);
    PyFPE_END_PROTECT(result)

    if (errno == ERANGE) {
        PyErr_SetString(PyExc_OverflowError,
                        "absolute value too large");
        return NULL;
    }
    return PyFloat_FromDouble(result);
}

static int
complex_bool(PyComplexObject *v)
{
    return v->cval.real != 0.0 || v->cval.imag != 0.0;
}

static PyObject *
complex_richcompare(PyObject *v, PyObject *w, int op)
{
    PyObject *res;
    Py_complex i;
    int equal;

    if (op != Py_EQ && op != Py_NE) {
        goto Unimplemented;
    }

    assert(PyComplex_Check(v));
    TO_COMPLEX(v, i);

    if (PyLong_Check(w)) {
        /* Check for 0.0 imaginary part first to avoid the rich
         * comparison when possible.
         */
        if (i.imag == 0.0) {
            PyObject *j, *sub_res;
            j = PyFloat_FromDouble(i.real);
            if (j == NULL)
                return NULL;

            sub_res = PyObject_RichCompare(j, w, op);
            Py_DECREF(j);
            return sub_res;
        }
        else {
            equal = 0;
        }
    }
    else if (PyFloat_Check(w)) {
        equal = (i.real == PyFloat_AsDouble(w) && i.imag == 0.0);
    }
    else if (PyComplex_Check(w)) {
        Py_complex j;

        TO_COMPLEX(w, j);
        equal = (i.real == j.real && i.imag == j.imag);
    }
    else {
        goto Unimplemented;
    }

    if (equal == (op == Py_EQ))
         res = Py_True;
    else
         res = Py_False;

    Py_INCREF(res);
    return res;

Unimplemented:
    Py_RETURN_NOTIMPLEMENTED;
}

static PyObject *
complex_int(PyObject *v)
{
    PyErr_SetString(PyExc_TypeError,
               "can't convert complex to int");
    return NULL;
}

static PyObject *
complex_float(PyObject *v)
{
    PyErr_SetString(PyExc_TypeError,
               "can't convert complex to float");
    return NULL;
}

static PyNumberMethods complex_as_number = {
    (binaryfunc)complex_add,                    /* nb_add */
    (binaryfunc)complex_sub,                    /* nb_subtract */
    (binaryfunc)complex_mul,                    /* nb_multiply */
    (binaryfunc)complex_remainder,              /* nb_remainder */
    (binaryfunc)complex_divmod,                 /* nb_divmod */
    (ternaryfunc)complex_pow,                   /* nb_power */
    (unaryfunc)complex_neg,                     /* nb_negative */
    (unaryfunc)complex_pos,                     /* nb_positive */
    (unaryfunc)complex_abs,                     /* nb_absolute */
    (inquiry)complex_bool,                      /* nb_bool */
    0,                                          /* nb_invert */
    0,                                          /* nb_lshift */
    0,                                          /* nb_rshift */
    0,                                          /* nb_and */
    0,                                          /* nb_xor */
    0,                                          /* nb_or */
    complex_int,                                /* nb_int */
    0,                                          /* nb_reserved */
    complex_float,                              /* nb_float */
    0,                                          /* nb_inplace_add */
    0,                                          /* nb_inplace_subtract */
    0,                                          /* nb_inplace_multiply*/
    0,                                          /* nb_inplace_remainder */
    0,                                          /* nb_inplace_power */
    0,                                          /* nb_inplace_lshift */
    0,                                          /* nb_inplace_rshift */
    0,                                          /* nb_inplace_and */
    0,                                          /* nb_inplace_xor */
    0,                                          /* nb_inplace_or */
    (binaryfunc)complex_int_div,                /* nb_floor_divide */
    (binaryfunc)complex_div,                    /* nb_true_divide */
    0,                                          /* nb_inplace_floor_divide */
    0,                                          /* nb_inplace_true_divide */
};

PyTypeObject PyComplex_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "complex",
    sizeof(PyComplexObject),
    0,
    complex_dealloc,                            /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    0,                     /* tp_repr */
    &complex_as_number,                         /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                     /* tp_hash */
    0,                                          /* tp_call */
    0,                     /* tp_str */
    0,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                                /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    complex_richcompare,                        /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    0,                            /* tp_methods */
    0,                            /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                        /* tp_alloc */
    0,                                /* tp_new */
    PyObject_Del,                               /* tp_free */
};
