#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#* The information in this module is proprietary software belonging to       */
#* Clark Development Company and is part of the PCBoard source code library. */
#* You are granted the right to use this information for the building of any */
#* of the PCBoard products you have licensed.  Any other usage is forbidden  */
#* without prior written consent from Clark Development Company, Inc.        */
#*                                                                           */
#* Be sure to read the source code license agreement before utilizing any    */
#* of the source code found herein.                                          */
#*                                                                           */
#* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
#*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


#=============================================================
#
#       ZMODEM.MAK - Makefile for ZMODEM Program
#
# NOTE:  This makefile creates both ZMSEND and ZMRECV at once
# and along the way it creates a ZMODEM.LIB from which both of
# them pull most of their .OBJ's.
#
#=============================================================

.SILENT
.AUTODEPEND

PROGNAME = ZMODEM

ROOT     = \PROJ
SOURCE1  = SOURCE
SOURCE2  = $(ROOT)\PCB\SOURCE\MODEM
SOURCE3  = $(ROOT)\PCB\SOURCE\DOS
SOURCE4  = $(ROOT)\PCB\SOURCE\ASM
OBJDIR   = $(BCCOMPILER)
LIBROOT  = $(ROOT)\LIB
LIBH     = $(LIBROOT)\H
LIBLIB   = $(LIBROOT)\BCDOS\$(BCCOMPILER)

CFG      = $(PROGNAME).CFG
MAK      = $(PROGNAME).MAK

MDL      = l

INCLUDEPATH = $(INCLUDE);$(LIBH);SOURCE;$(ROOT)\PCB\SOURCE\H

#=============================================================

!if $(DEBUG)
CODEOPT=-DDEBUG
!endif

COPT = -c -m$(MDL) -n$(OBJDIR)

!if $d(BC50)
#leave out -Oe due to a bug in Borland C 4.0 thru 5.0
COPT = $(COPT) -Obglmptv
!elif $d(TC30)
#leave out all of the extra -Oxxx switches for TC 3.0 because they aren't valid
!elif $d(BC31)
COPT = $(COPT) -Oebglmptv
!endif

ASMOPT = /m /mx /t /D__$(MDL)__                 # Assembler options

#=============================================================

.PATH.obj = $(OBJDIR)
.PATH.asm = $(SOURCE4)
.PATH.c   = $(SOURCE1)

#=============================================================

.c.obj:
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) {$< }

.asm.obj:
  tasm $(ASMOPT) $(.PATH.asm)\$.,$(.PATH.obj)\$&

#=============================================================

ZMSEND_DEPENDENCIES =   \
  $(OBJDIR)\ZMSEND.OBJ  \
  $(OBJDIR)\ZMODEM.LIB  \
  $(LIBLIB)\pcbkit_$(MDL).lib

ZMRECV_DEPENDENCIES =   \
  $(OBJDIR)\ZMRECV.OBJ  \
  $(OBJDIR)\ZMODEM.LIB  \
  $(LIBLIB)\pcbkit_$(MDL).lib

