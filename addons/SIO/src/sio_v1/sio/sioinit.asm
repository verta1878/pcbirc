;; ====================================================================
;; sioinit.asm — Driver Initialization (CMDInit)
;; ====================================================================
;; Called once at boot time by OS/2.
;; Request packet contains:
;;   Offset 0Eh: DWORD — pointer to INIT arguments (CONFIG.SYS line)
;;   Offset 12h: DWORD — DevHlp entry point
;; We must set:
;;   Offset 0Eh: WORD — end of code segment
;;   Offset 10h: WORD — end of data segment
;; ====================================================================

CmdInit:
        les     bx, DWORD PTR [ReqPktOff]

        ;; Save DevHlp entry point — we need this for everything
        mov     eax, es:[bx+12h]
        mov     [DriverData.ddDevHlp], eax

        ;; Print banner
        call    PrintMsg
        DW      OFFSET InitMsg

        ;; Parse CONFIG.SYS command line
        ;; ES:BX+0Eh → far pointer to argument string
        push    es
        push    bx
        les     bx, es:[bx+0Eh]
        call    ParseCommandLine
        pop     bx
        pop     es

        ;; If no ports were explicitly configured, default to COM1+COM2
        cmp     [DriverData.ddNumPorts], 0
        jne     @@HavePorts
        call    ConfigDefaultPorts
@@HavePorts:

        ;; Probe each configured port for UART hardware
        call    ProbeAllPorts

        ;; Allocate ring buffer memory for each detected port
        call    AllocateBuffers

        ;; Patch COMn device header names and chain them
        call    ChainDeviceHeaders

        ;; Set code and data end pointers in request packet
        ;; Everything after the init code can be discarded
        les     bx, DWORD PTR [ReqPktOff]
        mov     WORD PTR es:[bx+0Eh], OFFSET EndOfCode
        mov     WORD PTR es:[bx+10h], OFFSET EndOfData

        xor     ax, ax                  ; STATUS_OK
        jmp     StratExit


;; ====================================================================
;; ConfigDefaultPorts — Set up COM1 and COM2 at standard addresses
;; ====================================================================

ConfigDefaultPorts:
        ;; COM1: 3F8h, IRQ4
        lea     si, [DriverData.ddPorts]
        mov     [si].pdIOBase, COM1_BASE
        mov     [si].pdIOBaseDOS, COM1_BASE
        mov     [si].pdIRQ, COM1_IRQ
        mov     [si].pdIRQDOS, COM1_IRQ
        mov     [si].pdIsInternet, 0
        mov     [si].pdBaudLocked, 0
        mov     [si].pdProtMode, 0

        ;; COM2: 2F8h, IRQ3
        add     si, SIZE PORTDATA
        mov     [si].pdIOBase, COM2_BASE
        mov     [si].pdIOBaseDOS, COM2_BASE
        mov     [si].pdIRQ, COM2_IRQ
        mov     [si].pdIRQDOS, COM2_IRQ
        mov     [si].pdIsInternet, 0
        mov     [si].pdBaudLocked, 0
        mov     [si].pdProtMode, 0

        mov     [DriverData.ddNumPorts], 2
        ret


;; ====================================================================
;; ParseCommandLine — Parse CONFIG.SYS DEVICE= arguments
;; ====================================================================
;; In:  ES:BX → null-terminated argument string
;;      Format: (COM1[[:baud],addr[:dosaddr],irq[:dosirq][,prot]]) ...
;; ====================================================================

ParseCommandLine:
        push    si
        push    di
        mov     [DriverData.ddNumPorts], 0

@@ScanLoop:
        ;; Find next '('
        mov     al, es:[bx]
        test    al, al
        jz      @@ParseDone             ; End of string
        cmp     al, '('
        je      @@FoundParen
        inc     bx
        jmp     @@ScanLoop

@@FoundParen:
        inc     bx                      ; Skip '('
        call    ParseOnePort
        jmp     @@ScanLoop

@@ParseDone:
        pop     di
        pop     si
        ret


;; ====================================================================
;; ParseOnePort — Parse a single (COMn,...) definition
;; ====================================================================
;; In:  ES:BX → text after '('
;; Out: BX advanced past closing ')'
;;      DriverData updated with new port
;; ====================================================================

