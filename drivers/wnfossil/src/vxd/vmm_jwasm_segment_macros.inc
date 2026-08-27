;****************************************************************************
;                                                                           *
; THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY     *
; KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE       *
; IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR     *
; PURPOSE.                                                                  *
;                                                                           *
; Copyright (C) 1993-95  Microsoft Corporation.  All Rights Reserved.       *
;                                                                           *
;****************************************************************************

ifndef _VMM_
_VMM_	EQU	1
FALSE	EQU	0
VMM_TRUE	EQU	(NOT FALSE)
DEBLEVELRETAIL	EQU	0
DEBLEVELNORMAL	EQU	1
DEBLEVELMAX	EQU	2
ifndef DEBLEVEL
ifdef DEBUG
DEBLEVEL	EQU	DEBLEVELNORMAL
else
DEBLEVEL	EQU	DEBLEVELRETAIL
endif
endif
ifndef WIN31COMPAT
WIN40SERVICES	EQU	1
endif
ifdef MASM6
ifndef NO_MASM6_OPTIONS



    option oldmacros
ifndef  NEWSTRUCTS
    option oldstructs
endif
    option noscoped
    option segment:flat
    option offset:flat
    option proc:private
endif
endif




IFDEF MASM6
BeginDoc MACRO
     ENDM
EndDoc MACRO
       ENDM

BeginMsg MACRO
     ENDM
EndMsg MACRO
       ENDM
ELSE
BeginDoc EQU <>
EndDoc EQU <>

BeginMsg EQU <>
EndMsg EQU <>
ENDIF

UNDEFINED_DEVICE_ID	EQU	00000H
VMM_DEVICE_ID	EQU	00001H
DEBUG_DEVICE_ID	EQU	00002H
VPICD_DEVICE_ID	EQU	00003H
VDMAD_DEVICE_ID	EQU	00004H
VTD_DEVICE_ID	EQU	00005H
V86MMGR_DEVICE_ID	EQU	00006H
PAGESWAP_DEVICE_ID	EQU	00007H
PARITY_DEVICE_ID	EQU	00008H
REBOOT_DEVICE_ID	EQU	00009H
VDD_DEVICE_ID	EQU	0000AH
VSD_DEVICE_ID	EQU	0000BH
VMD_DEVICE_ID	EQU	0000CH
VKD_DEVICE_ID	EQU	0000DH
VCD_DEVICE_ID	EQU	0000EH
VPD_DEVICE_ID	EQU	0000FH
BLOCKDEV_DEVICE_ID	EQU	00010H
VMCPD_DEVICE_ID	EQU	00011H
EBIOS_DEVICE_ID	EQU	00012H
BIOSXLAT_DEVICE_ID	EQU	00013H
VNETBIOS_DEVICE_ID	EQU	00014H
DOSMGR_DEVICE_ID	EQU	00015H
WINLOAD_DEVICE_ID	EQU	00016H
SHELL_DEVICE_ID	EQU	00017H
VMPOLL_DEVICE_ID	EQU	00018H
VPROD_DEVICE_ID	EQU	00019H
DOSNET_DEVICE_ID	EQU	0001AH
VFD_DEVICE_ID	EQU	0001BH
VDD2_DEVICE_ID	EQU	0001CH
WINDEBUG_DEVICE_ID	EQU	0001DH
TSRLOAD_DEVICE_ID	EQU	0001EH
BIOSHOOK_DEVICE_ID	EQU	0001FH
INT13_DEVICE_ID	EQU	00020H
PAGEFILE_DEVICE_ID	EQU	00021H
SCSI_DEVICE_ID	EQU	00022H
MCA_POS_DEVICE_ID	EQU	00023H
SCSIFD_DEVICE_ID	EQU	00024H
VPEND_DEVICE_ID	EQU	00025H
APM_DEVICE_ID	EQU	00026H
VPOWERD_DEVICE_ID	EQU	APM_DEVICE_ID
VXDLDR_DEVICE_ID	EQU	00027H
NDIS_DEVICE_ID	EQU	00028H
BIOS_EXT_DEVICE_ID	EQU	00029H
VWIN32_DEVICE_ID	EQU	0002AH
VCOMM_DEVICE_ID	EQU	0002BH
SPOOLER_DEVICE_ID	EQU	0002CH
WIN32S_DEVICE_ID	EQU	0002DH
DEBUGCMD_DEVICE_ID	EQU	0002EH
CONFIGMG_DEVICE_ID	EQU	00033H
DWCFGMG_DEVICE_ID	EQU	00034H
SCSIPORT_DEVICE_ID	EQU	00035H
VFBACKUP_DEVICE_ID	EQU	00036H
ENABLE_DEVICE_ID	EQU	00037H
VCOND_DEVICE_ID	EQU	00038H
ISAPNP_DEVICE_ID	EQU	0003CH
BIOS_DEVICE_ID	EQU	0003DH
IFSMgr_Device_ID	EQU	00040H
VCDFSD_DEVICE_ID	EQU	00041H
MRCI2_DEVICE_ID	EQU	00042H
PCI_DEVICE_ID	EQU	00043H
PELOADER_DEVICE_ID	EQU	00044H
EISA_DEVICE_ID	EQU	00045H
DRAGCLI_DEVICE_ID	EQU	00046H
DRAGSRV_DEVICE_ID	EQU	00047H
PERF_DEVICE_ID	EQU	00048H
AWREDIR_DEVICE_ID	EQU	00049H
ETEN_Device_ID	EQU	00060H
CHBIOS_Device_ID	EQU	00061H
VMSGD_Device_ID	EQU	00062H
VPPID_Device_ID	EQU	00063H
VIME_Device_ID	EQU	00064H
VHBIOSD_Device_ID	EQU	00065H
BASEID_FOR_NAMEBASEDVXD	EQU	0f000H
BASEID_FOR_NAMEBASEDVXD_MASK	EQU	0fffH
VMM_INIT_ORDER	EQU	000000000H
DEBUG_INIT_ORDER	EQU	000000000H
DEBUGCMD_INIT_ORDER	EQU	000000000H
PERF_INIT_ORDER	EQU	000900000H
APM_INIT_ORDER	EQU	001000000H
VPOWERD_INIT_ORDER	EQU	APM_INIT_ORDER
BIOSHOOK_INIT_ORDER	EQU	006000000H
VPROD_INIT_ORDER	EQU	008000000H
VPICD_INIT_ORDER	EQU	00C000000H
VTD_INIT_ORDER	EQU	014000000H
VXDLDR_INIT_ORDER	EQU	016000000H
ENUMERATOR_INIT_ORDER	EQU	016800000H
ISAPNP_INIT_ORDER	EQU	ENUMERATOR_INIT_ORDER
EISA_INIT_ORDER	EQU	ENUMERATOR_INIT_ORDER
PCI_INIT_ORDER	EQU	ENUMERATOR_INIT_ORDER
BIOS_INIT_ORDER	EQU	ENUMERATOR_INIT_ORDER+1
CONFIGMG_INIT_ORDER	EQU	ENUMERATOR_INIT_ORDER+0FFFFH
VCDFSD_INIT_ORDER	EQU	016F00000H
IOS_INIT_ORDER	EQU	017000000H
PAGEFILE_INIT_ORDER	EQU	018000000H
PAGESWAP_INIT_ORDER	EQU	01C000000H
PARITY_INIT_ORDER	EQU	020000000H
REBOOT_INIT_ORDER	EQU	024000000H
EBIOS_INIT_ORDER	EQU	026000000H
VDD_INIT_ORDER	EQU	028000000H
VSD_INIT_ORDER	EQU	02C000000H
VCD_INIT_ORDER	EQU	030000000H
COMMDRVR_INIT_ORDER	EQU	(VCD_INIT_ORDER-1)
PRTCL_INIT_ORDER	EQU	(COMMDRVR_INIT_ORDER-2)
MODEM_INIT_ORDER	EQU	(COMMDRVR_INIT_ORDER-3)
PORT_INIT_ORDER	EQU	(COMMDRVR_INIT_ORDER-4)
VMD_INIT_ORDER	EQU	034000000H
VKD_INIT_ORDER	EQU	038000000H
VPD_INIT_ORDER	EQU	03C000000H
BLOCKDEV_INIT_ORDER	EQU	040000000H
MCA_POS_INIT_ORDER	EQU	041000000H
SCSIFD_INIT_ORDER	EQU	041400000H
SCSIMASTER_INIT_ORDER	EQU	041800000H
INT13_INIT_ORDER	EQU	042000000H
VMCPD_INIT_ORDER	EQU	048000000H
BIOSXLAT_INIT_ORDER	EQU	050000000H
VNETBIOS_INIT_ORDER	EQU	054000000H
DOSMGR_INIT_ORDER	EQU	058000000H
DOSNET_INIT_ORDER	EQU	05C000000H
WINLOAD_INIT_ORDER	EQU	060000000H
VMPOLL_INIT_ORDER	EQU	064000000H
UNDEFINED_INIT_ORDER	EQU	080000000H
WIN32_INIT_ORDER	EQU	UNDEFINED_INIT_ORDER
VCOND_INIT_ORDER	EQU	UNDEFINED_INIT_ORDER
WINDEBUG_INIT_ORDER	EQU	081000000H
VDMAD_INIT_ORDER	EQU	090000000H
V86MMGR_INIT_ORDER	EQU	0A0000000H
IFSMgr_Init_Order	EQU	10000H+V86MMGR_Init_Order
FSD_Init_Order	EQU	00100H+IFSMgr_Init_Order
VFD_INIT_ORDER	EQU	50000H+IFSMgr_Init_Order
UNDEF_TOUCH_MEM_INIT_ORDER	EQU	0A8000000H
SHELL_INIT_ORDER	EQU	0B0000000H






IO_Delay    macro
jmp $+2
ENDM

VXD_FAILURE	EQU	0
VXD_SUCCESS	EQU	1

Pushad_Struc	STRUC
Pushad_EDI	DD	?
Pushad_ESI	DD	?
Pushad_EBP	DD	?
Pushad_ESP	DD	?
Pushad_EBX	DD	?
Pushad_EDX	DD	?
Pushad_ECX	DD	?
Pushad_EAX	DD	?
Pushad_Struc	ENDS
ifndef Not_VxD
??_CUR_CODE_SEG = 0

??_LCODE    =   1
??_ICODE    =   2
??_PCODE    =   3
??_SCODE    =   4
??_DBOCODE  =   5
??_16ICODE  =   6
??_RCODE    =   7
??_LOCKABLECODE =   8

?_LCODE     equ <(??_CUR_CODE_SEG MOD 16) - ??_LCODE>
?_ICODE     equ <(??_CUR_CODE_SEG MOD 16) - ??_ICODE>
?_PCODE     equ <(??_CUR_CODE_SEG MOD 16) - ??_PCODE>
?_SCODE     equ <(??_CUR_CODE_SEG MOD 16) - ??_SCODE>
?_DBOCODE   equ <(??_CUR_CODE_SEG MOD 16) - ??_DBOCODE>
?_16ICODE   equ <(??_CUR_CODE_SEG MOD 16) - ??_16ICODE>
?_RCODE     equ <(??_CUR_CODE_SEG MOD 16) - ??_RCODE>
?_LOCKABLECODE  equ <(??_CUR_CODE_SEG MOD 16) - ??_LOCKABLECODE>

ifndef NO_SEGMENTS





IFDEF   MASM6
_FLAT   EQU FLAT
ELSE
_FLAT   EQU USE32
ENDIF


_LTEXT      SEGMENT DWORD PUBLIC _FLAT 'LCODE'
_LTEXT      ENDS

_TEXT       SEGMENT DWORD PUBLIC _FLAT 'LCODE'
_TEXT       ENDS


