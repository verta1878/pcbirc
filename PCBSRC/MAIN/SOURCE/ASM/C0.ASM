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


;  Modifications made by DWT on 4/19/93
;
;  The test for __nfile, which Borland was using to increase the number of
;  file handles, has been REMOVED.
;
;  The problem with their code was that it was calling DOS to allocate the
;  memory and was using the last-fit algorithm which left a piece of memory
;  allocated out at the 640K end of the map.  This was causing problems for
;  programs that ASSUMED that there was nothing between the 'end of program'
;  data and the 640K barrier.
;
;  Therefore, the DOS calls to set last-fit / allocate file handles / and
;  then set back to first-fit have been removed.
;
;  In its place is a small amount of code which simply copies the OLD file
;  handle table out of PSP:[18h] into a variable that is in the CODE SEGMENT,
;  named FileHandles.  This variable is PURPOSELY placed in the code segment
;  so that when PCBoard SWAPS to disk, leaving a file handle open, it does not
;  swap the handle out too!
;
;  After it finishes copying the OLD table into the FileHandles table, it then
;  updates the PSP:[34h] address to point to the new table and also updates the
;  max file handles count in PSP:[32h].
;
;  NOTE:  FileHandles is PURPOSELY set to all FFh's since DOS likes them to
;  be FFh if they are unopened.
;
;  On 12/13/93 - Added the ability to replace INT3 with INT21 to make it
;  difficult to debug PCBoard.


;[]------------------------------------------------------------[]
;|      C0.ASM -- Start Up Code for DOS                         |
;[]------------------------------------------------------------[]

;
;       C/C++ Run Time Library - Version 5.0
;
;       Copyright (c) 1987, 1992 by Borland International
;       All Rights Reserved.
;

                locals

                __C0__ = 1
INCLUDE         RULES.ASI

;       Segment and Group declarations

_TEXT           SEGMENT BYTE PUBLIC 'CODE'
                ENDS
_FARDATA        SEGMENT PARA PUBLIC 'FAR_DATA'
                ENDS
_FARBSS         SEGMENT PARA PUBLIC 'FAR_BSS'
                ENDS
IFNDEF __TINY__
_OVERLAY_       SEGMENT PARA PUBLIC 'OVRINFO'
        ENDS
_1STUB_     SEGMENT PARA PUBLIC 'STUBSEG'
        ENDS
ENDIF
_DATA           SEGMENT PARA PUBLIC 'DATA'
                ENDS
_INIT_          SEGMENT WORD PUBLIC 'INITDATA'
InitStart       label byte
                ENDS
_INITEND_       SEGMENT BYTE PUBLIC 'INITDATA'
InitEnd         label byte
                ENDS
_EXIT_          SEGMENT WORD PUBLIC 'EXITDATA'
ExitStart       label byte
                ENDS
_EXITEND_       SEGMENT BYTE PUBLIC 'EXITDATA'
ExitEnd         label byte
                ENDS
_CVTSEG         SEGMENT WORD PUBLIC 'DATA'
                ENDS
_SCNSEG         SEGMENT WORD PUBLIC 'DATA'
                ENDS
IFNDEF __HUGE__
  _BSS          SEGMENT WORD PUBLIC 'BSS'
                ENDS
  _BSSEND       SEGMENT BYTE PUBLIC 'BSSEND'
                ENDS
ENDIF
IFNDEF __TINY__
  _STACK        SEGMENT STACK 'STACK'
                ENDS
ENDIF

INT21           MACRO
                IFDEF SECURE
                  INT 3
                ELSE
                  INT 21h
                ENDIF
                ENDM

        ASSUME  CS:_TEXT, DS:DGROUP

;       External References

extrn       _main:DIST
extrn       _exit:DIST
extrn       __exit:DIST
extrn       __nfile:word
extrn       __setupio:near          ;required!
extrn       __stklen:word
IF LDATA  EQ  false
extrn       __heaplen:word
ENDIF

        SUBTTL  Start Up Code
        PAGE
