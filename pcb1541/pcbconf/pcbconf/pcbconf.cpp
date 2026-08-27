// ============================================================================
//  pcbconf.cpp
//  Purpose: imports a FidoNet BACKBONE.NA anf FILEBONE.NA echo-tag list into 
//  the PCB BBs conference database, starting at a given conference number.
// ============================================================================

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>      // for errno, used in the file-open error path

#if _MSC_VER < 1200
#error "This source targets Microsoft Visual C++ 6.0 (_MSC_VER 1200) or later; the WinServer SDK headers referenced below may not match an older toolchain."
#endif

extern "C" {
    // wcsrv.dll
    __declspec(dllimport) long __stdcall WildcatServerConnect(const char *pszServer, void *pReserved);
    __declspec(dllimport) void*__stdcall WildcatServerCreateContext(long hConnection);
    __declspec(dllimport) long __stdcall WildcatServerDeleteContext(void *hContext);
    __declspec(dllimport) long __stdcall LogoutUser(void *hContext);

    // wcsmw.dll  (MW = "member" / config API, used by wcCONFIG-class tools)
    __declspec(dllimport) long __stdcall MwLogin(void *hContext, const char *pszUserName, const char *pszPassword);
    __declspec(dllimport) long __stdcall MwUpdateConfDesc(void *hContext, long confNum, const char *pszEchoTag, const char *pszDescription, long flags1, long flags2);
}

// ----------------------------------------------------------------------
// Globals (mirrors the fixed data addresses referenced throughout .text:
// 0x4035c8 g_startConf, 0x4035cc g_naFlag, 0x4035d0 g_descFlag,
// 0x4034c0 g_pathBuf). Initialized by InitGlobals(), at VA 0x4011d0.
// ----------------------------------------------------------------------
static long g_startConf = -1;      // /CONF= starting conference number
static BOOL g_naFlag    = FALSE;   // /NA switch seen
static BOOL g_descFlag  = FALSE;   // /DESC switch seen
static char g_pathBuf[1024] = "";  // /FILE= path argument

// ----------------------------------------------------------------------
// PrintBanner()  — VA 0x4010c0
// ----------------------------------------------------------------------
static void PrintBanner()
{
    printf("\n");
    printf(" A Wildcat! FTN Fido-Utility\n");
    printf("wcFido - Copyright(C) 2003 by TKD Software, Inc.  All Rights Reserved.\n");
    printf("\n");
}

// ----------------------------------------------------------------------
// PrintUsage(argv0)  — VA 0x4010f0
// Strips the path from argv[0], leaving just the executable name, and
// prints the multi-line usage block.
// ----------------------------------------------------------------------
static void PrintUsage(const char *argv0)
{
    const char *slash = strrchr(argv0, '\\');
    const char *exeName = slash ? slash + 1 : argv0;

    // strip a trailing ".exe" if present, matching the ".exe" string
    // constant referenced near this routine
    char nameBuf[260];
    strncpy(nameBuf, exeName, sizeof(nameBuf) - 1);
    nameBuf[sizeof(nameBuf) - 1] = '\0';
    size_t len = strlen(nameBuf);
    if (len > 4 && _stricmp(nameBuf + len - 4, ".exe") == 0)
        nameBuf[len - 4] = '\0';

    printf("%s /NA /FILE=[path/file of backbone.na] /CONF=[start conf] [/DESC]\n", nameBuf);
    printf("- To import a BACKBONE.NA into the Wildcat! conference database, use\n");
    printf("  the following command:\n");
    printf("\n");
    printf("  /DESC - Requests the conference name to be the description field of the\n");
    printf("          BACKBONE.NA file and not the echo tag name which is the default.\n");
}

// ----------------------------------------------------------------------
// InitGlobals()  — VA 0x4011d0
// ----------------------------------------------------------------------
static void InitGlobals()
{
    g_startConf = -1;
    g_descFlag  = FALSE;
    g_naFlag    = FALSE;
    g_pathBuf[0] = '\0';
}

// ----------------------------------------------------------------------
// ParseArgs(argc, argv)  — VA 0x4011f0
// Walks argv[], recognizing (case-insensitively, per the widened-char
// compare in the binary) the /NA, /DESC, /CONF=, /FILE= switches.
// ----------------------------------------------------------------------
static void ParseArgs(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++)
    {
        const char *arg = argv[i];

        if (_strnicmp(arg, "/NA", 3) == 0)
        {
            g_naFlag = TRUE;
        }
        else if (_strnicmp(arg, "/desc", 5) == 0)
        {
            g_descFlag = TRUE;
        }
        else if (_strnicmp(arg, "/conf=", 6) == 0)
        {
            g_startConf = atol(arg + 6);
        }
        else if (_strnicmp(arg, "/file=", 6) == 0)
        {
            strncpy(g_pathBuf, arg + 6, sizeof(g_pathBuf) - 1);
            g_pathBuf[sizeof(g_pathBuf) - 1] = '\0';
        }
    }
}

// ----------------------------------------------------------------------
// ReadLine()  — VA 0x401980/0x4019c0 (CStdioFile::ReadString-equivalent)
// Reads one CRLF/LF/CR-terminated line from an already-opened file.
// Returns FALSE at end of file.
// ----------------------------------------------------------------------
static BOOL ReadLine(FILE *fp, char *buf, size_t bufSize)
{
    if (!fgets(buf, (int)bufSize, fp))
        return FALSE;

    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\r' || buf[len - 1] == '\n'))
        buf[--len] = '\0';

    return TRUE;
}

