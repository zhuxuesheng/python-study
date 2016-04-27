#include "Python.h"
#include <stdarg.h>

int main()
{
    #ifdef _DEBUG
    printf("This is debug version...\n");
    #endif
    printf("Python version: %s, hex: %08X\n", PY_VERSION, PY_VERSION_HEX); //patchlevel.h
    void *ptr = PyMem_Malloc(1024);
    PyMem_Free(ptr);
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