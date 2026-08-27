;/* ====================================================================
; * cylog.mc — Event Log Message Definitions
; * ====================================================================
; * This file is processed by the Message Compiler (mc.exe) from the
; * WDK to generate:
; *   cylog.h  — #define constants for message IDs (included by driver)
; *   cylog.rc — resource script linked into cyport.sys (message table)
; *   MSG00001.bin — binary message data (embedded in .rc)
; *
; * Build command:
; *   mc -v -w cylog.mc
; *   rc cylog.rc          (produces cylog.res)
; *   link ... cylog.res   (embed in cyport.sys)
; *
; * The message table in the driver binary allows Event Viewer to
; * display human-readable messages instead of raw error codes.
; * Without it, Event Viewer shows "The description for Event ID X
; * from source cyport cannot be found."
; *
; * Each message has:
; *   - Severity: Success, Informational, Warning, Error
; *   - Facility: application-defined (we use 0x100 = IoErrorCode)
; *   - SymbolicName: #define name used in source code
; *   - Language: English (expandable to other languages)
; *   - Text: printf-style format with %1 %2 etc. for insertion strings
; *
; * Insertion strings are passed via IoWriteErrorLogEntry's
; * DumpData and IO_ERROR_LOG_PACKET.StringOffset fields.
; *
; * Sources:
; *   - WDK mc.exe documentation
; *   - Original cyyport.sys uses IoAllocateErrorLogEntry +
; *     IoWriteErrorLogEntry (confirmed by import analysis)
; *
; * License: GPLv3
; * ==================================================================== */

;/* ---- Message Compiler Header ---- */

MessageIdTypedef=NTSTATUS

SeverityNames=(
    Success=0x0:STATUS_SEVERITY_SUCCESS
    Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
    Warning=0x2:STATUS_SEVERITY_WARNING
    Error=0x3:STATUS_SEVERITY_ERROR
)

FacilityNames=(
    System=0x0
    Cyclades=0x100:FACILITY_CYCLADES
)

LanguageNames=(
    English=0x409:MSG00409
)


;/* ====================================================================
; * INFORMATIONAL MESSAGES (Severity = Informational)
; * ==================================================================== */

MessageId=0x0001
Severity=Informational
Facility=Cyclades
SymbolicName=CYLOG_DRIVER_LOADED
Language=English
Cyclades Cyclom-Y driver loaded successfully. Detected %1 CD1400 chip(s), %2 serial port(s).
.

MessageId=0x0002
Severity=Informational
Facility=Cyclades
SymbolicName=CYLOG_PORT_OPENED
Language=English
Cyclades port %1 (COM%2) opened at %3 baud, %4.
.

MessageId=0x0003
Severity=Informational
Facility=Cyclades
SymbolicName=CYLOG_PORT_CLOSED
Language=English
Cyclades port %1 (COM%2) closed. TX=%3 bytes, RX=%4 bytes.
.

MessageId=0x0004
Severity=Informational
Facility=Cyclades
SymbolicName=CYLOG_POWER_SUSPEND
Language=English
Cyclades port %1 entering sleep state (D0 to D3). Channel state saved.
.

MessageId=0x0005
Severity=Informational
Facility=Cyclades
SymbolicName=CYLOG_POWER_RESUME
Language=English
Cyclades port %1 resuming from sleep (D3 to D0). Channel state restored.
.

MessageId=0x0006
Severity=Informational
Facility=Cyclades
SymbolicName=CYLOG_DRIVER_UNLOADED
Language=English
Cyclades Cyclom-Y driver unloaded.
.


;/* ====================================================================
; * WARNING MESSAGES (Severity = Warning)
; * ==================================================================== */

MessageId=0x0100
Severity=Warning
Facility=Cyclades
SymbolicName=CYLOG_BUFFER_OVERRUN
Language=English
Cyclades port %1: receive buffer overrun. %2 byte(s) lost. Application is not reading fast enough.
.

MessageId=0x0101
Severity=Warning
Facility=Cyclades
SymbolicName=CYLOG_FIFO_OVERRUN
Language=English
Cyclades port %1: hardware FIFO overrun. Data lost due to high interrupt latency.
.

MessageId=0x0102
Severity=Warning
Facility=Cyclades
SymbolicName=CYLOG_PARITY_ERRORS
Language=English
Cyclades port %1: %2 parity error(s) detected. Check cable and baud rate settings.
.

MessageId=0x0103
Severity=Warning
Facility=Cyclades
SymbolicName=CYLOG_FRAMING_ERRORS
Language=English
Cyclades port %1: %2 framing error(s) detected. Baud rate mismatch with remote device.
.

MessageId=0x0104
Severity=Warning
Facility=Cyclades
SymbolicName=CYLOG_BREAK_RECEIVED
Language=English
Cyclades port %1: break condition received from remote device.
.

MessageId=0x0105
Severity=Warning
Facility=Cyclades
SymbolicName=CYLOG_ISR_STUCK
Language=English
Cyclades chip %1: ISR reached maximum iteration limit (%2). Hardware may be stuck.
.


;/* ====================================================================
; * ERROR MESSAGES (Severity = Error)
; * ==================================================================== */

MessageId=0x0200
Severity=Error
Facility=Cyclades
SymbolicName=CYLOG_NO_CHIPS_FOUND
Language=English
Cyclades Cyclom-Y: no CD1400 chips detected at memory address %1. Card may not be installed or memory address may conflict with another device.
.

MessageId=0x0201
Severity=Error
Facility=Cyclades
SymbolicName=CYLOG_MEMORY_MAP_FAILED
Language=English
Cyclades Cyclom-Y: failed to map hardware memory at physical address %1 (length %2 bytes). MmMapIoSpace returned NULL.
.

MessageId=0x0202
Severity=Error
Facility=Cyclades
SymbolicName=CYLOG_INTERRUPT_CONNECT_FAILED
Language=English
Cyclades Cyclom-Y: failed to connect interrupt. IoConnectInterrupt returned %1. No serial I/O possible.
.

MessageId=0x0203
Severity=Error
Facility=Cyclades
SymbolicName=CYLOG_PDO_CREATE_FAILED
Language=English
Cyclades Cyclom-Y: failed to create device object for port %1. IoCreateDevice returned %1. Port will not be available.
.

MessageId=0x0204
Severity=Error
Facility=Cyclades
SymbolicName=CYLOG_EOSRR_MISSED
Language=English
Cyclades Cyclom-Y chip %1: EOSRR write may have been missed. Chip interrupts may be jammed. This is a driver bug — please report.
.

MessageId=0x0205
Severity=Error
Facility=Cyclades
SymbolicName=CYLOG_POWER_RESTORE_FAILED
Language=English
Cyclades port %1: failed to restore channel state after resume. Port may need to be closed and reopened.
.