;/*                                                     */
;/*-----------------------------------------------------*/
;/*                                                     */
;/*     Start Up Code                                   */
;/*     -------------                                   */
;/*                                                     */
;/*-----------------------------------------------------*/
;/*                                                     */
PSPHigh         equ     00002h
PSPEnv          equ     0002ch
PSPCmd          equ     00080h

                public  __AHINCR
                public  __AHSHIFT
__AHINCR        equ     1000h
__AHSHIFT       equ     12

IFDEF   __NOFLOAT__
MINSTACK        equ     128     ; minimal stack size in words
ELSE
MINSTACK        equ     256     ; minimal stack size in words
ENDIF
;
;       At the start, DS and ES both point to the segment prefix.
;       SS points to the stack segment except in TINY model where
;       SS is equal to CS
;
_TEXT           SEGMENT
IFDEF           __TINY__
                ORG     100h
ENDIF
STARTX          PROC    NEAR
;       Save general information, such as :
;               DGROUP segment address
;               DOS version number
;               Program Segment Prefix address
;               Environment address
;               Top of far heap

IFDEF   __TINY__
                mov     dx, cs          ; DX = GROUP Segment address
ELSE
                mov     dx, DGROUP      ; DX = GROUP Segment address
ENDIF
                        mov     cs:DGROUP@@, dx ;  __BOSS__
                mov     ah, 30h
                int     21h             ; get DOS version number
                mov     bp, ds:[PSPHigh]; BP = Highest Memory Segment Addr
                mov     bx, ds:[PSPEnv] ; BX = Environment Segment address
                mov     ds, dx
                mov     _version@, ax   ; Keep major and minor version number
                mov     _psp@, es       ; Keep Program Segment Prefix address
                mov     _envseg@, bx    ; Keep Environment Segment address
                mov     word ptr _heaptop@ + 2, bp
;
;       Save several vectors and install default divide by zero handler.
;
                call    SaveVectors


;       Count the number of environment variables and compute the size.
;       Each variable is ended by a 0 and a zero-length variable stops
;       the environment. The environment can NOT be greater than 32k.

                mov     ax, _envseg@
                mov     es, ax
                xor     ax, ax
                mov     bx, ax
                mov     di, ax

                mov     cx, 07FFFh      ; Environment cannot be > 32 Kbytes
                cld
@@EnvLoop:
                repnz   scasb
                jcxz    InitFailed      ; Bad environment !!!

                inc     bx              ; BX = Nb environment variables
                cmp     es:[di], al
                jne     @@EnvLoop       ; Next variable ...
                or      ch, 10000000b
                neg     cx
                mov     _envLng@, cx    ; Save Environment size
                mov     cx, dPtrSize / 2
                shl     bx, cl
                add     bx, dPtrSize * 4
                and     bx, not ((dPtrSize * 4) - 1)
                mov     _envSize@, bx   ; Save Environment Variables Nb.


;       Determine the amount of memory that we need to keep

IFDEF _DSSTACK_
                mov     dx, ds
ELSE
                mov     dx, ss
ENDIF
                sub     bp, dx          ; BP = remaining size in paragraphs
IF LDATA
                mov     di, seg __stklen
                mov     es, di
                mov     di, es:__stklen ; DI = Requested stack size
ELSE
                mov     di, __stklen    ; DI = Requested stack size
ENDIF
;
; Make sure that the requested stack size is at least MINSTACK words.
;
                cmp     di, 2*MINSTACK  ; requested stack big enough ?
                jae     AskedStackOK
                mov     di, 2*MINSTACK  ; no --> use minimal value
IF LDATA
                mov     es:__stklen, di ; override requested stack size
ELSE
                mov        __stklen, di ; override requested stack size
ENDIF

AskedStackOK    label   near
IFDEF _DSSTACK_
                add     di, offset DGROUP: edata@
                jb      InitFailed      ; DATA segment can NOT be > 64 Kbytes
ENDIF
IF LDATA  EQ  false
                add     di, __heaplen
                jb      InitFailed      ; DATA segment can NOT be > 64 Kbytes