LIB_DEPENDENCIES =        \
  $(OBJDIR)\zmcan.obj     \
  $(OBJDIR)\zmcount.obj   \
  $(OBJDIR)\zmcrc.obj     \
  $(OBJDIR)\zmdisp.obj    \
  $(OBJDIR)\zmerror.obj   \
  $(OBJDIR)\zmgbye.obj    \
  $(OBJDIR)\zmgbyeak.obj  \
  $(OBJDIR)\zmgetbyt.obj  \
  $(OBJDIR)\zmgethdr.obj  \
  $(OBJDIR)\zmgethex.obj  \
  $(OBJDIR)\zmgraph.obj   \
  $(OBJDIR)\zmgrecv.obj   \
  $(OBJDIR)\zminsync.obj  \
  $(OBJDIR)\zmnoxrd.obj   \
  $(OBJDIR)\zmprchdr.obj  \
  $(OBJDIR)\zmputhex.obj  \
  $(OBJDIR)\zmputsec.obj  \
  $(OBJDIR)\zmputstr.obj  \
  $(OBJDIR)\zmrbh32.obj   \
  $(OBJDIR)\zmrbhdr.obj   \
  $(OBJDIR)\zmrdbyte.obj  \
  $(OBJDIR)\zmrd16.obj    \
  $(OBJDIR)\zmrd32.obj    \
  $(OBJDIR)\zmrd32r.obj   \
  $(OBJDIR)\zmrdata.obj   \
  $(OBJDIR)\zmrecov.obj   \
  $(OBJDIR)\zmrfile.obj   \
  $(OBJDIR)\zmrfiles.obj  \
  $(OBJDIR)\zmrhhdr.obj   \
  $(OBJDIR)\zmsbh32.obj   \
  $(OBJDIR)\zmsbhdr.obj   \
  $(OBJDIR)\zmsd16.obj    \
  $(OBJDIR)\zmsd32.obj    \
  $(OBJDIR)\zmsd32r.obj   \
  $(OBJDIR)\zmsdata.obj   \
  $(OBJDIR)\zmsfdata.obj  \
  $(OBJDIR)\zmsfname.obj  \
  $(OBJDIR)\zmshhdr.obj   \
  $(OBJDIR)\zmsinit.obj   \
  $(OBJDIR)\zmstart.obj   \
  $(OBJDIR)\zmstore.obj   \
  $(OBJDIR)\zmtry.obj     \
  $(OBJDIR)\zmvars.obj    \
  $(OBJDIR)\zmwrlog.obj   \
  $(OBJDIR)\zmzpbyte.obj  \
  $(OBJDIR)\zmspool.obj   \
  $(OBJDIR)\modem.obj     \
  $(OBJDIR)\async.obj     \
  $(OBJDIR)\devioctl.obj  \
  $(OBJDIR)\dostime.obj   \
  $(OBJDIR)\usernet.obj   \
  $(OBJDIR)\dlpath.obj

#=============================================================
#
# Because we are building two separate .EXE files, we need a single target
# to cause them to be built, so we'll create a file called "done".

$(OBJDIR)\done:  $(OBJDIR)\zmsend.exe \
                 $(OBJDIR)\zmrecv.exe
    echo all done > $(OBJDIR)\done

#=============================================================

$(OBJDIR)\zmsend.exe: $(CFG) $(ZMSEND_DEPENDENCIES)
!if $d(COMMDRV)
  $(LINKER) /x/c/L$(LIBPATH) c0$(MDL).obj $(LIBLIB)\toolkit\large\commdrv.obj @&&|
$(OBJDIR)\zmsend.obj+
$(LIBLIB)\toolkit\large\noansi.obj+
$(LIBLIB)\toolkit\large\nochat.obj+
$(LIBLIB)\toolkit\large\nodisp.obj+
$(LIBLIB)\toolkit\large\nohelp.obj+
$(LIBLIB)\toolkit\large\noinput.obj+
$(LIBLIB)\toolkit\large\nolang.obj+
$(LIBLIB)\toolkit\large\noprint.obj+
$(LIBLIB)\toolkit\large\noscreen.obj+
$(LIBLIB)\toolkit\large\noshell.obj+
$(LIBLIB)\toolkit\large\nostatus.obj+
$(LIBLIB)\toolkit\large\nosys.obj+
$(LIBLIB)\toolkit\large\notxt.obj+
$(LIBLIB)\toolkit\large\noupdsys.obj+
$(LIBLIB)\toolkit\large\noxlate.obj
$(OBJDIR)\zmsend.exe
                # no map file
$(OBJDIR)\zmodem.lib+
$(LIBLIB)\pcbkit_$(MDL).lib+
$(LIBLIB)\screen_$(MDL).lib+
$(LIBLIB)\misc_$(MDL).lib+
$(LIBSDIR)\commdrv\lib\commdrbl.lib+
$(LIBSDIR)\commdrv\lib\libsbl.lib+
math$(MDL).lib+
emu.lib+
c$(MDL).lib
|
!else
  $(LINKER) /x/c/L$(LIBPATH) c0$(MDL).obj $(LIBLIB)\toolkit\large\fossil.obj @&&|
$(OBJDIR)\zmsend.obj+
$(LIBLIB)\toolkit\large\noansi.obj+
$(LIBLIB)\toolkit\large\nochat.obj+
$(LIBLIB)\toolkit\large\nodisp.obj+
$(LIBLIB)\toolkit\large\nohelp.obj+
$(LIBLIB)\toolkit\large\noinput.obj+
$(LIBLIB)\toolkit\large\nolang.obj+
$(LIBLIB)\toolkit\large\noprint.obj+
$(LIBLIB)\toolkit\large\noscreen.obj+
$(LIBLIB)\toolkit\large\noshell.obj+
$(LIBLIB)\toolkit\large\nostatus.obj+
$(LIBLIB)\toolkit\large\nosys.obj+
$(LIBLIB)\toolkit\large\notxt.obj+
$(LIBLIB)\toolkit\large\noupdsys.obj+
$(LIBLIB)\toolkit\large\noxlate.obj
$(OBJDIR)\zmsend.exe
                # no map file
