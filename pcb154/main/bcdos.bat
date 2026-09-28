@echo off

rem un-rem the compiler you will be using from the set of lines down below:

rem set bccompiler=tc30
set bccompiler=bc31
rem set bccompiler=bc50

goto %bccompiler%
echo error - bccompiler needs to be defined as tc30, bc31 or bc50
goto done


:tc30
if not '%bcroot%' == '' goto bc31done
  set bcroot=\b\t\cpp\300
  set bcpgm=%bcroot%\bin
  set include=%bcroot%\include
  set libpath=%bcroot%\lib
  set compiler=%bcpgm%\tcc.exe
  set linker=%bcpgm%\tlink.exe
  set tlib=%bcpgm%\tlib.exe
  set make=%bcpgm%\make.exe
rem  set path=%path%;%bcpgm%
  rem define tc30 and undefined bc31 and bc50
  set tc30=yes
  set bc31=
  set bc50=

:bc31
if not '%bcroot%' == '' goto bc31done
  set bcroot=%bc31path%
  set bcpgm=%bcroot%\bin
  set include=%bcroot%\include
  set libpath=%bcroot%\lib
  set compiler=%bcpgm%\bcc.exe
  set linker=%bcpgm%\tlink.exe
  set tlib=%bcpgm%\tlib.exe
  set make=%bcpgm%\make.exe
rem  set path=%path%;%bcpgm%
  rem define bc31 and undefined tc30 and bc50
  set bc31=yes
  set tc30=
  set bc50=

:bc31done
if not '%asmroot%' == '' goto done
  set asmroot=%bcpgm%
  set tasm=%asmroot%\tasm.exe
rem  set path=%path%;%asmroot%

:bc50
if not '%bcroot%' == '' goto bc50done
  set bcroot=\bc5
  set bcpgm=%bcroot%\bin
  set include=%bcroot%\include
  set libpath=%bcroot%\lib
  set compiler=%bcpgm%\bcc.exe
  set linker=%bcpgm%\tlink.exe
  set tlib=%bcpgm%\tlib.exe
  set make=%bcpgm%\make.exe
  set path=%path%;%bcpgm%
  rem define bc50 and undefined tc30 and bc31
  set bc50=yes
  set tc30=
  set bc31=

:bc50done
if not '%asmroot%' == '' goto done
  set asmroot=\tasm
  set tasm=%asmroot%\tasm.exe
  set path=%path%;%asmroot%

:done
  set libsdir=..\..\lib