_PTEXT      SEGMENT DWORD PUBLIC _FLAT 'PCODE'
_PTEXT      ENDS



; ====================================================================
; JWasm-compatible explicit segment macros (ported from MakeCodeSeg)
; ====================================================================
; The original DDK MakeCodeSeg generator defined VxD_<name>_CODE_SEG
; macros via a macro-def-inside-IRP-inside-MACRO idiom that JWasm's
; macro engine rejects ("Must be in segment block"). We define the
; segments FOSSIL.ASM actually uses explicitly instead. Semantics are
; identical: open a DWORD FLAT segment with the right class name.
; ====================================================================

_LTEXT   SEGMENT DWORD PUBLIC _FLAT 'LOCKEDCODE'
_LTEXT   ENDS
_LDATA   SEGMENT DWORD PUBLIC _FLAT 'LOCKEDDATA'
_LDATA   ENDS
_PTEXT   SEGMENT DWORD PUBLIC _FLAT 'PAGEABLECODE'
_PTEXT   ENDS
_PDATA   SEGMENT DWORD PUBLIC _FLAT 'PAGEABLEDATA'
_PDATA   ENDS
_ITEXT   SEGMENT DWORD PUBLIC _FLAT 'ICODE'
_ITEXT   ENDS
_RTEXT   SEGMENT DWORD PUBLIC _FLAT 'REALINITCODE'
_RTEXT   ENDS

VxD_LOCKED_CODE_SEG MACRO
    _LTEXT SEGMENT
    ASSUME cs:FLAT, ds:FLAT, es:FLAT, ss:FLAT
    ENDM
VxD_LOCKED_CODE_ENDS MACRO
    _LTEXT ENDS
    ENDM

VxD_LOCKED_DATA_SEG MACRO
    _LDATA SEGMENT
    ENDM
VxD_LOCKED_DATA_ENDS MACRO
    _LDATA ENDS
    ENDM

VxD_PAGEABLE_CODE_SEG MACRO
    _PTEXT SEGMENT
    ASSUME cs:FLAT, ds:FLAT, es:FLAT, ss:FLAT
    ENDM
VxD_PAGEABLE_CODE_ENDS MACRO
    _PTEXT ENDS
    ENDM

VxD_PAGEABLE_DATA_SEG MACRO
    _PDATA SEGMENT
    ENDM
VxD_PAGEABLE_DATA_ENDS MACRO
    _PDATA ENDS
    ENDM

VxD_ICODE_SEG MACRO
    _ITEXT SEGMENT
    ASSUME cs:FLAT, ds:FLAT, es:FLAT, ss:FLAT
    ENDM
VxD_ICODE_ENDS MACRO
    _ITEXT ENDS
    ENDM

VxD_REAL_INIT_SEG MACRO
    _RTEXT SEGMENT
    ASSUME cs:FLAT, ds:FLAT, es:FLAT, ss:FLAT
    ENDM
VxD_REAL_INIT_ENDS MACRO
    _RTEXT ENDS
    ENDM












DefLockableCodeBegin MACRO name, private
VxD_LOCKABLE_BEGIN_CODE_SEG
IFB <private>
    PUBLIC  name
ENDIF
name    LABEL   NEAR
VxD_LOCKABLE_BEGIN_CODE_ENDS
ifndef WIN31COMPAT
if DEBLEVEL
VxD_LOCKED_DATA_SEG
    PUBLIC name&_Debug_Flags
name&_Debug_Flags DD DFS_TEST_BLOCK
VxD_LOCKED_DATA_ENDS
??_debug_flags equ <name&_Debug_Flags>
endif
endif
    ENDM









DefLockableCodeEnd MACRO name, private
VxD_LOCKABLE_END_CODE_SEG
IFB <private>
    PUBLIC  name
ENDIF
name    LABEL   NEAR
VxD_LOCKABLE_END_CODE_ENDS
    ENDM





CodeLockFlags MACRO name
ifndef WIN31COMPAT
if DEBLEVEL
    ifndef name&_Debug_Flags
    VxD_LOCKED_DATA_SEG
        extrn   name&_Debug_Flags:dword
    VxD_LOCKED_DATA_ENDS
    ??_debug_flags equ <name&_Debug_Flags>
    endif
endif
endif
    ENDM






MarkCodeLocked MACRO
ifndef WIN31COMPAT
if DEBLEVEL
ifdef ??_debug_flags
    pushfd
    and ??_debug_flags,NOT DFS_TEST_BLOCK
    popfd
endif
endif
endif
    ENDM






MarkCodeUnlocked MACRO
ifndef WIN31COMPAT
if DEBLEVEL
ifdef ??_debug_flags
    pushfd
    or  ??_debug_flags,DFS_TEST_BLOCK
    popfd
endif
endif
endif
    ENDM



_ITEXT      SEGMENT DWORD PUBLIC _FLAT 'ICODE'
_ITEXT      ENDS


_LDATA      SEGMENT DWORD PUBLIC _FLAT 'LCODE'
_LDATA      ENDS

_DATA       SEGMENT DWORD PUBLIC _FLAT 'LCODE'
_DATA       ENDS


_PDATA      SEGMENT DWORD PUBLIC _FLAT 'PDATA'
_PDATA      ENDS


_IDATA      SEGMENT DWORD PUBLIC _FLAT 'ICODE'
_IDATA      ENDS


_BSS        SEGMENT DWORD PUBLIC _FLAT 'LCODE'
_BSS        ENDS

CONST       SEGMENT DWORD PUBLIC _FLAT 'LCODE'
CONST       ENDS

_TLS        SEGMENT DWORD PUBLIC _FLAT 'LCODE'
_TLS        ENDS


_STEXT      SEGMENT DWORD PUBLIC _FLAT 'SCODE'
_STEXT      ENDS


_SDATA      SEGMENT DWORD PUBLIC _FLAT 'SCODE'
_SDATA      ENDS


_DBOSTART   SEGMENT DWORD PUBLIC _FLAT 'DBOCODE'
_DBOSTART   ENDS


_DBOCODE    SEGMENT DWORD PUBLIC _FLAT 'DBOCODE'
_DBOCODE    ENDS


_DBODATA    SEGMENT DWORD PUBLIC _FLAT 'DBOCODE'
_DBODATA    ENDS

if DEBLEVEL

_PATHSTART  SEGMENT DWORD PUBLIC  _FLAT 'LCODE'
_PATHSTART  ENDS


_PATHDATA   SEGMENT DWORD PUBLIC  _FLAT 'LCODE'
_PATHDATA   ENDS


_PATHEND    SEGMENT DWORD PUBLIC  _FLAT 'LCODE'
_PATHEND    ENDS
endif


_16ICODE    SEGMENT WORD USE16 PUBLIC '16ICODE'
_16ICODE    ENDS


_RCODE      SEGMENT WORD USE16 PUBLIC 'RCODE'
_RCODE      ENDS

IFNDEF BLD_COFF
_LGROUP   GROUP _LTEXT, _TEXT, _LDATA, _DATA, _BSS, CONST, _TLS
_IGROUP   GROUP _ITEXT, _IDATA
_SGROUP   GROUP _STEXT, _SDATA
_DBOGROUP GROUP _DBOSTART, _DBOCODE, _DBODATA
IF DEBLEVEL
_PGROUP	  GROUP _PATHSTART, _PATHDATA, _PATHEND
ENDIF
ENDIF

endif

    ASSUME CS:FLAT, DS:FLAT, ES:FLAT, SS:FLAT

OFFSET32 EQU <OFFSET FLAT:>


BeginDoc





































EndDoc

Begin_Service_Table MACRO Device_Name, Def_Segment

IFDEF   Device_Name&_Name_Based
 IFNDEF @@NextInternalID
    @@NextInternalID    = 0
 ENDIF
 @@NextInternalID = (@@NextInternalID + 1)
 Device_Name&_Internal_ID   = @@NextInternalID + BASEID_FOR_NAMEBASEDVXD
 DefineVxDName  Device_Name, %Device_Name&_Internal_ID
ENDIF

IFB <Def_Segment>
    BST2 Device_Name, VxD
ELSE
    BST2 Device_Name, Def_Segment
ENDIF
    ENDM

DefineVxDName   MACRO Device_Name, InternalID
 @@VxDName&InternalID EQU   <___&Device_Name&STable>
ENDM


BST2 MACRO Device_Name, Def_Segment

Num_&Device_Name&_Services = 0

IFDEF Create_&Device_Name&_Service_Table

Def_Segment&_LOCKED_DATA_SEG

Device_Name&_Service_Table LABEL DWORD

Device_Name&_Service MACRO Procedure, Local_Seg, Condition
LOCAL $$&Procedure

  IFNB <Condition>
  $$&Procedure MACRO extern
    IFDEF &Condition
      IFNB <extern>
    EXTRN   Procedure:NEAR
      ELSE
    dd  OFFSET32 Procedure
      ENDIF
    ELSE
      IFB <extern>
      dd      0
      ENDIF
    ENDIF
    ENDM
  ENDIF

  IFDIFI <Procedure>, <RESERVED>
    PUBLIC _&Procedure
     IF1
    _&Procedure LABEL DWORD
     ENDIF
     IFDIFI <Local_Seg>, <LOCAL>
        IFNB <Local_Seg>
Local_Seg&_SEG
       ELSE
Def_Segment&_CODE_SEG
        ENDIF
        IFNB <Condition>
    $$&Procedure extern
       ELSE
    EXTRN   Procedure:NEAR
        ENDIF
        IFNB <Local_Seg>
Local_Seg&_ENDS
        ELSE
Def_Segment&_CODE_ENDS
        ENDIF
     ENDIF
      IFNB <Condition>
    $$&Procedure
      ELSE
    dd  OFFSET32 Procedure
      ENDIF

          IFDEF Device_Name&_Name_Based
        @@&Procedure = (Device_Name&_Internal_ID SHL 16) + Num_&Device_Name&_Services
          ELSE
        @@&Procedure = (Device_Name&_Device_ID SHL 16) + Num_&Device_Name&_Services
          ENDIF
  ELSE
    dd  0
  ENDIF
    Num_&Device_Name&_Services = Num_&Device_Name&_Services + 1
  IFNB <Condition>
    Purge $$&Procedure
  ENDIF
    ENDM

ELSE



IFDEF   Device_Name&_Name_Based

Device_Name&_Service MACRO Procedure, Local_Seg, Condition


  IFDIFI <Procedure>, <RESERVED>
    @@&Procedure = (Device_Name&_Internal_ID SHL 16) + Num_&Device_Name&_Services
  ENDIF
    Num_&Device_Name&_Services = Num_&Device_Name&_Services + 1

    ENDM
ELSE

Device_Name&_Service MACRO Procedure, Local_Seg, Condition

  IFDIFI <Procedure>, <RESERVED>
    @@&Procedure = (Device_Name&_Device_ID SHL 16) + Num_&Device_Name&_Services
  ENDIF
    Num_&Device_Name&_Services = Num_&Device_Name&_Services + 1

    ENDM
ENDIF

ENDIF

    ENDM



End_Service_Table MACRO Device_Name, Def_Segment

    PURGE   Device_Name&_Service

IFDEF Create_&Device_Name&_Service_Table

IFB <Def_Segment>
VxD_LOCKED_DATA_ENDS
ELSE
Def_Segment&_LOCKED_DATA_ENDS
ENDIF

ENDIF

    ENDM

GetVxDServiceOrdinal    macro   reg,service
    mov reg,@@&service
    endm

GetVxDServiceAddress    macro   reg,service
    mov reg,OFFSET32 service
    endm


















Begin_Win32_Services MACRO VxDName
ifndef Create_Win32_Services
    Create_Win32_Services = 0
endif
    .errb <VxDName>, <VxD name missing>
    ??w32svcno = 0
