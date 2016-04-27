@echo off

rem Predefined MACROs, include search path
rem don't use INCLUDE macro, it is used by VC env batch
set MACRO_DEFINE=/D_DEBUG /DPy_BUILD_CORE /DPy_NO_ENABLE_SHARED
set HEAD_FOLDER=/I..\Include /I..\PC
set CC=cl /Zi /c %MACRO_DEFINE% %HEAD_FOLDER%

@echo on
%CC% ..\Objects\obmalloc.c
%CC% test.c
link /out:test.exe *.obj
