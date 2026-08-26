/* ====================================================================
 * VSIO2K.SYS — Virtual Serial I/O Driver for OS/2 DOS VDMs (V2)
 * ====================================================================
 * Clean-room reimplementation. 32-bit Virtual Device Driver (VDD).
 *
 * VSIO2K virtualizes COM port access for DOS sessions running under OS/2.
 * It intercepts I/O port reads/writes from DOS programs and routes them
 * through SIO2K.SYS via the PDD-VDD communication interface.
 *
 * Key behaviors (from SIO2KREF.TXT):
 *   - Opens COM ports with sharing allowed (unlike VCOM which uses
 *     exclusive access)
 *   - Virtualizes RTS when RTS/CTS handshaking is active
 *   - Provides flow control signal virtualization
 *   - Reflects IRQs to the DOS session on the configured virtual IRQ
 *   - Maps I/O ports to virtual addresses for DOS programs
 * ====================================================================
 */

/* VDD API types and stubs for cross-compilation.
 * The actual mvdm.h is part of the OS/2 DDK, not the standard toolkit.
 * These definitions are derived from the toolkit inc/mvdm.inc and
 * inc/v8086.inc files which we have. */
#include "sio2k_idc.h"
#define INCL_DOS
#include "sio2k_idc.h"
#define INCL_DOSERRORS
#include <os2.h>
#include "vsio.h"

/* VDD types from mvdm.inc */
typedef ULONG   HVDM;
typedef ULONG   HVDD;
typedef ULONG   HIRQ;
typedef ULONG   HHOOK;
typedef ULONG   FLAGS;
typedef USHORT  PORT;

/* Client Register Frame (from v8086.inc CRF structure) */
typedef struct _CRF {
    ULONG   crf_edi, crf_esi, crf_ebp, crf_padesp;
    ULONG   crf_ebx, crf_edx, crf_ecx, crf_eax;
    ULONG   crf_pad2[2];
    ULONG   crf_eip;
    USHORT  crf_cs, crf_padcs;
    ULONG   crf_eflag, crf_esp;
    USHORT  crf_ss, crf_padss, crf_es, crf_pades;
    USHORT  crf_ds, crf_padds, crf_fs, crf_padfs;
    USHORT  crf_gs, crf_padgs;
} CRF, *PCRF;

/* IOH structure (from v8086.inc) */
typedef BYTE  (APIENTRY *PFNBIH)(ULONG, PCRF);
typedef VOID  (APIENTRY *PFNBOH)(BYTE, ULONG, PCRF);
typedef struct _IOH {
    PFNBIH  ioh_pbihByteInput;
    PFNBOH  ioh_pbohByteOutput;
    PVOID   ioh_pwihWordInput;
    PVOID   ioh_pwohWordOutput;
    PVOID   ioh_pothOther;
} IOH, *PIOH;

/* VDD-PDD communication */
typedef ULONG (APIENTRY *FPFNPDD)(ULONG, ULONG, ULONG);
typedef BOOL  (APIENTRY *FPFNVDD)(ULONG, ULONG, ULONG);
typedef BOOL  (APIENTRY *PFNSYSREQ)(ULONG, HVDM);
typedef BOOL  (APIENTRY *PFNDEVREQ)(ULONG, ULONG, ULONG);

/* SIOCMD_*, SIOEVT_*, and SIOPORT_INFO now come from the shared
 * inc/vsio.h contract (also used by sio2k.c's SioPddEntry) instead of
 * being duplicated locally.
 * Command codes VSIO2K still defines itself, since they're VDD-side
 * only and have no PDD-facing counterpart: */
#define VDMSYSREQ_CREATE    0
#define VDMSYSREQ_TERMINATE 1
#define PDDCMD_REGISTER     0

/* VDH function stubs — would be resolved by OS/2 VDD loader at runtime */
#define HOOKENTRY APIENTRY
#define VDDENTRY  APIENTRY

