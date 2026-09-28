PAGE 58,132
TITLE FOSSIL.ASM - 32-Bit Virtual FOSSIL Driver for WinFOSSIL
;============================================================================
; Title: FOSSIL.ASM - 32-Bit Virtual FOSSIL Driver for WinFOSSIL
; Driver: 32-Bit Virtual comport with true NS-16550/AFN FIFO UART emulation.
;	: 32-Bit Rev. 5 Virtual FOSSIL Driver with "X00 Superset" functions.
; Version: 2.0.0.4 - Alpha.sL/mL
; Copyright: 2026 FPC264IRC Contributors (GPLv3)
; Last Modified: February 10, 2001 @ 1:00pm EST
;============================================================================
.386P
.NoList
Include VMM.inc
Include Shell.inc
Include VPICD.inc
Include VComm.inc
Include VWin32.inc
Include RegDef.inc
Include VCommW32.inc
Include FOSSIL.inc
.List
;****************************************************************************
StatusStruct STRUC
 Node	     DB  0
 Enabled     Boolean False
 Initialized Boolean False
 InitError   DB  0
 Comport     ComportStruct <>
 PortInfo    _PORTINFORMATION <>
 UART	     UARTStruct <>
 FOSSIL      FOSSILStruct <>
 Command     CommandStruct <>
 VM_Handle   DD  0
 VCOMMOpened Boolean False
 Online      Boolean False
 Ringing     Boolean False
 Attempting  Boolean False
 Answered    Boolean False
 ATBuffer    DD  NULL
 TXBuffer    DD  NULL
 RXBuffer    DD  NULL
 ATIn	     DD  NULL
 TXIn	     DD  NULL
 RXIn	     DD  NULL
 ATOut	     DD  NULL
 TXOut	     DD  NULL
 RXOut	     DD  NULL
 ATEnd	     DD  NULL
 TXEnd	     DD  NULL
 RXEnd	     DD  NULL
 ATLength    DW  0
 TXLength    DW  0
 RXLength    DW  0
 Result      DB  0
 TimeOut     DB  0
 Trigger     DB  0
 LastChar    DB  0
 FOSSILBaud  DB  0
 RXTimeAddr   DD 0
 dwEventAddr  DD 0
 bMSRShdwAddr DD 0
 PrevLastChar  DB 0
 LastTimeslice DB 0
 ConnectRate DD NULL
 ConnectSize DD 0
 SimulateHandle DD 0
 FTSleep_Handle DD 0
 TimeOut_Handle	DD 0
 IORecvOK	Boolean False
 IOSendOK       Boolean False
 CK_Checking	Boolean False
 CK_Received	Boolean False
 Transmitter	Boolean False
 DriverDisable	Boolean False
 ICT_Ready	Boolean False
 Simulate_ELI	Boolean False
 Simulate_ETI	Boolean False
 Report_ELI	Boolean False
 Report_ETI	Boolean False
 CommandMode	Boolean False
 RXOFFReceived	Boolean False
 PossibleEscape Boolean False
StatusStruct EndS
;****************************************************************************
;			 VIRTUAL DEVICE DECLARATION!
;****************************************************************************
Declare_Virtual_Device FOSSIL, 2, 0, Control_Proc, Device_Driver_ID,\
		       Driver_Init_Order, ,
;****************************************************************************
;		       VIRTUAL DEVICE LOCAL VARIABLES!
;****************************************************************************
VxD_LOCKED_DATA_SEG

IRQ_Handle   DD 0
PortListHndl DD 0
StatusOffset DD NULL
ControlBlock DD NULL
V86Allocated DD NULL
Status_Array DD MaxNodes Dup (NULL)
VPICD_Descriptor VPICD_IRQ_Descriptor <0>
IOHandler_Table Label DWORD
   DD  Offset32  IOHandler00
   DD  Offset32  IOHandler01
   DD  Offset32  IOHandler02
   DD  Offset32  IOHandler03
   DD  Offset32  IOHandler04
   DD  Offset32  IOHandler05
   DD  Offset32  IOHandler06
   DD  Offset32  IOHandler07
IOHandler_Max = ($ - IOHandler_Table) / 4 - 1
Int14_Table Label DWORD
   DD  Offset32	 INT1400
   DD  Offset32	 INT1401
   DD  Offset32	 INT1402
   DD  Offset32	 INT1403
   DD  Offset32	 INT1404
   DD  Offset32	 INT1405
   DD  Offset32	 INT1406
   DD  Offset32	 INT1407
   DD  Offset32	 INT1408
   DD  Offset32	 INT1409
   DD  Offset32	 INT140A
   DD  Offset32	 INT140B
   DD  Offset32	 INT140C
   DD  Offset32	 INT140D
   DD  Offset32	 INT140E
   DD  Offset32	 INT140F
   DD  Offset32	 INT1410
   DD  Offset32	 INT1411
   DD  Offset32	 INT1412
   DD  Offset32	 INT1413
   DD  Offset32	 INT1414
   DD  Offset32	 INT1415
   DD  Offset32	 INT1416
   DD  Offset32	 INT1417
   DD  Offset32	 INT1418
   DD  Offset32	 INT1419
   DD  Offset32	 INT141A
   DD  Offset32  INT141B
   DD  Offset32	 INT141C
   DD  Offset32  INT141D
   DD  Offset32  INT141E
   DD  Offset32	 INT141F
   DD  Offset32	 INT1420
   DD  Offset32	 INT1421
Int14_Max = ($ - Int14_Table) / 4 - 1
IOCTL_Table Label DWORD
   DD  Offset32  IOCTL00 ; Get driver version
   DD  Offset32  IOCTL01 ; Get driver information
   DD  Offset32  IOCTL02 ; Unload port config
   DD  Offset32  IOCTL03 ; Reload port config
   DD  Offset32  IOCTL04 ; Unvirtualize IRQ
   DD  Offset32  IOCTL05 ; Virtualize IRQ
   DD  Offset32  IOCTL06 ; Startup
   DD  Offset32  IOCTL07 ; Shutdown
   DD  Offset32  IOCTL08 ; Register server window
   DD  Offset32  IOCTL09 ; Get initialization information
   DD  Offset32  IOCTL0A ; Reset node
   DD  Offset32  IOCTL0B ; Ring node
   DD  Offset32  IOCTL0C ; Answer check
   DD  Offset32  IOCTL0D ; Disconnect node
   DD  Offset32  IOCTL0E ; Input/Output
   DD  Offset32  IOCTL0F ; BREAK Received
   DD  Offset32  IOCTL10 ; Get word length (BINARY mode?)
IOCTL_Max = ($ - IOCTL_Table) / 4 - 1

VxD_LOCKED_DATA_ENDS

VxD_PAGEABLE_DATA_SEG

UndefinedDB  DB ?
UndefinedDW  DW ?
UndefinedDD  DD ?
hKey	     DD 0
SoftwareKey  DB "System\CurrentControlSet\Services\VxD\FOSSIL",NULL
ConfigValue  DB "ComportConfig",NULL
ConfigOffset DD NULL
Config_Size  DD (SizeOf ComportStruct * MaxNodes)
IRQ_Value    DB "IRQ",NULL
IRQ_Number   DD 0
IRQ_Size     DD 4
Enable_IRQ   Boolean False
Old_Int14Seg DW NULL
Old_Int14Ofs DW NULL
ServerLoaded Boolean False
ServerWindow DD 0
szBS	       DB  BS,32,BS
szPC	       DB  CR,LF,"1150",CR,LF
szRC	       DB  CR,LF,"3D20",CR,LF
szOK	       DB  CR,LF,"OK",CR,LF
szBUSY	       DB  CR,LF,"BUSY",CR,LF
szRING	       DB  CR,LF,"RING",CR,LF
szERROR	       DB  CR,LF,"ERROR",CR,LF
szNOANSWER     DB  CR,LF,"NO ANSWER",CR,LF
szNOCARRIER    DB  CR,LF,"NO CARRIER",CR,LF
szNODIALTONE   DB  CR,LF,"NO DIAL TONE",CR,LF
szCONNECT_300  DB  CR,LF,"CONNECT 300/ARQ/TELNET",CR,LF
szCONNECT_120  DB  CR,LF,"CONNECT 1200/ARQ/TELNET",CR,LF
szCONNECT_240  DB  CR,LF,"CONNECT 2400/ARQ/TELNET",CR,LF
szCONNECT_960  DB  CR,LF,"CONNECT 9600/ARQ/TELNET",CR,LF
szCONNECT_144  DB  CR,LF,"CONNECT 14400/ARQ/TELNET",CR,LF
szCONNECT_168  DB  CR,LF,"CONNECT 16800/ARQ/TELNET",CR,LF
szCONNECT_192  DB  CR,LF,"CONNECT 19200/ARQ/TELNET",CR,LF
szCONNECT_216  DB  CR,LF,"CONNECT 21600/ARQ/TELNET",CR,LF
szCONNECT_288  DB  CR,LF,"CONNECT 28800/ARQ/TELNET",CR,LF
szCONNECT_336  DB  CR,LF,"CONNECT 33600/ARQ/TELNET",CR,LF
szCONNECT_384  DB  CR,LF,"CONNECT 38400/ARQ/TELNET",CR,LF
szCONNECT_576  DB  CR,LF,"CONNECT 57600/ARQ/TELNET",CR,LF
szCONNECT_640  DB  CR,LF,"CONNECT 64000/ARQ/TELNET",CR,LF
szCONNECT_1152 DB  CR,LF,"CONNECT 115200/ARQ/TELNET",CR,LF
szProduct DB CR,LF,szProduct_ID,CR,LF
sz_FOSSIL DB szVersion_ID," GPLv3 - FPC264IRC Contributors",NULL ; 32 bytes

VxD_PAGEABLE_DATA_ENDS
;****************************************************************************
;			    VxD INIT SEGMENT!
;****************************************************************************
VxD_ICODE_SEG

 BeginProc SysDeviceCriticalInit
    pushad
    VMMCall   _HeapAllocate, <(SizeOf StatusStruct * MaxNodes), HEAPZEROINIT>
    or	      eax, eax
    jz	      SysDeviceCriticalInitFailed
    mov       [StatusOffset], eax
    xor       ecx, ecx
    .REPEAT
      pushd   ecx
      mov     eax, 4
      mul     ecx
      mov     ebx, Offset32 Status_Array
      add     ebx, eax
      mov     eax, SizeOf StatusStruct
      mul     ecx
      mov     edx, [StatusOffset]
      add     edx, eax
      mov     dword ptr [ebx], edx
      pop     ecx
      inc     ecx
    .UNTIL ecx == MaxNodes
    VMMCall   _HeapAllocate, <(SizeOf ComportStruct * MaxNodes), HEAPZEROINIT>
    or	      eax, eax
    jnz       Short @F
    mov       eax, [StatusOffset]
    VMMCall   _HeapFree, <eax, 0>
    jmp       SysDeviceCriticalInitFailed
    @@:
    mov       [ConfigOffset], eax
    VMMCall   _RegOpenKey, <HKEY_LOCAL_MACHINE, Offset32 SoftwareKey,\
		 Offset32 hKey>
    or	      eax, eax
    .IF (!ZERO?)
      mov     eax, [ConfigOffset]
      VMMCall _HeapFree, <eax, 0>
      jmp     GetConfigurationFailed
    .ENDIF
    VMMCall   _RegQueryValueEx, <hKey, Offset32 ConfigValue, NULL, NULL,\
		 ConfigOffset, Offset32 Config_Size>
    or	      eax, eax
    .IF (!ZERO?)
      VMMCall _RegCloseKey, <hKey>
      mov     eax, [ConfigOffset]
      VMMCall _HeapFree, <eax, 0>
      jmp     GetConfigurationFailed
    .ENDIF
    VMMCall   _RegCloseKey, <hKey>
    xor       ecx, ecx
    .REPEAT
      pushd   ecx
      mov     eax, 4
      mul     ecx
      mov     ebx, Offset32 Status_Array
      add     ebx, eax
      mov     edx, dword ptr [ebx]
      lea     edi, [edx].StatusStruct.Comport
      mov     eax, SizeOf ComportStruct
      mul     ecx
      mov     esi, [ConfigOffset]
      add     esi, eax
      mov     ecx, SizeOf ComportStruct
      rep     movsb
      pop     ecx
      inc     ecx
    .UNTIL ecx == MaxNodes
    mov       eax, [ConfigOffset]
    VMMCall   _HeapFree, <eax, 0>
    xor       ecx, ecx
    @@:
    pushd     ecx
    mov       eax, 4
    mul       ecx
    mov       edi, Offset32 Status_Array
    add       edi, eax
    mov       edi, dword ptr [edi]
    mov       al, [edi].StatusStruct.Comport.Node
    mov       [edi].StatusStruct.Node, al
    mov       al, [edi].StatusStruct.Comport.Enabled
    mov       [edi].StatusStruct.Enabled, al
    .IF ([edi].StatusStruct.Enabled) &&\
	([edi].StatusStruct.Comport.Emulation == emUART)
      xor     ecx, ecx
      .REPEAT
	mov	esi, Offset32 IOHandler
	xor	edx, edx
	mov	dx, [edi].StatusStruct.Comport.BaseAddress
	add	edx, ecx
	VMMCall Install_IO_Handler
	.IF (CARRY?)
	  mov [edi].StatusStruct.Enabled, False
	  mov [edi].StatusStruct.Initialized, False
	  mov [edi].StatusStruct.InitError, PORT_ERROR
	  .IF (ecx > 0)
	    .REPEAT
	      dec     edx
	      VMMCall Remove_IO_Handler
	      dec     ecx
	    .UNTIL ecx == 0
	  .ENDIF
	  pop ecx
	  inc ecx
	  cmp ecx, MaxNodes
	  jne Short @B
	  jmp @F
	.ENDIF
	inc   ecx
      .UNTIL ecx == 8
    .ENDIF
    .IF ([edi].StatusStruct.Enabled)
      mov     [edi].StatusStruct.UART.RBR, 00000000b
      mov     [edi].StatusStruct.UART.THR, 00000000b
      mov     [edi].StatusStruct.UART.IER, 00000000b
      mov     [edi].StatusStruct.UART.IIR, 00000001b
      mov     [edi].StatusStruct.UART.FCR, 00000000b
      mov     [edi].StatusStruct.UART.LCR, 00000000b
      mov     [edi].StatusStruct.UART.MCR, 00000000b
      mov     [edi].StatusStruct.UART.LSR, 01100000b
      mov     [edi].StatusStruct.UART.MSR, 00110000b
      mov     [edi].StatusStruct.UART.SCR, 00000000b
      mov     [edi].StatusStruct.UART.DLL, 30h
      mov     [edi].StatusStruct.UART.DLM, 00h
      mov     [edi].StatusStruct.FOSSIL.StrSiz, FOSSIL_Size
      mov     [edi].StatusStruct.FOSSIL.MajVer, 5
      mov     [edi].StatusStruct.FOSSIL.MinVer, 0
      mov     [edi].StatusStruct.FOSSIL.Ident, -1
      mov     ax, [UndefinedDW]
      mov     [edi].StatusStruct.FOSSIL.IBufr, ax
      mov     [edi].StatusStruct.FOSSIL.IFree, ax
      mov     [edi].StatusStruct.FOSSIL.OBufr, ax
      mov     [edi].StatusStruct.FOSSIL.OFree, ax
      mov     [edi].StatusStruct.FOSSIL.SWidth, 80
      mov     [edi].StatusStruct.FOSSIL.SHeight, 25
      mov     al, [UndefinedDB]
      mov     [edi].StatusStruct.FOSSIL.Baud, al
      mov     [edi].StatusStruct.VM_Handle, 0
      mov     [edi].StatusStruct.VCOMMOpened, False
      mov     [edi].StatusStruct.Online, False
      mov     [edi].StatusStruct.Ringing, False
      mov     [edi].StatusStruct.Attempting, False
      mov     [edi].StatusStruct.Answered, False
      mov     [edi].StatusStruct.Trigger, 1
      mov     [edi].StatusStruct.LastChar, NULL
      mov     [edi].StatusStruct.PrevLastChar, NULL
      mov     [edi].StatusStruct.IORecvOK, True
      mov     [edi].StatusStruct.IOSendOK, True
      mov     [edi].StatusStruct.CK_Checking, False
      mov     [edi].StatusStruct.CK_Received, False
      mov     [edi].StatusStruct.Transmitter, True
      mov     [edi].StatusStruct.DriverDisable, False
      mov     [edi].StatusStruct.ICT_Ready, True
      mov     [edi].StatusStruct.Simulate_ELI, False
      mov     [edi].StatusStruct.Simulate_ETI, False
      mov     [edi].StatusStruct.Report_ELI, False
      mov     [edi].StatusStruct.Report_ETI, False
      mov     [edi].StatusStruct.CommandMode, False
      mov     [edi].StatusStruct.RXOFFReceived, False
      mov     [edi].StatusStruct.PossibleEscape, False
      mov     [edi].StatusStruct.Command.H, 0
      mov     [edi].StatusStruct.Command.E, 0
      mov     [edi].StatusStruct.Command.Q, 0
      mov     [edi].StatusStruct.Command._C, 1
      mov     [edi].StatusStruct.Command._D, 2
      mov     [edi].StatusStruct.Command._H, 0
      mov     [edi].StatusStruct.Command._I, 0
      mov     [edi].StatusStruct.Command._K, 1
      mov     [edi].StatusStruct.Command._R, 2
      mov     [edi].StatusStruct.Command._S, 0
      mov     [edi].StatusStruct.Command._Y, 1
      mov     [edi].StatusStruct.Command.S0, 0
      mov     [edi].StatusStruct.Command.S1, 0
      mov     [edi].StatusStruct.Command.S2, 43
      mov     [edi].StatusStruct.Command.S3, 13
      mov     [edi].StatusStruct.Command.S4, 10
      mov     [edi].StatusStruct.Command.S5, 8
      mov     [edi].StatusStruct.Command.S6, 2
      mov     [edi].StatusStruct.Command.S7, 60
      mov     [edi].StatusStruct.Command.S8, 2
      mov     [edi].StatusStruct.Command.S9, 6
      mov     [edi].StatusStruct.Command.S10, 7
      mov     [edi].StatusStruct.Command.S11, 70
      mov     [edi].StatusStruct.Command.S12, 100
      mov     [edi].StatusStruct.Command.S13, 0
      mov     [edi].StatusStruct.Command.S14, 0
      mov     [edi].StatusStruct.Command.S15, 0
      mov     [edi].StatusStruct.Command.S16, 0
      mov     [edi].StatusStruct.Command.S18, 0
      mov     [edi].StatusStruct.Command.S19, 0
      mov     [edi].StatusStruct.Command.S21, 10
      mov     [edi].StatusStruct.Command.S22, 17
      mov     [edi].StatusStruct.Command.S23, 19
      mov     [edi].StatusStruct.Command.S25, 5
      mov     [edi].StatusStruct.Command.S27, 0
      mov     [edi].StatusStruct.Command.S28, 8
      mov     [edi].StatusStruct.Command.S34, 6
      mov     [edi].StatusStruct.Command.S38, 0
      xor     eax, eax
      mov     ax, [edi].StatusStruct.Comport.BufferSize
      mov     ebx, eax
      dec     ebx
      mov     ecx, 2
      mul     ecx
      add     eax, 43
      VMMCall _HeapAllocate, <eax, HEAPZEROINIT>
      or      eax, eax
      .IF (ZERO?)
	mov   [edi].StatusStruct.Enabled, False
	mov   [edi].StatusStruct.Initialized, False
	mov   [edi].StatusStruct.InitError, MEMORY_ERROR
	mov   [edi].StatusStruct.ATBuffer, NULL
	pop   ecx
	inc   ecx
	cmp   ecx, MaxNodes
	jne   @B
	jmp   @F
      .ENDIF
      mov     [edi].StatusStruct.ATLength, 0
      mov     [edi].StatusStruct.ATBuffer, eax
      mov     [edi].StatusStruct.ATIn, eax
      mov     [edi].StatusStruct.ATOut, eax
      add     eax, 41
      mov     [edi].StatusStruct.ATEnd, eax
      inc     eax
      inc     eax
      mov     [edi].StatusStruct.TXLength, 0
      mov     [edi].StatusStruct.TXBuffer, eax
      mov     [edi].StatusStruct.TXIn, eax
      mov     [edi].StatusStruct.TXOut, eax
      add     eax, ebx
      mov     [edi].StatusStruct.TXEnd, eax
      inc     eax
      mov     [edi].StatusStruct.RXLength, 0
      mov     [edi].StatusStruct.RXBuffer, eax
      mov     [edi].StatusStruct.RXIn, eax
      mov     [edi].StatusStruct.RXOut, eax
      add     eax, ebx
      mov     [edi].StatusStruct.RXEnd, eax
    .ENDIF
    mov       [edi].StatusStruct.Initialized, True
    mov       [edi].StatusStruct.InitError, NO_ERROR
    pop       ecx
    inc       ecx
    cmp	      ecx, MaxNodes
    jne       @B
    @@:
    xor       ecx, ecx
    .REPEAT
      mov     eax, 4
      mul     ecx
      mov     edi, Offset32 Status_Array
      add     edi, eax
      mov     edi, dword ptr [edi]
      .IF ([edi].StatusStruct.Enabled) &&\
	  ([edi].StatusStruct.Comport.Emulation == emUART)
	mov   [Enable_IRQ], True
	.BREAK
      .ELSE
	mov   [Enable_IRQ], False
      .ENDIF
      inc     ecx
    .UNTIL ecx == MaxNodes
    .IF (Enable_IRQ)
      VMMCall _RegOpenKey, <HKEY_LOCAL_MACHINE, Offset32 SoftwareKey,\
		 Offset32 hKey>
      VMMCall _RegQueryValueEx, <hKey, Offset32 IRQ_Value, NULL, NULL,\
		 Offset32 IRQ_Number, Offset32 IRQ_Size>
      VMMCall _RegCloseKey, <hKey>
      mov     edi, Offset32 VPICD_Descriptor
      mov     eax, [IRQ_Number]
      mov     [edi].VPICD_IRQ_Descriptor.VID_IRQ_Number, ax
      xor     eax, eax
      or      eax, VPICD_OPT_CAN_SHARE
      mov     [edi].VPICD_IRQ_Descriptor.VID_Options, ax
      mov     eax, Offset32 VPICD_Hw_Int_Proc
      mov     [edi].VPICD_IRQ_Descriptor.VID_Hw_Int_Proc, eax
      mov     eax, Offset32 VPICD_EOI_Proc
      mov     [edi].VPICD_IRQ_Descriptor.VID_EOI_Proc, eax
      mov     [edi].VPICD_IRQ_Descriptor.VID_IRET_Time_Out, 500
      mov     edi, Offset32 VPICD_Descriptor
      VxDCall VPICD_Virtualize_IRQ
      .IF (CARRY?)
	xor   ecx, ecx
	.REPEAT
	  mov eax, 4
	  mul ecx
	  mov edi, Offset32 Status_Array
	  add edi, eax
	  mov edi, dword ptr [edi]
	  .IF ([edi].StatusStruct.Enabled) &&\
	      ([edi].StatusStruct.Comport.Emulation == emUART)
	    mov [edi].StatusStruct.Enabled, False
	    mov [edi].StatusStruct.Initialized, False
	    mov [edi].StatusStruct.InitError, IRQ_ERROR
	  .ENDIF
	  inc ecx
	.UNTIL ecx == MaxNodes
	mov   [IRQ_Handle], 0
      .ELSE
	mov   [IRQ_Handle], eax
      .ENDIF
    .ENDIF
    GetConfigurationFailed:
    mov       eax, 2Fh
    mov       esi, Offset32 Int2F_Proc
    VMMCall   Hook_V86_Int_Chain
    jc	      SysDeviceCriticalInitFailed
    mov       eax, 14h
    VMMCall   Get_V86_Int_Vector
    mov       [Old_Int14Seg], cx
    mov       [Old_Int14Ofs], dx
    xor       edx, edx
    mov       esi, Offset32 Int14_Proc
    VMMCall   Allocate_V86_Call_Back
    jc	      Short SysDeviceCriticalInitFailed
    xor       ecx, ecx
    xor       edx, edx
    mov       dx, ax
    shr       eax, 16
    mov       cx, ax
    mov       eax, 14h
    VMMCall   Set_V86_Int_Vector
    ;
    ; NOTE: About "Control Block" memory allocation.
    ;
    ;  Allocate 6 bytes of memory in all VM's control block to store info.
    ;  byte 1 - FOSSIL Comport number (0 = com1, 1 = com2, etc..)
    ;  byte 2 - "True" if the FOSSIL driver is active.
    ;  byte 3..6 - long pointer (4 bytes) to a nodes StatusStruct.
    ;
    VMMCall   _Allocate_Device_CB_Area, <6, 0>
    or	      eax, eax
    .IF (ZERO?)
      xor     ecx, ecx
      .REPEAT
	mov   eax, 4
	mul   ecx
	mov   edi, Offset32 Status_Array
	add   edi, eax
	mov   edi, dword ptr [edi]
	mov   [edi].StatusStruct.Enabled, False
	mov   [edi].StatusStruct.Initialized, False
	mov   [edi].StatusStruct.InitError, CB_MEMORY_ERROR
	inc   ecx
      .UNTIL ecx == MaxNodes
    .ELSE
      mov     [ControlBlock], eax
    .ENDIF
    popad
    clc
    ret
    SysDeviceCriticalInitFailed:
    popad
    stc
    ret
 EndProc SysDeviceCriticalInit

 BeginProc DeviceInit
    pushad
    VxDCall   _VCOMM_Register_Port_Driver, <Offset32 _S_DriverControl>
    or	      eax, eax
    .IF (ZERO?)
      xor     ecx, ecx
      .REPEAT
	pushd ecx
	mov   eax, 4
	mul   ecx
	mov   edi, Offset32 Status_Array
	add   edi, eax
	mov   edi, dword ptr [edi]
	.IF ([edi].StatusStruct.Comport.Emulation == emUART)
	  mov [edi].StatusStruct.Enabled, False
	  mov [edi].StatusStruct.Initialized, False
	  mov [edi].StatusStruct.InitError, DRV_REG_ERROR
	.ENDIF
	pop   ecx
	inc   ecx
      .UNTIL ecx == MaxNodes
    .ENDIF
    VMMCall   _Allocate_Global_V86_Data_Area, <128, GVDADWordAlign>
    or	      eax, eax
    .IF (ZERO?)
      xor     ecx, ecx
      .REPEAT
	pushd ecx
	mov   eax, 4
	mul   ecx
	mov   edi, Offset32 Status_Array
	add   edi, eax
	mov   edi, dword ptr [edi]
	.IF ([edi].StatusStruct.Comport.Emulation == emFOSSIL)
	  mov [edi].StatusStruct.Enabled, False
	  mov [edi].StatusStruct.Initialized, False
	  mov [edi].StatusStruct.InitError, V86_MEMORY_ERROR
	.ENDIF
	pop   ecx
	inc   ecx
      .UNTIL ecx == MaxNodes
    .ELSE
      mov     [V86Allocated], eax
      mov     esi, Offset32 sz_FOSSIL
      mov     edi, eax
      mov     ecx, SizeOf sz_FOSSIL
      rep     movsb
    .ENDIF
    popad
    clc
    ret
    DeviceInitFailed:
    popad
    stc
    ret
 EndProc DeviceInit

VxD_ICODE_ENDS























;****************************************************************************
;			 VxD LOCKED CODE SEGMENT!
;****************************************************************************

