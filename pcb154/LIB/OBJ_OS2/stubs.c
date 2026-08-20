/* Phase 0 stubs — functions from failed LIB compiles */
#ifdef __cplusplus
extern "C" {
#endif

typedef char bool;

/* readcheck — read from file with error handling */
int readcheck(int handle, void *buf, unsigned len)
{
    extern int read(int, void *, unsigned);
    return read(handle, buf, len);
}

/* getextendederror — DOS extended error (INT 21h/59h) */
int ExtendedError = 0;
int Int24Error = 0;
int ExtendedAction = 0;
void getextendederror(void)
{
    ExtendedError = 0;
    Int24Error = 0;
    ExtendedAction = 0;
}

/* Scrn_24Hour — 24-hour time display flag */
char Scrn_24Hour = 1;

/* KbdStatus */
unsigned short *KbdStatus = 0;

/* UpperCase — ASCII uppercase table */
unsigned char UpperCase[256];

#ifdef __cplusplus
}
#endif