/* Stub declarations — real implementations in OS/2 kernel */
extern BOOL    APIENTRY VDHRegisterVDD(PSZ, PFNSYSREQ, PFNDEVREQ);
extern BOOL    APIENTRY VDHOpenPDD(PSZ, FPFNVDD);
extern PVOID   APIENTRY VDHAllocMem(ULONG, ULONG);
extern VOID    APIENTRY VDHFreeMem(PVOID);
extern BOOL    APIENTRY VDHInstallIOHook(HVDM, PORT, ULONG, PIOH, FLAGS);
extern BOOL    APIENTRY VDHRemoveIOHook(HVDM, PORT, ULONG, PIOH);
extern HIRQ    APIENTRY VDHOpenVIRQ(ULONG, PVOID, PVOID, ULONG, ULONG);
extern VOID    APIENTRY VDHCloseVIRQ(HIRQ);
extern HVDM    APIENTRY VDHQueryVDM(VOID);

/* Assert a previously-opened VIRQ so the DOS ISR runs. The exact MVDM
 * DDK entry point for "fire this VIRQ now" varies by toolkit version
 * and isn't in the headers available for this clean-room build;
 * declared here under the same VDHxxx convention as the stubs above.
 * Verify the real symbol name against the target DDK before linking
 * against an actual OS/2 kernel. */
extern VOID    APIENTRY VDHSetVirtIRQ(HIRQ);

#define VDHAM_FIXED 0

/* Inline memory functions — VDD can't link against C runtime */
static void *vdd_memset(void *s, int c, unsigned long n) {
    unsigned char *p = (unsigned char *)s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}
static void *vdd_memmove(void *dest, const void *src, unsigned long n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s2 = (const unsigned char *)src;
    if (d < s2) { while (n--) *d++ = *s2++; }
    else { d += n; s2 += n; while (n--) *--d = *--s2; }
    return dest;
}
static void *vdd_memcpy(void *dest, const void *src, unsigned long n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s2 = (const unsigned char *)src;
    while (n--) *d++ = *s2++;
    return dest;
}
#define memset  vdd_memset
#define memmove vdd_memmove
#define memcpy  vdd_memcpy

/* -------------------------------------------------------------------- */
/* UART Virtual State — one per VDM per COM port                        */
/* -------------------------------------------------------------------- */

#define MAX_VPORTS  16

typedef struct _VUART {
    USHORT  sio2kIndex;     /* Global SIO2K.SYS port index this maps to */
    USHORT  ioBase;         /* Real I/O base address                    */
    USHORT  ioBaseDOS;      /* Virtual I/O base for this VDM            */
    BYTE    irq;            /* Real IRQ                                 */
    BYTE    irqDOS;         /* Virtual IRQ for this VDM                 */
    BOOL    active;         /* Port is active in this VDM               */
    BOOL    open;           /* Port is open                             */
    HIRQ    hVIRQ;          /* Virtual IRQ handle from VDHOpenVIRQ      */

    /* Virtual UART register state (what the DOS program sees) */
    BYTE    vIER;           /* Virtual Interrupt Enable Register        */
    BYTE    vIIR;           /* Virtual Interrupt ID Register            */
    BYTE    vLCR;           /* Virtual Line Control Register            */
    BYTE    vMCR;           /* Virtual Modem Control Register           */
    BYTE    vLSR;           /* Virtual Line Status Register             */
    BYTE    vMSR;           /* Virtual Modem Status Register            */
    BYTE    vSCR;           /* Virtual Scratch Register                 */
    BYTE    vDLL;           /* Virtual Divisor Latch Low                */
    BYTE    vDLH;           /* Virtual Divisor Latch High               */
    BYTE    vFCR;           /* Virtual FIFO Control Register            */

    /* Communication with SIO2K.SYS PDD via IDC */
    HFILE   hPDD;           /* Handle from VDHOpenPDD                   */
} VUART, *PVUART;


