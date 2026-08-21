/* PCB153FX.H — Borland C++ 3.1 compatibility fixes for pcb153 headers */
/* Force-included before everything else                                */

/* BCC 3.1 errors on enum values >= 32768 (16-bit int overflow).       */
/* pcb153's PCBOARD.H defines AUTO=32768 and NOTBLANK=32768 inside     */
/* the displaytype enum. Fix: pre-define them as macros so the enum     */
/* values never reach the compiler.                                     */
#define AUTO     ((int)0x8000)
#define NOTBLANK ((int)0x8000)
