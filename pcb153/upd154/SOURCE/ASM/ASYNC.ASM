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

extrn MYDELAY
extrn _VerifyCDLoss:byte
extrn _FORCE16550A:byte
extrn _NO16550:byte
extrn _Allow16650:byte

;------- Async EQUATES ----------------------------------------------

;.................. offsets from base of UART Registers
UART_THR  Equ   00h               ;Transmit Holding Register
UART_RBR  Equ   00h               ;Receiver Buffer Register
UART_DLL  Equ   00h               ;Divisor Latch LSB (when DLAB = 1)
UART_DLM  Equ   01h               ;Divisor Latch MSB (when DLAB = 1)
UART_IER  Equ   01h               ;Interrupt Enable Register
UART_IIR  Equ   02h               ;Interrupt Identification Register (read only)
UART_FCR  Equ   02h               ;FIFO Control Register  (write only - same as IIR)
UART_EFR  Equ   02h               ;Extended Feature Register (16650 only - requires LCR bit7 set to 1)
UART_LCR  Equ   03h               ;Line Control Register
UART_MCR  Equ   04h               ;Modem Control Register
UART_LSR  Equ   05h               ;Line Status Register
UART_MSR  Equ   06h               ;Modem Status Register
UART_SCR  Equ   07h               ;Scratch Register

PICMSK1   Equ   0021h             ;8259 interrupt mask for INT's 0 thru 7
PICMSK2   Equ   00A1h             ;8259 interrupt mask for INT's 8 thru 15
C8259_1   Equ   0020h             ;Address of 8259 for INT's 0 thru 7
C8259_2   Equ   00A0h             ;Address of 8259 for INT's 8 thru 15
EOI       Equ   0020h             ;End of Interrupt

RTSOFF    Equ   00011101b         ;mask to turn off the RTS signal
RTSON     Equ   00000010b         ;mask to turn on the RTS signal

CDMASK    Equ   10000000b         ;mask to test CD signal (non-zero = on)
CTSMASK   Equ   00010000b         ;mask to test CTS signal (non-zero = on)
THREMASK  Equ   00100000b         ;mask to test THRE indicator (non-zero = empty)
RINGMASK  Equ   00000100b         ;mask to test RING indicator (non-zero = on)
LOOPMASK  Equ   00010000b         ;mask to turn on loop-back test

INBUFSIZE  Equ   4096             ;size of comm input buffer
INBUFWRAP  Equ   4095             ;MASK used for looping the pointer around
OUTBUFSIZE Equ   2048             ;size of comm output buffer
OUTBUFWRAP Equ   2047             ;MASK used for looping the pointer around

RCVEINT   Equ   00001101b         ;00001101b
XMITINT   Equ   00001111b         ;00001111b
                                  ;    |||+- Received Data Available interrupt
                                  ;    ||+-- Transmit Holding Register Empty interrupt
                                  ;    |+--- Receiver Line Status interrupt
                                  ;    +---- Modem Status interrupt


;   Global MACRO definitions
;   ~~~~~~~~~~~~~~~~~~~~~~~~
;-------------------------------------------------------------------------
GetV      Macro Register,VarName  ;Move VAR off stack into register
          Mov   Si,VarName
          Mov   Register,[Si]
          EndM
;-------------------------------------------------------------------------
PutV      Macro VarName,Register  ;Move VAR from register onto Stack
          Mov   Si,VarName
          Mov   [Si],Register
          EndM
;-------------------------------------------------------------------------
Dos       Macro Function          ;Perform DOS function call
          Mov   Ah,Function
          Int   21H
          EndM
;-------------------------------------------------------------------------
Jmps      Macro Dummy
          Jmp   Short Dummy
          EndM
;-------------------------------------------------------------------------
GetPort   Macro PortAddr
          Mov   Dx,PortAddr
          In    Al,Dx
          EndM
;-------------------------------------------------------------------------
PutPort   Macro PortAddr
          Mov   Dx,PortAddr
          Out   Dx,Al
          EndM
;-------------------------------------------------------------------------
EnableEFR Macro
          GetPort [LCR_Port]
          Or      Al,10000000b      ;set LCR bit7 to enable EFR Register
          PutPort [LCR_Port]
          EndM
;-------------------------------------------------------------------------
DisableEFR Macro
          GetPort [LCR_Port]
          And     Al,01111111b      ;unset LCR bit7 to disable EFR Register
          PutPort [LCR_Port]
          EndM
;-------------------------------------------------------------------------
EnableIER Macro                     ;NOTE:  EFR must be enabled!
          GetPort [EFR_Port]
          Or      Al,00010000b      ;set EFR bit4 to enable IER bits 4-7
          PutPort [EFR_Port]
          EndM
;-------------------------------------------------------------------------
DisableIER Macro                    ;NOTE:  EFR must be enabled!
          GetPort [EFR_Port]
          And     Al,11101111b      ;unset EFR bit7 to disable IER bits 4-7
          PutPort [EFR_Port]
          EndM
;-------------------------------------------------------------------------


;declare the variables as public so that the debugger can find them
;public Async_Irq,ComSaveVec
public _InWritePtr,_InReadPtr,_OutWritePtr,_OutReadPtr
public _InBuffer,_OutBuffer

;public ComPort_Open_Flag,DLL_Port,THR_Port,RBR_Port,DLM_Port
;public IER_Port,IIR_Port,LCR_Port,MCR_Port,LSR_Port,MSR_Port,CheckCTS

;public variables that can be accessed by the C code:
public __InBytes,__OutBytes,__OverrunErrors,__ParityErrors,__FramingErrors
public _RTSoff,_B16550,_B16550A,_B8250,__CTSokay
public __RingDetect,_CDokay,_CDup,_B16650

.data
                   ALIGN2
_InBuffer          dd   ?
_OutBuffer         dd   ?

Parity_Array       db  0,18h
Bits_Array         db  3,2
Async_Irq          db  0          ;irq for current open port

                   ALIGN2
_InReadPtr         dw  0          ;read  pointer for _InBuffer
_InWritePtr        dw  0          ;write pointer for _InBuffer
_OutReadPtr        dw  0          ;read  pointer for _OutBuffer
_OutWritePtr       dw  0          ;write pointer for _OutBuffer

DLL_Port           dw  0          ;#F8 - Port address for Low Byte of Divisor Latch
THR_Port           dw  0          ;#F8 - Port address for Transmit Holding Register
RBR_Port           dw  0          ;#F8 - Port address for Receiver Buffer Register
DLM_Port           dw  0          ;#F9 - Port address for High Byte of Divisor Latch
IER_Port           dw  0          ;#F9 - Port address for Interrupt Enable Register
IIR_Port           dw  0          ;#FA - Port address for Interrupt Identification Register (read only)
FCR_Port           dw  0          ;#FA - Port address for FIFO Control Register (write only - same as IIR)
EFR_Port           dw  0          ;#FA - Port address for Extended Feature Register (requires LCR bit7)
LCR_Port           dw  0          ;#FB - Port address for Line Control Register
MCR_Port           dw  0          ;#FC - Port address for Modem Control Register
LSR_Port           dw  0          ;#FD - Port address for Line Status Register
MSR_Port           dw  0          ;#FE - Port address for Modem Status Register
SCR_Port           dw  0          ;#FF - Port address for Scratch Register

OutCount           dw  0          ;set to 16 or 32 depending on if 16550 or 16650
LowWater           dw  0          ;LowWater limit for receive buffer
HighWater          dw  0          ;HighWater limit for receive buffer
PausedOutBytes     dw  0          ;Saved value of __OutBytes when output is paused

ComPort_Open_Flag  db  0
ShareIRQ           db  0          ;TRUE if on a PS/2 and we're sharing IRQ's
CheckCTS           db  1          ;TRUE if CTS is signal checking is available
__CTSokay          db  1          ;Non-Zero if CTS signal is up
_CDokay            db  128        ;Non-Zero if CD signal is okay (NEVER been lost)
_CDup              db  0          ;Non-Zero if CD signal is up
_B16550            db  0          ;Non-zero if a 16550 or 16550A chip is installed
_B16550A           db  0          ;Non-zero if a 16550A chip is installed
_B16650            db  0          ;Non-zero if a 16650 chip is installed
_B8250             db  0          ;Non-Zero if an 8250 chip is installed
__RingDetect       db  0          ;Non-Zero if a Ring Detect occured
FCR_value          db  0          ;Value of FIFO Control Register
                                  ;FCR is read-only, so we store it internally


;The following may be read from with C code
;------------------------------------------
                   ALIGN2
