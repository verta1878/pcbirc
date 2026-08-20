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


;*****************************************************************************
;
; Compilation Requirements:
;
;     TASM /MX /D__x__ [...] ANSI
;
;  The __x__ should be replaced with   __s__   small
;                                      __m__   medium
;                                      __c__   compact
;                                      __l__   large
;
;
;  [...] is for option defines - use the following according to your needs:
;
;     /DMATT      for Matt's use in PCBComm
;     /DPCBCOMM   to add PCBComm support
;     /DLIB       to strip out PCBoard code
;
;  examples:
;
;   For PCBoard use:  TASM /MX /D__m__ ANSI
;   For PCBComm use:  TASM /MX /D__l__ /DPCBCOMM /DLIB /DMATT ANSI
;   For Comm use:     TASM /MX /D__s__ /DPCBCOMM /DLIB ANSI
;
;*****************************************************************************


Page 66,132

IFDEF MATT
include \bc\include\rules.asi
ELSE
include \proj\pcb\source\h\rules.asi
ENDIF

ifdef __s__
  .model small, pascal
elseifdef __m__
  .model medium, pascal
elseifdef __c__
  .model compact, pascal
elseifdef __l__
  .model large, pascal
endif


IFDEF MATT
extrn _Changed_Lines:far ptr
extrn _indoor:byte            ; are we in door
ENDIF


extrn _Scrn_Addr: dword
extrn _Scrn_Rtrc: byte
ifdef PCBCOMM
extrn _SaveBuf: byte
extrn _Scrolling: byte
extrn _clearwindow : byte
endif
extrn SAVESCREEN:DIST
extrn RESTORESCREEN:DIST
extrn BELL:DIST

MAXCOL  = 79                       ;maximum columns on the screen

if LPROG
   _ansiemu  segment para public 'code'
   assume    cs:_ansiemu
   ALIGN2    equ ALIGN 2
   ALIGN16   equ ALIGN 16
else
   CSeg@
   ALIGN2    equ ALIGN 1
   ALIGN16   equ ALIGN 1
endif

Virtual   db    0                 ;write to screen = 0, write to virtscrn = 1
          ALIGN2
OldDs     dw    ?                 ;Our original data segment (default DS)
SaveDs    dw    ?                 ;Our saved data segment (could be different!)
VirtPos   dw    ?                 ;virtual cursor position when screen is off
SaveAddr  dw    ?                 ;original screen segment
SaveOfs   dw    ?                 ;original screen offset
SaveRtrc  db    ?                 ;original screen retrace mode
MaxRow    db    ?                 ;max rows on the screen
Color     db    7                 ;current color being used by ansi driver
          ALIGN2

ifdef PCBCOMM
SaveLen   dw    ?
SaveScrnAddr  dd ?
endif

ScrnBytes dw    ?                 ;number of bytes in screen buffer
SavePos   dw    ?                 ;position saved by ansi "save pos" command
CurAddr   dw    ?                 ;current screen address when in ansi()
CurOfs    dw    ?                 ;current screen offset when in ansi()
CurRtrc   db    ?                 ;current screen retrace mode when in ansi()

IFNDEF LIB
BufSeg    dw    ?                 ;Segment of Scrollback Buffer
BufOfs    dw    ?                 ;Offset of Scrollback Buffer
AddOfsArr dd    ?                 ;Address of OfsArray
BufLines  dw    ?                 ;Number of lines in Scrollback Buffer
CopyToBuf db    0                 ;Status of Scrollback Buffer
ENDIF

AscColors db    '0','4','2','6','1','5','3','7'

          ALIGN2
AndOrMask db    000h, 007h         ;0
          db    0FFh, 008h         ;1
          db    0FFh, 0FFh         ;2    (BAD CODE)
          db    0FFh, 0FFh         ;3    (BAD CODE)
          db    0F8h, 001h         ;4
          db    0FFh, 080h         ;5
          db    0FFh, 0FFh         ;6    (BAD CODE)
          db    0F8h, 070h         ;7
          db    088h, 000h         ;8
          db    0FFh, 0FFh         ;9    (BAD CODE)
          db    0FFh, 0FFh         ;10   (BAD CODE)
          db    0FFh, 0FFh         ;11   (BAD CODE)
          db    0FFh, 0FFh         ;12   (BAD CODE)
          db    0FFh, 0FFh         ;13   (BAD CODE)
          db    0FFh, 0FFh         ;14   (BAD CODE)
          db    0FFh, 0FFh         ;15   (BAD CODE)
          db    0FFh, 0FFh         ;16   (BAD CODE)
          db    0FFh, 0FFh         ;17   (BAD CODE)
          db    0FFh, 0FFh         ;18   (BAD CODE)
          db    0FFh, 0FFh         ;19   (BAD CODE)
          db    0FFh, 0FFh         ;20   (BAD CODE)
          db    0FFh, 0FFh         ;21   (BAD CODE)
          db    0FFh, 0FFh         ;22   (BAD CODE)
          db    0FFh, 0FFh         ;23   (BAD CODE)
          db    0FFh, 0FFh         ;24   (BAD CODE)
          db    0FFh, 0FFh         ;25   (BAD CODE)
          db    0FFh, 0FFh         ;26   (BAD CODE)
          db    0FFh, 0FFh         ;27   (BAD CODE)
          db    0FFh, 0FFh         ;28   (BAD CODE)
          db    0FFh, 0FFh         ;29   (BAD CODE)
          db    0F8h, 000h         ;30
          db    0F8h, 004h         ;31
          db    0F8h, 002h         ;32
          db    0F8h, 006h         ;33
          db    0F8h, 001h         ;34
          db    0F8h, 005h         ;35
          db    0F8h, 003h         ;36
          db    0F8h, 007h         ;37
          db    0FFh, 0FFh         ;38   (BAD CODE)
          db    0FFh, 0FFh         ;39   (BAD CODE)
          db    08Fh, 000h         ;40
          db    08Fh, 040h         ;41
          db    08Fh, 020h         ;42
          db    08Fh, 060h         ;43
          db    08Fh, 010h         ;44
          db    08Fh, 050h         ;45
          db    08Fh, 030h         ;46
          db    08Fh, 070h         ;47

          ALIGN2
Yoffsets  label     word          ;offsets for calculating screen addresses
          Y = 0
          rept      60
          dw        Y
          Y = Y + 160
          endm


;===========================================================================
;   SETLIMITS
;   sets the screen limits to be used by the ansi driver
;===========================================================================
          public SETLIMITS
          ALIGN2
