Device_Driver_ID   equ 3D20h  ; Same as NetModem (compatible)
Driver_Init_Order  equ	(VCD_Init_Order - 00001000h)
Driver_Version	   equ	2004h
MaxNodes      equ  NODECOUNT
CR	      equ  0Dh
LF	      equ  0Ah
BS	      equ  08h
NULL	      equ  00h
XON	      equ  11h
XOFF	      equ  13h
emUART	      equ  0
emFOSSIL      equ  1
rsOK	      equ  0
rsBUSY	      equ  1
rsERROR	      equ  2
rsNOANSWER    equ  3
rsNOCARRIER   equ  4
rsNODIALTONE  equ  5
rsNORESULT    equ  6
MAXDWORD      equ  0FFFFFFFFh

IFDEF PRO
 IFDEF ML
  szProduct_ID	equ  "WinFOSSIL v1.12.005" ; 33 bytes
  szVersion_ID	equ  "WinFOSSIL v1.12.005"  ; 34 bytes
 ELSE
  szProduct_ID	equ  "WinFOSSIL v1.12.005" ; 33 bytes
  szVersion_ID	equ  "WinFOSSIL v1.12.005" ; 34 bytes
 ENDIF
ELSE
 IFDEF ML
  szProduct_ID	equ  "WinFOSSIL v1.12.005" ; 29 bytes
  szVersion_ID	equ  "WinFOSSIL v1.12.005" ; 30 bytes
 ELSE
  szProduct_ID	equ  "WinFOSSIL v1.12.005" ; 29 bytes
  szVersion_ID	equ  "WinFOSSIL v1.12.005" ; 30 bytes
 ENDIF
ENDIF

Boolean TypeDef BYTE

ComportStruct STRUC BYTE
 Node		 DB 0
 Enabled	 DB 0
 ComportNumber	 DB 0
 szComportName	 DB 7 Dup (0)
 Emulation	 DB 0
 Baudrate	 DW 0
 Internetport	 DW 0
 Baseaddress	 DW 0
 Alwaysactive	 DB 0
 Lockedbaudrate  DB 0
 Managetimeslice DB 0
 Buffersize	 DW 0
ComportStruct EndS

_PORTINFORMATION STRUC
 pPORTDATA _PORTDATA <>
 pPORTFUNCTIONS _PORTFUNCTIONS <>
 COMMPROP _COMMPROP <>
 COMSTAT _COMSTAT <>
 DCB _DCB <>
 QSB _QSB <>
_PORTINFORMATION EndS

UARTStruct STRUC
 RBR  DB  0
 THR  DB  0
 IER  DB  0
 IIR  DB  0
 FCR  DB  0
 LCR  DB  0
 MCR  DB  0
 LSR  DB  0
 MSR  DB  0
 SCR  DB  0
 DLL  DB  0
 DLM  DB  0
UARTStruct EndS

FOSSILStruct STRUC
 _FOSSIL  equ $
 StrSiz   DW  FOSSIL_Size
 MajVer   DB  5
 MinVer   DB  0
 Ident	  DD -1
 IBufr	  DW  ?
 IFree	  DW  ?
 OBufr	  DW  ?
 OFree	  DW  ?
 SWidth   DB  80
 SHeight  DB  25
 Baud	  DB  ?
 FOSSIL_Size equ $-_FOSSIL
FOSSILStruct EndS

CommandStruct STRUC
 E	  DB  0
 H	  DB  0
 Q	  DB  0
 Y	  DB  0
 _C	  DB  0
 _D	  DB  0
 _H       DB  0
 _I	  DB  0
 _K       DB  0
 _R	  DB  0
 _S	  DB  0
 _Y       DB  0
 S0       DB  0
 S1       DB  0
 S2       DB  0
 S3       DB  0
 S4       DB  0
 S5       DB  0
 S6       DB  0
 S7       DB  0
 S8       DB  0
 S9       DB  0
 S10      DB  0
 S11      DB  0
 S12      DB  0
 S13      DB  0
 S14      DB  0
 S15      DB  0
 S16      DB  0
 S18      DB  0
 S19      DB  0
 S21      DB  0
 S22      DB  0
 S23      DB  0
 S25      DB  0
 S27      DB  0
 S28      DB  0
 S34      DB  0
 S38      DB  0
CommandStruct EndS

DriverInfo STRUC DWORD
 Version     DW 0
 Max_Nodes   DB 0
DriverInfo EndS

InitStruct STRUC DWORD
 Init_OK     Boolean False
 Init_Error  DB 0
InitStruct EndS

IOStruct STRUC DWORD
 RXPointer   DD NULL
 IORXLength  DD NULL
 Received    DW 0
 HXPointer   DD NULL
 IOHXLength  DD NULL
 HXFree      DW 0
IOStruct EndS
;
; Custom messages
;
WM_USER 	     equ 0400h
CM_CONNECT_NODE      equ WM_USER+409
CM_DISCONNECT_NODE   equ WM_USER+410
CM_SEND_REMOTE_BREAK equ WM_USER+417
CM_WONT_BINARY	     equ WM_USER+419
CM_WILL_BINARY	     equ WM_USER+420
;
; Error codes
;
NO_ERROR          equ  0
PORT_ERROR        equ  1
MEMORY_ERROR      equ  2
IRQ_ERROR         equ  3
V86_MEMORY_ERROR  equ  4
CB_MEMORY_ERROR   equ  5
DRV_REG_ERROR	  equ  6
