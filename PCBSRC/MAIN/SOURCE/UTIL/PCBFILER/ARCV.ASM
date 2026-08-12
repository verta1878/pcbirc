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



PAGE 66,132
.model large

extrn _PRINTARCLINE:far
extrn _MonthName:dword

InBufSz equ  4096             ; size of input buffer
stopper equ     0             ; end of display line indicator
arcmark equ    26             ; special archive marker
arcver  equ     9             ; highest compression code used

;   MACRO definitions
;   ~~~~~~~~~~~~~~~~~

Dos       Macro Function          ;Perform DOS function call
          Mov   Ah,Function
          Int   21H
          EndM

Jmps      Macro Dummy
          Jmp   Short Dummy
          EndM

print     macro name              ;display a field
          mov   dx,offset name
          call  prints
          endm

;   ArcView Variables
;   ~~~~~~~~~~~~~~~~~
        .data
        public  archdr, inbuf, vthdr
stkptr  dw      0               ; stack pointer upon entry
errlvl  dw      0               ; dos error level returned
archdl  dw      0               ; file handle

; --- i/o control variables
varsize equ     8               ; NOTE!!!  size (inlen..totmbrs) used to init
inlen   dw      0               ; bytes left in buffer     |     NOTE:
incurh  dw      0               ; current file offset      | these vars must
incurl  dw      0               ;  low word                | remain here and
                                ;                          | in this order
                                ;                          | so that the init
totlen  dw      0,0             ; total of file lengths    | section will work
totsize dw      0,0             ; total of file sizes      | correctly cuz it
totmbrs dw      0               ; total number of files    | does a REP STOSW

inptr   dw      offset inbuf    ; offset to current byte
hundred dw      100             ; for computing percentages

archdr  db      64 dup (0)              ; i/o area for a header
inbuf   db      inbufsz dup (0)


;   Format Variables
;   ~~~~~~~~~~~~~~~~
Ddptr    dw   0
Ddptr2   dw   0

; --- messages
IOerror db      'I/O error reading file',stopper
InvFmt  db      'Invalid archive format',stopper
InvHdr  db      'Invalid archive header',stopper

; --- display lines for verbose
vline   label   byte
vname   db      15 dup (' ')
vlength db      '         0  '  ; length in archive
vstyle  db      '          '    ; compression method
vfactor db      ' xx%  '        ; compression factor
vsize   db      12 dup (' ')    ; actual file bytes
vdate   db      'dd '           ; creation date
 vmonth db      'mmm '
 vyear  db      'yy  '
 vtime  db      'hh:mm'         ; creation time
        db      stopper

; --- final totals line
vthdrln db      '----------     ----------            ----  ----------',stopper
vthdr   label   byte
 vtmbrs db      10 dup (' '),'     '
 vtlen  db      10 dup (' '),'  '
        db      10 dup (' ')
 vtsf   db      '   %  '
 vtsize db      10 dup (' ')
        db      stopper
 sign   db      ' '

styles  db      '  ----- '      ; 1 = old, no compression
        db      '  ----- '      ; 2 = new, no compression
        db      ' Packed '      ; 3 = dle for repeat chars
        db      'Squeezed'      ; 4 = huffman encoding
        db      'crunched'      ; 5 = lz, no dle
        db      'crunched'      ; 6 = lz with dle
        db      'Crunched'      ; 7 = lz with readjust
        db      'Crunched'      ; 8 = lz with readjust and dle
        db      'Squashed'      ; 9 = 13-bit lz with no dle

;months  db      'Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec '


          .code
          public _ARCV

;=============================================================================
;   PROCEDURES:  Arcv,   OpenArc, ClosArc, Prints, GetTime, Cnvrt2, ArcGetDate
;                Format, Getc,    GetBlk, GetHdr,
;=============================================================================

;       Special version of ARCV to be called by a C program
; usage:
;
;       CALL ARCV ( "filename[.ARC]", RETCD%)
;
; notes:
;       Change 9/14/86 to dis-allow wildcards
;       Change 1/ 1/87 to recognize squash format
;       Change 1/26/87 to remove much unwanted code for PCBoard
;       Change 5/25/87 to add commas to the numbers
;       Change 7/19/88 to use with C
;       Change 7/28/88 to remove unneeded variables