__OverrunErrors    dw  0          ;number of overrun errors
__ParityErrors     dw  0          ;number of parity errors
__FramingErrors    dw  0          ;number of framing errors
__InBytes          dw  0          ;number of bytes in the input queue
__OutBytes         dw  0          ;number of bytes in the output queue
_CharWaitTime      dw  0          ;Wait time in 1/100's seconds between chars
_RTSoff            db  0          ;TRUE if RTS is OFF



if LPROG
   async  segment para public 'code'
   assume    cs:async
else
   CSeg@
endif

                   ALIGN2
saveds             dw  ?          ;our data segment
ComSaveVec         dd  0          ;Saved Vector for Comm Interrupt

;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  DOS_Set_Intrpt                                       ;
;                                                                      ;
;     Purpose:    Calls DOS to set interrupt vector                    ;
;                                                                      ;
;     Calling Parameters:                                              ;
;          Al --- interrupt vector number to set                       ;
;          Es --- segment address of interrupt routine                 ;
;          Dx --- offset address of interrupt routine                  ;
;                                                                      ;
;      Trashes:  Ah                                                    ;
;                                                                      ;
;----------------------------------------------------------------------;
DOS_Set_Intrpt  Proc  Near
          Push  Ds
          Push  Es
          Pop   Ds
          Mov   Ah,25h
          Int   21h
          Pop   Ds
          Ret
DOS_Set_Intrpt  EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  DOS_Get_Intrpt                                       ;
;                                                                      ;
;     Purpose:    Calls DOS to get interrupt vector                    ;
;                                                                      ;
;     Calling Parameters:                                              ;
;          Al --- interrupt vector number to get                       ;
;                                                                      ;
;     Returns:                                                         ;
;          Es --- segment address of interrupt routine                 ;
;          Bx --- offset address of interrupt routine                  ;
;                                                                      ;
;     Trashes:  Ah                                                     ;
;                                                                      ;
;----------------------------------------------------------------------;
DOS_Get_Intrpt  Proc  Near
          Mov   Ah,35h
          Int   21h
          Ret
DOS_Get_Intrpt  EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  TurnOnXmt                                            ;
;                                                                      ;
;     Purpose:    If there is data in the outbuffer AND if CTS is okay ;
;                 then it enables the "transmitter register empty"     ;
;                 interrupt to turn on the transmit capability.        ;
;                                                                      ;
;     NOTE:  coded as a MACRO to avoid function call overhead          ;
;                                                                      ;
;----------------------------------------------------------------------;
TurnOnXmt Macro CheckOut
          Local tox0, tox1
          ifnb  <CheckOut
          Cmp   [__OutBytes],0    ;is there anything in the output buffer?
          Je    tox1              ;  no, get out of here
          endif

          Mov   Dx,[MSR_Port]
          In    Al,Dx
          Mov   Ah,Al

          And   Ah,CDMASK         ;Check CD signal - if CD signal is up
          Mov   [_CDup],Ah        ;then store a non-Zero value in CDup

          Cmp   [CheckCTS],0      ;are we ignoring CTS checking?
          Je    tox0              ;if so then skip the CTS check

          And   Al,CTSMASK        ;Check CTS signal - if CTS signal is up
          Mov   [__CTSokay],Al    ;then store a non-Zero value in CTSokay
          Jz    tox1

tox0:     Mov   dx,[IER_Port]     ;we made it this far - so turn on the
          Mov   al,RCVEINT        ;transmitter interrupt by toggling it off
          Out   dx,al             ;and then back on
          Mov   al,XMITINT        ;transmitter interrupt by toggling it off
          Out   dx,al             ;and then back on
tox1:
          EndM


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  TurnOnXmit                                           ;
;                                                                      ;
;     Purpose:    Uses the TURNONXMT macro to re-enable XMIT int's     ;
;                                                                      ;
;     Called from C                                                    ;
;----------------------------------------------------------------------;
           Public ASYNC_TURNONXMIT
           ALIGN16
ASYNC_TurnOnXmit Proc
          ;Cli
          ;Cmp   [__OutBytes],0  ;is there anything in the output buffer?
          ;Je    TOX2            ;  no, get out of here
          ;
          ;Cmp   [CheckCTS],0    ;are we ignoring CTS checking?
          ;Je    TOX1            ;if so then skip the CTS check
          ;
          ;Mov   Dx,[MSR_Port]
          ;In    Al,Dx
          ;Mov   Ah,Al
          ;
          ;And   Ah,CDMASK       ;Check CD signal - if CD signal is up
          ;Mov   [_CDup],Ah      ;then store a non-Zero value in CDup
          ;
          ;And   Al,CTSMASK      ;Check CTS signal - if CTS signal is up
          ;Mov   [__CTSokay],Al  ;then store a non-Zero value in CTSokay
          ;Jz    TOX2
          ;
TOX1:     ;Mov   Dx,[IER_Port]   ;we made it this far - so turn on the
          ;Mov   Al,XMITINT      ;transmitter interrupt
          ;Out   Dx,Al
TOX2:     ;Sti
          ;Ret

           Cli
           TurnOnXmt CheckOut
           Sti
           Ret
ASYNC_TurnOnXmit EndP



;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  Async_Interrupt                                      ;
;                                                                      ;
;     Purpose:    Invoked when UART has RECEIVED a character from the  ;
;                 comm port or when ready to SEND a character          ;
;                                                                      ;
;----------------------------------------------------------------------;
ALIGN16
Asynch_Interrupt   PROC  FAR
        push  ax
        push  bx
        push  dx
        push  ds

        mov   ds,cs:saveds
        mov   dx,[IIR_Port]
        in    al,dx               ;get the byte to see if my chip interrupted
        test  al,1                ;is there an interrupt pending?
        jnz   notmine             ;1=no interrupt pending so get out

        and   ax,00000110b        ;mask off all but bits 1 & 2
        mov   bx,ax
        jmp   word ptr cs:jmptble[bx]

notmine:cmp   ShareIRQ,1          ;on a PS/2 we can share IRQ's.  If it's not
        jne   myexit              ;a PS/2 then terminate the interrupt

        pop   ds
        pop   dx
        pop   bx
        pop   ax
        jmp   ComSaveVec          ;call previous interrupt routine


; If we get here it's because we were INSIDE OUR INTERRUPT HANDLER and we are
; now on our way out.

myexit: mov   al,EOI
        cmp   [Async_IRQ],8
        jb    myexit2
        out   C8259_2,al
myexit2:out   C8259_1,al
        pop   ds
        pop   dx
        pop   bx
        pop   ax
        iret

;----------------------------------------------------------------------

        ALIGN16
L_TX:   mov   dx,[LSR_Port]
        in    al,dx               ;Check to see if the Transmitter Holding
        test  al,THREMASK         ;Register Empty (THRE) indicator is REALLY on
        jz    Loop01              ;If not, jump out now

        push  es                  ;save ES register
        push  si                  ;save SI register
        les   si,[_OutBuffer]
        mov   bx,[_OutReadPtr]
        mov   dx,[THR_Port]

        mov   al,es:[si+bx]       ;al = byte to send out
        out   dx,al
        inc   bx
        and   bx,OUTBUFWRAP       ;wrap outptr to 0 if necessary
        dec   [__OutBytes]        ;set Zero Flag if OutBytes goes to 0
        mov   [_OutReadPtr],bx
        pop   si                  ;restore SI register
        pop   es                  ;restore ES register
        jmps  txdone

;----------------------------------------------------------------------

        ALIGN16
Loop01: mov   dx,[IIR_Port]   ;Find out if an interrupt is pending...
        in    al,dx           ;get the byte
        test  al,1
        jnz   myexit          ;1=no interrupt pending so get out now

        and   ax,00000110b    ;mask off all but bits 1 & 2
        mov   bx,ax
        jmp   word ptr cs:jmptble[bx]

        ALIGN2
jmptble label word           ;values correspond to 0, 2, 4 and 6 (divided by 2)
        dw    L_MS           ;offset 0 = modem status interrupt
        dw    L_TX           ;offset 1 = transmitter empty interrupt
        dw    L_RX           ;offset 2 = received data interrupt
        dw    L_LS           ;offset 3 = line status interrupt

;----------------------------------------------------------------------

        ALIGN16
TXFIFO: mov   dx,[LSR_Port]
        in    al,dx               ;Check to see if the Transmitter Holding
        test  al,THREMASK         ;Register Empty (THRE) indicator is REALLY on
        jz    Loop01              ;If not, jump out now

        push  es                  ;save ES register
        push  si                  ;save SI register
        push  cx                  ;save CX register
        les   si,[_OutBuffer]
        mov   dx,[THR_Port]
        mov   bx,[_OutReadPtr]
        mov   cx,[OutCount]       ;set a maximum of 16 bytes going to the FIFO
        sti                       ;turn ints on (we could be here awhile)

