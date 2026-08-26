;; ====================================================================
;; os2ddk.inc — OS/2 DDK definitions for WASM (OpenWatcom assembler)
;; ====================================================================
;; Extracted from OS/2 Toolkit devhdr.inc, devcmd.inc, devhlp.inc
;; Rewritten in WASM-compatible syntax (no MASM-only macros).
;; ====================================================================

;; Device Header Attribute Flags
DEV_CHAR_DEV    EQU     8000h
DEV_IOCTL       EQU     4000h
DEV_NON_IBM     EQU     2000h
DEV_SHARE       EQU     1000h
DEV_30          EQU     0800h
DEV_GIOCTL      EQU     0040h
DEV_INITCOMPLETE EQU    0010h
DEV_IOCTL2      EQU     0001h

;; Strategy Command Codes
CMDInit         EQU     0
CMDInput        EQU     4
CMDNDR          EQU     5
CMDInputS       EQU     6
CMDInputF       EQU     7
CMDOutput       EQU     8
CMDOutputV      EQU     9
CMDOutputS      EQU     10
CMDOutputF      EQU     11
CMDOpen         EQU     13
CMDClose        EQU     14
CMDGenIOCTL     EQU     16
CMDDeInstall    EQU     20
CMDShutdown     EQU     28
CMDInitComplete EQU     31

;; DevHlp Function Codes
DevHlp_DevDone          EQU     1
DevHlp_ProcBlock        EQU     4
DevHlp_ProcRun          EQU     5
DevHlp_AllocPhys        EQU     24
DevHlp_FreePhys         EQU     25
DevHlp_SetIRQ           EQU     27
DevHlp_UnSetIRQ         EQU     28
DevHlp_EOI              EQU     49
DevHlp_PhysToVirt       EQU     21
DevHlp_VirtToPhys       EQU     22
DevHlp_PhysToGDTSel     EQU     84
DevHlp_Save_Message     EQU     61
DevHlp_VerifyAccess     EQU     39

;; ASYNC IOCtl Category and Function Codes
IOCTL_ASYNC     EQU     01h
