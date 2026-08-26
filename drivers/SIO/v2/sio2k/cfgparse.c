/* ====================================================================
 * cfgparse.c — SIO2K.CFG Configuration File Parser
 * ====================================================================
 * Parses the SIO2K configuration file with three section types:
 *   Os2Device  — defines OS/2 COM port devices
 *   BaseUart   — configures UART.SYS physical driver
 *   DosDevice  — defines DOS/VDM virtual port mappings
 * ====================================================================
 */

#include "cfgparse.h"
#include <string.h>

/* -------------------------------------------------------------------- */
/* Parser State                                                         */
/* -------------------------------------------------------------------- */

#define MAX_LINE    256
#define MAX_DEVICES 256

typedef enum {
    SEC_NONE,
    SEC_OS2DEVICE,
    SEC_BASEUART,
    SEC_DOSDEVICE
} SECTION;

static SIO2K_CONFIG g_config;
static SECTION      g_curSection;
static int          g_curOs2Dev;    /* Index into os2dev[] being built */
static int          g_curDosdev;    /* Index into dosdev[] being built */
static int          g_curUart;      /* Index into baseuart[] being built */

/* -------------------------------------------------------------------- */
/* Helpers                                                              */
/* -------------------------------------------------------------------- */

static void TrimLine(char *line)
{
    char *p;
    /* Strip comments (semicolon) */
    p = strchr(line, ';');
    if (p) *p = '\0';
    /* Strip trailing whitespace */
    p = line + strlen(line) - 1;
    while (p >= line && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
        *p-- = '\0';
}

static void SkipSpace(const char **pp)
{
    while (**pp == ' ' || **pp == '\t') (*pp)++;
}

static int StrIEq(const char *a, const char *b)
{
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return 0;
        a++; b++;
    }
    return (*a == *b);
}

static int StrIStartsWith(const char *line, const char *prefix)
{
    while (*prefix) {
        char cl = *line, cp = *prefix;
        if (cl >= 'a' && cl <= 'z') cl -= 32;
        if (cp >= 'a' && cp <= 'z') cp -= 32;
        if (cl != cp) return 0;
        line++; prefix++;
    }
    return 1;
}