ENDIF
                mov     cl, 4
                shr     di, cl          ; $$$ Do not destroy CL $$$
                inc     di              ; DI = DS size in paragraphs
                cmp     bp, di
IF LDATA  EQ  false
                jb      InitFailed      ; Not enough memory
                cmp     __stklen, 0
                je      ExpandDS        ; Expand DS up to 64 Kb
                cmp     __heaplen, 0
                jne     ExcessOfMemory  ; Much more available than needed
ExpandDS        label   near
                mov     di, 1000h
                cmp     bp, di
                ja      ExcessOfMemory  ; Enough to run the program
                mov     di, bp
                jmp     short ExcessOfMemory  ; Enough to run the program
ELSE
                jnb     ExcessOfMemory  ; Much more available than needed
ENDIF

;       All initialization errors arrive here

InitFailed      label   near
                jmp     near ptr _abort

;       Return to DOS the amount of memory in excess
;       Set far heap base and pointer

ExcessOfMemory  label   near
                mov     bx, di
                add     bx, dx
                mov     word ptr _heapbase@ + 2, bx
                mov     word ptr _brklvl@ + 2, bx
                mov     ax, _psp@
                sub     bx, ax          ; BX = Number of paragraphs to keep
                mov     es, ax          ; ES = Program Segment Prefix address
                mov     ah, 04Ah
                push    di              ; preserve DI
                INT21                   ; this call clobbers SI,DI,BP !!!!!!
                pop     di              ; restore  DI

                shl     di, cl          ; $$$ CX is still equal to 4 $$$

                cli                     ; req'd for pre-1983 88/86s
                mov     ss, dx          ; Set the program stack
                mov     sp, di
                sti

IFNDEF _DSSTACK_
                mov     ax, seg __stklen
                mov     es, ax
                mov     es:__stklen, di ; If separate stack segment, save size
ENDIF


IFNDEF  __HUGE__

;       Reset uninitialized data area

                xor     ax, ax
                mov     es, cs:DGROUP@@
                mov     di, offset DGROUP: bdata@
                mov     cx, offset DGROUP: edata@
                sub     cx, di
                cld
                rep     stosb
ENDIF

; ;   If default number of file handles have changed then tell DOS
;                 cmp     __nfile, 20
;                 jbe     @@NoChange
;
;                 cmp     _osmajor@, 3   ; Check for >= DOS 3.3
;                 jb      @@NoChange
;                 ja      @@DoChange
;                 cmp     _osminor@, 1Eh
;                 jb      @@NoChange
; @@DoChange:
;                 mov     ax, 5801h      ; Set last fit allocation
;                 mov     bx, 2
;                 int     21h
;                 jc      @@BadInit
;
;                 mov     ah, 67h        ; Expand handle table
;                 mov     bx, __nfile
;                 int     21h
;                 jc      @@BadInit
;
;                 mov     ah, 48h        ; Allocate 16 bytes to find new
;                 mov     bx, 1          ;   top of memory address
;                 int     21h
;                 jc      @@BadInit
;                 inc     ax             ; Adjust address to point after block
;                 mov     word ptr _heaptop@ + 2, ax
;
;                 dec     ax             ; Change back and release block
;                 mov     es, ax
;                 mov     ah, 49h
;                 int     21h
;                 jc      @@BadInit
;
;                 mov     ax, 5801h      ; Set first fit allocation
;                 mov     bx, 0
;                 int     21h
;                 jnc     @@NoChange
;
;  @@BadInit:      jmp near ptr _abort

                push    ds
                push    si
                push    di
                mov     ax, _psp@
                mov     ds, ax
                mov     cx, ds:[32h]        ; get OLD max file handle count

                mov     si, 18h             ; point to 1st entry in OLD table

                mov     ax, seg FileHandles
                mov     es, ax
                mov     di, offset FileHandles   ; point to top of NEW table
                mov     dx, di              ; save DI in DX for use below

                cld
                rep     movsb               ; copy OLD table into NEW table

                mov     byte ptr ds:[32h], 25  ; put NEW max file handle count
                mov     word ptr ds:[34h], dx  ; put NEW table address (OFFSET)
                mov     word ptr ds:[36h], ax  ; put NEW table address (SEG)
                pop     di
                pop     si
                pop     ds

