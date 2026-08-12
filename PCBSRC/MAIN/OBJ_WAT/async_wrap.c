/*
 * async_wrap.c — C wrapper to bridge ASYNC.C symbols to C++ callers.
 * Compiled as plain C (wcc386), exports lowercase names with correct
 * C linkage that C++ code can call.
 */

/* Import from ASYNC.obj (uppercase, #pragma aux "*") */
extern void ASYNC_CLOSECOM(void);
extern void ASYNC_TURNOFFDTR(void);
extern void ASYNC_CSENDBYTE(int);
extern void ASYNC_OPENCOM(int, int);
extern int  ASYNC_ONLINE(void);
extern void ASYNC_TURNONXMIT(void);
extern void ASYNC_CLEARINBUF(void);
extern void ASYNC_CLEAROUTBUF(void);
extern void ASYNC_TURNONDTR(void);
extern void ASYNC_TURNONRTS(void);
extern void ASYNC_TURNOFFRTS(void);
extern void ASYNC_TURNONFIFO(int);
extern void ASYNC_CSENDSTR(char*,int);
extern int  ASYNC_CGETSTR(char*,int);
extern int  ASYNC_CDSTILLUP(void);
extern void ASYNC_INIT(int,int,char*,char*,int,int,int,int);

#pragma aux ASYNC_CLOSECOM "*"
#pragma aux ASYNC_TURNOFFDTR "*"
#pragma aux ASYNC_CSENDBYTE "*"
#pragma aux ASYNC_OPENCOM "*"
#pragma aux ASYNC_ONLINE "*"
#pragma aux ASYNC_TURNONXMIT "*"
#pragma aux ASYNC_CLEARINBUF "*"
#pragma aux ASYNC_CLEAROUTBUF "*"
#pragma aux ASYNC_TURNONDTR "*"
#pragma aux ASYNC_TURNONRTS "*"
#pragma aux ASYNC_TURNOFFRTS "*"
#pragma aux ASYNC_TURNONFIFO "*"
#pragma aux ASYNC_CSENDSTR "*"
#pragma aux ASYNC_CGETSTR "*"
#pragma aux ASYNC_CDSTILLUP "*"
#pragma aux ASYNC_INIT "*"

/* Export lowercase wrappers (C linkage, wcc386 adds trailing _) */
void async_closecom(void)       { ASYNC_CLOSECOM(); }
void async_turnoffdtr(void)     { ASYNC_TURNOFFDTR(); }
void async_csendbyte(int c)     { ASYNC_CSENDBYTE(c); }
void async_opencom(int a,int b) { ASYNC_OPENCOM(a,b); }
int  async_online(void)         { return ASYNC_ONLINE(); }
