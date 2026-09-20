/* ============================================================================
 * irq.c — 8259 PIC control + shared-IRQ dispatcher for pcbdcom
 *
 * Ported from Linux arch/x86/kernel/i8259.c (kernel v6.6), GPLv2.
 * Original authors: Linus Torvalds, Ingo Molnar, others.
 * DOS 16-bit adaptations: pcbirc crew (hexadecimal), GPLv3.
 *
 * Design:
 *   - Per-IRQ dispatch table: pcbdcom_irq_ports[IRQ_MAX][MAX_PORTS_PER_IRQ]
 *     holds pointers to pcbdcom_port_t whose backend->isr() must be
 *     called when that IRQ fires.
 *   - Multiple ports per IRQ (COM1+COM3 share IRQ 4, COM2+COM4 share
 *     IRQ 3, Boca cards share one IRQ across all sub-ports).
 *   - Per-IRQ handler saves original vector via DOS INT 21h func 35h,
 *     installs our handler, dispatches to every registered port whose
 *     backend->isr() then decides if it has work.
 *   - EOI sent to PIC after all backends serviced.
 * ==========================================================================*/

#include <dos.h>
#include <conio.h>
#include "pcbdcom.h"
#include "backend.h"

#if defined(_MSC_VER)
# define PIC_OUT(port, val) _outp((port), (val))
# define PIC_IN(port)       (unsigned char)_inp((port))
#else
# define PIC_OUT(port, val) outp((port), (val))
# define PIC_IN(port)       (unsigned char)inp((port))
#endif

/* 8259 PIC ports */
#define PIC1_CMD   0x20   /* master PIC command port */
#define PIC1_DATA  0x21   /* master PIC data / IMR   */
#define PIC2_CMD   0xA0   /* slave PIC command port  */
#define PIC2_DATA  0xA1   /* slave PIC data / IMR    */
#define PIC_EOI    0x20   /* end-of-interrupt cmd    */

/* IRQ -> hardware interrupt vector (IRQ 0..7 = INT 08h..0Fh,
 *                                    IRQ 8..15 = INT 70h..77h) */
#define IRQ_TO_VECTOR(irq) ((irq) < 8 ? (0x08 + (irq)) : (0x70 + (irq) - 8))

#define IRQ_MAX               16
#define MAX_PORTS_PER_IRQ      8   /* covers COM1+COM3, Boca 8-port etc. */

/* Registered ports per IRQ line */
static pcbdcom_port_t *g_irq_ports[IRQ_MAX][MAX_PORTS_PER_IRQ];
static unsigned char   g_irq_count[IRQ_MAX];

/* Saved original interrupt vectors (for uninstall) */
static void (__interrupt __far *g_old_vector[IRQ_MAX])();
static int g_installed[IRQ_MAX];

/* PIC control */
void pcbdcom_pic_unmask(unsigned char irq)
{
    unsigned char port  = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    unsigned char bit   = 1 << (irq & 7);
    PIC_OUT(port, PIC_IN(port) & ~bit);
}

void pcbdcom_pic_mask(unsigned char irq)
{
    unsigned char port  = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    unsigned char bit   = 1 << (irq & 7);
    PIC_OUT(port, PIC_IN(port) | bit);
}

static void pcbdcom_pic_eoi(unsigned char irq)
{
    if (irq >= 8) PIC_OUT(PIC2_CMD, PIC_EOI);
    PIC_OUT(PIC1_CMD, PIC_EOI);
}

/* ----- Generic shared-IRQ handler ----- *
 * One of these per IRQ line. Walks the port table for that IRQ,
 * invokes each backend's ISR. Backend ISR is responsible for
 * checking its own hardware to see if it fired. */
static void pcbdcom_irq_dispatch(unsigned char irq)
{
    unsigned char i;
    for (i = 0; i < g_irq_count[irq]; i++) {
        pcbdcom_port_t *p = g_irq_ports[irq][i];
        if (p && p->backend && p->backend->isr)
            p->backend->isr(p);
    }
    pcbdcom_pic_eoi(irq);
}

/* Interrupt entry stubs — one per IRQ. Compiler generates the
 * IRET + register-save wrapper via __interrupt. */
static void __interrupt __far pcbdcom_isr3 (void) { pcbdcom_irq_dispatch(3);  }
static void __interrupt __far pcbdcom_isr4 (void) { pcbdcom_irq_dispatch(4);  }
static void __interrupt __far pcbdcom_isr5 (void) { pcbdcom_irq_dispatch(5);  }
static void __interrupt __far pcbdcom_isr7 (void) { pcbdcom_irq_dispatch(7);  }
static void __interrupt __far pcbdcom_isr10(void) { pcbdcom_irq_dispatch(10); }
static void __interrupt __far pcbdcom_isr11(void) { pcbdcom_irq_dispatch(11); }
static void __interrupt __far pcbdcom_isr12(void) { pcbdcom_irq_dispatch(12); }
static void __interrupt __far pcbdcom_isr15(void) { pcbdcom_irq_dispatch(15); }

static void (__interrupt __far * const g_isr_stubs[IRQ_MAX])() = {
    0, 0, 0,
    pcbdcom_isr3,  pcbdcom_isr4,  pcbdcom_isr5,  0, pcbdcom_isr7,
    0, 0,
    pcbdcom_isr10, pcbdcom_isr11, pcbdcom_isr12, 0, 0, pcbdcom_isr15
};

/* Register a port on an IRQ. Installs the vector on first port per IRQ. */
int pcbdcom_irq_register(unsigned char irq, pcbdcom_port_t *p)
{
    unsigned char vec;
    if (irq >= IRQ_MAX || !g_isr_stubs[irq]) return -1;
    if (g_irq_count[irq] >= MAX_PORTS_PER_IRQ) return -1;

    g_irq_ports[irq][g_irq_count[irq]++] = p;

    /* First port on this IRQ — install vector + unmask PIC */
    if (!g_installed[irq]) {
        vec = IRQ_TO_VECTOR(irq);
        g_old_vector[irq] = _dos_getvect(vec);
        _dos_setvect(vec, g_isr_stubs[irq]);
        pcbdcom_pic_unmask(irq);
        g_installed[irq] = 1;
    }
    return 0;
}

/* Uninstall all registered IRQs (called from TSR unload path) */
void pcbdcom_irq_shutdown(void)
{
    unsigned char irq;
    for (irq = 0; irq < IRQ_MAX; irq++) {
        if (g_installed[irq]) {
            pcbdcom_pic_mask(irq);
            _dos_setvect(IRQ_TO_VECTOR(irq), g_old_vector[irq]);
            g_installed[irq] = 0;
        }
        g_irq_count[irq] = 0;
    }
}