TXFIFO2:mov   al,es:[si+bx]       ;al = byte to send out
        inc   bx
        out   dx,al               ;send byte to comm port
        and   bx,OUTBUFWRAP       ;wrap outptr to 0 if necessary
        dec   [__OutBytes]        ;set Zero Flag if OutBytes goes to 0
        loopnz TXFIFO2            ;loop back up if more data to send
        cli                       ;turn interrupts back off
        mov   [_OutReadPtr],bx
        pop   cx                  ;restore CX register
        pop   si                  ;restore SI register
        pop   es                  ;restore ES register
;       jmps  txdone  FALL THRU!!!

;----------------------------------------------------------------------

txdone: jz    txoff               ;if OutBytes == 0 then shut transmitter off

        cmp   [CheckCTS],0        ;are we ignoring CTS checking?
        jz    Loop01              ;if so then skip the CTS check

        mov   dx,[MSR_Port]
        in    al,dx
        and   al,CTSMASK          ;Check CTS signal - if CTS signal is up
        mov   [__CTSokay],al      ;then store a non-Zero value in CTSokay
        jz    txoff               ;if CTS is off, then turn XMIT off now

        jmp   word ptr cs:jmptble[2] ;otherwise see if we can send more data
                                     ;by jumping to the TX routine where we
                                     ;will check the THRE bit and, if set,
                                     ;transmit more data, if not, we'll jump
                                     ;out to the Loop01

txoff:  mov   dx,[IER_Port]       ;No more data to send
        mov   al,RCVEINT          ;turn off transmit interrupt
        out   dx,al
        jmp   Loop01              ;go back and check for more interrupts

;----------------------------------------------------------------------

        ALIGN16
TX8250: mov   dx,[LSR_Port]
        in    al,dx               ;Check to see if the Transmitter Holding
        test  al,THREMASK         ;Register Empty (THRE) indicator is REALLY on
        jnz   TX8250A
        jmp   Loop01              ;If not, jump out now

TX8250A:push  es                  ;save ES register
        push  si                  ;save SI register
        les   si,[_OutBuffer]
        mov   bx,[_OutReadPtr]

        mov   dx,[LSR_Port]       ;is the transmitter REALLY empty?
liar:   in    al,dx
        and   al,00100000b
        jz    liar                ;no, it lied go try again

        mov   dx,[THR_Port]
        mov   al,es:[si+bx]       ;al = byte to send out
        out   dx,al
        inc   bx
        and   bx,OUTBUFWRAP       ;wrap outptr to 0 if necessary
        mov   [_OutReadPtr],bx
        dec   [__OutBytes]        ;set Zero Flag if OutBytes goes to 0
        pop   si                  ;restore SI register
        pop   es                  ;restore ES register
        jmps  txdone

;----------------------------------------------------------------------

        ALIGN16
L_RX:   push  es                  ;save ES register
        push  si                  ;save SI register
        push  cx                  ;save CX register
        mov   cx,[__InBytes]
        les   si,[_InBuffer]      ;point to buffer
        mov   bx,[_InWritePtr]    ;load _InWritePtr

L_RX1:  mov   dx,[RBR_Port]       ;load port in dx
        in    al,dx               ;get the byte
        mov   es:[si+bx],al       ;store char in buffer at _InWritePtr offset
        inc   bx                  ;inc _InWritePtr
        and   bx,INBUFWRAP        ;wrap pointer if necessary
        inc   cx                  ;inc input queue bytes

        cmp   cx,[HighWater]      ;have we reached the High Water limit?
        jge   L_RX3               ;yes, go turn RTS signal off
                                  ;else fall through to get out of here

L_RX2:  mov   [_InWritePtr],bx
        mov   [__InBytes],cx
        pop   cx                  ;restore CX register
        pop   si                  ;restore SI register
        pop   es                  ;restore ES register
        jmp   Loop01


L_RX3:; cmp   [CheckCTS],0        ;this check is no longer necessary!  See
      ; je    L_RX2               ;AsyncInit for what we did to HighWater

        mov   dx,[MCR_Port]       ;turn off the RTS signal
        in    al,dx               ;
        and   al,RTSOFF           ;
        jmps  $+2                 ;
        out   dx,al               ;
        mov   [_RTSoff],1         ;
        jmps  L_RX2               ;finish up and get out of here

;----------------------------------------------------------------------

        ALIGN16
RXFIFO: push  es                  ;save ES register
        push  si                  ;save SI register
        push  cx                  ;save CX register
        mov   cx,[__InBytes]
        les   si,[_InBuffer]      ;point to buffer
        mov   bx,[_InWritePtr]    ;load _InWritePtr
        sti                       ;turn ints on (we could be here awhile)

RXFIFO2:mov   dx,[RBR_Port]       ;load port in dx
        in    al,dx               ;get the byte
        mov   es:[si+bx],al       ;store char in buffer at _InWritePtr offset
        inc   bx                  ;inc _InWritePtr
        inc   cx                  ;inc input queue bytes
        and   bx,INBUFWRAP        ;wrap pointer if necessary

        cmp   cx,[HighWater]      ;have we reached the High Water limit?
        jge   RXFIFO4             ;yes, go turn RTS signal off
                                  ;else fall through to get out of here

RXFIFO3:mov   dx,[LSR_Port]       ;check for more data
        in    al,dx               ;if bit 0 of the Line Status Register
        test  al,1                ;is set then there is a byte sitting
        jnz   RXFIFO2             ;there so loop back to L_RX1 and get it
        cli                       ;turn interrupts back off

        mov   [_InWritePtr],bx
        mov   [__InBytes],cx
        pop   cx                  ;restore CX register
        pop   si                  ;restore SI register
        pop   es                  ;restore ES register
        jmp   Loop01

RXFIFO4:;cmp   [CheckCTS],0       ;this check is no longer necessary!  See
        ;je    RXFIFO3            ;AsyncInit for what we did to HighWater

        mov   dx,[MCR_Port]       ;turn off the RTS signal
        in    al,dx               ;
        and   al,RTSOFF           ;
        out   dx,al               ;
        mov   [_RTSoff],1         ;
        jmps  RXFIFO3

;----------------------------------------------------------------------

        ALIGN16
L_LS:   mov   dx,[LSR_Port]
        in    al,dx
        test  al,00010000b    ;Bit 4 = Break Interrupt
        jnz   L_BI
        test  al,00000010b    ;Bit 1 = Overrun Error
        jnz   L_OE
        test  al,00000100b    ;Bit 2 = Parity  Error
        jnz   L_PE
        test  al,00001000b    ;Bit 3 = Framing Error
        jnz   L_FE
        jmp   Loop01

L_OE:   inc   [__OverrunErrors]        ;bit 1 = Overrun Error
        jmp   Loop01
L_PE:   inc   [__ParityErrors]         ;bit 2 = Overrun Error
        jmp   Loop01
L_FE:   inc   [__FramingErrors]        ;bit 3 = Framing Error
        jmp   Loop01
L_BI:   mov   dx,[IER_Port]            ;bit 4 = Break Interrupt
        mov   al,RCVEINT               ;turn off transmitter interrupt NOW!
        out   dx,al                    ;to disable any further sending of data
        xor   ax,ax
        mov   [_OutReadPtr],ax         ;clear the out buffer
        mov   [_OutWritePtr],ax        ;
        mov   [__OutBytes],ax          ;
        mov   [_InReadPtr],ax          ;point the in buffer to the start
        inc   ax                       ;
        mov   [_InWritePtr],ax         ;indicate only 1 byte in buffer
        mov   [__InBytes],ax           ;
        mov   ax,es                    ;save the ES register in AX
        les   bx,[_InBuffer]           ;point to buffer
        mov   byte ptr es:[bx],11      ;and put a CTRL-X in the input buffer
        mov   es,ax                    ;restore the ES register from AX
        jmp   Loop01

;----------------------------------------------------------------------

        ALIGN16
