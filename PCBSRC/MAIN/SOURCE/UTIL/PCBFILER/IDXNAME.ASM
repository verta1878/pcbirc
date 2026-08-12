;*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
;* The source code in this module is proprietary software belonging to       */
;* Clark Development Company and is part of the PCBoard source code library. */
;* You are granted the right to use this source code for the building of any */
;* of the PCBoard products you have licensed.  Any other usage is forbidden  */
;* without prior written consent from Clark Development Company, Inc.        */
;*                                                                           */
;* Be sure to read the source code license agreement before utilizing any    */
;* of the source code found herein.                                          */
;*                                                                           */
;* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
;*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


Page 66,132

include \proj\lib\h\rules.asi

ifdef __s__
  .model small, pascal
elseifdef __m__
  .model medium, pascal
elseifdef __c__
  .model compact, pascal
elseifdef __l__
  .model large, pascal
endif

if LPROG
   ALIGN2  equ ALIGN 2
   ALIGN16 equ ALIGN 16
else
   ALIGN2  equ ALIGN 1
   ALIGN16 equ ALIGN 1
endif


;   Global MACRO definitions
;   ~~~~~~~~~~~~~~~~~~~~~~~~
;-------------------------------------------------------------------------
Jmps      Macro Dummy
          Jmp   Short Dummy
          EndM
;-------------------------------------------------------------------------

if LPROG
   cutil  segment para public 'code'
   assume    cs:cutil
else
   CSeg@
endif

          public MAKEIDXNAME
;         ALIGN16
MakeIdxName Proc  FileNameStr:ptr, IdxStr:ptr
          Uses  Si, Di

if LDATA
          Push  Ds
          Lds   Si,FileNameStr    ;Now DS:SI points to FileNameStr
          Les   Di,IdxStr         ;Now ES:DI points to IdxStr
else
          Mov   Si,FileNameStr    ;Now DS:SI points to FileNameStr
          Mov   Ax,Ds
          Mov   Es,Ax
          Mov   Di,IdxStr         ;Now ES:DI points to IdxStr
endif

          Cld
          Mov   Dx,2E20h          ;Dh = '.',  Dl = ' '
          Mov   Bx,Di
          Add   Bx,8              ;ES:BX points to the start of the extension
          Mov   Cx,Di
          Add   Cx,11

Min1:     Lodsb
          Cmp   Al,Dh
          Je    FoundDot
          Cmp   Al,Dl
          Jle   FoundEnd
          Stosb
          Cmp   Di,Bx
          Jle   Min1
          Jmps  FoundEnd

FoundDot: Xchg  Bx,Cx
          Sub   Cx,Di
          Mov   Al,Dl
          Rep   Stosb
          Mov   Cx,Bx

Min2:     Lodsb
          Cmp   Al,Dl
          Jle   FoundEnd
          Stosb
          Cmp   Di,Cx
          Jl    Min2
          Jmps  Done

FoundEnd: Sub   Cx,Di
          Mov   Al,Dl
          Rep   Stosb

Done:
if LDATA
          Pop   Ds
endif
          Ret
MakeIdxName EndP

if LPROG
  cutil  ends
else
  CSegEnd@
endif

          End
