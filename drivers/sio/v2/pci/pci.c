/* ====================================================================
 * pci.c — PCI.EXE Serial Card Detection (DOS program)
 * ====================================================================
 * DOS real-mode program. Scans PCI bus for serial port controllers.
 * Matches against PCI.INC device database.
 *
 * Three access methods (matching original):
 *   /pcib  PCI BIOS via INT 1Ah (default if BIOS supports it)
 *   /pci1  Mechanism #1: I/O ports 0xCF8 (address) + 0xCFC (data)
 *   /pci2  Mechanism #2: I/O port 0xC000-0xCFFF (deprecated)
 *
 * Usage:
 *   PCI                 Scan with auto-detect method
 *   PCI /pci1           Force Mechanism #1
 *   PCI /pci2           Force Mechanism #2
 *   PCI /pcib           Force PCI BIOS
 *   PCI /D              Dump config to PCI_REGS.DAT
 *   PCI /V              Verbose — show all devices
 *
 * Clean-room reimplementation from PCI.TXT + binary analysis.
 * GPLv3 — FPC264IRC Contributors, 2026.
 * ==================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>

#ifdef __WATCOMC__
#include <conio.h>
#define inpd(p)    inpd(p)
#define outpd(p,v) outpd(p,v)
#else
/* Borland C / DJGPP */
unsigned long inpd(unsigned short p) {
    unsigned long v; __asm { mov dx,p; in eax,dx; mov v,eax }; return v;
}
void outpd(unsigned short p, unsigned long v) {
    __asm { mov dx,p; mov eax,v; out dx,eax };
}
#endif

#define VERSION     "2.2"
#define MAX_DEVICES 128
#define MAX_DB      128
#define MAX_CONDS   5
#define MAX_UARTS   8

/* PCI config register offsets */
#define PCI_VENDOR      0x00
#define PCI_DEVICE      0x02
#define PCI_COMMAND     0x04
#define PCI_STATUS      0x06
#define PCI_REVISION    0x08
#define PCI_CLASS_PROG  0x09
#define PCI_CLASS_SUB   0x0A
#define PCI_CLASS_BASE  0x0B
#define PCI_HEADER      0x0E
#define PCI_BAR0        0x10
#define PCI_SUBSYS_VEN  0x2C
#define PCI_SUBSYS_ID   0x2E
#define PCI_IRQ         0x3C

/* Access methods */
#define METHOD_BIOS     0
#define METHOD_MECH1    1
#define METHOD_MECH2    2

/* PCI class for serial */
#define CLASS_COMM      0x07

static int g_method = -1;   /* -1 = auto-detect */

/* ---- PCI device storage ---- */

typedef struct {
    unsigned char bus, dev, func;
    unsigned char cfg[256];
    int valid;
} PCI_DEV;

static PCI_DEV g_devs[MAX_DEVICES];
static int g_devCount = 0;

/* ---- PCI.INC database ---- */

typedef struct {
    char            name[80];
    int             numConds;
    unsigned short  condOff[MAX_CONDS];
    unsigned short  condVal[MAX_CONDS];
    int             numUarts;
    unsigned char   uartBar[MAX_UARTS];
    unsigned short  uartSpacing;
} PCI_DB_ENTRY;

static PCI_DB_ENTRY g_db[MAX_DB];
static int g_dbCount = 0;

/* ---- Config space helpers ---- */

static unsigned short cfg16(unsigned char *c, int off)
{
    return (unsigned short)c[off] | ((unsigned short)c[off+1] << 8);
}

static unsigned long cfg32(unsigned char *c, int off)
{
    return (unsigned long)c[off] | ((unsigned long)c[off+1] << 8) |
           ((unsigned long)c[off+2] << 16) | ((unsigned long)c[off+3] << 24);
}


/* ================================================================
 * PCI BIOS (INT 1Ah)
 * ================================================================ */

static int bios_present(void)
{
    union REGS r;
    r.x.ax = 0xB101;
    int86(0x1A, &r, &r);
    if (r.h.ah != 0) return 0;
    if (r.x.dx != 0x4350) return 0;  /* 'CP' signature */
    return 1;
}

static int bios_read_byte(unsigned char bus, unsigned char dev,
                           unsigned char func, unsigned char reg)
{
    union REGS r;
    r.x.ax = 0xB108;
    r.h.bh = bus;
    r.h.bl = (dev << 3) | (func & 7);
    r.x.di = reg;
    int86(0x1A, &r, &r);
    if (r.h.ah != 0) return -1;
    return r.h.cl;
}

