#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "users.h"
#include "account.h"
#include "pcboard.h"
#include "screen.h"

/*=========================================================================*/
/* PCBMODEM stubs — satisfies dependency chain for PCBMODEM_W.EXE         */
/* 134 → 5 unresolved before this file; this resolves the rest.            */
/*=========================================================================*/

/* Direct stubs */

extern "C" { void fmemcpy(void *d, void *s, unsigned n) { memcpy(d, s, n); } }

/* Bool flags */
bool AccountSupport=0, AddressSupport=0, AliasSupport=0, AutoGoodBye=0;
bool NotesSupport=0, PasswordSupport=0, PersonalSupport=0, QwkSupport=0;
bool StatsSupport=0, VerifySupport=0, BankSupport=0;

/* Data globals */
char *ConfFlags=0, *ConfReg=0, *TempReg=0;
UData UsersData; UData *TempData=0;
URead UsersRead; URead *TempRead=0;
personal_psa_t Personal;
timebank_psa_t TimeBank;
accountratetype AccountRates;
hdrtype Hdr;
rectype UsersRec, TempInfRec;
passwordtype PwrdRec;
addresstype AddrRec;
qwkconfigtype QwkConfig;
long PersonalOffset=0, BankOffset=0;

/* Scalars */
char ReadAlias[26]={0}, YN[3]="YN";
int Update=0, UsersFile=-1, UsersInfFile=-1, scrCnt=0;
unsigned int Date=0, CurrentCountry=0, CodePage=0;
void (*turnonxmit)(void) = 0;
int (*comminkey)(void) = 0;
taskertype Tasker = NMT;
class cSCRIPT; cSCRIPT *scriptPtr = 0;

/* cfgtype — PCBMODEM internal struct (defined in MDMCMDS.CPP) */
typedef struct { char Modems[40],PCBDat[40],History[40],Name[15],Version[10]; } cfgtype;
cfgtype Config;

/* User management functions */
void setbankdefaults(void){} void init_uppercase(void){}
void restoretextinf(void){} void readtextinf(void){}

/* Modem/display/script stubs */
 
void setfont(fonttype f){} void reinstallhandlers(void){}
int doScript(char*a,char*b,int c){return 0;}

/* C-linkage stubs */
extern "C" {
    short bgetkey2(char opt) { return 0; }
    void scrollon(void) {}
    void execl(void) {}
    void swapenv(void) {}
}

/* C++ linkage (declared in pcboard.h as LIBENTRY, not extern "C") */
void begnoscroll(void) {}
void endnoscroll(void) {}

/* Stubs for functions whose real source failed to compile */
bool lastChar(char *s, char c) { return s && *s && s[strlen(s)-1] == c; }



/* Modem functions — stubs for DOS4G (real code needs ASYNC driver) */
int readaccountrates(void) { return 0; }

/* Modem functions — stubs (COMM path needs OS/2 APIs) */

/* PPL runtime stubs — PCBMODEM doesn't execute PPEs */
bool parsersearch(char*s,int a,char*d,bool b,int c){return 0;}
int tokenscan(char*s,char*d,bool b){return 0;}
void dispString(char*s,int n){}
void stopsearch(void){}
void cleanupScript(void){}

/* pcbstrdup — PCBoard's strdup wrapper */
char *pcbstrdup(char *s) { return strdup(s); }

/* getrows — detect text mode rows via BIOS INT 10h */
#include <dos.h>
char getrows(void) {
    union REGS r;
    r.w.ax = 0x1130;  /* Get font info */
    r.h.bh = 0;
    int386(0x10, &r, &r);
    if (r.h.dl == 0) return 25;  /* fallback */
    return (char)(r.h.dl + 1);
}

/*=========================================================================*/
/* Real modem functions backed by ASYNC.C (FOSSIL driver)                  */

/* These replace MODEM.C's closemodem/sendbyte/openmodem which need OS/2   */
/*=========================================================================*/


extern "C" {
    void async_closecom(void);
    void async_turnoffdtr(void);
    void async_csendbyte(int);
    void async_opencom(int,int);
}

void closemodem(bool TurnOffDTR) {
    if (TurnOffDTR) async_turnoffdtr();
    async_closecom();
}


void sendbyte(char Byte) {
    async_csendbyte((int)Byte);
}


void openmodem(showtype Show) {
    (void)Show;
    async_opencom(0, 0);
}

