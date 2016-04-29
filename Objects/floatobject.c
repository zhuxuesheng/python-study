
/* Float object implementation */

/* XXX There should be overflow checks here, but it's hard to check
   for any kind of float exception without losing portability. */

#include "Python.h"

#include <ctype.h>
#include <float.h>


/* Special free list
   free_list is a singly-linked list of available PyFloatObjects, linked
   via abuse of their ob_type members.
*/

#ifndef PyFloat_MAXFREELIST
#define PyFloat_MAXFREELIST    100
#endif
static int numfree = 0;
static PyFloatObject *free_list = NULL;

double
PyFloat_GetMax(void)
{
    return DBL_MAX;
}

double
PyFloat_GetMin(void)
{
    return DBL_MIN;
}


PyObject *
PyFloat_FromDouble(double fval)
{
    PyFloatObject *op = free_list;
    if (op != NULL) {
        free_list = (PyFloatObject *) Py_TYPE(op);
        numfree--;
    } else {
        op = (PyFloatObject*) PyObject_MALLOC(sizeof(PyFloatObject));
        if (!op)
            return PyErr_NoMemory();
    }
    /* Inline PyObject_New */
    (void)PyObject_INIT(op, &PyFloat_Type);
    op->ob_fval = fval;
    return (PyObject *) op;
}


static void
float_dealloc(PyFloatObject *op)
{
    if (PyFloat_CheckExact(op)) {
        if (numfree >= PyFloat_MAXFREELIST)  {
            PyObject_FREE(op);
            return;
        }
        numfree++;
        Py_TYPE(op) = (struct _typeobject *)free_list;
        free_list = op;
    }
    else
        Py_TYPE(op)->tp_free((PyObject *)op);
}

double
PyFloat_AsDouble(PyObject *op)
{
    PyNumberMethods *nb;
    PyFloatObject *fo;
    double val;

    if (op && PyFloat_Check(op))
        return PyFloat_AS_DOUBLE((PyFloatObject*) op);

    if (op == NULL) {
        PyErr_BadArgument();
        return -1;
    }

    if ((nb = Py_TYPE(op)->tp_as_number) == NULL || nb->nb_float == NULL) {
        PyErr_SetString(PyExc_TypeError, "a float is required");
        return -1;
    }

    fo = (PyFloatObject*) (*nb->nb_float) (op);
    if (fo == NULL)
        return -1;
    if (!PyFloat_Check(fo)) {
        Py_DECREF(fo);
        PyErr_SetString(PyExc_TypeError,
                        "nb_float should return float object");
        return -1;
    }

    val = PyFloat_AS_DOUBLE(fo);
    Py_DECREF(fo);

    return val;
}

/* Macro and helper that convert PyObject obj to a C double and store
   the value in dbl.  If conversion to double raises an exception, obj is
   set to NULL, and the function invoking this macro returns NULL.  If
   obj is not of float or int type, Py_NotImplemented is incref'ed,
   stored in obj, and returned from the function invoking this macro.
*/
#define CONVERT_TO_DOUBLE(obj, dbl)                     \
    if (PyFloat_Check(obj))                             \
        dbl = PyFloat_AS_DOUBLE(obj);                   \
    else if (convert_to_double(&(obj), &(dbl)) < 0)     \
        return obj;

/* Methods */

static int
convert_to_double(PyObject **v, double *dbl)
{
    PyObject *obj = *v;

    if (PyLong_Check(obj)) {
        *dbl = PyLong_AsDouble(obj);
        if (*dbl == -1.0 && PyErr_Occurred()) {
            *v = NULL;
            return -1;
        }
    }
    else {
        Py_INCREF(Py_NotImplemented);
        *v = Py_NotImplemented;
        return -1;
    }
    return 0;
}