static int bios_read_dword(unsigned char bus, unsigned char dev,
                            unsigned char func, unsigned char reg)
{
    union REGS r;
    r.x.ax = 0xB10A;
    r.h.bh = bus;
    r.h.bl = (dev << 3) | (func & 7);
    r.x.di = reg;
    int86(0x1A, &r, &r);
    if (r.h.ah != 0) return -1;
    return r.x.cx;  /* ECX in 32-bit, CX in 16-bit — need 32-bit call */
}

static int bios_read_config(unsigned char bus, unsigned char dev,
                             unsigned char func, unsigned char *cfg)
{
    int i;
    for (i = 0; i < 256; i++) {
        int val = bios_read_byte(bus, dev, func, (unsigned char)i);
        if (val < 0 && i == 0) return -1;
        cfg[i] = (unsigned char)(val >= 0 ? val : 0xFF);
    }
    return 0;
}


/* ================================================================
 * MECHANISM #1 (I/O ports 0xCF8 / 0xCFC)
 * ================================================================ */

static int mech1_present(void)
{
    unsigned long save, test;
    save = inpd(0xCF8);
    outpd(0xCF8, 0x80000000UL);
    test = inpd(0xCF8);
    outpd(0xCF8, save);
    return (test == 0x80000000UL) ? 1 : 0;
}

static unsigned long mech1_read(unsigned char bus, unsigned char dev,
                                 unsigned char func, unsigned char reg)
{
    unsigned long addr = 0x80000000UL |
        ((unsigned long)bus << 16) |
        ((unsigned long)(dev & 0x1F) << 11) |
        ((unsigned long)(func & 0x07) << 8) |
        (reg & 0xFC);
    outpd(0xCF8, addr);
    return inpd(0xCFC);
}

static int mech1_read_config(unsigned char bus, unsigned char dev,
                              unsigned char func, unsigned char *cfg)
{
    int i;
    for (i = 0; i < 256; i += 4) {
        unsigned long val = mech1_read(bus, dev, func, (unsigned char)i);
        cfg[i]   = (unsigned char)(val);
        cfg[i+1] = (unsigned char)(val >> 8);
        cfg[i+2] = (unsigned char)(val >> 16);
        cfg[i+3] = (unsigned char)(val >> 24);
    }
    if (cfg16(cfg, 0) == 0xFFFF) return -1;
    return 0;
}


/* ================================================================
 * MECHANISM #2 (I/O port 0xC000 range — deprecated)
 * ================================================================ */

static int mech2_read_config(unsigned char bus, unsigned char dev,
                              unsigned char func, unsigned char *cfg)
{
    int i;
    unsigned short base;

    /* Mechanism #2: enable via 0xCF8, access via 0xC000 range */
    outp(0xCF8, 0x80 | ((func & 7) << 1));
    outp(0xCFA, bus);

    base = (unsigned short)(0xC000 | (dev << 8));

    for (i = 0; i < 256; i++) {
        cfg[i] = inp(base + i);
    }

    outp(0xCF8, 0);  /* Disable */

    if (cfg16(cfg, 0) == 0xFFFF) return -1;
    return 0;
}


/* ================================================================
 * BUS SCAN
 * ================================================================ */

typedef int (*read_config_fn)(unsigned char, unsigned char, unsigned char, unsigned char *);

static void pci_scan(read_config_fn readfn)
{
    unsigned char bus, dev, func;
    unsigned char cfg[256];

    g_devCount = 0;
    for (bus = 0; bus < 8; bus++) {
        for (dev = 0; dev < 32; dev++) {
            for (func = 0; func < 8; func++) {
                if (readfn(bus, dev, func, cfg) != 0) continue;

                if (g_devCount >= MAX_DEVICES) return;
                g_devs[g_devCount].bus = bus;
                g_devs[g_devCount].dev = dev;
                g_devs[g_devCount].func = func;
                memcpy(g_devs[g_devCount].cfg, cfg, 256);
                g_devs[g_devCount].valid = 1;
                g_devCount++;

                if (func == 0 && !(cfg[PCI_HEADER] & 0x80)) break;
            }
        }
    }
}


/* ================================================================
 * PCI.INC DATABASE
 * ================================================================ */