@@NoChange:

;       Prepare main arguments

;               mov     ah, 0
;               int     1ah                     ; get current BIOS time in ticks
;               mov     word ptr _StartTime@,dx ; save it for clock() fn
;               mov     word ptr _StartTime@+2,cx
;               or      al,al                   ; was midnight flag set?
;               jz      @@NotMidnight
;               mov     ax,40h                  ; set BIOS midnight flag
;               mov     es,ax                   ;  at 40:70
;               mov     bx,70h
;               mov     byte ptr es:[bx],1

@@NotMidnight:
                xor     bp,bp                   ; set BP to 0 for overlay mgr

                mov     es, cs:DGROUP@@
                mov     si,offset DGROUP:InitStart      ;si = start of table
                mov     di,offset DGROUP:InitEnd        ;di = end of table
                call    Initialize

;       ExitCode = main(argc,argv,envp);

IF      LDATA
                push    word ptr __C0environ+2
                push    word ptr __C0environ
                push    word ptr __C0argv+2
                push    word ptr __C0argv
ELSE
                push    word ptr __C0environ
                push    word ptr __C0argv
ENDIF
                push    __C0argc
                call    _main

;       Flush and close streams and files

                push    ax
                call    _exit

;---------------------------------------------------------------------------
;       _cleanup()      call all #pragma exit cleanup routines.
;       _checknull()    check for null pointer zapping copyright message
;       _terminate(int) exit program with error code
;
;       These functions are called by exit(), _exit(), _cexit(),
;       and _c_exit().
;---------------------------------------------------------------------------

;       Call cleanup routines

__cleanup       PROC    DIST
                PUBLIC  __cleanup

                mov     es, cs:DGROUP@@
                push    si
                push    di
                mov     si,offset DGROUP:ExitStart
                mov     di,offset DGROUP:ExitEnd
                call    Cleanup
                pop     di
                pop     si

; We need to restore the PSP back to its original state in case the exec()
; family of functions is going to pass us over to a new program

                push    ds
                push    si
                push    di
                mov     ax, _psp@
                mov     es, ax
                mov     dx, ax
                mov     di, 18h

                mov     ax, word ptr es:[36h]
                mov     ds, ax
                mov     si, word ptr es:[34h]

                cld
                mov     cx, 20
                rep     movsb

                mov     byte ptr es:[32h], 20  ; put OLD max file handle count
                mov     word ptr es:[34h], 18h ; put OLD table address (OFFSET)
                mov     word ptr es:[36h], dx  ; put OLD table address (SEG)
                pop     di
                pop     si
                pop     ds

                ret
__cleanup       ENDP

;       Check for null pointers before exit

__checknull     PROC    DIST
                PUBLIC  __checknull

IF      LDATA  EQ  false
  IFNDEF  __TINY__
                push    si
                push    di
                mov     es, cs:DGROUP@@
                xor     ax, ax
                mov     si, ax
                mov     cx, lgth_CopyRight
ComputeChecksum label   near
                add     al, es:[si]
                adc     ah, 0
                inc     si
                loop    ComputeChecksum
                sub     ax, CheckSum
                jz      @@SumOK
                mov     cx, lgth_NullCheck
                mov     dx, offset DGROUP: NullCheck
                call    ErrorDisplay
@@SumOK:        pop     di
                pop     si
  ENDIF
ENDIF
                ret
__checknull     ENDP

;       Exit to DOS

__terminate     PROC    DIST
                PUBLIC  __terminate
                mov     bp,sp
                mov     ah,4Ch
                mov     al,[bp+cPtrSize]
                int     21h                     ; Exit to DOS
__terminate     ENDP

STARTX          ENDP

        SUBTTL  Vector save/restore & default Zero divide routines
        PAGE
