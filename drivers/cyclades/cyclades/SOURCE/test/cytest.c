/* ====================================================================
 * CYTEST.C — Cyclades CD1400 Hardware Detection and Test
 * ====================================================================
 * Detects Cyclom-Y cards by scanning for CD1400 chips in the
 * shared memory window. Reports revision, port count, and tests
 * basic read/write on each channel.
 *
 * Usage: CYTEST [membase]
 *   membase = shared memory base in hex (default D4000)
 *
 * Works on DOS (real mode, direct hardware access).
 * ====================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include "../inc/cd1400.h"

/* Memory-mapped access via far pointers (DOS real mode) */
typedef unsigned char far *FPBYTE;

static FPBYTE g_base;       /* Card base address as far pointer     */
static unsigned long g_memBase = CY_DEFAULT_MEMBASE;

/* Read a CD1400 register */
static unsigned char cy_read(int chip, int reg)
{
    FPBYTE addr = g_base + (chip * CY_REG_SIZE) + reg;
    return *addr;
}

/* Write a CD1400 register */
static void cy_write(int chip, int reg, unsigned char val)
{
    FPBYTE addr = g_base + (chip * CY_REG_SIZE) + reg;
    *addr = val;
}

/* Select a channel on a chip */
static void cy_select_channel(int chip, int chan)
{
    cy_write(chip, CyCAR, chan & 0x03);
}

/* Detect a CD1400 chip — returns revision or 0 if not found */
static unsigned char detect_chip(int chip)
{
    unsigned char rev;

    /* Read Global Firmware Revision Code Register */
    rev = cy_read(chip, CyGFRCR);

    /* Valid CD1400 revisions: Rev G (0x46) or Rev J (0x48) */
    if (rev == CD1400_REV_G || rev == CD1400_REV_J)
        return rev;

    /* Some clones may have different revision codes */
    /* Check if the register responds sensibly */
    if (rev >= 0x40 && rev <= 0x4F)
        return rev;     /* Probably a CD1400 variant */

    return 0;           /* Not a CD1400 */
}

/* Get clock frequency from revision */
static const char *clock_str(unsigned char rev)
{
    if (rev == CD1400_REV_J) return "60 MHz";
    if (rev == CD1400_REV_G) return "25 MHz";
    return "unknown";
}

/* Test a single channel — loopback write/read */
static int test_channel(int chip, int chan)
{
    unsigned char cor1, ccsr;

    cy_select_channel(chip, chan);

    /* Read COR1 to verify channel access */
    cor1 = cy_read(chip, CyCOR1);

    /* Read CCSR — should have some valid state bits */
    ccsr = cy_read(chip, CyCCSR);

    /* Check if channel responds (non-FF, non-00 is a good sign) */
    if (cor1 == 0xFF && ccsr == 0xFF)
        return 0;   /* No response — channel dead */

    return 1;       /* Channel responds */
}

/* Reset a chip */
static void reset_chip(int chip)
{
    cy_select_channel(chip, 0);
    cy_write(chip, CyCCR, CyCHIP_RESET);

    /* Wait for reset to complete */
    {
        int timeout = 1000;
        while (timeout-- > 0) {
            if (cy_read(chip, CyCCR) == 0)
                break;
        }
    }
}

int main(int argc, char *argv[])
{
    int chip, chan;
    int totalChips = 0;
    int totalPorts = 0;
    unsigned char rev;
    unsigned short seg;

    printf("CYTEST — Cyclades CD1400 Detection Utility\n");
    printf("==========================================\n\n");

    /* Parse optional memory base */
    if (argc > 1) {
        g_memBase = strtoul(argv[1], NULL, 16);
    }

    printf("Scanning memory window at %05lXh (segment %04Xh)\n\n",
           g_memBase, (unsigned short)(g_memBase >> 4));

    /* Set up far pointer to card memory */
    seg = (unsigned short)(g_memBase >> 4);
    g_base = (FPBYTE)MK_FP(seg, 0);

    /* Check for card presence — read EPLD revision */
    {
        FPBYTE epld = g_base + CY_EPLD_REV;
        unsigned char epldRev = *epld;
        if (epldRev == 0xFF || epldRev == 0x00) {
            printf("No Cyclades card detected at %05lXh\n", g_memBase);
            printf("(EPLD revision register reads %02Xh)\n", epldRev);
            printf("\nTry different memory addresses:\n");
            printf("  CYTEST D0000\n");
            printf("  CYTEST D2000\n");
            printf("  CYTEST D4000\n");
            printf("  CYTEST D6000\n");
            printf("  CYTEST D8000\n");
            return 1;
        }
        printf("EPLD revision: %02Xh\n\n", epldRev);
    }

    /* Scan for CD1400 chips (up to 8 per card) */
    printf("Chip  Revision    Clock     Ports  Status\n");
    printf("----  --------    --------  -----  ------\n");

    for (chip = 0; chip < CY_MAX_CHIPS; chip++) {
        rev = detect_chip(chip);
        if (rev == 0) {
            /* No more chips */
            break;
        }

        totalChips++;
        printf("  %d   CD1400 %c    %-8s  ",
               chip, (rev == CD1400_REV_J) ? 'J' : 'G',
               clock_str(rev));

        /* Test each channel on this chip */
        {
            int goodChans = 0;
            for (chan = 0; chan < CY_PORTS_PER_CHIP; chan++) {
                if (test_channel(chip, chan))
                    goodChans++;
            }
            printf("%d/4   ", goodChans);
            totalPorts += goodChans;

            if (goodChans == 4)
                printf("OK\n");
            else if (goodChans > 0)
                printf("PARTIAL (%d dead)\n", 4 - goodChans);
            else
                printf("FAIL\n");
        }
    }

    printf("\n==========================================\n");
    printf("Found: %d chip(s), %d port(s)\n", totalChips, totalPorts);

    if (totalChips == 0) {
        printf("No CD1400 chips detected.\n");
        return 1;
    }

    printf("\nCard type: Cyclom-%dY", totalChips * CY_PORTS_PER_CHIP);
    if (totalChips > 1)
        printf(" (%d-port)", totalPorts);
    printf("\n");

    /* Show FOSSIL driver load command */
    printf("\nTo use with FOSSIL:\n");
    printf("  DEVICE=CYFOSSIL.SYS %04X %d\n",
           (unsigned short)(g_memBase >> 4), totalPorts);

    return 0;
}