;---------------------
header  struc                           ; archive header
mbrcode db      0                       ;  compression code
mbrname db      13 dup (0)              ;  file name
mbrsize dw      0,0                     ;  file size in archive
mbrdate dw      0                       ;  creation date
mbrtime dw      0                       ;  creation time
mbrcrc  dw      0                       ;  cyclic redunancy check
mbrlen  dw      0,0                     ;  true file size, bytes
header  ends
;---------------------

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;   Main Procedure
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

_ARCV   proc    far
        push    bp                      ; save BASIC reg
        mov     bp,sp                   ; get parameter list pointer
        mov     stkptr,sp               ; save stack ptr
        mov     errlvl,0                ; init return code
        jmps    start                   ; do our thing

;       return with error

error:  mov     ax,DGROUP
        mov     ds,ax
        mov     es,ax
        mov     sp,stkptr
        mov     errlvl,-1               ; set bad return code
        jmps    arcv2a                  ; produce totals anyway

;       set DOS error level and Exit

ExitArc:mov     sp,stkptr               ; restore entry stack value
Exiting:mov     ax,errlvl               ; get return code
        pop     bp
        retf                            ; clear parms from stack

;---------------------

Start:
        Mov     Ax,DGROUP               ; set local seg regs
        Mov     Ds,Ax
        Mov     Es,Ax

Init:   Sub     Ax,Ax                   ; reset totals counters
        Mov     Di,offset InLen         ; by filling all vars from
        Mov     Cx,VarSize              ; inlen .. totmbrs
        Rep     Stosw                   ;
        Mov     Dx,offset Inbuf
        Mov     Inptr,Dx

        Call    Openarc                 ; see if archive exists

;       Process next archive header entry

Arcvnext:
        Call    Gethdr                  ; load next header
        Jnc     Arcv2
        Jmps    ExitArc                 ; all done

Arcv2:  Cmp     Archdr.mbrcode,0        ; archive eof?
        Je      Arcv2a
        Jmp     Arcvgo

Arcv2a:
        Mov     Ax,Totmbrs              ; total files
        Or      Ax,Ax                   ; are there any?
        Jnz     Format_Totals
        Jmp     Skip_Totals

Format_totals:
        Sub     Dx,Dx
        Mov     Si,offset Vtmbrs+2
        Call    Format

        Mov     Dx,Totlen+2             ; total actual file size
        Mov     Ax,Totlen
        Mov     Si,offset Vtlen+2
        Call    Format

        Mov     Dx,Totsize+2            ; total achive file size
        Mov     Ax,Totsize
        Mov     Si,offset Vtsize+2
        Call    Format

; reduce the total size/length to word values

        Mov     Bx,Totlen               ; get actual file size
        Mov     Ax,Totlen+2
        Mov     Cx,Totsize              ; length of file in archive
        Mov     Dx,Totsize+2
Arcv2b: Or      Ax,Ax                   ; big number?
        Jz      Arcv2c                  ; nope, can use it
        Shr     Ax,1                    ; yup, divide by two
        Rcr     Bx,1
        Shr     Dx,1
        Rcr     Cx,1
        Jmps    Arcv2b

Arcv2c: Mov     Si,offset vtsf-5        ; format stowage factor
        Mov     Ax,Bx
        Mov     Sign,' '                ; whata kludge
        Cmp     Ax,Cx                   ; arc is bigger than orig?
        Jb      Arcv2c1
        Sub     Ax,Cx                   ; amount saved
        Jmps    Arcv2f
Arcv2c1:
        Sub     Ax,Cx
        Neg     Ax
        Mov     Sign,'-'
Arcv2f:
        Mul     Hundred                 ; to percentage
        Add     Ax,50
        Adc     Dx,0                    ; round up percent
        Or      Bx,Bx                   ; empty file?
        Jnz     Arcv2d
        Mov     Ax,100
        Jmps    Arcv2e

Arcv2d: Div     Bx
Arcv2e: Sub     Dx,Dx
        Call    Format

        Print   Vthdrln

        Mov     Al,Sign
        Mov     Vtsf,Al
        Print   Vthdr                   ; display totals

