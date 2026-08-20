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

include rules.asi

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

INT16     MACRO
          IFDEF KBD3
            INT 3
          ELSE
            INT 16h
          ENDIF
          ENDM
;-------------------------------------------------------------------------

if LPROG
   bgkey  segment para public 'code'
   assume    cs:bgkey
else
   CSeg@
endif

          public BGETKEY2
          ALIGN16
BGETKEY2  Proc  Option:byte
          Cmp   Option,1          ;are we merely TESTING the keyboard?
          Jne   GetKey            ;  no, go get the keystroke
          Mov   Ah,1              ;  yes, ask BIOS for the scan code
          INT16                   ;
          Jnz   GotOne            ;if Z-flag is set then we don't have
          Xor   Ax,Ax             ;one so return FALSE otherwise
          Ret
GotOne:   Mov   Ax,1              ;return TRUE to indicate there's a
          Ret                     ;keystroke waiting to be picked up

GetKey:   Mov   Ah,Option
          INT16
          Ret
BGETKEY2  Endp

if LPROG
  bgkey  ends
else
  CSegEnd@
endif
          End