ParseOnePort:
        push    si

        ;; Get pointer to next available PORTDATA slot
        mov     ax, [DriverData.ddNumPorts]
        cmp     ax, MAX_PORTS
        jae     @@SkipPort              ; Too many ports
        mov     cx, SIZE PORTDATA
        mul     cx
        lea     si, [DriverData.ddPorts]
        add     si, ax

        ;; Zero out the port structure
        push    di
        push    es
        push    ds
        pop     es
        mov     di, si
        mov     cx, SIZE PORTDATA
        xor     al, al
        rep     stosb
        pop     es
        pop     di

        ;; Parse "COMn" — expect 'C','O','M' followed by digit
        cmp     BYTE PTR es:[bx], 'C'
        jne     @@SkipPort
        cmp     BYTE PTR es:[bx+1], 'O'
        jne     @@SkipPort
        cmp     BYTE PTR es:[bx+2], 'M'
        jne     @@SkipPort
        add     bx, 3

        ;; Get port number digit(s)
        call    ParseDecimal            ; AX = port number
        ;; Store as 0-based index — but we use ddNumPorts as the slot

        ;; Check for locked baud ":baud" after COMn
        cmp     BYTE PTR es:[bx], ':'
        jne     @@NoLockedBaud
        inc     bx
        call    ParseDecimal            ; AX = locked baud rate
        mov     WORD PTR [si].pdBaudRate, ax
        mov     [si].pdBaudLocked, 1
@@NoLockedBaud:

        ;; Skip comma separator
        cmp     BYTE PTR es:[bx], ','
        jne     @@DefaultAddr
        inc     bx

        ;; Parse I/O address or "INTERNET" or "Tport@addr"
        ;; Check for INTERNET keyword
        cmp     BYTE PTR es:[bx], 'I'
        jne     @@NotInternet
        cmp     BYTE PTR es:[bx+1], 'N'
        jne     @@NotInternet
        cmp     BYTE PTR es:[bx+2], 'T'
        jne     @@NotInternet
        ;; Matches "INT..." — confirm "INTERNET" (8 chars)
        mov     [si].pdIsInternet, 1
        or      [si].pdFlags, PF_INTERNET
        mov     [si].pdIRQ, 0FFh        ; No real IRQ
        ;; Skip to colon or comma
@@SkipInet:
        mov     al, es:[bx]
        cmp     al, ','
        je      @@AfterAddr
        cmp     al, ':'
        je      @@InetDosAddr
        cmp     al, ')'
        je      @@EndPort
        inc     bx
        jmp     @@SkipInet
@@InetDosAddr:
        inc     bx
        call    ParseHex                ; AX = DOS virtual address
        mov     [si].pdIOBaseDOS, ax
        jmp     @@AfterAddr

@@NotInternet:
        ;; Parse hex I/O address
        call    ParseHex                ; AX = I/O port address
        mov     [si].pdIOBase, ax
        mov     [si].pdIOBaseDOS, ax    ; Default DOS addr = real addr

        ;; Check for ":dosaddr" mapping
        cmp     BYTE PTR es:[bx], ':'
        jne     @@AfterAddr
        inc     bx
        call    ParseHex
        mov     [si].pdIOBaseDOS, ax

@@AfterAddr:
        jmp     @@DefaultAddr           ; Fall through (skip to comma)

@@DefaultAddr:
        ;; If no address given, use standard for this port number
        cmp     [si].pdIOBase, 0
        jne     @@ParseIRQ
        ;; Set default based on port index
        mov     ax, [DriverData.ddNumPorts]
        cmp     ax, 0
        jne     @@Def2
        mov     [si].pdIOBase, COM1_BASE
        mov     [si].pdIOBaseDOS, COM1_BASE
        jmp     @@ParseIRQ
@@Def2: cmp     ax, 1
        jne     @@Def3
        mov     [si].pdIOBase, COM2_BASE
        mov     [si].pdIOBaseDOS, COM2_BASE
        jmp     @@ParseIRQ
@@Def3: cmp     ax, 2
        jne     @@Def4
        mov     [si].pdIOBase, COM3_BASE
        mov     [si].pdIOBaseDOS, COM3_BASE
        jmp     @@ParseIRQ
@@Def4: mov     [si].pdIOBase, COM4_BASE
        mov     [si].pdIOBaseDOS, COM4_BASE