/* Per-VDM instance data */
typedef struct _VDMDATA {
    HVDM    hvdm;           /* VDM handle                              */
    VUART   ports[MAX_VPORTS];
    USHORT  numPorts;
} VDMDATA, *PVDMDATA;


/* -------------------------------------------------------------------- */
/* Globals                                                              */
/* -------------------------------------------------------------------- */

#define MAX_VDMS 64
static VDMDATA g_vdms[MAX_VDMS];
static USHORT  g_numVDMs = 0;

static FPFNPDD  fpfnSIO = NULL;        /* SIO2K.SYS PDD entry point   */
static PFN_PHYS_IDC g_pfnSio2kIDC = NULL; /* SIO2K IDC entry for direct calls */
static HVDD     hvdd    = 0;           /* Our VDD handle               */


/* -------------------------------------------------------------------- */
/* PDD-VDD Communication Commands — see inc/vsio.h                     */
/* -------------------------------------------------------------------- */


/* -------------------------------------------------------------------- */
/* Forward Declarations                                                 */
/* -------------------------------------------------------------------- */

static BOOL     VDDENTRY VsioSysReq(ULONG ulFunc, HVDM hvdm);
static BOOL     VDDENTRY VsioDevReq(ULONG ulFunc, ULONG ul1, ULONG ul2);
static BYTE     HOOKENTRY VsioByteIn(ULONG port, PCRF pcrf);
static VOID     HOOKENTRY VsioByteOut(BYTE data, ULONG port, PCRF pcrf);
static PVDMDATA GetVDMData(HVDM hvdm);
static PVDMDATA AllocVDMData(HVDM hvdm);
static void     FreeVDMData(HVDM hvdm);
static PVUART   PortFromAddr(PVDMDATA pvd, ULONG port);


/* ====================================================================
 * VDD Initialization Entry Point
 * ====================================================================
 * Called by OS/2 VDM Manager when VSIO.SYS is loaded.
 * ==================================================================== */

BOOL VDDENTRY VDDInit(PSZ pszCmdLine)
{
    /* Register ourselves with the VDM Manager */
    if (!VDHRegisterVDD("VSIO2K$", VsioSysReq, VsioDevReq)) {
        return FALSE;
    }

    /* Open communication channel to SIO.SYS PDD */
    if (!VDHOpenPDD("SIO2K$", (FPFNVDD)VsioDevReq)) {
        return FALSE;
    }

    return TRUE;
}


/* ====================================================================
 * System Request Handler
 * ====================================================================
 * Called by VDM Manager for system-level events:
 *   - VDM creation/termination
 *   - Session switch
 *   - PDB change
 * ==================================================================== */