/* Comparison is pretty much a nightmare.  When comparing float to float,
 * we do it as straightforwardly (and long-windedly) as conceivable, so
 * that, e.g., Python x == y delivers the same result as the platform
 * C x == y when x and/or y is a NaN.
 * When mixing float with an integer type, there's no good *uniform* approach.
 * Converting the double to an integer obviously doesn't work, since we
 * may lose info from fractional bits.  Converting the integer to a double
 * also has two failure modes:  (1) an int may trigger overflow (too
 * large to fit in the dynamic range of a C double); (2) even a C long may have
 * more bits than fit in a C double (e.g., on a 64-bit box long may have
 * 63 bits of precision, but a C double probably has only 53), and then
 * we can falsely claim equality when low-order integer bits are lost by
 * coercion to double.  So this part is painful too.
 */

static PyObject*
float_richcompare(PyObject *v, PyObject *w, int op)
{
    double i, j;
    int r = 0;

    assert(PyFloat_Check(v));
    i = PyFloat_AS_DOUBLE(v);

    /* Switch on the type of w.  Set i and j to doubles to be compared,
     * and op to the richcomp to use.
     */
    if (PyFloat_Check(w))
        j = PyFloat_AS_DOUBLE(w);

    else if (!Py_IS_FINITE(i)) {
        if (PyLong_Check(w))
            /* If i is an infinity, its magnitude exceeds any
             * finite integer, so it doesn't matter which int we
             * compare i with.  If i is a NaN, similarly.
             */
            j = 0.0;
        else
            goto Unimplemented;
    }

    else if (PyLong_Check(w)) {
        int vsign = i == 0.0 ? 0 : i < 0.0 ? -1 : 1;
        int wsign = _PyLong_Sign(w);
        size_t nbits;
        int exponent;

        if (vsign != wsign) {
            /* Magnitudes are irrelevant -- the signs alone
             * determine the outcome.
             */
            i = (double)vsign;
            j = (double)wsign;
            goto Compare;
        }
        /* The signs are the same. */
        /* Convert w to a double if it fits.  In particular, 0 fits. */
        nbits = _PyLong_NumBits(w);
        if (nbits == (size_t)-1 && PyErr_Occurred()) {
            /* This long is so large that size_t isn't big enough
             * to hold the # of bits.  Replace with little doubles
             * that give the same outcome -- w is so large that
             * its magnitude must exceed the magnitude of any
             * finite float.
             */
            PyErr_Clear();
            i = (double)vsign;
            assert(wsign != 0);
            j = wsign * 2.0;
            goto Compare;
        }
        if (nbits <= 48) {
            j = PyLong_AsDouble(w);
            /* It's impossible that <= 48 bits overflowed. */
            assert(j != -1.0 || ! PyErr_Occurred());
            goto Compare;
        }
        assert(wsign != 0); /* else nbits was 0 */
        assert(vsign != 0); /* if vsign were 0, then since wsign is
                             * not 0, we would have taken the
                             * vsign != wsign branch at the start */
        /* We want to work with non-negative numbers. */
        if (vsign < 0) {
            /* "Multiply both sides" by -1; this also swaps the
             * comparator.
             */
            i = -i;
            op = _Py_SwappedOp[op];
        }
        assert(i > 0.0);
        (void) frexp(i, &exponent);
        /* exponent is the # of bits in v before the radix point;
         * we know that nbits (the # of bits in w) > 48 at this point
         */
        if (exponent < 0 || (size_t)exponent < nbits) {
            i = 1.0;
            j = 2.0;
            goto Compare;
        }
        if ((size_t)exponent > nbits) {
            i = 2.0;
            j = 1.0;
            goto Compare;
        }
        /* v and w have the same number of bits before the radix
         * point.  Construct two ints that have the same comparison
         * outcome.
         */
        {
            double fracpart;
            double intpart;
            PyObject *result = NULL;
            PyObject *one = NULL;
            PyObject *vv = NULL;
            PyObject *ww = w;

            if (wsign < 0) {
                ww = PyNumber_Negative(w);
                if (ww == NULL)
                    goto Error;
            }
            else
                Py_INCREF(ww);

            fracpart = modf(i, &intpart);
            vv = PyLong_FromDouble(intpart);
            if (vv == NULL)
                goto Error;

            if (fracpart != 0.0) {
                /* Shift left, and or a 1 bit into vv
                 * to represent the lost fraction.
                 */
                PyObject *temp;

                one = PyLong_FromLong(1);
                if (one == NULL)
                    goto Error;

                temp = PyNumber_Lshift(ww, one);
                if (temp == NULL)
                    goto Error;
                Py_DECREF(ww);
                ww = temp;

                temp = PyNumber_Lshift(vv, one);
                if (temp == NULL)
                    goto Error;
                Py_DECREF(vv);
                vv = temp;

                temp = PyNumber_Or(vv, one);
                if (temp == NULL)
                    goto Error;
                Py_DECREF(vv);
                vv = temp;
            }

            r = PyObject_RichCompareBool(vv, ww, op);
            if (r < 0)
                goto Error;
            result = PyBool_FromLong(r);
         Error:
            Py_XDECREF(vv);
            Py_XDECREF(ww);
            Py_XDECREF(one);
            return result;
        }
    } /* else if (PyLong_Check(w)) */

    else        /* w isn't float or int */
        goto Unimplemented;

 Compare:
    PyFPE_START_PROTECT("richcompare", return NULL)
    switch (op) {
    case Py_EQ:
        r = i == j;
        break;
    case Py_NE:
        r = i != j;
        break;
    case Py_LE:
        r = i <= j;
        break;
    case Py_GE:
        r = i >= j;
        break;
    case Py_LT:
        r = i < j;
        break;
    case Py_GT:
        r = i > j;
        break;
    }
    PyFPE_END_PROTECT(r)
    return PyBool_FromLong(r);

 Unimplemented:
    Py_RETURN_NOTIMPLEMENTED;
}