if Create_Win32_Services
VxDName&_Win32_Services label dword
    dd  csvc&VxDName, 0
endif
    ??inw32svc = 1

    VxDName&_Win32_Service MACRO Name
        .erre ??inw32svc, <Missing Begin_Win32_Services>
    if Create_Win32_Services
        dd  OFFSET32 Name,cparm&Name
    endif
        @32&Name equ   ((VxDName&_Device_ID SHL 16) + ??w32svcno)
        ??w32svcno = ??w32svcno + 1
        ENDM
    ENDM










End_Win32_Services MACRO VxDName
    .errb <VxDName>, <VxD name misssing>
if Create_Win32_Services
    csvc&VxDName    equ ($ - VxDName&_Win32_Services)/8 - 1
endif
    ??inw32svc = 0
    PURGE VxDName&_Win32_Service
    ENDM











Declare_Win32_Service MACRO Name, cParms
ifndef Create_Win32_Services
    Create_Win32_Services = 0
endif
if Create_Win32_Services
    ?merge  <Name>,,,,<EQU>,<_>,<Name>,<@>,%(cParms*4 + 8)
    ?merge  <cparm>,<Name>,,,<EQU>,<cParms>
VxD_CODE_SEG
    ?merge  <EXTRN>,,,,,<_>,<Name>,<@>,%(cParms*4 + 8),<:NEAR>
VxD_CODE_ENDS
endif
    ENDM












Win32call MACRO Service, CallBack
ifndef Create_Win32_Services
    Create_Win32_Services = 0
endif
ife Create_Win32_Services
    mov eax,@32&Service
ifdef IS_16
    movzx   esp,sp
endif
    call    fword ptr [CallBack]
ifdef DEBUG
    int 3
endif
endif
    ENDM







Dword_Align MACRO Seg_Name
    LOCAL segn
IFDEF MASM6
    align 4
ELSE
IFNB <Seg_Name>
    segn equ Seg_Name
ELSE
IFE ?_LCODE
    segn equ <_LTEXT>
ELSE
IFE ?_ICODE
    segn equ <_ITEXT>
ELSE
IFE ?_PCODE
    segn equ <_PTEXT>
ELSE
IFE ?_SCODE
    segn equ <_STEXT>
ELSE
.err <Dword_Align not supported>
ENDIF
ENDIF
ENDIF
ENDIF
ENDIF
IF (($-OFFSET segn:0) MOD 4)
db 4 - (($-OFFSET segn:0) MOD 4) DUP (90h)
ENDIF
ENDIF
        ENDM


BeginDoc

















EndDoc

Fatal_Error MACRO Msg_Ptr, Exit_Flags
    pushad
IFB <Msg_Ptr>
    xor esi, esi
ELSE
    mov esi, Msg_Ptr
IFB <Exit_Flags>
    xor eax, eax
ELSE
    mov eax, Exit_Flags
ENDIF
ENDIF
    VMMCall Fatal_Error_Handler
    ENDM

EF_Hang_On_Exit     EQU     1h


cb_s	STRUC
CB_VM_Status	DD	?
CB_High_Linear	DD	?
CB_Client_Pointer	DD	?
CB_VMID	DD	?
CB_Signature	DD	?
cb_s	ENDS
VMCB_ID	EQU	62634D56H
VMSTAT_EXCLUSIVE_BIT	EQU	00H
VMSTAT_EXCLUSIVE	EQU	(1 SHL VMSTAT_EXCLUSIVE_BIT)
VMSTAT_BACKGROUND_BIT	EQU	01H
VMSTAT_BACKGROUND	EQU	(1 SHL VMSTAT_BACKGROUND_BIT)
VMSTAT_CREATING_BIT	EQU	02H
VMSTAT_CREATING	EQU	(1 SHL VMSTAT_CREATING_BIT)
VMSTAT_SUSPENDED_BIT	EQU	03H
VMSTAT_SUSPENDED	EQU	(1 SHL VMSTAT_SUSPENDED_BIT)
VMSTAT_NOT_EXECUTEABLE_BIT	EQU	04H
VMSTAT_NOT_EXECUTEABLE	EQU	(1 SHL VMSTAT_NOT_EXECUTEABLE_BIT)
VMSTAT_PM_EXEC_BIT	EQU	05H
VMSTAT_PM_EXEC	EQU	(1 SHL VMSTAT_PM_EXEC_BIT)
VMSTAT_PM_APP_BIT	EQU	06H
VMSTAT_PM_APP	EQU	(1 SHL VMSTAT_PM_APP_BIT)
VMSTAT_PM_USE32_BIT	EQU	07H
VMSTAT_PM_USE32	EQU	(1 SHL VMSTAT_PM_USE32_BIT)
VMSTAT_VXD_EXEC_BIT	EQU	08H
VMSTAT_VXD_EXEC	EQU	(1 SHL VMSTAT_VXD_EXEC_BIT)
VMSTAT_HIGH_PRI_BACK_BIT	EQU	09H
VMSTAT_HIGH_PRI_BACK	EQU	(1 SHL VMSTAT_HIGH_PRI_BACK_BIT)
VMSTAT_BLOCKED_BIT	EQU	0AH
VMSTAT_BLOCKED	EQU	(1 SHL VMSTAT_BLOCKED_BIT)
VMSTAT_AWAKENING_BIT	EQU	0BH
VMSTAT_AWAKENING	EQU	(1 SHL VMSTAT_AWAKENING_BIT)
VMSTAT_PAGEABLEV86BIT	EQU	0CH
VMSTAT_PAGEABLEV86_BIT	EQU	VMSTAT_PAGEABLEV86BIT
VMSTAT_PAGEABLEV86	EQU	(1 SHL VMSTAT_PAGEABLEV86BIT)
VMSTAT_V86INTSLOCKEDBIT	EQU	0DH
VMSTAT_V86INTSLOCKED_BIT	EQU	VMSTAT_V86INTSLOCKEDBIT
VMSTAT_V86INTSLOCKED	EQU	(1 SHL VMSTAT_V86INTSLOCKEDBIT)
VMSTAT_IDLE_TIMEOUT_BIT	EQU	0EH
VMSTAT_IDLE_TIMEOUT	EQU	(1 SHL VMSTAT_IDLE_TIMEOUT_BIT)
VMSTAT_IDLE_BIT	EQU	0FH
VMSTAT_IDLE	EQU	(1 SHL VMSTAT_IDLE_BIT)
VMSTAT_CLOSING_BIT	EQU	10H
VMSTAT_CLOSING	EQU	(1 SHL VMSTAT_CLOSING_BIT)
VMSTAT_TS_SUSPENDED_BIT	EQU	11H
VMSTAT_TS_SUSPENDED	EQU	(1 SHL VMSTAT_TS_SUSPENDED_BIT)
VMSTAT_TS_MAXPRI_BIT	EQU	12H
VMSTAT_TS_MAXPRI	EQU	(1 SHL VMSTAT_TS_MAXPRI_BIT)
VMSTAT_USE32_MASK	EQU	(VMSTAT_PM_USE32 OR VMSTAT_VXD_EXEC)

tcb_s	STRUC
TCB_Flags	DD	?
TCB_Reserved1	DD	?
TCB_Reserved2	DD	?
TCB_Signature	DD	?
TCB_ClientPtr	DD	?
TCB_VMHandle	DD	?
TCB_ThreadId	DW	?
TCB_PMLockOrigSS	DW	?
TCB_PMLockOrigESP	DD	?
TCB_PMLockOrigEIP	DD	?
TCB_PMLockStackCount	DD	?
TCB_PMLockOrigCS	DW	?
TCB_PMPSPSelector	DW	?
TCB_ThreadType	DD	?
TCB_pad1	DW	?
TCB_pad2	DB	?
TCB_extErrLocus	DB	?
TCB_extErr	DW	?
TCB_extErrAction	DB	?
TCB_extErrClass	DB	?
TCB_extErrPtr	DD	?
tcb_s	ENDS
SCHED_OBJ_ID_THREAD	EQU	42434854H
THFLAG_SUSPENDED_BIT	EQU	03H
THFLAG_SUSPENDED	EQU	(1 SHL THFLAG_SUSPENDED_BIT)
THFLAG_NOT_EXECUTEABLE_BIT	EQU	04H
THFLAG_NOT_EXECUTEABLE	EQU	(1 SHL THFLAG_NOT_EXECUTEABLE_BIT)
THFLAG_THREAD_CREATION_BIT	EQU	08H
THFLAG_THREAD_CREATION	EQU	(1 SHL THFLAG_THREAD_CREATION_BIT)
THFLAG_THREAD_BLOCKED_BIT	EQU	0AH
THFLAG_THREAD_BLOCKED	EQU	(1 SHL THFLAG_THREAD_BLOCKED_BIT)
THFLAG_RING0_THREAD_BIT	EQU	1CH
THFLAG_RING0_THREAD	EQU	(1 SHL THFLAG_RING0_THREAD_BIT)
THFLAG_CHARSET_BITS	EQU	10H
THFLAG_CHARSET_MASK	EQU	(3 SHL THFLAG_CHARSET_BITS)
THFLAG_ANSI	EQU	(0 SHL THFLAG_CHARSET_BITS)
THFLAG_OEM	EQU	(1 SHL THFLAG_CHARSET_BITS)
THFLAG_UNICODE	EQU	(2 SHL THFLAG_CHARSET_BITS)
THFLAG_RESERVED	EQU	(3 SHL THFLAG_CHARSET_BITS)
THFLAG_EXTENDED_HANDLES_BIT	EQU	12H
THFLAG_EXTENDED_HANDLES	EQU	(1 SHL THFLAG_EXTENDED_HANDLES_BIT)
THFLAG_OPEN_AS_IMMOVABLE_FILE_BIT	EQU	13H
THFLAG_OPEN_AS_IMMOVABLE_FILE	EQU	(1 SHL THFLAG_OPEN_AS_IMMOVABLE_FILE_BIT)

pmcb_s	STRUC
PMCB_Flags	DD	?
PMCB_Parent	DD	?
pmcb_s	ENDS

