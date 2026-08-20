/* C++ stubs for PCBOARD2.EXE OS/2 link — matches mangled names */
/* GPL v3.0 — pcbrevival project by hexadecimal */
#include <stdlib.h>
#include <string.h>

/* Screen globals (from GETMODE.C which has inline ASM) */
void *Scrn_Buf = 0;
char Scrn_Mode = 3;
char Scrn_BottomRow = 24;
char Scrn_24Hour = 0;
char Scrn_DateSeparator = '/';
char Scrn_Rtrc = 0;
char Scrn_Box = 0;
char Scrn_X = 0;
char Scrn_Y = 0;
short Scrn_Size16 = 4000;
int Scrn_Size32 = 4000;
int Scrn_SizeBytes = 4000;
int Scrn_Adapter = 0;

/* Screen functions */
void getmode(void) {}
void hidescreen(void) {}
void unhidescreen(void) {}
void updatelines(int a, int b, int c) { (void)a;(void)b;(void)c; }
void updatelinesnow(void) {}
void setscreenupdateinterval(int n) { (void)n; }
int getviolines(void) { return 25; }
void setviolines(int n) { (void)n; }

/* Keyboard */
unsigned short bgetkey(unsigned char wait) { (void)wait; return 0; }
unsigned short getkbdstatus(void) { return 0; }
void setkbdstatus(unsigned short s) { (void)s; }

/* Usernet (C++ mangled) */
bool InChat = false;
bool PerformValidation = false;
int UserNetFile = -1;
int NumExitKeys = 0;
int ExitKeyFlag[10] = {0};
char ExitKeyNum[10] = {0};
char ALLCONF[8] = "ALL";
char ALLFILE[8] = "ALL";

void readusernetrecord(unsigned r, void *b) { (void)r;(void)b; }
void updateusernetrecord(unsigned r, void *b) { (void)r;(void)b; }

/* Menu (from SCRNIO MENU.C) */
typedef struct { int dummy[16]; } MenuType;
MenuType Menu[64];
void (*MenuFunc[64])(void);