static Py_hash_t
float_hash(PyFloatObject *v)
{
    return _Py_HashDouble(v->ob_fval);
}

static PyObject *
float_add(PyObject *v, PyObject *w)
{
    double a,b;
    CONVERT_TO_DOUBLE(v, a);
    CONVERT_TO_DOUBLE(w, b);
    PyFPE_START_PROTECT("add", return 0)
    a = a + b;
    PyFPE_END_PROTECT(a)
    return PyFloat_FromDouble(a);
}

static PyObject *
float_sub(PyObject *v, PyObject *w)
{
    double a,b;
    CONVERT_TO_DOUBLE(v, a);
    CONVERT_TO_DOUBLE(w, b);
    PyFPE_START_PROTECT("subtract", return 0)
    a = a - b;
    PyFPE_END_PROTECT(a)
    return PyFloat_FromDouble(a);
}

static PyObject *
float_mul(PyObject *v, PyObject *w)
{
    double a,b;
    CONVERT_TO_DOUBLE(v, a);
    CONVERT_TO_DOUBLE(w, b);
    PyFPE_START_PROTECT("multiply", return 0)
    a = a * b;
    PyFPE_END_PROTECT(a)
    return PyFloat_FromDouble(a);
}

static PyObject *
float_div(PyObject *v, PyObject *w)
{
    double a,b;
    CONVERT_TO_DOUBLE(v, a);
    CONVERT_TO_DOUBLE(w, b);
    if (b == 0.0) {
        PyErr_SetString(PyExc_ZeroDivisionError,
                        "float division by zero");
        return NULL;
    }
    PyFPE_START_PROTECT("divide", return 0)
    a = a / b;
    PyFPE_END_PROTECT(a)
    return PyFloat_FromDouble(a);
}

static PyObject *
float_rem(PyObject *v, PyObject *w)
{
    double vx, wx;
    double mod;
    CONVERT_TO_DOUBLE(v, vx);
    CONVERT_TO_DOUBLE(w, wx);
    if (wx == 0.0) {
        PyErr_SetString(PyExc_ZeroDivisionError,
                        "float modulo");
        return NULL;
    }
    PyFPE_START_PROTECT("modulo", return 0)
    mod = fmod(vx, wx);
    if (mod) {
        /* ensure the remainder has the same sign as the denominator */
        if ((wx < 0) != (mod < 0)) {
            mod += wx;
        }
    }
    else {
        /* the remainder is zero, and in the presence of signed zeroes
           fmod returns different results across platforms; ensure
           it has the same sign as the denominator. */
        mod = copysign(0.0, wx);
    }
    PyFPE_END_PROTECT(mod)
    return PyFloat_FromDouble(mod);
}

