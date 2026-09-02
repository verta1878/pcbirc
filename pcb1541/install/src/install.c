/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* install.c -- PCBoard Installer (Phase 27 Reproduction)                   */
/*                                                                           */
/* Exact reproduction of Clark's INSTALL.EXE (331K, Aug 31 1996).            */
/* Script-driven installer that reads INSTALL.DAT and processes              */
/* @Command directives to install PCBoard from distribution disks.           */
/*                                                                           */
/* Original source: W:/master/install/main.c (from string scan)              */
/*                                                                           */
/* This file implements the ~40 @ commands actually used by PCBoard's        */
/* INSTALL.DAT. The full INSTALL.EXE supports 250+ commands but most         */
/* are for other Clark products.                                              */
/*                                                                           */
/* DEPENDENCY: .RED container extraction/creation                            */
/*   The @BeginLib/@File/@EndLib directives unpack files from Clark's        */
/*   .RED containers (RR magic, LH5-family compression, per-record header    */
/*   with compressed/uncompressed sizes and CRC16). Once pcb1541/install/archivers/redx      */
/*   and pcb1541/install/archivers/redc land (built on pcb1541/install/archivers/lha/), this installer       */
/*   links against them instead of shelling out. See pcb1541/install/archivers/README.md     */
/*   for the codec phase plan.                                                */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef __WATCOMC__
#include <conio.h>
#include <dos.h>
#include <direct.h>
#include <sys/stat.h>
#define PATH_SEP '\\'
#define strcasecmp  stricmp
#define strncasecmp strnicmp
#include <stdint.h>
#elif defined(_WIN32)
#include <windows.h>
#include <direct.h>
#include <conio.h>
#include <sys/stat.h>
#define PATH_SEP '\\'
#define strcasecmp  _stricmp
#define strncasecmp _strnicmp
#include <stdint.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdint.h>
#include <strings.h>
#define PATH_SEP '/'
#endif

#define MAX_VARS      64                /* max script variables           */
#define MAX_GROUPS    16                /* max install groups             */
#define MAX_LINE     512                /* max script line length         */
#define MAX_PATH_LEN 260                /* max file path                 */
#define MAX_LABELS    64                /* max @Goto labels               */


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                        .RED Archive Format                                */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#define RED_SIGNATURE  0x5252           /* "RR" little-endian             */
#define RED_VERSION    0x01             /* archive version                */

#pragma pack(push, 1)
typedef struct {
    uint16_t Signature;                 /* 0x5252 "RR"                   */
    uint8_t  Version;                   /* 0x01                          */
    uint32_t CrcOrSize;                 /* CRC or compressed total       */
    uint32_t Field2;                    /* uncompressed size?             */
    uint32_t Field3;                    /* data offset?                  */
    uint16_t Marker;                    /* 0xFFFF                        */
    uint32_t Field4;                    /* unknown                       */
    uint16_t FileCount;                 /* number of files               */
    uint16_t NameLen;                   /* length of first filename      */
} RedHeader;
#pragma pack(pop)


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      Script Variables                                     */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef struct {
    char Name[32];                      /* variable name                 */
    char Value[MAX_PATH_LEN];           /* variable value                */
} ScriptVar;

typedef struct {
    char Name[64];                      /* group name                    */
    int  Selected;                      /* selected by user              */
} InstGroup;

typedef struct {
    char Name[32];                      /* label name                    */
    long FilePos;                       /* position in script file       */
} ScriptLabel;

typedef struct {
    /* Project info */
    char     ProjName[64];              /* @Name value                   */
    char     ProjVersion[16];           /* @Version value                */
    char     SubDir[MAX_PATH_LEN];      /* install subdirectory          */
    char     OutDrive;                  /* output drive letter           */
    char     InDrive;                   /* input drive letter            */

    /* Variables */
    ScriptVar Vars[MAX_VARS];           /* user-defined variables        */
    int       NumVars;                  /* variable count                */

    /* Groups */
    InstGroup Groups[MAX_GROUPS];       /* install option groups         */
    int       NumGroups;                /* group count                   */

    /* Labels */
    ScriptLabel Labels[MAX_LABELS];     /* @Goto targets                 */
    int         NumLabels;              /* label count                   */

    /* State */
    FILE    *ScriptFp;                  /* INSTALL.DAT file handle       */
    int      UseCheckBox;               /* checkbox mode for groups      */
    int      AskOverwrite;              /* ask before overwriting files   */
    int      Aborted;                   /* user pressed ESC              */
    char     CurrentDisk[32];           /* current disk label            */
} InstState;


