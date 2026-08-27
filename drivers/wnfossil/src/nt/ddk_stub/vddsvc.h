/* ============================================================
 * vddsvc.h — STUB for compile verification ONLY
 * ============================================================
 * This is NOT the real NT DDK header. It declares the NTVDM
 * VDD service functions and register accessors so wf_vdd.c can
 * be compile-checked outside the DDK. For a real NT4/2000 build,
 * use the genuine vddsvc.h + vdmdbg.lib from the NT DDK and
 * REMOVE this stub from the include path.
 *
 * The register accessors below map to NTVDM's VdmContext. Real
 * NTVDM provides these as macros over a thread-local context.
 * ============================================================ */
#ifndef VDDSVC_STUB_H
#define VDDSVC_STUB_H

#include <windows.h>

/* ---- Register accessors (NTVDM provides these as context macros) ---- */
extern UCHAR  getAL(void); extern UCHAR  getAH(void);
extern USHORT getAX(void); extern ULONG  getEAX(void);
extern USHORT getBX(void); extern UCHAR  getBH(void); extern UCHAR getBL(void);
extern USHORT getCX(void); extern USHORT getDX(void); extern UCHAR getDL(void);
extern USHORT getDI(void); extern USHORT getES(void);
extern USHORT getSS(void); extern USHORT getSP(void);

extern void setAX(USHORT); extern void setBX(USHORT);
extern void setBH(UCHAR);  extern void setBL(UCHAR);
extern void setCX(USHORT); extern void setDX(USHORT);
extern void setCF(USHORT);

/* Pointer-form aliases used in wf_vdd.c (pGetAX == getAX etc.) */
#define pGetAL   getAL
#define pGetAH   getAH
#define pGetAX   getAX
#define pGetEAX  getEAX
#define pGetBX   getBX
#define pGetCX   getCX
#define pGetDX   getDX
#define pGetDL   getDL
#define pGetDI   getDI
#define pGetES   getES
#define pGetSS   getSS
#define pGetSP   getSP
#define pSetAX   setAX
#define pSetBH   setBH
#define pSetBL   setBL
#define pSetCX   setCX
#define pSetDX   setDX
#define pSetCF   setCF

/* ---- VDD service functions ---- */
typedef VOID (*PVDD_IO_HANDLER)(VOID);
typedef struct _VDD_IO_PORTRANGE { USHORT First, Last; } VDD_IO_PORTRANGE, *PVDD_IO_PORTRANGE;
typedef struct _VDD_IO_HANDLERS { PVDD_IO_HANDLER inb, inw, insb, insw, outb, outw, outsb, outsw; } VDD_IO_HANDLERS, *PVDD_IO_HANDLERS;

BOOL VDDInstallIOHook(HANDLE, WORD, PVDD_IO_PORTRANGE, PVDD_IO_HANDLERS);
VOID VDDDeInstallIOHook(HANDLE, WORD, PVDD_IO_PORTRANGE);
PVOID VdmMapFlat(USHORT, ULONG, int);

#endif /* VDDSVC_STUB_H */