static BOOL VDDENTRY VsioSysReq(ULONG ulFunc, HVDM hvdm)
{
    PVDMDATA pvd;

    switch (ulFunc) {

    case VDMSYSREQ_CREATE:
        /* New VDM created — set up per-VDM port state */
        pvd = AllocVDMData(hvdm);
        if (!pvd) return FALSE;

        pvd->numPorts = 0;

        /* Query SIO2K.SYS for configured ports and install I/O hooks
         * on each one's virtual address range. */
        {
            USHORT  i;
            USHORT  numFound = 0;
            IOH     ioh;
            SIOPORT_INFO info;

            ioh.ioh_pbihByteInput  = VsioByteIn;
            ioh.ioh_pbohByteOutput = VsioByteOut;
            ioh.ioh_pwihWordInput  = NULL;  /* Simulate */
            ioh.ioh_pwohWordOutput = NULL;  /* Simulate */
            ioh.ioh_pothOther      = NULL;  /* Simulate */

            for (i = 0; fpfnSIO && i < MAX_VPORTS && numFound < MAX_VPORTS; i++) {
                if (!fpfnSIO(SIOCMD_GETPORTINFO, i, (ULONG)&info)) {
                    /* SIO2K reports "no such port index" for anything
                     * past its highest configured port — since indices
                     * are assigned densely from 0, stop scanning. */
                    if (i > 0) break;
                    continue;
                }
                if (!(info.flags & SPIF_EXISTS)) continue;

                {
                    PVUART pv = &pvd->ports[numFound];

                    pv->sio2kIndex = i;
                    pv->ioBase     = info.ioBase;
                    pv->ioBaseDOS  = info.ioBaseDOS;
                    pv->irq        = info.irq;
                    pv->irqDOS     = info.irqDOS;
                    pv->active     = TRUE;
                    pv->open       = FALSE;

                    /* Initialize virtual UART state */
                    pv->vIER = 0;
                    pv->vIIR = 0x01;    /* No interrupt pending */
                    pv->vLCR = 0x03;    /* 8N1 */
                    pv->vMCR = 0;
                    pv->vLSR = 0x60;    /* THR empty, TX empty */
                    pv->vMSR = 0;
                    pv->vSCR = 0;
                    pv->vDLL = 0x0C;    /* 9600 baud */
                    pv->vDLH = 0;
                    pv->vFCR = 0;

                    VDHInstallIOHook(hvdm, pv->ioBaseDOS, 8, &ioh, 0);
                    pv->hVIRQ = VDHOpenVIRQ(pv->irqDOS, NULL, NULL, 0, 0);

                    numFound++;
                }
            }

            if (numFound == 0) {
                /* SIO2K.SYS's PDD isn't reachable yet (fpfnSIO not
                 * registered) or reports no configured ports — fall
                 * back to standard COM1/COM2 addresses so a DOS
                 * session still sees something usable rather than no
                 * ports at all. */
                for (i = 0; i < 2 && i < MAX_VPORTS; i++) {
                    PVUART pv = &pvd->ports[i];
                    USHORT base = (i == 0) ? 0x3F8 : 0x2F8;
                    BYTE   irq  = (i == 0) ? 4 : 3;

                    pv->sio2kIndex = i;
                    pv->ioBase    = base;
                    pv->ioBaseDOS = base;
                    pv->irq       = irq;
                    pv->irqDOS    = irq;
                    pv->active    = TRUE;
                    pv->open      = FALSE;

                    pv->vIER = 0;
                    pv->vIIR = 0x01;
                    pv->vLCR = 0x03;
                    pv->vMCR = 0;
                    pv->vLSR = 0x60;
                    pv->vMSR = 0;
                    pv->vSCR = 0;
                    pv->vDLL = 0x0C;
                    pv->vDLH = 0;
                    pv->vFCR = 0;

                    VDHInstallIOHook(hvdm, base, 8, &ioh, 0);
                    pv->hVIRQ = VDHOpenVIRQ(irq, NULL, NULL, 0, 0);

                    numFound++;
                }
            }

            pvd->numPorts = numFound;
        }

        break;

    case VDMSYSREQ_TERMINATE:
        /* VDM being destroyed — clean up */
        pvd = GetVDMData(hvdm);
        if (pvd) {
            USHORT i;
            for (i = 0; i < pvd->numPorts; i++) {
                PVUART pv = &pvd->ports[i];
                if (pv->active) {
                    IOH ioh;
                    ioh.ioh_pbihByteInput  = VsioByteIn;
                    ioh.ioh_pbohByteOutput = VsioByteOut;
                    ioh.ioh_pwihWordInput  = NULL;
                    ioh.ioh_pwohWordOutput = NULL;
                    ioh.ioh_pothOther      = NULL;
                    VDHRemoveIOHook(hvdm, pv->ioBaseDOS, 8, &ioh);

                    if (pv->hVIRQ) {
                        VDHCloseVIRQ(pv->hVIRQ);
                    }

                    if (pv->open && fpfnSIO) {
                        fpfnSIO(SIOCMD_CLOSEPORT, (ULONG)i, 0);
                    }
                }
            }

            FreeVDMData(hvdm);
        }
        break;

    default:
        break;
    }

    return TRUE;
}