static void load_pci_inc(void)
{
    FILE *f;
    char line[512];

    f = fopen("PCI.INC", "r");
    if (!f) { printf("Warning: PCI.INC not found\n"); return; }

    while (fgets(line, sizeof(line), f) && g_dbCount < MAX_DB) {
        PCI_DB_ENTRY *e = &g_db[g_dbCount];
        char *p = line, *q;

        q = strchr(p, ';'); if (q) *q = '\0';
        while (*p == ' ' || *p == '\t') p++;
        if (!*p || *p == '\n') continue;

        memset(e, 0, sizeof(*e));

        /* Extract quoted name */
        q = strchr(p, '"');
        if (q) { q++; char *end = strchr(q, '"');
            if (end) { int len = (int)(end - q); if (len > 79) len = 79;
                strncpy(e->name, q, len); p = end + 1; } }

        /* Parse offset=value conditions */
        while (*p && e->numConds < MAX_CONDS) {
            while (*p == ' ' || *p == '\t') p++;
            if (!*p || *p == '\n') break;
            if (strnicmp(p, "uart", 4) == 0) break;
            if (isxdigit(*p)) {
                unsigned short off = (unsigned short)strtol(p, &p, 16);
                if (*p == '=') { p++;
                    unsigned short val = (unsigned short)strtol(p, &p, 16);
                    e->condOff[e->numConds] = off;
                    e->condVal[e->numConds] = val;
                    e->numConds++;
                }
            } else p++;
        }

        /* Parse UART entries */
        while (*p) {
            while (*p == ' ' || *p == '\t') p++;
            if (!*p || *p == '\n') break;
            if (strnicmp(p, "uart", 4) == 0) {
                p += 4; while (isdigit(*p)) p++;
                if (*p == '@' || *p == '=') { p++;
                    if (*p == '+') { p++; e->uartSpacing = (unsigned short)strtol(p, &p, 16); }
                    else if (e->numUarts < MAX_UARTS)
                        e->uartBar[e->numUarts++] = (unsigned char)strtol(p, &p, 16);
                }
            } else p++;
        }

        if (e->numConds > 0) g_dbCount++;
    }
    fclose(f);
}

static PCI_DB_ENTRY *match_dev(unsigned char *cfg)
{
    int i, j;
    for (i = 0; i < g_dbCount; i++) {
        int ok = 1;
        for (j = 0; j < g_db[i].numConds; j++)
            if (cfg16(cfg, g_db[i].condOff[j]) != g_db[i].condVal[j]) { ok = 0; break; }
        if (ok) return &g_db[i];
    }
    return NULL;
}


/* ================================================================
 * DISPLAY + DUMP
 * ================================================================ */

static void show_dev(PCI_DEV *d, int verbose)
{
    unsigned char classBase = d->cfg[PCI_CLASS_BASE];
    PCI_DB_ENTRY *e;

    if (!verbose && classBase != CLASS_COMM) return;

    printf("Bus %d Dev %2d Fn %d: Vendor=%04X Device=%04X Rev=%02X Class=%02X.%02X IRQ=%d\n",
           d->bus, d->dev, d->func,
           cfg16(d->cfg, PCI_VENDOR), cfg16(d->cfg, PCI_DEVICE),
           d->cfg[PCI_REVISION], classBase, d->cfg[PCI_CLASS_SUB],
           d->cfg[PCI_IRQ]);

    { int b; for (b = 0; b < 6; b++) {
        unsigned long bar = cfg32(d->cfg, PCI_BAR0 + b * 4);
        if (bar & 1) printf("  BAR%d: I/O  %04lXh\n", b, bar & 0xFFFC);
        else if (bar) printf("  BAR%d: MEM  %08lXh\n", b, bar & 0xFFFFFFF0UL);
    }}

    e = match_dev(d->cfg);
    if (e) {
        int u; unsigned long last = 0;
        printf("  ** %s **\n", e->name);
        for (u = 0; u < e->numUarts; u++) {
            unsigned long addr;
            if (e->uartBar[u] == 0xFF) addr = last + e->uartSpacing;
            else addr = cfg32(d->cfg, e->uartBar[u]) & 0xFFFC;
            printf("  UART%d: %04lXh\n", u, addr);
            last = addr;
        }
        /* SIO2K.CFG suggestion */
        printf("  SIO2K.CFG: DEVICE=SIO2K.SYS");
        last = 0;
        for (u = 0; u < e->numUarts; u++) {
            unsigned long addr;
            if (e->uartBar[u] == 0xFF) addr = last + e->uartSpacing;
            else addr = cfg32(d->cfg, e->uartBar[u]) & 0xFFFC;
            printf(" (COM%d,%lXh", u + 5, addr);
            if (u == 0 && d->cfg[PCI_IRQ]) printf(",IRQ%d", d->cfg[PCI_IRQ]);
            else printf(",NONE");
            printf(")");
            last = addr;
        }
        printf("\n");
    }
    printf("\n");
}

