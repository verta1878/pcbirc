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


;  Timer0 - modem CTS/RTS flow control timer
;             used to insure that the flow does not remain stopped for
;             more than 60 seconds
;
;  Timer1 - keyboard timer
;             used in a session to determine if the keyboard has timed out
;
;  Timer2 - status line/usernet timer
;             used in a session to determine when to update the status line
;           callwait clock timer
;             used in the call waiting screen to update the time of day
;
;  Timer3 - showerror timer
;             used to determine the 2 second retry intervals on networks
;           modem timer
;             used to determine how long to wait for "OK" to come back
;
;  Timer4 - file transfer timer
;             used to watch file timeouts during file transfers
;           callwait modem timer
;             used to count down seconds before recycling the modem
;           sysop page
;             used to count off 2-second increments
;           activity timer
;             used to count off 1-second increments for activity display
;           msg entry ESC timer
;             used to count off 2/10ths of a second for ESC validation
;           event wait timer
;             count down timer for event buffer period
;           modem initialization timer
;             count down 10 seconds before modem reset error
;           network error retry (found in showerr.c and messages.c)
;             retry count down based on network timeout delay
;           node chat timer
;             count down .5 seconds before checking for new chat data
;           printer busy timer
;             count down ten seconds then turn printer off
;
;  Timer5 - session countdown timer
;  Timer6 - session elapsed timer
;
;  Timer7 - usernet access timer
;             used to determine when to access the usernet.dat file
;
;  Timer8 - save timer
;             used to remember session time during login sequence
;           call wait modem-reset timer
;             used to reset the modem every 5 minutes if no RING occurs
;           chat lock timer
;             used to avoid overly fast attempts at locking CHAT.DAT
;
;  Timer9 - chat access timer
;             used to control frequency of disk access
;           goodbye timer
;             used to count off 1-second increments for 10-second goodbye
;           call wait - fido send mail timer
;             used to decide when to check for outgoing mail
;
;  Timer10 - chat keyboard timer
;             used to control relinquishing of the focus
;            call wait - fido export timer
;             used to decide when to check for exporting mail
;
;  Timer11 - call wait - fido packet frequency
;             used to decide when to process (import) mail that has been received
;
;  Timer12 - alias protector
;             used to decide when to update the usernet file
;           callwait screen timer
;             used to control the 3 minute screen clear


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

public   SETTIMER, GETTIMER, GETTICKS

.data
even
STimer   dd  13 dup (0)           ;set up 10 long integer start times
ETimer   dd  13 dup (0)           ;set up 10 long integer end times

if LPROG
   timers  segment para public 'code'
   assume    cs:timers
   ALIGN2    equ ALIGN 2
   ALIGN16   equ ALIGN 16
else
   CSeg@
   ALIGN2    equ ALIGN 1
   ALIGN16   equ ALIGN 1
endif

         ALIGN16
SETTIMER proc Which:word, Ticks:dword
         Mov  Bx,[Which]
         Shl  Bx,1
         Shl  Bx,1

         Xor  Ax,Ax
         Mov  Es,Ax
         Les  Ax,Es:[46Ch]        ;load DoubleWord in thru ES:AX
         Mov  Dx,Es

         Mov  word ptr STimer[Bx],Ax
         Mov  word ptr STimer[Bx+2],Dx

         Add  Ax,word ptr [Ticks]
         Adc  Dx,word ptr [Ticks+2]

         Mov  word ptr ETimer[Bx],Ax
         Mov  word ptr ETimer[Bx+2],Dx
         Ret
SETTIMER endp

         ALIGN16
GETTIMER proc Which:word
         Uses Si
         Mov  Si,[Which]
         Shl  Si,1
         Shl  Si,1

         Xor  Ax,Ax
         Mov  Es,Ax
         Les  Bx,Es:[46Ch]        ;load DoubleWord in thru ES:BX
         Mov  Cx,Es

         Mov  Ax,word ptr ETimer[Si]
         Mov  Dx,word ptr ETimer[Si+2]
         Sub  Ax,Bx
         Sbb  Dx,Cx

;Compare current time to start time by subtracting start time from current time
;check flags to see if current time is less than start time (if so, we've
;crossed midnight)
;
;SUB sets the same flags as CMP so jump to Rolled if current time is less than
;start time or jump to Okay if current time is greater

         Sub  Bx,word ptr STimer[Si]
         Sbb  Cx,word ptr STimer[Si+2]
         Jl   Rolled
Okay:    Ret

;Having gotten this far the CX:BX long integer pair now have a value that
;equates to the DIFFERENCE between the current time and the start time.  At
;this point we are going to verify that midnight has REALLY rolled over.

Rolled:  Cmp  Cx,0FFFFh                ;if CX = FFFF then we're less than
         Je   Okay                     ;0000FFFF clock ticks away from midnight
                                       ;which means time might have simply gone
                                       ;backwards so don't adjust for midnight
                                       ;rollover.

         Sub  Ax,00B0h                 ;Add 1800B0h for the midnight time
         Sbb  Dx,18h
         Ret
GETTIMER endp


GETTICKS proc
         Xor  Ax,Ax
         Mov  Es,Ax
         Les  Ax,Es:[46Ch]        ;load DoubleWord in thru ES:AX
         Mov  Dx,Es
         Ret
GETTICKS endp


;  public GETTIME
;  GETTIME  proc Which:word
;           Xor  Ax,Ax
;           Mov  Es,Ax
;           Les  Ax,Es:[46Ch]        ;load DoubleWord in thru ES:BX which does
;           Mov  Dx,Es               ;not require disabling interrupts! (CX:BX)
;           Ret
;  GETTIME  endp
;
;  public GETDIFF
;  GETDIFF  proc Which:word
;           Uses Si
;           Mov  Si,[Which]
;           Shl  Si,1
;           Shl  Si,1
;
;           Xor  Ax,Ax
;           Mov  Es,Ax
;           Les  Ax,Es:[46Ch]        ;load DoubleWord in thru ES:BX which does
;           Mov  Dx,Es               ;not require disabling interrupts! (CX:BX)
;
;           Sub  Ax,word ptr STimer[Si]
;           Sbb  Dx,word ptr STimer[Si+2] ;Check for midnight rollover
;
;           Jnc  Done
;           Neg  Dx
;           Neg  Ax
;           Sbb  Dx,0
;
;  Done:    Ret
;  GETDIFF  endp

if LPROG
  timers  ends
else
  CSegEnd@
endif

         end