;[]------------------------------------------------------------[]
;|                                                              |
;| Interrupt Save/Restore routines and default divide by zero   |
;| handler.                                                     |
;|                                                              |
;[]------------------------------------------------------------[]

ZeroDivision    PROC    FAR
                mov     cx, lgth_ZeroDivMSG
                mov     dx, offset DGROUP: ZeroDivMSG
                jmp     MsgExit3
ZeroDivision    ENDP

;--------------------------------------------------------------------------
;       savevectors()
;
;       Save vectors for 0, 4, 5 & 6 interrupts.  This is for extended
;       signal()/raise() support as the signal functions can steal these
;       vectors during runtime.
;--------------------------------------------------------------------------
SaveVectors     PROC    NEAR
                push    ds

IFDEF SECURE
; Save INT 3
                mov     ax, 3503h
                int     021h
                mov     word ptr _Int3Vector@, bx
                mov     word ptr _Int3Vector@+2, es

; The replace INT 3 with INT 21
                mov     ax, 3521h
                int     021h
                mov     dx, es
                xor     ax,ax
                mov     es,ax
                mov     es:[0+(3*4)],bx
                mov     es:[0+(3*4)+2],dx
ENDIF

; Save INT 0
                mov     ax, 3500h
                INT21
                mov     word ptr _Int0Vector@, bx
                mov     word ptr _Int0Vector@+2, es
; Save INT 4
                mov     ax, 3504h
                INT21
                mov     word ptr _Int4Vector@, bx
                mov     word ptr _Int4Vector@+2, es
; Save INT 5
                mov     ax, 3505h
                INT21
                mov     word ptr _Int5Vector@, bx
                mov     word ptr _Int5Vector@+2, es
; Save INT 6
                mov     ax, 3506h
                INT21
                mov     word ptr _Int6Vector@, bx
                mov     word ptr _Int6Vector@+2, es
;
;       Install default divide by zero handler.
;
                mov     ax, 2500h
                mov     dx, cs
                mov     ds, dx
                mov     dx, offset ZeroDivision
                INT21

                pop     ds
                ret
SaveVectors     ENDP

;--------------------------------------------------------------------------
;       _restorezero() puts back all the vectors that SaveVectors took.
;
;NOTE : TSRs must BE AWARE that signal() functions which take these
;       vectors will be deactivated if the keep() function is executed.
;       If a TSR wants to use the signal functions when it is active it
;       will have to save/restore these vectors itself when activated and
;       deactivated.
;--------------------------------------------------------------------------
__restorezero   PROC    DIST
                PUBLIC  __restorezero
IFDEF   __HUGE__
                push    ds
                mov     ds, cs: DGROUP@@
ENDIF

IFDEF SECURE
                push    ds
                mov     ax, 2503h
                lds     dx, _Int3Vector@
                int     21h
                pop     ds
ENDIF

                push    ds
                mov     ax, 2500h
                lds     dx, _Int0Vector@
                int     21h
                pop     ds

                push    ds
                mov     ax, 2504h
                lds     dx, _Int4Vector@
                int     21h
                pop     ds

                push    ds
                mov     ax, 2505h
                lds     dx, _Int5Vector@
                int     21h
                pop     ds

IFNDEF   __HUGE__
                push    ds
ENDIF
                mov     ax, 2506h
                lds     dx, _Int6Vector@
                int     21h
                pop     ds

                ret
                ENDP

;------------------------------------------------------------------
;  Loop through a startup/exit (SE) table,
;  calling functions in order of priority.
;  ES:SI is assumed to point to the beginning of the SE table
;  ES:DI is assumed to point to the end of the SE table
;  First 64 priorities are reserved by Borland
;------------------------------------------------------------------
PNEAR           EQU     0
PFAR            EQU     1
NOTUSED         EQU     0ffh

SE              STRUC
calltype        db      ?                       ; 0=near,1=far,ff=not used
priority        db      ?                       ; 0=highest,ff=lowest
addrlow         dw      ?
addrhigh        dw      ?
SE              ENDS

