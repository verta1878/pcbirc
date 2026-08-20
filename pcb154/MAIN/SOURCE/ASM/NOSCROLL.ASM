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

CSeg           Segment  Para Public 'CODE'
               assume cs:CSeg, ds:nothing, es:nothing, ss:nothing

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³  NoScroll.Asm  v.1.2                                                       ³
;³                                                                            ³
;³  A program to restrict the scrolling of output to the screen.              ³
;³                                                                            ³
;³  by R. P. Byrne - 8/15/89                                                  ³
;³                                                                            ³
;³  Syntax:   NoScroll [n]                                                    ³
;³                                                                            ³
;³   where n represents the first row on the screen that will be allowed to   ³
;³   scroll (if not specified, defaults to line 4)                            ³
;³                                                                            ³
;³  First call installs this utility, second call de-installs it              ³
;³                                                                            ³
;³                                                                            ³
;³  Credit where due:  This program is based on concepts implemented by       ³
;³                     Sam Smith for his Prons.Asm program.                   ³
;³                                                                            ³
;³  MODIFIED to be linked into PCBoard v15.0 by David W. Terry                ³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³  Data Storage Area (stored in data segment)                                 ³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

MaxRow         equ      24
FirstRow       db       3       ;(first scrollable row on screen) - 1

OldInt10       dd       ?        ;the "real" bios video isr
OldInt21       dd       ?        ;the "real" dos function call isr

DummyReturn    dw       ?        ;just a dummy variable used for stack adjustment

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³  Commonly called subroutines                                                ³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

UseOldInt10    Proc  Near
               push        bp       ;save bp just in case machine has old bios bug
               pushf                ;push flags to simulate an int instruction
               call dword ptr cs:OldInt10    ;call bios video service
               pop         bp       ;restore base pointer
               ret                  ;return to caller
UseOldInt10    EndP

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³  Interrupt service routine (ISR) for Bios Video interrupt                   ³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

Entry10        Proc  far
               sti
   Chk0E:      cmp         ah,0Eh        ;TTY write request?
               jnz         Chk06         ;no, go check for scroll request
               cmp         al,0Ah        ;yes.  is character a linefeed?
               jnz         AllOther      ;nope ... no special treatment
               call        ChkTTY        ;yup ... go take care of it
               jmp short   AllDone

   Chk06:      cmp         ah,06h        ;scroll page request?
               jnz         Chk02         ;no, go check for cursor move request
               call        ChkScroll     ;yes, go take care of it
               jmp short   AllDone

   Chk02:      cmp         ah,02h        ;set pos request?
               jnz         AllOther      ;no ... no special treatment
               cmp         dh,FirstRow   ;request outside of window?
               jae         AllOther      ;nope ...
               call        ChkCursor     ;yup ... go fix it
               jmp short   AllDone

   AllOther:   call        UseOldInt10   ;call the "real" int 10h for service

   AllDone:    IRet                      ;Return to requestor of video service

Entry10        EndP

;ÄÄÄÄÄÄ[ Int 10h, function 0Eh - TTY write request ]ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

ChkTTY         Proc  near

               push        ax
               push        bx
               push        cx
               push        dx
               push        si
               push        di
               push        ds
               push        es
               push        bp

               push        ax                   ;save ax
               mov         bh,0                 ;video page 0
               mov         ah,03h               ;call Bios to get the current
               call        UseOldInt10          ;    cursor position into DX
               pop         ax                   ;restore ax

   ChkNewRow:  inc         dh                   ;adjust row number for LF
               cmp         dh,MaxRow            ;outside the window?
               ja          ScrollIt             ;yes ... scroll the window
               call        UseOldInt10          ;no, let "real" bios handle it
               jmp         TTY_GoBack

   ScrollIt:   mov         ah,06h               ;video scroll up function
               mov         al,1                 ;only scroll 1 line
               mov         ch,FirstRow          ;
               xor         cl,cl                ;upper left corner = FirstRow,0
               mov         dh,MaxRow            ;
               mov         dl,4Fh               ;lower right corner = MaxRow,79
               mov         bh,07h               ;use white on black attribute
               call        UseOldInt10          ;Call Int 10h

   MovCursor:  mov         bh,0                 ;video page 0
               mov         ah,2                 ;set cursor function
               mov         dh,MaxRow            ;1st col of last row
               mov         dl,0                 ;  ie. MaxRow,0
               call        UseOldInt10          ;move the cursor

