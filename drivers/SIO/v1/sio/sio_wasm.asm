        .386p
        TITLE   SIO - Serial I/O Device Driver for OS/2

;; Constants
DEVATTR         EQU     0D840h
CMDInit         EQU     0
CMDInitComplete EQU     31
DevHlp_EOI      EQU     49

_DATA   SEGMENT WORD PUBLIC USE16 'DATA'

;; Device Header (must be at offset 0)
SIODevHdr       LABEL BYTE
        DD      -1
        DW      DEVATTR
        DW      OFFSET _TEXT:Strategy
        DW      0
        DB      'SIO$    '
        DW      0, 0, 0, 0
        DD      0011h

NumPorts        DW      0
DevHlpEntry     DD      0
ReqPktOff       DW      0
ReqPktSeg       DW      0

InitMsg         DB      'SIO v1.00 Serial I/O Driver for OS/2', 0Dh, 0Ah, '$'
EndOfData       LABEL BYTE
_DATA   ENDS

_TEXT   SEGMENT WORD PUBLIC USE16 'CODE'
        ASSUME  CS:_TEXT, DS:_DATA

;; Strategy Entry
Strategy:
        mov     word ptr ds:[ReqPktOff], bx
        mov     word ptr ds:[ReqPktSeg], es

        mov     al, es:[bx+2]
        xor     ah, ah
        cmp     al, CMDInitComplete
        ja      short BadCmd

        cmp     al, CMDInit
        je      short DoInit

        ; All other commands: return OK
        xor     ax, ax
        jmp     short StratExit

BadCmd:
        mov     ax, 8103h

StratExit:
        les     bx, dword ptr ds:[ReqPktOff]
        or      ax, 0100h
        mov     es:[bx+3], ax
        retf

;; INIT command
DoInit:
        les     bx, dword ptr ds:[ReqPktOff]

        ; Save DevHlp
        mov     eax, es:[bx+12h]
        mov     ds:[DevHlpEntry], eax

        ; Print banner
        push    dx
        mov     dx, OFFSET _DATA:InitMsg
        mov     ah, 09h
        int     21h
        pop     dx

        ; Set end pointers
        les     bx, dword ptr ds:[ReqPktOff]
        mov     word ptr es:[bx+0Eh], OFFSET _TEXT:EndOfCode
        mov     word ptr es:[bx+10h], OFFSET _DATA:EndOfData

        xor     ax, ax
        jmp     short StratExit

EndOfCode       LABEL BYTE
_TEXT   ENDS
        END