L_MS:   mov   dx,[MSR_Port]            ;reset the modem status register
        in    al,dx
        mov   ah,al
        and   al,CDMASK                ;Check CD signal - if CD signal is up
        mov   _CDup,al                 ;then store a non-Zero value in CDup
        and   _CDokay,al               ;And the "CDokay" value with AL so that
                                       ;if it is ever lost it will go to zero
                                       ;and stay zero from then on

        cmp   [CheckCTS],0             ;are we ignoring CTS checking?
        je    L_MS2                    ;if so then skip the CTS check

        mov   al,ah
        and   al,CTSMASK               ;Check CTS signal - if CTS signal is up
        mov   [__CTSokay],al           ;then store a non-Zero value in CTSokay
        jnz   L_MS2                    ;CTS is up
        mov   dx,[IER_Port]            ;CTS was dropped, so quit sending!
        mov   al,RCVEINT
        out   dx,al
        jmp   Loop01                   ;then get out of here

L_MS2:  Cmp   [__OutBytes],0           ;is there anything in the output buffer?
        Je    L_MS3                    ;  no, go check for a RING indicator
        Mov   dx,[IER_Port]            ;CTS is up, there are bytes in the
        Mov   al,XMITINT               ;buffer, so let's make sure that the
        Out   dx,al                    ;we're sending the data out the port!

L_MS3:
        mov   al,ah
        and   al,RINGMASK              ;Check RING indicator
        mov   [__RingDetect],al
        jmp   Loop01
Asynch_Interrupt   endp


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  TurnOnFIFO                                           ;
;                                                                      ;
;     Purpose:    Turns on the FIFO buffer of the 16550 and 16550A chip;
;                                                                      ;
;----------------------------------------------------------------------;
          Public ASYNC_TURNONFIFO
          ALIGN2
ASYNC_TurnOnFIFO Proc TriggerLevel:byte
          Xor   Ax,Ax
          Mov   [_B16550],Al
          Mov   [_B16550A],Al
          Mov   [_B16650],Al
          Cli                     ;disable interrupts

          Mov   Dx,[FCR_Port]     ;Port[FCR_Port] = 0
          Mov   [FCR_value],Al
          Out   Dx,Al
          Cmp   [_NO16550],1      ;If set to 1 then skip the following tests
          Jne   TestFifo
          Jmp   No16550           ;force it to NOT be accessed as a 16550

TestFifo: Mov   Al,[Triggerlevel] ;00 = 1, 01 = 4, 10 = 8, 11 =14
          Or    Al,00000001b
          Mov   [FCR_value],Al    ;Record the FCR value
          Or    Al,00000110b      ;xx000xxx
          Out   Dx,Al             ;||   ||+- Enable XMIT & RCVR int's for FIFO
          Jmps  $+2               ;||   |+-- Clear all bytes in RCVR FIFO
                                  ;||   +--- Clear all bytes in XMIT FIFO
                                  ;++------- RCVR Trigger level

          Cmp   [_FORCE16550A],1
          Jnz   No_Force
          Mov   [_B16550],1
          Jmp   short Use_A

No_Force: In    Al,Dx             ;Read the IIR register and check bits 6 and 7
          And   Al,11000000b      ;Mask off the top two bits
          Jnz   Use16550          ;NOTE: if AL=192 then it's a 16550A, if AL=
          Jmp   No16550           ;128 then it's a 16550, else it's not a 16550


Use16550: Mov   [_B16550],1       ;We'll use B16550 to indicate BOTH of them
          And   Al,01000000b      ;is bit #6 set?
          Jz    No_A              ;  no, it's not a 16550A
Use_A:    Mov   [_B16550A],1      ;And if it's a 16550A then record that too
No_A:     Mov   word ptr cs:jmptble[1 * 2],offset TXFIFO

          Mov   [OutCount],16     ;default to 16 bytes per FIFO transmition

                                  ;test for a 16650
          GetPort [MCR_Port]      ;get MCR_Port value in AL
          Or    Al,10000000b      ;set the Clock to 4x
          PutPort [MCR_Port]
          GetPort [MCR_Port]
          And   Al,10000000b      ;see if clock bit is still set to 4x
          Jnz   Do16650           ;  find out if we should test for 16650
          Jmp   No16650           ;  if not, it can't be a 16650, jump out now

Do16650:
          Cmp   [_Allow16650],1   ;If no set to 1 then skip the following tests
          Je    TestSleep         ;go do sleep test
          Jmps  No16650           ;force it to NOT be accessed as a 16650

TestSleep:                        ;test if sleep mode bit is setable
          EnableEFR               ;Enable the EFR Register
          EnableIER               ;Enable IER Register bits 4-7
          GetPort [IER_Port]
          Or    Al,00010000b      ;try setting the sleep mode bit on
          PutPort [IER_Port]
          GetPort [IER_Port]
          Mov   Bl,Al             ;remember whether or not sleep mode worked
          And   Al,11101111b      ;set the sleep mode bit back off
          PutPort [IER_Port]
          DisableIER
          DisableEFR

          And   Bl,00010000b      ;test to see if sleep mode bit was set
          Jz    No16650           ;  no, get out now


                                  ;by now, we know we have a 16650
          EnableEFR               ;Enable the EFR Register
          GetPort [EFR_Port]
          Or    Al,11010000b      ;set 16650-controlled CTS and RTS
          PutPort [EFR_Port]
          DisableEFR

          Mov   [_B16650],1       ;record that it is a 16650
          Mov   [OutCount],32     ;set to 32 bytes per FIFO transmition

No16650:  Sti
          Ret

No16550:  Xor   Ax,Ax             ;Port[FCR_Port] = 0 (to turn FIFO off)
          Mov   [FCR_value],Al
          Out   Dx,Al
          Jmps  $+2
          Mov   Dx,[SCR_Port]
          Mov   Al,'A'
          Out   Dx,Al
          Jmps  $+2
          Mov   [_B8250],0
          In    Al,Dx
          Cmp   Al,'A'
          Je    Yes16450
          Mov   [_B8250],1
          Mov   word ptr cs:jmptble[1 * 2],offset TX8250
Yes16450: Sti
          Ret
ASYNC_TurnOnFIFO EndP



;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  TurnOffFIFO                                          ;
;                                                                      ;
;     Purpose:    Turns off the FIFO buffer                            ;
;                                                                      ;
;----------------------------------------------------------------------;
TurnOffFIFO Proc Near
          Cmp   [_B16550],1
          Jne   offexit
          Mov   Dx,[FCR_Port]
          Xor   Ax,Ax
          Out   Dx,Al
offexit:  Ret
TurnOffFIFO EndP



;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  TurnOnRTS                                            ;
;                                                                      ;
;     Purpose:    Turns RTS on                                         ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_TURNONRTS
ASYNC_TurnOnRTS Proc
          Cmp   [CheckCTS],0
          Je    onexit
          Push  Ax
          Push  Dx
          Mov   Dx,[MCR_Port]
          In    Al,Dx
          Jmps  $+2
          Or    Al,RTSON
          Out   Dx,Al
          Mov   [_RTSoff],0
          Pop   Dx
          Pop   Ax
onexit:   Ret
ASYNC_TurnOnRTS EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  TurnOffRTS                                           ;
;                                                                      ;
;     Purpose:    Turns RTS off                                        ;
;                                                                      ;
;----------------------------------------------------------------------;
           public ASYNC_TURNOFFRTS
ASYNC_TurnOffRTS Proc
          Cmp   [CheckCTS],0
          Je    rtsret
          Push  Ax
          Push  Dx
          Mov   Dx,[MCR_Port]
          In    Al,Dx
          Jmps  $+2
          And   Al,RTSOFF
          Out   Dx,Al
          Mov   [_RTSoff],1
          Pop   Dx
          Pop   Ax
rtsret:   Ret
ASYNC_TurnOffRTS EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  TurnOnDtr                                            ;
;                                                                      ;
;     Purpose:    Set the state of DTR to ON                           ;
;                                                                      ;
;     Called from C                                                    ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_TURNONDTR
ASYNC_TurnOnDtr Proc
          Mov   Dx,[MCR_Port]
          In    Al,Dx
          Jmps  $+2
          Or    Al,00001b
          Out   Dx,Al
          Ret
ASYNC_TurnOnDtr EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  TurnOffDtr                                           ;
;                                                                      ;
;     Purpose:    Set the state of DTR to OFF                          ;
;                                                                      ;
;     Called from C                                                    ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_TURNOFFDTR
ASYNC_TurnOffDtr Proc
          Mov   Dx,[MCR_Port]
          In    Al,Dx
          Jmps  $+2
          And   Al,11110b
          Out   Dx,Al
          Ret
ASYNC_TurnOffDtr EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  ClearInBuf                                           ;
;                                                                      ;
;     Purpose:    Resets the pointers for the INCOMING buffer          ;
;                                                                      ;
;     Called from C                                                    ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_CLEARINBUF
          ALIGN2