static PyObject *
float_divmod(PyObject *v, PyObject *w)
{
    double vx, wx;
    double div, mod, floordiv;
    CONVERT_TO_DOUBLE(v, vx);
    CONVERT_TO_DOUBLE(w, wx);
    if (wx == 0.0) {
        PyErr_SetString(PyExc_ZeroDivisionError, "float divmod()");
        return NULL;
    }
    PyFPE_START_PROTECT("divmod", return 0)
    mod = fmod(vx, wx);
    /* fmod is typically exact, so vx-mod is *mathematically* an
       exact multiple of wx.  But this is fp arithmetic, and fp
       vx - mod is an approximation; the result is that div may
       not be an exact integral value after the division, although
       it will always be very close to one.
    */
    div = (vx - mod) / wx;
    if (mod) {
        /* ensure the remainder has the same sign as the denominator */
        if ((wx < 0) != (mod < 0)) {
            mod += wx;
            div -= 1.0;
        }
    }
    else {
        /* the remainder is zero, and in the presence of signed zeroes
           fmod returns different results across platforms; ensure
           it has the same sign as the denominator. */
        mod = copysign(0.0, wx);
    }
    /* snap quotient to nearest integral value */
    if (div) {
        floordiv = floor(div);
        if (div - floordiv > 0.5)
            floordiv += 1.0;
    }
    else {
        /* div is zero - get the same sign as the true quotient */
        floordiv = copysign(0.0, vx / wx); /* zero w/ sign of vx/wx */
    }
    PyFPE_END_PROTECT(floordiv)
    return Py_BuildValue("(dd)", floordiv, mod);
}

static PyObject *
float_floor_div(PyObject *v, PyObject *w)
{
    PyObject *t, *r;

    t = float_divmod(v, w);
    if (t == NULL || t == Py_NotImplemented)
        return t;
    //assert(PyTuple_CheckExact(t));
    //r = PyTuple_GET_ITEM(t, 0);
    Py_INCREF(r);
    Py_DECREF(t);
    return r;
}

/* determine whether x is an odd integer or not;  assumes that
   x is not an infinity or nan. */
#define DOUBLE_IS_ODD_INTEGER(x) (fmod(fabs(x), 2.0) == 1.0)