@@ParseIRQ:
        ;; Skip to comma or ')'
        cmp     BYTE PTR es:[bx], ','
        jne     @@CheckEnd
        inc     bx

        ;; Parse IRQ: "IRQn", "NONE", or just a number
        cmp     BYTE PTR es:[bx], 'N'
        jne     @@NotNone
        ;; "NONE" — skip 4 chars
        add     bx, 4
        mov     [si].pdIRQ, 0FFh
        jmp     @@CheckDosIRQ
@@NotNone:
        cmp     BYTE PTR es:[bx], 'I'
        jne     @@IRQNum
        add     bx, 3                  ; Skip "IRQ"
@@IRQNum:
        call    ParseDecimal
        mov     [si].pdIRQ, al
        mov     [si].pdIRQDOS, al       ; Default DOS IRQ = real IRQ

@@CheckDosIRQ:
        ;; Check for ":dosirq"
        cmp     BYTE PTR es:[bx], ':'
        jne     @@CheckProt
        inc     bx
        cmp     BYTE PTR es:[bx], 'N'
        jne     @@NotNone2
        add     bx, 4
        mov     [si].pdIRQDOS, 0FFh
        jmp     @@CheckProt
@@NotNone2:
        cmp     BYTE PTR es:[bx], 'I'
        jne     @@DIRQNum
        add     bx, 3
@@DIRQNum:
        call    ParseDecimal
        mov     [si].pdIRQDOS, al

@@CheckProt:
        ;; Check for protection mode parameter
        cmp     BYTE PTR es:[bx], ','
        jne     @@CheckEnd
        inc     bx
        mov     al, es:[bx]
        cmp     al, '-'
        je      @@SetProt
        cmp     al, '+'
        je      @@SetProt
        jmp     @@CheckEnd
@@SetProt:
        mov     [si].pdProtMode, al
        inc     bx

@@CheckEnd:
@@EndPort:
        ;; Find closing ')'
@@FindClose:
        mov     al, es:[bx]
        test    al, al
        jz      @@PortDone
        cmp     al, ')'
        je      @@CloseParen
        inc     bx
        jmp     @@FindClose
@@CloseParen:
        inc     bx                      ; Skip ')'

@@PortDone:
        ;; Initialize DCB defaults
        mov     [si].pdDCB.dcbWriteTimeout, 100   ; 1 second
        mov     [si].pdDCB.dcbReadTimeout, 100    ; 1 second
        mov     [si].pdDCB.dcbFlags1, DCB_F1_DTR_ENA
        mov     [si].pdDCB.dcbFlags2, DCB_F2_RTS_ENA
        mov     [si].pdDCB.dcbFlags3, DCB_F3_RDTO_NRM OR DCB_F3_FIFO_ENA OR DCB_F3_TRIG_8 OR DCB_F3_TXLOAD
        mov     [si].pdDCB.dcbErrorRepl, 0
        mov     [si].pdDCB.dcbBreakRepl, 0
        mov     [si].pdDCB.dcbXonChar, 11h
        mov     [si].pdDCB.dcbXoffChar, 13h

        inc     [DriverData.ddNumPorts]
        pop     si
        ret

@@SkipPort:
        ;; Skip to closing ')' without adding port
@@SkipFind:
        mov     al, es:[bx]
        test    al, al
        jz      @@SkipDone
        cmp     al, ')'
        je      @@SkipClose
        inc     bx
        jmp     @@SkipFind
@@SkipClose:
        inc     bx
@@SkipDone:
        pop     si
        ret


;; ====================================================================
;; ProbeAllPorts — Detect UART hardware on each configured port
;; ====================================================================

ProbeAllPorts:
        mov     cx, [DriverData.ddNumPorts]
        test    cx, cx
        jz      @@ProbeDone
        lea     si, [DriverData.ddPorts]