VMFaultInfo	STRUC
VMFI_EIP	DD	?
VMFI_CS	DW	?
VMFI_Ints	DW	?
VMFaultInfo	ENDS
Begin_Service_Table VMM, VMM
VMM_Service  Get_VMM_Version, LOCAL
VMM_Service  Get_Cur_VM_Handle
VMM_Service  Test_Cur_VM_Handle
VMM_Service  Get_Sys_VM_Handle
VMM_Service  Test_Sys_VM_Handle
VMM_Service  Validate_VM_Handle
VMM_Service  Get_VMM_Reenter_Count
VMM_Service  Begin_Reentrant_Execution
VMM_Service  End_Reentrant_Execution
VMM_Service  Install_V86_Break_Point
VMM_Service  Remove_V86_Break_Point
VMM_Service  Allocate_V86_Call_Back
VMM_Service  Allocate_PM_Call_Back
VMM_Service  Call_When_VM_Returns
VMM_Service  Schedule_Global_Event
VMM_Service  Schedule_VM_Event
VMM_Service  Call_Global_Event
VMM_Service  Call_VM_Event
VMM_Service  Cancel_Global_Event
VMM_Service  Cancel_VM_Event
VMM_Service  Call_Priority_VM_Event
VMM_Service  Cancel_Priority_VM_Event
VMM_Service  Get_NMI_Handler_Addr
VMM_Service  Set_NMI_Handler_Addr
VMM_Service  Hook_NMI_Event
VMM_Service  Call_When_VM_Ints_Enabled
VMM_Service  Enable_VM_Ints
VMM_Service  Disable_VM_Ints
VMM_Service  Map_Flat
VMM_Service  Map_Lin_To_VM_Addr
VMM_Service  Adjust_Exec_Priority
VMM_Service  Begin_Critical_Section
VMM_Service  End_Critical_Section
VMM_Service  End_Crit_And_Suspend
VMM_Service  Claim_Critical_Section
VMM_Service  Release_Critical_Section
VMM_Service  Call_When_Not_Critical
VMM_Service  Create_Semaphore
VMM_Service  Destroy_Semaphore
VMM_Service  Wait_Semaphore
VMM_Service  Signal_Semaphore
VMM_Service  Get_Crit_Section_Status
VMM_Service  Call_When_Task_Switched
VMM_Service  Suspend_VM
VMM_Service  Resume_VM
VMM_Service  No_Fail_Resume_VM
VMM_Service  Nuke_VM
VMM_Service  Crash_Cur_VM
VMM_Service  Get_Execution_Focus
VMM_Service  Set_Execution_Focus
VMM_Service  Get_Time_Slice_Priority
VMM_Service  Set_Time_Slice_Priority
VMM_Service  Get_Time_Slice_Granularity
VMM_Service  Set_Time_Slice_Granularity
VMM_Service  Get_Time_Slice_Info
VMM_Service  Adjust_Execution_Time
VMM_Service  Release_Time_Slice
VMM_Service  Wake_Up_VM
VMM_Service  Call_When_Idle
VMM_Service  Get_Next_VM_Handle
VMM_Service  Set_Global_Time_Out
VMM_Service  Set_VM_Time_Out
VMM_Service  Cancel_Time_Out
VMM_Service  Get_System_Time
VMM_Service  Get_VM_Exec_Time
VMM_Service  Hook_V86_Int_Chain
VMM_Service  Get_V86_Int_Vector
VMM_Service  Set_V86_Int_Vector
VMM_Service  Get_PM_Int_Vector
VMM_Service  Set_PM_Int_Vector
VMM_Service  Simulate_Int
VMM_Service  Simulate_Iret
VMM_Service  Simulate_Far_Call
VMM_Service  Simulate_Far_Jmp
VMM_Service  Simulate_Far_Ret
VMM_Service  Simulate_Far_Ret_N
VMM_Service  Build_Int_Stack_Frame
VMM_Service  Simulate_Push
VMM_Service  Simulate_Pop
VMM_Service  _HeapAllocate
VMM_Service  _HeapReAllocate
VMM_Service  _HeapFree
VMM_Service  _HeapGetSize
HEAPZEROINIT	EQU	00000001H
HEAPZEROREINIT	EQU	00000002H
HEAPNOCOPY	EQU	00000004H
HEAPLOCKEDIFDP	EQU	00000100H
HEAPSWAP	EQU	00000200H
HEAPINIT	EQU	00000400H
HEAPCLEAN	EQU	00000800H
VMM_Service  _PageAllocate
VMM_Service  _PageReAllocate
VMM_Service  _PageFree
VMM_Service  _PageLock
VMM_Service  _PageUnLock
VMM_Service  _PageGetSizeAddr
VMM_Service  _PageGetAllocInfo
VMM_Service  _GetFreePageCount
VMM_Service  _GetSysPageCount
VMM_Service  _GetVMPgCount
VMM_Service  _MapIntoV86
VMM_Service  _PhysIntoV86
VMM_Service  _TestGlobalV86Mem
VMM_Service  _ModifyPageBits
VMM_Service  _CopyPageTable
VMM_Service  _LinMapIntoV86
VMM_Service  _LinPageLock
VMM_Service  _LinPageUnLock
VMM_Service  _SetResetV86Pageable
VMM_Service  _GetV86PageableArray
VMM_Service  _PageCheckLinRange
VMM_Service  _PageOutDirtyPages
VMM_Service  _PageDiscardPages
PAGEZEROINIT	EQU	00000001H
PAGEUSEALIGN	EQU	00000002H
PAGECONTIG	EQU	00000004H
PAGEFIXED	EQU	00000008H
PAGEDEBUGNULFAULT	EQU	00000010H
PAGEZEROREINIT	EQU	00000020H
PAGENOCOPY	EQU	00000040H
PAGELOCKED	EQU	00000080H
PAGELOCKEDIFDP	EQU	00000100H
PAGESETV86PAGEABLE	EQU	00000200H
PAGECLEARV86PAGEABLE	EQU	00000400H
PAGESETV86INTSLOCKED	EQU	00000800H
PAGECLEARV86INTSLOCKED	EQU	00001000H
PAGEMARKPAGEOUT	EQU	00002000H
PAGEPDPSETBASE	EQU	00004000H
PAGEPDPCLEARBASE	EQU	00008000H
PAGEDISCARD	EQU	00010000H
PAGEPDPQUERYDIRTY	EQU	00020000H
PAGEMAPFREEPHYSREG	EQU	00040000H
PAGENOMOVE	EQU	10000000H
PAGEMAPGLOBAL	EQU	40000000H
PAGEMARKDIRTY	EQU	80000000H
MAPV86_IGNOREWRAP	EQU	00000001H
VMM_Service  _GetNulPageHandle
VMM_Service  _GetFirstV86Page
VMM_Service  _MapPhysToLinear
VMM_Service  _GetAppFlatDSAlias
VMM_Service  _SelectorMapFlat
VMM_Service  _GetDemandPageInfo
VMM_Service  _GetSetPageOutCount
GSPOC_F_GET	EQU	00000001H
VMM_Service  Hook_V86_Page
VMM_Service  _Assign_Device_V86_Pages
VMM_Service  _DeAssign_Device_V86_Pages
VMM_Service  _Get_Device_V86_Pages_Array
VMM_Service  MMGR_SetNULPageAddr
VMM_Service  _Allocate_GDT_Selector
VMM_Service  _Free_GDT_Selector
VMM_Service  _Allocate_LDT_Selector
VMM_Service  _Free_LDT_Selector
VMM_Service  _BuildDescriptorDWORDs
VMM_Service  _GetDescriptor
VMM_Service  _SetDescriptor
BDDEXPLICITDPL	EQU	00000001H
ALDTSPECSEL	EQU	00000001H
VMM_Service  _MMGR_Toggle_HMA
MMGRHMAPHYSICAL	EQU	00000001H
MMGRHMAENABLE	EQU	00000002H
MMGRHMADISABLE	EQU	00000004H
MMGRHMAQUERY	EQU	00000008H
VMM_Service  Get_Fault_Hook_Addrs
VMM_Service  Hook_V86_Fault
VMM_Service  Hook_PM_Fault
VMM_Service  Hook_VMM_Fault
VMM_Service  Begin_Nest_V86_Exec
VMM_Service  Begin_Nest_Exec
VMM_Service  Exec_Int
VMM_Service  Resume_Exec
VMM_Service  End_Nest_Exec
VMM_Service  Allocate_PM_App_CB_Area, VMM_ICODE
VMM_Service  Get_Cur_PM_App_CB
VMM_Service  Set_V86_Exec_Mode
VMM_Service  Set_PM_Exec_Mode
VMM_Service  Begin_Use_Locked_PM_Stack
VMM_Service  End_Use_Locked_PM_Stack
VMM_Service  Save_Client_State
VMM_Service  Restore_Client_State
VMM_Service  Exec_VxD_Int
VMM_Service  Hook_Device_Service
VMM_Service  Hook_Device_V86_API
VMM_Service  Hook_Device_PM_API
VMM_Service  System_Control
VMM_Service  Simulate_IO
VMM_Service  Install_Mult_IO_Handlers
VMM_Service  Install_IO_Handler
VMM_Service  Enable_Global_Trapping
VMM_Service  Enable_Local_Trapping
VMM_Service  Disable_Global_Trapping
VMM_Service  Disable_Local_Trapping
VMM_Service  List_Create
VMM_Service  List_Destroy
VMM_Service  List_Allocate
VMM_Service  List_Attach
VMM_Service  List_Attach_Tail
VMM_Service  List_Insert
VMM_Service  List_Remove
VMM_Service  List_Deallocate
VMM_Service  List_Get_First
VMM_Service  List_Get_Next
VMM_Service  List_Remove_First
LF_ASYNC_BIT	EQU	0
LF_ASYNC	EQU	(1 SHL LF_ASYNC_BIT)
LF_USE_HEAP_BIT	EQU	1
LF_USE_HEAP	EQU	(1 SHL LF_USE_HEAP_BIT)
LF_ALLOC_ERROR_BIT	EQU	2
LF_ALLOC_ERROR	EQU	(1 SHL LF_ALLOC_ERROR_BIT)
LF_SWAP	EQU	(LF_USE_HEAP+(1 SHL 3))
VMM_Service  _AddInstanceItem
VMM_Service  _Allocate_Device_CB_Area
VMM_Service  _Allocate_Global_V86_Data_Area, VMM_ICODE
VMM_Service  _Allocate_Temp_V86_Data_Area, VMM_ICODE
VMM_Service  _Free_Temp_V86_Data_Area, VMM_ICODE
GVDAWordAlign	EQU	00000001H
GVDADWordAlign	EQU	00000002H
GVDAParaAlign	EQU	00000004H
GVDAPageAlign	EQU	00000008H
GVDAInstance	EQU	00000100H
GVDAZeroInit	EQU	00000200H
GVDAReclaim	EQU	00000400H
GVDAInquire	EQU	00000800H
GVDAHighSysCritOK	EQU	00001000H
GVDAOptInstance	EQU	00002000H
GVDAForceLow	EQU	00004000H
TVDANeedTilInitComplete	EQU	00000001H
VMM_Service  Get_Profile_Decimal_Int, VMM_ICODE
VMM_Service  Convert_Decimal_String, VMM_ICODE
VMM_Service  Get_Profile_Fixed_Point, VMM_ICODE
VMM_Service  Convert_Fixed_Point_String, VMM_ICODE
VMM_Service  Get_Profile_Hex_Int, VMM_ICODE
VMM_Service  Convert_Hex_String, VMM_ICODE
VMM_Service  Get_Profile_Boolean, VMM_ICODE
VMM_Service  Convert_Boolean_String, VMM_ICODE
VMM_Service  Get_Profile_String, VMM_ICODE
VMM_Service  Get_Next_Profile_String, VMM_ICODE
VMM_Service  Get_Environment_String, VMM_ICODE
VMM_Service  Get_Exec_Path, VMM_ICODE
VMM_Service  Get_Config_Directory, VMM_ICODE
VMM_Service  OpenFile, VMM_ICODE
VMM_OPENFILE_BUF_SIZE	EQU	260
VMM_Service  Get_PSP_Segment, VMM_ICODE
VMM_Service  GetDOSVectors, VMM_ICODE
VMM_Service  Get_Machine_Info
GMIF_80486_BIT	EQU	10H
GMIF_80486	EQU	(1 SHL GMIF_80486_BIT)
GMIF_PCXT_BIT	EQU	11H
GMIF_PCXT	EQU	(1 SHL GMIF_PCXT_BIT)
GMIF_MCA_BIT	EQU	12H
GMIF_MCA	EQU	(1 SHL GMIF_MCA_BIT)
GMIF_EISA_BIT	EQU	13H
GMIF_EISA	EQU	(1 SHL GMIF_EISA_BIT)
GMIF_CPUID_BIT	EQU	14H
GMIF_CPUID	EQU	(1 SHL GMIF_CPUID_BIT)
VMM_Service  GetSet_HMA_Info
VMM_Service  Set_System_Exit_Code
VMM_Service  Fatal_Error_Handler
VMM_Service  Fatal_Memory_Error
VMM_Service  Update_System_Clock
VMM_Service  Test_Debug_Installed
VMM_Service  Out_Debug_String
VMM_Service  Out_Debug_Chr
VMM_Service  In_Debug_Chr
VMM_Service  Debug_Convert_Hex_Binary
VMM_Service  Debug_Convert_Hex_Decimal
VMM_Service  Debug_Test_Valid_Handle
VMM_Service  Validate_Client_Ptr
VMM_Service  Test_Reenter
VMM_Service  Queue_Debug_String
VMM_Service  Log_Proc_Call
VMM_Service  Debug_Test_Cur_VM
VMM_Service  Get_PM_Int_Type
VMM_Service  Set_PM_Int_Type
VMM_Service  Get_Last_Updated_System_Time
VMM_Service  Get_Last_Updated_VM_Exec_Time
VMM_Service  Test_DBCS_Lead_Byte
.errnz  @@Test_DBCS_Lead_Byte - 100D1h

