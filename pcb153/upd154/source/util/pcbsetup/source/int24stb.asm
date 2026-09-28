; INT24STB.ASM - INT 23/24 handler stubs for PCBSETUP
.MODEL LARGE
.DATA
PUBLIC _int24hnd, _int23hnd, _Int24Error, _Int24Flags, _oldint24hnd
_Int24Error DW 0
_Int24Flags DW 0
_oldint24hnd DD 0
.CODE
_int24hnd PROC FAR
    MOV AL,3
    IRET
_int24hnd ENDP
_int23hnd PROC FAR
    IRET
_int23hnd ENDP
END
