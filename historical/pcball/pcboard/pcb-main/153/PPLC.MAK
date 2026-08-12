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
OBJPTH  = d:\proj\pcb\obj\ppl\ # Comment needed to keep \ from continuing to next line
DSTNAME = pplc
MODEL   = l

OBJLST  = d:\proj\pcb\obj\ppl\scomp.obj    \
          d:\proj\pcb\obj\ppl\newscr.obj   \
          d:\proj\pcb\obj\ppl\scrcomp.obj  \
          d:\proj\pcb\obj\ppl\scrmisc.obj  \
          d:\proj\pcb\obj\ppl\pcbmisc.obj  \
          d:\proj\pcb\obj\ppl\var.obj      \
          d:\proj\pcb\obj\ppl\label.obj    \
          d:\proj\pcb\obj\ppl\ceh.obj      \
          d:\proj\pcb\obj\ppl\h2name.obj

LIBLST  = misc_$(MODEL).lib          \
          pcbkit_$(MODEL).lib        \
          math$(MODEL).lib           \
          emu.lib                    \
          c$(MODEL).lib

################################################################################

.path.obj = $(OBJPTH)
.path.c   = d:\proj\pcb\source\compiler

################################################################################

CC = bcc
CO = -c -m$(MODEL)
CD = -DLIB;COMM;$(SRCDEF)

AC = tasm.exe
AO = /m3

LC = tlink
LO = /yx+ /ye-

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

INCPTH = $(INCPTH)e:\tc\h;
INCPTH = $(INCPTH)d:\proj\pcb\source\h;
INCPTH = $(INCPTH)d:\proj\lib\h

LIBPTH = $(LIBPTH)e:\tc\lib;
LIBPTH = $(LIBPTH)d:\proj\lib\bcdos

################################################################################

# Implicit Rules

.c.obj:
        $(CC) $(CO) $(CD) {$< }

{d:\proj\pcb\source\compiler}.cpp.obj:
        $(CC) $(CO) $(CD) {$< }

{d:\proj\pcb\source\ppl}.cpp.obj:
        $(CC) $(CO) $(CD) {$< }

{x:\sdrlib}.cpp.obj:
        $(CC) $(CO) $(CD) {$< }

{d:\proj\pcb\source\compiler}.asm.obj:
        $(AC) $(AO) $<,$(OBJPTH)$&

################################################################################

# Explicit Rules

d:\proj\pcb\obj\ppl\pplc.exe: $(OBJLST)
        $(LC) $(LO) /L$(LIBPTH) @&&|
c0$(MODEL).obj $(**:turboc.cfg=)
d:\proj\pcb\obj\ppl\pplc.exe
# No map file
$(LIBLST)
|