static PyObject *
float_pow(PyObject *v, PyObject *w, PyObject *z)
{
    double iv, iw, ix;
    int negate_result = 0;

    if ((PyObject *)z != Py_None) {
        PyErr_SetString(PyExc_TypeError, "pow() 3rd argument not "
            "allowed unless all arguments are integers");
        return NULL;
    }

    CONVERT_TO_DOUBLE(v, iv);
    CONVERT_TO_DOUBLE(w, iw);

    /* Sort out special cases here instead of relying on pow() */
    if (iw == 0) {              /* v**0 is 1, even 0**0 */
        return PyFloat_FromDouble(1.0);
    }
    if (Py_IS_NAN(iv)) {        /* nan**w = nan, unless w == 0 */
        return PyFloat_FromDouble(iv);
    }
    if (Py_IS_NAN(iw)) {        /* v**nan = nan, unless v == 1; 1**nan = 1 */
        return PyFloat_FromDouble(iv == 1.0 ? 1.0 : iw);
    }
    if (Py_IS_INFINITY(iw)) {
        /* v**inf is: 0.0 if abs(v) < 1; 1.0 if abs(v) == 1; inf if
         *     abs(v) > 1 (including case where v infinite)
         *
         * v**-inf is: inf if abs(v) < 1; 1.0 if abs(v) == 1; 0.0 if
         *     abs(v) > 1 (including case where v infinite)
         */
        iv = fabs(iv);
        if (iv == 1.0)
            return PyFloat_FromDouble(1.0);
        else if ((iw > 0.0) == (iv > 1.0))
            return PyFloat_FromDouble(fabs(iw)); /* return inf */
        else
            return PyFloat_FromDouble(0.0);
    }
    if (Py_IS_INFINITY(iv)) {
        /* (+-inf)**w is: inf for w positive, 0 for w negative; in
         *     both cases, we need to add the appropriate sign if w is
         *     an odd integer.
         */
        int iw_is_odd = DOUBLE_IS_ODD_INTEGER(iw);
        if (iw > 0.0)
            return PyFloat_FromDouble(iw_is_odd ? iv : fabs(iv));
        else
            return PyFloat_FromDouble(iw_is_odd ?
                                      copysign(0.0, iv) : 0.0);
    }
    if (iv == 0.0) {  /* 0**w is: 0 for w positive, 1 for w zero
                         (already dealt with above), and an error
                         if w is negative. */
        int iw_is_odd = DOUBLE_IS_ODD_INTEGER(iw);
        if (iw < 0.0) {
            PyErr_SetString(PyExc_ZeroDivisionError,
                            "0.0 cannot be raised to a "
                            "negative power");
            return NULL;
        }
        /* use correct sign if iw is odd */
        return PyFloat_FromDouble(iw_is_odd ? iv : 0.0);
    }

    if (iv < 0.0) {
        /* Whether this is an error is a mess, and bumps into libm
         * bugs so we have to figure it out ourselves.
         */
        if (iw != floor(iw)) {
            /* Negative numbers raised to fractional powers
             * become complex.
             */
            return NULL; //PyComplex_Type.tp_as_number->nb_power(v, w, z);
        }
        /* iw is an exact integer, albeit perhaps a very large
         * one.  Replace iv by its absolute value and remember
         * to negate the pow result if iw is odd.
         */
        iv = -iv;
        negate_result = DOUBLE_IS_ODD_INTEGER(iw);
    }

    if (iv == 1.0) { /* 1**w is 1, even 1**inf and 1**nan */
        /* (-1) ** large_integer also ends up here.  Here's an
         * extract from the comments for the previous
         * implementation explaining why this special case is
         * necessary:
         *
         * -1 raised to an exact integer should never be exceptional.
         * Alas, some libms (chiefly glibc as of early 2003) return
         * NaN and set EDOM on pow(-1, large_int) if the int doesn't
         * happen to be representable in a *C* integer.  That's a
         * bug.
         */
        return PyFloat_FromDouble(negate_result ? -1.0 : 1.0);
    }

    /* Now iv and iw are finite, iw is nonzero, and iv is
     * positive and not equal to 1.0.  We finally allow
     * the platform pow to step in and do the rest.
     */
    errno = 0;
    PyFPE_START_PROTECT("pow", return NULL)
    ix = pow(iv, iw);
    PyFPE_END_PROTECT(ix)
    Py_ADJUST_ERANGE1(ix);
    if (negate_result)
        ix = -ix;

    if (errno != 0) {
        /* We don't expect any errno value other than ERANGE, but
         * the range of libm bugs appears unbounded.
         */
        PyErr_SetFromErrno(errno == ERANGE ? PyExc_OverflowError :
                             PyExc_ValueError);
        return NULL;
    }
    return PyFloat_FromDouble(ix);
}

#undef DOUBLE_IS_ODD_INTEGER

static PyObject *
float_neg(PyFloatObject *v)
{
    return PyFloat_FromDouble(-v->ob_fval);
}

static PyObject *
float_abs(PyFloatObject *v)
{
    return PyFloat_FromDouble(fabs(v->ob_fval));
}

static int
float_bool(PyFloatObject *v)
{
    return v->ob_fval != 0.0;
}

static PyObject *
float_is_integer(PyObject *v)
{
    double x = PyFloat_AsDouble(v);
    PyObject *o;

    if (x == -1.0 && PyErr_Occurred())
        return NULL;
    if (!Py_IS_FINITE(x))
        Py_RETURN_FALSE;
    errno = 0;
    PyFPE_START_PROTECT("is_integer", return NULL)
    o = (floor(x) == x) ? Py_True : Py_False;
    PyFPE_END_PROTECT(x)
    if (errno != 0) {
        PyErr_SetFromErrno(errno == ERANGE ? PyExc_OverflowError :
                             PyExc_ValueError);
        return NULL;
    }
    Py_INCREF(o);
    return o;
}