/* ====================================================================
 * FireVirtualIRQ — locate the VDM/port owning a SIO2K port index and
 * raise its VIRQ, reflecting the event into the virtual register
 * state first so the DOS ISR sees something sensible when it reads
 * IIR/LSR/MSR in response.
 * ==================================================================== */

static void FireVirtualIRQ(USHORT sio2kIndex, ULONG evtFlags)
{
    USHORT v, p;

    for (v = 0; v < g_numVDMs; v++) {
        PVDMDATA pvd = &g_vdms[v];
        for (p = 0; p < pvd->numPorts; p++) {
            PVUART pv = &pvd->ports[p];
            if (!pv->active || pv->sio2kIndex != sio2kIndex) continue;

            if (evtFlags & SIOEVT_RXDATA) {
                pv->vLSR |= 0x01;                 /* Data Ready */
                if (pv->vIER & 0x01) pv->vIIR = 0x04;  /* RX interrupt */
            }
            if (evtFlags & SIOEVT_TXEMPTY) {
                pv->vLSR |= 0x20;                 /* THR Empty */
                if ((pv->vIER & 0x02) && pv->vIIR == 0x01) pv->vIIR = 0x02;
            }
            if (evtFlags & SIOEVT_MSR) {
                if ((pv->vIER & 0x08) && pv->vIIR == 0x01) pv->vIIR = 0x00;
            }
            if (evtFlags & SIOEVT_LSR) {
                pv->vLSR |= 0x02;                 /* Overrun */
                if (pv->vIER & 0x04) pv->vIIR = 0x06;  /* Line status */
            }
            if (evtFlags & SIOEVT_BREAK) {
                pv->vLSR |= 0x10;                 /* Break Interrupt */
                if (pv->vIER & 0x04) pv->vIIR = 0x06;
            }

            if (pv->hVIRQ) VDHSetVirtIRQ(pv->hVIRQ);
            return;   /* A SIO2K port index maps to exactly one VDM/port */
        }
    }
}


/* ====================================================================
 * Device Request Handler (PDD-VDD Communication)
 * ====================================================================
 * Called by SIO2K.SYS to notify us of events (data received, modem
 * status change, etc.) and by VDM Manager for PDD registration.
 * ==================================================================== */

static BOOL VDDENTRY VsioDevReq(ULONG ulFunc, ULONG ul1, ULONG ul2)
{
    switch (ulFunc) {

    case PDDCMD_REGISTER:
        /* VDM Manager is registering the PDD entry point.
         * Per OS/2 VDD-PDD calling convention (mvdm.inc FNPDD):
         *   ul1.off = VDD's CS,  ul1.sel = 0
         *   ul2.off = low 16 bits of EIP,  ul2.sel = high 16 bits of EIP
         * For 16:16 PDD entry points:
         *   ul1 = segment:0 (segment in low word)
         *   ul2 = offset (in low word)
         * Construct 16:16 far pointer: segment from ul1, offset from ul2 */
        {
            USHORT seg = (USHORT)(ul1 & 0xFFFF);
            USHORT ofs = (USHORT)(ul2 & 0xFFFF);
            fpfnSIO = (FPFNPDD)(((ULONG)seg << 16) | ofs);
        }
        return TRUE;

    case SIOCMD_NOTIFY:
        /* SIO.SYS notifying us of an event (RX data, MSR change) */
        /* ul1 = port index, ul2 = event flags */
        /* Fire virtual IRQ to wake the DOS program */
        FireVirtualIRQ((USHORT)ul1, ul2);
        return TRUE;

    default:
        return FALSE;
    }
}


/* ====================================================================
 * I/O Port Byte Input Handler
 * ====================================================================
 * Called when the DOS program reads from a UART register.
 * We return virtual register state, querying SIO2K.SYS for live data
 * (RX buffer, modem status) as needed.
 * ==================================================================== */