static void dump_regs(void)
{
    FILE *f; int i, j;
    f = fopen("PCI_REGS.DAT", "w");
    if (!f) { printf("Cannot create PCI_REGS.DAT\n"); return; }
    fprintf(f, "PCI Register Dump — PCI.EXE v%s\n\n", VERSION);
    for (i = 0; i < g_devCount; i++) {
        PCI_DEV *d = &g_devs[i];
        fprintf(f, "Bus %d Dev %d Fn %d: Vendor=%04X Device=%04X\n",
                d->bus, d->dev, d->func, cfg16(d->cfg, 0), cfg16(d->cfg, 2));
        for (j = 0; j < 256; j += 16) {
            int k; fprintf(f, "  %02X:", j);
            for (k = 0; k < 16; k++) fprintf(f, " %02X", d->cfg[j+k]);
            fprintf(f, "\n");
        }
        fprintf(f, "\n");
    }
    fclose(f);
    printf("PCI register dump saved to PCI_REGS.DAT\n");
}


/* ================================================================
 * MAIN
 * ================================================================ */

int main(int argc, char *argv[])
{
    int verbose = 0, dump = 0, i;
    read_config_fn readfn;
    const char *methName;

    printf("  PCI.exe%46sVersion %s   \n", "", VERSION);
    printf("  SIO2K PCI Serial Card Detection\n");
    printf("  GPLv3 — FPC264IRC Contributors, 2026\n\n");

    for (i = 1; i < argc; i++) {
        if (stricmp(argv[i], "/pci1") == 0) g_method = METHOD_MECH1;
        else if (stricmp(argv[i], "/pci2") == 0) g_method = METHOD_MECH2;
        else if (stricmp(argv[i], "/pcib") == 0) g_method = METHOD_BIOS;
        else if (stricmp(argv[i], "/V") == 0) verbose = 1;
        else if (stricmp(argv[i], "/D") == 0) dump = 1;
        else if (argv[i][0] == '/' || argv[i][0] == '?') {
            printf("Valid command line options:\n");
            printf("   /pci1 Use PCI Mechanism #1 to access PCI Configuration Space\n");
            printf("   /pci2 Use PCI Mechanism #2 to access PCI Configuration Space\n");
            printf("=> /pcib Use BIOS PCI Interrupts to access PCI Configuration Space\n");
            printf("         (Note:  this is only the default if PCI BIOS Functions are\n");
            printf("          supported, otherwise /pci1 is the default)\n");
            printf("   /V    Verbose — show all PCI devices\n");
            printf("   /D    Dump PCI config space to PCI_REGS.DAT\n");
            return 0;
        }
    }

    /* Auto-detect access method */
    if (g_method < 0) {
        if (bios_present()) g_method = METHOD_BIOS;
        else if (mech1_present()) g_method = METHOD_MECH1;
        else g_method = METHOD_MECH2;
    }

    switch (g_method) {
    case METHOD_BIOS:  readfn = bios_read_config; methName = "BIOS"; break;
    case METHOD_MECH1: readfn = mech1_read_config; methName = "Mechanism #1"; break;
    case METHOD_MECH2: readfn = mech2_read_config; methName = "Mechanism #2"; break;
    default:           readfn = mech1_read_config; methName = "Mechanism #1"; break;
    }

    printf("PCI access method: %s\n", methName);

    load_pci_inc();
    printf("Database: %d card definitions loaded\n\n", g_dbCount);

    pci_scan(readfn);
    printf("# of PCI Devices found: %d\n\n", g_devCount);

    if (g_devCount == 0) {
        printf("No PCI devices found.\n");
        if (g_method == METHOD_BIOS) printf("Try: PCI /pci1  (to try Mechanism #1)\n");
        else printf("Try: PCI /pcib  (to try BIOS access)\n");
        return 1;
    }

    for (i = 0; i < g_devCount; i++)
        show_dev(&g_devs[i], verbose);

    if (dump) dump_regs();

    return 0;
}