ASYNC_ClearInBuf Proc
          Cli
          Xor   Ax,Ax
          Mov   [_InReadPtr],Ax
          Mov   [_InWritePtr],Ax
          Mov   [__InBytes],Ax

          Cmp   [_B16550],0
          Je    CIBexit
          Mov   Dx,[FCR_Port]
          Mov   Al,[FCR_value]
          Or    Al,00000010b      ;00000xx0b
          Out   Dx,Al             ;     |+-- Clear all bytes in RCVR FIFO
                                  ;     +--- Clear all bytes in XMIT FIFO
CIBexit:  Sti
          Ret
ASYNC_ClearInBuf EndP



;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  ClearOutBuf                                          ;
;                                                                      ;
;     Purpose:    Resets the pointers for the OUTGOING buffer          ;
;                                                                      ;
;     Called from C                                                    ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_CLEAROUTBUF
          ALIGN2
ASYNC_ClearOutBuf Proc
          Cli
          Mov   Dx,[IER_Port]
          Mov   Al,RCVEINT             ;turn the XMIT interrupt off
          Out   Dx,Al

          Xor   Ax,Ax
          Mov   [_OutReadPtr],Ax
          Mov   [_OutWritePtr],Ax
          Mov   [__OutBytes],Ax

          Cmp   [_B16550],0
          Je    COBexit
          Mov   Dx,[FCR_Port]
          Mov   Al,[FCR_value]
          Or    Al,00000100b      ;00000xx0
          Out   Dx,Al             ;     |+-- Clear all bytes in RCVR FIFO
                                  ;     +--- Clear all bytes in XMIT FIFO
COBexit:  Sti
          Ret
ASYNC_ClearOutBuf EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  Online                                               ;
;                                                                      ;
;     Purpose:    Returns a value of 0 if OFFLINE, non-zero for ONLINE ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_ONLINE
          ALIGN16
ASYNC_Online    Proc
          Mov   Dx,[MSR_Port]     ;reset the modem status register
          In    Al,Dx
          Mov   Ah,Al
          And   Al,CDMASK         ;Check CD signal - if CD signal is up
          Mov   _CDup,Al          ;then store a non-Zero value in CDup

          And   _CDokay,Al        ;And the "CDokay" value with AL so that
                                  ;if it is ever lost it will go to zero
                                  ;and stay zero from then on

          And   Ah,CTSMASK        ;Check CTS signal - if CTS signal is up
          Mov   [__CTSokay],Ah    ;then store a non-Zero value in CTSokay
          Xor   Ah,Ah
          Ret                     ;Return AX = online state
ASYNC_Online    EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  cdstillup                                            ;
;                                                                      ;
;     Purpose:    Returns a value of 0 if OFFLINE, non-zero for ONLINE ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_CDSTILLUP
          ALIGN16
ASYNC_cdstillup Proc
          Cmp   [_VerifyCDLoss],1 ;are we verifying CD loss?
          Je    cdcheck           ;  no we are, go do a regular CD check
          Cmp   [_CDokay],0       ;is cd okay?  (cd has NEVER been dropped?)
          Jne   cdisup            ;  yes, go indicate that CD is up
          Mov   Al,RCVEINT        ;Otherwise shut everything down now and
          Mov   Dx,[IER_Port]     ;get out of here
          Out   Dx,Al             ;
          Jmps  cdout             ;

cdcheck:  Cmp   _CDup,0           ;is CDup FALSE?
          Je    cddown            ;  yes, go wait for 1/10th sec to make sure
cdisup:   Mov   Ax,1              ;return TRUE (indicating caller is online)
          Ret

cddown:   Mov   Cx,15             ;Run the loop 15 times (total 1.5 seconds)
          Mov   Al,RCVEINT        ;CD is FALSE - turn off transmit interrupt
          Mov   Dx,[IER_Port]
          Out   Dx,Al

cdtop:    Mov   Ax,10
          Push  Ax
IF LPROG
          Call  far ptr MYDELAY   ;Wait for 10/100ths of a second
ELSE
          Call  near ptr MYDELAY  ;Wait for 10/100ths of a second
ENDIF
          Mov   Dx,[MSR_Port]     ;Check the CD signal now
          In    Al,Dx
          Mov   Ah,Al

          Cmp   [CheckCTS],0      ;are we ignoring CTS checking?
          Je    cdchkup           ;if so then skip the CTS check
          And   Al,CTSMASK        ;Check CTS signal - if CTS signal is up
          Mov   [__CTSokay],Al    ;then store a non-Zero value in CTSokay

cdchkup:  And   Ah,CDMASK         ;Check CD signal - if CD signal is up
          Mov   _CDup,Ah          ;then store a non-Zero value in CDup
          Jnz   cdisup            ;Is CD still down? (AL != 0 means CD is up)   no, it's back up - go report it now
          Loop  cdtop             ;Loop and try again

cdout:    Call  ASYNC_ClearOutBuf ;If we made it here CD must be down for good
          Call  ASYNC_ClearInBuf  ;so clear the buffers
          Xor   Ax,Ax             ;and return FALSE
          Ret
ASYNC_cdstillup EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  Async_Init                                           ;
;                                                                      ;
;     Purpose:    Initializes all async variables                      ;
;                                                                      ;
;     NOTES:      The in and out buffer sizes MUST be set to a power   ;
;                 of 2 in order to utilize the ANDing method of        ;
;                 wrapping the buffer around to the beginning.         ;
;                                                                      ;
;                 In other words, if a size of 2048 is chosen then     ;
;                 the buffer size is stored as 0000011111111111b       ;
;                 which is 2048 minus 1.  Then the buffer pointer is   ;
;                 ANDed with this value which will cause it to return  ;
;                 to zero at the end of the buffer.  The following is  ;
;                 an example:                                          ;
;                                                                      ;
;                 Instead of this:    if BufPtr = BufSize              ;
;                                       BufPtr = 0                     ;
;                                                                      ;
;                 We get this:        BufPtr = BufPtr AND (BufSize-1)  ;
;                                     (note this avoids a JMP)         ;
;                                                                      ;
;     Called from C                                                    ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_INIT
Async_Init Proc  IrqNum:byte, Address:word, PtrToInBuf:dword, PtrToOutBuf:dword, InSize:word, OutSize:word, UseCTS:byte, Share:byte
          Les   Ax,PtrToInBuf
          Mov   word ptr [_InBuffer+2],Es
          Mov   word ptr [_InBuffer],Ax

          Les   Ax,PtrToOutBuf
          Mov   word ptr [_OutBuffer+2],Es
          Mov   word ptr [_OutBuffer],Ax

          Mov   Ax,InSize
          Dec   Ax                          ;Subtract 1 from Size for ANDing
          Mov   [HighWater],Ax
          Mov   [LowWater],Ax
          Sub   [HighWater],32
          Sub   [LowWater],128

          Mov   Al,Share
          Mov   [ShareIRQ],Al

          Mov   Al,UseCTS
          Mov   [CheckCTS],Al          ;will we be using CTS flow control?
          Or    Al,Al
          Jnz   AI1                    ;if yes, skip
                                       ;  Otherwise, we're going to disable
                                       ;  RTS dropping in the RECEIVE routines
                                       ;  by making sure that the receive
                                       ;  buffer NEVER reaches the high water
                                       ;  limit.  This is basically what would
                                       ;  have happened anyway!

          Mov   [HighWater],7FFFh      ;set high water limit outside of buffer

AI1:      Mov   [__CTSokay],1
          Xor   Ax,Ax
          Mov   [ComPort_Open_Flag],Al
          Mov   [_InReadPtr],Ax
          Mov   [_InWritePtr],Ax
          Mov   [__InBytes],Ax
          Mov   [_OutReadPtr],Ax
          Mov   [_OutWritePtr],Ax
          Mov   [__OutBytes],Ax
          Mov   Cs:saveds,Ds

          Mov   Al,IrqNum
          Mov   [Async_Irq],Al
          Mov   Ax,Address

          Mov   [THR_Port]  ,UART_THR  ;these are used thru-out the asynch routines
          Mov   [RBR_Port]  ,UART_RBR  ;done here (and below) so it doesn't waste
          Mov   [LSR_Port]  ,UART_LSR  ;time when actually processing
          Mov   [IER_Port]  ,UART_IER
          Mov   [IIR_Port]  ,UART_IIR
          Mov   [FCR_Port]  ,UART_FCR
          Mov   [EFR_Port]  ,UART_EFR
          Mov   [MCR_Port]  ,UART_MCR
          Mov   [LCR_Port]  ,UART_LCR
          Mov   [MSR_Port]  ,UART_MSR
          Mov   [DLL_Port]  ,UART_DLL
          Mov   [DLM_Port]  ,UART_DLM
          Mov   [SCR_Port]  ,UART_SCR

          Add   [THR_Port]  ,Ax
          Add   [RBR_Port]  ,Ax
          Add   [LSR_Port]  ,Ax
          Add   [IER_Port]  ,Ax
          Add   [IIR_Port]  ,Ax
          Add   [FCR_Port]  ,Ax
          Add   [EFR_Port]  ,Ax
          Add   [MCR_Port]  ,Ax
          Add   [LCR_Port]  ,Ax
          Add   [MSR_Port]  ,Ax
          Add   [DLL_Port]  ,Ax
          Add   [DLM_Port]  ,Ax
          Add   [SCR_Port]  ,Ax
          Ret
