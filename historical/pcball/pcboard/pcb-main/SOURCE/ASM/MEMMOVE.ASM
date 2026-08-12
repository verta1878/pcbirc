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



include \proj\pcb\source\h\rules.asi

ifdef __s__
  .model small
elseifdef __m__
  .model medium
elseifdef __c__
  .model compact
elseifdef __l__
  .model large
endif

       USE386 = 0

ifdef  CPU386
ifndef LIB
ifndef DEBUG
       USE386 = 1
endif
endif
endif

if LPROG
   ALIGN2  equ ALIGN 2
   ALIGN16 equ ALIGN 16
else
   ALIGN2  equ ALIGN 1
   ALIGN16 equ ALIGN 1
endif

if LPROG
   memfuncs  segment para public 'code'
   assume    cs:memfuncs
else
   CSeg@
endif

IF     USE386
          public _memmove,_memcpy,_strcpy
ENDIF
          public FMEMCPY


;------------------------------------------------------------------------
; void * memmove(void *dest,const void *src, size_t count);
;------------------------------------------------------------------------

IF     USE386
          ALIGN16
_memmove  proc   c dest:ptr, srce:ptr, count:word
          mov    cx,count              ;CX = number of bytes to move
          jcxz   mmov_skip             ;if count=0, we're done

.386
          mov    bx,si                 ;save register variables in EBX
          shl    ebx,16                ;save SI in top of EBX register
          mov    bx,di                 ;save DI in bottom of EBX register

if LDATA
          mov    dx,ds
          shl    edx,16                ;save DS in top of EDX register
          les    di,dest               ;ES:DI points to destination
          lds    si,srce               ;DS:SI points to source
          mov    dx,es
          mov    ax,di                 ;DX:AX points to dest (RETURN VALUE)
else
          mov    ax,ds
          mov    es,ax                 ;force ES into DS (small model)
          mov    di,dest               ;ES:DI points to destination
          mov    si,srce               ;DS:SI points to source
          mov    ax,di                 ;DS:AX points to dest (RETURN VALUE)
endif

          cmp    di,si                 ;do we have possible overlap?
          jb     mmov3                 ;if not, go do it
          je     mmov_ret              ;if dest=src then do nothing

          mov    ax,si
          add    ax,cx                 ;find end point of source
          cmp    di,ax                 ;do we overlap with dest?
          mov    ax,di
          jae    mmov3                 ;no overlap, go do it

;move bytes in a reverse direction

          add    si,cx                 ;copy from end of string back
          add    di,cx
          std                          ;set reverse direction
          dec    si                    ;point to last byte
          dec    di
          shr    cx,1                  ;convert to words
          jnc    mmov1                 ;carry is set if we need to move a byte
          movsb                        ;move single byte
mmov1:    dec    si                    ;set index back for word move
          dec    di

          shr    cx,1                  ;convert to double words
          jnc    mmov2                 ;carry is set if we need to move a word
          movsw                        ;move single word
mmov2:    sub    di,2                  ;adjust ptr back for 4 byte move
          sub    si,2

          rep    movsd
          cld

mmov_ret: mov    di,bx                 ;restore DI from bottom of EBX
          shr    ebx,16
          mov    si,bx                 ;restore SI from top of EBX
if LDATA
          mov    ebx,edx               ;don't mess with DX because
          shr    ebx,16                ;DX:AX points to the return address
          mov    ds,bx                 ;restore DS from top of EDX register
endif
.8086
mmov_skip:ret
.386

;move bytes in a forward direction

mmov3:    cld
          shr    cx,1                  ;convert byte count to words
          jnc    mmov4                 ;carry is set if we need to move a byte
          movsb

mmov4:    shr    cx,1                  ;convert to double words
          rep    movsd                 ;move double words
          adc    cx,cx                 ;if carry was odd
          rep    movsw                 ;then movsw to move the last word

          mov    di,bx                 ;restore DI from bottom of EBX
          shr    ebx,16
          mov    si,bx                 ;restore SI from top of EBX
if LDATA
          mov    ebx,edx               ;don't mess with DX because
          shr    ebx,16                ;DX:AX points to the return address
          mov    ds,bx                 ;restore DS from top of EDX register
endif
.8086
          ret
_memmove  endp


;------------------------------------------------------------------------
; void * memcpy(void *dest,const void *src, size_t count);
;------------------------------------------------------------------------

          ALIGN16
_memcpy   proc   c dest:ptr, srce:ptr, count:word
          mov    cx,count              ;CX = number of bytes to move
          jcxz   mcpyret
.386

          mov    bx,si                 ;save register variables in EBX
          shl    ebx,16                ;save SI in top of EBX register
          mov    bx,di                 ;save DI in bottom of EBX register

if LDATA
          mov    dx,ds
          shl    edx,16                ;save DS in top of EDX register
          les    di,dest               ;ES:DI points to destination
          lds    si,srce               ;DS:SI points to source
          mov    dx,es
          mov    ax,di                 ;DX:AX points to dest (RETURN VALUE)