SETLIMITS proc   NumLines:byte
          Mov    OldDs,Ds
          Mov    Al,NumLines
          Xor    Bx,Bx                 ;Save NumLines in Bx
          Mov    Bl,Al                 ;...
          Dec    Al
          Mov    MaxRow,Al

          Shl   Bx,1                   ;Calculate total # bytes in scrn buffer
          Mov   Ax,[Yoffsets+Bx]       ;Ax = MaxRow * 160
          Mov   ScrnBytes,Ax           ;Remember it for later use
ifdef PCBCOMM
          Mov   SaveLen,0
ifdef MSC
ifdef LDATA
          Push  Ds
          Mov   Ax,seg _Scrn_Addr
          Mov   Ds,Ax
endif
endif
          Les   Ax,dword ptr _Scrn_Addr
          Mov   word ptr [SaveScrnAddr+2],Es
          Mov   word ptr [SaveScrnAddr],Ax
ifdef MSC
ifdef LDATA
          Pop   Ds
endif
endif
endif
          Ret
SETLIMITS endp


ifndef PCBCOMM
;===========================================================================
;   TOGGLEOFF
;   Turns the screen display off and sends screen data to the virtual buffer
;===========================================================================
          public TOGGLEOFF
          ALIGN2
TOGGLEOFF proc   BufAddr:far ptr
          Les    Ax,BufAddr
          Push   Es
          Push   Ax
          Call   SAVESCREEN

          Add    MaxRow,2              ;we want to clear the ENTIRE screen
          Add    ScrnBytes,320         ;we want to clear the ENTIRE screen

ifdef MSC
ifdef LDATA
          Push  Ds
          Mov   Ax,seg _Scrn_Addr
          Mov   Ds,Ax
endif
endif
          Mov    Es,word ptr [_Scrn_Addr+2]
          Mov    Ax,word ptr [_Scrn_Addr]
          Mov    CurOfs,Ax
          Mov    Al,_Scrn_Rtrc
          Mov    CurRtrc,Al
ifdef MSC
ifdef LDATA
          Pop   Ds
endif
endif

          Call   clear

          Sub    ScrnBytes,320         ;Restore ScrnBytes
          Sub    MaxRow,2              ;Restore MaxRow

ifdef MSC
ifdef LDATA
          Push  Ds
          Mov   Ax,seg _Scrn_Addr
          Mov   Ds,Ax
endif
endif

          Les    Ax,BufAddr            ;ES:AX = long pointer to buffer address
          Xchg   Ax,word ptr [_Scrn_Addr] ;switch buffer offset with screen offset
          Mov    SaveOfs,Ax            ;Save old Scrn_Ofs
          Mov    Ax,Es                 ;Ax now has the buffer segment
          Xchg   Ax,word ptr [_Scrn_Addr+2] ;switch buffer segment with screen segment
          Mov    SaveAddr,Ax           ;save old Scrn_Addr
          Mov    Al,_Scrn_Rtrc         ;Al now has the screen retrace setting
          Mov    SaveRtrc,Al           ;save the old screen retrace
          Mov    _Scrn_Rtrc,0          ;set retrace to 0 for full speed writing
          Mov    Virtual,1

ifdef MSC
ifdef LDATA
          Pop   Ds
endif
endif
          Ret
TOGGLEOFF endp


;===========================================================================
;   TOGGLEON
;   Turns the screen display on after restoring from the virtual buffer
;===========================================================================
          public TOGGLEON
          ALIGN2
TOGGLEON  proc   BufAddr:far ptr
ifdef MSC
ifdef LDATA
          Push  Ds
          Mov   Ax,seg _Scrn_Addr
          Mov   Ds,Ax
endif
endif

          Mov    Ax,SaveAddr
          Mov    word ptr [_Scrn_Addr+2],Ax
          Mov    Ax,SaveOfs
          Mov    word ptr [_Scrn_Addr],Ax
          Mov    Al,SaveRtrc
          Mov    _Scrn_Rtrc,Al

ifdef MSC
ifdef LDATA
          Pop   Ds
endif
endif

          Les    Ax,BufAddr
          Push   Es
          Push   Ax
          Call   RESTORESCREEN
          Mov    Ah,2                   ;Update cursor position
          Xor    Bx,Bx
          Mov    Dx,VirtPos
          Int    10h
          Mov    Virtual,0
          Ret
TOGGLEON  endp
endif

;===========================================================================
;   CURCOLOR
;   returns to the caller the current color being displayed by the ansi driver
;===========================================================================
          public CURCOLOR
          ALIGN2
CURCOLOR  proc
          Mov    Al,Color
          Ret
CURCOLOR  endp


;===========================================================================
;   AWHEREX
;   returns to the caller the current column position of the cursor
;===========================================================================
          public AWHEREX
          ALIGN2
AWHEREX   proc
VirtualX: Mov   Dx,VirtPos
AXreturn: Mov   Al,Dl
          Ret
AWHEREX   endp


;===========================================================================
;   AWHEREY
;   returns to the caller the current column position of the cursor
;===========================================================================
          public AWHEREY
          ALIGN2
AWHEREY   proc
VirtualY: Mov   Dx,VirtPos
AYreturn: Mov   Al,Dh
          Ret
AWHEREY   endp


;===========================================================================
;   AGOTOXY
;   Informs the ansi driver of X,Y coordinates then moves the real cursor
;===========================================================================
          public AGOTOXY
          ALIGN2
AGOTOXY   proc  X:byte, Y:byte
          Mov   Dl,X
          Mov   Dh,Y

          Cmp   Dh,MaxRow
          Jbe   yokay
          Mov   Dh,MaxRow

yokay:    Mov   VirtPos,Dx        ;Update the ansi cursor position
          Mov   Ah,2
          Xor   Bh,bh
          Int   10h
          Ret
AGOTOXY   endp


;===========================================================================
;   ASETCOLOR
;   Informs the ansi driver of a new color to be used
;===========================================================================
          public ASETCOLOR
          ALIGN2
ASETCOLOR proc  NewColor:byte
          Mov   Al,NewColor
          Mov   Color,Al
          Ret
ASETCOLOR endp


;===========================================================================
;   COLORSTR
;   creates an ansi escape sequence string in "Buf" that would create the
;   desired color - if one is necessary
;===========================================================================
          public COLORSTR
          ALIGN16
COLORSTR  proc   Buf:ptr, NewColor:byte
          Xor    Ax,Ax                 ;Set default return code to 0
          Mov    Bl,Color
          Mov    Dl,NewColor
          Cmp    Bl,Dl
          Jne    CSstart               ;Return AX=0 to indicate no string
          Ret

CSstart:  Push   Si
          Push   Di
if LDATA
          Les    Di,Buf                ;ES:DI points to buf
else
          Mov    Di,Ds
          Mov    Es,Di
          Mov    Di,Buf                ;ES:DI points to buf