VxD_LOCKED_CODE_SEG

 BeginProc Control_Proc, High_Freq
    Control_Dispatch Sys_Critical_Init, SysDeviceCriticalInit
    Control_Dispatch Device_Init, DeviceInit
    Control_Dispatch W32_DeviceIoControl, W32DeviceIoControl
    Control_Dispatch Sys_Critical_Exit, SysCriticalExit
    Control_Dispatch System_Exit, SystemExit
    Control_Dispatch VM_Not_Executeable, VMNotExecuteable
    clc
    ret
 EndProc Control_Proc

 BeginProc VPICD_Hw_Int_Proc, High_Freq
    stc
    ret
 EndProc VPICD_Hw_Int_Proc

 BeginProc VPICD_EOI_Proc, High_Freq
    VxDCall   VPICD_Get_Complete_Status
    test      ecx, VPICD_STAT_VIRT_DEV_REQ
    jz	      Short VPICD_EOI_Proc_Done
    VxDCall   VPICD_Clear_Int_Request
    mov       eax, 3
    VMMCall   Adjust_Execution_Time
    VPICD_EOI_Proc_Done:
    ret
 EndProc VPICD_EOI_Proc

 BeginProc Simulate_Interrupt, High_Freq, PUBLIC
    mov       ebx, [edi].StatusStruct.VM_Handle
    test      [ebx].cb_s.CB_VM_Status, VMStat_PM_Use32
    jnz       Short @F
    cmp       [IRQ_Handle], 0
    je	      Short @F
    mov       eax, [IRQ_Handle]
    VxDCall   VPICD_Set_Int_Request
    @@:
    ret
 EndProc Simulate_Interrupt

 BeginProc Interrupt_Generator, High_Freq
    pushad
    mov       ebx, [edx].StatusStruct.VM_Handle
    VMMCall   Validate_VM_Handle
    .IF (CARRY?) || ([edx].StatusStruct.VM_Handle == 0)
      cmp     [edx].StatusStruct.VM_Handle, 0
      je      Short Handle_Is_Zero
      cmp     [IRQ_Handle], 0
      je      Short Handle_Is_Zero
      @@:
      mov     eax, [IRQ_Handle]
      mov     ebx, [edx].StatusStruct.VM_Handle
      VxDCall VPICD_Get_Complete_Status
      test    ecx, VPICD_STAT_VIRT_DEV_REQ
      jz      Short Handle_Is_Zero
      VxDCall VPICD_Clear_Int_Request
      jmp     Short @B
      Handle_Is_Zero:
      mov     [edx].StatusStruct.SimulateHandle, 0
      .IF (![edx].StatusStruct.Online)
	or    [edx].StatusStruct.UART.IIR, 00000001b
	and   [edx].StatusStruct.UART.IIR, 11110001b
	mov   [edx].StatusStruct.Simulate_ELI, False
	mov   [edx].StatusStruct.Simulate_ETI, False
	mov   [edx].StatusStruct.Report_ELI, False
	mov   [edx].StatusStruct.Report_ETI, False
	mov   [edx].StatusStruct.ATLength, 0
	mov   ecx, [edx].StatusStruct.ATBuffer
	mov   [edx].StatusStruct.ATIn, ecx
	mov   [edx].StatusStruct.ATOut, ecx
	mov   [edx].StatusStruct.TXLength, 0
	mov   ecx, [edx].StatusStruct.TXBuffer
	mov   [edx].StatusStruct.TXIn, ecx
	mov   [edx].StatusStruct.TXOut, ecx
	mov   [edx].StatusStruct.RXLength, 0
	mov   ecx, [edx].StatusStruct.RXBuffer
	mov   [edx].StatusStruct.RXIn, ecx
	mov   [edx].StatusStruct.RXOut, ecx
	mov   [edx].StatusStruct.ICT_Ready, True
	mov   [edx].StatusStruct.DriverDisable, False
      .ENDIF
      popad
      ret
    .ENDIF
    mov       eax, [IRQ_Handle]
    mov       ebx, [edx].StatusStruct.VM_Handle
    VxDCall   VPICD_Get_Complete_Status



    .IF ([edx].StatusStruct.DriverDisable == False) &&\
	!(ecx & VPICD_STAT_VIRT_DEV_REQ) &&\
	([edx].StatusStruct.ICT_Ready)
      xor     eax, eax
      .IF ([edx].StatusStruct.UART.IIR & 00000001b)
	.IF !([edx].StatusStruct.UART.MCR & 00010000b) &&\
	     ([edx].StatusStruct.Simulate_ELI)
	  mov [edx].StatusStruct.Simulate_ELI, False
	  mov [edx].StatusStruct.Report_ELI, True
	  and [edx].StatusStruct.UART.IIR, 11111110b
	  .IF ([edx].StatusStruct.UART.IER & 00000100b) &&\
	      ([edx].StatusStruct.UART.MCR & 00001000b)
	    inc eax
	  .ENDIF
	  jmp Simulate_Check
	.ENDIF
	.IF ([edx].StatusStruct.RXLength > 0)
	  .IF (([edx].StatusStruct.Command._I == 1) ||\
	      ([edx].StatusStruct.Command._I == 2)) &&\
	      ([edx].StatusStruct.RXOFFReceived)
	    jmp Short @F
	  .ENDIF
	  .IF ([edx].StatusStruct.Command._R == 2) &&\
	     !([edx].StatusStruct.UART.MCR & 00000010b)
	    jmp Short @F
	  .ENDIF
	  and [edx].StatusStruct.UART.IIR, 11111110b
	  .IF ([edx].StatusStruct.UART.IER & 00000001b) &&\
	      ([edx].StatusStruct.UART.MCR & 00001000b)
	    inc eax
	  .ENDIF
	  jmp Short Simulate_Check
	.ENDIF
	@@:
	.IF ([edx].StatusStruct.Simulate_ETI)
	  mov [edx].StatusStruct.Simulate_ETI, False
	  mov [edx].StatusStruct.Report_ETI, True
	  and [edx].StatusStruct.UART.IIR, 11111110b
	  .IF ([edx].StatusStruct.UART.IER & 00000010b) &&\
	      ([edx].StatusStruct.UART.MCR & 00001000b)
	    inc eax
	  .ENDIF
	  jmp Short Simulate_Check
	.ENDIF
	.IF ([edx].StatusStruct.UART.MSR & 00000001b) ||\
	    ([edx].StatusStruct.UART.MSR & 00000010b) ||\
	    ([edx].StatusStruct.UART.MSR & 00000100b) ||\
	    ([edx].StatusStruct.UART.MSR & 00001000b)
	  and [edx].StatusStruct.UART.IIR, 11111110b
	  .IF ([edx].StatusStruct.UART.IER & 00001000b) &&\
	      ([edx].StatusStruct.UART.MCR & 00001000b)
	    inc eax
	  .ENDIF
	.ENDIF
      .ELSEIF ([edx].StatusStruct.UART.MCR & 00001000b)
	inc   eax
      .ENDIF
      Simulate_Check:
      .IF (eax == 1)
	mov   edi, edx
	call  Simulate_Interrupt
	cmp   [edx].StatusStruct.LastTimeslice, 255
	je    Short @F
	inc   [edx].StatusStruct.LastTimeslice
	@@:
      .ELSE
	.IF ([edx].StatusStruct.LastTimeslice >= 3) &&\
	    ([edx].StatusStruct.TXLength == 0) &&\
	    ([edx].StatusStruct.RXLength == 0)
	  VMMCall Release_Time_Slice
	  mov	  [edx].StatusStruct.LastTimeslice, 0
	.ELSEIF ([edx].StatusStruct.LastTimeslice < 255)
	  inc	  [edx].StatusStruct.LastTimeslice
	.ENDIF
      .ENDIF
    .ENDIF






    .IF (![edx].StatusStruct.VCOMMOpened)
      mov     eax, 1
      mov     esi, Offset32 Interrupt_Generator
      VMMCall Set_Global_Time_Out
      mov     [edx].StatusStruct.SimulateHandle, esi
    .ELSE
      mov     [edx].StatusStruct.SimulateHandle, 0
    .ENDIF
    popad
    ret
 EndProc Interrupt_Generator
























 BeginProc Event_Generator, High_Freq
    pushad
    mov       ebx, [edx].StatusStruct.VM_Handle
    VMMCall   Validate_VM_Handle
    .IF (CARRY?) || ([edx].StatusStruct.VM_Handle == 0)
      mov     [edx].StatusStruct.SimulateHandle, 0
      .IF (![edx].StatusStruct.Online)
	mov   [edx].StatusStruct.ATLength, 0
	mov   ecx, [edx].StatusStruct.ATBuffer
	mov   [edx].StatusStruct.ATIn, ecx
	mov   [edx].StatusStruct.ATOut, ecx
	mov   [edx].StatusStruct.TXLength, 0
	mov   ecx, [edx].StatusStruct.TXBuffer
	mov   [edx].StatusStruct.TXIn, ecx
	mov   [edx].StatusStruct.TXOut, ecx
	mov   [edx].StatusStruct.RXLength, 0
	mov   ecx, [edx].StatusStruct.RXBuffer
	mov   [edx].StatusStruct.RXIn, ecx
	mov   [edx].StatusStruct.RXOut, ecx
	mov   [edx].StatusStruct.ICT_Ready, True
      .ENDIF
      popad
      ret
    .ENDIF
    ;
    ; TODO: Check for possible Win32 comm events
    ;
    .IF ([edx].StatusStruct.VCOMMOpened)
      mov     eax, 1
      mov     esi, Offset32 Event_Generator
      VMMCall Set_Global_Time_Out
      mov     [edx].StatusStruct.SimulateHandle, esi
    .ELSE
      mov     [edx].StatusStruct.SimulateHandle, 0
    .ENDIF
    popad
    ret
 EndProc Event_Generator

 BeginProc Sleep_TimeOut, High_Freq
    pushad
    mov       ebx, [edx].StatusStruct.VM_Handle
    VMMCall   Validate_VM_Handle
    .IF (CARRY?) || ([edx].StatusStruct.VM_Handle == 0) ||\
	(![edx].StatusStruct.Online)
      mov     [edx].StatusStruct.FTSleep_Handle, 0
      popad
      ret
    .ENDIF
    VMMCall   Wake_Up_VM
    .IF ([edx].StatusStruct.Online)
      mov     eax, 1
      mov     esi, Offset32 Sleep_TimeOut
      VMMCall Set_Global_Time_Out
      mov     [edx].StatusStruct.FTSleep_Handle, esi
    .ELSE
      mov     [edx].StatusStruct.FTSleep_Handle, 0
    .ENDIF
    popad
    ret
 EndProc Sleep_TimeOut

 BeginProc Ringer_Time_Out
    pushad
    cmp       [edx].StatusStruct.Answered, False
    je	      Short @F
    popad
    ret
    @@:
    mov       [edx].StatusStruct.ICT_Ready, False
    and       [edx].StatusStruct.UART.MSR, 10111111b
    or	      [edx].StatusStruct.UART.MSR, 00000100b
    mov       ebx, [edx].StatusStruct.VM_Handle
    VMMCall   Validate_VM_Handle
    .IF (!CARRY?)
      mov     esi, Offset32 szRING
      mov     edi, [edx].StatusStruct.RXIn
      mov     ecx, SizeOf szRING
      @@:
      mov     al, byte ptr [esi]
      mov     byte ptr [edi], al
      inc     [edx].StatusStruct.RXLength
      or      [edx].StatusStruct.UART.LSR, 00000001b
      inc     esi
      inc     edi
      .IF (edi > [edx].StatusStruct.RXEnd)
	mov   edi, [edx].StatusStruct.RXBuffer
      .ENDIF
      loop    Short @B
      mov     [edx].StatusStruct.RXIn, edi
    .ENDIF
    mov       [edx].StatusStruct.ICT_Ready, True
    popad
    ret
 EndProc Ringer_Time_Out

 BeginProc Connect_Time_Out
    pushad
    cmp       [edx].StatusStruct.Attempting, True
    je	      Connect_Client
    mov       edi, edx
    mov	      [edi].StatusStruct.ICT_Ready, False
    mov	      [edi].StatusStruct.ATLength, 0
    mov	      ecx, [edi].StatusStruct.ATBuffer
    mov	      [edi].StatusStruct.ATIn, ecx
    mov	      [edi].StatusStruct.ATOut, ecx
    mov	      [edi].StatusStruct.TXLength, 0
    mov	      ecx, [edi].StatusStruct.TXBuffer
    mov	      [edi].StatusStruct.TXIn, ecx
    mov	      [edi].StatusStruct.TXOut, ecx
    mov	      [edi].StatusStruct.RXLength, 0
    mov	      ecx, [edi].StatusStruct.RXBuffer
    mov	      [edi].StatusStruct.RXIn, ecx
    mov	      [edi].StatusStruct.RXOut, ecx
    mov	      ebx, Offset32 szNOCARRIER
    mov	      edx, [edi].StatusStruct.RXIn
    mov	      ecx, SizeOf szNOCARRIER
    @@:
    mov       al, byte ptr [ebx]
    mov	      byte ptr [edx], al
    inc	      [edi].StatusStruct.RXLength
    inc	      ebx
    inc	      edx
    .IF (edx > [edi].StatusStruct.RXEnd)
      mov     edx, [edi].StatusStruct.RXBuffer
    .ENDIF
    loop      Short @B
    mov	      [edi].StatusStruct.RXIn, edx
    mov	      [edi].StatusStruct.ICT_Ready, True
    popad
    ret
    Connect_Client:
    mov       [edx].StatusStruct.ICT_Ready, False
    mov       [edx].StatusStruct.Online, True
    or	      [edx].StatusStruct.UART.MSR, 00001000b
    mov       [edx].StatusStruct.IORecvOK, False
    .IF ([edx].StatusStruct.Comport.Baudrate == 3)
      mov     esi, Offset32 szCONNECT_300
      mov     ecx, SizeOf szCONNECT_300
    .ELSEIF ([edx].StatusStruct.Comport.Baudrate == 12)
      mov     esi, Offset32 szCONNECT_120
      mov     ecx, SizeOf szCONNECT_120
    .ELSEIF ([edx].StatusStruct.Comport.Baudrate == 24)
      mov     esi, Offset32 szCONNECT_240
      mov     ecx, SizeOf szCONNECT_240
    .ELSEIF ([edx].StatusStruct.Comport.Baudrate == 96)
      mov     esi, Offset32 szCONNECT_960
      mov     ecx, SizeOf szCONNECT_960
    .ELSEIF ([edx].StatusStruct.Comport.Baudrate == 144)
      mov     esi, Offset32 szCONNECT_144
      mov     ecx, SizeOf szCONNECT_144
    .ELSEIF ([edx].StatusStruct.Comport.Baudrate == 168)
      mov     esi, Offset32 szCONNECT_168
      mov     ecx, SizeOf szCONNECT_168
    .ELSEIF ([edx].StatusStruct.Comport.Baudrate == 192)
      mov     esi, Offset32 szCONNECT_192
      mov     ecx, SizeOf szCONNECT_192
    .ELSEIF ([edx].StatusStruct.Comport.Baudrate == 216)
      mov     esi, Offset32 szCONNECT_216
      mov     ecx, SizeOf szCONNECT_216
    .ELSEIF ([edx].StatusStruct.Comport.Baudrate == 288)
      mov     esi, Offset32 szCONNECT_288
      mov     ecx, SizeOf szCONNECT_288
    .ELSEIF ([edx].StatusStruct.Comport.Baudrate == 336)
      mov     esi, Offset32 szCONNECT_336
      mov     ecx, SizeOf szCONNECT_336
    .ELSEIF ([edx].StatusStruct.Comport.Baudrate == 384)
      mov     esi, Offset32 szCONNECT_384
      mov     ecx, SizeOf szCONNECT_384
    .ELSEIF ([edx].StatusStruct.Comport.Baudrate == 576)
      mov     esi, Offset32 szCONNECT_576
      mov     ecx, SizeOf szCONNECT_576
    .ELSEIF ([edx].StatusStruct.Comport.Baudrate == 640)
      mov     esi, Offset32 szCONNECT_640
      mov     ecx, SizeOf szCONNECT_640
    .ELSEIF ([edx].StatusStruct.Comport.Baudrate == 1152)
      mov     esi, Offset32 szCONNECT_1152
      mov     ecx, SizeOf szCONNECT_1152
    .ENDIF
    mov       [edx].StatusStruct.ConnectRate, esi
    mov       [edx].StatusStruct.ConnectSize, ecx
    mov       edi, [edx].StatusStruct.RXIn
    @@:
    mov       al, byte ptr [esi]
    mov       byte ptr [edi], al
    inc       [edx].StatusStruct.RXLength
    inc       esi
    inc       edi
    .IF (edi > [edx].StatusStruct.RXEnd)
      mov     edi, [edx].StatusStruct.RXBuffer
    .ENDIF
    loop      Short @B
    mov       [edx].StatusStruct.RXIn, edi
    mov       [edx].StatusStruct.IORecvOK, True
    mov       [edx].StatusStruct.ICT_Ready, True
    popad
    ret
 EndProc Connect_Time_Out

 BeginProc Escape_Time_Out
    pushad
    mov       [edx].StatusStruct.TimeOut_Handle, 0
    cmp       [edx].StatusStruct.PossibleEscape, True
    je	      Short @F
    popad
    ret
    @@:
    mov       [edx].StatusStruct.CommandMode, True
    mov       [edx].StatusStruct.PossibleEscape, False
    mov       esi, Offset32 szOK
    mov       edi, [edx].StatusStruct.RXIn
    mov       ecx, SizeOf szOK
    @@:
    mov       al, byte ptr [esi]
    mov       byte ptr [edi], al
    inc       [edx].StatusStruct.RXLength
    inc       esi
    inc       edi
    .IF (edi > [edx].StatusStruct.RXEnd)
      mov     edi, [edx].StatusStruct.RXBuffer
    .ENDIF
    loop      Short @B
    mov       [edx].StatusStruct.RXIn, edi
    popad
    ret
 EndProc Escape_Time_Out

 BeginProc Break_Time_Out
    mov       [edx].StatusStruct.TimeOut_Handle, 0
    and       [edx].StatusStruct.UART.LSR, 11101111b
    ret
 EndProc Break_Time_Out


















 BeginProc ParseATCommand, PUBLIC
    pushad
    mov       [edi].StatusStruct.Result, rsOK
    mov       esi, [edi].StatusStruct.ATOut
    xor       ecx, ecx
    mov       cx, [edi].StatusStruct.ATLength
    Again:
    pushd     ecx
    .IF ([edi].StatusStruct.ATLength == 0) ||\
        ([edi].StatusStruct.Result == rsERROR)
      pop     ecx
      jmp     ParseATCommand_Done
    .ENDIF
    .IF (byte ptr [esi] == 'A')
      dec     [edi].StatusStruct.ATLength
      inc     esi
      .IF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	  (byte ptr [esi] == NULL)
	.IF ([edi].StatusStruct.Answered)
	  mov	  [edi].StatusStruct.Result, rsNORESULT
	  jmp	  Next
	.ENDIF
	.IF ([edi].StatusStruct.Ringing)
	  mov	  [edi].StatusStruct.Answered, True
	  mov	  [edi].StatusStruct.Ringing, False
	  mov	  [edi].StatusStruct.Result, rsNORESULT
	  pushd	  esi
	  mov	  eax, 3000
	  mov	  edx, edi
	  mov	  esi, Offset32 Connect_Time_Out
	  VMMCall Set_Global_Time_Out
	  pop	  esi
	.ELSEIF (![edi].StatusStruct.Online)
	  mov	  [edi].StatusStruct.Result, rsNOCARRIER
	.ELSE
	  mov	  [edi].StatusStruct.Result, rsOK
	.ENDIF
      .ELSE
	mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF
    .IF (byte ptr [esi] == 'E')
      dec     [edi].StatusStruct.ATLength
      inc     esi
      .IF (byte ptr [esi] == '0') || (byte ptr [esi] == '1')
	mov   al, byte ptr [esi]
	dec   [edi].StatusStruct.ATLength
	inc   esi
	.IF (al == '0')
	  mov [edi].StatusStruct.Command.E, 0
	.ELSEIF (al == '1')
	  mov [edi].StatusStruct.Command.E, 1
	.ENDIF
      .ELSEIF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	      (byte ptr [esi] == NULL)
	mov   [edi].StatusStruct.Command.E, 0
      .ELSE
        mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF
    .IF (byte ptr [esi] == 'H')
      dec     [edi].StatusStruct.ATLength
      inc     esi
      .IF (byte ptr [esi] == '0') || (byte ptr [esi] == '1')
	mov   al, byte ptr [esi]
	dec   [edi].StatusStruct.ATLength
	inc   esi
	.IF (al == '0')
	  mov [edi].StatusStruct.Command.H, 0
	  .IF (ServerLoaded) && ([edi].StatusStruct.Online)
	    mov     [edi].StatusStruct.Result, rsNORESULT
	    xor     eax, eax
	    xor     ecx, ecx
	    mov     ax, CM_DISCONNECT_NODE
	    mov     cl, [edi].StatusStruct.Comport.Node
	    VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	  .ENDIF
        .ELSE
	  mov [edi].StatusStruct.Command.H, 1
	.ENDIF
      .ELSEIF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	      (byte ptr [esi] == NULL)
	mov   [edi].StatusStruct.Command.H, 0
	.IF (ServerLoaded) && ([edi].StatusStruct.Online)
	  mov	  [edi].StatusStruct.Result, rsNORESULT
	  xor	  eax, eax
	  xor	  ecx, ecx
	  mov	  ax, CM_DISCONNECT_NODE
	  mov	  cl, [edi].StatusStruct.Comport.Node
	  VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	.ENDIF
      .ELSE
        mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF




















    .IF (byte ptr [esi] == 'I')
      dec     [edi].StatusStruct.ATLength
      inc     esi
      .IF (byte ptr [esi] == '0') || (byte ptr [esi] == '1') ||\
          (byte ptr [esi] == '2') || (byte ptr [esi] == '3') ||\
          (byte ptr [esi] == '4') || (byte ptr [esi] == '5') ||\
          (byte ptr [esi] == '6') || (byte ptr [esi] == '7')
	mov   al, byte ptr [esi]
	dec   [edi].StatusStruct.ATLength
	inc   esi
	mov   edx, [edi].StatusStruct.RXIn


	.IF (al == '0')
	  mov ebx, Offset32 szPC
	  mov ecx, 8
	  ; mov ecx, 9

	.ELSEIF (al == '1')
	  mov ebx, Offset32 szRC
	  mov ecx, 8
	.ELSEIF (al == '2')
	  mov ebx, Offset32 szOK
	  mov ecx, 6
	.ELSEIF (al == '3')
	  mov ebx, Offset32 szProduct
	  mov ecx, SizeOf szProduct


	.ELSEIF (al == '4')
	  ;
          ; TODO: Add support for I4, I5, I6 and I7
	  ;
	  mov ebx, Offset32 szProduct
	  mov ecx, SizeOf szProduct
	.ELSEIF (al == '5')
	  mov ebx, Offset32 szProduct
	  mov ecx, SizeOf szProduct
	.ELSEIF (al == '6')
	  mov ebx, Offset32 szProduct
	  mov ecx, SizeOf szProduct
	.ELSEIF (al == '7')
	  mov ebx, Offset32 szProduct
	  mov ecx, SizeOf szProduct
	.ENDIF



	@@:
	mov   al, byte ptr [ebx]
	mov   byte ptr [edx], al
	inc   [edi].StatusStruct.RXLength
	inc   ebx
	inc   edx
	.IF (edx > [edi].StatusStruct.RXEnd)
	  mov edx, [edi].StatusStruct.RXBuffer
	.ENDIF
	loop  Short @B
	mov   [edi].StatusStruct.RXIn, edx





      .ELSEIF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	      (byte ptr [esi] == NULL)
	mov   ebx, Offset32 szPC
	mov   edx, [edi].StatusStruct.RXIn
	mov   ecx, 8

	; mov ecx, 9


	@@:
	mov   al, byte ptr [ebx]
	mov   byte ptr [edx], al
	inc   [edi].StatusStruct.RXLength
	inc   ebx
	inc   edx
	.IF (edx > [edi].StatusStruct.RXEnd)
	  mov edx, [edi].StatusStruct.RXBuffer
	.ENDIF
	loop  Short @B
	mov   [edi].StatusStruct.RXIn, edx




      .ELSE
        mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF


















    .IF (byte ptr [esi] == 'O')
      dec     [edi].StatusStruct.ATLength
      inc     esi
      .IF (byte ptr [esi] == '0') || (byte ptr [esi] == '1')
	mov   al, byte ptr [esi]
	dec   [edi].StatusStruct.ATLength
	inc   esi
	cmp   [edi].StatusStruct.Online, True
	jne   Short NotOnlineA
	mov   ebx, [edi].StatusStruct.ConnectRate
	mov   edx, [edi].StatusStruct.RXIn
	mov   ecx, [edi].StatusStruct.ConnectSize
	@@:
	mov   al, byte ptr [ebx]
	mov   byte ptr [edx], al
	inc   [edi].StatusStruct.RXLength
	inc   ebx
	inc   edx
	.IF (edx > [edi].StatusStruct.RXEnd)
	  mov edx, [edi].StatusStruct.RXBuffer
	.ENDIF
	loop  Short @B
	mov   [edi].StatusStruct.RXIn, edx
	mov   [edi].StatusStruct.CommandMode, False
	mov   [edi].StatusStruct.Result, rsNORESULT
	NotOnlineA:
      .ELSEIF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	      (byte ptr [esi] == NULL)
	cmp   [edi].StatusStruct.Online, True
	jne   Short NotOnlineB
	mov   ebx, [edi].StatusStruct.ConnectRate
	mov   edx, [edi].StatusStruct.RXIn
	mov   ecx, [edi].StatusStruct.ConnectSize
	@@:
	mov   al, byte ptr [ebx]
	mov   byte ptr [edx], al
	inc   [edi].StatusStruct.RXLength
	inc   ebx
	inc   edx
	.IF (edx > [edi].StatusStruct.RXEnd)
	  mov edx, [edi].StatusStruct.RXBuffer
	.ENDIF
	loop  Short @B
	mov   [edi].StatusStruct.RXIn, edx
	mov   [edi].StatusStruct.CommandMode, False
	mov   [edi].StatusStruct.Result, rsNORESULT
	NotOnlineB:
      .ELSE
        mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF
    .IF (byte ptr [esi] == 'Q')
      dec     [edi].StatusStruct.ATLength
      inc     esi
      .IF (byte ptr [esi] == '0') || (byte ptr [esi] == '1')
	mov   al, byte ptr [esi]
	dec   [edi].StatusStruct.ATLength
	inc   esi
        .IF (al == '0')
          mov [edi].StatusStruct.Command.Q, 0
        .ELSE
          mov [edi].StatusStruct.Command.Q, 1
        .ENDIF
      .ELSEIF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	      (byte ptr [esi] == NULL)
        mov   [edi].StatusStruct.Command.Q, 0
      .ELSE    
        mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF
    .IF (byte ptr [esi] == 'Y')
      dec     [edi].StatusStruct.ATLength
      inc     esi
      .IF (byte ptr [esi] == '0') || (byte ptr [esi] == '1')
	mov   al, byte ptr [esi]
	dec   [edi].StatusStruct.ATLength
	inc   esi
        .IF (al == '0')
	  mov [edi].StatusStruct.Command.Y, 0
        .ELSE
	  mov [edi].StatusStruct.Command.Y, 1
        .ENDIF
      .ELSEIF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	      (byte ptr [esi] == NULL)
	mov   [edi].StatusStruct.Command.Y, 0
      .ELSE    
        mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF
    .IF (byte ptr [esi] == 'Z')
      dec     [edi].StatusStruct.ATLength
      inc     esi
      .IF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	  (byte ptr [esi] == NULL)
	mov   [edi].StatusStruct.Command.E, 0
	mov   [edi].StatusStruct.Command.Q, 0
	mov   [edi].StatusStruct.Command.Y, 0
	mov   [edi].StatusStruct.Command._C, 1
	mov   [edi].StatusStruct.Command._D, 2
	mov   [edi].StatusStruct.Command._H, 0
	mov   [edi].StatusStruct.Command._I, 0
	mov   [edi].StatusStruct.Command._K, 1
	mov   [edi].StatusStruct.Command._R, 2
	mov   [edi].StatusStruct.Command._S, 0
	mov   [edi].StatusStruct.Command._Y, 1
      .ELSE
        mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF
    .IF (byte ptr [esi] == '&') && (byte ptr [esi+1] == 'C')
      sub     [edi].StatusStruct.ATLength, 2
      add     esi, 2
      .IF (byte ptr [esi] == '0') || (byte ptr [esi] == '1')
	mov   al, byte ptr [esi]
	dec   [edi].StatusStruct.ATLength
	inc   esi
	.IF (al == '0')
	  mov [edi].StatusStruct.Command._C, 0
	.ELSEIF (al == '1')
	  mov [edi].StatusStruct.Command._C, 1
	.ENDIF
      .ELSEIF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	      (byte ptr [esi] == NULL)
	mov   [edi].StatusStruct.Command._C, 0
      .ELSE
        mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF
    .IF (byte ptr [esi] == '&') && (byte ptr [esi+1] == 'D')
      sub     [edi].StatusStruct.ATLength, 2
      add     esi, 2
      .IF (byte ptr [esi] == '0') || (byte ptr [esi] == '2')
	mov   al, byte ptr [esi]
	dec   [edi].StatusStruct.ATLength
	inc   esi
	.IF (al == '0')
	  mov [edi].StatusStruct.Command._D, 0
	.ELSEIF (al == '2')
	  mov [edi].StatusStruct.Command._D, 2
	.ENDIF
      .ELSEIF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	      (byte ptr [esi] == NULL)
	mov   [edi].StatusStruct.Command._D, 0
      .ELSE
        mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF
    .IF (byte ptr [esi] == '&') && (byte ptr [esi+1] == 'H')
      sub     [edi].StatusStruct.ATLength, 2
      add     esi, 2
      .IF (byte ptr [esi] == '0') || (byte ptr [esi] == '1') ||\
	  (byte ptr [esi] == '2') || (byte ptr [esi] == '3')
	mov   al, byte ptr [esi]
	dec   [edi].StatusStruct.ATLength
	inc   esi
	.IF (al == '0')
	  mov [edi].StatusStruct.Command._H, 0
	.ELSEIF (al == '1')
	  mov [edi].StatusStruct.Command._H, 1
	.ELSEIF (al == '2')
	  mov [edi].StatusStruct.Command._H, 2
	.ELSEIF (al == '3')
	  mov [edi].StatusStruct.Command._H, 3
	.ENDIF
      .ELSEIF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	      (byte ptr [esi] == NULL)
	mov   [edi].StatusStruct.Command._H, 0
      .ELSE
        mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF
    .IF (byte ptr [esi] == '&') && (byte ptr [esi+1] == 'I')
      sub     [edi].StatusStruct.ATLength, 2
      add     esi, 2
      .IF (byte ptr [esi] == '0') || (byte ptr [esi] == '1') ||\
	  (byte ptr [esi] == '2')
	mov   al, byte ptr [esi]
	dec   [edi].StatusStruct.ATLength
	inc   esi
	.IF (al == '0')
	  mov [edi].StatusStruct.Command._I, 0
	.ELSEIF (al == '1')
	  mov [edi].StatusStruct.Command._I, 1
	.ELSEIF (al == '2')
	  mov [edi].StatusStruct.Command._I, 2
	.ENDIF
      .ELSEIF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	      (byte ptr [esi] == NULL)
	mov   [edi].StatusStruct.Command._I, 0
      .ELSE
        mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF
    .IF (byte ptr [esi] == '&') && (byte ptr [esi+1] == 'K')
      sub     [edi].StatusStruct.ATLength, 2
      add     esi, 2
      .IF (byte ptr [esi] == '0') || (byte ptr [esi] == '1') ||\
	  (byte ptr [esi] == '2')
	mov   al, byte ptr [esi]
	dec   [edi].StatusStruct.ATLength
	inc   esi
	.IF (al == '0')
	  mov [edi].StatusStruct.Command._K, 0
	.ELSEIF (al == '1')
	  mov [edi].StatusStruct.Command._K, 1
	.ELSEIF (al == '2')
	  mov [edi].StatusStruct.Command._K, 2
	.ENDIF
      .ELSEIF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	      (byte ptr [esi] == NULL)
	mov   [edi].StatusStruct.Command._K, 0
      .ELSE
        mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF
    .IF (byte ptr [esi] == '&') && (byte ptr [esi+1] == 'R')
      sub     [edi].StatusStruct.ATLength, 2
      add     esi, 2
      .IF (byte ptr [esi] == '1') || (byte ptr [esi] == '2')
	mov   al, byte ptr [esi]
	dec   [edi].StatusStruct.ATLength
	inc   esi
	.IF (al == '1')
	  mov [edi].StatusStruct.Command._R, 1
	.ELSEIF (al == '2')
	  mov [edi].StatusStruct.Command._R, 2
	.ENDIF
      .ELSEIF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	      (byte ptr [esi] == NULL)
	mov   [edi].StatusStruct.Command._R, 0
      .ELSE
        mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF
    .IF (byte ptr [esi] == '&') && (byte ptr [esi+1] == 'S')
      sub     [edi].StatusStruct.ATLength, 2
      add     esi, 2
      .IF (byte ptr [esi] == '0') || (byte ptr [esi] == '1')
	mov   al, byte ptr [esi]
	dec   [edi].StatusStruct.ATLength
	inc   esi
	.IF (al == '0')
	  mov [edi].StatusStruct.Command._S, 0
	.ELSEIF (al == '1')
	  mov [edi].StatusStruct.Command._S, 1
	.ENDIF
      .ELSEIF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	      (byte ptr [esi] == NULL)
	mov   [edi].StatusStruct.Command._S, 0
      .ELSE
        mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF
    .IF (byte ptr [esi] == '&') && (byte ptr [esi+1] == 'Y')
      sub     [edi].StatusStruct.ATLength, 2
      add     esi, 2
      .IF (byte ptr [esi] == '0') || (byte ptr [esi] == '1') ||\
	  (byte ptr [esi] == '2') || (byte ptr [esi] == '3')
	mov   al, byte ptr [esi]
	dec   [edi].StatusStruct.ATLength
	inc   esi
	.IF (al == '0')
	  mov [edi].StatusStruct.Command._Y, 0
	.ELSEIF (al == '1')
	  mov [edi].StatusStruct.Command._Y, 1
	.ELSEIF (al == '2')
	  mov [edi].StatusStruct.Command._Y, 2
	.ELSEIF (al == '3')
	  mov [edi].StatusStruct.Command._Y, 3
	.ENDIF
      .ELSEIF ((byte ptr [esi] > 64) && (byte ptr [esi] < 91)) ||\
	      (byte ptr [esi] == NULL)
	mov   [edi].StatusStruct.Command._Y, 0
      .ELSE
        mov   [edi].StatusStruct.Result, rsERROR
      .ENDIF
      jmp     Next
    .ENDIF



    dec       [edi].StatusStruct.ATLength
    inc       esi
    Next:
    pop       ecx
    dec       ecx
    cmp       ecx, 0
    ja	      Again
    ParseATCommand_Done:
    popad
    ret
 EndProc ParseATCommand

























 BeginProc ATCommand, PUBLIC
    pushad
    mov       esi, [edi].StatusStruct.ATOut
    xor       ecx, ecx
    mov       cx, [edi].StatusStruct.ATLength
    @@:
    mov       al, byte ptr [esi]
    .IF (al > 96) && (al < 123)
      sub     al, 32
    .ENDIF
    mov       byte ptr [esi], al
    inc       esi
    .IF (esi > [edi].StatusStruct.ATEnd)
      mov     esi, [edi].StatusStruct.ATBuffer
    .ENDIF
    loop      Short @B
    mov       esi, [edi].StatusStruct.ATOut
    mov       ah, byte ptr [esi]
    dec       [edi].StatusStruct.ATLength
    inc       esi
    .IF (esi > [edi].StatusStruct.ATEnd)
      mov     esi, [edi].StatusStruct.ATBuffer
    .ENDIF
    mov       al, byte ptr [esi]
    dec       [edi].StatusStruct.ATLength
    inc       esi
    .IF (esi > [edi].StatusStruct.ATEnd)
      mov     esi, [edi].StatusStruct.ATBuffer
    .ENDIF
    mov       [edi].StatusStruct.ATOut, esi
    .IF (ax != 'AT')
      mov     ecx, [edi].StatusStruct.RXIn
      mov     byte ptr [ecx], CR
      inc     [edi].StatusStruct.RXLength
      inc     ecx
      .IF (ecx > [edi].StatusStruct.RXEnd)
	mov   ecx, [edi].StatusStruct.RXBuffer
      .ENDIF
      mov     byte ptr [ecx], LF
      inc     [edi].StatusStruct.RXLength
      inc     ecx
      .IF (ecx > [edi].StatusStruct.RXEnd)
	mov   ecx, [edi].StatusStruct.RXBuffer
      .ENDIF
      mov     [edi].StatusStruct.RXIn, ecx
      mov     eax, [edi].StatusStruct.ATBuffer
      mov     ecx, 43
      @@:
      mov     byte ptr [eax], NULL
      inc     eax
      loop    Short @B
      mov     [edi].StatusStruct.ATLength, 0
      mov     ecx, [edi].StatusStruct.ATBuffer
      mov     [edi].StatusStruct.ATIn, ecx
      mov     [edi].StatusStruct.ATOut, ecx
      popad
      ret
    .ENDIF
    call      ParseATCommand
    .IF ([edi].StatusStruct.Command.Q == 0)
      mov     edx, edi
      mov     edi, [edx].StatusStruct.RXIn
      .IF ([edx].StatusStruct.Result == rsOK)
	mov   esi, Offset32 szOK
	mov   ecx, 6
      .ELSEIF ([edx].StatusStruct.Result == rsERROR)
	mov   esi, Offset32 szERROR
	mov   ecx, 9
      .ELSEIF ([edx].StatusStruct.Result == rsNOCARRIER)
	mov   esi, Offset32 szNOCARRIER
	mov   ecx, 14
      .ELSEIF ([edx].StatusStruct.Result == rsNODIALTONE)
	mov   esi, Offset32 szNODIALTONE
	mov   ecx, 16
      .ELSEIF ([edx].StatusStruct.Result == rsNORESULT)
	mov   edi, edx
	jmp   Short NoResult
      .ENDIF
      @@:
      mov     al, byte ptr [esi]
      mov     byte ptr [edi], al
      inc     [edx].StatusStruct.RXLength
      inc     esi
      inc     edi
      .IF (edi > [edx].StatusStruct.RXEnd)
	mov   edi, [edx].StatusStruct.RXBuffer
      .ENDIF
      loop    Short @B
      mov     [edx].StatusStruct.RXIn, edi
      mov     edi, edx
    .ELSE
      mov     ecx, [edi].StatusStruct.RXIn
      mov     byte ptr [ecx], CR
      inc     [edi].StatusStruct.RXLength
      inc     ecx
      .IF (ecx > [edi].StatusStruct.RXEnd)
	mov   ecx, [edi].StatusStruct.RXBuffer
      .ENDIF
      mov     byte ptr [ecx], LF
      inc     [edi].StatusStruct.RXLength
      inc     ecx
      .IF (ecx > [edi].StatusStruct.RXEnd)
	mov   ecx, [edi].StatusStruct.RXBuffer
      .ENDIF
      mov     [edi].StatusStruct.RXIn, ecx
    .ENDIF
    NoResult:
    mov       eax, [edi].StatusStruct.ATBuffer
    mov       ecx, 43
    @@:
    mov       byte ptr [eax], NULL
    inc       eax
    loop      Short @B
    mov       [edi].StatusStruct.ATLength, 0
    mov       ecx, [edi].StatusStruct.ATBuffer
    mov       [edi].StatusStruct.ATIn, ecx
    mov       [edi].StatusStruct.ATOut, ecx
    popad
    ret
 EndProc ATCommand

 BeginProc GetStatusStructAddress, High_Freq, PUBLIC
    pushd     eax
    pushd     ecx
    xor	      ecx, ecx
    @@:
    pushd     edx
    mov       eax, 4
    mul       ecx
    mov       esi, Offset32 Status_Array
    add       esi, eax
    mov       edi, dword ptr [esi]
    mov       ax, [edi].StatusStruct.Comport.BaseAddress
    mov       si, ax
    add       si, 7
    pop       edx
    .IF (dx >= ax) && (dx <= si)
      mov     ecx, ebx
      add     ecx, [ControlBlock]
      mov     dword ptr [ecx+2], edi
      pop     ecx
      pop     eax
      ret
    .ENDIF
    inc	      ecx
    cmp       ecx, MaxNodes
    jne       Short @B
    mov       edi, 0
    pop	      ecx
    pop	      eax
    ret
 EndProc GetStatusStructAddress









 BeginProc IOHandler, High_Freq
    Emulate_Non_Byte_IO
    mov       edi, ebx
    add       edi, [ControlBlock]
    mov       edi, dword ptr [edi+2]
    cmp       edi, 0
    je	      Short Acquire_Comport
    cmp       [edi].StatusStruct.VM_Handle, ebx
    je	      Short Comport_Acquired
    Acquire_Comport:
    call      GetStatusStructAddress
    cmp       edi, 0
    jne       Short @F
    ret
    @@:
    mov       [edi].StatusStruct.VM_Handle, ebx
    Comport_Acquired:
    mov       [edi].StatusStruct.DriverDisable, True
    cmp       [edi].StatusStruct.SimulateHandle, 0
    jne       Short @F
    pushd     eax
    pushd     edx
    mov       eax, 1
    mov       edx, edi
    mov       esi, Offset32 Interrupt_Generator
    VMMCall   Set_Global_Time_Out
    mov       [edi].StatusStruct.SimulateHandle, esi
    pop       edx
    pop       eax
    @@:
    mov       bx, [edi].StatusStruct.Comport.BaseAddress
    xor       esi, esi
    mov       si, dx
    sub       si, bx
    jmp       IOHandler_Table[esi * 4]











    IOHandler00:
      test    [edi].StatusStruct.UART.LCR, 10000000b
      jnz     DLL_Set
      cmp     ecx, Byte_Output
      je      Short @F
      mov     cl, [edi].StatusStruct.UART.IIR
      shl     cl, 5
      cmp     cl, 10000000b
      jne     Short NotRDI
      and     [edi].StatusStruct.UART.IIR, 11110000b
      or      [edi].StatusStruct.UART.IIR, 00000001b
      NotRDI:
      cmp     [edi].StatusStruct.RXLength, 0
      ja      Short ReceiveData
      and     [edi].StatusStruct.UART.LSR, 11111110b
      mov     al, 0
      mov     [edi].StatusStruct.DriverDisable, False
      ret
      ReceiveData:
      mov     [edi].StatusStruct.IORecvOK, False
      mov     esi, [edi].StatusStruct.RXOut
      mov     al, byte ptr [esi]
      dec     [edi].StatusStruct.RXLength
      inc     esi
      .IF (esi > [edi].StatusStruct.RXEnd)
	mov   esi, [edi].StatusStruct.RXBuffer
      .ENDIF
      mov     [edi].StatusStruct.RXOut, esi
      mov     [edi].StatusStruct.IORecvOK, True
      mov     [edi].StatusStruct.DriverDisable, False
      ret
      @@:
      mov     [edi].StatusStruct.UART.THR, al
      mov     [edi].StatusStruct.Report_ETI, False
      mov     cl, [edi].StatusStruct.UART.IIR
      shl     cl, 5
      cmp     cl, 01000000b
      jne     Short @F
      or      [edi].StatusStruct.UART.IIR, 00000001b
      and     [edi].StatusStruct.UART.IIR, 11110001b
      @@:
      mov     [edi].StatusStruct.Simulate_ETI, True
      .IF ([edi].StatusStruct.UART.MCR & 00010000b)
	mov   esi, [edi].StatusStruct.RXIn
	mov   al, [edi].StatusStruct.UART.THR
	mov   byte ptr [esi], al
	inc   [edi].StatusStruct.RXLength
	inc   esi
        .IF (esi > [edi].StatusStruct.RXEnd)
          mov esi, [edi].StatusStruct.RXBuffer
        .ENDIF
	mov   [edi].StatusStruct.RXIn, esi
	mov   [edi].StatusStruct.DriverDisable, False
	ret
      .ENDIF
      .IF ([edi].StatusStruct.Online) && (![edi].StatusStruct.CommandMode)
	mov   cx, [edi].StatusStruct.Comport.BufferSize
	.IF ([edi].StatusStruct.TXLength == cx)
	  .REPEAT
	    VMMCall Release_Time_Slice
	  .UNTIL ([edi].StatusStruct.TXLength < cx) ||\
		 (![edi].StatusStruct.Online)
	.ENDIF
	mov   [edi].StatusStruct.IOSendOK, False
	mov   al, [edi].StatusStruct.UART.THR
	.IF (([edi].StatusStruct.Command._I == 1) ||\
	    ([edi].StatusStruct.Command._I == 2)) &&\
	    ((al == XOFF) || (al == XON))
	  .IF (al == XOFF)
	    mov [edi].StatusStruct.RXOFFReceived, True
	  .ELSE
	    mov [edi].StatusStruct.RXOFFReceived, False
	  .ENDIF
	  cmp [edi].StatusStruct.Command._I, 2
	  je  Short @F
	.ENDIF
	mov   esi, [edi].StatusStruct.TXIn
	mov   byte ptr [esi], al
	inc   [edi].StatusStruct.TXLength
	inc   esi
	.IF (esi > [edi].StatusStruct.TXEnd)
	  mov esi, [edi].StatusStruct.TXBuffer
	.ENDIF
	mov   [edi].StatusStruct.TXIn, esi
	@@:
	mov   [edi].StatusStruct.IOSendOK, True
	.IF ([edi].StatusStruct.PossibleEscape)
	  mov	  esi, [edi].StatusStruct.TimeOut_Handle
	  VMMCall Cancel_Time_Out
	  mov	  [edi].StatusStruct.TimeOut_Handle, 0
	  mov	  [edi].StatusStruct.PossibleEscape, False
	  mov	  [edi].StatusStruct.LastChar, NULL
	  mov	  [edi].StatusStruct.PrevLastChar, NULL
	  cmp	  al, '+'
	  jne	  Short @F
	  mov	  al, NULL
	  @@:
	.ENDIF
	.IF (al == '+') && ([edi].StatusStruct.LastChar == '+') &&\
            ([edi].StatusStruct.PrevLastChar == '+')
	  mov	  [edi].StatusStruct.PossibleEscape, True
          xor     eax, eax
          mov     al, [edi].StatusStruct.Command.S12
          mov     ecx, 20
          mul     ecx
          mov     edx, edi
          mov     esi, Offset32 Escape_Time_Out
	  VMMCall Set_Global_Time_Out
	  mov	  [edi].StatusStruct.TimeOut_Handle, esi
	.ELSE
	  mov	  cl, [edi].StatusStruct.LastChar
	  mov	  [edi].StatusStruct.LastChar, al
          mov     [edi].StatusStruct.PrevLastChar, cl
	.ENDIF
	mov   [edi].StatusStruct.DriverDisable, False
	ret
      .ENDIF
      mov     al, [edi].StatusStruct.UART.THR
      .IF (al == LF)
	nop
      .ELSEIF (al == CR)
	cmp   [edi].StatusStruct.ATLength, 1
	jna   Short @F
	call  ATCommand
	jmp   Short CalledATCommand
	@@:
	mov   ecx, [edi].StatusStruct.RXIn
	mov   byte ptr [ecx], CR
	inc   [edi].StatusStruct.RXLength
	inc   ecx
	.IF (ecx > [edi].StatusStruct.RXEnd)
	  mov ecx, [edi].StatusStruct.RXBuffer
	.ENDIF
	mov   byte ptr [ecx], LF
	inc   [edi].StatusStruct.RXLength
	inc   ecx
	.IF (ecx > [edi].StatusStruct.RXEnd)
	  mov ecx, [edi].StatusStruct.RXBuffer
	.ENDIF
	mov   [edi].StatusStruct.RXIn, ecx
	mov   ebx, [edi].StatusStruct.ATBuffer
	mov   ecx, 43
	@@:
	mov   byte ptr [ebx], NULL
	inc   ebx
	loop  Short @B
	mov   [edi].StatusStruct.ATLength, 0
	mov   ecx, [edi].StatusStruct.ATBuffer
	mov   [edi].StatusStruct.ATIn, ecx
	mov   [edi].StatusStruct.ATOut, ecx
	CalledATCommand:
      .ELSEIF (al == BS)
	.IF ([edi].StatusStruct.ATLength > 0)
	  mov	esi, [edi].StatusStruct.ATIn
	  dec	esi
	  .IF (esi < [edi].StatusStruct.ATBuffer)
	    mov esi, [edi].StatusStruct.ATBuffer
	  .ENDIF
	  mov	[edi].StatusStruct.ATIn, esi
	  mov	byte ptr [esi], NULL
	  dec	[edi].StatusStruct.ATLength
	  mov	ebx, Offset32 szBS
	  mov	edx, [edi].StatusStruct.RXIn
	  mov	ecx, SizeOf szBS
	  @@:
	  mov	al, byte ptr [ebx]
	  mov	byte ptr [edx], al
	  inc	[edi].StatusStruct.RXLength
	  inc	ebx
	  inc	edx
	  .IF (edx > [edi].StatusStruct.RXEnd)
	    mov edx, [edi].StatusStruct.RXBuffer
	  .ENDIF
	  loop	Short @B
	  mov	[edi].StatusStruct.RXIn, edx
	.ENDIF
      .ELSEIF ([edi].StatusStruct.ATLength < 42)
	mov   esi, [edi].StatusStruct.ATIn
	mov   byte ptr [esi], al
	inc   [edi].StatusStruct.ATLength
	inc   [edi].StatusStruct.ATIn
	cmp   [edi].StatusStruct.Command.E, 1
	jne   Short @F
        mov   esi, [edi].StatusStruct.RXIn
        mov   byte ptr [esi], al
	inc   [edi].StatusStruct.RXLength
	inc   esi
        .IF (esi > [edi].StatusStruct.RXEnd)
          mov esi, [edi].StatusStruct.RXBuffer
        .ENDIF
        mov   [edi].StatusStruct.RXIn, esi
        @@:
      .ENDIF
      mov     [edi].StatusStruct.DriverDisable, False
      ret
      DLL_Set:
      cmp     ecx, Byte_Output
      je      Short @F
      mov     al, [edi].StatusStruct.UART.DLL
      mov     [edi].StatusStruct.DriverDisable, False
      ret
      @@:
      mov     [edi].StatusStruct.UART.DLL, al
      mov     [edi].StatusStruct.DriverDisable, False
      ret
    IOHandler01:
      test    [edi].StatusStruct.UART.LCR, 10000000b
      jnz     Short DLM_Set
      cmp     ecx, Byte_Output
      je      Short @F
      and     [edi].StatusStruct.UART.IER, 00001111b
      mov     al, [edi].StatusStruct.UART.IER
      mov     [edi].StatusStruct.DriverDisable, False
      ret
      @@:
      mov     [edi].StatusStruct.UART.IER, al
      mov     [edi].StatusStruct.DriverDisable, False
      ret
      DLM_Set:
      cmp     ecx, Byte_Output
      je      Short @F
      mov     al, [edi].StatusStruct.UART.DLM
      mov     [edi].StatusStruct.DriverDisable, False
      ret
      @@:
      mov     [edi].StatusStruct.UART.DLM, al
      mov     [edi].StatusStruct.DriverDisable, False
      ret
    IOHandler02:
      cmp     ecx, Byte_Output
      je      @F
      .IF ([edi].StatusStruct.UART.FCR & 00000001b)
	or    [edi].StatusStruct.UART.IIR, 11000000b
      .ELSE
	and   [edi].StatusStruct.UART.IIR, 00111111b
      .ENDIF
      and     [edi].StatusStruct.UART.IIR, 11000000b
      xor     al, al
      .IF ([edi].StatusStruct.RXLength > 0)
	.IF (([edi].StatusStruct.Command._I == 1) ||\
	    ([edi].StatusStruct.Command._I == 2)) &&\
	    ([edi].StatusStruct.RXOFFReceived)
	  mov al, True
	.ENDIF
	.IF ([edi].StatusStruct.Command._R == 2) &&\
	    !([edi].StatusStruct.UART.MCR & 00000010b)
	  mov al, True
	.ENDIF
      .ENDIF
      .IF !([edi].StatusStruct.UART.MCR & 00010000b) &&\
	   ([edi].StatusStruct.Report_ELI)
	or    [edi].StatusStruct.UART.IIR, 00000110b
	mov   [edi].StatusStruct.Report_ELI, False
      .ELSEIF ([edi].StatusStruct.RXLength > 0) && (al == False)
	.IF ([edi].StatusStruct.UART.FCR & 00000001b)
	  xor ecx, ecx
	  mov cl, [edi].StatusStruct.Trigger
	  .IF (cl > 3) && ([edi].StatusStruct.RXLength < cx)
	    or [edi].StatusStruct.UART.IIR, 00001100b
	  .ELSE
	    or [edi].StatusStruct.UART.IIR, 00000100b
	  .ENDIF
	.ELSE
	  or  [edi].StatusStruct.UART.IIR, 00000100b
	.ENDIF
      .ELSEIF ([edi].StatusStruct.Report_ETI)
	or    [edi].StatusStruct.UART.IIR, 00000010b
	mov   [edi].StatusStruct.Report_ETI, False
      .ELSEIF ([edi].StatusStruct.UART.MSR & 00000001b) ||\
	      ([edi].StatusStruct.UART.MSR & 00000010b) ||\
	      ([edi].StatusStruct.UART.MSR & 00000100b) ||\
	      ([edi].StatusStruct.UART.MSR & 00001000b)
	and   [edi].StatusStruct.UART.IIR, 11110001b
      .ELSE
	or    [edi].StatusStruct.UART.IIR, 00000001b
      .ENDIF
      mov     al, [edi].StatusStruct.UART.IIR
      mov     cl, al
      shl     cl, 5
      .IF (cl == 01000000b)
	or    [edi].StatusStruct.UART.IIR, 00000001b
	and   [edi].StatusStruct.UART.IIR, 11110001b
      .ENDIF
      mov     [edi].StatusStruct.DriverDisable, False
      ret
      @@:
      mov     cl, [edi].StatusStruct.UART.FCR
      mov     [edi].StatusStruct.UART.FCR, al
      and     [edi].StatusStruct.UART.FCR, 11001111b
      test    [edi].StatusStruct.UART.FCR, 00000001b
      jnz     Short FIFO_Enabled
      .IF (cl & 00000001b)
	mov   [edi].StatusStruct.IORecvOK, False
	mov   [edi].StatusStruct.IOSendOK, False
	mov   [edi].StatusStruct.RXLength, 0
	mov   ecx, [edi].StatusStruct.RXBuffer
	mov   [edi].StatusStruct.RXIn, ecx
	mov   [edi].StatusStruct.RXOut, ecx
	mov   [edi].StatusStruct.ATLength, 0
	mov   ecx, [edi].StatusStruct.ATBuffer
	mov   [edi].StatusStruct.ATIn, ecx
	mov   [edi].StatusStruct.ATOut, ecx
	mov   [edi].StatusStruct.TXLength, 0
	mov   ecx, [edi].StatusStruct.TXBuffer
	mov   [edi].StatusStruct.TXIn, ecx
	mov   [edi].StatusStruct.TXOut, ecx
	mov   [edi].StatusStruct.IOSendOK, True
	mov   [edi].StatusStruct.IORecvOK, True
	mov   [edi].StatusStruct.Simulate_ETI, True
      .ENDIF
      mov     [edi].StatusStruct.DriverDisable, False
      ret
      FIFO_Enabled:
      test    [edi].StatusStruct.UART.FCR, 00000010b
      jz      Short @F
      and     [edi].StatusStruct.UART.FCR, 11111101b
      mov     [edi].StatusStruct.IORecvOK, False
      mov     [edi].StatusStruct.RXLength, 0
      mov     ecx, [edi].StatusStruct.RXBuffer
      mov     [edi].StatusStruct.RXIn, ecx
      mov     [edi].StatusStruct.RXOut, ecx
      mov     [edi].StatusStruct.IORecvOK, True
      @@:
      test    [edi].StatusStruct.UART.FCR, 00000100b
      jz      Short @F
      and     [edi].StatusStruct.UART.FCR, 11111011b
      mov     [edi].StatusStruct.IOSendOK, False
      mov     [edi].StatusStruct.ATLength, 0
      mov     ecx, [edi].StatusStruct.ATBuffer
      mov     [edi].StatusStruct.ATIn, ecx
      mov     [edi].StatusStruct.ATOut, ecx
      mov     [edi].StatusStruct.TXLength, 0
      mov     ecx, [edi].StatusStruct.TXBuffer
      mov     [edi].StatusStruct.TXIn, ecx
      mov     [edi].StatusStruct.TXOut, ecx
      mov     [edi].StatusStruct.IOSendOK, True
      mov     [edi].StatusStruct.Simulate_ETI, True
      @@:
      mov     al, [edi].StatusStruct.UART.FCR
      .IF (al & 11000000b)
	mov   [edi].StatusStruct.Trigger, 14
      .ELSEIF (al & 10000000b)
	mov   [edi].StatusStruct.Trigger, 8
      .ELSEIF (al & 01000000b)
	mov   [edi].StatusStruct.Trigger, 4
      .ELSE
	mov   [edi].StatusStruct.Trigger, 1
      .ENDIF
      mov     [edi].StatusStruct.DriverDisable, False
      ret
    IOHandler03:
      cmp     ecx, Byte_Output
      je      Short @F
      mov     al, [edi].StatusStruct.UART.LCR
      mov     [edi].StatusStruct.DriverDisable, False
      ret
      @@:
      mov     [edi].StatusStruct.UART.LCR, al
      .IF (ServerLoaded)
	.IF ([edi].StatusStruct.UART.LCR & 00000011b)
	  xor	  eax, eax
	  xor	  ecx, ecx
	  mov	  ax, CM_WILL_BINARY
	  mov	  cl, [edi].StatusStruct.Comport.Node
	  VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	.ELSE
	  xor	  eax, eax
	  xor	  ecx, ecx
	  mov	  ax, CM_WONT_BINARY
	  mov	  cl, [edi].StatusStruct.Comport.Node
	  VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	.ENDIF
      .ENDIF
      .IF ([edi].StatusStruct.UART.LCR & 01000000b)
	.IF ([edi].StatusStruct.Command._Y == 0)
	  mov  [edi].StatusStruct.IOSendOK, False
	  mov  [edi].StatusStruct.ATLength, 0
	  mov  ecx, [edi].StatusStruct.ATBuffer
	  mov  [edi].StatusStruct.ATIn, ecx
	  mov  [edi].StatusStruct.ATOut, ecx
	  mov  [edi].StatusStruct.TXLength, 0
	  mov  ecx, [edi].StatusStruct.TXBuffer
	  mov  [edi].StatusStruct.TXIn, ecx
	  mov  [edi].StatusStruct.TXOut, ecx
	  mov  [edi].StatusStruct.IOSendOK, True
	.ELSEIF ([edi].StatusStruct.Command._Y == 1)
	  mov  [edi].StatusStruct.IOSendOK, False
	  mov  [edi].StatusStruct.ATLength, 0
	  mov  ecx, [edi].StatusStruct.ATBuffer
	  mov  [edi].StatusStruct.ATIn, ecx
	  mov  [edi].StatusStruct.ATOut, ecx
	  mov  [edi].StatusStruct.TXLength, 0
	  mov  ecx, [edi].StatusStruct.TXBuffer
	  mov  [edi].StatusStruct.TXIn, ecx
	  mov  [edi].StatusStruct.TXOut, ecx
	  mov  [edi].StatusStruct.IOSendOK, True
	  .IF (ServerLoaded) && ([edi].StatusStruct.Online)
	    xor     eax, eax
	    xor     ecx, ecx
	    mov     ax, CM_SEND_REMOTE_BREAK
	    mov     cl, [edi].StatusStruct.Comport.Node
	    VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	  .ENDIF
	.ELSEIF ([edi].StatusStruct.Command._Y == 2)
	  .IF (ServerLoaded) && ([edi].StatusStruct.Online)
	    xor     eax, eax
	    xor     ecx, ecx
	    mov     ax, CM_SEND_REMOTE_BREAK
	    mov     cl, [edi].StatusStruct.Comport.Node
	    VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	  .ENDIF
	.ELSEIF ([edi].StatusStruct.Command._Y == 3)
	  mov	   [edi].StatusStruct.TimeOut, 0
	  .WHILE ([edi].StatusStruct.TXLength > 0)
	    mov    eax, 1000
	    call   _Delay
	    inc    [edi].StatusStruct.TimeOut
	    .BREAK .IF ([edi].StatusStruct.TimeOut == 3)
	  .ENDW
	  .IF (ServerLoaded) && ([edi].StatusStruct.Online)
	    xor     eax, eax
	    xor     ecx, ecx
	    mov     ax, CM_SEND_REMOTE_BREAK
	    mov     cl, [edi].StatusStruct.Comport.Node
	    VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	  .ENDIF
	.ENDIF
      .ENDIF
      mov     [edi].StatusStruct.DriverDisable, False
      ret
    IOHandler04:
      cmp     ecx, Byte_Output
      je      Short @F
      and     [edi].StatusStruct.UART.MCR, 00011111b
      .IF ([edi].StatusStruct.Command._D == 0)
	or    [edi].StatusStruct.UART.MCR, 00000001b
      .ENDIF
      mov     al, [edi].StatusStruct.UART.MCR
      mov     [edi].StatusStruct.DriverDisable, False
      ret
      @@:
      mov     [edi].StatusStruct.UART.MCR, al
      .IF ([edi].StatusStruct.Command._S == 1)
	.IF ([edi].StatusStruct.UART.MCR & 00000001b)
	  .IF !([edi].StatusStruct.UART.MSR & 00100000b)
	    or [edi].StatusStruct.UART.MSR, 00100010b
	  .ENDIF
	.ELSE
	  .IF ([edi].StatusStruct.UART.MSR & 00100000b)
	    and [edi].StatusStruct.UART.MSR, 11011111b
	    or	[edi].StatusStruct.UART.MSR, 00000010b
	  .ENDIF
	.ENDIF
      .ENDIF
      .IF !([edi].StatusStruct.UART.MCR & 00000001b) &&\
	  ([edi].StatusStruct.Online)
	.IF ([edi].StatusStruct.Command._D == 2) && (ServerLoaded)
	  xor	  eax, eax
	  xor	  ecx, ecx
	  mov	  ax, CM_DISCONNECT_NODE
	  mov	  cl, [edi].StatusStruct.Comport.Node
	  VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	.ENDIF
      .ENDIF
      mov     [edi].StatusStruct.DriverDisable, False
      ret
    IOHandler05:
      cmp     ecx, Byte_Input
      je      Short @F
      mov     [edi].StatusStruct.DriverDisable, False
      ret
      @@:
      or      [edi].StatusStruct.UART.LSR, 01100001b
      .IF ([edi].StatusStruct.RXLength == 0)
	and   [edi].StatusStruct.UART.LSR, 11111110b
      .ENDIF
      .IF (([edi].StatusStruct.Command._I == 1) ||\
	  ([edi].StatusStruct.Command._I == 2)) &&\
	  ([edi].StatusStruct.RXOFFReceived)
	and   [edi].StatusStruct.UART.LSR, 11111110b
      .ENDIF
      .IF ([edi].StatusStruct.Command._R == 2) &&\
	  !([edi].StatusStruct.UART.MCR & 00000010b)
	and   [edi].StatusStruct.UART.LSR, 11111110b
      .ENDIF
      mov     al, [edi].StatusStruct.UART.LSR
      and     [edi].StatusStruct.UART.LSR, 01100001b
      mov     [edi].StatusStruct.Report_ELI, False
      mov     cl, [edi].StatusStruct.UART.IIR
      shl     cl, 5
      cmp     cl, 11000000b
      jne     Short @F
      and     [edi].StatusStruct.UART.IIR, 11110000b
      or      [edi].StatusStruct.UART.IIR, 00000001b
      @@:
      mov     [edi].StatusStruct.DriverDisable, False
      ret
    IOHandler06:
      cmp     ecx, Byte_Input
      je      Short @F
      mov     [edi].StatusStruct.DriverDisable, False
      ret
      @@:
      .IF ([edi].StatusStruct.UART.MCR & 00010000b)
	xor   al, al
	test  [edi].StatusStruct.UART.MCR, 00000010b
	jz    Short @F
	or    al, 00010000b
	@@:
	test  [edi].StatusStruct.UART.MCR, 00000001b
	jz    Short @F
	or    al, 00100000b
	@@:
	test  [edi].StatusStruct.UART.MCR, 00000100b
	jz    Short @F
	or    al, 01000000b
	@@:
	test  [edi].StatusStruct.UART.MCR, 00001000b
	jz    Short @F
	or    al, 10000000b
	@@:
	mov   [edi].StatusStruct.DriverDisable, False
	ret
      .ENDIF
      .IF ([edi].StatusStruct.Command._C == 1)
	.IF ([edi].StatusStruct.Online)
	  or  [edi].StatusStruct.UART.MSR, 10000000b
	.ELSE
	  and [edi].StatusStruct.UART.MSR, 01111111b
	.ENDIF
      .ELSE
	or    [edi].StatusStruct.UART.MSR, 10000000b
      .ENDIF
      .IF ([edi].StatusStruct.Command._S == 0)
	or    [edi].StatusStruct.UART.MSR, 00100000b
      .ENDIF
      .IF ([edi].StatusStruct.Command._H == 0)
	or    [edi].StatusStruct.UART.MSR, 00010000b
      .ELSEIF ([edi].StatusStruct.Command._H == 1) ||\
	      ([edi].StatusStruct.Command._H == 3)
	mov   cx, [edi].StatusStruct.Comport.BufferSize
	.IF ([edi].StatusStruct.TXLength < cx)
	  or  [edi].StatusStruct.UART.MSR, 00010000b
	.ELSE
	  and [edi].StatusStruct.UART.MSR, 11101111b
	.ENDIF
      .ENDIF
      mov     al, [edi].StatusStruct.UART.MSR
      and     [edi].StatusStruct.UART.MSR, 11110000b
      mov     cl, [edi].StatusStruct.UART.IIR
      shl     cl, 5
      cmp     cl, 00000000b
      jne     Short @F
      and     [edi].StatusStruct.UART.IIR, 11110000b
      or      [edi].StatusStruct.UART.IIR, 00000001b
      @@:
      mov     [edi].StatusStruct.DriverDisable, False
      ret
    IOHandler07:
      cmp     ecx, Byte_Output
      je      Short @F
      mov     al, [edi].StatusStruct.UART.SCR
      mov     [edi].StatusStruct.DriverDisable, False
      ret
      @@:
      mov     [edi].StatusStruct.UART.SCR, al
      mov     [edi].StatusStruct.DriverDisable, False
      ret
 EndProc IOHandler

 BeginProc Int2F_Proc, High_Freq
    cmp       [ebp].Client_AX, 1680h
    jne       Short @F
    pushd     ebx
    add       ebx, [ControlBlock]
    .IF (byte ptr [ebx+1] == True) &&\
	([dword ptr [ebx+2]].StatusStruct.Comport.Emulation == emFOSSIL)
      mov     ebx, dword ptr [ebx+2]
      mov     [ebx].StatusStruct.LastTimeslice, 0
      .IF ([ebx].StatusStruct.TXLength > 0) ||\
	  ([ebx].StatusStruct.RXLength > 0)
	mov   [ebp].Client_AL, 00h
	pop   ebx
        clc
        ret
      .ENDIF
    .ENDIF
    pop       ebx
    @@:
    stc
    ret
 EndProc Int2F_Proc

 BeginProc TimesliceMgr, High_Freq, PUBLIC
    .IF ([ebp].Client_AH == 03h) || ([ebp].Client_AH == 1Bh)
      .IF ([edi].StatusStruct.LastTimeslice >= 3) &&\
	  ([edi].StatusStruct.TXLength == 0) &&\
	  ([edi].StatusStruct.RXLength == 0)
	VMMCall Release_Time_Slice
	mov	[edi].StatusStruct.LastTimeslice, 0
      .ELSEIF ([edi].StatusStruct.LastTimeslice < 255)
	inc   [edi].StatusStruct.LastTimeslice
      .ENDIF
    .ELSE
      mov     [edi].StatusStruct.LastTimeslice, 0
    .ENDIF
    ret
 EndProc TimesliceMgr

 BeginProc _Delay, PUBLIC
    pushad
    mov       ebx, eax
    VMMCall   Get_System_Time
    add       ebx, eax
    .WHILE (True)
      VMMCall Get_System_Time
      .BREAK  .IF (eax >= ebx)
      VMMCall Enable_VM_Ints
      VMMCall Begin_Nest_Exec
      VMMCall Resume_Exec
      VMMCall End_Nest_Exec
      VMMCall Release_Time_Slice
    .ENDW
    popad
    ret
 EndProc _Delay

 BeginProc _Cli, High_Freq, PUBLIC
    mov       [edi].StatusStruct.IORecvOK, False
    mov       [edi].StatusStruct.IOSendOK, False
    ret
 EndProc _Cli

 BeginProc _Sti, High_Freq, PUBLIC
    mov       [edi].StatusStruct.IOSendOK, True
    mov       [edi].StatusStruct.IORecvOK, True
    ret
 EndProc _Sti

 BeginProc IsFOSSILComport, High_Freq, PUBLIC
    pushd     ecx
    xor       ecx, ecx
    @@:
    pushd     ecx
    mov       eax, 4
    mul       ecx
    mov       esi, Offset32 Status_Array
    add       esi, eax
    mov       edi, dword ptr [esi]
    mov       cl, [ebp].Client_DL
    .IF ([edi].StatusStruct.Comport.ComportNumber == cl) &&\
	([edi].StatusStruct.Comport.Emulation == emFOSSIL) &&\
	([edi].StatusStruct.Enabled)
      mov     al, True
      pop     ecx
      pop     ecx
      ret
    .ENDIF
    pop       ecx
    inc       ecx
    cmp       ecx, MaxNodes
    jne       Short @B
    mov       al, False
    pop       ecx
    ret
 EndProc IsFOSSILComport

 BeginProc Int14_Proc, High_Freq
    pushad
    cmp       [ebp].Client_AH, 7Eh
    je	      INT147E
    cmp       [ebp].Client_AH, 7Fh
    je	      INT147F
    movzx     eax, [ebp].Client_AH
    cmp       ax, Int14_Max
    ja	      Int14_Fail
    cmp       [ebp].Client_DL, 0FFh
    je	      @F
    mov       edi, ebx
    add       edi, [ControlBlock]
    mov       dl, byte ptr [edi]
    .IF (dl != [ebp].Client_DL)
      .IF ([ebp].Client_AH == 07h) || ([ebp].Client_AH == 0Dh) ||\
	  ([ebp].Client_AH == 0Eh) || ([ebp].Client_AH == 11h) ||\
	  ([ebp].Client_AH == 12h)
	jmp   Int14_Table[eax * 4]
      .ENDIF
      .IF ([ebp].Client_AH == 13h) || ([ebp].Client_AH == 15h) ||\
	  ([ebp].Client_AH == 16h) || ([ebp].Client_AH == 17h) ||\
	  ([ebp].Client_AH == 1Bh)
	jmp   Int14_Table[eax * 4]
      .ENDIF
    .ENDIF
    .IF (dl != [ebp].Client_DL) || (byte ptr [edi+1] != True)
      .IF ([ebp].Client_AH == 04h) || ([ebp].Client_AH == 1Ch)
	jmp   Int14_Table[eax * 4]
      .ELSE
	jmp   Short Int14_Fail
      .ENDIF
    .ENDIF
    mov       edi, dword ptr [edi+2]
    mov       al, [edi].StatusStruct.Enabled
    and       al, [edi].StatusStruct.Comport.ManageTimeslice
    jz	      Short @F
    call      TimesliceMgr
    @@:
    movzx     eax, [ebp].Client_AH
    jmp       Int14_Table[eax * 4]
    Int14_Fail:
      xor     ecx, ecx
      xor     edx, edx
      mov     cx, [Old_Int14Seg]
      mov     dx, [Old_Int14Ofs]
      VMMcall Simulate_Far_Jmp
      popad
      ret
    INT1400:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      mov     al, [ebp].Client_AL
      mov     [edi].StatusStruct.FOSSILBaud, al
      xor     eax, eax
      .IF ([edi].StatusStruct.RXLength > 0)
	or    ah, 00000001b
      .ENDIF
      mov     cx, [edi].StatusStruct.Comport.BufferSize
      .IF ([edi].StatusStruct.RXLength > cx)
	or    ah, 00000010b
      .ENDIF
      .IF ([edi].StatusStruct.UART.LSR & 00010000b)
	or    ah, 00010000b
      .ENDIF
      .IF ([edi].StatusStruct.TXLength < cx)
	or    ah, 00100000b
      .ENDIF
      sub     cx, 15
      .IF ([edi].StatusStruct.TXLength < cx)
	or    ah, 01000000b
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 00000001b)
	or    al, 00000001b
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 00000010b)
	or    al, 00000010b
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 00001000b)
	or    al, 00000100b
      .ENDIF
      or      al, 00001000b
      .IF ([edi].StatusStruct.Command._H == 0)
	or    al, 00010000b
      .ELSEIF ([edi].StatusStruct.Command._H == 1) ||\
	      ([edi].StatusStruct.Command._H == 3)
	mov   cx, [edi].StatusStruct.Comport.BufferSize
	.IF ([edi].StatusStruct.TXLength < cx)
	  or  al, 00010000b
	.ENDIF
      .ENDIF
      .IF ([edi].StatusStruct.Command._S == 0)
	or    al, 00100000b
      .ELSE
	.IF ([edi].StatusStruct.UART.MCR & 00000001b)
	  or  al, 00100000b
	.ENDIF
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 01000000b) ||\
	  ([edi].StatusStruct.UART.MSR & 00000100b)
	or    al, 01000000b
      .ENDIF
      .IF ([edi].StatusStruct.Command._C == 1)
	.IF ([edi].StatusStruct.Online)
	  or  al, 10000000b
	.ENDIF
      .ELSE
	or    al, 10000000b
      .ENDIF
      mov     [ebp].Client_AX, ax
      VMMCall Simulate_Iret
      popad
      ret
    INT1401:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      .IF ([edi].StatusStruct.UART.MCR & 00010000b)
	call  _Cli
	mov   esi, [edi].StatusStruct.RXIn
	mov   dl, [ebp].Client_AL
	mov   byte ptr [esi], dl
	inc   [edi].StatusStruct.RXLength
	inc   esi
	.IF (esi > [edi].StatusStruct.RXEnd)
	  mov esi, [edi].StatusStruct.RXBuffer
	.ENDIF
	mov   [edi].StatusStruct.RXIn, esi
	call  _Sti
	xor   eax, eax
	jmp   NoSend
      .ENDIF
      mov     [edi].StatusStruct.TimeOut, 0
      mov     cx, [edi].StatusStruct.Comport.BufferSize
      .WHILE ([edi].StatusStruct.TXLength == cx)
	mov   eax, 1000
	call  _Delay
	inc   [edi].StatusStruct.TimeOut
	.IF ([edi].StatusStruct.TimeOut == 30)
	  xor eax, eax
	  or  ah, 10000000b
	  jmp NoSend
	.ENDIF
      .ENDW
      .IF ([edi].StatusStruct.Online) && (![edi].StatusStruct.CommandMode)
	call  _Cli
	mov   dl, [ebp].Client_AL
	mov   ecx, [edi].StatusStruct.TXIn
	mov   byte ptr [ecx], dl
	inc   [edi].StatusStruct.TXLength
	inc   ecx
	.IF (ecx > [edi].StatusStruct.TXEnd)
	  mov ecx, [edi].StatusStruct.TXBuffer
	.ENDIF
	mov   [edi].StatusStruct.TXIn, ecx
	call  _Sti
	.IF ([edi].StatusStruct.PossibleEscape)
	  mov	  esi, [edi].StatusStruct.TimeOut_Handle
	  VMMCall Cancel_Time_Out
	  mov	  [edi].StatusStruct.TimeOut_Handle, 0
	  mov	  [edi].StatusStruct.PossibleEscape, False
	  mov	  [edi].StatusStruct.LastChar, NULL
	  mov	  [edi].StatusStruct.PrevLastChar, NULL
	  cmp	  dl, '+'
	  jne	  Short @F
	  mov	  dl, NULL
	  @@:
	.ENDIF
	.IF (dl == '+') && ([edi].StatusStruct.LastChar == '+') &&\
	    ([edi].StatusStruct.PrevLastChar == '+')
	  mov	  [edi].StatusStruct.PossibleEscape, True
	  xor	  eax, eax
	  mov	  al, [edi].StatusStruct.Command.S12
	  mov	  ecx, 20
	  mul	  ecx
	  mov	  edx, edi
	  mov	  esi, Offset32 Escape_Time_Out
	  VMMCall Set_Global_Time_Out
	  mov	  [edi].StatusStruct.TimeOut_Handle, esi
	.ELSE
	  mov cl, [edi].StatusStruct.LastChar
	  mov [edi].StatusStruct.LastChar, dl
	  mov [edi].StatusStruct.PrevLastChar, cl
	.ENDIF
	xor   eax, eax
	jmp   NoSend
      .ENDIF
      mov     dl, [ebp].Client_AL
      .IF (dl == LF)
	nop
      .ELSEIF (dl == CR)
	cmp   [edi].StatusStruct.ATLength, 1
	jna   Short @F
	call  ATCommand
	jmp   Short _CalledATCommand
	@@:
	mov   ecx, [edi].StatusStruct.RXIn
	mov   byte ptr [ecx], CR
	inc   [edi].StatusStruct.RXLength
	inc   ecx
	.IF (ecx > [edi].StatusStruct.RXEnd)
	  mov ecx, [edi].StatusStruct.RXBuffer
	.ENDIF
	mov   byte ptr [ecx], LF
	inc   [edi].StatusStruct.RXLength
	inc   ecx
	.IF (ecx > [edi].StatusStruct.RXEnd)
	  mov ecx, [edi].StatusStruct.RXBuffer
	.ENDIF
	mov   [edi].StatusStruct.RXIn, ecx
	mov   ebx, [edi].StatusStruct.ATBuffer
	mov   ecx, 43
	@@:
	mov   byte ptr [ebx], NULL
	inc   ebx
	loop  Short @B
	mov   [edi].StatusStruct.ATLength, 0
	mov   ecx, [edi].StatusStruct.ATBuffer
	mov   [edi].StatusStruct.ATIn, ecx
	mov   [edi].StatusStruct.ATOut, ecx
	_CalledATCommand:
      .ELSEIF (dl == BS)
	.IF ([edi].StatusStruct.ATLength > 0)
	  mov esi, [edi].StatusStruct.ATIn
	  dec esi
	  .IF (esi < [edi].StatusStruct.ATBuffer)
	    mov esi, [edi].StatusStruct.ATBuffer
	  .ENDIF
	  mov [edi].StatusStruct.ATIn, esi
	  mov byte ptr [esi], NULL
	  dec [edi].StatusStruct.ATLength
	  mov esi, Offset32 szBS
	  mov ebx, [edi].StatusStruct.RXIn
	  mov ecx, SizeOf szBS
	  @@:
	  mov dl, byte ptr [esi]
	  mov byte ptr [ebx], dl
	  inc [edi].StatusStruct.RXLength
	  inc esi
	  inc ebx
	  .IF (ebx > [edi].StatusStruct.RXEnd)
	    mov ebx, [edi].StatusStruct.RXBuffer
	  .ENDIF
	  loop Short @B
	  mov  [edi].StatusStruct.RXIn, ebx
	.ENDIF
      .ELSEIF ([edi].StatusStruct.ATLength < 42)
	mov   esi, [edi].StatusStruct.ATIn
	mov   byte ptr [esi], dl
	inc   [edi].StatusStruct.ATLength
	inc   [edi].StatusStruct.ATIn
	cmp   [edi].StatusStruct.Command.E, 1
	jne   Short @F
	mov   esi, [edi].StatusStruct.RXIn
	mov   byte ptr [esi], dl
	inc   [edi].StatusStruct.RXLength
	inc   esi
	.IF (esi > [edi].StatusStruct.RXEnd)
	  mov esi, [edi].StatusStruct.RXBuffer
	.ENDIF
	mov   [edi].StatusStruct.RXIn, esi
	@@:
      .ENDIF
      xor     eax, eax
      NoSend:
      .IF ([edi].StatusStruct.RXLength > 0)
	or    ah, 00000001b
      .ENDIF
      mov     cx, [edi].StatusStruct.Comport.BufferSize
      .IF ([edi].StatusStruct.RXLength > cx)
	or    ah, 00000010b
      .ENDIF
      .IF ([edi].StatusStruct.UART.LSR & 00010000b)
	or    ah, 00010000b
      .ENDIF
      .IF ([edi].StatusStruct.TXLength < cx)
	or    ah, 00100000b
      .ENDIF
      sub     cx, 15
      .IF ([edi].StatusStruct.TXLength < cx)
	or    ah, 01000000b
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 00000001b)
	or    al, 00000001b
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 00000010b)
	or    al, 00000010b
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 00001000b)
	or    al, 00000100b
      .ENDIF
      or      al, 00001000b
      .IF ([edi].StatusStruct.Command._H == 0)
	or    al, 00010000b
      .ELSEIF ([edi].StatusStruct.Command._H == 1) ||\
	      ([edi].StatusStruct.Command._H == 3)
	mov   cx, [edi].StatusStruct.Comport.BufferSize
	.IF ([edi].StatusStruct.TXLength < cx)
	  or  al, 00010000b
	.ENDIF
      .ENDIF
      .IF ([edi].StatusStruct.Command._S == 0)
	or    al, 00100000b
      .ELSE
	.IF ([edi].StatusStruct.UART.MCR & 00000001b)
	  or  al, 00100000b
	.ENDIF
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 01000000b) ||\
	  ([edi].StatusStruct.UART.MSR & 00000100b)
	or    al, 01000000b
      .ENDIF
      .IF ([edi].StatusStruct.Command._C == 1)
	.IF ([edi].StatusStruct.Online)
	  or  al, 10000000b
	.ENDIF
      .ELSE
	or    al, 10000000b
      .ENDIF
      mov     [ebp].Client_AX, ax
      VMMCall Simulate_Iret
      popad
      ret
    INT1402:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      mov     [edi].StatusStruct.TimeOut, 0
      .WHILE ([edi].StatusStruct.RXLength == 0)
	mov   eax, 1000
	call  _Delay
	inc   [edi].StatusStruct.TimeOut
	.IF ([edi].StatusStruct.TimeOut == 30)
	  xor eax, eax
	  or  ah, 10000000b
	  jmp Short NoRecv
	.ENDIF
      .ENDW
      xor     eax, eax
      call    _Cli
      mov     esi, [edi].StatusStruct.RXOut
      mov     al, byte ptr [esi]
      dec     [edi].StatusStruct.RXLength
      inc     esi
      .IF (esi > [edi].StatusStruct.RXEnd)
	mov   esi, [edi].StatusStruct.RXBuffer
      .ENDIF
      mov     [edi].StatusStruct.RXOut, esi
      call    _Sti
      NoRecv:
      .IF ([edi].StatusStruct.RXLength > 0)
	or    ah, 00000001b
      .ENDIF
      mov     cx, [edi].StatusStruct.Comport.BufferSize
      .IF ([edi].StatusStruct.RXLength > cx)
	or    ah, 00000010b
      .ENDIF
      .IF ([edi].StatusStruct.UART.LSR & 00010000b)
	or    ah, 00010000b
      .ENDIF
      .IF ([edi].StatusStruct.TXLength < cx)
	or    ah, 00100000b
      .ENDIF
      sub     cx, 15
      .IF ([edi].StatusStruct.TXLength < cx)
	or    ah, 01000000b
      .ENDIF
      mov     [ebp].Client_AX, ax
      VMMCall Simulate_Iret
      popad
      ret
    INT1403:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      xor     eax, eax
      .IF ([edi].StatusStruct.RXLength > 0)
	or    ah, 00000001b
      .ENDIF
      mov     cx, [edi].StatusStruct.Comport.BufferSize
      .IF ([edi].StatusStruct.RXLength > cx)
	or    ah, 00000010b
      .ENDIF
      .IF ([edi].StatusStruct.UART.LSR & 00010000b)
	or    ah, 00010000b
      .ENDIF
      .IF ([edi].StatusStruct.TXLength < cx)
	or    ah, 00100000b
      .ENDIF
      sub     cx, 15
      .IF ([edi].StatusStruct.TXLength < cx)
	or    ah, 01000000b
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 00000001b)
	or    al, 00000001b
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 00000010b)
	or    al, 00000010b
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 00001000b)
	or    al, 00000100b
      .ENDIF
      or      al, 00001000b
      .IF ([edi].StatusStruct.Command._H == 0)
	or    al, 00010000b
      .ELSEIF ([edi].StatusStruct.Command._H == 1) ||\
	      ([edi].StatusStruct.Command._H == 3)
	mov   cx, [edi].StatusStruct.Comport.BufferSize
	.IF ([edi].StatusStruct.TXLength < cx)
	  or  al, 00010000b
	.ENDIF
      .ENDIF
      .IF ([edi].StatusStruct.Command._S == 0)
	or    al, 00100000b
      .ELSE
	.IF ([edi].StatusStruct.UART.MCR & 00000001b)
	  or  al, 00100000b
	.ENDIF
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 01000000b) ||\
	  ([edi].StatusStruct.UART.MSR & 00000100b)
	or    al, 01000000b
      .ENDIF
      .IF ([edi].StatusStruct.Command._C == 1)
	.IF ([edi].StatusStruct.Online)
	  or  al, 10000000b
	.ENDIF
      .ELSE
	or    al, 10000000b
      .ENDIF
      mov     [ebp].Client_AX, ax
      VMMCall Simulate_Iret
      popad
      ret
    INT1404:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      mov     [ebp].Client_AX, 1954h
      mov     [ebp].Client_BX, 0521h
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      mov     ecx, ebx
      add     ecx, [ControlBlock]
      .IF (byte ptr [ecx+1] == True) &&\
	  ([dword ptr [ecx+2]].StatusStruct.Enabled)
	call	_Cli
	mov	[edi].StatusStruct.ATLength, 0
	mov	ecx, [edi].StatusStruct.ATBuffer
	mov	[edi].StatusStruct.ATIn, ecx
	mov	[edi].StatusStruct.ATOut, ecx
	mov	[edi].StatusStruct.TXLength, 0
	mov	ecx, [edi].StatusStruct.TXBuffer
	mov	[edi].StatusStruct.TXIn, ecx
	mov	[edi].StatusStruct.TXOut, ecx
	mov	[edi].StatusStruct.RXLength, 0
	mov	ecx, [edi].StatusStruct.RXBuffer
	mov	[edi].StatusStruct.RXIn, ecx
	mov	[edi].StatusStruct.RXOut, ecx
	call	_Sti
	mov	[edi].StatusStruct.LastTimeslice, 0
	mov	[edi].StatusStruct.Transmitter, True
	mov	[edi].StatusStruct.CK_Checking, False
	mov	[edi].StatusStruct.CK_Received, False
	mov	[ebp].Client_AX, 1954h
	mov	[ebp].Client_BX, 0521h
	VMMCall Simulate_Iret
	popad
	ret
      .ENDIF
      call    IsFOSSILComport
      cmp     al, True
      je      Short @F
      xor     ecx, ecx
      xor     edx, edx
      mov     cx, [Old_Int14Seg]
      mov     dx, [Old_Int14Ofs]
      VMMCall Simulate_Far_Jmp
      popad
      ret
      @@:
      mov     [edi].StatusStruct.VM_Handle, ebx
      add     ebx, [ControlBlock]
      mov     dl, [ebp].Client_DL
      mov     byte ptr [ebx], dl
      mov     byte ptr [ebx+1], True
      mov     dword ptr [ebx+2], edi
      call    _Cli
      mov     [edi].StatusStruct.ATLength, 0
      mov     ecx, [edi].StatusStruct.ATBuffer
      mov     [edi].StatusStruct.ATIn, ecx
      mov     [edi].StatusStruct.ATOut, ecx
      mov     [edi].StatusStruct.TXLength, 0
      mov     ecx, [edi].StatusStruct.TXBuffer
      mov     [edi].StatusStruct.TXIn, ecx
      mov     [edi].StatusStruct.TXOut, ecx
      mov     [edi].StatusStruct.RXLength, 0
      mov     ecx, [edi].StatusStruct.RXBuffer
      mov     [edi].StatusStruct.RXIn, ecx
      mov     [edi].StatusStruct.RXOut, ecx
      call    _Sti
      mov     [edi].StatusStruct.LastTimeslice, 0
      mov     [edi].StatusStruct.Transmitter, True
      mov     [edi].StatusStruct.CK_Checking, False
      mov     [edi].StatusStruct.CK_Received, False
      or      [edi].StatusStruct.UART.MCR, 00000001b
      mov     [ebp].Client_AX, 1954h
      mov     [ebp].Client_BX, 0521h
      VMMCall Simulate_Iret
      popad
      ret
    INT1405:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      add     ebx, [ControlBlock]
      .IF (byte ptr [ebx+1] == True)
	cmp   [edi].StatusStruct.Comport.Alwaysactive, True
	je    Short @F
	mov   byte ptr [ebx], NULL
	mov   byte ptr [ebx+1], False
	@@:
	call  _Cli
	mov   [edi].StatusStruct.ATLength, 0
	mov   ecx, [edi].StatusStruct.ATBuffer
	mov   [edi].StatusStruct.ATIn, ecx
	mov   [edi].StatusStruct.ATOut, ecx
	mov   [edi].StatusStruct.TXLength, 0
	mov   ecx, [edi].StatusStruct.TXBuffer
	mov   [edi].StatusStruct.TXIn, ecx
	mov   [edi].StatusStruct.TXOut, ecx
	mov   [edi].StatusStruct.RXLength, 0
	mov   ecx, [edi].StatusStruct.RXBuffer
	mov   [edi].StatusStruct.RXIn, ecx
	mov   [edi].StatusStruct.RXOut, ecx
	call  _Sti
	mov   [edi].StatusStruct.LastTimeslice, 0
	mov   [edi].StatusStruct.Transmitter, True
	mov   [edi].StatusStruct.CK_Checking, False
	mov   [edi].StatusStruct.CK_Received, False
      .ENDIF
      VMMCall Simulate_Iret
      popad
      ret
    INT1406:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      .IF ([ebp].Client_AL == 00h)
	and   [edi].StatusStruct.UART.MCR, 11111110b
	.IF ([edi].StatusStruct.Command._D == 2) &&\
	    (ServerLoaded) && ([edi].StatusStruct.Online)
	  xor	  eax, eax
	  xor	  ecx, ecx
	  mov	  ax, CM_DISCONNECT_NODE
	  mov	  cl, [edi].StatusStruct.Comport.Node
	  VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	.ENDIF
      .ELSEIF ([ebp].Client_AL == 01h)
	or    [edi].StatusStruct.UART.MCR, 00000001b
      .ENDIF
      VMMCall Simulate_Iret
      popad
      ret
    INT1407:
      mov     [ebp].Client_AH, 18
      mov     [ebp].Client_AL, 08h
      xor     eax, eax
      xor     ecx, ecx
      mov     ax, 1000
      mov     cl, 18
      div     ecx
      mov     [ebp].Client_DX, ax
      VMMCall Simulate_Iret
      popad
      ret
    INT1408:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      mov     [edi].StatusStruct.TimeOut, 0
      .WHILE ([edi].StatusStruct.TXLength > 0)
	mov   eax, 1000
	call  _Delay
	inc   [edi].StatusStruct.TimeOut
	.BREAK .IF ([edi].StatusStruct.TimeOut == 30)
      .ENDW
      call    _Cli
      mov     [edi].StatusStruct.ATLength, 0
      mov     ecx, [edi].StatusStruct.ATBuffer
      mov     [edi].StatusStruct.ATIn, ecx
      mov     [edi].StatusStruct.ATOut, ecx
      mov     [edi].StatusStruct.TXLength, 0
      mov     ecx, [edi].StatusStruct.TXBuffer
      mov     [edi].StatusStruct.TXIn, ecx
      mov     [edi].StatusStruct.TXOut, ecx
      call    _Sti
      VMMcall Simulate_Iret
      popad
      ret
    INT1409:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      call    _Cli
      mov     [edi].StatusStruct.ATLength, 0
      mov     ecx, [edi].StatusStruct.ATBuffer
      mov     [edi].StatusStruct.ATIn, ecx
      mov     [edi].StatusStruct.ATOut, ecx
      mov     [edi].StatusStruct.TXLength, 0
      mov     ecx, [edi].StatusStruct.TXBuffer
      mov     [edi].StatusStruct.TXIn, ecx
      mov     [edi].StatusStruct.TXOut, ecx
      call    _Sti
      VMMcall Simulate_Iret
      popad
      ret
    INT140A:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      call    _Cli
      mov     [edi].StatusStruct.RXLength, 0
      mov     ecx, [edi].StatusStruct.RXBuffer
      mov     [edi].StatusStruct.RXIn, ecx
      mov     [edi].StatusStruct.RXOut, ecx
      call    _Sti
      VMMcall Simulate_Iret
      popad
      ret
    INT140B:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      .IF ([edi].StatusStruct.UART.MCR & 00010000b)
	call	_Cli
	mov	esi, [edi].StatusStruct.RXIn
	mov	dl, [ebp].Client_AL
	mov	byte ptr [esi], dl
	inc	[edi].StatusStruct.RXLength
	inc	esi
	.IF (esi > [edi].StatusStruct.RXEnd)
	  mov	esi, [edi].StatusStruct.RXBuffer
	.ENDIF
	mov	[edi].StatusStruct.RXIn, esi
	call	_Sti
	mov	[ebp].Client_AX, 0001h
	VMMCall Simulate_Iret
	popad
	ret
      .ENDIF
      mov     cx, [edi].StatusStruct.Comport.BufferSize
      .IF ([edi].StatusStruct.TXLength < cx)
	.IF ([edi].StatusStruct.Online) && (![edi].StatusStruct.CommandMode)
	  call	_Cli
	  mov	dl, [ebp].Client_AL
	  mov	ecx, [edi].StatusStruct.TXIn
	  mov	byte ptr [ecx], dl
	  inc	[edi].StatusStruct.TXLength
	  inc	ecx
	  .IF (ecx > [edi].StatusStruct.TXEnd)
	    mov ecx, [edi].StatusStruct.TXBuffer
	  .ENDIF
	  mov	[edi].StatusStruct.TXIn, ecx
	  call	_Sti
	  .IF ([edi].StatusStruct.PossibleEscape)
	    mov	    esi, [edi].StatusStruct.TimeOut_Handle
	    VMMCall Cancel_Time_Out
	    mov	    [edi].StatusStruct.TimeOut_Handle, 0
	    mov	    [edi].StatusStruct.PossibleEscape, False
	    mov	    [edi].StatusStruct.LastChar, NULL
	    mov	    [edi].StatusStruct.PrevLastChar, NULL
	    cmp	    dl, '+'
	    jne	    Short @F
	    mov	    dl, NULL
	    @@:
	  .ENDIF
	  .IF (dl == '+') && ([edi].StatusStruct.LastChar == '+') &&\
	      ([edi].StatusStruct.PrevLastChar == '+')
	    mov	    [edi].StatusStruct.PossibleEscape, True
	    xor	    eax, eax
	    mov	    al, [edi].StatusStruct.Command.S12
	    mov	    ecx, 20
	    mul	    ecx
	    mov	    edx, edi
	    mov	    esi, Offset32 Escape_Time_Out
	    VMMCall Set_Global_Time_Out
	    mov	    [edi].StatusStruct.TimeOut_Handle, esi
	  .ELSE
	    mov cl, [edi].StatusStruct.LastChar
	    mov [edi].StatusStruct.LastChar, dl
	    mov [edi].StatusStruct.PrevLastChar, cl
	  .ENDIF
	  mov	  [ebp].Client_AX, 0001h
	  VMMCall Simulate_Iret
	  popad
	  ret
	.ENDIF
	mov   dl, [ebp].Client_AL
	.IF (dl == LF)
	  nop
	.ELSEIF (dl == CR)
	  cmp	[edi].StatusStruct.ATLength, 1
	  jna	Short @F
	  call	ATCommand
	  jmp	Short __CalledATCommand
	  @@:
	  mov	ecx, [edi].StatusStruct.RXIn
	  mov	byte ptr [ecx], CR
	  inc	[edi].StatusStruct.RXLength
	  inc	ecx
	  .IF (ecx > [edi].StatusStruct.RXEnd)
	    mov ecx, [edi].StatusStruct.RXBuffer
	  .ENDIF
	  mov	byte ptr [ecx], LF
	  inc	[edi].StatusStruct.RXLength
	  inc	ecx
	  .IF (ecx > [edi].StatusStruct.RXEnd)
	    mov ecx, [edi].StatusStruct.RXBuffer
	  .ENDIF
	  mov	[edi].StatusStruct.RXIn, ecx
	  mov	ebx, [edi].StatusStruct.ATBuffer
	  mov	ecx, 43
	  @@:
	  mov	byte ptr [ebx], NULL
	  inc	ebx
	  loop	Short @B
	  mov	[edi].StatusStruct.ATLength, 0
	  mov	ecx, [edi].StatusStruct.ATBuffer
	  mov	[edi].StatusStruct.ATIn, ecx
	  mov	[edi].StatusStruct.ATOut, ecx
	  __CalledATCommand:
	.ELSEIF (dl == BS)
	  .IF ([edi].StatusStruct.ATLength > 0)
	    mov   esi, [edi].StatusStruct.ATIn
	    dec   esi
	    .IF (esi < [edi].StatusStruct.ATBuffer)
	      mov esi, [edi].StatusStruct.ATBuffer
	    .ENDIF
	    mov   [edi].StatusStruct.ATIn, esi
	    mov   byte ptr [esi], NULL
	    dec   [edi].StatusStruct.ATLength
	    mov   esi, Offset32 szBS
	    mov   ebx, [edi].StatusStruct.RXIn
	    mov   ecx, SizeOf szBS
	    @@:
	    mov   dl, byte ptr [esi]
	    mov   byte ptr [ebx], dl
	    inc   [edi].StatusStruct.RXLength
	    inc   esi
	    inc   ebx
	    .IF (ebx > [edi].StatusStruct.RXEnd)
	      mov ebx, [edi].StatusStruct.RXBuffer
	    .ENDIF
	    loop  Short @B
	    mov	  [edi].StatusStruct.RXIn, ebx
	  .ENDIF
	.ELSEIF ([edi].StatusStruct.ATLength < 42)
	  mov	esi, [edi].StatusStruct.ATIn
	  mov	byte ptr [esi], dl
	  inc	[edi].StatusStruct.ATLength
	  inc	[edi].StatusStruct.ATIn
	  cmp	[edi].StatusStruct.Command.E, 1
	  jne	Short @F
	  mov	esi, [edi].StatusStruct.RXIn
	  mov	byte ptr [esi], dl
	  inc	[edi].StatusStruct.RXLength
	  inc	esi
	  .IF (esi > [edi].StatusStruct.RXEnd)
	    mov esi, [edi].StatusStruct.RXBuffer
	  .ENDIF
	  mov	[edi].StatusStruct.RXIn, esi
	  @@:
	.ENDIF
	mov	[ebp].Client_AX, 0001h
      .ELSE
	mov	[ebp].Client_AX, 0000h
      .ENDIF
      VMMCall Simulate_Iret
      popad
      ret
    INT140C:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      .IF ([edi].StatusStruct.RXLength == 0)
	mov   [ebp].Client_AX, 0FFFFh
      .ELSE
	mov   ecx, [edi].StatusStruct.RXOut
	mov   dl, byte ptr [ecx]
	mov   [ebp].Client_AH, 00h
	mov   [ebp].Client_AL, dl
      .ENDIF
      VMMCall Simulate_Iret
      popad
      ret
    INT140D:
      VMMCall Begin_Nest_V86_Exec
      mov     [ebp].Client_AH, 01h
      mov     eax, 16h
      VMMCall Exec_Int
      mov     ax, [ebp].Client_Flags
      VMMCall End_Nest_Exec
      .IF (ax & 01000000b)
	mov   [ebp].Client_AX, 0FFFFh
      .ENDIF
      VMMCall Simulate_Iret
      popad
      ret
    INT140E:
      VMMCall Begin_Nest_V86_Exec
      mov     [ebp].Client_AH, 00h
      mov     eax, 16h
      VMMCall Exec_Int
      VMMCall End_Nest_Exec
      VMMCall Simulate_Iret
      popad
      ret
    INT140F:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      ;
      ; NOTE: XON/XOFF software flow control is not supported
      ;       as per AnDan author of the ADF driver
      ;
      VMMCall Simulate_Iret
      popad
      ret
    INT1410:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      .IF ([ebp].Client_AL & 00000001b)
	mov   [edi].StatusStruct.CK_Checking, True
      .ELSE
	mov   [edi].StatusStruct.CK_Checking, False
      .ENDIF
      .IF ([ebp].Client_AL & 00000010b)
	mov   [edi].StatusStruct.Transmitter, False
      .ELSE
	mov   [edi].StatusStruct.Transmitter, True
      .ENDIF
      .IF ([edi].StatusStruct.CK_Checking) &&\
	  ([edi].StatusStruct.CK_Received)
	mov   [edi].StatusStruct.CK_Received, False
	mov   [ebp].Client_AX, 0001h
      .ELSE
	mov   [ebp].Client_AX, 0000h
      .ENDIF
      VMMCall Simulate_Iret
      popad
      ret
    INT1411:
      VMMCall Begin_Nest_V86_Exec
      mov     [ebp].Client_AH, 02h
      mov     [ebp].Client_BX, 0
      mov     eax, 10h
      VMMCall Exec_Int
      VMMCall End_Nest_Exec
      VMMCall Simulate_Iret
      popad
      ret
    INT1412:
      VMMCall Begin_Nest_V86_Exec
      mov     [ebp].Client_AH, 03h
      mov     [ebp].Client_BX, 0
      mov     eax, 10h
      VMMCall Exec_Int
      VMMCall End_Nest_Exec
      VMMCall Simulate_Iret
      popad
      ret
    INT1413:
      VMMCall Begin_Nest_V86_Exec
      mov     eax, 29h
      VMMCall Exec_Int
      VMMCall End_Nest_Exec
      VMMCall Simulate_Iret
      popad
      ret
    INT1414:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      ;
      ; NOTE: DCD Watchdog is not supported
      ;
      VMMCall Simulate_Iret
      popad
      ret
    INT1415:
      VMMCall Begin_Nest_V86_Exec
      mov     [ebp].Client_AH, 0Eh
      mov     [ebp].Client_BX, 0
      mov     eax, 10h
      VMMCall Exec_Int
      VMMCall End_Nest_Exec
      VMMCall Simulate_Iret
      popad
      ret
    INT1416:
      mov     [ebp].Client_AX, 0FFFFh
      VMMCall Simulate_Iret
      popad
      ret
    INT1417:
      mov     eax, 1000
      xor     ecx, ecx
      VMMCall Close_VM
      VMMCall Simulate_Iret
      popad
      ret
    INT1418:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      xor     ecx, ecx
      mov     cx, [edi].StatusStruct.RXLength
      .IF (cx > 0) && ([ebp].Client_CX > 0)
	.IF (cx > [ebp].Client_CX)
	  mov cx, [ebp].Client_CX
	.ENDIF
	mov   [ebp].Client_AX, cx
	Client_Ptr_Flat esi, ES, DI, USES_EAX
	mov   ebx, [edi].StatusStruct.RXOut
	call  _Cli
	@@:
	mov   dl, byte ptr [ebx]
	mov   byte ptr [esi], dl
	dec   [edi].StatusStruct.RXLength
	inc   esi
	inc   ebx
	.IF (ebx > [edi].StatusStruct.RXEnd)
	  mov ebx, [edi].StatusStruct.RXBuffer
	.ENDIF
	loop  Short @B
	mov   [edi].StatusStruct.RXOut, ebx
	call  _Sti
      .ELSE
	mov   [ebp].Client_AX, 0
      .ENDIF
      VMMCall Simulate_Iret
      popad
      ret
    INT1419:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      xor     ecx, ecx
      mov     cx, [edi].StatusStruct.Comport.BufferSize
      sub     cx, [edi].StatusStruct.TXLength
      .IF (cx > 0) && ([ebp].Client_CX > 0)
	.IF (cx > [ebp].Client_CX)
	  mov cx, [ebp].Client_CX
	.ENDIF
	mov   [ebp].Client_AX, cx
	.IF ([edi].StatusStruct.UART.MCR & 00010000b)
	  Client_Ptr_Flat esi, ES, DI, USES_EAX
	  mov	  ebx, [edi].StatusStruct.RXIn
	  call	  _Cli
	  @@:
	  mov	  dl, byte ptr [esi]
	  mov	  byte ptr [ebx], dl
	  inc	  [edi].StatusStruct.RXLength
	  inc	  esi
	  inc	  ebx
	  .IF (ebx > [edi].StatusStruct.RXEnd)
	    mov   ebx, [edi].StatusStruct.RXBuffer
	  .ENDIF
	  loop	  Short @B
	  mov	  [edi].StatusStruct.RXIn, ebx
	  call	  _Sti
	  VMMCall Simulate_Iret
	  popad
	  ret
	.ENDIF
	.IF ([edi].StatusStruct.Online) && (![edi].StatusStruct.CommandMode)
	  Client_Ptr_Flat esi, ES, DI, USES_EAX
	  mov	ebx, [edi].StatusStruct.TXIn
	  call	_Cli
	  INT1419TXLoop:
	  pushd ecx
	  mov	dl, byte ptr [esi]
	  mov	byte ptr [ebx], dl
	  inc	[edi].StatusStruct.TXLength
	  inc	esi
	  inc	ebx
	  .IF (ebx > [edi].StatusStruct.TXEnd)
	    mov ebx, [edi].StatusStruct.TXBuffer
	  .ENDIF
	  .IF ([edi].StatusStruct.PossibleEscape)
	    mov	    esi, [edi].StatusStruct.TimeOut_Handle
	    VMMCall Cancel_Time_Out
	    mov	    [edi].StatusStruct.TimeOut_Handle, 0
	    mov	    [edi].StatusStruct.PossibleEscape, False
	    mov	    [edi].StatusStruct.LastChar, NULL
	    mov	    [edi].StatusStruct.PrevLastChar, NULL
	    cmp	    dl, '+'
	    jne	    Short @F
	    mov	    dl, NULL
	    @@:
	  .ENDIF
	  .IF (dl == '+') && ([edi].StatusStruct.LastChar == '+') &&\
	      ([edi].StatusStruct.PrevLastChar == '+') &&\
	      (ecx == 1)
	    mov	    [edi].StatusStruct.PossibleEscape, True
	    xor	    eax, eax
	    mov	    al, [edi].StatusStruct.Command.S12
	    mov	    ecx, 20
	    mul	    ecx
	    mov	    edx, edi
	    mov	    esi, Offset32 Escape_Time_Out
	    VMMCall Set_Global_Time_Out
	    mov	    [edi].StatusStruct.TimeOut_Handle, esi
	  .ELSE
	    mov cl, [edi].StatusStruct.LastChar
	    mov [edi].StatusStruct.LastChar, dl
	    mov [edi].StatusStruct.PrevLastChar, cl
	  .ENDIF
	  pop	  ecx
	  dec	  ecx
	  cmp	  ecx, 0
	  ja	  INT1419TXLoop
	  mov	  [edi].StatusStruct.TXIn, ebx
	  call	  _Sti
	  VMMCall Simulate_Iret
	  popad
	  ret
	.ENDIF
	Client_Ptr_Flat esi, ES, DI, USES_EAX
	mov   ebx, [edi].StatusStruct.ATIn
	call  _Cli
	INT1419ATLoop:
	mov   dl, byte ptr [esi]
	.IF (dl == LF)
	  nop
	.ELSEIF (dl == CR)
	  cmp	[edi].StatusStruct.ATLength, 1
	  jna	Short @F
	  call	ATCommand
	  jmp	Short ___CalledATCommand
	  @@:
	  mov	edx, [edi].StatusStruct.RXIn
	  mov	byte ptr [edx], CR
	  inc	[edi].StatusStruct.RXLength
	  inc	edx
	  .IF (edx > [edi].StatusStruct.RXEnd)
	    mov edx, [edi].StatusStruct.RXBuffer
	  .ENDIF
	  mov	byte ptr [edx], LF
	  inc	[edi].StatusStruct.RXLength
	  inc	edx
	  .IF (edx > [edi].StatusStruct.RXEnd)
	    mov edx, [edi].StatusStruct.RXBuffer
	  .ENDIF
	  mov	[edi].StatusStruct.RXIn, edx
	  pushd ecx
	  mov	edx, [edi].StatusStruct.ATBuffer
	  mov	ecx, 43
	  @@:
	  mov	byte ptr [edx], NULL
	  inc	edx
	  loop	Short @B
	  pop	ecx
	  mov	[edi].StatusStruct.ATLength, 0
	  mov	edx, [edi].StatusStruct.ATBuffer
	  mov	[edi].StatusStruct.ATIn, edx
	  mov	[edi].StatusStruct.ATOut, edx
	  ___CalledATCommand:
	.ELSEIF (dl == BS)
	  .IF ([edi].StatusStruct.ATLength > 0)
	    mov   edx, [edi].StatusStruct.ATIn
	    dec   edx
	    .IF (edx < [edi].StatusStruct.ATBuffer)
	      mov edx, [edi].StatusStruct.ATBuffer
	    .ENDIF
	    mov   [edi].StatusStruct.ATIn, edx
	    mov   byte ptr [edx], NULL
	    dec   [edi].StatusStruct.ATLength
	    pushd ebx
	    pushd ecx
	    mov   eax, Offset32 szBS
	    mov   ebx, [edi].StatusStruct.RXIn
	    mov   ecx, SizeOf szBS
	    @@:
	    mov   dl, byte ptr [eax]
	    mov   byte ptr [ebx], dl
	    inc   [edi].StatusStruct.RXLength
	    inc   eax
	    inc   ebx
	    .IF (ebx > [edi].StatusStruct.RXEnd)
	      mov ebx, [edi].StatusStruct.RXBuffer
	    .ENDIF
	    loop  Short @B
	    mov	  [edi].StatusStruct.RXIn, ebx
	    pop   ecx
	    pop   ebx
	  .ENDIF
	.ELSEIF ([edi].StatusStruct.ATLength < 42)
	  mov	byte ptr [ebx], dl
	  inc	[edi].StatusStruct.ATIn
	  inc	[edi].StatusStruct.ATLength
	  cmp	[edi].StatusStruct.Command.E, 1
	  jne	Short @F
	  mov	eax, [edi].StatusStruct.RXIn
	  mov	byte ptr [eax], dl
	  inc	[edi].StatusStruct.RXLength
	  inc	eax
	  .IF (eax > [edi].StatusStruct.RXEnd)
	    mov eax, [edi].StatusStruct.RXBuffer
	  .ENDIF
	  mov	[edi].StatusStruct.RXIn, eax
	  @@:
	.ENDIF
	inc   esi
	inc   ebx
	dec   ecx
	cmp   ecx, 0
	ja    INT1419ATLoop
	call  _Sti
      .ELSE
	mov   [ebp].Client_AX, 0
      .ENDIF
      VMMCall Simulate_Iret
      popad
      ret
    INT141A:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      .IF ([ebp].Client_AL == 01h)
	or    [edi].StatusStruct.UART.LCR, 01000000b
	.IF ([edi].StatusStruct.Command._Y == 0)
	  call _Cli
	  mov  [edi].StatusStruct.ATLength, 0
	  mov  ecx, [edi].StatusStruct.ATBuffer
	  mov  [edi].StatusStruct.ATIn, ecx
	  mov  [edi].StatusStruct.ATOut, ecx
	  mov  [edi].StatusStruct.TXLength, 0
	  mov  ecx, [edi].StatusStruct.TXBuffer
	  mov  [edi].StatusStruct.TXIn, ecx
	  mov  [edi].StatusStruct.TXOut, ecx
	  call _Sti
	.ELSEIF ([edi].StatusStruct.Command._Y == 1)
	  call _Cli
	  mov  [edi].StatusStruct.ATLength, 0
	  mov  ecx, [edi].StatusStruct.ATBuffer
	  mov  [edi].StatusStruct.ATIn, ecx
	  mov  [edi].StatusStruct.ATOut, ecx
	  mov  [edi].StatusStruct.TXLength, 0
	  mov  ecx, [edi].StatusStruct.TXBuffer
	  mov  [edi].StatusStruct.TXIn, ecx
	  mov  [edi].StatusStruct.TXOut, ecx
	  call _Sti
	  .IF (ServerLoaded) && ([edi].StatusStruct.Online)
	    xor     eax, eax
	    mov     ax, CM_SEND_REMOTE_BREAK
	    xor     ecx, ecx
	    mov     cl, [edi].StatusStruct.Comport.Node
	    VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	  .ENDIF
	.ELSEIF ([edi].StatusStruct.Command._Y == 2)
	  .IF (ServerLoaded) && ([edi].StatusStruct.Online)
	    xor     eax, eax
	    mov     ax, CM_SEND_REMOTE_BREAK
	    xor     ecx, ecx
	    mov     cl, [edi].StatusStruct.Comport.Node
	    VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	  .ENDIF
	.ELSEIF ([edi].StatusStruct.Command._Y == 3)
	   mov	    [edi].StatusStruct.TimeOut, 0
	   .WHILE ([edi].StatusStruct.TXLength > 0)
	     mov    eax, 1000
	     call   _Delay
	     inc    [edi].StatusStruct.TimeOut
	     .BREAK .IF ([edi].StatusStruct.TimeOut == 3)
	   .ENDW
	  .IF (ServerLoaded) && ([edi].StatusStruct.Online)
	    xor     eax, eax
	    mov     ax, CM_SEND_REMOTE_BREAK
	    xor     ecx, ecx
	    mov     cl, [edi].StatusStruct.Comport.Node
	    VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	  .ENDIF
	.ENDIF
      .ELSEIF ([ebp].Client_AL == 00h)
	and   [edi].StatusStruct.UART.LCR, 10111111b
      .ENDIF
      VMMCall Simulate_Iret
      popad
      ret
    INT141B:
      lea     esi, [edi].StatusStruct.FOSSIL
      mov     [esi].FOSSILStruct.StrSiz, 13h
      mov     [esi].FOSSILStruct.MajVer, 5
      mov     [esi].FOSSILStruct.MinVer, 0
      mov     eax, [V86Allocated]
      mov     [esi].FOSSILStruct.Ident, eax
      add     ebx, [ControlBlock]
      mov     dl, byte ptr [ebx]
      .IF ([ebp].Client_DL == 0FFh) || ([ebp].Client_DL != dl)
	mov   ax, [UndefinedDW]
	mov   [esi].FOSSILStruct.IBufr, ax
	mov   [esi].FOSSILStruct.IFree, ax
	mov   [esi].FOSSILStruct.OBufr, ax
	mov   [esi].FOSSILStruct.OFree, ax
	mov   al, [UndefinedDB]
	mov   [esi].FOSSILStruct.Baud, al
      .ELSE
	xor   ecx, ecx
	mov   cx, [edi].StatusStruct.Comport.BufferSize
	mov   [esi].FOSSILStruct.IBufr, cx
	sub   cx, [edi].StatusStruct.RXLength
	mov   [esi].FOSSILStruct.IFree, cx
	mov   cx, [edi].StatusStruct.Comport.BufferSize
	mov   [esi].FOSSILStruct.OBufr, cx
	sub   cx, [edi].StatusStruct.TXLength
	mov   [esi].FOSSILStruct.OFree, cx
	mov   al, [edi].StatusStruct.FOSSILBaud
	mov   [esi].FOSSILStruct.Baud, al
      .ENDIF
      mov     [esi].FOSSILStruct.SWidth, 80
      mov     [esi].FOSSILStruct.SHeight, 25
      xor     ecx, ecx
      mov     cx, SizeOf FOSSILStruct
      .IF (cx > [ebp].Client_CX)
	mov   cx, [ebp].Client_CX
      .ENDIF
      mov     [ebp].Client_AX, cx
      Client_Ptr_Flat edi, ES, DI, USES_EAX
      rep     movsb
      mov     [ebp].Client_CX, '0X'
      mov     [ebp].Client_DX, ' 0'
      VMMCall Simulate_Iret
      popad
      ret
    INT141C:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      mov     [ebp].Client_AX, 1954h
      mov     [ebp].Client_BX, 0521h
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      mov     ecx, ebx
      add     ecx, [ControlBlock]
      .IF (byte ptr [ecx+1] == True) &&\
	  ([dword ptr [ecx+2]].StatusStruct.Enabled)
	call	_Cli
	mov	[edi].StatusStruct.ATLength, 0
	mov	ecx, [edi].StatusStruct.ATBuffer
	mov	[edi].StatusStruct.ATIn, ecx
	mov	[edi].StatusStruct.ATOut, ecx
	mov	[edi].StatusStruct.TXLength, 0
	mov	ecx, [edi].StatusStruct.TXBuffer
	mov	[edi].StatusStruct.TXIn, ecx
	mov	[edi].StatusStruct.TXOut, ecx
	mov	[edi].StatusStruct.RXLength, 0
	mov	ecx, [edi].StatusStruct.RXBuffer
	mov	[edi].StatusStruct.RXIn, ecx
	mov	[edi].StatusStruct.RXOut, ecx
	call	_Sti
	mov	[edi].StatusStruct.LastTimeslice, 0
	mov	[edi].StatusStruct.Transmitter, True
	mov	[edi].StatusStruct.CK_Checking, False
	mov	[edi].StatusStruct.CK_Received, False
	mov	[ebp].Client_AX, 1954h
	mov	[ebp].Client_BX, 0521h
	VMMCall Simulate_Iret
	popad
	ret
      .ENDIF
      call    IsFOSSILComport
      cmp     al, True
      je      Short @F
      xor     ecx, ecx
      xor     edx, edx
      mov     cx, [Old_Int14Seg]
      mov     dx, [Old_Int14Ofs]
      VMMCall Simulate_Far_Jmp
      popad
      ret
      @@:
      mov     [edi].StatusStruct.VM_Handle, ebx
      add     ebx, [ControlBlock]
      mov     dl, [ebp].Client_DL
      mov     byte ptr [ebx], dl
      mov     byte ptr [ebx+1], True
      mov     dword ptr [ebx+2], edi
      call    _Cli
      mov     [edi].StatusStruct.ATLength, 0
      mov     ecx, [edi].StatusStruct.ATBuffer
      mov     [edi].StatusStruct.ATIn, ecx
      mov     [edi].StatusStruct.ATOut, ecx
      mov     [edi].StatusStruct.TXLength, 0
      mov     ecx, [edi].StatusStruct.TXBuffer
      mov     [edi].StatusStruct.TXIn, ecx
      mov     [edi].StatusStruct.TXOut, ecx
      mov     [edi].StatusStruct.RXLength, 0
      mov     ecx, [edi].StatusStruct.RXBuffer
      mov     [edi].StatusStruct.RXIn, ecx
      mov     [edi].StatusStruct.RXOut, ecx
      call    _Sti
      mov     [edi].StatusStruct.LastTimeslice, 0
      mov     [edi].StatusStruct.Transmitter, True
      mov     [edi].StatusStruct.CK_Checking, False
      mov     [edi].StatusStruct.CK_Received, False
      or      [edi].StatusStruct.UART.MCR, 00000001b
      mov     [ebp].Client_AX, 1954h
      mov     [ebp].Client_BX, 0521h
      VMMCall Simulate_Iret
      popad
      ret
    INT141D:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      add     ebx, [ControlBlock]
      .IF (byte ptr [ebx+1] == True)
	cmp   [edi].StatusStruct.Comport.Alwaysactive, True
	je    Short @F
	mov   byte ptr [ebx], NULL
	mov   byte ptr [ebx+1], False
	@@:
	call  _Cli
	mov   [edi].StatusStruct.ATLength, 0
	mov   ecx, [edi].StatusStruct.ATBuffer
	mov   [edi].StatusStruct.ATIn, ecx
	mov   [edi].StatusStruct.ATOut, ecx
	mov   [edi].StatusStruct.TXLength, 0
	mov   ecx, [edi].StatusStruct.TXBuffer
	mov   [edi].StatusStruct.TXIn, ecx
	mov   [edi].StatusStruct.TXOut, ecx
	mov   [edi].StatusStruct.RXLength, 0
	mov   ecx, [edi].StatusStruct.RXBuffer
	mov   [edi].StatusStruct.RXIn, ecx
	mov   [edi].StatusStruct.RXOut, ecx
	call  _Sti
	mov   [edi].StatusStruct.LastTimeslice, 0
	mov   [edi].StatusStruct.Transmitter, True
	mov   [edi].StatusStruct.CK_Checking, False
	mov   [edi].StatusStruct.CK_Received, False
      .ENDIF
      VMMCall Simulate_Iret
      popad
      ret
    INT141E:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      .IF ([ebp].Client_AL == 01h)
	or    [edi].StatusStruct.UART.LCR, 01000000b
	.IF ([edi].StatusStruct.Command._Y == 0)
	  call _Cli
	  mov  [edi].StatusStruct.ATLength, 0
	  mov  ecx, [edi].StatusStruct.ATBuffer
	  mov  [edi].StatusStruct.ATIn, ecx
	  mov  [edi].StatusStruct.ATOut, ecx
	  mov  [edi].StatusStruct.TXLength, 0
	  mov  ecx, [edi].StatusStruct.TXBuffer
	  mov  [edi].StatusStruct.TXIn, ecx
	  mov  [edi].StatusStruct.TXOut, ecx
	  call _Sti
	.ELSEIF ([edi].StatusStruct.Command._Y == 1)
	  call _Cli
	  mov  [edi].StatusStruct.ATLength, 0
	  mov  ecx, [edi].StatusStruct.ATBuffer
	  mov  [edi].StatusStruct.ATIn, ecx
	  mov  [edi].StatusStruct.ATOut, ecx
	  mov  [edi].StatusStruct.TXLength, 0
	  mov  ecx, [edi].StatusStruct.TXBuffer
	  mov  [edi].StatusStruct.TXIn, ecx
	  mov  [edi].StatusStruct.TXOut, ecx
	  call _Sti
	  .IF (ServerLoaded) && ([edi].StatusStruct.Online)
	    xor     eax, eax
	    mov     ax, CM_SEND_REMOTE_BREAK
	    xor     ecx, ecx
	    mov     cl, [edi].StatusStruct.Comport.Node
	    VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	  .ENDIF
	.ELSEIF ([edi].StatusStruct.Command._Y == 2)
	  .IF (ServerLoaded) && ([edi].StatusStruct.Online)
	    xor     eax, eax
	    mov     ax, CM_SEND_REMOTE_BREAK
	    xor     ecx, ecx
	    mov     cl, [edi].StatusStruct.Comport.Node
	    VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	  .ENDIF
	.ELSEIF ([edi].StatusStruct.Command._Y == 3)
	   mov	    [edi].StatusStruct.TimeOut, 0
	   .WHILE ([edi].StatusStruct.TXLength > 0)
	     mov    eax, 1000
	     call   _Delay
	     inc    [edi].StatusStruct.TimeOut
	     .BREAK .IF ([edi].StatusStruct.TimeOut == 3)
	   .ENDW
	  .IF (ServerLoaded) && ([edi].StatusStruct.Online)
	    xor     eax, eax
	    mov     ax, CM_SEND_REMOTE_BREAK
	    xor     ecx, ecx
	    mov     cl, [edi].StatusStruct.Comport.Node
	    VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	  .ENDIF
	.ENDIF
      .ELSEIF ([ebp].Client_AL == 00h)
	and   [edi].StatusStruct.UART.LCR, 10111111b
      .ENDIF
      xor     eax, eax
      .IF ([edi].StatusStruct.RXLength > 0)
	or    ah, 00000001b
      .ENDIF
      mov     cx, [edi].StatusStruct.Comport.BufferSize
      .IF ([edi].StatusStruct.RXLength > cx)
	or    ah, 00000010b
      .ENDIF
      .IF ([edi].StatusStruct.UART.LSR & 00010000b)
	or    ah, 00010000b
      .ENDIF
      .IF ([edi].StatusStruct.TXLength < cx)
	or    ah, 00100000b
      .ENDIF
      sub     cx, 15
      .IF ([edi].StatusStruct.TXLength < cx)
	or    ah, 01000000b
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 00000001b)
	or    al, 00000001b
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 00000010b)
	or    al, 00000010b
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 00001000b)
	or    al, 00000100b
      .ENDIF
      or      al, 00001000b
      .IF ([edi].StatusStruct.Command._H == 0)
	or    al, 00010000b
      .ELSEIF ([edi].StatusStruct.Command._H == 1) ||\
	      ([edi].StatusStruct.Command._H == 3)
	mov   cx, [edi].StatusStruct.Comport.BufferSize
	.IF ([edi].StatusStruct.TXLength < cx)
	  or  al, 00010000b
	.ENDIF
      .ENDIF
      .IF ([edi].StatusStruct.Command._S == 0)
	or    al, 00100000b
      .ELSE
	.IF ([edi].StatusStruct.UART.MCR & 00000001b)
	  or  al, 00100000b
	.ENDIF
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 01000000b) ||\
	  ([edi].StatusStruct.UART.MSR & 00000100b)
	or    al, 01000000b
      .ENDIF
      .IF ([edi].StatusStruct.Command._C == 1)
	.IF ([edi].StatusStruct.Online)
	  or  al, 10000000b
	.ENDIF
      .ELSE
	or    al, 10000000b
      .ENDIF
      mov     [ebp].Client_AX, ax
      VMMCall Simulate_Iret
      popad
      ret
    INT141F:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      .IF ([ebp].Client_AL == 00h)
	mov   al, [edi].StatusStruct.UART.MCR
	mov   [ebp].Client_BL, al
      .ELSEIF ([ebp].Client_AL == 01h)
	mov   al, [ebp].Client_BL
	mov   [edi].StatusStruct.UART.MCR, al
	.IF !([edi].StatusStruct.UART.MCR & 00000001b) &&\
	    ([edi].StatusStruct.Command._D == 2) &&\
	    (ServerLoaded) && ([edi].StatusStruct.Online)
	  xor	  eax, eax
	  xor	  ecx, ecx
	  mov	  ax, CM_DISCONNECT_NODE
	  mov	  cl, [edi].StatusStruct.Comport.Node
	  VxDCall _SHELL_PostMessage, <ServerWindow, eax, ecx, 0, 0, 0>
	.ENDIF
      .ENDIF
      xor     eax, eax
      .IF ([edi].StatusStruct.RXLength > 0)
	or    ah, 00000001b
      .ENDIF
      mov     cx, [edi].StatusStruct.Comport.BufferSize
      .IF ([edi].StatusStruct.RXLength > cx)
	or    ah, 00000010b
      .ENDIF
      .IF ([edi].StatusStruct.UART.LSR & 00010000b)
	or    ah, 00010000b
      .ENDIF
      .IF ([edi].StatusStruct.TXLength < cx)
	or    ah, 00100000b
      .ENDIF
      sub     cx, 15
      .IF ([edi].StatusStruct.TXLength < cx)
	or    ah, 01000000b
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 00000001b)
	or    al, 00000001b
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 00000010b)
	or    al, 00000010b
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 00001000b)
	or    al, 00000100b
      .ENDIF
      or      al, 00001000b
      .IF ([edi].StatusStruct.Command._H == 0)
	or    al, 00010000b
      .ELSEIF ([edi].StatusStruct.Command._H == 1) ||\
	      ([edi].StatusStruct.Command._H == 3)
	mov   cx, [edi].StatusStruct.Comport.BufferSize
	.IF ([edi].StatusStruct.TXLength < cx)
	  or  al, 00010000b
	.ENDIF
      .ENDIF
      .IF ([edi].StatusStruct.Command._S == 0)
	or    al, 00100000b
      .ELSE
	.IF ([edi].StatusStruct.UART.MCR & 00000001b)
	  or  al, 00100000b
	.ENDIF
      .ENDIF
      .IF ([edi].StatusStruct.UART.MSR & 01000000b) ||\
	  ([edi].StatusStruct.UART.MSR & 00000100b)
	or    al, 01000000b
      .ENDIF
      .IF ([edi].StatusStruct.Command._C == 1)
	.IF ([edi].StatusStruct.Online)
	  or  al, 10000000b
	.ENDIF
      .ELSE
	or    al, 10000000b
      .ENDIF
      mov     [ebp].Client_AX, ax
      VMMCall Simulate_Iret
      popad
      ret
    INT1420:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      call    _Cli
      mov     ecx, [edi].StatusStruct.RXOut
      mov     al, byte ptr [ecx]
      dec     [edi].StatusStruct.RXLength
      inc     ecx
      .IF (ecx > [edi].StatusStruct.RXEnd)
	mov   ecx, [edi].StatusStruct.RXBuffer
      .ENDIF
      mov     [edi].StatusStruct.RXOut, ecx
      call    _Sti
      mov     [ebp].Client_AH, 00h
      mov     [ebp].Client_AL, al
      VMMCall Simulate_Iret
      popad
      ret
    INT1421:
      cmp     [ebp].Client_DL, 0FFh
      jne     Short @F
      nop
      VMMCall Simulate_Iret
      popad
      ret
      @@:
      .IF ([edi].StatusStruct.CK_Checking)
	.IF ([ebp].Client_AL == '') || ([ebp].Client_AL == '')
	  mov	  [edi].StatusStruct.CK_Received, True
	  VMMCall Simulate_Iret
	  popad
	  ret
	.ENDIF
      .ENDIF
      call    _Cli
      mov     ecx, [edi].StatusStruct.RXIn
      mov     al, [ebp].Client_AL
      mov     byte ptr [ecx], al
      inc     [edi].StatusStruct.RXLength
      inc     ecx
      .IF (ecx > [edi].StatusStruct.RXEnd)
	mov   ecx, [edi].StatusStruct.RXBuffer
      .ENDIF
      mov     [edi].StatusStruct.RXIn, ecx
      call    _Sti
      VMMCall Simulate_Iret
      popad
      ret
    INT147E:
      mov     al, [ebp].Client_AL
      mov     [ebp].Client_BL, al
      mov     [ebp].Client_AX, 1954h
      mov     [ebp].Client_BH, 00h
      VMMCall Simulate_Iret
      popad
      ret
    INT147F:
      mov     al, [ebp].Client_AL
      mov     [ebp].Client_BL, al
      mov     [ebp].Client_AX, 1954h
      mov     [ebp].Client_BH, 00h
      VMMCall Simulate_Iret
      popad
      ret
 EndProc Int14_Proc

 BeginProc W32DeviceIoControl, High_Freq
    pushad
    mov       eax, [esi].DIOCParams.dwIoControlCode
    cmp       eax, IOCTL_Max
    ja	      Short @F
    jmp       IOCTL_Table[eax * 4]
    @@:
    popad
    mov       eax, 1
    ret
    IOCTL00:
      popad
      xor     eax, eax
      ret
    IOCTL01:
      mov     edi, [esi].DIOCParams.lpvInBuffer
      mov     ax, Driver_Version
      mov     [edi].DriverInfo.Version, ax
      mov     al, MaxNodes
      mov     [edi].DriverInfo.Max_Nodes, al
      popad
      xor     eax, eax
      ret
    IOCTL02:
      mov	eax, 4
      mov	edi, [esi].DIOCParams.lpcbBytesReturned
      mov	ecx, dword ptr [edi]
      dec	ecx
      mul	ecx
      mov	edi, Offset32 Status_Array
      add	edi, eax
      mov	edi, dword ptr [edi]
      .IF ([edi].StatusStruct.Comport.Enabled)
	cmp	[edi].StatusStruct.SimulateHandle, 0
	je	Short @F
	mov	esi, [edi].StatusStruct.SimulateHandle
	mov	[edi].StatusStruct.SimulateHandle, 0
	VMMCall Cancel_Time_Out
	@@:
	cmp	[edi].StatusStruct.FTSleep_Handle, 0
	je	Short @F
	mov	esi, [edi].StatusStruct.FTSleep_Handle
	mov	[edi].StatusStruct.FTSleep_Handle, 0
	VMMCall Cancel_Time_Out
	@@:
	cmp	[edi].StatusStruct.TimeOut_Handle, 0
	je	Short @F
	mov	esi, [edi].StatusStruct.TimeOut_Handle
	mov	[edi].StatusStruct.TimeOut_Handle, 0
	VMMCall Cancel_Time_Out
	@@:
	.IF ([edi].StatusStruct.ATBuffer != NULL) &&\
	    ([edi].StatusStruct.ATBuffer != 0)
	  mov	  eax, [edi].StatusStruct.ATBuffer
	  VMMCall _HeapFree, <eax, 0>
	  mov	  [edi].StatusStruct.ATBuffer, NULL
	  mov	  [edi].StatusStruct.TXBuffer, NULL
	  mov	  [edi].StatusStruct.RXBuffer, NULL
	  mov	  [edi].StatusStruct.ATLength, 0
	  mov	  [edi].StatusStruct.TXLength, 0
	  mov	  [edi].StatusStruct.RXLength, 0
	.ENDIF
	.IF ([edi].StatusStruct.Comport.Emulation == emUART)
	  mov	  ebx, [edi].StatusStruct.VM_Handle
	  VMMCall Validate_VM_Handle
	  .IF (!CARRY?) && ([edi].StatusStruct.VM_Handle != 0) &&\
	      (IRQ_Handle != 0)
	    @@:
	    mov	    eax, [IRQ_Handle]
	    mov	    ebx, [edi].StatusStruct.VM_Handle
	    VxDCall VPICD_Get_Complete_Status
	    test    ecx, VPICD_STAT_VIRT_DEV_REQ
	    jz	    Short @F
	    VxDCall VPICD_Clear_Int_Request
	    jmp	    Short @B
	    @@:
	  .ENDIF
	  mov ecx, 8
	  .REPEAT
	    dec	    ecx
	    xor	    edx, edx
	    mov	    dx, [edi].StatusStruct.Comport.Baseaddress
	    add	    edx, ecx
	    VMMCall Remove_IO_Handler
	  .UNTIL ecx == 0
	.ELSEIF ([edi].StatusStruct.Comport.Emulation == emFOSSIL)
	  mov [edi].StatusStruct.FOSSIL.StrSiz, FOSSIL_Size
	  mov [edi].StatusStruct.FOSSIL.MajVer, 5
	  mov [edi].StatusStruct.FOSSIL.MinVer, 0
	  mov [edi].StatusStruct.FOSSIL.Ident, -1
	  mov ax, [UndefinedDW]
	  mov [edi].StatusStruct.FOSSIL.IBufr, ax
	  mov [edi].StatusStruct.FOSSIL.IFree, ax
	  mov [edi].StatusStruct.FOSSIL.OBufr, ax
	  mov [edi].StatusStruct.FOSSIL.OFree, ax
	  mov [edi].StatusStruct.FOSSIL.SWidth, 80
	  mov [edi].StatusStruct.FOSSIL.SHeight, 25
	  mov al, [UndefinedDB]
	  mov [edi].StatusStruct.FOSSIL.Baud, al
	.ENDIF
	mov	ebx, [edi].StatusStruct.VM_Handle
	VMMCall Validate_VM_Handle
	.IF (!CARRY?) && ([edi].StatusStruct.VM_Handle != 0)
	  mov ebx, [edi].StatusStruct.VM_Handle
	  add ebx, [ControlBlock]
	  mov byte ptr [ebx], NULL
	  mov byte ptr [ebx+1], False
	  xor ecx, ecx
	  mov dword ptr [ebx+2], ecx
	.ENDIF
	mov   [edi].StatusStruct.UART.RBR, 00000000b
	mov   [edi].StatusStruct.UART.THR, 00000000b
	mov   [edi].StatusStruct.UART.IER, 00000000b
	mov   [edi].StatusStruct.UART.IIR, 00000001b
	mov   [edi].StatusStruct.UART.FCR, 00000000b
	mov   [edi].StatusStruct.UART.LCR, 00000000b
	mov   [edi].StatusStruct.UART.MCR, 00000000b
	mov   [edi].StatusStruct.UART.LSR, 01100000b
	mov   [edi].StatusStruct.UART.MSR, 00110000b
	mov   [edi].StatusStruct.UART.SCR, 00000000b
	mov   [edi].StatusStruct.UART.DLL, 30h
	mov   [edi].StatusStruct.UART.DLM, 00h
	mov   [edi].StatusStruct.Initialized, False
	mov   [edi].StatusStruct.InitError, 21
	mov   [edi].StatusStruct.VM_Handle, 0
	mov   [edi].StatusStruct.Online, False
	mov   [edi].StatusStruct.Ringing, False
	mov   [edi].StatusStruct.Attempting, False
	mov   [edi].StatusStruct.Answered, False
	mov   [edi].StatusStruct.TimeOut, 0
	mov   [edi].StatusStruct.Trigger, 1
	mov   [edi].StatusStruct.Simulate_ELI, False
	mov   [edi].StatusStruct.Simulate_ETI, False
	mov   [edi].StatusStruct.Report_ELI, False
	mov   [edi].StatusStruct.Report_ETI, False
	mov   [edi].StatusStruct.LastChar, NULL
	mov   [edi].StatusStruct.FOSSILBaud, 0
	mov   [edi].StatusStruct.PrevLastChar, NULL
	mov   [edi].StatusStruct.LastTimeSlice, 0
	mov   [edi].StatusStruct.IORecvOK, True
	mov   [edi].StatusStruct.IOSendOK, True
	mov   [edi].StatusStruct.CK_Checking, False
	mov   [edi].StatusStruct.CK_Received, False
	mov   [edi].StatusStruct.Transmitter, True
	mov   [edi].StatusStruct.IRQ_Ready, True
	mov   [edi].StatusStruct.ICT_Ready, True
	mov   [edi].StatusStruct.DriverDisable, False
	mov   [edi].StatusStruct.CommandMode, False
	mov   [edi].StatusStruct.RXOFFReceived, False
	mov   [edi].StatusStruct.PossibleEscape, False
	mov   [edi].StatusStruct.Command.H, 0
	mov   [edi].StatusStruct.Command.E, 0
	mov   [edi].StatusStruct.Command.Q, 0
	mov   [edi].StatusStruct.Command._C, 1
	mov   [edi].StatusStruct.Command._D, 2
	mov   [edi].StatusStruct.Command._H, 0
	mov   [edi].StatusStruct.Command._I, 0
	mov   [edi].StatusStruct.Command._K, 1
	mov   [edi].StatusStruct.Command._R, 2
	mov   [edi].StatusStruct.Command._S, 0
	mov   [edi].StatusStruct.Command._Y, 1
	mov   [edi].StatusStruct.Command.S0, 0
	mov   [edi].StatusStruct.Command.S1, 0
	mov   [edi].StatusStruct.Command.S2, 43
	mov   [edi].StatusStruct.Command.S3, 13
	mov   [edi].StatusStruct.Command.S4, 10
	mov   [edi].StatusStruct.Command.S5, 8
	mov   [edi].StatusStruct.Command.S6, 2
	mov   [edi].StatusStruct.Command.S7, 60
	mov   [edi].StatusStruct.Command.S8, 2
	mov   [edi].StatusStruct.Command.S9, 6
	mov   [edi].StatusStruct.Command.S10, 7
	mov   [edi].StatusStruct.Command.S11, 70
	mov   [edi].StatusStruct.Command.S12, 100
	mov   [edi].StatusStruct.Command.S13, 0
	mov   [edi].StatusStruct.Command.S14, 0
	mov   [edi].StatusStruct.Command.S15, 0
	mov   [edi].StatusStruct.Command.S16, 0
	mov   [edi].StatusStruct.Command.S18, 0
	mov   [edi].StatusStruct.Command.S19, 0
	mov   [edi].StatusStruct.Command.S21, 10
	mov   [edi].StatusStruct.Command.S22, 17
	mov   [edi].StatusStruct.Command.S23, 19
	mov   [edi].StatusStruct.Command.S25, 5
	mov   [edi].StatusStruct.Command.S27, 0
	mov   [edi].StatusStruct.Command.S28, 8
	mov   [edi].StatusStruct.Command.S34, 6
	mov   [edi].StatusStruct.Command.S38, 0
      .ENDIF
      popad
      xor     eax, eax
      ret
    IOCTL03:
      mov     eax, 4
      mov     edi, [esi].DIOCParams.lpcbBytesReturned
      mov     ecx, dword ptr [edi]
      dec     ecx
      mul     ecx
      mov     edi, Offset32 Status_Array
      add     edi, eax
      mov     edi, dword ptr [edi]
      mov     esi, [esi].DIOCParams.lpvInBuffer
      pushd   edi
      lea     edi, [edi].StatusStruct.Comport
      mov     ecx, SizeOf ComportStruct
      rep     movsb
      pop     edi
      mov     al, [edi].StatusStruct.Comport.Node
      mov     [edi].StatusStruct.Node, al
      mov     al, [edi].StatusStruct.Comport.Enabled
      mov     [edi].StatusStruct.Enabled, al
      .IF ([edi].StatusStruct.Comport.Enabled)
	.IF ([edi].StatusStruct.Comport.Emulation == emUART)
	  xor ecx, ecx
	  .REPEAT
	    mov     esi, Offset32 IOHandler
	    xor	    edx, edx
	    mov	    dx, [edi].StatusStruct.Comport.BaseAddress
	    add	    edx, ecx
	    VMMCall Install_IO_Handler
	    .IF (CARRY?)
	      .IF (ecx > 0)
		.REPEAT
		  dec	  edx
		  VMMCall Remove_IO_Handler
		  dec	  ecx
		.UNTIL ecx == 0
	      .ENDIF
	      mov   [edi].StatusStruct.Enabled, False
	      mov   [edi].StatusStruct.Initialized, False
	      mov   [edi].StatusStruct.InitError, PORT_ERROR
	      popad
	      mov   eax, 1
	      ret
	    .ENDIF
	    inc	ecx
	  .UNTIL ecx == 8
	.ENDIF
	xor	eax, eax
	mov	ax, [edi].StatusStruct.Comport.BufferSize
	mov	ebx, eax
	dec	ebx
	mov	ecx, 2
	mul	ecx
	add	eax, 43
	VMMCall _HeapAllocate, <eax, HEAPZEROINIT>
	or	eax, eax
	.IF (ZERO?)
	  .IF ([edi].StatusStruct.Comport.Emulation == emUART)
	    mov ecx, 8
	    .REPEAT
	      dec     ecx
	      xor     edx, edx
	      mov     dx, [edi].StatusStruct.Comport.Baseaddress
	      add     edx, ecx
	      VMMCall Remove_IO_Handler
	    .UNTIL ecx == 0
	  .ENDIF
	  mov	[edi].StatusStruct.ATBuffer, NULL
	  mov	[edi].StatusStruct.Enabled, False
	  mov	[edi].StatusStruct.Initialized, False
	  mov	[edi].StatusStruct.InitError, MEMORY_ERROR
	  popad
	  mov	eax, 1
	  ret
	.ENDIF
	mov   [edi].StatusStruct.ATLength, 0
	mov   [edi].StatusStruct.ATBuffer, eax
	mov   [edi].StatusStruct.ATIn, eax
	mov   [edi].StatusStruct.ATOut, eax
	add   eax, 41
	mov   [edi].StatusStruct.ATEnd, eax
	inc   eax
	inc   eax
	mov   [edi].StatusStruct.TXLength, 0
	mov   [edi].StatusStruct.TXBuffer, eax
	mov   [edi].StatusStruct.TXIn, eax
	mov   [edi].StatusStruct.TXOut, eax
	add   eax, ebx
	mov   [edi].StatusStruct.TXEnd, eax
	inc   eax
	mov   [edi].StatusStruct.RXLength, 0
	mov   [edi].StatusStruct.RXBuffer, eax
	mov   [edi].StatusStruct.RXIn, eax
	mov   [edi].StatusStruct.RXOut, eax
	add   eax, ebx
	mov   [edi].StatusStruct.RXEnd, eax
      .ENDIF
      mov     [edi].StatusStruct.Enabled, True
      mov     [edi].StatusStruct.Initialized, True
      mov     [edi].StatusStruct.InitError, NO_ERROR
      popad
      xor     eax, eax
      ret
    IOCTL04:
      .IF (IRQ_Handle != 0)
	xor   ecx, ecx
	.REPEAT
	  pushd	  ecx
	  mov	  eax, 4
	  mul	  ecx
	  mov	  edi, Offset32 Status_Array
	  add	  edi, eax
	  mov	  edi, dword ptr [edi]
	  mov	  ebx, [edi].StatusStruct.VM_Handle
	  VMMCall Validate_VM_Handle
	  .IF (!CARRY?) && ([edi].StatusStruct.VM_Handle != 0)
	    @@:
	    mov	    eax, [IRQ_Handle]
	    mov	    ebx, [edi].StatusStruct.VM_Handle
	    VxDCall VPICD_Get_Complete_Status
	    test    ecx, VPICD_STAT_VIRT_DEV_REQ
	    jz	    Short @F
	    VxDCall VPICD_Clear_Int_Request
	    jmp	    Short @B
	    @@:
	  .ENDIF
	  pop ecx
	  inc ecx
	.UNTIL ecx == MaxNodes
	mov	eax, [IRQ_Handle]
	VxDCall VPICD_Force_Default_Behavior
	mov	[IRQ_Handle], 0
      .ENDIF
      popad
      xor     eax, eax
      ret
    IOCTL05:
      mov     ecx, [esi].DIOCParams.lpcbBytesReturned
      mov     eax, dword ptr [ecx]
      mov     edi, Offset32 VPICD_Descriptor
      mov     [edi].VPICD_IRQ_Descriptor.VID_IRQ_Number, ax
      xor     eax, eax
      or      eax, VPICD_OPT_CAN_SHARE
      mov     [edi].VPICD_IRQ_Descriptor.VID_Options, ax
      mov     eax, Offset32 VPICD_Hw_Int_Proc
      mov     [edi].VPICD_IRQ_Descriptor.VID_Hw_Int_Proc, eax
      mov     eax, Offset32 VPICD_EOI_Proc
      mov     [edi].VPICD_IRQ_Descriptor.VID_EOI_Proc, eax
      mov     [edi].VPICD_IRQ_Descriptor.VID_IRET_Time_Out, 500
      VxDCall VPICD_Virtualize_IRQ
      .IF (CARRY?)
	mov   [IRQ_Handle], 0
	xor   ecx, ecx
	.REPEAT
	  mov eax, 4
	  mul ecx
	  mov edi, Offset32 Status_Array
	  add edi, eax
	  mov edi, dword ptr [edi]
	  .IF ([edi].StatusStruct.Comport.Enabled) &&\
	      ([edi].StatusStruct.Comport.Emulation == emUART)
	    mov [edi].StatusStruct.Enabled, False
	    mov [edi].StatusStruct.Initialized, False
	    mov [edi].StatusStruct.InitError, IRQ_ERROR
	  .ENDIF
	  inc ecx
	.UNTIL ecx == MaxNodes
	popad
	mov   eax, 1
	ret
      .ELSE
	mov   [IRQ_Handle], eax
	xor   ecx, ecx
	.REPEAT
	  mov eax, 4
	  mul ecx
	  mov edi, Offset32 Status_Array
	  add edi, eax
	  mov edi, dword ptr [edi]
	  .IF ([edi].StatusStruct.Comport.Enabled) &&\
	      ([edi].StatusStruct.Comport.Emulation == emUART)
	    mov [edi].StatusStruct.Enabled, True
	    mov [edi].StatusStruct.Initialized, True
	    mov [edi].StatusStruct.InitError, NO_ERROR
	  .ENDIF
	  inc ecx
	.UNTIL ecx == MaxNodes
      .ENDIF
      popad
      xor     eax, eax
      ret
    IOCTL06:
      mov     [ServerLoaded], True
      popad
      xor     eax, eax
      ret
    IOCTL07:
      mov     [ServerLoaded], False
      popad
      xor     eax, eax
      ret
    IOCTL08:
      mov     edi, [esi].DIOCParams.lpcbBytesReturned
      mov     eax, dword ptr [edi]
      mov     [ServerWindow], eax
      popad
      xor     eax, eax
      ret
    IOCTL09:
      mov     eax, 4
      mov     edi, [esi].DIOCParams.lpcbBytesReturned
      mov     ecx, dword ptr [edi]
      dec     ecx
      mul     ecx
      mov     edi, Offset32 Status_Array
      add     edi, eax
      mov     edi, dword ptr [edi]
      mov     esi, [esi].DIOCParams.lpvInBuffer
      mov     al, [edi].StatusStruct.Initialized
      mov     [esi].InitStruct.Init_OK, al
      mov     al, [edi].StatusStruct.InitError
      mov     [esi].InitStruct.Init_Error, al
      popad
      xor     eax, eax
      ret
    IOCTL0A:
      mov     eax, 4
      mov     edi, [esi].DIOCParams.lpcbBytesReturned
      mov     ecx, dword ptr [edi]
      dec     ecx
      mul     ecx
      mov     edi, Offset32 Status_Array
      add     edi, eax
      mov     edi, dword ptr [edi]
      .IF ([edi].StatusStruct.Comport.Emulation == emUART)
	mov	ebx, [edi].StatusStruct.VM_Handle
	VMMCall Validate_VM_Handle
	.IF (!CARRY?) && ([edi].StatusStruct.VM_Handle != 0) &&\
	    (IRQ_Handle != 0)
	  @@:
	  mov	  eax, [IRQ_Handle]
	  mov	  ebx, [edi].StatusStruct.VM_Handle
	  VxDCall VPICD_Get_Complete_Status
	  test	  ecx, VPICD_STAT_VIRT_DEV_REQ
	  jz	  Short @F
	  VxDCall VPICD_Clear_Int_Request
	  jmp	  Short @B
	  @@:
	.ENDIF
      .ELSEIF ([edi].StatusStruct.Comport.Emulation == emFOSSIL)
	mov   [edi].StatusStruct.FOSSIL.StrSiz, FOSSIL_Size
	mov   [edi].StatusStruct.FOSSIL.MajVer, 5
	mov   [edi].StatusStruct.FOSSIL.MinVer, 0
	mov   [edi].StatusStruct.FOSSIL.Ident, -1
	mov   ax, [UndefinedDW]
	mov   [edi].StatusStruct.FOSSIL.IBufr, ax
	mov   [edi].StatusStruct.FOSSIL.IFree, ax
	mov   [edi].StatusStruct.FOSSIL.OBufr, ax
	mov   [edi].StatusStruct.FOSSIL.OFree, ax
	mov   [edi].StatusStruct.FOSSIL.SWidth, 80
	mov   [edi].StatusStruct.FOSSIL.SHeight, 25
	mov   al, [UndefinedDB]
	mov   [edi].StatusStruct.FOSSIL.Baud, al
      .ENDIF
      mov     ebx, [edi].StatusStruct.VM_Handle
      VMMCall Validate_VM_Handle
      .IF (!CARRY?) && ([edi].StatusStruct.VM_Handle != 0)
	mov   ebx, [edi].StatusStruct.VM_Handle
	add   ebx, [ControlBlock]
	mov   byte ptr [ebx], NULL
	mov   byte ptr [ebx+1], False
	xor   ecx, ecx
	mov   dword ptr [ebx+2], ecx
      .ENDIF
      mov     [edi].StatusStruct.UART.RBR, 00000000b
      mov     [edi].StatusStruct.UART.THR, 00000000b
      mov     [edi].StatusStruct.UART.IER, 00000000b
      mov     [edi].StatusStruct.UART.IIR, 00000001b
      mov     [edi].StatusStruct.UART.FCR, 00000000b
      mov     [edi].StatusStruct.UART.LCR, 00000000b
      mov     [edi].StatusStruct.UART.MCR, 00000000b
      mov     [edi].StatusStruct.UART.LSR, 01100000b
      mov     [edi].StatusStruct.UART.MSR, 00110000b
      mov     [edi].StatusStruct.UART.SCR, 00000000b
      mov     [edi].StatusStruct.UART.DLL, 30h
      mov     [edi].StatusStruct.UART.DLM, 00h
      mov     [edi].StatusStruct.VM_Handle, 0
      mov     [edi].StatusStruct.Online, False
      mov     [edi].StatusStruct.Ringing, False
      mov     [edi].StatusStruct.Attempting, False
      mov     [edi].StatusStruct.Answered, False
      mov     [edi].StatusStruct.TimeOut, 0
      mov     [edi].StatusStruct.Trigger, 1
      mov     [edi].StatusStruct.Simulate_ELI, False
      mov     [edi].StatusStruct.Simulate_ETI, False
      mov     [edi].StatusStruct.Report_ELI, False
      mov     [edi].StatusStruct.Report_ETI, False
      mov     [edi].StatusStruct.LastChar, NULL
      mov     [edi].StatusStruct.FOSSILBaud, 0
      mov     [edi].StatusStruct.PrevLastChar, NULL
      mov     [edi].StatusStruct.LastTimeSlice, 0
      mov     [edi].StatusStruct.IORecvOK, True
      mov     [edi].StatusStruct.IOSendOK, True
      mov     [edi].StatusStruct.CK_Checking, False
      mov     [edi].StatusStruct.CK_Received, False
      mov     [edi].StatusStruct.Transmitter, True
      mov     [edi].StatusStruct.IRQ_Ready, True
      mov     [edi].StatusStruct.ICT_Ready, True
      mov     [edi].StatusStruct.DriverDisable, False
      mov     [edi].StatusStruct.CommandMode, False
      mov     [edi].StatusStruct.RXOFFReceived, False
      mov     [edi].StatusStruct.PossibleEscape, False
      mov     [edi].StatusStruct.Command.H, 0
      mov     [edi].StatusStruct.Command.E, 0
      mov     [edi].StatusStruct.Command.Q, 0
      mov     [edi].StatusStruct.Command._C, 1
      mov     [edi].StatusStruct.Command._D, 2
      mov     [edi].StatusStruct.Command._H, 0
      mov     [edi].StatusStruct.Command._I, 0
      mov     [edi].StatusStruct.Command._K, 1
      mov     [edi].StatusStruct.Command._R, 2
      mov     [edi].StatusStruct.Command._S, 0
      mov     [edi].StatusStruct.Command._Y, 1
      mov     [edi].StatusStruct.Command.S0, 0
      mov     [edi].StatusStruct.Command.S1, 0
      mov     [edi].StatusStruct.Command.S2, 43
      mov     [edi].StatusStruct.Command.S3, 13
      mov     [edi].StatusStruct.Command.S4, 10
      mov     [edi].StatusStruct.Command.S5, 8
      mov     [edi].StatusStruct.Command.S6, 2
      mov     [edi].StatusStruct.Command.S7, 60
      mov     [edi].StatusStruct.Command.S8, 2
      mov     [edi].StatusStruct.Command.S9, 6
      mov     [edi].StatusStruct.Command.S10, 7
      mov     [edi].StatusStruct.Command.S11, 70
      mov     [edi].StatusStruct.Command.S12, 100
      mov     [edi].StatusStruct.Command.S13, 0
      mov     [edi].StatusStruct.Command.S14, 0
      mov     [edi].StatusStruct.Command.S15, 0
      mov     [edi].StatusStruct.Command.S16, 0
      mov     [edi].StatusStruct.Command.S18, 0
      mov     [edi].StatusStruct.Command.S19, 0
      mov     [edi].StatusStruct.Command.S21, 10
      mov     [edi].StatusStruct.Command.S22, 17
      mov     [edi].StatusStruct.Command.S23, 19
      mov     [edi].StatusStruct.Command.S25, 5
      mov     [edi].StatusStruct.Command.S27, 0
      mov     [edi].StatusStruct.Command.S28, 8
      mov     [edi].StatusStruct.Command.S34, 6
      mov     [edi].StatusStruct.Command.S38, 0
      popad
      xor     eax, eax
      ret
    IOCTL0B:
      mov     edi, [esi].DIOCParams.lpcbBytesReturned
      mov     ecx, dword ptr [edi]
      dec     ecx
      mov     eax, 4
      mul     ecx
      mov     edi, Offset32 Status_Array
      add     edi, eax
      mov     edi, dword ptr [edi]
      mov     ebx, [edi].StatusStruct.VM_Handle
      VMMCall Validate_VM_Handle
      .IF (CARRY?) || ([edi].StatusStruct.VM_Handle == 0)
	popad
	mov   eax, 1
	ret
      .ENDIF
      .IF ([edi].StatusStruct.Command.H == 1) ||\
	  ([edi].StatusStruct.Answered) ||\
	  ([edi].StatusStruct.Online) ||\
	  ([edi].StatusStruct.UART.MSR & 01000000b)
	popad
	xor   eax, eax
	ret
      .ENDIF
      mov     [edi].StatusStruct.Ringing, True
      mov     [edi].StatusStruct.Attempting, True
      or      [edi].StatusStruct.UART.MSR, 01000000b
      mov     eax, 1000
      mov     edx, edi
      mov     esi, Offset32 Ringer_Time_Out
      VMMCall Set_Global_Time_Out
      popad
      xor     eax, eax
      ret
    IOCTL0C:
      mov     edi, [esi].DIOCParams.lpcbBytesReturned
      mov     ecx, dword ptr [edi]
      dec     ecx
      mov     eax, 4
      mul     ecx
      mov     edi, Offset32 Status_Array
      add     edi, eax
      mov     edi, dword ptr [edi]
      mov     ebx, [edi].StatusStruct.VM_Handle
      VMMCall Validate_VM_Handle
      .IF (CARRY?) || ([edi].StatusStruct.VM_Handle == 0)
	popad
	mov   eax, 1
	ret
      .ENDIF
      xor     eax, eax
      mov     al, [edi].StatusStruct.Answered
      and     al, [edi].StatusStruct.Online
      mov     ecx, [esi].DIOCParams.lpvOutBuffer
      mov     dword ptr [ecx], eax
      popad
      xor     eax, eax
      ret
    IOCTL0D:
      mov     edi, [esi].DIOCParams.lpcbBytesReturned
      mov     ecx, dword ptr [edi]
      dec     ecx
      mov     eax, 4
      mul     ecx
      mov     edi, Offset32 Status_Array
      add     edi, eax
      mov     edi, dword ptr [edi]
      mov     [edi].StatusStruct.Attempting, False
      .IF ([edi].StatusStruct.Online)
	mov   [edi].StatusStruct.ICT_Ready, False
	mov   [edi].StatusStruct.IORecvOK, False
	mov   [edi].StatusStruct.IOSendOK, False
	mov   [edi].StatusStruct.Online, False
	and   [edi].StatusStruct.UART.MSR, 00111011b
	or    [edi].StatusStruct.UART.MSR, 00001000b
	mov   [edi].StatusStruct.ATLength, 0
	mov   ecx, [edi].StatusStruct.ATBuffer
	mov   [edi].StatusStruct.ATIn, ecx
	mov   [edi].StatusStruct.ATOut, ecx
	mov   [edi].StatusStruct.TXLength, 0
	mov   ecx, [edi].StatusStruct.TXBuffer
	mov   [edi].StatusStruct.TXIn, ecx
	mov   [edi].StatusStruct.TXOut, ecx
	mov   [edi].StatusStruct.RXLength, 0
	mov   ecx, [edi].StatusStruct.RXBuffer
	mov   [edi].StatusStruct.RXIn, ecx
	mov   [edi].StatusStruct.RXOut, ecx
	mov   ebx, Offset32 szNOCARRIER
	mov   edx, [edi].StatusStruct.RXIn
	mov   ecx, SizeOf szNOCARRIER
	@@:
	mov   al, byte ptr [ebx]
	mov   byte ptr [edx], al
	inc   [edi].StatusStruct.RXLength
	inc   ebx
	inc   edx
	.IF (edx > [edi].StatusStruct.RXEnd)
	  mov edx, [edi].StatusStruct.RXBuffer
	.ENDIF
	loop  Short @B
	mov   [edi].StatusStruct.RXIn, edx
	mov   [edi].StatusStruct.IOSendOK, True
	mov   [edi].StatusStruct.IORecvOK, True
	mov   [edi].StatusStruct.ICT_Ready, True
      .ENDIF
      mov     [edi].StatusStruct.CK_Received, False
      mov     [edi].StatusStruct.LastTimeslice, 0
      mov     [edi].StatusStruct.Ringing, False
      mov     [edi].StatusStruct.Answered, False
      mov     [edi].StatusStruct.PossibleEscape, False
      mov     [edi].StatusStruct.CommandMode, False
      mov     [edi].StatusStruct.RXOFFReceived, False
      mov     [edi].StatusStruct.LastChar, NULL
      mov     [edi].StatusStruct.PrevLastChar, NULL
      popad
      xor     eax, eax
      ret
    IOCTL0E:
      mov     eax, 4
      mov     edi, [esi].DIOCParams.lpcbBytesReturned
      mov     ecx, dword ptr [edi]
      dec     ecx
      mul     ecx
      mov     edi, Offset32 Status_Array
      add     edi, eax
      mov     edi, dword ptr [edi]
      cmp     [edi].StatusStruct.Online, True
      je      Short @F
      popad
      mov     eax, 1
      ret
      @@:
      cmp     [edi].StatusStruct.VM_Handle, 0
      jne     Short @F
      popad
      xor     eax, eax
      ret
      @@:
      .IF ([edi].StatusStruct.Comport.Emulation == emUART) &&\
	  ([edi].StatusStruct.SimulateHandle == 0)
	pushad
	mov	eax, 1
	mov	edx, edi
	mov	esi, Offset32 Interrupt_Generator
	VMMCall Set_Global_Time_Out
	mov	[edi].StatusStruct.SimulateHandle, esi
	popad
      .ENDIF
      .IF ([edi].StatusStruct.FTSleep_Handle == 0) &&\
	  ([edi].StatusStruct.Online)
	pushad
	mov	eax, 1
	mov	edx, edi
	mov	esi, Offset32 Sleep_TimeOut
	VMMCall Set_Global_Time_Out
	mov	[edi].StatusStruct.FTSleep_Handle, esi
	popad
      .ENDIF
      mov     esi, [esi].DIOCParams.lpvInBuffer
      mov     [esi].IOStruct.Received, 0
      mov     bx, [edi].StatusStruct.Comport.BufferSize
      sub     bx, [edi].StatusStruct.RXLength
      cmp     bx, 0
      jna     IOCTL_Recv_Done
      mov     eax, [esi].IOStruct.IORXLength
      xor     ecx, ecx
      mov     cx, word ptr [eax]
      cmp     cx, 0
      jna     IOCTL_Recv_Done
      .IF (cx > bx)
	mov   cx, bx
      .ENDIF
      mov     ebx, [esi].IOStruct.RXPointer
      mov     edx, [edi].StatusStruct.RXIn
      @@:
      cmp     [edi].StatusStruct.Online, False
      je      @F
      cmp     [edi].StatusStruct.IORecvOK, False
      je      @F
      cmp     [edi].StatusStruct.CommandMode, True
      je      @F
      test    [edi].StatusStruct.UART.MCR, 00010000b
      jnz     @F
      .IF ([edi].StatusStruct.Comport.Emulation == emUART)
	.IF (([edi].StatusStruct.Command._I == 1) ||\
	    ([edi].StatusStruct.Command._I == 2)) &&\
	    ([edi].StatusStruct.RXOFFReceived)
	  jmp Short @F
	.ENDIF
	.IF ([edi].StatusStruct.Command._R == 2) &&\
	    !([edi].StatusStruct.UART.MCR & 00000010b)
	  jmp Short @F
	.ENDIF
      .ENDIF
      .IF ([edi].StatusStruct.Comport.Emulation == emFOSSIL) &&\
	  ([edi].StatusStruct.CK_Checking) &&\
	  ((byte ptr [ebx] == '') || (byte ptr [ebx] == ''))
	mov   [edi].StatusStruct.CK_Received, True
	mov   eax, [esi].IOStruct.IORXLength
	dec   word ptr [eax]
	inc   [esi].IOStruct.Received
	inc   ebx
	dec   ecx
	cmp   ecx, 0
	ja    @B
	jmp   Short @F
      .ENDIF
      mov     al, byte ptr [ebx]
      mov     byte ptr [edx], al
      mov     eax, [esi].IOStruct.IORXLength
      dec     word ptr [eax]
      inc     [edi].StatusStruct.RXLength
      inc     [esi].IOStruct.Received
      inc     ebx
      inc     edx
      .IF (edx > [edi].StatusStruct.RXEnd)
	mov   edx, [edi].StatusStruct.RXBuffer
      .ENDIF
      dec     ecx
      cmp     ecx, 0
      ja      @B
      @@:
      mov     [edi].StatusStruct.RXIn, edx
      IOCTL_Recv_Done:
      mov     ebx, [edi].StatusStruct.VM_Handle
      VMMCall Wake_Up_VM
      mov     eax, 3
      VMMCall Adjust_Execution_Time
      cmp     [edi].StatusStruct.TXLength, 0
      jna     IOCTL_Send_Done
      mov     bx, [esi].IOStruct.HXFree
      cmp     bx, 0
      jna     IOCTL_Send_Done
      xor     ecx, ecx
      mov     cx, [edi].StatusStruct.TXLength
      .IF (cx > bx)
	mov   cx, bx
      .ENDIF
      mov     ebx, [edi].StatusStruct.TXOut
      mov     edx, [esi].IOStruct.HXPointer
      @@:
      cmp     [edi].StatusStruct.Online, False
      je      Short @F
      cmp     [edi].StatusStruct.IOSendOK, False
      je      Short @F
      cmp     [edi].StatusStruct.CommandMode, True
      je      Short @F
      test    [edi].StatusStruct.UART.LCR, 01000000b
      jnz     Short @F
      test    [edi].StatusStruct.UART.MCR, 00010000b
      jnz     Short @F
      .IF ([edi].StatusStruct.Comport.Emulation == emFOSSIL) &&\
	  (![edi].StatusStruct.Transmitter)
	je    Short @F
      .ENDIF
      mov     al, byte ptr [ebx]
      mov     byte ptr [edx], al
      dec     [edi].StatusStruct.TXLength
      mov     eax, [esi].IOStruct.IOHXLength
      inc     word ptr [eax]
      dec     [esi].IOStruct.HXFree
      inc     ebx
      inc     edx
      .IF (ebx > [edi].StatusStruct.TXEnd)
	mov   ebx, [edi].StatusStruct.TXBuffer
      .ENDIF
      loop    Short @B
      @@:
      mov     [edi].StatusStruct.TXOut, ebx
      IOCTL_Send_Done:
      popad
      xor     eax, eax
      ret
    IOCTL0F:
      mov     eax, 4
      mov     edi, [esi].DIOCParams.lpcbBytesReturned
      mov     ecx, dword ptr [edi]
      dec     ecx
      mul     ecx
      mov     edi, Offset32 Status_Array
      add     edi, eax
      mov     edi, dword ptr [edi]
      cmp     [edi].StatusStruct.Online, True
      je      Short @F
      popad
      mov     eax, 1
      ret
      @@:
      or      [edi].StatusStruct.UART.LSR, 00010000b
      mov     [edi].StatusStruct.Simulate_ELI, True
      xor     eax, eax
      mov     al, [edi].StatusStruct.Command.S21
      mov     ecx, 10
      mul     ecx
      mov     edx, edi
      mov     esi, Offset32 Break_Time_Out
      VMMCall Set_Global_Time_Out
      mov     [edi].StatusStruct.TimeOut_Handle, esi
      popad
      xor     eax, eax
      ret
    IOCTL10:
      mov     eax, 4
      mov     edi, [esi].DIOCParams.lpcbBytesReturned
      mov     ecx, dword ptr [edi]
      dec     ecx
      mul     ecx
      mov     edi, Offset32 Status_Array
      add     edi, eax
      mov     edi, dword ptr [edi]
      mov     esi, [esi].DIOCParams.lpvInBuffer
      .IF ([edi].StatusStruct.UART.LCR & 00000011b)
	mov   dword ptr [esi], 1
      .ELSE
	mov   dword ptr [esi], 0
      .ENDIF
      popad
      xor     eax, eax
      ret
 EndProc W32DeviceIoControl

 BeginProc SysCriticalExit
    pushad
    mov       eax, [ControlBlock]
    VMMCall   _Deallocate_Device_CB_Area, <eax, 0>
    mov       eax, 14h
    xor       ecx, ecx
    xor       edx, edx
    mov       cx, [Old_Int14Seg]
    mov       dx, [Old_Int14Ofs]
    VMMCall   Set_V86_Int_Vector
    mov       eax, 2Fh
    mov       esi, Offset32 Int2F_Proc
    VMMCall   Unhook_V86_Int_Chain
    xor       ecx, ecx
    .REPEAT
      pushd   ecx
      mov     eax, 4
      mul     ecx
      mov     edi, Offset32 Status_Array
      add     edi, eax
      mov     edi, dword ptr [edi]
      .IF ([edi].StatusStruct.Enabled)
	cmp	[edi].StatusStruct.SimulateHandle, 0
	je	Short @F
	mov	esi, [edi].StatusStruct.SimulateHandle
	mov	[edi].StatusStruct.SimulateHandle, 0
	VMMCall Cancel_Time_Out
	@@:
	cmp	[edi].StatusStruct.FTSleep_Handle, 0
	je	Short @F
	mov	esi, [edi].StatusStruct.FTSleep_Handle
	mov	[edi].StatusStruct.FTSleep_Handle, 0
	VMMCall Cancel_Time_Out
	@@:
	cmp	[edi].StatusStruct.TimeOut_Handle, 0
	je	Short @F
	mov	esi, [edi].StatusStruct.TimeOut_Handle
	mov	[edi].StatusStruct.TimeOut_Handle, 0
	VMMCall Cancel_Time_Out
	@@:
	.IF ([edi].StatusStruct.ATBuffer != NULL) &&\
	    ([edi].StatusStruct.ATBuffer != 0)
	  mov	  eax, [edi].StatusStruct.ATBuffer
	  VMMCall _HeapFree, <eax, 0>
	.ENDIF
	.IF ([edi].StatusStruct.Comport.Emulation == emUART)
	  @@:
	  mov	  eax, [IRQ_Handle]
	  mov	  ebx, [edi].StatusStruct.VM_Handle
	  VxDCall VPICD_Get_Complete_Status
	  test	  ecx, VPICD_STAT_VIRT_DEV_REQ
	  jz	  Short @F
	  VxDCall VPICD_Clear_Int_Request
	  jmp	  Short @B
	  @@:
	  mov	  ecx, 8
	  .REPEAT
            xor     edx, edx
	    mov	    dx, [edi].StatusStruct.Comport.BaseAddress
	    add	    edx, ecx
	    dec	    edx
	    VMMCall Remove_IO_Handler
	    dec	    ecx
	  .UNTIL ecx == 0
	.ENDIF
      .ENDIF
      mov     [edi].StatusStruct.VM_Handle, 0
      pop     ecx
      inc     ecx
    .UNTIL ecx == MaxNodes
    cmp       [IRQ_Handle], 0
    je	      Short @F
    mov       eax, [IRQ_Handle]
    VxDCall   VPICD_Force_Default_Behavior
    mov       [IRQ_Handle], 0
    @@:
    mov       [IRQ_Number], 0
    .IF (StatusOffset != NULL) && (StatusOffset != 0)
      mov     eax, [StatusOffset]
      VMMCall _HeapFree, <eax, 0>
    .ENDIF
    popad
    clc
    ret
 EndProc SysCriticalExit

 BeginProc SystemExit
    pushd     esi
    mov       esi, [PortListHndl]
    VMMCall   List_Destroy
    pop       esi
    clc
    ret
 EndProc SystemExit

 BeginProc GetStatusStructAddressEx, High_Freq, PUBLIC
    pushd     eax
    pushd     ecx
    xor       ecx, ecx
    @@:
    mov       eax, 4
    mul       ecx
    mov       edi, Offset32 Status_Array
    add       edi, eax
    mov       edi, dword ptr [edi]
    lea       eax, [edi].StatusStruct.PortInfo
    .IF (esi == eax)
      pop     ecx
      pop     eax
      ret
    .ENDIF
    inc       ecx
    cmp       ecx, MaxNodes
    jne       Short @B
    mov       edi, 0
    pop       ecx
    pop       eax
    ret
 EndProc GetStatusStructAddressEx

 BeginProc VMNotExecuteable, High_Freq
    pushad
    xor       ecx, ecx
    .REPEAT
      pushd   ecx
      mov     eax, 4
      mul     ecx
      mov     edi, Offset32 Status_Array
      add     edi, eax
      mov     edi, dword ptr [edi]
      cmp     [edi].StatusStruct.VM_Handle, ebx
      jne     @F
      mov     [edi].StatusStruct.VM_Handle, 0
      .IF ([edi].StatusStruct.VCOMMOpened) &&\
	  ([edi].StatusStruct.ATBuffer != NULL) &&\
	  ([edi].StatusStruct.ATBuffer != 0)
	;
	; Move data in buffers to new TX/RX buffers
	;
	mov	eax, [edi].StatusStruct.ATBuffer
	VMMCall _HeapFree, <eax, 0>
	xor	eax, eax
	mov	ax, [edi].StatusStruct.Comport.BufferSize
	mov	ebx, eax
	dec	ebx
	mov	ecx, 2
	mul	ecx
	add	eax, 43
	VMMCall _HeapAllocate, <eax, HEAPZEROINIT>
	mov	[edi].StatusStruct.ATLength, 0
	mov	[edi].StatusStruct.ATBuffer, eax
	mov	[edi].StatusStruct.ATIn, eax
	mov	[edi].StatusStruct.ATOut, eax
	add	eax, 41
	mov	[edi].StatusStruct.ATEnd, eax
	inc	eax
	inc	eax
	mov	[edi].StatusStruct.TXLength, 0
	mov	[edi].StatusStruct.TXBuffer, eax
	mov	[edi].StatusStruct.TXIn, eax
	mov	[edi].StatusStruct.TXOut, eax
	add	eax, ebx
	mov	[edi].StatusStruct.TXEnd, eax
	inc	eax
	mov	[edi].StatusStruct.RXLength, 0
	mov	[edi].StatusStruct.RXBuffer, eax
	mov	[edi].StatusStruct.RXIn, eax
	mov	[edi].StatusStruct.RXOut, eax
	add	eax, ebx
	mov	[edi].StatusStruct.RXEnd, eax
      .ENDIF
      pop     ecx
      .BREAK
      @@:
      pop     ecx
      inc     ecx
    .UNTIL ecx == MaxNodes
    popad
    clc
    ret
 EndProc VMNotExecuteable