// ----------------------------------------------------------------------
// CountLines()  — VA 0x401a40
// Pre-scans the file to report "Total Echos Detected", then rewinds.
// ----------------------------------------------------------------------
static long CountLines(FILE *fp)
{
    long count = 0;
    char scratch[1024];
    long savedPos = ftell(fp);

    while (ReadLine(fp, scratch, sizeof(scratch)))
        count++;

    fseek(fp, savedPos, SEEK_SET);
    return count;
}

// ----------------------------------------------------------------------
// ParseBackboneLine()
// A BACKBONE.NA record line is tag-separated ("EchoTag Description...");
// splits into echo tag (first token) and description (remainder).
// NOTE: BACKBONE.NA's real-world format uses whitespace-delimited
// columns; this reproduces that with strtok, matching the `strtok`
// import pulled in by the binary.
// ----------------------------------------------------------------------
static void ParseBackboneLine(char *line, char *tagOut, size_t tagSize,
                               char *descOut, size_t descSize)
{
    char *tok = strtok(line, " \t");
    tagOut[0] = '\0';
    descOut[0] = '\0';

    if (tok)
    {
        strncpy(tagOut, tok, tagSize - 1);
        tagOut[tagSize - 1] = '\0';

        char *rest = strtok(NULL, "");
        if (rest)
        {
            while (*rest == ' ' || *rest == '\t')
                rest++;
            strncpy(descOut, rest, descSize - 1);
            descOut[descSize - 1] = '\0';
        }
    }
}

// ----------------------------------------------------------------------
// GetWcConfigPassword()  — VA around 0x4013a9 / uses `gets`
// The binary calls the CRT `gets()` import directly here (no echo
// suppression visible in the disassembly — the compiled tool really
// does take the password as plain visible console input).
// ----------------------------------------------------------------------
static void GetWcConfigPassword(char *buf, size_t bufSize)
{
    printf("+ Please enter your WCCONFIG password [press ENTER if none]: ");
    fflush(stdout);
    if (!fgets(buf, (int)bufSize, stdin))
    {
        buf[0] = '\0';
        return;
    }
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\r' || buf[len - 1] == '\n'))
        buf[--len] = '\0';
}

// ----------------------------------------------------------------------
// RunImport()  — VA 0x40130f, the heart of the program.
// ----------------------------------------------------------------------
static int RunImport()
{
    long hConn = WildcatServerConnect(NULL, NULL);   // NULL = local server
    if (hConn == 0)
    {
        long err = GetLastError();
        printf("! Error #%x while trying to connect to Wildcat! Server.\n", err);
        return 1;
    }

    void *hCtx = WildcatServerCreateContext(hConn);
    if (hCtx == NULL)
    {
        long err = GetLastError();
        printf("! Error #%x while trying to create Wildcat! Context.\n", err);
        return 1;
    }

    printf("? Attempting WCCONFIG access, please wait...\n");

    char password[256];
    GetWcConfigPassword(password, sizeof(password));

    long loginResult = MwLogin(hCtx, "WCCONFIG", password);
    if (loginResult != 0)
    {
        if (loginResult == 0x20000005)
        {
            printf("! You may already have wcCONFIG open.  Please close wcCONFIG and try again.\n");
        }
        else if (loginResult == 0x20000017)
        {
            printf("! You entered an incorrect WCCONFIG password.  Please try again.\n");
        }
        else
        {
            printf("! Error #%x while trying to login as WCCONFIG.\n", loginResult);
        }
        WildcatServerDeleteContext(hCtx);
        return 1;
    }

    // --- open BACKBONE.NA ---
    FILE *fp = fopen(g_pathBuf, "r");
    if (!fp)
    {
        printf("! Error #%d while trying to open %s\n", errno, g_pathBuf);
        LogoutUser(hCtx);
        WildcatServerDeleteContext(hCtx);
        return 1;
    }

    long totalEchos = CountLines(fp);
    printf("? Total Echos Detected: %d\n", totalEchos);

    printf("+ Parsing %s file, please wait...\n", g_pathBuf);

    long confNum = g_startConf;
    long imported = 0;
    char line[1024];

    while (ReadLine(fp, line, sizeof(line)))
    {
        char tag[128], desc[512];
        ParseBackboneLine(line, tag, sizeof(tag), desc, sizeof(desc));

        if (tag[0] == '\0')
            continue;   // blank/comment line

        const char *confName = g_descFlag ? desc : tag;

        long result = MwUpdateConfDesc(hCtx, confNum, tag, confName, /*flags1*/1, /*flags2*/1);
        if (result != 0)
        {
            printf("! Could not add %s [conf #%d]\n", confName, confNum);
        }
        else
        {
            printf("+ Added %s [conf #%d]\n", confName, confNum);
            imported++;
        }

        confNum++;
    }

    fclose(fp);
    printf("% Imported %d conference records.\n", imported);

    LogoutUser(hCtx);
    WildcatServerDeleteContext(hCtx);
    return 0;
}

// ----------------------------------------------------------------------
// main()  — VA 0x401735 (the app's real entry, called from the CRT
// startup thunk at 0x401b30 via the MFC-init trampoline at 0x4016f0)
// ----------------------------------------------------------------------
int main(int argc, char *argv[])
{
    PrintBanner();

    if (argc <= 1)
    {
        PrintUsage(argv[0]);
        return 1;
    }

    InitGlobals();
    ParseArgs(argc - 1, argv + 1);

    if (g_naFlag && g_pathBuf[0] != '\0' && g_startConf != -1)
    {
        return RunImport();
    }

    PrintUsage(argv[0]);
    return 1;
}