$(OBJDIR)\zmodem.lib+
$(LIBLIB)\pcbkit_$(MDL).lib+
$(LIBLIB)\screen_$(MDL).lib+
$(LIBLIB)\misc_$(MDL).lib+
math$(MDL).lib+
emu.lib+
c$(MDL).lib
|
!endif

#=============================================================

$(OBJDIR)\zmrecv.exe: $(CFG) $(ZMRECV_DEPENDENCIES)
!if $d(COMMDRV)
  $(LINKER) /x/c/L$(LIBPATH) c0$(MDL).obj $(LIBLIB)\toolkit\large\commdrv.obj @&&|
$(OBJDIR)\zmrecv.obj+
$(LIBLIB)\toolkit\large\noansi.obj+
$(LIBLIB)\toolkit\large\nochat.obj+
$(LIBLIB)\toolkit\large\nodisp.obj+
$(LIBLIB)\toolkit\large\nohelp.obj+
$(LIBLIB)\toolkit\large\noinput.obj+
$(LIBLIB)\toolkit\large\nolang.obj+
$(LIBLIB)\toolkit\large\noprint.obj+
$(LIBLIB)\toolkit\large\noscreen.obj+
$(LIBLIB)\toolkit\large\noshell.obj+
$(LIBLIB)\toolkit\large\nostatus.obj+
$(LIBLIB)\toolkit\large\nosys.obj+
$(LIBLIB)\toolkit\large\notxt.obj+
$(LIBLIB)\toolkit\large\noupdsys.obj+
$(LIBLIB)\toolkit\large\noxlate.obj
$(OBJDIR)\zmrecv.exe
                # no map file
$(OBJDIR)\zmodem.lib+
$(LIBLIB)\pcbkit_$(MDL).lib+
$(LIBLIB)\screen_$(MDL).lib+
$(LIBLIB)\misc_$(MDL).lib+
$(LIBSDIR)\commdrv\lib\commdrbl.lib+
$(LIBSDIR)\commdrv\lib\libsbl.lib+
math$(MDL).lib+
emu.lib+
c$(MDL).lib
|
!else
  $(LINKER) /x/c/L$(LIBPATH) c0$(MDL).obj $(LIBLIB)\toolkit\large\fossil.obj @&&|
$(OBJDIR)\zmrecv.obj+
$(LIBLIB)\toolkit\large\noansi.obj+
$(LIBLIB)\toolkit\large\nochat.obj+
$(LIBLIB)\toolkit\large\nodisp.obj+
$(LIBLIB)\toolkit\large\nohelp.obj+
$(LIBLIB)\toolkit\large\noinput.obj+
$(LIBLIB)\toolkit\large\nolang.obj+
$(LIBLIB)\toolkit\large\noprint.obj+
$(LIBLIB)\toolkit\large\noscreen.obj+
$(LIBLIB)\toolkit\large\noshell.obj+
$(LIBLIB)\toolkit\large\nostatus.obj+
$(LIBLIB)\toolkit\large\nosys.obj+
$(LIBLIB)\toolkit\large\notxt.obj+
$(LIBLIB)\toolkit\large\noupdsys.obj+
$(LIBLIB)\toolkit\large\noxlate.obj
$(OBJDIR)\zmrecv.exe
                # no map file
$(OBJDIR)\zmodem.lib+
$(LIBLIB)\pcbkit_$(MDL).lib+
$(LIBLIB)\screen_$(MDL).lib+
$(LIBLIB)\misc_$(MDL).lib+
math$(MDL).lib+
emu.lib+
c$(MDL).lib
|
!endif


#=============================================================

$(OBJDIR)\zmodem.lib: $(CFG) $(LIB_DEPENDENCIES)
  $(TLIB) /e $(OBJDIR)\zmodem.lib @&&|