/*-----------------------------------------------------------------------*/
/* inst_set_var() -- Set or create a script variable                     */
/*-----------------------------------------------------------------------*/

static void inst_set_var(InstState *St, const char *Name, const char *Value)
{
    int i;                              /* search index                  */

    for (i = 0; i < St->NumVars; i++) {
        if (strcasecmp(St->Vars[i].Name, Name) == 0) {
            strncpy(St->Vars[i].Value, Value, MAX_PATH_LEN - 1);
            return;
        }
    }
    if (St->NumVars < MAX_VARS) {
        strncpy(St->Vars[St->NumVars].Name, Name, 31);
        strncpy(St->Vars[St->NumVars].Value, Value, MAX_PATH_LEN - 1);
        St->NumVars++;
    }
}


/*-----------------------------------------------------------------------*/
/* inst_get_var() -- Get a script variable value                        */
/*-----------------------------------------------------------------------*/

static const char *inst_get_var(InstState *St, const char *Name)
{
    int i;                              /* search index                  */

    /* Built-in variables */
    if (strcasecmp(Name, "Name") == 0)     return St->ProjName;
    if (strcasecmp(Name, "Version") == 0)  return St->ProjVersion;
    if (strcasecmp(Name, "SubDir") == 0)   return St->SubDir;
    if (strcasecmp(Name, "OutDrive") == 0) {
        static char Drv[4];
        Drv[0] = St->OutDrive; Drv[1] = '\0';
        return Drv;
    }

    for (i = 0; i < St->NumVars; i++) {
        if (strcasecmp(St->Vars[i].Name, Name) == 0)
            return St->Vars[i].Value;
    }
    return "";
}


/*-----------------------------------------------------------------------*/
/* inst_expand() -- Expand @Variable references in a string             */
/*                                                                       */
/* Replaces @Name, @Version, @SubDir, @OutDrive, and user variables.    */
/*-----------------------------------------------------------------------*/

static void inst_expand(InstState *St, const char *Src, char *Dst, int DstSize)
{
    const char *p = Src;                /* source scan pointer           */
    int         d = 0;                  /* destination position          */

    while (*p && d < DstSize - 1) {
        if (*p == '@') {
            /* Extract variable name */
            char VarName[64];           /* variable name buffer          */
            int  v = 0;                 /* name position                 */
            const char *Val;            /* resolved value                */

            p++;
            while (*p && (isalnum(*p) || *p == '_') && v < 63)
                VarName[v++] = *p++;
            VarName[v] = '\0';

            Val = inst_get_var(St, VarName);
            while (*Val && d < DstSize - 1)
                Dst[d++] = *Val++;
        } else {
            Dst[d++] = *p++;
        }
    }
    Dst[d] = '\0';
}


/*-----------------------------------------------------------------------*/
/* inst_group_selected() -- Check if a group is selected                */
/*-----------------------------------------------------------------------*/

