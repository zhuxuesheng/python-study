#ifndef Py_CEVAL_H
#define Py_CEVAL_H
#ifdef __cplusplus
extern "C" {
#endif



#ifndef Py_LIMITED_API
PyAPI_FUNC(int) _PyEval_SliceIndex(PyObject *, Py_ssize_t *);
PyAPI_FUNC(void) _PyEval_SignalAsyncExc(void);
#endif


#ifdef __cplusplus
}
#endif
#endif /* !Py_CEVAL_H */