TTY_GoBack:    pop         bp
               pop         es
               pop         ds
               pop         di
               pop         si
               pop         dx
               pop         cx
               pop         bx
               pop         ax

               ret

ChkTTY         EndP

;ÄÄÄÄÄÄ[ Int 10h, function 6 - scroll page up request ]ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

ChkScroll      Proc  near

               cmp         ch,FirstRow    ;row of UL corner <= max?
               jae         ULisOK         ;yes, go check LR corner
               mov         ch,FirstRow    ;no, adjust it
   ULisOK:     cmp         dh,MaxRow      ;row of LR corner <= max?
               jbe         LRisOK         ;yes, no adjustment needed
               mov         dh,MaxRow      ;no, adjust it
   LRisOK:     call        UseOldInt10    ;let old ISR do the scroll
               ret

ChkScroll      EndP

;ÄÄÄÄÄÄ[ Int 10h, function 2 - set cursor position request ]ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

ChkCursor      Proc  near

               mov         dh,FirstRow    ;fix the 1st row to be scrolled
   CursorOk:   call        UseOldInt10    ;Let "real" bios move cursor
               ret

ChkCursor      EndP

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³  Interrupt service routine (ISR) for Dos function request interrupt         ³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

Entry21        Proc  far
               sti

   Chk09D:     cmp         ah,9
               jnz         Chk02D
               call        ChkString
               jmp short   AlldoneD


   Chk02D:     cmp         ah,2          ;console output request?
               jnz         Chk40D        ;no, go check for file/device write
               cmp         dl,0Ah        ;yes.  is character a linefeed?
               jnz         AllOtherD     ;nope ... no special treatment
               call        ChkConOut     ;yup ... go take care of it
               jmp short   AllDoneD

   Chk40D:     cmp         ah,40h        ;device output request?
               jnz         AllOtherD     ;no ... no special treatment
               cmp         bx,1          ;standard output device?
               jne         ChkStdErr     ;no, see if std err
               jcxz        AllOtherD     ;if char count 0, no special treatment
               call        ChkWrite      ;yes, go watch for linefeeds ...
               jmp short   AllDoneD      ;   and then exit
   ChkStderr:  cmp         bx,2          ;standard error device?
               jne         AllOtherD     ;no, no special treatment
               call        Chkwrite      ;yup ... go handle the request
               jmp short   AllDoneD

   AllOtherD:  jmp dword ptr cs:OldInt21     ;jump into dos

   AllDoneD:   Retf 2                    ;Return to requestor of service

Entry21        EndP

;ÄÄÄÄÄÄ[ Int 21h, function 09h - write string to console ]ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

ChkString      Proc     near
;
; Calling program has requested a write to either the standard output device or
; the standard error device
;
               push        bx          ;preserve all registers that don't
               push        cx          ;   return status information
               push        dx          ;   for the call
               push        si          ;
               push        di          ;   CF acts as an error flag
               push        ds          ;   AX returns number of bytes written
               push        es          ;
               push        bp          ;

               mov         si,dx       ;ds:si --> string to display
   OneChar:    lodsb                   ;get a byte to display
               cmp         al,'$'      ;end of string?
               jz          NoMore      ;yes...time to quit
               mov         dl,al       ;load dl for dos call
               mov         ah,02h      ;dos console output function
               int         21h         ;display one character
               jmp short   OneChar     ;repeat for all characters in buffer

   NoMore:     pop         bp
               pop         es
               pop         ds
               pop         di
               pop         si
               pop         dx
               pop         cx
               pop         bx

               clc                     ;clear CF for return

               ret