Async_Init EndP

;----------------------------------------------------------------------;
;                                                                      ;
;     Function:   SetPort                                              ;
;                                                                      ;
;     Purpose:    sets the baud rate and databits/parity/stop bits     ;
;                                                                      ;
;     Calling Sequence:                                                ;
;                                                                      ;
;        Called from C                                                 ;
;                                                                      ;
;        SetPort(BaudRate,DataBits)                                    ;
;                                                                      ;
;        The following numbers are determined by a calculation         ;
;        of 115200 divided by the Baud Rate before calling the routine ;
;                                                                      ;
;           BaudRate ---    300 = 180h                                 ;
;                          1200 =  60h                                 ;
;                          2400 =  30h                                 ;
;                          4800 =  18h                                 ;
;                          9600 =   Ch                                 ;
;                         19200 =   6h                                 ;
;                         38400 =   3h                                 ;
;                         57600 =   2h                                 ;
;                        115200 =   1h                                 ;
;                                                                      ;
;           DataBits --- 7 = 2                                         ;
;                        8 = 3                                         ;
;                                                                      ;
;           StopBits --- 1 or 2                                        ;
;                                                                      ;
;           Parity   --- 0 = None                                      ;
;                        1 = Odd                                       ;
;                        2 = Even                                      ;
;                                                                      ;
;           Flag returned 0  if port initialized successfully          ;
;           Flag returned -1 if any errors.                            ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_SETPORT
ASYNC_SetPort   Proc  BaudDivisor:word, DataBits:word
          Mov   Bx,DataBits
          And   Bl,00000001b           ;if DataBits=8 then Bx = 0 else 1
          Mov   Al,Bits_Array[Bx]      ;DataBits == 8 ? 3 : 2
          Or    Al,Parity_Array[Bx]    ;DataBits == 8 ? 0 : 18h
          ;Or   Al,0                   ;0 = 1 stop bit.  We won't use 2 here!

          Mov   Dx,[LCR_Port]
          Mov   Bl,Al                  ;Store Al in Bl to restore it later...
          Or    Al,80h                 ;Use 80h to gain access to DLAB
          Out   Dx,Al                  ;Port[LCR_Port] = 80h or Parity or DataBits or StopBits
          Jmps  $+2

          Mov   Cx,BaudDivisor
          Mov   Dx,[DLM_Port]
          Mov   Al,Ch
          Out   Dx,Al                  ;Port[DLM_Port] = hi(BaudRate)
          Jmps  $+2

          Mov   Dx,[DLL_Port]
          Mov   Al,Cl
          Out   Dx,Al                  ;Port[DLL_Port] = lo(BaudRate)
          Jmps  $+2

          Mov   Al,Bl                  ;Get saved LCRreg to reset the DLAB
          Mov   Dx,[LCR_Port]
          Out   Dx,Al
          Ret
ASYNC_SetPort   EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Function:   OpenCom                                              ;
;                                                                      ;
;     Purpose:    Opens communications port by testing to see if it    ;
;                 is there, setting the port speed & other parameters  ;
;                 and finally linking up the interrupt handler         ;
;                                                                      ;
;     Calling Sequence:                                                ;
;                                                                      ;
;        Called from C                                                 ;
;                                                                      ;
;        AsyncOpen(ComPort,BaudDivisor,DataBits)                       ;
;                                                                      ;
;     returns   0 if port initialized successfully                     ;
;     returns  -1 if any errors occured                                ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_OPENCOM
ASYNC_OpenCom   Proc  BaudDivisor:word, DataBits:word
          Cmp   [ComPort_Open_Flag],1  ;Make sure we don't EVER do this twice
          Jne   continue               ;in a row, ever!
          Jmp   Ao2

continue: Mov   Dx,[IIR_Port]          ;Port[IIR_Port] = 0
          Xor   Ax,Ax
          Out   Dx,Al
          Jmps  $+2

          In    Al,Dx                  ;Al = Port[IIR_Port]
          And   Al,30h                 ;look for bits 4 and 5 of LCR being OFF
          Jz    LoopTest               ;If AL & 30 = 0 then port is okay so far
          Mov   Ax,-1
          Ret

         ;;see Dr. Dobbs Journal, July 1991 page 123 for further explanation
         ;;as to how the following loopback test works
         ;
LoopTest:;Mov   Dx,[MCR_Port]          ;Turn on LOOPBACK mode in the UART
         ;In    Al,Dx
         ;Mov   Bl,Al                  ;Bl = Port[MCR_Port]
         ;Or    Al,LOOPMASK
         ;Out   Dx,Al                  ;Port[MCR_Port] |= LOOPMASK
         ;
         ;Mov   Dx,[MSR_Port]          ;Get the oringal MSR value
         ;In    Al,Dx
         ;Mov   Bh,Al                  ;Bh = Port[MSR_Port]
         ;
         ;Mov   Dx,[MCR_Port]          ;Put a hex 0Ah into MCR without losing
         ;Mov   Al,0Ah + LOOPMASK      ;the LOOPBACK setting
         ;Out   Dx,Al                  ;Port[MCR_Port] = 0Ah + LOOPMASK
         ;
         ;Mov   Dx,[MSR_Port]          ;Read the value the UART put into MSR
         ;In    Al,Dx                  ;if correct, the HIGH 4 bits will be
         ;And   Al,0F0h                ;equal to the low 4 bits placed into
         ;Mov   Ah,Al                  ;MCR with the bottom two bits reversed
         ;
         ;Mov   Al,Bh                  ;Set the MSR port back to original
         ;Out   Dx,Al
         ;Mov   Dx,[MCR_Port]          ;Turn off LOOPBACK mode
         ;Mov   Al,Bl
         ;And   Al,not LOOPMASK
         ;Out   Dx,Al
         ;
         ;Cmp   Ah,90h                 ;0Ah in should result in 90h out
         ;Je    PortOkay               ;if so, the port passed the test
         ;Mov   Ax,-2
         ;Ret

PortOkay: Mov   Ax,BaudDivisor
          Push  Ax
          Mov   Ax,DataBits
          Push  Ax
          Call  ASYNC_SetPort

          Mov   Bl,8
          Mov   Al,[Async_Irq]         ;save the address of the old handler
          Cmp   Al,8
          Jb    oc1
          Mov   Bl,104
oc1:      Add   Al,Bl
          Call  DOS_Get_Intrpt
          Mov   word ptr [ComSaveVec+2],Es
          Mov   word ptr [ComSaveVec],Bx

          Mov   Ax,seg Asynch_Interrupt
          Mov   Es,Ax
          Mov   Bl,8
          Mov   Al,[Async_Irq]
          Cmp   Al,8
          Jb    oc2
          Mov   Bl,104
oc2:      Add   Al,Bl
          Mov   Dx,offset Asynch_Interrupt ;ES:DX points to Asynch_Interrupt
          Call  DOS_Set_Intrpt
          Mov   [ComPort_Open_Flag],1

          Cli
          Xor   Ax,Ax             ;0="1-byte FIFO trigger"
          Push  Ax
          Call  ASYNC_TurnOnFIFO

          Mov   Dx,[LSR_Port]
          In    Al,Dx             ;Read Line Status Register to reset any errors
          Jmps  $+2

          Mov   Dx,[MSR_Port]
          In    Al,Dx             ;Read Modem Status Register
          Jmps  $+2

          Mov   Dx,[RBR_Port]
          In    Al,Dx             ;Read Receiver Buffer in case a char is in it

          Cli
          Mov   Dx,PICMSK1        ;default to PICMSK1 for INT's 0-7
          Mov   Cl,[Async_Irq]
          Cmp   Cl,8
          Jb    Ao1               ;if INT < 8 skip over INT 8-15 adjustment

          Sub   Cl,8              ;  else subtract 8
          Mov   Dx,PICMSK2        ;  and set for PICMSK2 instead of PICMSK1

