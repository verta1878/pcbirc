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


public  _int24hnd,_Int24Error,_Int24Flags,_int23hnd

ifdef __s__
  .model small
elseifdef __m__
  .model medium
elseifdef __c__
  .model compact
elseifdef __l__
 .model large
endif

extrn _oldint24hnd:dword

     DATASEG
_Int24Error db     ?
_Int24Flags db     ?

     CODESEG
oldint  dd    ?

PROC _int24hnd
     pushf
     push    ds
     push    ax
     mov     ax,@Data
     mov     ds,ax
     pop     ax
     xchg    ax,di
     cmp     al,55-19             ;Is it "Device no longer exists"?
     je      retry
     mov     [_Int24Error],al
     xchg    ax,di

     test    ah,00010000b         ;Is RETRY allowed?
     jz      fail
     mov     [_Int24Flags],2      ;Tell SHOWERR to retry the operation
     jmp     short exit
fail:mov     [_Int24Flags],4      ;Tell SHOWERR to abort the operation

exit:pop     ds
     popf
     mov     al,3   ; set to FAIL the process (SHOWERR.C will handle it!)
     iret           ; return to who called the int 21h

retry:              ; If we get here it's because we had an INT 24H error 36
     xchg    ax,di  ; (or extended error 55) which means "Network Device no
     pop     ds     ; longer exists".   We'll set up for a RETRY right now
     popf           ; simulating the user entering "R" for RETRY.  We could get
     mov     al,1   ; stuck in a loop continuously retrying this operation but
     iret           ; as soon as the network is back we're in business again!

ENDP

PROC _int23hnd
     iret
ENDP
     END
