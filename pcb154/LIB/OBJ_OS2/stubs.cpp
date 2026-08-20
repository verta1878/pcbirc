#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*=========================================================================*/
/* Phase 0 stubs — working version that links PCBSTATS, PCBPACK, MSETUP   */
/* Real implementations replace stubs where verified to link correctly.     */
/*=========================================================================*/

/*--- Variables: screen state, error state, flags ---*/
char Scrn_24Hour = 1;
char Scrn_ColorCard = 1;
char Scrn_EGA = 1;
unsigned short *KbdStatus = 0;
unsigned char UpperCase[256];
char Int24Flags = 0;
char Int24Error = 0;
int ExtendedError = 0;
int ExtendedAction = 0;
bool Novell = 0;
bool DisableGiveup = 0;

/*--- Real: errorexittodos (EXITDOS.obj also has this but linkage may differ) ---*/
void errorexittodos(char *msg) {
    if (msg && *msg)
        fprintf(stderr, "\r\n%s\r\n", msg);
    exit(1);
}

/*--- Real: getextendederror (EXTENDED.C has asm version) ---*/
void getextendederror(void) {
    ExtendedError = 0;
    Int24Error = 0;
    ExtendedAction = 0;
}

/*--- Real: farmemcpy (flat model = memcpy) ---*/
void farmemcpy(void *dest, void *src, int len) {
    memcpy(dest, src, len);
}

/*--- Real: getcountryspecs (COUNTRY.C needs OS/2 APIs) ---*/
void getcountryspecs(int codepage, int country) { }

/*--- DOS stubs for OS/2-only functions ---*/
void installhandlers(void) { }
void uninstallhandlers(void) { }
void checkmultitaskers(void) { }

extern "C" {
    void giveup(void) { }
    void mydelay(int hundredths) { }
    long settimer(int which, long ticks) { return 0; }
    long gettimer(int which) { return 0; }
    int readcheck(int handle, void *buf, unsigned len) {
        extern int read(int, void *, unsigned);
        return read(handle, buf, len);
    }
}

/*--- Country struct ---*/
#include "country.h"
countrytype Country;

/* setdelay — used by OFFLINE utility */
void setdelay(void) { }

/* PCBDIAG stubs */
int cputype(void) { return 5; } /* report Pentium */
long Scrn_Addr = 0xB8000L;
char ShareStatus = 0;
int _atexitcnt = 0;
void (*_atexittbl[32])(void) = {0};





/* PcbData — PCBoard configuration */


/* PcbData — pcbdattype instance (sized to match pcbtools.h struct) */
/* Cannot include pcbtools.h due to typedef conflicts with country.h */
struct _pcbdat_stub { char data[8192]; };
typedef struct _pcbdat_stub pcbdattype;
pcbdattype PcbData;