#if 0
static PyObject *
float_is_inf(PyObject *v)
{
    double x = PyFloat_AsDouble(v);
    if (x == -1.0 && PyErr_Occurred())
        return NULL;
    return PyBool_FromLong((long)Py_IS_INFINITY(x));
}

static PyObject *
float_is_nan(PyObject *v)
{
    double x = PyFloat_AsDouble(v);
    if (x == -1.0 && PyErr_Occurred())
        return NULL;
    return PyBool_FromLong((long)Py_IS_NAN(x));
}

static PyObject *
float_is_finite(PyObject *v)
{
    double x = PyFloat_AsDouble(v);
    if (x == -1.0 && PyErr_Occurred())
        return NULL;
    return PyBool_FromLong((long)Py_IS_FINITE(x));
}
#endif

static PyObject *
float_trunc(PyObject *v)
{
    double x = PyFloat_AsDouble(v);
    double wholepart;           /* integral portion of x, rounded toward 0 */

    (void)modf(x, &wholepart);
    /* Try to get out cheap if this fits in a Python int.  The attempt
     * to cast to long must be protected, as C doesn't define what
     * happens if the double is too big to fit in a long.  Some rare
     * systems raise an exception then (RISCOS was mentioned as one,
     * and someone using a non-default option on Sun also bumped into
     * that).  Note that checking for >= and <= LONG_{MIN,MAX} would
     * still be vulnerable:  if a long has more bits of precision than
     * a double, casting MIN/MAX to double may yield an approximation,
     * and if that's rounded up, then, e.g., wholepart=LONG_MAX+1 would
     * yield true from the C expression wholepart<=LONG_MAX, despite
     * that wholepart is actually greater than LONG_MAX.
     */
    if (LONG_MIN < wholepart && wholepart < LONG_MAX) {
        const long aslong = (long)wholepart;
        return PyLong_FromLong(aslong);
    }
    return PyLong_FromDouble(wholepart);
}

/* double_round: rounds a finite double to the closest multiple of
   10**-ndigits; here ndigits is within reasonable bounds (typically, -308 <=
   ndigits <= 323).  Returns a Python float, or sets a Python error and
   returns NULL on failure (OverflowError and memory errors are possible). */

#ifndef PY_NO_SHORT_FLOAT_REPR
/* version of double_round that uses the correctly-rounded string<->double
   conversions from Python/dtoa.c */

static PyObject *
double_round(double x, int ndigits) {

    double rounded;
    Py_ssize_t buflen, mybuflen=100;
    char *buf, *buf_end, shortbuf[100], *mybuf=shortbuf;
    int decpt, sign;
    PyObject *result = NULL;
    _Py_SET_53BIT_PRECISION_HEADER;

    /* round to a decimal string */
    _Py_SET_53BIT_PRECISION_START;
    buf = _Py_dg_dtoa(x, 3, ndigits, &decpt, &sign, &buf_end);
    _Py_SET_53BIT_PRECISION_END;
    if (buf == NULL) {
        PyErr_NoMemory();
        return NULL;
    }

    /* Get new buffer if shortbuf is too small.  Space needed <= buf_end -
    buf + 8: (1 extra for '0', 1 for sign, 5 for exp, 1 for '\0').  */
    buflen = buf_end - buf;
    if (buflen + 8 > mybuflen) {
        mybuflen = buflen+8;
        mybuf = (char *)PyMem_Malloc(mybuflen);
        if (mybuf == NULL) {
            PyErr_NoMemory();
            goto exit;
        }
    }
    /* copy buf to mybuf, adding exponent, sign and leading 0 */
    PyOS_snprintf(mybuf, mybuflen, "%s0%se%d", (sign ? "-" : ""),
                  buf, decpt - (int)buflen);

    /* and convert the resulting string back to a double */
    errno = 0;
    _Py_SET_53BIT_PRECISION_START;
    rounded = _Py_dg_strtod(mybuf, NULL);
    _Py_SET_53BIT_PRECISION_END;
    if (errno == ERANGE && fabs(rounded) >= 1.)
        PyErr_SetString(PyExc_OverflowError,
                        "rounded value too large to represent");
    else
        result = PyFloat_FromDouble(rounded);

    /* done computing value;  now clean up */
    if (mybuf != shortbuf)
        PyMem_Free(mybuf);
  exit:
    _Py_dg_freedtoa(buf);
    return result;
}