VMM_Service  _AddFreePhysPage, VMM_ICODE
VMM_Service  _PageResetHandlePAddr
VMM_Service  _SetLastV86Page, VMM_ICODE
VMM_Service  _GetLastV86Page
VMM_Service  _MapFreePhysReg
VMM_Service  _UnmapFreePhysReg
VMM_Service  _XchgFreePhysReg
VMM_Service  _SetFreePhysRegCalBk, VMM_ICODE
VMM_Service  Get_Next_Arena, VMM_ICODE
VMM_Service  Get_Name_Of_Ugly_TSR, VMM_ICODE
VMM_Service  Get_Debug_Options, VMM_ICODE
AFPP_SWAPOUT	EQU	0001H
PCP_CHANGEPAGER	EQU	1H
PCP_CHANGEPAGERDATA	EQU	2H
PCP_VIRGINONLY	EQU	4H
GNA_HIDOSLINKED	EQU	0002H
GNA_ISHIGHDOS	EQU	0004H
VMM_Service  Set_Physical_HMA_Alias, VMM_ICODE
VMM_Service  _GetGlblRng0V86IntBase, VMM_ICODE
VMM_Service  _Add_Global_V86_Data_Area, VMM_ICODE
VMM_Service  GetSetDetailedVMError
GSDVME_PRIVINST	EQU	00010001H
GSDVME_INVALINST	EQU	00010002H
GSDVME_INVALPGFLT	EQU	00010003H
GSDVME_INVALGPFLT	EQU	00010004H
GSDVME_INVALFLT	EQU	00010005H
GSDVME_USERNUKE	EQU	00010006H
GSDVME_DEVNUKE	EQU	00010007H
GSDVME_DEVNUKEHDWR	EQU	00010008H
GSDVME_NUKENOMSG	EQU	00010009H
GSDVME_OKNUKEMASK	EQU	80000000H
GSDVME_INSMEMV86	EQU	00020001H
GSDVME_INSV86SPACE	EQU	00020002H
GSDVME_INSMEMXMS	EQU	00020003H
GSDVME_INSMEMEMS	EQU	00020004H
GSDVME_INSMEMV86HI	EQU	00020005H
GSDVME_INSMEMVID	EQU	00020006H
GSDVME_INSMEMVM	EQU	00020007H
GSDVME_INSMEMDEV	EQU	00020008H
GSDVME_CRTNOMSG	EQU	00020009H
VMM_Service  Is_Debug_Chr
VMM_Service  Clear_Mono_Screen
VMM_Service  Out_Mono_Chr
VMM_Service  Out_Mono_String
VMM_Service  Set_Mono_Cur_Pos
VMM_Service  Get_Mono_Cur_Pos
VMM_Service  Get_Mono_Chr
VMM_Service  Locate_Byte_In_ROM, VMM_ICODE
VMM_Service  Hook_Invalid_Page_Fault
VMM_Service  Unhook_Invalid_Page_Fault
IPF_PGDIR	EQU	00000001H
IPF_V86PG	EQU	00000002H
IPF_V86PGH	EQU	00000004H
IPF_INVTYP	EQU	00000008H
IPF_PGERR	EQU	00000010H
IPF_REFLT	EQU	00000020H
IPF_VMM	EQU	00000040H
IPF_PM	EQU	00000080H
IPF_V86	EQU	00000100H
VMM_Service  Set_Delete_On_Exit_File
VMM_Service  Close_VM
CVF_CONTINUE_EXEC_BIT	EQU	0
CVF_CONTINUE_EXEC	EQU	(1 SHL CVF_CONTINUE_EXEC_BIT)
VMM_Service  Enable_Touch_1st_Meg
VMM_Service  Disable_Touch_1st_Meg
VMM_Service  Install_Exception_Handler
VMM_Service  Remove_Exception_Handler
VMM_Service  Get_Crit_Status_No_Block

.errnz   @@Get_Crit_Status_No_Block - 100F1h

ifdef WIN40SERVICES
VMM_Service  _GetLastUpdatedThreadExecTime
VMM_Service  _Trace_Out_Service
VMM_Service  _Debug_Out_Service
VMM_Service  _Debug_Flags_Service
endif
DFS_LOG_BIT	EQU	0
DFS_LOG	EQU	(1 SHL DFS_LOG_BIT)
DFS_PROFILE_BIT	EQU	1
DFS_PROFILE	EQU	(1 SHL DFS_PROFILE_BIT)
DFS_TEST_CLD_BIT	EQU	2
DFS_TEST_CLD	EQU	(1 SHL DFS_TEST_CLD_BIT)
DFS_NEVER_REENTER_BIT	EQU	3
DFS_NEVER_REENTER	EQU	(1 SHL DFS_NEVER_REENTER_BIT)
DFS_TEST_REENTER_BIT	EQU	4
DFS_TEST_REENTER	EQU	(1 SHL DFS_TEST_REENTER_BIT)
DFS_NOT_SWAPPING_BIT	EQU	5
DFS_NOT_SWAPPING	EQU	(1 SHL DFS_NOT_SWAPPING_BIT)
DFS_TEST_BLOCK_BIT	EQU	6
DFS_TEST_BLOCK	EQU	(1 SHL DFS_TEST_BLOCK_BIT)
DFS_RARE_SERVICES	EQU	0FFFFFF80H
DFS_EXIT_NOBLOCK	EQU	(DFS_RARE_SERVICES+0)
DFS_ENTER_NOBLOCK	EQU	(DFS_RARE_SERVICES+DFS_TEST_BLOCK)
DFS_TEST_NEST_EXEC	EQU	(DFS_RARE_SERVICES+1)
ifdef WIN40SERVICES
VMM_Service  VMMAddImportModuleName
VMM_Service  VMM_Add_DDB
VMM_Service  VMM_Remove_DDB
VMM_Service  Test_VM_Ints_Enabled
VMM_Service  _BlockOnID
VMM_Service  Schedule_Thread_Event
VMM_Service  Cancel_Thread_Event
VMM_Service  Set_Thread_Time_Out
VMM_Service  Set_Async_Time_Out
VMM_Service  _AllocateThreadDataSlot
VMM_Service  _FreeThreadDataSlot
MUTEX_MUST_COMPLETE	EQU	1
MUTEX_NO_CLEANUP_THREAD_STATE	EQU	2
VMM_Service  _CreateMutex
VMM_Service  _DestroyMutex
VMM_Service  _GetMutexOwner
VMM_Service  Call_When_Thread_Switched
VMM_Service  VMMCreateThread
VMM_Service  _GetThreadExecTime
VMM_Service  VMMTerminateThread
VMM_Service  Get_Cur_Thread_Handle
VMM_Service  Test_Cur_Thread_Handle
VMM_Service  Get_Sys_Thread_Handle
VMM_Service  Test_Sys_Thread_Handle
VMM_Service  Validate_Thread_Handle
VMM_Service  Get_Initial_Thread_Handle
VMM_Service  Test_Initial_Thread_Handle
VMM_Service  Debug_Test_Valid_Thread_Handle
VMM_Service  Debug_Test_Cur_Thread
VMM_Service  VMM_GetSystemInitState
VMM_Service  Cancel_Call_When_Thread_Switched
VMM_Service  Get_Next_Thread_Handle
VMM_Service  Adjust_Thread_Exec_Priority
VMM_Service  _Deallocate_Device_CB_Area
VMM_Service  Remove_IO_Handler
VMM_Service  Remove_Mult_IO_Handlers
VMM_Service  Unhook_V86_Int_Chain
VMM_Service  Unhook_V86_Fault
VMM_Service  Unhook_PM_Fault
VMM_Service  Unhook_VMM_Fault
VMM_Service  Unhook_Device_Service
VMM_Service  _PageReserve
VMM_Service  _PageCommit
VMM_Service  _PageDecommit
VMM_Service  _PagerRegister
VMM_Service  _PagerQuery
VMM_Service  _PagerDeregister
VMM_Service  _ContextCreate
VMM_Service  _ContextDestroy
VMM_Service  _PageAttach
VMM_Service  _PageFlush
VMM_Service  _SignalID
VMM_Service  _PageCommitPhys
VMM_Service  _Register_Win32_Services
VMM_Service  Cancel_Call_When_Not_Critical
VMM_Service  Cancel_Call_When_Idle
VMM_Service  Cancel_Call_When_Task_Switched
VMM_Service  _Debug_Printf_Service
VMM_Service  _EnterMutex
VMM_Service  _LeaveMutex
VMM_Service  Simulate_VM_IO
VMM_Service  Signal_Semaphore_No_Switch
VMM_Service  _ContextSwitch
VMM_Service  _PageModifyPermissions
VMM_Service  _PageQuery
VMM_Service  _EnterMustComplete
VMM_Service  _LeaveMustComplete
VMM_Service  _ResumeExecMustComplete
THREAD_TERM_STATUS_CRASH_PEND	EQU	1
THREAD_TERM_STATUS_NUKE_PEND	EQU	2
THREAD_TERM_STATUS_SUSPEND_PEND	EQU	4
VMM_Service  _GetThreadTerminationStatus
VMM_Service  _GetInstanceInfo
INSTINFO_NONE	EQU	0
INSTINFO_SOME	EQU	1
INSTINFO_ALL	EQU	2
VMM_Service  _ExecIntMustComplete
VMM_Service  _ExecVxDIntMustComplete
VMM_Service  Begin_V86_Serialization
VMM_Service  Unhook_V86_Page
VMM_Service  VMM_GetVxDLocationList
VMM_Service  VMM_GetDDBList
VMM_Service  Unhook_NMI_Event
VMM_Service  Get_Instanced_V86_Int_Vector
VMM_Service  Get_Set_Real_DOS_PSP
GSRDP_Set	EQU	0001H
VMM_Service  Call_Priority_Thread_Event
VMM_Service  Get_System_Time_Address
VMM_Service  Get_Crit_Status_Thread
VMM_Service  Get_DDB
VMM_Service  Directed_Sys_Control
VMM_Service  _RegOpenKey
VMM_Service  _RegCloseKey
VMM_Service  _RegCreateKey
VMM_Service  _RegDeleteKey
VMM_Service  _RegEnumKey
VMM_Service  _RegQueryValue
VMM_Service  _RegSetValue
VMM_Service  _RegDeleteValue
VMM_Service  _RegEnumValue
VMM_Service  _RegQueryValueEx
VMM_Service  _RegSetValueEx
ifndef REG_SZ
REG_SZ	EQU	0001H
REG_BINARY	EQU	0003H
endif
ifndef HKEY_LOCAL_MACHINE
HKEY_CLASSES_ROOT	EQU	80000000H
HKEY_CURRENT_USER	EQU	80000001H
HKEY_LOCAL_MACHINE	EQU	80000002H
HKEY_USERS	EQU	80000003H
HKEY_PERFORMANCE_DATA	EQU	80000004H
HKEY_CURRENT_CONFIG	EQU	80000005H
HKEY_DYN_DATA	EQU	80000006H
endif
VMM_Service  _CallRing3
VMM_Service  Exec_PM_Int
VMM_Service  _RegFlushKey
VMM_Service  _PageCommitContig
VMM_Service  _GetCurrentContext
VMM_Service  _LocalizeSprintf
VMM_Service  _LocalizeStackSprintf
VMM_Service  Call_Restricted_Event
VMM_Service  Cancel_Restricted_Event
VMM_Service  Register_PEF_Provider, VMM_ICODE
VMM_Service  _GetPhysPageInfo
VMM_Service  _RegQueryInfoKey
VMM_Service  MemArb_Reserve_Pages
PHYSINFO_NONE	EQU	0
PHYSINFO_SOME	EQU	1
PHYSINFO_ALL	EQU	2
VMM_Service  Time_Slice_Sys_VM_Idle
VMM_Service  Time_Slice_Sleep
VMM_Service  Boost_With_Decay
VMM_Service  Set_Inversion_Pri
VMM_Service  Reset_Inversion_Pri
VMM_Service  Release_Inversion_Pri
VMM_Service  Get_Thread_Win32_Pri
VMM_Service  Set_Thread_Win32_Pri
VMM_Service  Set_Thread_Static_Boost
VMM_Service  Set_VM_Static_Boost
VMM_Service  Release_Inversion_Pri_ID
VMM_Service  Attach_Thread_To_Group
VMM_Service  Detach_Thread_From_Group
VMM_Service  Set_Group_Static_Boost
VMM_Service  _GetRegistryPath, VMM_ICODE
VMM_Service  _GetRegistryKey
REGTYPE_ENUM	EQU	0
REGTYPE_CLASS	EQU	1
REGTYPE_VXD	EQU	2
REGKEY_OPEN	EQU	0
REGKEY_CREATE_IFNOTEXIST	EQU	1
ASSERT_RANGE_NULL_BAD	EQU	00000000H
ASSERT_RANGE_NULL_OK	EQU	00000001H
ASSERT_RANGE_NO_DEBUG	EQU	80000000H
ASSERT_RANGE_BITS	EQU	80000001H
VMM_Service  Cleanup_Thread_State
VMM_Service  _RegRemapPreDefKey
VMM_Service  End_V86_Serialization
VMM_Service  _Assert_Range
VMM_Service  _Sprintf
VMM_Service  _PageChangePager
VMM_Service  _RegCreateDynKey
VMM_Service  _RegQueryMultipleValues
VMM_Service  Boost_Thread_With_VM
BOOT_CLEAN	EQU	00000001H
BOOT_DOSCLEAN	EQU	00000002H
BOOT_NETCLEAN	EQU	00000004H
BOOT_INTERACTIVE	EQU	00000008H
VMM_Service  Get_Boot_Flags
VMM_Service  Set_Boot_Flags
VMM_Service  _lstrcpyn
VMM_Service  _lstrlen
VMM_Service  _lmemcpy
VMM_Service  _GetVxDName
VMM_Service  Force_Mutexes_Free
VMM_Service  Restore_Forced_Mutexes
VMM_Service  _AddReclaimableItem
VMM_Service  _SetReclaimableItem
VMM_Service  _EnumReclaimableItem
VMM_Service  Time_Slice_Wake_Sys_VM
VMM_Service  VMM_Replace_Global_Environment
VMM_Service  Begin_Non_Serial_Nest_V86_Exec
VMM_Service  Get_Nest_Exec_Status
VMM_Service  Open_Boot_Log
VMM_Service  Write_Boot_Log
VMM_Service  Close_Boot_Log
VMM_Service  EnableDisable_Boot_Log
VMM_Service  _Call_On_My_Stack
VMM_Service  Get_Inst_V86_Int_Vec_Base
VMM_Service  _lstrcmpi
VMM_Service  _strupr
VMM_Service  Log_Fault_Call_Out
VMM_Service  _AtEventTime
endif
End_Service_Table VMM, VMM
RS_RECLAIM	EQU	00000001H
RS_RESTORE	EQU	00000002H
RS_DOSARENA	EQU	00000004H