Initialize      proc near
@@Start:        mov     ax,100h                 ;start with lowest priority
                mov     dx,di                   ;set sentinel to end of table
                mov     bx,si                   ;bx = start of table

@@TopOfTable:   cmp     bx,di                   ;and the end of the table?
                je      @@EndOfTable            ;yes, exit the loop
                cmp     es:[bx.calltype],NOTUSED;check the call type
                je      @@Next
                mov     cl, es:[bx.priority]    ;move priority to CX
                xor     ch, ch
                cmp     cx,ax                   ;check the priority
                jae     @@Next                  ;too high?  skip
                mov     ax,cx                   ;keep priority
                mov     dx,bx                   ;keep index in dx
@@Next:         add     bx,SIZE SE              ;bx = next item in table
                jmp     @@TopOfTable

@@EndOfTable:   cmp     dx,di                   ;did we exhaust the table?
                je      @@Done                  ;yes, quit
                mov     bx,dx                   ;bx = highest priority item
                cmp     es:[bx.calltype],PNEAR  ;is it near or far?
                mov     es:[bx.calltype],NOTUSED;wipe the call type
                push    es                      ;save es
                je      @@NearCall

@@FarCall:      call    DWORD PTR es:[bx.addrlow]
                pop     es                      ;restore es
                jmp     short @@Start

@@NearCall:     call    WORD PTR es:[bx.addrlow]
                pop     es                      ;restore es
                jmp     short @@Start

@@Done:         ret
                endp

Cleanup         proc near
@@Start:        mov     ah,0                    ;start with highest priority
                mov     dx,di                   ;set sentinel to end of table
                mov     bx,si                   ;bx = start of table

@@TopOfTable:   cmp     bx,di                   ;and the end of the table?
                je      @@EndOfTable            ;yes, exit the loop
                cmp     es:[bx.calltype],NOTUSED;check the call type
                je      @@Next
                cmp     es:[bx.priority],ah     ;check the priority
                jb      @@Next                  ;too low?  skip
                mov     ah,es:[bx.priority]     ;keep priority
                mov     dx,bx                   ;keep index in dx
@@Next:         add     bx,SIZE SE              ;bx = next item in table
                jmp     @@TopOfTable

@@EndOfTable:   cmp     dx,di                   ;did we exhaust the table?
                je      @@Done                  ;yes, quit
                mov     bx,dx                   ;bx = highest priority item
                cmp     es:[bx.calltype],PNEAR  ;is it near or far?
                mov     es:[bx.calltype],NOTUSED;wipe the call type
                push    es                      ;save es
                je      @@NearCall

@@FarCall:      call    DWORD PTR es:[bx.addrlow]
                pop     es                      ;restore es
                jmp     short @@Start

@@NearCall:     call    WORD PTR es:[bx.addrlow]
                pop     es                      ;restore es
                jmp     short @@Start

@@Done:         ret
                endp

;------------------------------------------------------------------

ErrorDisplay    PROC    NEAR
                mov     ah, 040h
                mov     bx, 2
                int     021h
                ret
ErrorDisplay    ENDP

_abort          PROC    DIST
                PUBLIC  _abort
                mov     cx, lgth_abortMSG
                mov     dx, offset DGROUP: abortMSG
MsgExit3        label   near
                mov     ds, cs: DGROUP@@
                call    ErrorDisplay
CallExit3       label   near
                mov     ax, 3
                push    ax
                call    __exit           ; _exit(3);
                ENDP

; The DGROUP@ variable is used to reload DS with DGROUP

PubSym@         DGROUP@, <dw    ?>, __PASCAL__
FileHandles     db 26 dup (0FFh)


; __MMODEL is used to determine the memory model or the default
; pointer types at run time.

                public __MMODEL
__MMODEL        dw      MMODEL

_TEXT           ENDS

                SUBTTL  Start Up Data Area
                PAGE