#else /* PY_NO_SHORT_FLOAT_REPR */

/* fallback version, to be used when correctly rounded binary<->decimal
   conversions aren't available */

static PyObject *
double_round(double x, int ndigits) {
    double pow1, pow2, y, z;
    if (ndigits >= 0) {
        if (ndigits > 22) {
            /* pow1 and pow2 are each safe from overflow, but
               pow1*pow2 ~= pow(10.0, ndigits) might overflow */
            pow1 = pow(10.0, (double)(ndigits-22));
            pow2 = 1e22;
        }
        else {
            pow1 = pow(10.0, (double)ndigits);
            pow2 = 1.0;
        }
        y = (x*pow1)*pow2;
        /* if y overflows, then rounded value is exactly x */
        if (!Py_IS_FINITE(y))
            return PyFloat_FromDouble(x);
    }
    else {
        pow1 = pow(10.0, (double)-ndigits);
        pow2 = 1.0; /* unused; silences a gcc compiler warning */
        y = x / pow1;
    }

    z = round(y);
    if (fabs(y-z) == 0.5)
        /* halfway between two integers; use round-half-even */
        z = 2.0*round(y/2.0);

    if (ndigits >= 0)
        z = (z / pow2) / pow1;
    else
        z *= pow1;

    /* if computation resulted in overflow, raise OverflowError */
    if (!Py_IS_FINITE(z)) {
        PyErr_SetString(PyExc_OverflowError,
                        "overflow occurred during round");
        return NULL;
    }

    return PyFloat_FromDouble(z);
}

#endif /* PY_NO_SHORT_FLOAT_REPR */

/* round a Python float v to the closest multiple of 10**-ndigits */

static PyObject *
float_round(PyObject *v, PyObject *args)
{
    double x, rounded;
    PyObject *o_ndigits = NULL;
    Py_ssize_t ndigits;

    x = PyFloat_AsDouble(v);
    if (!PyArg_ParseTuple(args, "|O", &o_ndigits))
        return NULL;
    if (o_ndigits == NULL || o_ndigits == Py_None) {
        /* single-argument round or with None ndigits:
         * round to nearest integer */
        rounded = round(x);
        if (fabs(x-rounded) == 0.5)
            /* halfway case: round to even */
            rounded = 2.0*round(x/2.0);
        return PyLong_FromDouble(rounded);
    }

    /* interpret second argument as a Py_ssize_t; clips on overflow */
    ndigits = PyNumber_AsSsize_t(o_ndigits, NULL);
    if (ndigits == -1 && PyErr_Occurred())
        return NULL;

    /* nans and infinities round to themselves */
    if (!Py_IS_FINITE(x))
        return PyFloat_FromDouble(x);

    /* Deal with extreme values for ndigits. For ndigits > NDIGITS_MAX, x
       always rounds to itself.  For ndigits < NDIGITS_MIN, x always
       rounds to +-0.0.  Here 0.30103 is an upper bound for log10(2). */
#define NDIGITS_MAX ((int)((DBL_MANT_DIG-DBL_MIN_EXP) * 0.30103))
#define NDIGITS_MIN (-(int)((DBL_MAX_EXP + 1) * 0.30103))
    if (ndigits > NDIGITS_MAX)
        /* return x */
        return PyFloat_FromDouble(x);
    else if (ndigits < NDIGITS_MIN)
        /* return 0.0, but with sign of x */
        return PyFloat_FromDouble(0.0*x);
    else
        /* finite x, and ndigits is not unreasonably large */
        return double_round(x, (int)ndigits);
#undef NDIGITS_MAX
#undef NDIGITS_MIN
}