Skip_totals:
        Call    ClosArc
        Jmp     ExitArc

;       format single line for each member

Arcvgo:
        Mov     Di,offset Vname         ; copy file name
        Mov     Si,offset Archdr.mbrname
        Mov     Cx,13
Arcv3:
        Lodsb
        And     Al,Al                   ; end of name?
        Jz      Arcv4
        Stosb
        Loop    Arcv3
        Jmps    Arcv5
Arcv4:
        Mov     Al,' '                  ; pad with blanks
        Rep     Stosb
Arcv5:
        Inc     Totmbrs

; reduce the size/length to word values

        Mov     Bx,Archdr.mbrlen        ; get actual file size
        Mov     Ax,Archdr.mbrlen+2

        Mov     Cx,Archdr.mbrsize       ; length of file in archive
        Mov     Dx,Archdr.mbrsize+2

Arcv51: Or      Ax,Ax                   ; big number?
        Jz      Arcv52                  ; nope, can use it
        Shr     Ax,1                    ; yup, divide by two
        Rcr     Bx,1
        Shr     Dx,1
        Rcr     Cx,1
        Jmps    Arcv51

Arcv52: Mov     Si,offset vfactor-5     ; format stowage factor
        Mov     Ax,Bx                   ; low word of actual size
        Mov     Sign,' '
        Cmp     Ax,Cx                   ; arc member is larger?
        Jb      Arcv520
        Sub     Ax,Cx                   ; amount saved
        Jmps    Arcv56
Arcv520:
        Sub     Ax,Cx
        Neg     Ax
        Mov     Sign,'-'
Arcv56:
        Mul     Hundred                 ; to percentage
        Add     Ax,50
        Adc     Dx,0                    ; round up percent
        Or      Bx,Bx                   ; empty file?
        Jnz     Arcv53
        Mov     Ax,100
        Jmps    Arcv54

Arcv53: Div     Bx
Arcv54: Sub     Dx,Dx
        Cmp     Ax,100                  ; archive fouled?
        Jbe     Arcv55
        Sub     Ax,Ax
Arcv55:
        Call    Format
        Mov     Al,Sign
        Mov     Vfactor,Al

        Sub     Bx,Bx                   ; determine style
        Mov     Bl,Archdr.mbrcode
        Mov     Cl,3                    ; eight bytes each entry
        Shl     Bx,Cl
        Lea     Si,Styles-8[Bx]         ; get ptr to style name
        Mov     Di,offset Vstyle
        Mov     Cx,8
        Rep     Movsb

        Mov     Si,offset Vsize+2       ; format file size
        Mov     Dx,Archdr.mbrsize+2
        Mov     Ax,Archdr.mbrsize
        Add     Totsize,Ax
        Adc     Totsize+2,Dx
        Call    Format

        Mov     Si,offset Vlength+2     ; format file length
        Mov     Dx,Archdr.mbrlen+2
        Mov     Ax,Archdr.mbrlen
        Add     Totlen,Ax
        Adc     Totlen+2,Dx
        Call    Format

        Mov     Ax,Archdr.mbrdate       ; format file date
        Call    ArcGetDate

        Mov     Ax,Archdr.mbrtime       ; format file time
        Call    Gettime

        Print   Vline                   ; display this file info
        Or      Ax,Ax                   ; was there an abort signal?
        Jz      Arcv57                  ;   no, skip
        Jmp     Skip_Totals             ;   yes, exit

Arcv57: Mov     Cx,word ptr Archdr.mbrsize+2
        Mov     Dx,word ptr Archdr.mbrsize
        Add     Dx,Incurl               ; add current file offset
        Adc     Cx,0
        Add     Cx,Incurh
        Mov     Ax,4200h                ; skip over file data
        Mov     Bx,Archdl
        Int     21h
        Mov     Incurh,Dx               ; new position
        Mov     Incurl,Ax
        Mov     Inlen,0                 ; reset read buffer
        Jmp     Arcvnext

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;    Open Archive
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OpenArc:                                ; open new archive
        Mov     Errlvl,0
        Push    Ds
        Lds     Dx,[BP+6]               ;change this to 4 for NEAR calls
        Mov     Ax,3D20h                ; for input
        Int     21h
        Pop     Ds
        Jc      OpenErr
        Mov     Archdl,Ax               ; save file handle
        Clc
        retn