static int inst_group_selected(InstState *St, const char *VarName)
{
    int i;                              /* search index                  */

    /* Groups are stored as single-letter variables (a-z) */
    for (i = 0; i < St->NumGroups; i++) {
        if (strcasecmp(St->Groups[i].Name, VarName) == 0)
            return St->Groups[i].Selected;
    }
    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      .RED Decompression Stub                              */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* TODO: Reverse engineer exact decompression algorithm from INSTALL.EXE.
 * For now, stub that reads the .RED header and reports file entries.
 * The actual decompression needs to be determined from disassembly. */


/*-----------------------------------------------------------------------*/
/* red_open() -- Open and validate a .RED archive                       */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

static int red_open(const char *Path, FILE **Fp)
{
    RedHeader Hdr;                      /* archive header                */

    *Fp = fopen(Path, "rb");
    if (!*Fp) {
        printf(" ERROR: Cannot open %s\n", Path);
        return -1;
    }

    if (fread(&Hdr, sizeof(Hdr), 1, *Fp) != 1) {
        printf(" ERROR: Cannot read header from %s\n", Path);
        fclose(*Fp);
        return -1;
    }

    if (Hdr.Signature != RED_SIGNATURE || Hdr.Version != RED_VERSION) {
        printf(" ERROR: Invalid .RED signature in %s\n", Path);
        fclose(*Fp);
        return -1;
    }

    return 0;
}


/*-----------------------------------------------------------------------*/
/* red_extract_file() -- Extract a named file from a .RED archive       */
/*                                                                       */
/* TODO: Implement actual decompression. Currently a stub.               */
/*-----------------------------------------------------------------------*/

static int red_extract_file(FILE *RedFp, const char *FileName,
                             const char *OutPath)
{
    /* STUB — needs decompression algorithm from disassembly */
    printf("  Extracting %s -> %s\n", FileName, OutPath);
    (void)RedFp;
    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                     Script Command Dispatcher                             */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* inst_read_block() -- Read a multi-line block until @End<token>        */
/*-----------------------------------------------------------------------*/

static void inst_read_block(InstState *St, const char *EndToken,
                             char *Buf, int BufSize)
{
    char Line[MAX_LINE];                /* line read buffer              */
    int  Pos = 0;                       /* buffer write position         */

    Buf[0] = '\0';
    while (fgets(Line, sizeof(Line), St->ScriptFp)) {
        char *p = Line;                 /* line scan pointer             */

        while (*p == ' ' || *p == '\t') p++;

        /* Check for end token */
        if (strncasecmp(p, EndToken, strlen(EndToken)) == 0)
            break;

        /* Append to buffer */
        {
            int Len = (int)strlen(p);
            if (Pos + Len < BufSize - 1) {
                memcpy(Buf + Pos, p, Len);
                Pos += Len;
            }
        }
    }
    Buf[Pos] = '\0';
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_display() -- Handle @Display ... @EndDisplay block          */
/*-----------------------------------------------------------------------*/

static void inst_cmd_display(InstState *St)
{
    char Block[4096];                   /* display text buffer           */
    char Expanded[4096];                /* after variable expansion      */

    inst_read_block(St, "@EndDisplay", Block, sizeof(Block));
    inst_expand(St, Block, Expanded, sizeof(Expanded));

    printf("%s", Expanded);
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_define_project() -- Handle @DefineProject block             */
/*-----------------------------------------------------------------------*/

static void inst_cmd_define_project(InstState *St)
{
    char Line[MAX_LINE];                /* line read buffer              */

    while (fgets(Line, sizeof(Line), St->ScriptFp)) {
        char *p = Line;                 /* line scan pointer             */
        char  Key[64];                  /* parsed key                    */
        char  Val[256];                 /* parsed value                  */

        while (*p == ' ' || *p == '\t') p++;
        if (strncasecmp(p, "@EndProject", 11) == 0) break;

        if (*p == '@') p++;
        if (sscanf(p, "%63s = %255[^\r\n]", Key, Val) == 2) {
            /* Strip quotes */
            if (Val[0] == '"') {
                char *End;              /* closing quote pointer         */
                memmove(Val, Val + 1, strlen(Val));
                End = strrchr(Val, '"');
                if (End) *End = '\0';
            }

            if (strcasecmp(Key, "Name") == 0)
                strncpy(St->ProjName, Val, 63);
            else if (strcasecmp(Key, "Version") == 0)
                strncpy(St->ProjVersion, Val, 15);
            else if (strcasecmp(Key, "Subdir") == 0) {
                /* Strip escaped backslashes */
                char *s = Val, *d = St->SubDir;
                while (*s) {
                    if (*s == '\\' && *(s+1) == '\\') s++;
                    *d++ = *s++;
                }
                *d = '\0';
            }
            else if (strcasecmp(Key, "OutDrive") == 0)
                St->OutDrive = Val[0];
        }
    }
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_define_vars() -- Handle @DefineVars block                   */
/*-----------------------------------------------------------------------*/

static void inst_cmd_define_vars(InstState *St)
{
    char Line[MAX_LINE];                /* line read buffer              */

    while (fgets(Line, sizeof(Line), St->ScriptFp)) {
        char *p = Line;                 /* line scan pointer             */
        char  Type[32];                 /* variable type                 */
        char  Name[64];                 /* variable name                 */
        char  Val[256] = "";            /* default value                 */

        while (*p == ' ' || *p == '\t') p++;
        if (strncasecmp(p, "@EndVars", 8) == 0) break;

        if (*p == '@') p++;
        if (sscanf(p, "%31s %63s = %255[^\r\n]", Type, Name, Val) >= 2) {
            if (Name[0] == '@') memmove(Name, Name + 1, strlen(Name));
            /* Strip quotes from value */
            if (Val[0] == '"') {
                char *End;              /* closing quote pointer         */
                memmove(Val, Val + 1, strlen(Val));
                End = strrchr(Val, '"');
                if (End) *End = '\0';
            }
            inst_set_var(St, Name, Val);
        }
    }
}


/*-----------------------------------------------------------------------*/
/* inst_process() -- Main script processing loop                        */
/*-----------------------------------------------------------------------*/

static int inst_process(InstState *St)
{
    char Line[MAX_LINE];                /* line read buffer              */

    while (fgets(Line, sizeof(Line), St->ScriptFp)) {
        char *p = Line;                 /* line scan pointer             */

        while (*p == ' ' || *p == '\t') p++;

        /* Skip empty lines and comments */
        if (*p == '\0' || *p == '\n' || *p == '\r') continue;
        if (p[0] == '/' && p[1] == '/') continue;

        /* Strip trailing whitespace */
        {
            char *End = p + strlen(p) - 1;
            while (End > p && (*End == '\n' || *End == '\r' ||
                               *End == ' '))
                *End-- = '\0';
        }

        /* Dispatch @ commands */
        if (strncasecmp(p, "@DefineProject", 14) == 0)
            inst_cmd_define_project(St);
        else if (strncasecmp(p, "@DefineVars", 11) == 0)
            inst_cmd_define_vars(St);
        else if (strncasecmp(p, "@Display", 8) == 0)
            inst_cmd_display(St);
        else if (strncasecmp(p, "@Cls", 4) == 0)
            printf("\033[2J\033[H");     /* ANSI clear screen             */
        else if (strncasecmp(p, "@Pause", 6) == 0) {
            printf("\n PRESS ANY KEY ");
            fflush(stdout);
#ifdef __WATCOMC__
            getch();
#else
            getchar();
#endif
            printf("\n");
        }
        else if (strncasecmp(p, "@Abort", 6) == 0) {
            printf("\nInstallation aborted.\n");
            return 1;
        }
        else if (strncasecmp(p, "@Exit", 5) == 0) {
            return 0;
        }
        /* TODO: Implement remaining ~35 commands */
    }

    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                             Main Entry                                    */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

int main(int Argc, char *Argv[])
{
    InstState St;                       /* installer state               */
    const char *DatFile = "INSTALL.DAT";/* script file path              */
    int Rc;                             /* process return code           */

    memset(&St, 0, sizeof(St));
    St.OutDrive    = 'C';
    St.InDrive     = 'A';
    St.AskOverwrite = 1;

    /* Allow override of INSTALL.DAT path */
    if (Argc > 1)
        DatFile = Argv[1];

    St.ScriptFp = fopen(DatFile, "r");
    if (!St.ScriptFp) {
        printf("Unable to reopen script file \"INSTALL.DAT\"\n");
        printf("The installation process cannot continue.\n");
        return 1;
    }

    printf("\n PCBoard Installation Program\n\n");

    Rc = inst_process(&St);

    fclose(St.ScriptFp);

    if (Rc == 0)
        printf("\nInstallation complete.\n");

    return Rc;
}