static PyObject *
float_float(PyObject *v)
{
    if (PyFloat_CheckExact(v))
        Py_INCREF(v);
    else
        v = PyFloat_FromDouble(((PyFloatObject *)v)->ob_fval);
    return v;
}

typedef enum {
    unknown_format, ieee_big_endian_format, ieee_little_endian_format
} float_format_type;

static float_format_type double_format, float_format;
static float_format_type detected_double_format, detected_float_format;
PyNumberMethods float_as_number = {
    float_add,          /*nb_add*/
    float_sub,          /*nb_subtract*/
    float_mul,          /*nb_multiply*/
    float_rem,          /*nb_remainder*/
    float_divmod,       /*nb_divmod*/
    float_pow,          /*nb_power*/
    (unaryfunc)float_neg, /*nb_negative*/
    (unaryfunc)float_float, /*nb_positive*/
    (unaryfunc)float_abs, /*nb_absolute*/
    (inquiry)float_bool, /*nb_bool*/
    0,                  /*nb_invert*/
    0,                  /*nb_lshift*/
    0,                  /*nb_rshift*/
    0,                  /*nb_and*/
    0,                  /*nb_xor*/
    0,                  /*nb_or*/
    float_trunc,        /*nb_int*/
    0,                  /*nb_reserved*/
    float_float,        /*nb_float*/
    0,                  /* nb_inplace_add */
    0,                  /* nb_inplace_subtract */
    0,                  /* nb_inplace_multiply */
    0,                  /* nb_inplace_remainder */
    0,                  /* nb_inplace_power */
    0,                  /* nb_inplace_lshift */
    0,                  /* nb_inplace_rshift */
    0,                  /* nb_inplace_and */
    0,                  /* nb_inplace_xor */
    0,                  /* nb_inplace_or */
    float_floor_div, /* nb_floor_divide */
    float_div,          /* nb_true_divide */
    0,                  /* nb_inplace_floor_divide */
    0,                  /* nb_inplace_true_divide */
};

PyTypeObject PyFloat_Type;

int
_PyFloat_Init(void)
{
    /* We attempt to determine if this machine is using IEEE
       floating point formats by peering at the bits of some
       carefully chosen values.  If it looks like we are on an
       IEEE platform, the float packing/unpacking routines can
       just copy bits, if not they resort to arithmetic & shifts
       and masks.  The shifts & masks approach works on all finite
       values, but what happens to infinities, NaNs and signed
       zeroes on packing is an accident, and attempting to unpack
       a NaN or an infinity will raise an exception.

       Note that if we're on some whacked-out platform which uses
       IEEE formats but isn't strictly little-endian or big-
       endian, we will fall back to the portable shifts & masks
       method. */

#if SIZEOF_DOUBLE == 8
    {
        double x = 9006104071832581.0;
        if (memcmp(&x, "\x43\x3f\xff\x01\x02\x03\x04\x05", 8) == 0)
            detected_double_format = ieee_big_endian_format;
        else if (memcmp(&x, "\x05\x04\x03\x02\x01\xff\x3f\x43", 8) == 0)
            detected_double_format = ieee_little_endian_format;
        else
            detected_double_format = unknown_format;
    }
#else
    detected_double_format = unknown_format;
#endif

#if SIZEOF_FLOAT == 4
    {
        float y = 16711938.0;
        if (memcmp(&y, "\x4b\x7f\x01\x02", 4) == 0)
            detected_float_format = ieee_big_endian_format;
        else if (memcmp(&y, "\x02\x01\x7f\x4b", 4) == 0)
            detected_float_format = ieee_little_endian_format;
        else
            detected_float_format = unknown_format;
    }
#else
    detected_float_format = unknown_format;
#endif

    double_format = detected_double_format;
    float_format = detected_float_format;

    return 1;
}

int
PyFloat_ClearFreeList(void)
{
    PyFloatObject *f = free_list, *next;
    int i = numfree;
    while (f) {
        next = (PyFloatObject*) Py_TYPE(f);
        PyObject_FREE(f);
        f = next;
    }
    free_list = NULL;
    numfree = 0;
    return i;
}

void
PyFloat_Fini(void)
{
    (void)PyFloat_ClearFreeList();
}