Ao1:      Mov   Bl,1
          Shl   Bl,Cl
          Xor   Bl,0FFh           ;Bl = (1 shl Async_Irq) xor 0FFh
          In    Al,Dx             ;Al = Port[PICMSK1]  (or PICMSK2)
          And   Al,Bl
          Out   Dx,Al             ;Port[PICMSK1] = Al and Bl

          Mov   Dx,[IER_Port]     ;Enable the comm port interrupts
          Mov   Al,RCVEINT
          Out   Dx,Al
          Jmps  $+2
          Mov   Dx,[MCR_Port]
          In    Al,Dx
          Jmps  $+2
          And   Al,11101111b      ;Turn off the LOOPBACK bit (found a uart that booted up in LOOPBACK mode! 9/3/93)
          Or    Al,00001000b      ;Turn on the OUT2 signal...  why?  heck, I
          Out   Dx,Al             ;dunno but it doesn't work without it!
          Sti

Ao2:      Xor   Ax,Ax
          Ret
ASYNC_OpenCom   Endp


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  CloseCom                                             ;
;                                                                      ;
;     Purpose:    Resets interrupt system when UART interrupts         ;
;                 are no longer needed.                                ;
;                                                                      ;
;     Called from C                                                    ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_CLOSECOM
ASYNC_CloseCom  Proc
          Cmp   [ComPort_Open_Flag],0
          Je    NotOpen

          Cli
          Mov   Dx,PICMSK1        ;Use PICMSK1 for IRQ's less than 8
          Mov   Cl,[Async_IRQ]
          Cmp   Cl,8
          Jb    CC1

          Mov   Dx,PICMSK2        ;Use PICMSK2 for IRQ's greater than 7
          Sub   Cl,8              ;set up for Bl = (1 shl (Async_IRQ-8))

CC1:      Mov   Bl,1
          Shl   Bl,Cl             ;Bl = 1 shl Async_IRQ
          In    Al,Dx             ;Al = Port[PICMSK1] (or PICMSK2)
          Or    Al,Bl
          Out   Dx,Al             ;Port[PICMSK1] = Al or Bl

          call  TurnOffFIFO

          Mov   Dx,[IER_Port]     ;Stop the comm port
          Mov   Al,00000000b      ;00000000b
          Out   Dx,Al             ;    |||+- Received Data Available interrupt
          Jmps  $+2               ;    ||+-- Transmit Holding Register Empty interrupt
                                  ;    |+--- Receiver Line Status interrupt
                                  ;    +---- Modem Status interrupt
          Mov   Dx,[MCR_Port]
          In    Al,Dx
          Jmps  $+2
          And   Al,10111b         ;Turn off the OUT2 signal...  why?  heck, I
          Out   Dx,Al             ;dunno but I had to turn it on to begin with!

          Mov   Bl,8
          Mov   Al,[Async_Irq]    ;Restore old comm port interrupt handler
          Cmp   Al,8
          Jb    CC2
          Mov   Bl,104
CC2:      Add   Al,Bl
          Les   Dx,[ComSaveVec]
          Call  DOS_Set_Intrpt
          Sti

          Mov   [ComPort_Open_Flag],0
NotOpen:  Ret
ASYNC_CloseCom  EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  CommInkey                                            ;
;                                                                      ;
;     Purpose:    Gets a single character out of the INCOMING buffer   ;
;                                                                      ;
;     Called from C                                                    ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_COMMINKEY
          ALIGN16
ASYNC_CommInkey Proc
          Cli
          Cmp   word ptr [__InBytes],0     ; Is there anything in there?
          Sti
          Jz    C002

          Push  Si                         ; Save SI for C's use
          Mov   Bx,[_InReadPtr]            ; load _InReadPtr
          Les   Si,[_InBuffer]             ; ES:SI points to _InBuffer
          Xor   Ax,Ax
          Mov   Al,Es:[Si+Bx]              ; get char in AL
          Pop   Si                         ; Restore SI
          Inc   Bx                         ; inc _InReadPtr
          And   Bx,INBUFWRAP               ; wrap pointer to 0 if necessary
          Mov   [_InReadPtr],Bx            ; store the _InReadPtr

          Cli                              ; turn off interrupts
          Dec   [__InBytes]                ; decrement queue bytes
          Sti                              ; turn interrupts back on

          Cmp   [_RTSoff],1                ; Has RTS been shut down?
          Jne   C001                       ;   no, get out now
          Mov   Cx,[LowWater]              ;   yes...
          Cli
          Cmp   Cx,[__InBytes]             ;   are we ready yet?
          Sti
          Jl    C001                       ;     no, get out now
          Call  ASYNC_TurnOnRTS            ;     yes, turn RTS back on
C001:     Ret                              ; return character in AL

C002:     Mov   Ax,-1                      ; return -1 if no data present
          Ret
ASYNC_CommInkey EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  cgetbuf                                              ;
;                                                                      ;
;     Purpose:    Gets a buffer of data out of the INCOMING buffer     ;
;                                                                      ;
;     Return:     With RetBuf filled and AX equal to the # of bytes    ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_CGETBUF
ASYNC_cgetbuf   Proc  RetBuf:ptr, RetBufLen:word
          Cli
          Mov   Cx,[__InBytes]
          Sti
CG00:     Xor   Ax,Ax
          Jcxz  CG04                   ;if INBYTES = 0 then exit now

          Push  Si
          Push  Di
if LDATA
          Les   Di,RetBuf
else
          Mov   Ax,Ds
          Mov   Es,Ax
          Mov   Di,RetBuf              ;ES:DI points to the buffer we're going to stuff