OpenErr:
        Jmp     Error

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;    Close Archive
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Closarc:
        Mov     Bx,Archdl               ; previous handle
        Or      Bx,Bx                   ; already open?
        Jz      Closed
        Dos     3Eh                     ; yes, so close it
Closed: Mov     Archdl,0
        retn

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;   Prints:  print string by calling a C function
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Prints: Push    Es                      ; save ES cuz we use it!
        Push    Ds                      ; push DS:DX (ptr to string to print)
        Push    Dx
        Call far ptr _PRINTARCLINE      ; call a C routine to print the line
        Pop     Es
        retn

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;   GetTime:  format the time
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

time    record  hour:5,min:6,sec:5      ;packed time

GetTime:                                ;format the date
        mov     di,offset vtime
        or      ax,ax                   ;it is zero?
        jz      gottime
        push    ax                      ;save date
        and     ax,mask hour            ;get hour part
        mov     cl,hour                 ;bits to shift
        shr     ax,cl
        call    cnvrt1
        stosw
        mov     al,':'
        stosb

gt3:    pop     ax                      ;get the time back
        and     ax,mask min             ;get min part
        mov     cl,min                  ;bits to shift
        call    cnvrt
        stosw
gottime:retn

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;    Cnvrt2:  Convert to Ascii
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Cnvrt2:                                 ;convert to ascii
cnvrt:  shr     ax,cl
cnvrt1: aam                             ;make al into bcd
        or      ax,'00'                 ; and to ascii
        xchg    al,ah
cnvrtd: retn

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;   ArcGetDate:  format the date
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

date    record  yr:7,mo:4,dy:5          ;packed date

ArcGetDate:                                ;format the date
        or      ax,ax                   ;is it zero?
        jz      gotdate
        push    ax                      ;save date
        and     ax,mask yr              ;get year part
        mov     cl,yr                   ;bits to shift
        call    cnvrt
        mov     di,offset vyear
        or      al,'8'                  ;adjust for base year
        stosw

        pop     bx                      ;get the date back
        push    bx                      ;save it
        and     bx,mask mo              ;get month part
        mov     cl,mo                   ;bits to shift
        shr     bx,cl
;       add     bx,bx                   ; form month table index
;       add     bx,bx
;       lea     si,word ptr months-4[bx]

        dec     bx
        shl     bx,1
        shl     bx,1
        push    ds
        lds     si,DGROUP:_MonthName[bx]

        mov     cx,3
        mov     di,offset vmonth
        rep     movsb
        pop     ds

        pop     ax                      ;get the date back
        and     ax,mask dy              ;get day part
        mov     cl,dy                   ;bits to shift
        call    cnvrt
        mov     di,offset vdate
        stosw
gotdate:retn

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;   Format:  Formats a 32 bit integer in Dx:Ax to Ds:Si
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Format:
         Push Bp
         Mov  Ddptr,Si        ;addr of target field
         Mov  Ddptr2,Si       ;addr of target field
         Mov  Di,Dx           ;routine uses di:si
         Mov  Si,Ax
         Call Printdd
         Call Comma
         Pop  Bp
         retn

Printdd: Xor  Ax,Ax           ;zero out the
         Mov  Bx,Ax           ; working
         Mov  Bp,Ax           ; registers.
         Mov  Cx,32           ;# bits of precision
Cv1:     Shl  Si,1
         Rcl  Di,1
         Xchg Bp,Ax
         Call Cv6
         Xchg Bp,Ax
         Xchg Bx,Ax
         Call Cv6
         Xchg Bx,Ax
         Loop Cv1
         Mov  Cx,1710h
         Mov  Ax,Bx
         Call Cv2
         Mov  Ax,Bp
Cv2:     Push Ax
         Mov  Dl,Ah
         Call Cv3
         Pop  Dx
Cv3:     Mov  Dh,Dl
         Shr  Dl,1            ;move high
         Shr  Dl,1            ; nibble to
         Shr  Dl,1            ; the low
         Shr  Dl,1            ; position.
         Call Cv4
         Mov  Dl,Dh