endif

          Cld
          Mov    Ax,'['               ;Start off the string with "["
          Stosw
          Mov    Ah,';'                ;Default AH to ";"

          Cmp    Dl,70h                ;Is it reverse video Black on White?
          Je     Reverse

Setup:    Mov    Cl,4                  ;split up the background & foreground
          Xor    Bh,Bh
          Mov    Dh,Bh
          Shl    Bx,Cl                 ;Move old Background into BH
          Shr    Bl,Cl                 ;Move old Foreground back into BL
          Shl    Dx,Cl                 ;Move new Background into DH
          Shr    Dl,Cl                 ;Move new Foreground back into DL

          Xor    Cx,Cx                 ;Default CX to 0

          Or     Bx,Bx                 ;If the old color was 0 that is a signal
          Je     Reset                 ;to do a full reset of the colors

          Cmp    Bh,Dh
          Jne    Reset                 ;if oldback != newback then reset
          Cmp    Bl,7
          Jle    TestFore              ;if oldfore <= 7 then go test foreground
          Cmp    Dl,8
          Jge    TestFore              ;if newfore >= 8 then go test foreground

Reset:    Mov    Al,'0'                ;Reset both foreground and background
          Stosw                        ;by adding "0;" to the string
          Mov    Bx,07h                ;OldBack = 0, OldFore = 7
          And    Dh,Dh
          Jz     TestFore              ;If newback == 0 then go to foreground

SetBack:  Cmp    Dh,7                  ;If newback <= 7 then go set background
          Jle    NormBack
          Mov    Al,'5'                ;set blink code by adding "5;" then
          Stosw                        ;go set background
          Sub    Dh,8                  ;don't forget to subtract the blink bit
NormBack: Mov    Al,'4'
          Stosb                        ;Add "4" to the string
          Mov    Cl,Dh
          Mov    Si,Cx
          Mov    Al,AscColors[Si]
          Stosw                        ;Add "x;" to the string (x = color code)

TestFore: Cmp    Bl,Dl
          Je     Setm                  ;if newfore == oldfore then exit
          Cmp    Dl,7
          Jle    SetFore
          Cmp    Bl,8
          Jge    SubHigh

SetHigh:  Mov    Al,'1'                ;Add "1;" to string
          Stosw
SubHigh:  Sub    Dl,8                  ;Reduce newfore by 8 since we've done it

SetFore:  Mov    Al,'3'
          Stosb                        ;Add "3" to the string
          Mov    Cl,Dl
          Mov    Si,Cx
          Mov    Al,AscColors[Si]
          Stosb                        ;Add "x" (x = color code)
          Jmp    short Doit

Setm: ;   Cmp    byte ptr Es:[Di-1],';'
      ;   Jne    Doit
          Dec    Di                    ;backup 1 byte to remove trailing ';'
Doit:     Mov    Ax,006Dh              ;Add "m" followed by NULL terminator
          Stosw
          Mov    Al,NewColor           ;Remember the color we set it to!
          Mov    Color,Al
          Mov    Ax,1                  ;Set return code to TRUE
          Pop    Di
          Pop    Si
          Ret