static unsigned long ParseHex(const char *s)
{
    unsigned long val = 0;
    while (*s) {
        char c = *s++;
        if (c >= '0' && c <= '9') val = (val << 4) | (c - '0');
        else if (c >= 'a' && c <= 'f') val = (val << 4) | (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') val = (val << 4) | (c - 'A' + 10);
        else break;
    }
    return val;
}

static unsigned long ParseDec(const char *s)
{
    unsigned long val = 0;
    while (*s >= '0' && *s <= '9')
        val = val * 10 + (*s++ - '0');
    return val;
}

static const char *GetValue(const char *line)
{
    const char *p = strchr(line, '=');
    if (!p) return NULL;
    p++;
    SkipSpace(&p);
    return p;
}

/* -------------------------------------------------------------------- */
/* Section Parsers                                                      */
/* -------------------------------------------------------------------- */

static void ParseOs2Device(const char *line)
{
    const char *val;
    OS2DEV_CFG *dev;

    if (g_curOs2Dev < 0 || g_curOs2Dev >= MAX_DEVICES) return;
    dev = &g_config.os2dev[g_curOs2Dev];

    if (StrIStartsWith(line, "Name")) {
        val = GetValue(line);
        if (val) {
            int i;
            for (i = 0; i < 8 && val[i] && val[i] != ' '; i++)
                dev->name[i] = val[i];
            dev->name[i] = '\0';
        }
    }
    else if (StrIStartsWith(line, "AltDriver")) {
        val = GetValue(line);
        if (val) {
            /* Format: name,index — e.g., "uart$,1" */
            int i;
            for (i = 0; i < 8 && val[i] && val[i] != ','; i++)
                dev->altDriverName[i] = val[i];
            dev->altDriverName[i] = '\0';
            if (val[i] == ',')
                dev->altDriverPort = (unsigned short)ParseDec(&val[i+1]);
        }
    }
    else if (StrIStartsWith(line, "LockedBitRate")) {
        val = GetValue(line);
        if (val) {
            dev->lockedBaud = ParseDec(val);
            dev->baudLocked = 1;
        }
    }
    else if (StrIStartsWith(line, "Os2Shares")) {
        dev->os2Shares = 1;
    }
}

static void ParseBaseUart(const char *line)
{
    const char *val;
    BASEUART_CFG *uart;

    if (g_curUart < 0 || g_curUart >= MAX_DEVICES) return;
    uart = &g_config.baseuart[g_curUart];

    if (StrIStartsWith(line, "SuperIO")) {
        g_config.superIOEnabled = 1;
    }
    else if (StrIStartsWith(line, "ExclusiveIRQ")) {
        uart->exclusiveIRQ = 1;
    }
    else if (StrIStartsWith(line, "Hardware")) {
        val = GetValue(line);
        if (val) {
            if (StrIEq(val, "AutoDetect"))
                uart->hwMode = HW_AUTODETECT;
            else if (StrIEq(val, "PCI"))
                uart->hwMode = HW_PCI;
            else if (StrIEq(val, "8250") || StrIEq(val, "16450"))
                uart->hwMode = HW_FORCE_8250;
            else if (StrIEq(val, "16550"))
                uart->hwMode = HW_FORCE_16550;
            else if (StrIEq(val, "16650") || StrIEq(val, "16650A"))
                uart->hwMode = HW_FORCE_16650;
            else if (StrIEq(val, "16654"))
                uart->hwMode = HW_FORCE_16654;
            else if (StrIEq(val, "16750"))
                uart->hwMode = HW_FORCE_16750;
            else if (StrIEq(val, "16850"))
                uart->hwMode = HW_FORCE_16850;
            else if (StrIEq(val, "16950"))
                uart->hwMode = HW_FORCE_16950;
            else
                uart->hwMode = HW_AUTODETECT;
        }
    }
    else if (StrIStartsWith(line, "IO_Address")) {
        val = GetValue(line);
        if (val) {
            if (StrIEq(val, "AutoDetect"))
                uart->ioAddr = 0;   /* Auto */
            else if (StrIEq(val, "BIOS"))
                uart->ioAddr = 0xFFFF;  /* Use BIOS data area */
            else
                uart->ioAddr = (unsigned short)ParseHex(val);
        }
    }
}

static void ParseDosDevice(const char *line)
{
    const char *val;
    DOSDEV_CFG *dev;

    if (g_curDosdev < 0 || g_curDosdev >= MAX_DEVICES) return;
    dev = &g_config.dosdev[g_curDosdev];

    if (StrIStartsWith(line, "Os2DevName")) {
        val = GetValue(line);
        if (val) {
            int i;
            for (i = 0; i < 8 && val[i] && val[i] != ' '; i++)
                dev->os2DevName[i] = val[i];
            dev->os2DevName[i] = '\0';
        }
    }
    else if (StrIStartsWith(line, "VirtualIO")) {
        val = GetValue(line);
        if (val) {
            if (StrIStartsWith(val, "BiosRamCom"))
                dev->virtualIO = 0xFFFF;    /* Use BIOS mapping */
            else
                dev->virtualIO = (unsigned short)ParseHex(val);
        }
    }
    else if (StrIStartsWith(line, "VirtualIRQ")) {
        val = GetValue(line);
        if (val)
            dev->virtualIRQ = (unsigned char)ParseDec(val);
    }
    else if (StrIStartsWith(line, "DosShares")) {
        dev->dosShares = 1;
    }
    else if (StrIStartsWith(line, "VirtualUart")) {
        val = GetValue(line);
        if (val) {
            if (StrIEq(val, "16550"))
                dev->virtualUartType = 1;   /* 16550 emulation */
            else
                dev->virtualUartType = 0;   /* 16450 emulation */
        }
    }
}

/* -------------------------------------------------------------------- */
/* Main Parser                                                          */
/* -------------------------------------------------------------------- */

int CfgParse(const char *filename, SIO2K_CONFIG *pCfg)
{
    char line[MAX_LINE];
    FILE *f;

    /* Initialize config to defaults */
    memset(&g_config, 0, sizeof(g_config));
    g_config.superIOEnabled = 0;
    g_curSection = SEC_NONE;
    g_curOs2Dev  = -1;
    g_curDosdev  = -1;
    g_curUart    = -1;

    f = fopen(filename, "r");
    if (!f) return -1;  /* No config file — use defaults */

    while (fgets(line, MAX_LINE, f)) {
        TrimLine(line);

        /* Skip empty lines */
        if (line[0] == '\0') continue;

        /* Check for section headers (start in column 0) */
        if (line[0] != ' ' && line[0] != '\t') {
            if (StrIStartsWith(line, "Os2Device")) {
                g_curSection = SEC_OS2DEVICE;
                g_curOs2Dev = g_config.numOs2Dev++;
            }
            else if (StrIStartsWith(line, "BaseUart")) {
                g_curSection = SEC_BASEUART;
                g_curUart = g_config.numBaseUart++;
            }
            else if (StrIStartsWith(line, "DosDevice")) {
                g_curSection = SEC_DOSDEVICE;
                g_curDosdev = g_config.numDosDev++;
            }
            continue;
        }

        /* Parse option within current section */
        SkipSpace((const char **)&line);

        switch (g_curSection) {
        case SEC_OS2DEVICE:
            ParseOs2Device(line);
            break;
        case SEC_BASEUART:
            ParseBaseUart(line);
            break;
        case SEC_DOSDEVICE:
            ParseDosDevice(line);
            break;
        default:
            break;
        }
    }

    fclose(f);

    /* Copy result */
    memcpy(pCfg, &g_config, sizeof(SIO2K_CONFIG));
    return 0;
}

/* -------------------------------------------------------------------- */
/* Default Config (no config file)                                      */
/* -------------------------------------------------------------------- */

void CfgDefault(SIO2K_CONFIG *pCfg)
{
    int i;
    static const unsigned short defAddr[] = { 0x3F8, 0x2F8, 0x3E8, 0x2E8 };

    memset(pCfg, 0, sizeof(SIO2K_CONFIG));

    /* Default: COM1-COM4 with UART$ as physical driver */
    pCfg->numOs2Dev = 4;
    for (i = 0; i < 4; i++) {
        pCfg->os2dev[i].name[0] = 'c';
        pCfg->os2dev[i].name[1] = 'o';
        pCfg->os2dev[i].name[2] = 'm';
        pCfg->os2dev[i].name[3] = '1' + i;
        pCfg->os2dev[i].name[4] = '\0';
        memcpy(pCfg->os2dev[i].altDriverName, "uart$", 6);
        pCfg->os2dev[i].altDriverPort = i + 1;
    }

    /* Default: 4 BaseUart entries with auto-detect */
    pCfg->numBaseUart = 4;
    for (i = 0; i < 4; i++) {
        pCfg->baseuart[i].hwMode = HW_AUTODETECT;
        pCfg->baseuart[i].ioAddr = defAddr[i];
        pCfg->baseuart[i].exclusiveIRQ = 0;
    }

    /* Default: 2 DosDevice entries (COM1, COM2) */
    pCfg->numDosDev = 2;
    memcpy(pCfg->dosdev[0].os2DevName, "com1", 5);
    pCfg->dosdev[0].virtualIO = 0xFFFF;  /* BiosRamCom1 */
    pCfg->dosdev[0].virtualIRQ = 4;
    pCfg->dosdev[0].virtualUartType = 1;  /* 16550 */
    memcpy(pCfg->dosdev[1].os2DevName, "com2", 5);
    pCfg->dosdev[1].virtualIO = 0xFFFF;
    pCfg->dosdev[1].virtualIRQ = 3;
    pCfg->dosdev[1].virtualUartType = 1;
}
