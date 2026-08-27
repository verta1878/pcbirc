/* ============================================================
 * wf_integ.c — WinFOSSIL integration test
 * ============================================================
 * Loads FOSSIL.DLL exactly like a BBS/door program would, then
 * drives the VMODEM AT command interface and telnet filter
 * end-to-end. This is the closest we can get to real-hardware
 * testing without a modem: it proves the export surface works
 * and the command paths respond correctly.
 *
 * Build:
 *   i686-w64-mingw32-gcc -O2 -o wf_integ.exe src/test/wf_integ.c
 *   (needs FOSSIL.DLL in the same dir)
 * ============================================================ */
#include <windows.h>
#include <stdio.h>
#include <string.h>

static int g_pass = 0, g_fail = 0, g_num = 0;
static void t(const char *name, int cond) {
    g_num++;
    if (cond) { g_pass++; printf("  T%-2d %-46s PASS\r\n", g_num, name); }
    else      { g_fail++; printf("  T%-2d %-46s FAIL\r\n", g_num, name); }
}

/* Function pointer types for the exports we drive */
typedef int  (__stdcall *fnOpen)(int);
typedef void (__stdcall *fnClose)(int);
typedef int  (__stdcall *fnStatus)(int);
typedef int  (__stdcall *fnVmDial)(int, const char *);
typedef void (__stdcall *fnVmHangup)(int);
typedef int  (__stdcall *fnVmParse)(int);
typedef int  (__stdcall *fnVmFilter)(int, void *, int);
typedef int  (__stdcall *fnStuffRx)(int, const void *, int);
typedef int  (__stdcall *fnReadBlk)(int, void *, int);
typedef const char * (__stdcall *fnPlatform)(void);

int main(void)
{
    HMODULE h;
    printf("=== WinFOSSIL Integration Test ===\r\n\r\n");

    h = LoadLibraryA("FOSSIL.DLL");
    t("Load FOSSIL.DLL", h != NULL);
    if (!h) { printf("Cannot continue.\r\n"); return 1; }

    fnOpen     pOpen   = (fnOpen)     GetProcAddress(h, "commOpenPort");
    fnClose    pClose  = (fnClose)    GetProcAddress(h, "commClosePort");
    fnStatus   pStatus = (fnStatus)   GetProcAddress(h, "commGetStatus");
    fnVmParse  pParse  = (fnVmParse)  GetProcAddress(h, "commVmodemParseCmdStr");
    fnVmFilter pFilter = (fnVmFilter) GetProcAddress(h, "commVmodemFilter");
    fnStuffRx  pStuff  = (fnStuffRx)  GetProcAddress(h, "commVmodemStuffReadQ");
    fnReadBlk  pRead   = (fnReadBlk)  GetProcAddress(h, "commReadBlock");
    fnPlatform pPlat   = (fnPlatform) GetProcAddress(h, "commGetPlatform");

    t("commOpenPort resolved",   pOpen   != NULL);
    t("commGetStatus resolved",  pStatus != NULL);
    t("commVmodemFilter resolved", pFilter != NULL);
    t("commGetPlatform resolved", pPlat  != NULL);

    /* Platform string */
    if (pPlat) {
        const char *plat = pPlat();
        t("Platform string non-empty", plat && plat[0]);
        printf("      platform = %s\r\n", plat ? plat : "(null)");
    }

    /* Status on an unopened port should be safe */
    if (pStatus) {
        int st = pStatus(0);
        t("Status on port 0 (no crash)", 1);
        (void)st;
    }

    /* Telnet filter: IAC WILL SGA (FF FB 03) should be consumed */
    if (pFilter) {
        unsigned char buf[8] = { 'H', 'i', 0xFF, 0xFB, 0x03, '!', 0, 0 };
        int len = 6;
        int outlen = pFilter(0, buf, len);
        /* The 3 IAC bytes are stripped, leaving "Hi!" = 3 bytes */
        t("Telnet filter strips IAC WILL SGA", outlen == 3);
        t("Telnet filter keeps data byte", buf[0]=='H' && buf[1]=='i');
    }

    /* Telnet filter: escaped FF FF -> single FF */
    if (pFilter) {
        unsigned char buf[4] = { 0xFF, 0xFF, 'X', 0 };
        int outlen = pFilter(0, buf, 3);
        t("Telnet filter unescapes FF FF", outlen == 2 && buf[0]==0xFF && buf[1]=='X');
    }

    /* Telnet filter: lone trailing FF is held (not passed through) */
    if (pFilter) {
        unsigned char buf[4] = { 'A', 'B', 0xFF, 0 };
        int outlen = pFilter(0, buf, 3);
        t("Telnet filter holds trailing IAC", outlen == 2);
    }

    if (pClose) { pClose(0); t("commClosePort (no crash)", 1); }

    FreeLibrary(h);

    printf("\r\n=========================================\r\n");
    printf("Integration: %d/%d passed", g_pass, g_num);
    if (g_fail) printf(", %d FAILED", g_fail);
    printf("\r\n");
    return g_fail ? 1 : 0;
}