Cv4:     And  Dl,0Fh          ;mask low nibble
         Jz   Cv5             ;if not zero
         Xor  Cl,Cl
Cv5:     Dec  Ch
         And  Cl,Ch
         Or   Dl,'0'          ;fold in ascii zero
         Sub  Dl,Cl
         Mov  Bx,Ddptr
         Mov  [Bx],Dl         ;ptr to next target field
         Inc  Ddptr
         retn

Cv6:     Adc  Al,Al
         Daa
         Xchg Al,Ah
         Adc  Al,Al
         Daa
         Xchg Al,Ah
         retn

Comma:   Mov  Di,Ddptr2
         Mov  Si,Di
         Mov  Al,' '
         Dec  Di
         Mov  [Di],Al
         Dec  Di
         Mov  [Di],Al
         Mov  Di,Si
         Mov  Cx,8
         Repe Scasb
         Cmp  Cx,6
         Jge  Comma2
         Cmp  Cx,3
         Jge  Comma1
         retn

Comma2:  Mov  Di,Si
         Dec  Di
         Dec  Di
         Movsw
         Mov  Byte ptr [Di],','
Comma1:  Mov  Di,Ddptr2
         Inc  Di
         Mov  Si,Di
         Inc  Si
         Movsw
         Movsb
         Mov  Byte ptr [Di],','
         retn

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;   Getc:  Get Character
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

GetC:                                   ;return next byte in al or cf=1 for eof
getc1:  dec     inlen                   ; any left in buffer
        jge     getc2                   ; yes, pick it up
        call    getblk
        jnc     getc1
        retn                            ; return cf=1 at eof

getc2:  mov     si,inptr                ; offset to next byte
        lodsb
        mov     inptr,si
        add     incurl,1                ; bump file offset
        adc     incurh,0
        retn

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;    GetBlk:  Read next block
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

GetBlk:                                 ; read next block
        mov     bx,archdl               ; read from arc file handle
        mov     cx,inbufsz              ; input buffer size
        mov     dx,offset inbuf         ; offset to input buffer
        mov     inptr,dx
        Dos     3Fh
        jc      getblkr                 ; oops
        or      ax,ax                   ; anything read?
        jnz     getblka
        stc                             ; no, set cf=1 for eof
        retn                            ; and Exit

getblka:mov     inlen,ax                ; return count of bytes read
        retn

getblkr:Print   IOerror
        jmp     error                   ; gotta quit


;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;    GetHdr:  Load Next Archive Header
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

GetHdr:
        mov     cx,132                  ; gotta look for the damn thing
gethdr2:
        call    getc                    ; get next file byte
        jc      gethdrr1                ; premature eof
        cmp     al,arcmark              ; start of header?
        je      gethdr3                 ; yup, let's start cookin
        loop    gethdr2
gethdrr1:
        Print   InvFmt
        jmp     error

gethdr3:call    getc                    ; get version code
        jc      gethdrr1
        mov     archdr.mbrcode,al
        cmp     al,arcver               ; reasonable code?
        ja      gethdrr1                ; nope, funny stuff
        And     Al,Al                   ; archive eof?
        Jz      gethdr9                 ; yup done

        mov     cx,13                   ; get member name
        mov     di,offset archdr.mbrname
gethdr4:
        call    getc
        jc      gethdrr1
        stosb
        loop    gethdr4
gethdr5:
        mov     cx,10                   ; length remaining
        cmp     archdr.mbrcode,1        ; old format?
        je      gethdr6                 ; yes, it's short
        mov     cl,14
gethdr6:
        mov     di,offset archdr.mbrsize
gethdr7:
        call    getc
        jc      gethdrr1
        stosb
        loop    gethdr7
gethdr8:
        cmp     archdr.mbrcode,1        ; old format?
        jne     gethdr9                 ; if so, it's short
        mov     si,offset archdr.mbrsize
        mov     di,offset archdr.mbrlen
        mov     cx,4
        rep     movsb
gethdr9:
        clc
        retn

gethdrr2:
        Print   InvHdr
        jmp     error
_ARCV   endp

        End
