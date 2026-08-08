/* ====================================================================
 * cfgparse.h — SIO2K Configuration File Data Structures
 * ==================================================================== */

#ifndef CFGPARSE_H
#define CFGPARSE_H

#include <stdio.h>

#define MAX_CFG_DEVICES 256

/* Hardware detection mode */
#define HW_AUTODETECT   0
#define HW_PCI          1
#define HW_FORCE_8250   10
#define HW_FORCE_16550  11
#define HW_FORCE_16650  12
#define HW_FORCE_16654  13
#define HW_FORCE_16750  14
#define HW_FORCE_16850  15
#define HW_FORCE_16950  16

/* Os2Device section */
typedef struct _OS2DEV_CFG {
    char            name[9];            /* Device name (e.g., "com1")   */
    char            altDriverName[9];   /* IDC driver name ("uart$")    */
    unsigned short  altDriverPort;      /* Port index within driver     */
    unsigned long   lockedBaud;         /* Locked baud rate (0=none)    */
    unsigned char   baudLocked;         /* Non-zero if baud locked      */
    unsigned char   os2Shares;          /* Allow DOS access when open   */
} OS2DEV_CFG;

/* BaseUart section */
typedef struct _BASEUART_CFG {
    unsigned char   hwMode;             /* HW_xxx detection mode        */
    unsigned short  ioAddr;             /* I/O address (0=auto)         */
    unsigned char   exclusiveIRQ;       /* Use IRQ exclusively          */
} BASEUART_CFG;

/* DosDevice section */
typedef struct _DOSDEV_CFG {
    char            os2DevName[9];      /* Links to Os2Device name      */
    unsigned short  virtualIO;          /* Virtual I/O port (FFFFh=BIOS)*/
    unsigned char   virtualIRQ;         /* Virtual IRQ for DOS session  */
    unsigned char   dosShares;          /* OS/2 can access DOS port     */
    unsigned char   virtualUartType;    /* 0=16450, 1=16550 emulation   */
} DOSDEV_CFG;

/* Complete configuration */
typedef struct _SIO2K_CONFIG {
    int             numOs2Dev;
    int             numBaseUart;
    int             numDosDev;
    unsigned char   superIOEnabled;
    OS2DEV_CFG      os2dev[MAX_CFG_DEVICES];
    BASEUART_CFG    baseuart[MAX_CFG_DEVICES];
    DOSDEV_CFG      dosdev[MAX_CFG_DEVICES];
} SIO2K_CONFIG;

/* Parser functions */
int  CfgParse(const char *filename, SIO2K_CONFIG *pCfg);
void CfgDefault(SIO2K_CONFIG *pCfg);

#endif /* CFGPARSE_H */