VxD_LOCKED_CODE_ENDS

VxD_PAGEABLE_CODE_SEG

 BeginProc IsPortVCOMM_Added, PUBLIC
    mov       esi, [PortListHndl]
    VMMCall   List_Get_First
    jnz       Short @F
    mov       eax, False
    ret
    @@:
    .REPEAT
      pushd   eax
      mov     edi, dword ptr [eax]
      lea     edi, [edi].StatusStruct.Comport.szComportName
      VMMCall _lstrcmpi, <ebx, edi>
      cmp     eax, 0
      jne     Short @F
      pop     eax
      mov     eax, True
      ret
      @@:
      pop     eax
      VMMCall List_Get_Next
    .UNTIL (ZERO?)
    mov       eax, False
    ret
 EndProc IsPortVCOMM_Added


















 BeginProc S_DriverControl, CCALL, esp, PUBLIC

   ArgVar DriverFunction, DWORD
   ArgVar hDevNode, DWORD
   ArgVar ReferenceData, DWORD

     EnterProc
       mov    eax, DriverFunction
       or     eax, eax
       jz     _InitFunction
     LeaveProc
     Return

 EndProc S_DriverControl

 BeginProc InitFunction, CCALL, esp, PUBLIC

   ArgVar DriverFunction, DWORD
   ArgVar hDevNode, DWORD
   ArgVar ReferenceData, DWORD
   ArgVar AllocIOBase, DWORD
   ArgVar AllocIRQ, DWORD
   ArgVar PortName, DWORD

     EnterProc
       SaveReg <ebx, ecx, edx, esi, edi>
       mov     eax, LF_Alloc_Error
       mov     ecx, 4
       VMMCall List_Create
       mov     [PortListHndl], esi
       xor     ecx, ecx
       .REPEAT
	 SaveReg <ecx>
	 mov  eax, 4
	 mul  ecx
	 mov  edi, Offset32 Status_Array
	 add  edi, eax
	 mov  edi, dword ptr [edi]
	 .IF ([edi].StatusStruct.Enabled) &&\
	     ([edi].StatusStruct.Comport.Emulation == emUART)

	COMMENT *
	   lea eax, [edi].StatusStruct.PortInfo.pPORTFUNCTIONS
	   mov [eax]._PORTFUNCTIONS.pPortSetState, Offset32 PortSetState
	   mov [eax]._PORTFUNCTIONS.pPortGetState, Offset32 PortGetState
	   mov [eax]._PORTFUNCTIONS.pPortSetup, Offset32 PortSetup
	   mov [eax]._PORTFUNCTIONS.pPortTransmitChar, Offset32 PortTramsmitChar
	   mov [eax]._PORTFUNCTIONS.pPortClose, Offset32 PostClose
	   mov [eax]._PORTFUNCTIONS.pPortGetQueueStatus, Offset32 PortGetQueueStatus
	   mov [eax]._PORTFUNCTIONS.pPortClearError, Offset32 PortClearError
	   mov [eax]._PORTFUNCTIONS.pPortSetModemStatusShadow, Offset32 PortSetModemStatusShadow
	   mov [eax]._PORTFUNCTIONS.pPortGetProperties, Offset32 PortGetProperties
	   mov [eax]._PORTFUNCTIONS.pPortEscapeFunction, Offset32 PortEscapeFunction
	   mov [eax]._PORTFUNCTIONS.pPortPurge, Offset32 PortPurge
	   mov [eax]._PORTFUNCTIONS.pPortSetEventMask, Offset32 PortSetEventMask
	   mov [eax]._PORTFUNCTIONS.pPortGetEventMask, Offset32 PortGetEventMask
	   mov [eax]._PORTFUNCTIONS.pPortWrite, Offset32 PortWrite
	   mov [eax]._PORTFUNCTIONS.pPortRead, Offset32 PortRead
	   mov [eax]._PORTFUNCTIONS.pPortEnableNotification, Offset32 PortEnableNotification
	   mov [eax]._PORTFUNCTIONS.pPortSetReadCallback, Offset32 PortSetReadCallback
	   mov [eax]._PORTFUNCTIONS.pPortSetWriteCallback, Offset32 PortSetWriteCallback
	   mov [eax]._PORTFUNCTIONS.pPortGetModemStatus, Offset32 PortGetModemStatus
	   mov [eax]._PORTFUNCTIONS.pPortGetCommConfig, Offset32 PortGetCommConfig
	   mov [eax]._PORTFUNCTIONS.pPortSetCommConfig, Offset32 PortSetCommConfig
	   mov [eax]._PORTFUNCTIONS.pPortGetWin32Error, Offset32 PortGetWin32Error
		*

	   SaveReg <edi>
	   lea edi, [edi].StatusStruct.PortInfo.pPORTDATA
	   mov [edi]._PORTDATA.PDLength, SizeOf _PORTDATA
	   mov [edi]._PORTDATA.PDFunctions, eax
	   mov [edi]._PORTDATA.PDNumFunctions, (SizeOf _PORTFUNCTIONS) /4
	   mov [edi]._PORTDATA.dwLastError, 0
	   mov [edi]._PORTDATA.dwClientEventMask, 0
	   mov [edi]._PORTDATA.lpClientEventNotify, NULL
	   mov [edi]._PORTDATA.lpClientReadNotify, NULL
	   mov [edi]._PORTDATA.lpClientWriteNotify, NULL
	   mov [edi]._PORTDATA.dwClientRefData, 0
	   mov [edi]._PORTDATA.dwDetectedEvents, 0
	   mov [edi]._PORTDATA.dwCommError, 0
	   mov [edi]._PORTDATA.bMSRShadow, 0
	   lea eax, [edi]._PORTDATA.cmto
	   mov [eax].COMMTIMEOUTS.ReadIntervalTimeout, MAXDWORD
	   mov [eax].COMMTIMEOUTS.ReadTotalTimeoutMultiplier, 0
	   mov [eax].COMMTIMEOUTS.ReadTotalTimeoutConstant, 0
	   mov [eax].COMMTIMEOUTS.WriteTotalTimeoutMultiplier, 0
	   mov [eax].COMMTIMEOUTS.WriteTotalTimeoutConstant, 0
	   mov [edi]._PORTDATA.dwLastReceiveTime, 0
	   RestoreReg <edi>
	   lea	   eax, [edi].StatusStruct.Comport.szComportName
	   VxDCall _VCOMM_Add_Port, <ReferenceData, Offset32 _PortOpen, eax>
	   or	   eax, eax
	   .IF (ZERO?)
	     mov   [edi].StatusStruct.Enabled, False
	     mov   [edi].StatusStruct.Initialized, False
	     mov   [edi].StatusStruct.InitError, DRV_REG_ERROR
	   .ELSE
	     mov     esi, [PortListHndl]
	     VMMCall List_Allocate
	     mov     dword ptr [eax], edi
	     VMMCall List_Attach
	     xor     eax, eax
	     mov     ax, [edi].StatusStruct.Comport.BufferSize
	     SaveReg <edi>
	     lea     edi, [edi].StatusStruct.PortInfo.COMMPROP
	     mov     [edi]._COMMPROP.wPacketLength, SizeOf _COMMPROP
	     mov     [edi]._COMMPROP.dwServiceMask, SP_SERIALCOMM
	     mov     [edi]._COMMPROP.dwMaxTxQueue, 8192
	     mov     [edi]._COMMPROP.dwMaxRxQueue, 8192
	     mov     [edi]._COMMPROP.dwMaxBaud, 115200
	     mov     [edi]._COMMPROP.dwProvSubType, PST_RS232
	     mov     ecx, (PCF_RTSCTS Or PCF_PARITY_CHECK Or PCF_XONXOFF)
	     mov     [edi]._COMMPROP.dwProvCapabilities, ecx
	     mov     ecx, (SP_PARITY Or SP_BAUD Or SP_DATABITS Or SP_STOPBITS Or SP_HANDSHAKING Or SP_PARITY_CHECK Or SP_RLSD)
	     mov     [edi]._COMMPROP.dwSettableParams, ecx
	     mov     ecx, (BAUD_300 Or BAUD_1200 Or BAUD_2400 Or BAUD_9600 Or BAUD_14400 Or BAUD_19200 Or BAUD_38400 Or BAUD_56K Or BAUD_128K)
	     mov     [edi]._COMMPROP.dwSettableBaud, ecx
	     mov     ecx, (DATABITS_7 Or DATABITS_8)
	     mov     [edi]._COMMPROP.wSettableData, cx
	     mov     ecx, (STOPBITS_10 Or STOPBITS_15 Or STOPBITS_20 Or PARITY_NONE Or PARITY_ODD Or PARITY_EVEN Or PARITY_MARK Or PARITY_SPACE)
	     mov     [edi]._COMMPROP.wSettableStopParity, cx
	     mov     [edi]._COMMPROP.dwCurrentTxQueue, eax
	     mov     [edi]._COMMPROP.dwCurrentRxQueue, eax
	     RestoreReg <edi>
	     SaveReg <edi>
	     lea     edi, [edi].StatusStruct.PortInfo.COMSTAT
	     mov     [edi]._COMSTAT.BitMask, 0
	     mov     [edi]._COMSTAT.cbInque, 0
	     mov     [edi]._COMSTAT.cbOutque, 0
	     RestoreReg <edi>
	     SaveReg <edi>
	     lea     edi, [edi].StatusStruct.PortInfo.DCB
	     mov     [edi]._DCB.XonChar, XON
	     mov     [edi]._DCB.XoffChar, XOFF
	     RestoreReg <edi>
	     mov     eax, [edi].StatusStruct.RXOut
	     SaveReg <edi>
	     lea     edi, [edi].StatusStruct.PortInfo.QSB
	     mov     [edi]._QSB.ReceiveQueueGetPtr, eax
	     mov     [edi]._QSB.ReceiveQueueCount, 0
	     RestoreReg <edi>
	   .ENDIF
	 .ENDIF
	 RestoreReg <ecx>
	 inc	ecx
       .UNTIL ecx == MaxNodes
       RestoreReg <edi, esi, edx, ecx, ebx>
       mov    eax, True
     LeaveProc
     Return

 EndProc InitFunction

 BeginProc PortOpen, CCALL, PUBLIC

   ArgVar PortName, DWORD
   ArgVar VMId, DWORD
   ArgVar lpError, DWORD

     EnterProc
       cmp    PortName, NULL
       je     @F
       pushd  ebx
       pushd  ecx
       pushd  edx
       pushd  edi
       xor    ecx, ecx
       .REPEAT
	 pushd	 ecx
	 mov	 eax, 4
	 mul	 ecx
	 mov	 edi, Offset32 Status_Array
	 add	 edi, eax
	 mov	 edi, dword ptr [edi]
	 mov	 ebx, PortName
	 lea	 edx, [edi].StatusStruct.Comport.szComportName
	 VMMCall _lstrcmpi, <ebx, edx>
	 cmp	 eax, 0
	 jne	 No_Match
	 .IF ([edi].StatusStruct.Enabled) &&\
	     ([edi].StatusStruct.Comport.Emulation == emUART)
	   mov	   ebx, [edi].StatusStruct.VM_Handle
	   VMMCall Validate_VM_Handle
	   .IF (CARRY?) || ([edi].StatusStruct.VM_Handle == 0)
	     mov ebx, VMId
	     mov [edi].StatusStruct.VM_Handle, ebx
	     mov [edi].StatusStruct.VCOMMOpened, True




	  COMMENT *
	     lea eax, [edi].StatusStruct.PortInfo.pPORTDATA
	     mov ecx, [edi].StatusStruct.RXBuffer
	     mov [eax]._PORTDATA.QInAddr, ecx
	     xor ecx, ecx
	     mov cx, [edi].StatusStruct.Comport.BufferSize
	     mov [eax]._PORTDATA.QInSize, ecx
	     mov ecx, [edi].StatusStruct.TXBuffer
	     mov [eax]._PORTDATA.QOutAddr, ecx
	     xor ecx, ecx
	     mov cx, [edi].StatusStruct.Comport.BufferSize
	     mov [eax]._PORTDATA.QOutSize, cx




	     mov [eax]._PORTDATA.QInCount,
	     mov [eax]._PORTDATA.QInGet,
	     mov [eax]._PORTDATA.QInPut,
	     mov [eax]._PORTDATA.QOutCount,
	     mov [eax]._PORTDATA.QOutGet,
	     mov [eax]._PORTDATA.QOutPut,

		   *






	     xor ecx, ecx
	     dec ecx
	     lea eax, [edi].StatusStruct.PortInfo.DCB
	     mov [eax]._DCB.BaudRate, 0
	     mov [eax]._DCB.BitMask, fBinary
	     mov [eax]._DCB.XonLim, ecx
	     mov [eax]._DCB.XoffLim, ecx
	     mov [eax]._DCB.ByteSize, 8
	     lea eax, [edi].StatusStruct.PortInfo.pPORTDATA.dwLastReceiveTime
	     mov [edi].StatusStruct.RXTimeAddr, eax
	     lea eax, [edi].StatusStruct.PortInfo.pPORTDATA.dwDetectedEvents
	     mov [edi].StatusStruct.dwEventAddr, eax
	     lea eax, [edi].StatusStruct.PortInfo.pPORTDATA.bMSRShadow
	     mov [edi].StatusStruct.bMSRShdwAddr, eax
	     lea eax, [edi].StatusStruct.PortInfo.pPORTDATA
	     mov [eax]._PORTDATA.dwLastError, 0
	     lea eax, [edi].StatusStruct.PortInfo
	     mov ebx, lpError
	     mov dword ptr [ebx], 0
	     pop ecx
	     .BREAK
	   .ELSE
	     lea eax, [edi].StatusStruct.PortInfo.pPORTDATA
	     mov [eax]._PORTDATA.dwLastError, IE_OPEN
	     mov eax, 0
	     mov ebx, lpError
	     mov dword ptr [ebx], IE_OPEN
	     pop ecx
	     .BREAK
	   .ENDIF
	 .ENDIF
	 No_Match:
	 mov  eax, 0
	 mov  ebx, lpError
	 mov  dword ptr [ebx], IE_HARDWARE
	 pop  ecx
	 inc  ecx
       .UNTIL ecx == MaxNodes
       pop    edi
       pop    edx
       pop    ecx
       pop    ebx
       @@:
     LeaveProc
     Return

 EndProc PortOpen

 BeginProc PortClose, CCALL, PUBLIC

   ArgVar hPort, DWORD

     EnterProc
       pushd  ebx
       pushd  ecx
       pushd  edx
       pushd  esi
       pushd  edi
       mov    esi, hPort
       call   GetStatusStructAddressEx
       .IF (edi != 0) && ([edi].StatusStruct.VCOMMOpened)
	 mov  [edi].StatusStruct.VCOMMOpened, False
	 mov  [edi].StatusStruct.VM_Handle, 0






	 .IF ([edi].StatusStruct.ATBuffer != NULL) &&\
	     ([edi].StatusStruct.ATBuffer != 0)
	   mov	   eax, [edi].StatusStruct.ATBuffer
	   VMMCall _HeapFree, <eax, 0>
	   xor	   eax, eax
	   mov	   ax, [edi].StatusStruct.Comport.BufferSize
	   mov	   ebx, eax
	   dec	   ebx
	   mov	   ecx, 2
	   mul	   ecx
	   add	   eax, 43
	   VMMCall _HeapAllocate, <eax, HEAPZEROINIT>
	   mov	   [edi].StatusStruct.ATLength, 0
	   mov	   [edi].StatusStruct.ATBuffer, eax
	   mov	   [edi].StatusStruct.ATIn, eax
	   mov	   [edi].StatusStruct.ATOut, eax
	   add	   eax, 41
	   mov	   [edi].StatusStruct.ATEnd, eax
	   inc	   eax
	   inc	   eax
	   mov	   [edi].StatusStruct.TXLength, 0
	   mov	   [edi].StatusStruct.TXBuffer, eax
	   mov	   [edi].StatusStruct.TXIn, eax
	   mov	   [edi].StatusStruct.TXOut, eax
	   add	   eax, ebx
	   mov	   [edi].StatusStruct.TXEnd, eax
	   inc	   eax
	   mov	   [edi].StatusStruct.RXLength, 0
	   mov	   [edi].StatusStruct.RXBuffer, eax
	   mov	   [edi].StatusStruct.RXIn, eax
	   mov	   [edi].StatusStruct.RXOut, eax
	   add	   eax, ebx
	   mov	   [edi].StatusStruct.RXEnd, eax
	 .ENDIF




	 mov  eax, True
       .ELSE
	 mov  eax, False
       .ENDIF
       pop    edi
       pop    esi
       pop    edx
       pop    ecx
       pop    ebx
     LeaveProc
     Return

 EndProc PortClose
VxD_PAGEABLE_CODE_ENDS

VxD_LOCKED_CODE_SEG

VxD_LOCKED_CODE_ENDS
;****************************************************************************
;			   REAL MODE INIT SEGMENT!
;****************************************************************************
VxD_REAL_INIT_SEG
 BeginProc Real_Mode_Proc
    cmp       bx, Duplicate_Device_ID
    je        Short @F
    xor       bx, bx
    xor       edx, edx
    xor       si, si
    mov       ax, Device_Load_OK
    ret
    @@:
    mov       ax, Abort_Device_Load + No_Fail_Message
    ret
 EndProc Real_Mode_Proc
VxD_REAL_INIT_ENDS
;****************************************************************************
END