ReclaimStruc	STRUC
RS_Linear	DD	?
RS_Bytes	DD	?
RS_CallBack	DD	?
RS_RefData	DD	?
RS_HookTable	DD	?
RS_Flags	DD	?
ReclaimStruc	ENDS

frmtx	STRUC
frmtx_pfrmtxNext	DD	?
frmtx_hmutex	DD	?
frmtx_cEnterCount	DD	?
frmtx_pthcbOwner	DD	?
frmtx_htimeout	DD	?
frmtx	ENDS

vmmfrinfo	STRUC
vmmfrinfo_frmtxDOS	DB	SIZE frmtx DUP (?)
vmmfrinfo_frmtxV86	DB	SIZE frmtx DUP (?)
vmmfrinfo_frmtxOther	DB	SIZE frmtx DUP (?)
vmmfrinfo	ENDS

DemandInfoStruc	STRUC
DILin_Total_Count	DD	?
DIPhys_Count	DD	?
DIFree_Count	DD	?
DIUnlock_Count	DD	?
DILinear_Base_Addr	DD	?
DILin_Total_Free	DD	?
DIPage_Faults	DD	?
DIPage_Ins	DD	?
DIPage_Outs	DD	?
DIPage_Discards	DD	?
DIInstance_Faults	DD	?
DIPagingFileMax	DD	?
DIPagingFileInUse	DD	?
DICommit_Count	DD	?
DIReserved	DD	2 DUP (?)
DemandInfoStruc	ENDS

InstDataStruc	STRUC
InstLinkF	DD	0
InstLinkB	DD	0
InstLinAddr	DD	?
InstSize	DD	?
InstType	DD	?
InstDataStruc	ENDS
INDOS_FIELD	EQU	100H
ALWAYS_FIELD	EQU	200H
OPTIONAL_FIELD	EQU	400H

IPF_Data	STRUC
IPF_LinAddr	DD	?
IPF_MapPageNum	DD	?
IPF_PTEEntry	DD	?
IPF_FaultingVM	DD	?
IPF_Flags	DD	?
IPF_Data	ENDS

Exception_Handler_Struc	STRUC
EH_Reserved	DD	?
EH_Start_EIP	DD	?
EH_End_EIP	DD	?
EH_Handler	DD	?
Exception_Handler_Struc	ENDS
PR_PRIVATE	EQU	80000400H
PR_SHARED	EQU	80060000H
PR_SYSTEM	EQU	80080000H
PR_FIXED	EQU	00000008H
PR_4MEG	EQU	00000001H
PR_STATIC	EQU	00000010H
PD_ZEROINIT	EQU	00000001H
PD_NOINIT	EQU	00000002H
PD_FIXEDZERO	EQU	00000003H
PD_FIXED	EQU	00000004H
PC_FIXED	EQU	00000008H
PC_LOCKED	EQU	00000080H
PC_LOCKEDIFDP	EQU	00000100H
PC_WRITEABLE	EQU	00020000H
PC_USER	EQU	00040000H
PC_INCR	EQU	40000000H
PC_PRESENT	EQU	80000000H
PC_STATIC	EQU	20000000H
PC_DIRTY	EQU	08000000H
PCC_ZEROINIT	EQU	00000001H
PCC_NOLIN	EQU	10000000H
ifndef _WINNT_

_MEMORY_BASIC_INFORMATION	STRUC
mbi_BaseAddress	DD	?
mbi_AllocationBase	DD	?
mbi_AllocationProtect	DD	?
mbi_RegionSize	DD	?
mbi_State	DD	?
mbi_Protect	DD	?
mbi_Type	DD	?
_MEMORY_BASIC_INFORMATION	ENDS
PAGE_NOACCESS	EQU	01H
PAGE_READONLY	EQU	02H
PAGE_READWRITE	EQU	04H
MEM_COMMIT	EQU	1000H
MEM_RESERVE	EQU	2000H
MEM_FREE	EQU	10000H
MEM_PRIVATE	EQU	20000H
endif

pd_s	STRUC
pd_virginin	DD	?
pd_taintedin	DD	?
pd_cleanout	DD	?
pd_dirtyout	DD	?
pd_virginfree	DD	?
pd_taintedfree	DD	?
pd_dirty	DD	?
pd_type	DD	?
pd_s	ENDS
PD_SWAPPER	EQU	0
PD_PAGERONLY	EQU	1
PD_NESTEXEC	EQU	2
endif
PAGESHIFT	EQU	12
PAGESIZE	EQU	(1 SHL PAGESHIFT)
PAGEMASK	EQU	(PAGESIZE-1)
MAXSYSTEMLADDR	EQU	(0ffbfffffH)
MINSYSTEMLADDR	EQU	(0c0000000H)
MAXSHAREDLADDR	EQU	(0bfffffffH)
MINSHAREDLADDR	EQU	(80000000H)
MAXPRIVATELADDR	EQU	(7fffffffH)
MINPRIVATELADDR	EQU	(00400000H)
MAXDOSLADDR	EQU	(003fffffH)
MINDOSLADDR	EQU	(00000000H)
MAXSYSTEMPAGE	EQU	(MAXSYSTEMLADDR SHR PAGESHIFT)
MINSYSTEMPAGE	EQU	(MINSYSTEMLADDR SHR PAGESHIFT)
MAXSHAREDPAGE	EQU	(MAXSHAREDLADDR SHR PAGESHIFT)
MINSHAREDPAGE	EQU	(MINSHAREDLADDR SHR PAGESHIFT)
MAXPRIVATEPAGE	EQU	(MAXPRIVATELADDR SHR PAGESHIFT)
MINPRIVATEPAGE	EQU	(MINPRIVATELADDR SHR PAGESHIFT)
MAXDOSPAGE	EQU	(MAXDOSLADDR SHR PAGESHIFT)
MINDOSPAGE	EQU	(MINDOSLADDR SHR PAGESHIFT)
CBPRIVATE	EQU	(1+MAXPRIVATELADDR-MINPRIVATELADDR)
CBSHARED	EQU	(1+MAXSHAREDLADDR-MINSHAREDLADDR)
CBSYSTEM	EQU	(1+MAXSYSTEMLADDR-MINSYSTEMLADDR)
CBDOS	EQU	(1+MAXDOSLADDR-MINDOSLADDR)
CPGPRIVATE	EQU	(1+MAXPRIVATEPAGE-MINPRIVATEPAGE)
CPGSHARED	EQU	(1+MAXSHAREDPAGE-MINSHAREDPAGE)
CPGSYSTEM	EQU	(1+MAXSYSTEMPAGE-MINSYSTEMPAGE)
CPGDOS	EQU	(1+MAXDOSPAGE-MINDOSPAGE)
IFDEF DEBUG
DebFar  EQU NEAR PTR
ELSE
DebFar  EQU SHORT
ENDIF