@@ProbeLoop:
        push    cx

        ;; Skip INTERNET ports — no hardware to probe
        test    [si].pdFlags, PF_INTERNET
        jnz     @@ProbeNext

        ;; Detect UART
        mov     dx, [si].pdIOBase
        call    DetectUART
        mov     [si].pdUARTType, al
        jc      @@ProbeNotFound

        ;; Mark as existing
        or      [si].pdFlags, PF_EXISTS

        ;; Set FIFO depth based on type
        cmp     al, UART_16550A
        jb      @@NoFIFO_P
        mov     [si].pdFIFOSize, 16     ; 16550A: 16-byte FIFO
        cmp     al, UART_16650
        jb      @@FIFOSet
        mov     [si].pdFIFOSize, 32     ; 16650: 32-byte FIFO
        cmp     al, UART_16750
        jb      @@FIFOSet
        mov     [si].pdFIFOSize, 64     ; 16750: 64-byte FIFO
        cmp     al, UART_COMBIC
        jb      @@FIFOSet
        mov     [si].pdFIFOSize, 255    ; ComBic: 1K (capped at byte)
        jmp     @@FIFOSet
@@NoFIFO_P:
        mov     [si].pdFIFOSize, 0
@@FIFOSet:

        ;; Print detection message
        push    si
        call    PrintPortDetect
        pop     si
        jmp     @@ProbeNext

@@ProbeNotFound:
        mov     [si].pdUARTType, UART_NONE
        and     [si].pdFlags, NOT PF_EXISTS

@@ProbeNext:
        add     si, SIZE PORTDATA
        pop     cx
        loop    @@ProbeLoop

@@ProbeDone:
        ret


;; ====================================================================
;; AllocateBuffers — Allocate RX/TX ring buffer memory
;; ====================================================================
;; Uses DevHlp_AllocPhys for each port's buffers.
;; ====================================================================

AllocateBuffers:
        mov     cx, [DriverData.ddNumPorts]
        test    cx, cx
        jz      @@AllocDone
        lea     si, [DriverData.ddPorts]
        xor     bx, bx                  ; Port index

@@AllocLoop:
        push    cx
        push    bx

        ;; Only allocate for detected ports
        test    [si].pdFlags, PF_EXISTS
        jz      @@AllocSkip
        test    [si].pdFlags, PF_INTERNET
        jnz     @@AllocSkip

        ;; Allocate RX buffer
        mov     bx, 0                   ; Above 1MB OK
        mov     eax, RX_BUF_SIZE
        mov     dl, DevHlp_AllocPhys
        call    [DriverData.ddDevHlp]
        jc      @@AllocFail

        ;; Convert physical address to GDT selector for ring buffer use
        ;; DevHlp_PhysToGDTSelector: EAX=phys addr, result in AX:0
        push    eax                     ; Save physical address
        mov     dl, DevHlp_PhysToGDTSel
        mov     ecx, RX_BUF_SIZE
        mov     dh, 0                   ; Read/write access
        call    [DriverData.ddDevHlp]
        ;; EAX now contains selector (or use returned AX as selector)
        pop     ecx                     ; Discard saved phys
        ;; Store as selector:0 far pointer
        push    di
        lea     di, [si].pdRxBuf
        mov     cx, RX_BUF_SIZE
        call    RingBufInit             ; EAX = virtual addr (sel:0)
        pop     di

        ;; Allocate TX buffer
        mov     bx, 0
        mov     eax, TX_BUF_SIZE
        mov     dl, DevHlp_AllocPhys
        call    [DriverData.ddDevHlp]
        jc      @@AllocFail

        push    eax
        mov     dl, DevHlp_PhysToGDTSel
        mov     ecx, TX_BUF_SIZE
        mov     dh, 0
        call    [DriverData.ddDevHlp]
        pop     ecx
        push    di
        lea     di, [si].pdTxBuf
        mov     cx, TX_BUF_SIZE
        call    RingBufInit
        pop     di

@@AllocSkip:
        add     si, SIZE PORTDATA
        pop     bx
        inc     bx
        pop     cx
        loop    @@AllocLoop

@@AllocDone:
        ret

@@AllocFail:
        ;; Memory allocation failed — disable this port
        and     [si].pdFlags, NOT PF_EXISTS
        jmp     @@AllocSkip


;; ====================================================================
;; ChainDeviceHeaders — Patch COMn names and link headers
;; ====================================================================
;; Creates COM1, COM2, ... device headers so apps can DosOpen("COMn").
;; ====================================================================

ChainDeviceHeaders:
        mov     cx, [DriverData.ddNumPorts]
        test    cx, cx
        jz      @@ChainDone
        cmp     cx, NUM_COM_HEADERS
        jbe     @@ChainOk
        mov     cx, NUM_COM_HEADERS      ; Cap at available headers