static BYTE HOOKENTRY VsioByteIn(ULONG port, PCRF pcrf)
{
    HVDM    hvdm = VDHQueryVDM();       /* Which VDM is accessing? */
    PVDMDATA pvd = GetVDMData(hvdm);
    PVUART   pv;
    ULONG    reg;

    if (!pvd) return 0xFF;
    pv = PortFromAddr(pvd, port);
    if (!pv)  return 0xFF;

    reg = port - pv->ioBaseDOS;         /* Register offset 0-7 */

    switch (reg) {

    case 0: /* RBR/DLL */
        if (pv->vLCR & 0x80) {
            /* DLAB=1: return divisor low */
            return pv->vDLL;
        }
        /* DLAB=0: read received byte from SIO2K */
        if (fpfnSIO) {
            ULONG result = 0;
            USHORT idx = (USHORT)(pv - pvd->ports);
            fpfnSIO(SIOCMD_READBYTE, (ULONG)idx, (ULONG)&result);
            return (BYTE)(result & 0xFF);
        }
        return 0;

    case 1: /* IER/DLH */
        if (pv->vLCR & 0x80) {
            return pv->vDLH;
        }
        return pv->vIER;

    case 2: /* IIR (read-only) */
        return pv->vIIR;

    case 3: /* LCR */
        return pv->vLCR;

    case 4: /* MCR */
        return pv->vMCR;

    case 5: /* LSR */
        /* Query SIO2K for live line status */
        if (fpfnSIO) {
            ULONG lsr = 0;
            USHORT idx = (USHORT)(pv - pvd->ports);
            fpfnSIO(SIOCMD_GETLSR, (ULONG)idx, (ULONG)&lsr);
            pv->vLSR = (BYTE)(lsr & 0xFF);
        }
        {
            BYTE result = pv->vLSR;
            /* Reading LSR clears error bits (standard UART behavior) */
            pv->vLSR &= 0x60;  /* Keep THRE and TEMT */
            return result;
        }

    case 6: /* MSR */
        /* Query SIO2K for live modem status */
        if (fpfnSIO) {
            ULONG msr = 0;
            USHORT idx = (USHORT)(pv - pvd->ports);
            fpfnSIO(SIOCMD_GETMSR, (ULONG)idx, (ULONG)&msr);
            pv->vMSR = (BYTE)(msr & 0xFF);
        }
        {
            BYTE result = pv->vMSR;
            /* Reading MSR clears delta bits */
            pv->vMSR &= 0xF0;
            return result;
        }

    case 7: /* SCR */
        return pv->vSCR;
    }

    return 0xFF;
}


/* ====================================================================
 * I/O Port Byte Output Handler
 * ====================================================================
 * Called when the DOS program writes to a UART register.
 * We update virtual state and forward to SIO.SYS as appropriate.
 *
 * Key VSIO behavior: when RTS/CTS handshaking is active, VSIO
 * virtualizes the RTS settings. The DOS program's RTS writes go to
 * the virtual MCR, while SIO2K.SYS controls the real RTS signal.
 * ==================================================================== */

