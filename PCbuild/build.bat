@echo off

rem Predefined MACROs, include search path
rem don't use INCLUDE macro, it is used by VC env batch
set MACRO_DEFINE=/DPy_BUILD_CORE /DPy_NO_ENABLE_SHARED
set HEAD_FOLDER=/I..\Include /I..\PC
set CC=cl /Zi /w /c %MACRO_DEFINE% %HEAD_FOLDER%

@echo on

REM ** Object **
%CC% ..\Objects\obmalloc.c
%CC% ..\Objects\object.c
%CC% ..\Objects\longobject.c
%CC% ..\Objects\boolobject.c
%CC% ..\Objects\floatobject.c
%CC% ..\Objects\tupleobject.c
%CC% ..\Objects\bytesobject.c
%CC% ..\Objects\bytearrayobject.c
%CC% ..\Objects\complexobject.c
%CC% ..\Objects\listobject.c
%CC% ..\Objects\dictobject.c
%CC% ..\Objects\abstract.c
%CC% ..\Objects\setobject.c
%CC% ..\Objects\rangeobject.c
%CC% ..\Objects\memoryobject.c
%CC% ..\Objects\odictobject.c
%CC% ..\Objects\enumobject.c
%CC% ..\Objects\sliceobject.c
%CC% ..\Objects\structseq.c
%CC% ..\Objects\moduleobject.c
%CC% ..\Objects\bytes_methods.c
%CC% ..\Objects\exceptions.c
%CC% ..\Objects\unicodectype.c
%CC% ..\Objects\unicodeobject.c
%CC% ..\Objects\codeobject.c

REM ** Python **
%CC% ..\Python\pyctype.c
%CC% ..\Python\pyhash.c
%CC% ..\Python\pystrtod.c
%CC% ..\Python\dtoa.c
%CC% ..\Python\modsupport.c
%CC% ..\Python\errors.c
%CC% ..\Python\getargs.c
%CC% ..\Python\pystate.c

%CC% test.c
link /out:test.exe /debug *.obj