ifndef Not_VxD
SYS_CRITICAL_INIT	EQU	0000H
DEVICE_INIT	EQU	0001H
INIT_COMPLETE	EQU	0002H
SYS_VM_INIT	EQU	0003H
SYS_VM_TERMINATE	EQU	0004H
SYSTEM_EXIT	EQU	0005H
SYS_CRITICAL_EXIT	EQU	0006H
CREATE_VM	EQU	0007H
VM_CRITICAL_INIT	EQU	0008H
VM_INIT	EQU	0009H
VM_TERMINATE	EQU	000AH
VM_NOT_EXECUTEABLE	EQU	000BH
DESTROY_VM	EQU	000CH
VNE_CRASHED_BIT	EQU	00H
VNE_CRASHED	EQU	(1 SHL VNE_CRASHED_BIT)
VNE_NUKED_BIT	EQU	01H
VNE_NUKED	EQU	(1 SHL VNE_NUKED_BIT)
VNE_CREATEFAIL_BIT	EQU	02H
VNE_CREATEFAIL	EQU	(1 SHL VNE_CREATEFAIL_BIT)
VNE_CRINITFAIL_BIT	EQU	03H
VNE_CRINITFAIL	EQU	(1 SHL VNE_CRINITFAIL_BIT)
VNE_INITFAIL_BIT	EQU	04H
VNE_INITFAIL	EQU	(1 SHL VNE_INITFAIL_BIT)
VNE_CLOSED_BIT	EQU	05H
VNE_CLOSED	EQU	(1 SHL VNE_CLOSED_BIT)
VM_SUSPEND	EQU	000DH
VM_RESUME	EQU	000EH
SET_DEVICE_FOCUS	EQU	000FH
BEGIN_MESSAGE_MODE	EQU	0010H
END_MESSAGE_MODE	EQU	0011H
REBOOT_PROCESSOR	EQU	0012H
QUERY_DESTROY	EQU	0013H
DEBUG_QUERY	EQU	0014H
BEGIN_PM_APP	EQU	0015H
BPA_32_BIT	EQU	01H
BPA_32_BIT_FLAG	EQU	1
END_PM_APP	EQU	0016H
DEVICE_REBOOT_NOTIFY	EQU	0017H
CRIT_REBOOT_NOTIFY	EQU	0018H
CLOSE_VM_NOTIFY	EQU	0019H
CVNF_CRIT_CLOSE_BIT	EQU	0
CVNF_CRIT_CLOSE	EQU	(1 SHL CVNF_CRIT_CLOSE_BIT)
POWER_EVENT	EQU	001AH
SYS_DYNAMIC_DEVICE_INIT	EQU	001BH
SYS_DYNAMIC_DEVICE_EXIT	EQU	001CH
CREATE_THREAD	EQU	001DH
THREAD_INIT	EQU	001EH
TERMINATE_THREAD	EQU	001FH
THREAD_Not_Executeable	EQU	0020H
DESTROY_THREAD	EQU	0021H
PNP_NEW_DEVNODE	EQU	0022H
W32_DEVICEIOCONTROL	EQU	0023H
DIOC_GETVERSION	EQU	0H
DIOC_OPEN	EQU	DIOC_GETVERSION
DIOC_CLOSEHANDLE	EQU	-1
SYS_VM_TERMINATE2	EQU	0024H
SYSTEM_EXIT2	EQU	0025H
SYS_CRITICAL_EXIT2	EQU	0026H
VM_TERMINATE2	EQU	0027H
VM_NOT_EXECUTEABLE2	EQU	0028H
DESTROY_VM2	EQU	0029H
VM_SUSPEND2	EQU	002AH
END_MESSAGE_MODE2	EQU	002BH
END_PM_APP2	EQU	002CH
DEVICE_REBOOT_NOTIFY2	EQU	002DH
CRIT_REBOOT_NOTIFY2	EQU	002EH
CLOSE_VM_NOTIFY2	EQU	002FH
GET_CONTENTION_HANDLER	EQU	0030H
KERNEL32_INITIALIZED	EQU	0031H
KERNEL32_SHUTDOWN	EQU	0032H
MAX_SYSTEM_CONTROL	EQU	0032H
BEGIN_RESERVED_PRIVATE_SYSTEM_CONTROL	EQU	70000000H
END_RESERVED_PRIVATE_SYSTEM_CONTROL	EQU	7FFFFFFFH
endif
SYSSTATE_PRESYSCRITINIT	EQU	00000000H
SYSSTATE_PREDEVICEINIT	EQU	10000000H
SYSSTATE_PREINITCOMPLETE	EQU	20000000H
SYSSTATE_VXDINITCOMPLETED	EQU	40000000H
SYSSTATE_KERNEL32INITED	EQU	50000000H
SYSSTATE_KERNEL32TERMINATED	EQU	0A0000000H
SYSSTATE_PRESYSVMTERMINATE	EQU	0B0000000H
SYSSTATE_PRESYSTEMEXIT	EQU	0E0000000H
SYSSTATE_PRESYSTEMEXIT2	EQU	0E4000000H
SYSSTATE_PRESYSCRITEXIT	EQU	0F0000000H
SYSSTATE_PRESYSCRITEXIT2	EQU	0F4000000H
SYSSTATE_POSTSYSCRITEXIT2	EQU	0FFF00000H
SYSSTATE_PREDEVICEREBOOT	EQU	0FFFF0000H
SYSSTATE_PRECRITREBOOT	EQU	0FFFFF000H
SYSSTATE_PREREBOOTCPU	EQU	0FFFFFF00H
BeginDoc











































































EndDoc












??_pf_Check equ 1
??_pf_ArgsUsed  equ 2
??_pf_Entered   equ 4
??_pf_Left  equ 8
??_pf_Returned  equ    16

??_pushed       =       0
??_align    =   0
??_ends     equ <>

BeginProc macro Name, P1, P2, P3, P4, P5, P6, P7, LastArg
    local   Profile_Data, prelabeldata, ??_hookvar
    ??_frame = 0
    ??_aframe = 0
    ??_initaframe = 0
    ??_numargs = 0
    ??_numlocals = 0
    ??_numlocalsymbols = 0
    ??_procflags = 0
    ??_esp = 0
    ??_pushed = 0
    ??_align = 0
    ??_hook = 0
    ??_hookarg = 0
    ??_service = 0
    ??_async_service = 0
IF DEBLEVEL GT DEBLEVELNORMAL
    ??_log = DFS_LOG
    ??_profile = DFS_PROFILE
    ??_test_cld = DFS_TEST_CLD
ELSE
    ??_log = 0
IFDEF DEBUG
IFDEF profileall
IF ?_ICODE
    ??_profile = DFS_PROFILE
ELSE
    ??_profile = 0
ENDIF
ELSE
    ??_profile = 0
ENDIF
ELSE
    ??_profile = 0
ENDIF
    ??_test_cld = 0
ENDIF
    ??_might_block = 0
    ??_test_reenter = 0
    ??_never_reenter = 0
    ??_not_swapping = 0
    ??_prolog_disabled = 0
    ??_public = 1
    ??_cleanoff = 0
    ??_ccall = 0
    ??_pcall = 0
    ??_scall = 0
    ??_w32svc = 0
    ??_fleave = FALSE

    ??_name equ <Name>

    .errnb ??_ends, <Cannot nest functions with named segments>
    .errnb <LastArg>, <Too many arguments to BeginProc>


    irp arg, <P1, P2, P3, P4, P5, P6, P7>
        if ??_hookarg
        ??_hookarg = 0
        ??_hookvar equ <arg>
        elseifdef ?&arg&_BeginProc
            ?&arg&_BeginProc
        elseifdef VxD_&arg&_CODE_SEG
        ??_ends textequ <VxD_&arg&_CODE_ENDS>
        VxD_&arg&_CODE_SEG
        else
        .err <Bad param "&arg" to BeginProc>
        endif
    endm


    ifndef Not_VxD
	ife ??_service
	    ifndef profileall
	      ??_profile = 0
	    endif
	    ifdef VMMSYS
		??_prolog_disabled = 1
	    endif
	else
	    ??_test_cld = DFS_TEST_CLD
	endif

	ife ?_16ICODE
	    ??_prolog_disabled = 1
	else
	ife ?_RCODE
	    ??_prolog_disabled = 1
	else
	    ife ?_PCODE
		??_might_block = DFS_TEST_BLOCK
	    endif
	    if ??_service
		ife ??_async_service
		    ??_test_reenter = DFS_TEST_REENTER
		endif
	    endif
    endif
    endif
    endif

    if ??_esp

	??_basereg equ <esp+??_pushed>
	??_initaframe = 4
    else

	??_basereg equ <ebp>
	??_initaframe = 8
    endif
    @Caller equ <dword ptr [??_basereg+??_initaframe-4]>

    ??_cleanoff = ??_pcall or ??_scall


    ??_dfs = ??_never_reenter + ??_test_reenter + ??_not_swapping + \
         ??_log + ??_profile + ??_test_cld + ??_might_block

    if ??_prolog_disabled
        ??_dfs = 0
    endif



    ifndef Not_VxD

	if ??_hook
	    if ??_align
	    Dword_Align
	    endif
	    prelabeldata:
	    ifndef ??_hookvar
	    .err <HOOK_PROC requires next arg to be name of dword location>
	    endif
	    jmp short Name
	    jmp [??_hookvar]
	    ifdef DEBUG
	    Profile_Data dd  0
	    endif
	    if ??_align
	    .errnz ($ - prelabeldata) mod 4
	    endif
	endif

	ifdef DEBUG
	    ?prolog_&Name label near
	    if (??_service OR ??_profile) AND (??_hook EQ 0)
	    jmp short Name
	    if ??_align
		Dword_Align
	    endif

	    IF ?_ICODE
	    ifdef profileall
	      ?ProfileHeader_BeginProc Profile_Data, %@filename
	    else
	      Profile_Data dd 0
	    endif
	    ELSE
	      Profile_Data dd 0
	    ENDIF

	    endif
	endif

	if ??_align
	    Dword_Align
	endif

    endif

    Name proc near



    ife ??_pcall or ??_ccall or ??_scall
        if ??_public
            public Name
        else
            ifdef DEBUG
                % ?merge @FileName,$,Name,:
                % ?merge public,,,,,@FileName,$,Name
            endif
        endif
    endif
    if ??_ccall
        if ??_public
            _&Name equ Name
	    ifdef Not_VxD
		 public C Name
	    else
                 public _&Name
	    endif
        endif
    endif
    if ??_pcall
        if ??_public
            ?toupper Name
            ?merge  public,,,,%?upper
        endif
    endif


    ifndef Not_VxD
	ifdef DEBUG
	    if ??_dfs EQ DFS_LOG
		VMMCall Log_Proc_Call
	    else
	    if ??_dfs EQ DFS_TEST_REENTER
		VMMCall Test_Reenter
	    else
	    if ??_dfs or ?_LOCKABLECODE eq 0
		ifdef WIN31COMPAT
		    if ??_dfs AND DFS_LOG
			VMMCall Log_Proc_Call
		    endif
		    if ??_dfs AND DFS_TEST_REENTER
			VMMCall Test_Reenter
		    endif
		else
		    ife ?_LOCKABLECODE
		    ifdef ??_debug_flags
			push    ??_debug_flags
			if ??_dfs
			pushfd
			or  dword ptr [esp+4],??_dfs
			popfd
			endif
			VMMCall _Debug_Flags_Service
		    elseif ??_dfs
			push    ??_dfs
			VMMCall _Debug_Flags_Service
		    endif
		    else
		    push    ??_dfs
		    VMMCall _Debug_Flags_Service
		    endif
		endif
	    else
	      ifdef profileall
		IncProfileCount
	      endif
	    endif
	    endif
	    endif
	endif
    endif



endm






?_BeginProc macro
endm

?PUBLIC_BeginProc macro
    ??_public = 1
endm

?LOCAL_BeginProc macro
    ??_public = 0
endm

?HIGH_FREQ_BeginProc macro
    ??_align = 1
endm

?HOOK_PROC_BeginProc macro
    ??_hook = 1
    ??_hookarg = 1
endm

?SERVICE_BeginProc macro
    ??_service = 1
    .erre ?_16ICODE, <SERVICEs must be in 32 bit code>
    .erre ?_RCODE, <SERVICEs must be in 32 bit code>
endm

?ASYNC_SERVICE_BeginProc macro
    ??_service = 1
    ??_async_service = 1
    .errnz ?_LCODE, <ASYNC_SERVICE's must be in LOCKED code>
endm

?NO_LOG_BeginProc macro
    ??_log = 0
endm

?NO_PROFILE_BeginProc macro
    ??_profile = 0
endm

?NO_TEST_CLD_BeginProc macro
    ??_test_cld = 0
endm

?TEST_BLOCK_BeginProc macro
    ??_might_block = DFS_TEST_BLOCK
endm

