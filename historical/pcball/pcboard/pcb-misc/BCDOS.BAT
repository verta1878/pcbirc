@echo off

rem un-REM the compiler you will be using from the set of lines down below:

rem set BCCOMPILER=tc30
    set BCCOMPILER=bc31
rem set BCCOMPILER=bc50

goto %bccompiler%
echo Error - BCCOMPILER needs to be defined as tc30, bc31 or bc50
goto done


:tc30
if not '%BCROOT%' == '' goto BC31DONE
  set BCROOT=\TC30
  set BCPGM=%BCROOT%\BIN
  set INCLUDE=%BCROOT%\INCLUDE
  set LIBPATH=%BCROOT%\LIB
  set COMPILER=%BCPGM%\TCC.EXE
  set LINKER=%BCPGM%\TLINK.EXE
  set TLIB=%BCPGM%\TLIB.EXE
  set MAKE=%BCPGM%\MAKE.EXE
  set PATH=%PATH%;%BCPGM%
  rem define TC30 and undefined BC31 and BC50
  set TC30=yes
  set BC31=
  set BC50=

:bc31
if not '%BCROOT%' == '' goto BC31DONE
  set BCROOT=\BC31
  set BCPGM=%BCROOT%
  set INCLUDE=%BCROOT%\INCLUDE
  set LIBPATH=%BCROOT%\LIB
  set COMPILER=%BCPGM%\BCC.EXE
  set LINKER=%BCPGM%\TLINK.EXE
  set TLIB=%BCPGM%\TLIB.EXE
  set MAKE=%BCPGM%\MAKE.EXE
  set PATH=%PATH%;%BCPGM%
  rem define BC31 and undefined TC30 and BC50
  set BC31=yes
  set TC30=
  set BC50=

:bc31done
if not '%ASMROOT%' == '' goto DONE
  set ASMROOT=\TA
  set TASM=%ASMROOT%\TASM.EXE
  set PATH=%PATH%;%ASMROOT%

:bc50
if not '%BCROOT%' == '' goto BC50DONE
  set BCROOT=\BC5
  set BCPGM=%BCROOT%\BIN
  set INCLUDE=%BCROOT%\INCLUDE
  set LIBPATH=%BCROOT%\LIB
  set COMPILER=%BCPGM%\BCC.EXE
  set LINKER=%BCPGM%\TLINK.EXE
  set TLIB=%BCPGM%\TLIB.EXE
  set MAKE=%BCPGM%\MAKE.EXE
  set PATH=%PATH%;%BCPGM%
  rem define BC50 and undefined TC30 and BC31
  set BC50=yes
  set TC30=
  set BC31=

:bc50done
if not '%ASMROOT%' == '' goto DONE
  set ASMROOT=\TA
  set TASM=%ASMROOT%\TASM.EXE
  set PATH=%PATH%;%ASMROOT%

:done
  set LIBSDIR=\LIBS
