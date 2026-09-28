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


.NOSILENT
.AUTODEPEND

!ifndef ROOT
ROOT     = \OUT
!endif
!ifndef BRANCH
BRANCH   = PWA153
!endif
!ifndef CVER
CVER     = $(BCCOMPILER)
!endif
!ifndef SRC
SRC      = \PCB153
!endif
!ifndef TKIT
TKIT     = \TOOLKIT\PWA153
!endif
!ifndef LIBSDIR
LIBSDIR  = \PCBCBASE
!endif
OUTDIR   = $(ROOT)\$(BRANCH)
SDK      = $(OUTDIR)\SDK\$(CVER)
SDKLIB   = $(SDK)\LIB
SDKOBJ   = $(SDK)\OBJ

# Process DEBUG
#DEBUG = 1
#TD = 1
#DEBUGSCR = 1
#DBGNEWDEL = 1
DBASE = 1
#___USE_VAR___ = 1

#!if $d(___USE_VARS___)
#___USE_VAR___ = 1
#!endif

!if $d(DBASE)
DBASE = 1
!endif

!if $d(DEBUG) && !$d(TD)
TD = 1
!endif

!if $d(DEBUG) && !$d(DEBUGSCR)
DEBUGSCR = 1
!endif

!if $d(DEBUG) && !$d(DBGNEWDEL)
DBGNEWDEL = 1
!endif

!if $d(DEBUG) && !$d(ERRBRK)
ERRBRK = 1
!endif

SRCDEF  = ___COMP___
OBJPTH  = obj\ppl
DSTNAME = pplc
MODEL   = l

OBJLST  = $(OBJPTH)\scomp.obj    \
          $(OBJPTH)\newscr.obj   \
          $(OBJPTH)\scrcomp.obj  \
          $(OBJPTH)\scrmisc.obj  \
          $(OBJPTH)\pcbmisc.obj  \
          $(OBJPTH)\var.obj      \
          $(OBJPTH)\label.obj    \
          $(OBJPTH)\ceh.obj      \
          $(OBJPTH)\h2name.obj

LIBLST  = country$(MODEL).386        \
          dos_$(MODEL).386           \
          misc_$(MODEL).386          \
          system_$(MODEL).386        \
          $(SDKLIB)\PCBKIT_$(MODEL).LIB     \
          math$(MODEL).lib           \
          emu.lib                    \
          c$(MODEL).lib

################################################################################

.path.obj = $(OBJPTH)
.path.c   = source\compiler

################################################################################

CC = bcc
CO = -c -m$(MODEL)
CD = -DLIB;COMM;$(SRCDEF)

AC = tasm.exe
AO = /m3

LC = tlink
LO = /yx+ /ye- /x

!if $d(TD)
CO = $(CO) -v
LO = $(LO) /v
!endif

!if $d(___USE_VAR___)
CD = $(CD);___USE_VAR___
!endif

!if $d(DEBUG)
CD = $(CD);DEBUG
!endif

!if $d(DEBUGSCR)
CD = $(CD);DEBUGSCR
!endif

!if $d(DBGNEWDEL)
CD = $(CD);DBGNEWDEL
!endif

!if $d(ERRBRK)
CD = $(CD);ERRBRK
!endif

!if $d(PCB_DEMO)
CD = $(CD);PCB_DEMO
!endif

!if $d(386)
CO = $(CO) -3
!endif

!if $d(PCB152)
CD = $(CD);PCB152
!endif

################################################################################





LIBPTH = $(LIBPTH)$(LIBPATH)
LIBPTH = $(LIBPTH);$(SDKLIB)

################################################################################

# Implicit Rules

.c.obj:
        $(CC) +153\pplc.cfg -P $(CO) $(CD) {$< }

{source\compiler}.cpp.obj:
        $(CC) +153\pplc.cfg $(CO) $(CD) {$< }

{source\ppl}.cpp.obj:
        $(CC) +153\pplc.cfg $(CO) $(CD) {$< }

#{x:\sdrlib}.cpp.obj:
#        $(CC) $(CO) $(CD) {$< }

{source\compiler}.asm.obj:
        $(AC) $(AO) $<,$(OBJPTH)\$&

################################################################################

# Explicit Rules

$(OBJPTH)\pplc.exe: $(OBJLST)
        $(LC) $(LO) /L$(LIBPTH) @&&|
c0$(MODEL).obj $(**:turboc.cfg=)
$(OBJPTH)\pplc.exe
# No map file
$(LIBLST)
|


#=============================================================
#       CLEAN - remove everything this makefile produces
#=============================================================

CLEAN:
  -if exist $(OBJPTH)\*.obj del $(OBJPTH)\*.obj
  -if exist $(OBJPTH)\*.map del $(OBJPTH)\*.map
  -if exist $(OBJPTH)\*.res del $(OBJPTH)\*.res
  -if exist $(OBJPTH)\PPLC.EXE del $(OBJPTH)\PPLC.EXE