+$(OBJDIR)\zmcan.obj    &
+$(OBJDIR)\zmcount.obj  &
+$(OBJDIR)\zmcrc.obj    &
+$(OBJDIR)\zmdisp.obj   &
+$(OBJDIR)\zmerror.obj  &
+$(OBJDIR)\zmgbye.obj   &
+$(OBJDIR)\zmgbyeak.obj &
+$(OBJDIR)\zmgetbyt.obj &
+$(OBJDIR)\zmgethdr.obj &
+$(OBJDIR)\zmgethex.obj &
+$(OBJDIR)\zmgraph.obj  &
+$(OBJDIR)\zmgrecv.obj  &
+$(OBJDIR)\zminsync.obj &
+$(OBJDIR)\zmnoxrd.obj  &
+$(OBJDIR)\zmprchdr.obj &
+$(OBJDIR)\zmputhex.obj &
+$(OBJDIR)\zmputsec.obj &
+$(OBJDIR)\zmputstr.obj &
+$(OBJDIR)\zmrbh32.obj  &
+$(OBJDIR)\zmrbhdr.obj  &
+$(OBJDIR)\zmrd16.obj   &
+$(OBJDIR)\zmrd32.obj   &
+$(OBJDIR)\zmrd32r.obj  &
+$(OBJDIR)\zmrdata.obj  &
+$(OBJDIR)\zmrdbyte.obj &
+$(OBJDIR)\zmrecov.obj  &
+$(OBJDIR)\zmrfile.obj  &
+$(OBJDIR)\zmrfiles.obj &
+$(OBJDIR)\zmrhhdr.obj  &
+$(OBJDIR)\zmsbh32.obj  &
+$(OBJDIR)\zmsbhdr.obj  &
+$(OBJDIR)\zmsd16.obj   &
+$(OBJDIR)\zmsd32.obj   &
+$(OBJDIR)\zmsd32r.obj  &
+$(OBJDIR)\zmsdata.obj  &
+$(OBJDIR)\zmsfdata.obj &
+$(OBJDIR)\zmsfname.obj &
+$(OBJDIR)\zmshhdr.obj  &
+$(OBJDIR)\zmsinit.obj  &
+$(OBJDIR)\zmstart.obj  &
+$(OBJDIR)\zmstore.obj  &
+$(OBJDIR)\zmtry.obj    &
+$(OBJDIR)\zmvars.obj   &
+$(OBJDIR)\zmwrlog.obj  &
+$(OBJDIR)\zmzpbyte.obj &
+$(OBJDIR)\zmspool.obj  &
+$(OBJDIR)\modem.obj    &
+$(OBJDIR)\async.obj    &
+$(OBJDIR)\devioctl.obj &
+$(OBJDIR)\dostime.obj  &
+$(OBJDIR)\usernet.obj  &
+$(OBJDIR)\dlpath.obj
|

#=============================================================

# rules for individual files where necessary

$(OBJDIR)\MODEM.OBJ: $(SOURCE2)\MODEM.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(SOURCE2)\MODEM.C

$(OBJDIR)\DEVIOCTL.OBJ: $(SOURCE2)\DEVIOCTL.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(SOURCE2)\DEVIOCTL.C

$(OBJDIR)\DOSTIME.OBJ: $(SOURCE3)\DOSTIME.C
  $(COMPILER) +$(CFG) $(COPT) $(CODEOPT) $(SOURCE3)\DOSTIME.C

#=============================================================

$(CFG): $(MAK)
  copy &&|
-wbbf
-wbig
-wdpu
-wdup
-weas
-wext
-wpin
-wret
-wstu
-wsus
-wvoi
-wzdi
-wamb
-wamp
-wasm
-waus
-wccc
-wdef
-weff
-wias
-will
-wnod
-wpar
-wpia
-wpro
-wrch
-wrvl
-wstv
-wuse
-wcln
-wcpt
-wrng
-wrpt
-wsig
-wucp
-wbei
-wdsz
-whid
-wibc
-winl
-wlin
-wlvc
-wmpc
-wmpd
-wncf
-wnci
-wnst
-wnvf
-wobi
-wofp
-wovl
-wpre
-f-
-ff-
-C
-K
-G
-O
-P
-Z
-k-
-d
-I$(INCLUDEPATH)
-L$(LIBPATH)
-DLIB
-DCOMM
-DMULTIPORT
-DCOMMDRV
-DFOSSIL
-DZMODEM
-DOSDRIVER
| $(CFG)

#=============================================================