static VOID HOOKENTRY VsioByteOut(BYTE data, ULONG port, PCRF pcrf)
{
    HVDM    hvdm = VDHQueryVDM();
    PVDMDATA pvd = GetVDMData(hvdm);
    PVUART   pv;
    ULONG    reg;

    if (!pvd) return;
    pv = PortFromAddr(pvd, port);
    if (!pv)  return;

    reg = port - pv->ioBaseDOS;

    switch (reg) {

    case 0: /* THR/DLL */
        if (pv->vLCR & 0x80) {
            /* DLAB=1: buffer divisor low — don't send yet */
            pv->vDLL = data;
        } else {
            /* DLAB=0: transmit byte via SIO2K */
            if (fpfnSIO) {
                USHORT idx = (USHORT)(pv - pvd->ports);
                fpfnSIO(SIOCMD_WRITEBYTE, (ULONG)idx, (ULONG)data);
            }
        }
        break;

    case 1: /* IER/DLH */
        if (pv->vLCR & 0x80) {
            /* DLAB=1: buffer divisor high — don't send yet */
            pv->vDLH = data;
        } else {
            pv->vIER = data & 0x0F;
            if (fpfnSIO) {
                USHORT idx = (USHORT)(pv - pvd->ports);
                fpfnSIO(SIOCMD_SETIER, (ULONG)idx, (ULONG)pv->vIER);
            }
        }
        break;

    case 2: /* FCR (write-only) */
        pv->vFCR = data;
        if (fpfnSIO) {
            USHORT idx = (USHORT)(pv - pvd->ports);
            fpfnSIO(SIOCMD_SETFCR, (ULONG)idx, (ULONG)data);
        }
        break;

    case 3: /* LCR */
        {
            BYTE oldLCR = pv->vLCR;
            pv->vLCR = data;
            if (fpfnSIO) {
                USHORT idx = (USHORT)(pv - pvd->ports);
                fpfnSIO(SIOCMD_SETLCR, (ULONG)idx, (ULONG)data);

                /* If DLAB was 1 and is now 0, apply buffered divisor */
                if ((oldLCR & 0x80) && !(data & 0x80)) {
                    USHORT div = (pv->vDLH << 8) | pv->vDLL;
                    if (div != 0) {
                        fpfnSIO(SIOCMD_SETBAUD, (ULONG)idx, (ULONG)div);
                    }
                }
            }
        }
        break;

    case 4: /* MCR */
        /* VSIO virtualizes RTS when RTS handshaking is active.
         * The DOS program writes to the virtual MCR, but SIO
         * completely controls the real RTS signal. Only forward
         * DTR changes (unless protection mode '+' inhibits it). */
        pv->vMCR = data;
        if (fpfnSIO) {
            USHORT idx = (USHORT)(pv - pvd->ports);
            fpfnSIO(SIOCMD_SETMCR, (ULONG)idx, (ULONG)data);
        }
        break;

    case 5: /* LSR — read-only, write ignored */
        break;

    case 6: /* MSR — read-only, write ignored */
        break;

    case 7: /* SCR */
        pv->vSCR = data;
        break;
    }
}


/* ====================================================================
 * Helper: Get VDM instance data
 * ==================================================================== */

/* Simplified — production version would use VDM instance data API
 * or a hash table keyed by HVDM. */



static PVDMDATA GetVDMData(HVDM hvdm)
{
    USHORT i;
    for (i = 0; i < g_numVDMs; i++) {
        if (g_vdms[i].hvdm == hvdm)
            return &g_vdms[i];
    }
    return NULL;
}

/* Allocate a new VDM slot, returns NULL if full */
static PVDMDATA AllocVDMData(HVDM hvdm)
{
    if (g_numVDMs >= MAX_VDMS)
        return NULL;
    memset(&g_vdms[g_numVDMs], 0, sizeof(VDMDATA));
    g_vdms[g_numVDMs].hvdm = hvdm;
    return &g_vdms[g_numVDMs++];
}

/* Free a VDM slot, compact the array */
static void FreeVDMData(HVDM hvdm)
{
    USHORT i;
    for (i = 0; i < g_numVDMs; i++) {
        if (g_vdms[i].hvdm == hvdm) {
            if (i < g_numVDMs - 1) {
                memmove(&g_vdms[i], &g_vdms[i+1],
                        (g_numVDMs - i - 1) * sizeof(VDMDATA));
            }
            g_numVDMs--;
            return;
        }
    }
}


/* ====================================================================
 * Helper: Find VUART from I/O port address
 * ==================================================================== */

static PVUART PortFromAddr(PVDMDATA pvd, ULONG port)
{
    USHORT i;
    for (i = 0; i < pvd->numPorts; i++) {
        USHORT base = pvd->ports[i].ioBaseDOS;
        if (port >= base && port < base + 8)
            return &pvd->ports[i];
    }
    return NULL;
}