Reverse:  Mov    Al,'0'                ;Reverse video - make it ESC[0;7m
          Stosw                        ;for black on white
          Mov    Al,'7'
          Stosb
          Jmp    short Doit
COLORSTR  endp

;===========================================================================
;   GETNUM
;   Reads bytes from the incoming string and turns them into a number.  The
;   reading stops as soon as the first NON-numeric value comes in.  If NO
;   numeric values come in at all then the CARRY FLAG is set.  Otherwise,
;   the CARRY FLAG is cleared and the value is returned in CX.
;===========================================================================

          ALIGN16
getnum    proc  near
          Mov   Cl,01h                 ;CL = 01h serves as a default value

          Lodsb                        ;Get first character
          Sub   Al,'0'
          Jb    gtbad
          Cmp   Al,9
          Ja    gtbad
          Mov   Cl,Al                  ;store number in CL

gtnext:   Lodsb                        ;Get next character
          Sub   Al,'0'
          Jb    gtgood
          Cmp   Al,9
          Ja    gtgood

          Mov   Ch,Cl                  ;Save previous number in CH and prepare
          Shl   Cl,1                   ;to multiply it by 10 by first doing 3
          Shl   Cl,1                   ;shift lefts (CL * 8) and then add it
          Shl   Cl,1                   ;back to itself twice making it 10.
          Add   Cl,Ch
          Add   Cl,Ch
          Add   Cl,Al                  ;Add the new number, now (CL * 10) + AL
          Jmp   short gtnext           ;Go check for more characters

gtgood:   Add   Al,'0'                 ;add '0' back into the non-numeric char
          Xor   Ch,Ch                  ;clear top of CX & unset carry flag too!
          Ret

gtbad:    Add   Al,'0'
          Stc                          ;set carry flag for bad number
          Ret
getnum    endp


;===========================================================================
;===========================================================================

          ALIGN16
docolor   proc  near
          Call  getnum
          Jc    dc0                    ;carry is set if num was bad or missing

          Cmp   Cx,47
          Ja    dcbad
          Mov   Bx,Cx
          Shl   Bx,1
          Mov   Bx,word ptr AndOrMask[Bx]
          Cmp   Bx,0FFFFh
          Je    dcbad
          And   Ah,Bl
          Or    Ah,Bh
          Mov   Color,Ah
          Ret

dc0:      Mov   Ah,07
          Mov   Color,Ah
dcbad:    Ret
docolor   endp


;===========================================================================
;===========================================================================

          ALIGN16
scroll    proc  near
          Push  Ax
          Cmp   CurRtrc,1
          Jne   scdirect

          Push  Bp                ;save BP because old PC's forget it
          Push  Dx
          Mov   Ax,0601h
          Mov   Bh,07h
          Xor   Cx,Cx
          Mov   Dl,79
          Mov   Dh,MaxRow
          Int   10h
          Pop   Dx
          Pop   Bp
          Jmp   short scexit

scdirect: Push  Ds
          Push  Si
          Push  Di
          Mov   Ax,CurAddr
          Mov   Es,Ax
          Mov   Ds,Ax
          Mov   Ah,Color               ;get current color
          Mov   Di,CurOfs
          Mov   Si,Di
          Add   Si,160

          Cld
          Mov   Cx,ScrnBytes           ;Number of bytes in screen buffer
          Shr   Cx,1                   ;Number of words in screen buffer
          Sub   Cx,80                  ;Minus one line
          Rep   Movsw                  ;scroll all the lines at once

          Mov   Al,20h                 ;spaces colored with with current color
          Mov   Cl,80
          Rep   Stosw                  ;now create a blank line

IFDEF PCBCOMM
          push  ax                      ;
          mov   ax,ds                   ; save frame
          push  ax                      ;
          mov   ax,SEG _Scrolling       ; get right segment
          mov   ds,ax                   ;
          inc   _Scrolling
;          mov   _Scrolling,1            ; set scrolling to true
          pop   ax                      ;
          mov   ds,ax                   ; restore frame
          pop   ax                      ;
ENDIF
          Pop   Di
          Pop   Si
          Pop   Ds

scexit:   Pop   Ax
          Ret
scroll    endp

;===========================================================================
;===========================================================================

          ALIGN2
clear     proc  near
          Push  Ax
          Push  Dx

IFNDEF PCBCOMM
          Cmp   CurRtrc,0
          Je    directcls
          Push  Bp
          Mov   Bh,Ah
          Mov   Ax,0600h
          Xor   Cx,Cx
          Mov   Dl,79
          Mov   Dh,MaxRow
          Int   10h
          Pop   Bp
          Jmp   short clexit
ENDIF

directcls:Cld
          Push  Di
          Mov   Cx,ScrnBytes      ;Number of bytes in screen buffer
          Shr   Cx,1              ;Number of words in screen buffer
IFDEF PCBCOMM
          mov   al,0                    ; put NULLs in buffer
ELSE
          Mov   Al,' '
ENDIF
          Mov   Di,CurOfs
          Rep   Stosw

IFDEF PCBCOMM
          push  ax                              ;
          mov   ax,es                           ;
          push  ax                              ; Save current frame
          mov   ax,ds                           ;
          push  ax                              ;
          push  di                              ;
          push  cx                              ;
          mov   ax,SEG _Changed_Lines           ;
          mov   es,ax                           ;
          mov   di,OFFSET _Changed_Lines        ; Get pointer to array
          mov   al,1                            ;
          mov   cx,25                           ;
          rep   stosb                           ; turn all lines to false
          mov   ax,SEG _clearwindow             ;
          mov   ds,ax                           ;
          mov   _clearwindow,1                  ; set clearwindow to true
          pop   cx                              ;
          pop   di                              ;
          pop   ax                              ;
          mov   ds,ax                           ;
          pop   ax                              ;
          mov   es,ax                           ; Restore status
          pop   ax                              ;
ENDIF
          Pop   Di

clexit:   Pop   Dx
          Pop   Ax
          Ret
clear     endp

;===========================================================================
;===========================================================================

          ALIGN2
clreol    proc  near
          Push  Ax
          Push  Dx

IFNDEF PCBCOMM
          Cmp   CurRtrc,0
          Je    direct
          Push  Bp
          Mov   Bh,Ah
          Mov   Ax,0600h
          Mov   Cx,Dx
          Mov   Dl,MAXCOL
          Int   10h
          Pop   Bp
          Jmp   short clrexit
ENDIF

direct:   Cld
          Xor   Cx,Cx
          Mov   Cl,MAXCOL
          Sub   Cl,Dl
          Inc   Cl
          Mov   Al,' '
          Mov   Dx,Di
          Rep   Stosw
          Mov   Di,Dx

IFDEF PCBCOMM
          push  ax                              ;
          mov   ax,ds                           ;
          push  ax                              ; Save current status
          push  si                              ;
          mov   ax,SEG _Changed_Lines           ;
          mov   ds,ax                           ;
          mov   si,OFFSET _Changed_Lines        ; Get pointer to array
          mov   bl,dh                           ;
          xor   bh,bh                           ;
          mov   BYTE PTR si[bx],1               ; We changed line DH
          pop   si                              ;
          pop   ax                              ;
          mov   ds,ax                           ; Restore status
          pop   ax                              ;
ENDIF

clrexit:  Pop   Dx
          Pop   Ax
          Ret
clreol    endp


IFNDEF LIB
           public DOESCCODES
           ALIGN2
doesccodes proc
           Mov   word ptr cs:ctrltable[27 * 2],offset escape
           Ret
doesccodes endp

           public NOESCCODES
           ALIGN2
noesccodes proc
           Mov   word ptr cs:ctrltable[27 * 2],offset noesc
           Ret
noesccodes endp
ENDIF


          ALIGN2
ctrltable:dw    exit         ; 0 - get out
          dw    pchar        ; 1 - print it
          dw    pchar        ; 2 - print it
          dw    pchar        ; 3 - print it
          dw    pchar        ; 4 - print it
          dw    pchar        ; 5 - print it
          dw    pchar        ; 6 - print it
          dw    beep         ; 7 - beep
          dw    backspace    ; 8 - backspace
          dw    tab          ; 9 - tab
          dw    linefeed     ;10 - linefeed
          dw    pchar        ;11 - print it
          dw    pchar        ;12 - print it
          dw    return       ;13 - return
          dw    pchar        ;14 - print it
          dw    pchar        ;15 - print it
          dw    pchar        ;16 - print it
          dw    pchar        ;17 - print it
          dw    pchar        ;18 - print it
          dw    pchar        ;19 - print it
          dw    pchar        ;20 - print it
          dw    pchar        ;21 - print it
          dw    pchar        ;22 - print it
          dw    pchar        ;23 - print it
          dw    pchar        ;24 - print it
          dw    pchar        ;25 - print it
          dw    ploop        ;26 - strip it
          dw    escape       ;27 - ansi escape codes

JmpTable  Dw    A            ;A - Cursor Up
          Dw    B            ;B - Cursor Down
          Dw    C            ;C - Cursor Forward
          Dw    D            ;D - Cursor Backward
          Dw    backout      ;E - invalid
          Dw    backout      ;F - invalid
          Dw    backout      ;G - invalid
          Dw    fH           ;H - Cursor Position
          Dw    backout      ;I - invalid
          Dw    J            ;J - Erase Screen
          Dw    K            ;K - Erase to End of Line
          Dw    backout      ;L - invalid
          Dw    backout      ;M - invalid
          Dw    backout      ;N - invalid
          Dw    backout      ;O - invalid
          Dw    backout      ;P - invalid
          Dw    backout      ;Q - invalid
          Dw    backout      ;R - invalid
          Dw    backout      ;S - invalid
          Dw    backout      ;T - invalid
          Dw    backout      ;U - invalid
          Dw    backout      ;V - invalid
          Dw    backout      ;W - invalid
          Dw    backout      ;X - invalid
          Dw    backout      ;Y - invalid
          Dw    backout      ;Z - invalid
          Dw    backout      ;[ - invalid
          Dw    backout      ;\ - invalid
          Dw    backout      ;] - invalid
          Dw    backout      ;^ - invalid
          Dw    backout      ;_ - invalid
          Dw    backout      ;' - invalid
          Dw    backout      ;a - invalid
          Dw    backout      ;b - invalid
          Dw    backout      ;c - invalid
          Dw    backout      ;d - invalid
          Dw    backout      ;e - invalid
          Dw    fH           ;f - Cursor Position
          Dw    backout      ;g - invalid
          Dw    ploop        ;h - Set Mode (not implemented)
          Dw    backout      ;i - invalid
          Dw    backout      ;j - invalid
          Dw    backout      ;k - invalid
          Dw    ploop        ;l - Reset Mode (not implemented)
          Dw    m            ;m - Color
          Dw    backout      ;n - invalid
          Dw    backout      ;o - invalid
          Dw    ploop        ;p - Keyboard (not implemented)
          Dw    backout      ;q - invalid
          Dw    backout      ;r - invalid
          Dw    s            ;s - Save Cursor
IFDEF MATT
          Dw    t            ;t - in door
ELSE
          Dw    backout      ;t - invalid
ENDIF
          Dw    u            ;u - Restore Cursor

;===========================================================================
;   ANSI
;   sends a string of data to the screen while interpreting ANSI codes
;===========================================================================
;***************************************************************************
;   General Register Usage is a follows:
;     AH    = Current color used in displaying characters on the screen
;     AL    = Character to be displayed on the screen
;     BX    = Scratch variable - value need not be preserved
;     CH    = While processing ESC codes a value of 0FFh for invalid number
;     CL    = While processing ESC codes the value of an ascii number
;     DX    = Current cursor position (DH = row, DL = column)
;     ES:DI = Points to next position where next character will be displayed
;     DS:SI = Points to next character in the string to be displayed
;
;***************************************************************************
          public ANSI
          ALIGN16
ANSI      proc  String:ptr
          Local InitPos:word           ;initial position when ANSI starts
          Local SaveSi:word            ;used to hold SI register for backing up
          Local TempY:byte             ;used to hold row pos during ansi move

          Push  Si
          Push  Di

ifdef MSC
ifdef LDATA
          Push  Ds
          Mov   Ax,seg _Scrn_Addr
          Mov   Ds,Ax
endif
endif

ifdef PCBCOMM
          Les   Di,dword ptr SaveScrnAddr
else
          Les   Di,dword ptr _Scrn_Addr
endif
          Mov   CurOfs,Di              ;Hold the screen offset in CurOfs
          Mov   Al,_Scrn_Rtrc
          Mov   CurRtrc,Al             ;Hold _Scrn_Rtrc in CurRtrc

ifdef MSC
ifdef LDATA
          Pop   Ds
endif
endif

          Mov   Ax,Es                  ;Save screen segment in DI
          Mov   CurAddr,Ax             ;Hold the screen segment in CurAddr
          Mov   SaveDs,Ds

if LDATA
          Lds   Si,String              ;DS:SI points to string
else
          Mov   Si,String              ;DS:SI points to string
endif

start:    Cld
          Mov   Es,Ax                  ;ES points to screen
          Mov   Dx,VirtPos
          Mov   InitPos,Dx             ;Remember cursor position in InitPos

calcofs:  Xor   Bx,Bx
          Mov   Bl,Dh
          Shl   Bx,1                   ;Bx = Y * 2 (for Yoffset array position)
          Mov   Di,[Yoffsets+Bx]       ;Di = Y * 160
          Mov   Bl,Dl                  ;Bx = X
          Add   Di,Bx                  ;Di = Y * 160 + X
          Add   Di,Bx                  ;Di = Y * 160 + X * 2
          Add   Di,CurOfs              ;ES:DI = cursor offset in video buffer

top:      Mov   Ah,Color
ploop:    Mov   Al,[Si]
          Inc   Si
          Cmp   Al,27                  ;Control characters?
          Jbe   control

pchar:    Cmp   CurRtrc,0              ;check to see if retrace is needed
          Jnz   retrace
pchar2:   Mov   Es:[Di],Ax
          Inc   Dl
          Inc   Di
          Inc   Di
          Cmp   Dl,MAXCOL
          Jbe   ploop
          Jmp   short crlf

retrace:  Mov   Cx,Dx
          Mov   Bl,Al
          Mov   Dx,03DAh
retry:    In    Al,Dx
          Test  Al,01
          Jz    retry
          Mov   Al,Bl
          Mov   Dx,Cx
          Jmp   short pchar2

exit:     Mov   VirtPos,Dx
          Cmp   Dx,InitPos             ;Did the cursor move?
          Je    skip                   ;  No, exit
          Cmp   Virtual,1
          Je    skip
          Mov   Ah,2                   ;  Yes, Update cursor position
          Xor   Bx,Bx
          Int   10h

skip:     Mov   Ds,SaveDs
          Pop   Di
          Pop   Si
          Ret

;--------------------------------------------------------------------------
noesc:    Mov   Al,''
          Jmp   short pchar
;--------------------------------------------------------------------------
control:  Xor   Bx,Bx
          Mov   Bl,Al
          Shl   Bl,1
          Jmp   word ptr cs:ctrltable[Bx]
;--------------------------------------------------------------------------
crlf:     Xor   Dl,Dl
linefeed:
IFNDEF LIB
          Cmp   CopyToBuf,0
          Je    lfeed
          Call  copyline
ENDIF
lfeed:    Inc   Dh
          Cmp   Dh,MaxRow
          Jbe   lfret
          Call  Scroll
          Mov   Dh,MaxRow
lfret:    Jmp   calcofs
;--------------------------------------------------------------------------
beep:     Push  Ax
          Push  Bx
          Push  Cx
          Push  Dx
          Push  Si
          Push  Di
          Push  Es           ; must save ES because bell() calls mydelay() and
          Push  Ds           ; mydelay() trashes ES.  Other registers are saved
          Mov   Ds,OldDs     ; "just in case", but may not be necessary
          Call  Bell
          Pop   Ds
          Pop   Es
          Pop   Di
          Pop   Si
          Pop   Dx
          Pop   Cx
          Pop   Bx
          Pop   Ax
          Jmp   ploop
;--------------------------------------------------------------------------
backspace:Sub   Dl,1
          Jnc   bsdone
          Mov   Dl,0
bsdone:   Jmp   calcofs
;--------------------------------------------------------------------------
tab:      Mov   Al,Dl                  ;Al = current cursor position
          Aam   08h                    ;divide by 8 (Al = modulo 8)
          Mov   Ah,8
          Sub   Ah,Al                  ;Ah = 8 - (Dl mod 8)
          Add   Dl,Ah                  ;Dl = Dl + (8 - (Dl mod 8))
          Cmp   Dl,MAXCOL
          Ja    crlf
          Jmp   calcofs
;--------------------------------------------------------------------------
escape:   Lodsb
          Mov   SaveSi,Si              ;Save current Si (in case it's invalid)
          Cmp   Al,'['                 ;check for esc code's bracket
          Je    scan                   ;it's valid so go process it
jumpout:  Cmp   Al,27                  ;if it was a control character then
          Jbe   control                ;  act on it
          Jmp   ploop                  ;it was bad so skip over it

scan:     Lodsb                        ;Scan for the ANSI command identifier
          Cmp   Al,'0'
          Jl    backout                ;Bad escape sequence?
          Cmp   Al,'A'
          Jb    scan                   ;parameters?
          Cmp   Al,'u'
          Ja    scan                   ;parameters?

          Xor   Bx,Bx
          Mov   Bl,Al
          Sub   Bl,'A'                 ;Set offset to 0 for the letter 'A'
          Shl   Bx,1
          Jmp   word ptr cs:JmpTable[Bx]

backout:  Mov   Si,SaveSi
          Jmp   ploop
;--------------------------------------------------------------------------
return:   Xor   Ax,Ax
          Mov   Al,Dl             ;Store X position in Ax
          Mov   Dl,Ah             ;Change X to 0
          Shl   Ax,1              ;Multiple X by 2
          Sub   Di,Ax             ;And subtract it out of our DI pointer
          Jmp   top
;--------------------------------------------------------------------------
IFDEF MATT
t:        push  ax
          mov   ax,ds                  ; save current status
          push  ax
          mov   ax,SEG _indoor
          mov   ds,ax
          Cmp   _indoor,1              ; Are we in a door?
          Jne   retloop                ; If not, get out now
          Mov   _indoor,0              ; If we're in a door, get out
          pop   ax
          mov   ds,ax                  ; restore status
          pop   ax
retloop:  Jmp   ploop
ENDIF
;--------------------------------------------------------------------------
m:        Mov   Si,SaveSi
magain:   Call  docolor
          Cmp   Al,';'                 ;Do we have more numbers to process?
          Je    magain                 ;  yes, do it again
          Cmp   Al,'m'                 ;Are we all done?
          Jne   goback                 ;Hmm, must be bad .. goto goback
          Jmp   ploop                  ;else, go to top of print loop
;--------------------------------------------------------------------------
s:        Mov   SavePos,Dx
          Jmp   ploop
;--------------------------------------------------------------------------
u:        Mov   Dx,SavePos
          Jmp   calcofs
;--------------------------------------------------------------------------
fH:       Mov   Si,SaveSi
          Mov   TempY,0      ;set TempY to 0 in case of no FIRST parameter
          Call  getnum
          Jc    fHnofirst    ;carry was set if num was bad or missing
          Or    Cl,Cl        ;Is parameter 0?
          Jz    fHstoreit    ;  yes, skip to store
          Cmp   Cl,MaxRow    ;Is parameter less than MaxRow
          Jle   fHdecy       ;  yes, it's okay
          Mov   Cl,MaxRow    ;  no, set it to MaxRow
          Inc   Cl           ;  and increment it since we're going to decrement
fHdecy:   Dec   Cl           ;decrement by 1 for 0-based addressing
fHstoreit:Mov   TempY,Cl
fHnofirst:Mov   Cl,1         ;set Cl to 1 in case there was only ONE parameter
          Cmp   Al,';'       ;is there another parameter coming?
          Jne   fH1          ;  no, skip to see if one was all we had
          Call  getnum
          Jc    goback       ;carry was set if num was bad or missing
fH1:      Cmp   Al,'f'       ;check for valid ending (either 'f' or 'H')
          Je    fHdone
          Cmp   Al,'H'
          Jne   goback
fHdone:   Mov   Dh,TempY     ;Recall the cursor ROW position
          Mov   Dl,Cl        ;Get cursor COLUMN position
          Or    Dl,Dl        ;Is parameter 0?
          Jz    fH3          ;  yes, skip over decrement
          Cmp   Dl,MAXCOL+1  ;Is parameter less than the maximum column?
          Jle   fH2          ;  yes, skip to decrement
          Mov   Dl,MAXCOL+1  ;  no, put maximum column+1 in prior to decrement
fH2:      Dec   Dl
fH3:      Jmp   calcofs

fHnone:   Cmp   Al,'f'       ;no parameters, was there a valid ending?
          Je    fHnone1
          Cmp   Al,'H'
          Jne   goback
fHnone1:  Xor   Dx,Dx        ;move it to the top left (row and column)
          Mov   Di,CurOfs    ;and the start of the screen address
          Jmp   top          ;and skip over the offset calculation
;--------------------------------------------------------------------------
goback:   Mov   Si,SaveSi
          Dec   Si
          Jmp   ploop
;--------------------------------------------------------------------------
A:        Mov   Si,SaveSi
          Call  getnum
          Cmp   Al,'A'
          Jne   goback
          Sub   Dh,Cl
          Jnc   Adone
          Xor   Dh,Dh
Adone:    Jmp   calcofs
;--------------------------------------------------------------------------
B:        Mov   Si,SaveSi
          Call  getnum
          Cmp   Al,'B'
          Jne   goback
          Add   Dh,Cl
          Cmp   Dh,MaxRow
          Jbe   Bdone
          Mov   Dh,MaxRow
Bdone:    Jmp   calcofs
;--------------------------------------------------------------------------
C:        Mov   Si,SaveSi
          Call  getnum
          Cmp   Al,'C'
          Jne   goback
          Add   Dl,Cl
          Cmp   Dl,MAXCOL
          Jbe   Cdone
          Mov   Dl,MAXCOL
Cdone:    Jmp   calcofs
;--------------------------------------------------------------------------
D:        Mov   Si,SaveSi
          Call  getnum
          Cmp   Al,'D'
          Jne   goback
          Sub   Dl,Cl
          Jnc   Ddone
          Xor   Dl,Dl
Ddone:    Jmp   calcofs
;--------------------------------------------------------------------------
J:        Mov   Si,SaveSi
          Call  getnum
          Cmp   Al,'J'
          Jne   goback
          Cmp   Cl,2
          Jne   goback
          Call  clear
          Xor   Dx,Dx             ;Move to the top left (row and column)
          Mov   Di,CurOfs         ;Di = start of screen address
          Jmp   top               ;skip over offset calculation
;--------------------------------------------------------------------------
K:        Call  clreol
          Jmp   ploop
;--------------------------------------------------------------------------
ANSI      endp


;===========================================================================
;===========================================================================

IFNDEF LIB
          public SCROLLON
          ALIGN2
scrollon  proc  BufAddr:far ptr,OfsArray:far ptr,NumLines:word
          Uses  Si,Di
          Les   Si,BufAddr
          Mov   BufSeg,Es
          Mov   BufOfs,Si

          Les   Di,OfsArray
          Mov   word ptr [AddOfsArr+2],Es
          Mov   word ptr [AddOfsArr],Di

          Mov   Cx,NumLines
          Mov   BufLines,Cx

          Mov   CopyToBuf,1            ;Indicate the Scrollback is now ON

          Cld
          Mov   Ax,Si                  ;Initialize OfsArray with offsets
soloop:   Stosw                        ;into Buffer
          Add   Ax,160
          Loop  soloop

          Mov   Ax,NumLines            ;Initialize Buffer to all spaces
          Mov   Cx,80
          Mul   Cx                     ;We won't be going beyond 64K so just
          Mov   Cx,Ax                  ;copy AX into the CX register

          Les   Di,BufAddr
          Mov   Ax,0720h               ;char = space, attribute = lightgray
          Rep   Stosw
          Ret
scrollon  endp

;===========================================================================
;===========================================================================

          public SCROLLOFF
          ALIGN2
scrolloff proc
          Mov   CopyToBuf,0
          Ret
scrolloff endp

;===========================================================================
;===========================================================================

          public REENABLESCROLL
          ALIGN2
reenablescroll  proc
          Mov   CopyToBuf,1
          Ret
reenablescroll endp

;===========================================================================
;===========================================================================

          ALIGN16
copyline  proc  near
          Uses  Ds,Es,Si,Di,Dx,Ax

          Xor   Ax,Ax
          Mov   Al,Dh
          Shl   Ax,1
          Mov   Di,Ax
          Mov   Ax,[Yoffsets+Di]
          Add   Ax,CurOfs

          Cld
          Les   Di,[AddOfsArr]
          Mov   Ds,word ptr [AddOfsArr+2]
          Mov   Dx,[Di]           ;i = Offset[0]

          Mov   Cx,BufLines;memmove(&Offset[0],&Offset[1],sizeof(int)*NUMLINES)
          Mov   Si,Di
          Inc   Si
          Inc   Si
          Rep   Movsw

          Sub   Di,2              ;Offset[NUMLINES-1] = i
          Mov   [Di],Dx

          Mov   Es,BufSeg         ;farmemcpy(&Buffer[i],&ScrnPtr[Row*160],160)
          Mov   Di,Dx
          Mov   Ds,CurAddr
          Mov   Si,Ax
          Mov   Cx,80
          Rep   Movsw
          Ret
copyline  endp

;===========================================================================
;===========================================================================

          public VIEWBUFF
          ALIGN2
viewbuff  proc  TopLine:word
          Uses  Ds,Si,Di
          Mov   Bx,TopLine
          Shl   Bx,1
          Add   Bx,word ptr [AddOfsArr]     ;Get offset of OfsArray

ifdef MSC
ifdef LDATA
          Push  Ds
          Mov   Ax,seg _Scrn_Addr
          Mov   Ds,Ax
endif
endif

          Les   Di,dword ptr _Scrn_Addr

ifdef MSC
ifdef LDATA
          Pop   Ds
endif
endif

          Cld
          Xor   Dx,Dx
vbloop:   Mov   Ds,word ptr [AddOfsArr+2]   ;Get segment of OfsArray
          Mov   Si,[Bx]
          Mov   Ds,BufSeg
          Mov   Cx,80
          Rep   Movsw
          Inc   Bx
          Inc   Bx
          Inc   Dx
          Cmp   Dx,23
          Jne   vbloop

          Ret
viewbuff  endp

;===========================================================================
;   TAGEM
;
;   Tags an area in the Scrollback Buffer by reversing the video attributes
;   (taken from the following C code:)
;
;   void pascal tagem(char X1, int Y1, char X2, int Y2, char Attr) {
;     int X;
;     int EndX;
;     int Ofs;
;
;     X1 = (X1 << 1) + 1;
;     X2 = (X2 << 1) + 1;
;     EndX = 159;
;
;     for (; Y1 <= Y2; Y1++) {
;       if (Y1 == Y2)
;         EndX = X2;
;       for (X = X1; X <= EndX; X+=2) {
;         Ofs = Offset[Y1]+X;
;         Buffer[Ofs] = Attr;
;       }
;       X1 = 1;
;     }
;   }
;
;  Register usage is as follows:
;
;     AL = holds the attribute byte
;     BX = during XLOOP holds "end value"
;     CX = temporary storage
;     DL = X1
;     DH = X2
;  DS:SI = points to OfsArray
;  ES:DI = points to Buffer
;===========================================================================

          public TAGEM
          ALIGN2
tagem     proc  X1:byte,Y1:word,X2:byte,Y2:word,Attr:byte
          Uses  Ds,Si,Di
          Lds   Si,[AddOfsArr]
          Mov   Es,BufSeg

          Cld
          Mov   Al,Attr
          Mov   Dl,X1
          Mov   Dh,X2
          Cmp   Dh,255
          Jne   tg1
          Mov   Dh,79
          Dec   Y2

tg1:      Shl   Dx,1
          Inc   Dl
          Inc   Dh

          Shl   Y1,1
          Add   Y1,Si
          Shl   Y2,1
          Add   Y2,Si

          Mov   Ch,159
          Mov   Si,Y1
yloop:    Cmp   Si,Y2
          Ja    texit
          Jne   xstart
          Mov   Ch,Dh

xstart:   Xor   Bx,Bx
          Mov   Bl,Dl
          Mov   Di,Bx
          Mov   Bl,Ch
          Add   Bx,[Si]
          Add   Di,[Si]
xloop:    Cmp   Di,Bx
          Ja    endx
          Stosb
          Inc   Di
          Jmp   short xloop

endx:     Mov   Dl,1
          Inc   Si
          Inc   Si
          Jmp   short yloop

texit:    Ret
tagem     endp

;===========================================================================
;Register usage is as follows:
;   AH = 0
;   AL = temporary storage
;   BX = during XLOOP holds "end value"
;   CX = holds SI value inside xloop
;   DL = X1
;   DH = X2
;ES:DI = points to OfsArray
;DS:SI = points to Buffer
;
;during loop ES:DI points to Gstr
;===========================================================================

          public GETTAGGED
          ALIGN2
gettagged proc  Gstr:far ptr,X1:byte,X2:byte,Y:word,Attr:byte
          Uses  Ds,Si,Di

          Les   Di,[AddOfsArr]
          Mov   Ds,BufSeg

          Mov   Dl,X1
          Mov   Dh,X2
          Shl   Dx,1

          Shl   Y,1
          Add   Y,Di
          Mov   Ah,0              ;Initialize Ah to 0
          Cld

          Mov   Di,Y

gtxstart: Mov   Bx,Es:[Di]             ;Get Array Offset Value in BX
          Mov   Si,Bx                  ;Store in SI as well
          Mov   Al,Dl
          Add   Si,Ax                  ;Si = X1
          Mov   Al,Dh
          Add   Bx,Ax                  ;Bx = X2

          Les   Di,Gstr
          Mov   Dl,Attr

gtxloop:  Cmp   Si,Bx
          Ja    gexit
          Lodsb
          Stosb
          Mov   byte ptr [Si],Dl
          Inc   Si
          Jmp   short gtxloop

gexit:    Mov   byte ptr Es:[Di],0
          Ret
gettagged endp
ENDIF

;===========================================================================
;===========================================================================

ifdef PCBCOMM
          public PREANSI
          ALIGN2
preansi   proc  String:ptr, Len:word
          Mov   Cx,Len
          Jcxz  PAend

          Push  Es
          Push  Si
          Push  Di

if LDATA
          Push  Ds
          Lds   Si,String              ;DS:SI points to string
else
          Mov   Si,String              ;DS:SI points to string
endif

          Mov   Dx,SaveLen             ;DX will still have SaveLen if we
          Or    Dx,Dx                  ;get to PAsaved, if not then DX
          Jnz   PAsaved                ;will hold the string length

          Mov   Ax,Ds
          Mov   Es,Ax
          Mov   Di,Si                  ;ES:DI now points to string
          Mov   Cx,Len                 ;store string length in CX
          Mov   Dx,Cx                  ;save string length in DX
          Add   Di,Cx                  ;add string length to string pointer
          Dec   Di                     ;ES:DI points to last char in string

          Std                          ;we're going to go backwards
          Mov   Al,27                  ;searching for an ESC character
          Repne Scasb
          Cld
          Jnz   PAdone                 ;no ESC found, just print the string

; we found an ESC character so now start going forward again to see if the
; ESC code is terminated.  If it is, then just go print the string.  If it
; is not, then we'll need to save the string and print it later when it
; is finished.

          Inc   Di                     ;reposition ES:DI on ESC character
          Mov   Bx,Di                  ;remember where we found the ESC char
          Inc   Di                     ;point to next character
          Mov   Si,Di                  ;DS:SI = char after ESC
PAtop:    Lodsb
          Cmp   Al,0                   ;string terminator?
          Je    PAsave                 ;  save and come back later
          Cmp   Al,'0'                 ;ascii 1 to 47?
          Jb    PAdone                 ;  invalid, just go print it
          Cmp   Al,'['                 ;part of ansi code?
          Je    PAtop                  ;  loop up and keep searching for end
          Cmp   Al,';'                 ;ascii 60 to 255?
          Ja    PAdone                 ;  either invalid or good end, go print
          Jmp   short PAtop            ;part of ansi code, loop back for more


PAsave:   Sub   Dx,Cx
          Mov   Cx,Dx                  ;Cx = number of bytes (ESC to end)

          Mov   Ax,@Data               ;Copy the unfinished codes into the
          Mov   Es,Ax                  ;SaveBuf holding place
          Mov   Si,Bx
          Mov   Di,offset _SaveBuf
          Mov   SaveLen,Cx
          Rep   Movsb

                                       ;Terminate the original string where
          Mov   byte ptr [Bx],0        ;the unfinished ESC code began

PAdone:

if LDATA
          Push  Ds
endif
          Mov   Ax,word ptr String
          Push  Ax
          Call  ANSI
if LDATA
          Pop   Ds
endif
          Pop   Di
          Pop   Si
          Pop   Es
PAend:    Ret


PAsaved:  Mov   Di,offset _SaveBuf
          Add   Di,Dx
          Mov   Ax,@Data
          Mov   Es,Ax                  ;ES:DI now points to end of SaveBuf

PAtop2:   Cmp   Dl,128                 ;make sure we don't overflow SaveBuf
          Ja    PAfound
          Lodsb                        ;get character from string
          Stosb

          Cmp   Al,'0'
          Jb    PAfound
          Cmp   Al,'['
          Je    PAloop
          Cmp   Al,';'
          Ja    PAfound

;         Cmp   Al,27
;         Je    PAfound
;         Cmp   Al,'['
;         Je    PAloop
;         Cmp   Al,';'
;         Je    PAloop
;         Cmp   Al,'9'
;         Ja    PAfound
;         Cmp   Al,'0'
;         Jb    PAfound

PAloop:   Inc   Dx
          Loop  PAtop2

          Mov   SaveLen,Dx
if LDATA
          Pop   Ds
endif
          Pop   Di
          Pop   Si
          Pop   Es
          Ret

PAfound:  Xor   Ax,Ax
          Mov   SaveLen,Ax
          Stosb

if LDATA
          Mov   Ax,@Data
          Push  Ax
endif
          Mov   Ax,offset _SaveBuf
          Push  Ax
          Call  ANSI

          Mov   Cx,Len
          Add   Cx,word ptr String
          Sub   Cx,Si

if LDATA
          Push  Ds
endif
          Push  Si
          Push  Cx
          Call  PREANSI

if LDATA
          Pop   Ds
endif
          Pop   Di
          Pop   Si
          Pop   Es
          Ret
preansi   endp

;===========================================================================
;===========================================================================

          public BYTEANSI
          ALIGN2
byteansi  proc  NextByte:byte
          Push  Si

if LDATA
          Push  Ds
          Mov   Ax,@Data
          Mov   Ds,Ax
endif

          Mov   Si,offset _SaveBuf     ;DS:SI  =  points to SaveBuf
          Mov   Al,NextByte            ;AL     =  character to print
          Mov   Bx,SaveLen             ;BX     =  length of SaveBuf contents

          Mov   [Si+Bx],Al             ;store NextByte into SaveBuf

          Or    Bx,Bx                  ;if BX != 0 then SaveBuf has something
          Jne   BYsaved                ;  go see if the escape code is done

          Cmp   Al,27                  ;if AL == 27 then
          Je    BYsaveit               ;  save it and come back later
                                       ;  else display what we've got

BYshow:   Xor   Ax,Ax
          Mov   SaveLen,Ax
          Mov   [Si+Bx+1],Al

if LDATA
          Push  Ds
endif
          Push  Si
          Call  ANSI

if LDATA
          Pop   Ds
endif
          Pop   Si
          Ret

BYsaved:  Cmp   Bl,120                 ;make sure we don't overflow SaveBuf
          Ja    BYshow

          Cmp   Al,'0'
          Jb    BYshow
          Cmp   Al,'['
          Je    BYsaveit
          Cmp   Al,';'
          Ja    BYshow

BYsaveit: Inc   Bx
          Mov   SaveLen,Bx

if LDATA
          Pop   Ds
endif
          Pop   Si
          Ret
byteansi  endp

endif

;===========================================================================
;===========================================================================

if LPROG
  _ansiemu  ends
else
  CSegEnd@
endif

          end