;[]------------------------------------------------------------[]
;|      Start Up Data Area                                      |
;|                                                              |
;|      WARNING         Do not move any variables in the data   |
;|                      segment unless you're absolutely sure   |
;|                      that it does not matter.                |
;[]------------------------------------------------------------[]

_DATA           SEGMENT

;       Magic symbol used by the debug info to locate the data segment
                public DATASEG@
DATASEG@        label   byte

;       The CopyRight string must NOT be moved or changed without
;       changing the null pointer check logic

CopyRight       db      4 dup(0)
                db      'Borland C++ - Copyright 1991 Borland Intl.',0
lgth_CopyRight  equ     $ - CopyRight

IF      LDATA  EQ  false
IFNDEF  __TINY__
CheckSum        equ     00D5Ch
NullCheck       db      'Null pointer assignment', 13, 10
lgth_NullCheck  equ     $ - NullCheck
ENDIF
ENDIF

ZeroDivMSG      db      'Divide error', 13, 10
lgth_ZeroDivMSG equ     $ - ZeroDivMSG

abortMSG        db      'Abnormal program termination', 13, 10
lgth_abortMSG   equ     $ - abortMSG

;
;                       Interrupt vector save areas
;
;       Interrupt vectors 0,4,5 & 6 are saved at startup and then restored
;       when the program terminates.  The signal/raise functions might
;       steal these vectors during execution.
;
;       Note: These vectors save area must not be altered
;             without changing the save/restore logic.
;
PubSym@         _Int0Vector     <dd     0>,             __CDECL__
PubSym@         _Int3Vector     <dd     0>,             __CDECL__
PubSym@         _Int4Vector     <dd     0>,             __CDECL__
PubSym@         _Int5Vector     <dd     0>,             __CDECL__
PubSym@         _Int6Vector     <dd     0>,             __CDECL__
;
;                       Miscellaneous variables
;
PubSym@         _C0argc,        <dw     0>,             __CDECL__
dPtrPub@        _C0argv,        0,                      __CDECL__
dPtrPub@        _C0environ,     0,                      __CDECL__
PubSym@         _envLng,        <dw     0>,             __CDECL__
PubSym@         _envseg,        <dw     0>,             __CDECL__
PubSym@         _envSize,       <dw     0>,             __CDECL__
PubSym@         _psp,           <dw     0>,             __CDECL__
PubSym@         _version,       <label word>,           __CDECL__
PubSym@         _osversion,     <label word>,           __CDECL__
PubSym@         _osmajor,       <db     0>,             __CDECL__
PubSym@         _osminor,       <db     0>,             __CDECL__
PubSym@         errno,          <dw     0>,             __CDECL__
PubSym@         _StartTime,     <dw   0,0>,             __CDECL__



;       Memory management variables

IF      LDATA  EQ  false
PubSym@         __heapbase,     <dw   DGROUP:edata@>,   __CDECL__
ENDIF
IFNDEF __HUGE__
PubSym@         __brklvl,       <dw   DGROUP:edata@>,   __CDECL__
ENDIF
PubSym@         _heapbase,      <dd   0>,       __CDECL__
PubSym@         _brklvl,        <dd   0>,       __CDECL__
PubSym@         _heaptop,       <dd   0>,       __CDECL__

;       If stack in DS and Large data model then override location of __emu

IFDEF   _DSSTACK_
IF      LDATA
public  __emu
__emu   db      044h    DUP (0)
        db      0CCh    DUP (?)
ENDIF
ENDIF

_DATA           ENDS


_CVTSEG         SEGMENT
PubSym@         _RealCvtVector, <label  word>,  __CDECL__
                ENDS

_SCNSEG         SEGMENT
PubSym@         _ScanTodVector,  <label word>,  __CDECL__
                ENDS

IFNDEF __HUGE__
_BSS            SEGMENT
bdata@          label   byte
                ENDS

_BSSEND         SEGMENT
edata@          label   byte
                ENDS
ENDIF

IFNDEF __TINY__
_STACK          SEGMENT
                db      128 dup(?)               ;minimum stack size
                ENDS
ENDIF  ; __TINY__
                END     STARTX