ChkString      EndP

;ÄÄÄÄÄÄ[ Int 21h, function 40h - write to file/device ]ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

ChkWrite       Proc     near
;
; Calling program has requested a write to either the standard output device or
; the standard error device
;
               push        bx          ;preserve all registers that don't
               push        cx          ;   return status information
               push        dx          ;   for the call
               push        si          ;
               push        di          ;   CF acts as an error flag
               push        ds          ;   AX returns number of bytes written
               push        es          ;
               push        bp          ;

               mov         si,dx       ;ds:si --> string to display
   DisplayOne: lodsb                   ;get a byte to display
               mov         dl,al       ;
               mov         ah,02h      ;dos console output function
               int         21h         ;display one character
               loop        DisplayOne  ;repeat for all characters in buffer

               pop         bp
               pop         es
               pop         ds
               pop         di
               pop         si
               pop         dx
               pop         cx
               pop         bx

               clc                     ;clear CF for return
               mov         ax,cx       ;set byte count for return

               ret

ChkWrite       EndP

;ÄÄÄÄÄÄ[ Int 21h, function 02h - write to console ]ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

ChkConOut      Proc     near

               ; calling program has requested a linefeed be sent to either
               ; the standard output or standard error device.  Use int 10h
               ; to satisfy the request while checking for scroll out of
               ; window
               push        ax
               push        bx
               push        cx
               push        dx
               push        si
               push        di
               push        ds
               push        es
               push        bp

               mov         ax,0E0Ah
               int         10h

               pop         bp
               pop         es
               pop         ds
               pop         di
               pop         si
               pop         dx
               pop         cx
               pop         bx
               pop         ax

               clc                  ;clear carry for return

               ret

ChkConOut      EndP

;ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
;³  Program installation section                                               ³
;ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

               public   BEGNOSCROLL
begnoscroll    proc     far
               mov      ah,35h         ;Get vector
               mov      al,10h         ;   for int 10h
               int      21h
               mov word ptr [OldInt10],bx       ;and store in memory for later
               mov word ptr [OldInt10+2],es  ;store in OldInt10

               push     ds
               mov      ax,cs
               mov      ds,ax

               mov      ah,25h         ;set new vector for ...
               mov      al,10h         ;   int 10h to use the routine at Entry10
               mov      dx,offset Entry10    ;ds:dx ---> new int 10 isr
               int      21h

               mov      ah,35h         ;Get vector
               mov      al,21h         ;   for int 21h
               int      21h
               mov word ptr [OldInt21],bx       ;and store in memory for later
               mov word ptr [OldInt21+2],es  ;store in OldInt10

               mov      ah,25h         ;set new vector for ...
               mov      al,21h         ;   int 21h to use the routine at Entry10
               mov      dx,offset Entry21    ;ds:dx ---> new int 21 isr
               int      21h

               pop      ds
               ret
begnoscroll    endp

               public   ENDNOSCROLL
endnoscroll    proc     far
               mov      ah,35h         ;function 35h retrieves interrupt vector
               mov      al,10h         ;we're interested in interrupt 10h
               int      21h            ;get int 10 vector into es:bx

               push     ds             ;save our data seg for final message
               lds      dx,OldInt10    ;load addr of original isr into ds:dx
               mov      ah,25h         ;set interrupt vector
               mov      al,10h         ;reset int 10h isr back to where it was ...
               int      21h            ;  before user installed NoScroll
                                       ;
               lds      dx,OldInt21    ;load addr of original isr into ds:dx
               mov      ah,25h         ;set interrupt vector
               mov      al,21h         ;reset int 21h isr back to where it was ...
               int      21h            ;  before user installed NoScroll
               pop      ds             ;restore our data segment register
               ret
endnoscroll    endp

CSeg           EndS
               end