endif
          Mov   Dx,RetBufLen           ;Dx (MAXLEN) = length of buffer

          Mov   Ax,INBUFSIZE           ;Calculate the number of bytes in the
          Mov   Bx,[_InReadPtr]        ;buffer BEFORE the buffer wraps around
          Sub   Ax,Bx                  ;(because MOVSB won't wrap)

          Cld
          Cmp   Cx,Dx
          Jle   CG01                   ;if BYTES <= MAXLEN then goto CG01
          Mov   Cx,Dx                  ;  else make BYTES=MAXLEN

CG01:     Cmp   Cx,Ax
          Jle   CG02                   ;if BYTES <= MAXBEFOREWRAP goto CG02
          Mov   Cx,Ax                  ;  else make BYTES=MAXBEFOREWRAP

CG02:     Mov   Ax,Cx                  ;Save the number of BYTES in Ax
          Push  Ds
          Lds   Si,[_InBuffer]
          Add   Si,Bx                  ;DS:SI points to the 1st byte to read
          Shr   Cx,1
          Jnc   CG03                   ;if BYTES was odd
          Movsb                        ;  then move one BYTE
CG03:     Rep   Movsw                  ;now move 'em a WORD at a time
          Pop   Ds
          Add   Bx,Ax                  ;Add BYTES to _InReadPtr
          And   Bx,INBUFWRAP           ;reset _InReadPtr to 0 if necessary

          Mov   [_InReadPtr],Bx        ;Now store our new _InReadPtr
          Cli
          Sub   [__InBytes],Ax         ;Reduce the number of __InBytes
          Sti
          Pop   Di
          Pop   Si

          Cmp   [_RTSoff],1            ;Has RTS been shut down?
          Jne   CG04                   ;  no, get out
          Mov   Cx,[LowWater]
          Cli
          Cmp   Cx,[__InBytes]         ;are we ready yet?
          Sti
          Jl    CG04                   ;  no, get out
          Call  ASYNC_TurnOnRTS        ;  yes, turn RTS back on

CG04:     Ret                          ;Return (in AX) the number of bytes read
ASYNC_cgetbuf   EndP


          public ASYNC_CGETSTR
ASYNC_cgetstr   Proc  RetStr:ptr, RetStrLen:word
          Xor   Ax,Ax
          Mov   Cx,[RetStrLen]
          Dec   Cx
          Jcxz  CGRET

if LDATA
          Les   Ax,RetStr
          Push  Es
else
          Mov   Ax,RetStr
endif
          Push  Ax
          Push  Cx
if LPROG
          Push  Cs
endif
          Call  near ptr ASYNC_cgetbuf

          LES_  Bx,RetStr
          Add   Bx,Ax                ;point to end of string
          Mov   word ptr ES_[Bx],0   ;store a null terminator

CGRET:    Ret                     ;Ax returns with # of bytes found
ASYNC_cgetstr   EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  CSendByte                                            ;
;                                                                      ;
;     Purpose:    Sends a single character out the comm port           ;
;                                                                      ;
;     Called from C                                                    ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_CSENDBYTE
          ALIGN16
ASYNC_CSendByte Proc  Char:word
          Uses  Di

          Les   Di,[_OutBuffer]
          Mov   Bx,[_OutWritePtr]

          Mov   Ax,Char
          Mov   Es:[Di+Bx],Al
          Inc   Bx
          And   Bx,OUTBUFWRAP          ;wrap _OutWritePtr to 0 if necessary
          Mov   [_OutWritePtr],Bx

          Cli
          Inc   [__OutBytes]
          TurnOnXmt
          Sti
          Ret
ASYNC_CSendByte EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  CSendStr                                             ;
;                                                                      ;
;     Purpose:    Sends a string of data out the comm port             ;
;                                                                      ;
;     Called from C                                                    ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_CSENDSTR
          ALIGN16
ASYNC_CSendStr  Proc  StrVar:ptr, StrVarLen:word
          Mov   Cx,StrVarLen      ;Get String Length
          Jcxz  ssend             ;exit if nothing to send
          Jmps  ssstart
ssend:    Ret

ssstart:
          Push  Si
          Push  Di

          Cld
          Les   Di,[_OutBuffer]
          Mov   Bx,[_OutWritePtr]

if LDATA
          Push  Ds
          Lds   Si,StrVar         ;DS:SI points to string to send
          Push  Bp                ;we're going to BORROW the BP register
          Mov   Bp,Di             ;to remember where the _OutBuffer is
else
          Mov   Si,StrVar         ;DS:SI points to string to send
endif

          Add   Di,Bx             ;ES:DI points to _OutBuffer[_OutWritePtr]
          Mov   Ax,OUTBUFSIZE
          Sub   Ax,Bx             ;Ax = OutBufSize - _OutWritePtr
          Mov   Dx,Cx             ;Save strlen in Dx
          Cmp   Ax,Cx
          Jle   sswrap            ;If Ax < Num bytes to send, goto sswrap

          Shr   Cx,1
          Jnc   ss01
          Movsb                   ;Move a single byte
Ss01:     Rep   Movsw             ;Now move the rest a word at a time
          Add   Bx,Dx             ;_OutWritePtr += len(Str)
          And   Bx,OUTBUFWRAP     ;wrap OutWritPtr
          Jmps  ssupdte           ;done - just need to update the pointers now

                                  ;otherwise our buffer will wrap so we do less
sswrap:   Sub   Cx,Ax             ;Cx = strlen - max room in buffer
          Xchg  Cx,Ax             ;Now Cx = maxroom in buffer, Ax = bytes left
          Shr   Cx,1              ;We need to fill up the rest of the buffer
          Jnc   ss02
          Movsb                   ;Move a single byte
Ss02:     Rep   Movsw             ;Now move the rest a word at a time
          Mov   Cx,Ax             ;Then get ready to start over from the first
          Mov   Bx,Ax             ;_OutWritePtr = len(Str) - OutBufSize - _OutWritePtr

if LDATA
          Mov   Di,Bp             ;reset to beginning of the buffer
else
          Mov   Di,word ptr [_OutBuffer];reset to the beginning of the buffer
endif

          Shr   Cx,1
          Jnc   ss03
          Movsb                   ;Move a single byte
Ss03:     Rep   Movsw             ;Now move the rest a word at a time

ssupdte:
if LDATA
          Pop   Bp
          Pop   Ds
endif
          Mov   [_OutWritePtr],Bx
          Cli
          Add   [__OutBytes],Dx
          TurnOnXmt
          Sti
ssend2:   Pop   Di
          Pop   Si
          Ret
ASYNC_CSendStr  EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  checkcomm                                            ;
;                                                                      ;
;     Purpose:    Checks the InBuffer to see if a CTRL-K, CTRL-X or    ;
;                 or CTRL-S has come in and if so it returns it        ;
;                                                                      ;
;     Called from C                                                    ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_CHECKCOMM
          ALIGN16
ASYNC_checkcomm Proc
          Xor   Ax,Ax
          Cli
          Mov   Cx,[__InBytes]
          Sti
          Jcxz  CC02                   ;if BYTES = 0 then exit now

          Push  Si
          Push  Ds
          Mov   Bx,[_InReadPtr]        ;ES:SI+BX will point to data in buffer
          Mov   Dx,INBUFWRAP           ;Store InBufSize in DX for fast access
          Lds   Si,[_InBuffer]         ;ES:SI points to the input buffer

CCloop:   Mov   Al,[Si+Bx]
          Cmp   Al,19                  ;Is it a CTRL-S?
          Je    CC01
          Cmp   Al,24                  ;Is it a CTRL-X?
          Je    CC01
          Cmp   Al,11                  ;Is it a CTRL-K?
          Je    CC01
          Inc   Bx
          And   Bx,Dx                  ;wrap pointer to 0 if necessary
          Loop  CCloop
          Xor   Ax,Ax                  ;not found, set it back to 0

CC01:     Pop   Ds
          Pop   Si
CC02:     Ret
ASYNC_checkcomm EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  commpause                                            ;
;                                                                      ;
;     Purpose:    Stops the output buffer without losing the data that ;
;                 was already in it.                                   ;
;                                                                      ;
;     Called from C                                                    ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_COMMPAUSE
          ALIGN2
ASYNC_commpause Proc
          Cli
          Mov   Dx,[IER_Port]
          Mov   Al,RCVEINT
          Out   Dx,Al
          Sti
          Ret
ASYNC_commpause EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  commstop                                             ;
;                                                                      ;
;     Purpose:    Stops the output buffer without losing the data that ;
;                 was already in it.                                   ;
;                                                                      ;
;     Called from C                                                    ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_COMMSTOP
          ALIGN2
ASYNC_commstop  Proc
          Cli
          Mov   Ax,[__OutBytes]
          Mov   [PausedOutBytes],Ax
          Mov   [__OutBytes],0
          Mov   Dx,[IER_Port]
          Mov   Al,RCVEINT
          Out   Dx,Al
          Sti
          Ret
ASYNC_commstop  EndP


;----------------------------------------------------------------------;
;                                                                      ;
;     Procedure:  commgo                                               ;
;                                                                      ;
;     Purpose:    Resumes output (after a commstop stopped it)         ;
;                                                                      ;
;     Called from C                                                    ;
;                                                                      ;
;----------------------------------------------------------------------;
          public ASYNC_COMMGO
          ALIGN2
ASYNC_commgo    Proc
          Cli
          Mov   Cx,[PausedOutBytes]
          Mov   [__OutBytes],Cx
          Jcxz  cgexit
          Mov   Dx,[IER_Port]
          Mov   Al,XMITINT
          Out   Dx,Al
cgexit:   Sti
          Ret
ASYNC_commgo    EndP


;   ;----------------------------------------------------------------------;
;   ;                                                                      ;
;   ;     Procedure:  sendbreak                                            ;
;   ;                                                                      ;
;   ;     Purpose:    Sends a "break" signal to the remote modem           ;
;   ;                                                                      ;
;   ;     Called from C                                                    ;
;   ;                                                                      ;
;   ;----------------------------------------------------------------------;
;             public SENDBREAK
;             ALIGN2
;   sendbreak Proc
;             Mov   Dx,[LCR_Port]
;             In    Al,Dx
;             Jmps  $+2
;             Push  Ax
;             Push  Dx
;             Or    Al,01000000b
;             Out   Dx,Al                   ; send the break signal
;
;             Xor   Ax,Ax                   ; Put a null byte into the outbound
;             Push  Ax                      ; buffer for the "break" timing
;             Call  CSendByte               ;
;
;   waitloop: TurnOnXmt
;             Mov   Ax,1
;             Push  Ax
;             Call  MYDELAY
;             Cli
;             Mov   Ax,[__OutBytes]
;             Sti
;             Or    Ax,Ax
;             Jnz   waitloop
;
;   sbdone:   Pop   Dx
;             Pop   Ax
;             Out   Dx,Al
;
;             Mov   Ax,20
;             Push  Ax
;             Call  MYDELAY
;             Ret
;   sendbreak EndP


;          public GETADDR
;          ALIGN2
;getaddr   Proc
;          Mov   Al,[Async_Irq]         ;save the address of the old handler
;          Add   Al,8
;          Call  DOS_Get_Intrpt
;          Mov   Dx,Es
;          Mov   Ax,Bx
;          Ret
;getaddr   EndP


if LPROG
  async  ends
else
  CSegEnd@
endif

          End