@@ChainOk:

        ;; Point to first COMn header
        lea     di, COMHeaders
        mov     al, '1'                  ; Start with COM1

@@ChainLoop:
        push    cx

        ;; Patch device name: "COMn    "
        mov     BYTE PTR [di+8], 'C'
        mov     BYTE PTR [di+9], 'O'
        mov     BYTE PTR [di+10], 'M'
        mov     BYTE PTR [di+11], al
        mov     BYTE PTR [di+12], ' '
        mov     BYTE PTR [di+13], ' '
        mov     BYTE PTR [di+14], ' '
        mov     BYTE PTR [di+15], ' '

        ;; Link previous header's SDevNext → this header
        cmp     cx, [DriverData.ddNumPorts]  ; First iteration?
        je      @@FirstHdr
        ;; Patch previous header's SDevNext to point here
        push    di
        sub     di, SIZE SysDev3         ; Back to previous header
        mov     WORD PTR [di], OFFSET COMHeaders  ; Simplified — need proper linking
        ;; TODO: Full linking requires storing current header offset
        pop     di
@@FirstHdr:

        inc     al                       ; Next port number
        add     di, SIZE SysDev3         ; Next header
        pop     cx
        loop    @@ChainLoop

@@ChainDone:
        ret


;; ====================================================================
;; Helper: ParseDecimal — Parse decimal number from ES:BX
;; ====================================================================
;; Out: AX = number, BX advanced past digits
;; ====================================================================

ParseDecimal:
        xor     ax, ax
@@DecLoop:
        mov     cl, es:[bx]
        cmp     cl, '0'
        jb      @@DecDone
        cmp     cl, '9'
        ja      @@DecDone
        sub     cl, '0'
        push    dx
        mov     dx, 10
        mul     dx
        pop     dx
        xor     ch, ch
        add     ax, cx
        inc     bx
        jmp     @@DecLoop
@@DecDone:
        ret


;; ====================================================================
;; Helper: ParseHex — Parse hex number from ES:BX
;; ====================================================================
;; Out: AX = number, BX advanced past hex digits
;; ====================================================================

ParseHex:
        xor     ax, ax
@@HexLoop:
        mov     cl, es:[bx]
        cmp     cl, '0'
        jb      @@HexDone
        cmp     cl, '9'
        jbe     @@HexDigit
        cmp     cl, 'A'
        jb      @@HexDone
        cmp     cl, 'F'
        jbe     @@HexUpper
        cmp     cl, 'a'
        jb      @@HexDone
        cmp     cl, 'f'
        ja      @@HexDone
        sub     cl, 'a' - 10
        jmp     @@HexAdd
@@HexUpper:
        sub     cl, 'A' - 10
        jmp     @@HexAdd
@@HexDigit:
        sub     cl, '0'
@@HexAdd:
        shl     ax, 4
        xor     ch, ch
        or      ax, cx
        inc     bx
        jmp     @@HexLoop
@@HexDone:
        ret


;; ====================================================================
;; Helper: PrintMsg — Print null-terminated message via DevHlp
;; ====================================================================
;; Call as: call PrintMsg / DW OFFSET msgString
;; ====================================================================

PrintMsg:
        pop     si                       ; Return address = msg pointer
        mov     ax, cs:[si]              ; Get message offset
        add     si, 2
        push    si                       ; Fix return address

        ;; During INIT, we can write directly to stdout (handle 1)
        ;; using the request packet's init-time I/O interface.
        ;; For simplicity, use INT 21h AH=09h (DOS print string)
        ;; which works during driver INIT in OS/2.
        push    dx
        mov     dx, ax                  ; DS:DX → message string
        mov     ah, 09h
        int     21h
        pop     dx
        ret


;; ====================================================================
;; PrintPortDetect — Print UART detection result for a port
;; ====================================================================
;; In:  DS:SI → PORTDATA
;; ====================================================================

PrintPortDetect:
        ;; Format: "  COMn at XXXXh Detected UART_TYPE"
        ;; Simplified: use Save_Message with pre-formatted buffer
        ;; Full formatting would convert pdIOBase to hex string
        ;; and index UARTNames — deferred to integration phase
        ret


;; ====================================================================
;; End-of-init markers (everything after these is discarded)
;; ====================================================================

EndOfCode       LABEL   BYTE