?TEST_REENTER_BeginProc macro
    ??_test_reenter = DFS_TEST_REENTER
endm

?NEVER_REENTER_BeginProc macro
    ??_never_reenter = DFS_NEVER_REENTER
endm

?NOT_SWAPPING_BeginProc macro
    ??_not_swapping = DFS_NOT_SWAPPING
endm

?NO_PROLOG_BeginProc macro
    ??_prolog_disabled = 1
endm

?ESP_BeginProc macro
    ??_esp = VMM_TRUE
    ifndef Not_VxD
	.erre ?_16ICODE, <Beginproc ESP attribute invalid in 16 bit seg.>
	.erre ?_RCODE, <Beginproc ESP attribute invalid in real-mode seg.>
    endif
endm

?CCALL_BeginProc macro
    ??_ccall = 1
endm

?PCALL_BeginProc macro
    ??_pcall = 1
endm

?SCALL_BeginProc macro
    ??_scall = 1
endm

?ICALL_BeginProc macro
    ??_scall = 1
endm

?W32SVC_BeginProc macro
    ??_scall = 1
    ??_w32svc = 1
endm

ifdef DEBUG
ifdef profileall
?ProfileHeader_BeginProc macro PL, filename
ifndef _&filename&__proc_list
  _&filename&__proc_list = 0
  PUBLIC _&filename&__proc_list
endif
    dd OFFSET32 _&filename&__proc_list
PL  dd 0
_&filename&__proc_list = PL
endm
endif

IncProfileCount macro
    if ??_service OR ??_profile
        inc dword ptr [??_name-4]
    else
        ifndef profileall
        .err <IncProfileCount can be used only in services.>
        endif
    endif
endm
else
IncProfileCount macro
endm
endif
















ArgVar  macro   name,length,used
    ??_numargs = ??_numargs + 1
    if ??_pcall
        ?mkarg  <name>, <length>, <used>, %??_numargs
    else
        ?argvar <name>, <length>, <used>
    endif
    ??_procflags = ??_procflags OR ??_pf_Check
    endm

?mkarg  macro   name, length, used, num
    .xcref  ?MKA&num
    ?deflocal <name>
    ?MKA&num &macro
        ?argvar <name>, <length>, <used>
        &endm
    ??_aframe = ??_aframe + 4
    endm
    .xcref  ?mkarg

?argvar macro   name,length,used
    local   a
    ifidni  <length>,<BYTE>
        a = ??_aframe
        ??_aframe =  ??_aframe + 4
        ?setname <name>, <byte ptr [??_basereg+??_initaframe+a]>, <used>
    elseifidni <length>,<WORD>
        a = ??_aframe
        ??_aframe =  ??_aframe + 4
        ?setname <name>, <word ptr [??_basereg+??_initaframe+a]>, <used>
    elseifidni <length>,<DWORD>
        a = ??_aframe
        ??_aframe =  ??_aframe + 4
        ?setname <name>,  <dword ptr [??_basereg+??_initaframe+a]>, <used>
        ?setname <name&l>,<word ptr [??_basereg+??_initaframe+a]>, <used>
        ?setname <name&ll>,<byte ptr [??_basereg+??_initaframe+a]>, <used>
        ?setname <name&lh>,<byte ptr [??_basereg+??_initaframe+a+1]>, <used>
        ?setname <name&h>,<word ptr [??_basereg+??_initaframe+a+2]>, <used>
        ?setname <name&hl>,<byte ptr [??_basereg+??_initaframe+a+2]>, <used>
        ?setname <name&hh>,<byte ptr [??_basereg+??_initaframe+a+3]>, <used>
    else
        a = ??_aframe
        ??_aframe =  ??_aframe + ((length + 3)/4)*4
        ?setname <name>, <[??_basereg+??_initaframe+a]>, <used>
    endif
endm






?setname macro name, value, used
    ?deflocal <name>
    ifidni <used>, <NOTUSED>
        name equ _inaccessible_NOTUSED_
    else
        name equ value
        ??_procflags = ??_procflags OR ??_pf_ArgsUsed OR ??_pf_Check
    endif
endm



















LocalVar    macro   name,length,flag
    local   a
    ??_numlocals = ??_numlocals + 1
    ??_pad = 1
    ifidni <flag>, <PACK>
        ??_pad = 0
    endif
    ifidni  <length>,<BYTE>
        ??_frame = ??_frame + 1 + 3 * ??_pad
        a = ??_frame
        ?deflocal <name>
        name equ byte ptr [??_basereg-a]
    elseifidni <length>,<WORD>
        ??_frame =  ??_frame + 2 + 2 * ??_pad
        a = ??_frame
        ?deflocal <name>
        name equ word ptr [??_basereg-a]
    elseifidni <length>,<DWORD>
        ??_frame = ??_frame + 4
        a = ??_frame
        ?deflocal <name, name&l, name&ll, name&lh, name&h, name&hl, name&hh>
        name equ dword ptr [??_basereg-a]
        name&l equ word ptr [??_basereg-a]
        name&ll equ byte ptr [??_basereg-a]
        name&lh equ byte ptr [??_basereg-a+1]
        name&h equ word ptr [??_basereg-a+2]
        name&hl equ byte ptr [??_basereg-a+2]
        name&hh equ byte ptr [??_basereg-a+3]
    else
        ??_frame =  ??_frame + ((length + 3)/4)*4
        a = ??_frame
        ?deflocal <name>
        name equ [??_basereg-a]
    endif
    ??_procflags = ??_procflags OR ??_pf_Check
endm

?deflocal macro name
    irp nm, <name>
        ??_numlocalsymbols = ??_numlocalsymbols + 1
        ?dodeflocal <nm>, %(??_numlocalsymbols)
    endm
endm
    .xcref  ?deflocal

?dodeflocal macro name, num
    .xcref  ?LOC&num
    ?LOC&num &macro
        name    equ <__inaccessible__NOTINSCOPE__>
        &endm
    endm
    .xcref  ?dodeflocal



EnterProc macro
    .errnz ??_frame and 3, <Total size of local variables not a multiple of 4.>
    if ??_scall
        if ??_public
	    ifdef Not_VxD
                ?merge  %??_name,@,%(??_aframe),,label,near
                ?merge  public,,,,C,%??_name,@,%(??_aframe)
	    else
                ?merge  _,%??_name,@,%(??_aframe),label,near
                ?merge  public,,,,,_,%??_name,@,%(??_aframe)
	    endif
        endif
    endif
    if ??_pcall
        ??_aframe = 0
        ?count = ??_numargs
        rept    ??_numargs
            ?invprg <?MKA>,%?count
            ?count = ?count - 1
        endm
    endif
    ??_fleave = FALSE
    if ??_esp
        if  ??_frame
            sub esp, ??_frame
            ??_pushed = ??_pushed + ??_frame
            ??_fleave = VMM_TRUE
        endif
    else
        if  ??_frame eq 0
            if (??_aframe eq 0) OR ((??_procflags AND ??_pf_ArgsUsed) EQ 0)
                ifdef DEBUG
                    push    ebp
                    mov ebp,esp
                    ??_fleave = VMM_TRUE
                endif
            else
                push    ebp
                mov ebp,esp
                ??_fleave = VMM_TRUE
            endif
        else
            enter   ??_frame, 0
            ??_fleave = VMM_TRUE
        endif
    endif
    ??_procflags = ??_procflags OR ??_pf_Entered
endm















LeaveProc macro flags
    if ??_fleave
        if ??_esp
            ifidni <flags>,<PRESERVE_FLAGS>
                lea esp,[esp + ??_frame]
            else
                add esp,??_frame
            endif
        else
            leave
        endif
    endif
    ??_procflags = ??_procflags OR ??_pf_Left
endm







Return  macro
    if  ??_cleanoff OR ??_w32svc
        if  ??_w32svc AND (??_aframe LT 8)
            ret 8
        else
            ret ??_aframe
        endif
    else
        ret
    endif
    ??_procflags = ??_procflags OR ??_pf_Returned
    endm




EndProc macro Name, Flag
    Name endp
if ??_w32svc
    if ??_aframe lt 8
        cparm&Name equ 0
    else
        cparm&Name equ (??_aframe/4 - 2)
    endif
endif
if ??_procflags AND ??_pf_Left
if ??_fleave
if ??_esp
    ??_pushed = ??_pushed - ??_frame
endif
endif
endif
if
iame
    irp nm, <nam-i   irp nm, <nam-i   irp nm,ndifE<nam-i   irp nm,ndifE irp  ir nm,ndifrp n2	EQU	002CH'   Ltif
  	002CH'   m,ndiLtght_bl '   m,ndiLEY_CLASe  ?MrgdiLEYVATEPAGE)
CPGSHAR&TiLEYr_bl '   m,ndiLEY_CLAS m,ndiLtght_bl Y_CLA
    .errnz ??_frame and 3, <Total size of local variables not ags LAS m,nd not not ags agsd noal vam,nd agsd noal vam,nd a,nd t notvam,nd a,nd t notvaE t FALSE,nd a,nd t notvaE t FALotvaE t FALSE,ne  %?aE t EaE t FALotvD
    PE__>
        &endaE t  =   0
?     endifp,??_frds,  =   0
  a =ifp,?Nroc macr 0
  a =ifp,?Nr,ndiLtghtt FALSE,nd a,nd   0
  andiLtghtt F?_frame
      
    iT)
d4023H
DIOC_qDsd n
   
    H
DIOC?NrocdoDC_qDsd n

endm
OC?NrocdoDC_doDC_qDsdEl vam,nd a,nd t notvam,nd a,nd t notvaE t FALSE,nd a,nd t notvaE t FALotvaE t FALSE,ne  %?aE t EaE t FALotvD
    PE__>
        &endaE t  =   0
?     endifp,??_frds,  =   0
  a =ifp,?Nroc macr 0
  a =ifp,?Nr,ndiLtghtt FALSE,nd a,nd   0
  an.err <Bad param "&d a,nd ifpd = E,nd a,nd ifpd = E,nd a,nd ifd ifp ifpdp,esnd b ifp ifpdp,Xsnd OCK
nd a,nd ifpd = E,nd a,nd ifd ifp ifpdp,esnd b ifp ifpdp,Xsnd b ifp ifpdp,Xsndnd bXsnd
	  fp 


Rnd if


Rnd if


Rnd if		ifdf


Rnd if		ifdf
??_p   if ??_esp
     ??_p ifdf
?NO_P ??_p ifdf
?NO_P ???_pp,esf
?,[esP ???ifdf
?
?NOfdf
?ifdf
Ti   T

Rp ifdi= VM_puP ??_equ _inaccessible_NOT
  f
?if_inacee_NOT
 ces
	    if ??   f
?if_i??_equ _e??   f
ifdf
??NOa,nd ifpd e??   f
if      ,nd i2   ??_  f
?fdf
Ti    T
ne  %?words	    _NOfdf
?ifdf
pe  %?aE t  U?g-H8Wds	  nd if


Rnd if		ifdf

log_2   _NOfdR1  f
ifdfH  if Af		ifdf
   T
ne?   f
8f     FALSE,nmSIsIC _&filename&df
Ti    T
p,?Eifdf
?
?NOfdf
?ifdf
TiFS_LO2p U?g-H8Wds	  nd ind if


Rnd ifd if		ifdf

log_2>,  <dword ptr 
?
?NOfdf
ER+tZ  FALSE,nmSIsIC _&filename&df
Ti    T
p,?Eifdf
?
   if ??   ff
?
?NOfdf
?if a = ??_aframe
 AL_EXIT2	EQUkE_afrc_EXIce
	EXITame&
?NOfdf
?if a = ??_aframe
 AL_EXIT2	frame
 AL_EXIT2_proTame
s AND ??_pf_Left
i AND f a = ??_aframe
 AL_frame
 AL_frame