else
          mov    ax,ds
          mov    es,ax                 ;force ES into DS (small model)
          mov    di,dest               ;ES:DI points to destination
          mov    si,srce               ;DS:SI points to source
          mov    ax,di                 ;DS:AX points to dest (RETURN VALUE)
endif

          cld
          shr    cx,1                  ;convert byte count to words
          jnc    mcpy1                 ;carry is set if we need to move a byte
          movsb

mcpy1:    shr    cx,1                  ;convert to double words
          rep    movsd                 ;move double words
          adc    cx,cx                 ;if carry was odd
          rep    movsw                 ;then movsw to move the last word

          mov    di,bx                 ;restore DI from bottom of EBX
          shr    ebx,16
          mov    si,bx                 ;restore SI from top of EBX
if LDATA
          mov    ebx,edx               ;don't mess with DX because
          shr    ebx,16                ;DX:AX points to the return address
          mov    ds,bx                 ;restore DS from top of EDX register
endif
mcpyret:
.8086
          ret
_memcpy   endp


;------------------------------------------------------------------------
; void * strcpy(void *dest,const *void *src);
;------------------------------------------------------------------------

          ALIGN16
_strcpy   proc   c dest:ptr, srce:ptr
          cld
.386

          mov    bx,si                 ;save register variables in EBX
          shl    ebx,16                ;save SI in top of EBX register
          mov    bx,di                 ;save DI in bottom of EBX register

if LDATA
          mov    dx,ds
          shl    edx,16                ;save DS in top of EDX register
          les    di,srce               ;ES:DI points to source
          mov    si,di
          mov    ax,es
          mov    ds,ax                 ;DS:SI points to source
else
          mov    ax,ds
          mov    es,ax                 ;force ES into DS (small model)
          mov    di,srce               ;ES:DI points to source
          mov    si,di                 ;DS:SI points to source
endif

          xor    al,al                 ;scan the source for the NULL terminator
          mov    cx,0FFFFh
          repnz  scasb
          not    cx                    ;CX equals the length of the string

if LDATA
          les    di,dest               ;ES:DI points to destination
          mov    dx,es
          mov    ax,di                 ;DX:AX points to dest (RETURN VALUE)
else
          mov    di,dest               ;ES:DI points to destination
          mov    ax,di                 ;AX points to dest (RETURN VALUE)
endif

          shr    cx,1                  ;convert byte count to words
          jnc    scpy1                 ;carry is set if we need to move a byte
          movsb

scpy1:    shr    cx,1                  ;convert to double words
          rep    movsd                 ;move double words
          adc    cx,cx                 ;if carry was odd
          rep    movsw                 ;then movsw to move the last word

          mov    di,bx                 ;restore DI from bottom of EBX
          shr    ebx,16
          mov    si,bx                 ;restore SI from top of EBX
if LDATA
          mov    ebx,edx               ;don't mess with DX because
          shr    ebx,16                ;DX:AX points to the return address
          mov    ds,bx                 ;restore DS from top of EDX register
endif
.8086
          ret
_strcpy   endp


;------------------------------------------------------------------------
; void * pascal fmemcpy(void far *dest,void far *src,size_t len);
;------------------------------------------------------------------------

          ALIGN16
FMEMCPY   proc   pascal fdest:far ptr, fsrce:far ptr, fcount:word
          mov    dx,di                 ;save register variables:
          mov    bx,si                 ;DI into DX, SI into BX
          mov    ax,ds                 ;save DS in AX
.386

          cld
          les    di,fdest              ;ES:DI points to destination
          lds    si,fsrce              ;DS:SI points to source
          mov    cx,fcount

          shr    cx,1                  ;convert byte count to words
          jnc    fcpy1                 ;carry is set if we need to move a byte
          movsb

fcpy1:    shr    cx,1                  ;convert to double words
          rep    movsd                 ;move double words
          adc    cx,cx                 ;if carry was odd
          rep    movsw                 ;then movsw to move the last word

          mov    si,bx                 ;restore register variables
          mov    di,dx
          mov    ds,ax

.8086
          ret
FMEMCPY   endp

ELSE

;  FMEMCPY is duplicated here for the sake of the code which is not 386-
;  specific - this FMEMCPY will work on all CPU's

FMEMCPY   proc   pascal fdest:far ptr, fsrce: far ptr, fcount: word
          mov    dx,di                 ;save register variables:
          mov    bx,si                 ;DI into DX, SI into BX
          mov    ax,ds                 ;save DS in AX

          cld
          les    di,fdest              ;ES:DI points to destination
          lds    si,fsrce              ;DS:SI points to source
          mov    cx,fcount

          shr    cx,1                  ;convert byte count to words
          rep    movsw
          adc    cx,0
          rep    movsb

          mov    si,bx                 ;restore register variables
          mov    di,dx
          mov    ds,ax
          ret
FMEMCPY   endp
ENDIF


if LPROG
  memfuncs  ends
else
  CSegEnd@
endif
          end
