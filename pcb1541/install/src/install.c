/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* install.c -- PCBoard Installer (install v1.10.4)                          */
/*                                                                            */
/* C reimplementation of Clark's INSTALL.EXE (338,548 B, NE format).          */
/* Reads INSTALL.DAT and processes @-directives to lay down 481 files of      */
/* a PCBoard install from Clark's 8 .RED archives.                            */
/*                                                                            */
/* Original binary strings:                                                   */
/*   $Logfile: W:/master/install/main.c_v $                                   */
/*   INSTALL Ver ... Copyright (c) 1987-1995                                  */
/*                                                                            */
/* Phase progress (60 semantically distinct @-directives total):              */
/*   v1.10.0  --  12  Project metadata + display + basic control              */
/*   v1.10.1  --   6  File operations                                         */
/*   v1.10.2  --  11  Variables + control flow + string ops                   */
/*   v1.10.3  --  10  Filesystem + disk sequencing + @File parser fixes      */
/*   v1.10.4  --  10  Interactive menu                                        */
/*                    (@GetGroups/@CheckBox real; @SetGroup/@ClearGroup       */
/*                     modify SelectedGroups; @AskOverwrite/@Prompt/          */
/*                     @Qstring/@RegCode as accepted directives; real         */
/*                     @GetString reads stdin when TTY else uses stub;        */
/*                     TTY-conditional minimal interactive menu.)             */
/*   v1.10.5      System hooks + finish                                       */
/*   v1.10.6      Disassembly parity check against INSTALL.EXE                */
/*                                                                            */
/* Runtime dep: redx binary (pcb1541/install/archivers/redx) — this           */
/* installer shells out to `redx extract <archive> <destdir>` for each        */
/* @BeginLib block. See RUNTIME-DEPS.md for the full runtime-dependency       */
/* list (matches Clark's original shell-out pattern to DSZ/GSZ etc).          */
/*                                                                            */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

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
#define MAX_PATH_LEN 260                /* max file path                  */
#define MAX_LABELS    64                /* max @Goto labels               */


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                        .RED Archive Format                                */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#define RED_SIGNATURE  0x5252           /* "RR" little-endian             */
#define RED_VERSION    0x01             /* archive version                */

#pragma pack(push, 1)
typedef struct {
    uint16_t Signature;                 /* 0x5252 "RR"                    */
    uint8_t  Version;                   /* 0x01                           */
    uint32_t CrcOrSize;                 /* CRC or compressed total        */
    uint32_t Field2;                    /* uncompressed size?             */
    uint32_t Field3;                    /* data offset?                   */
    uint16_t Marker;                    /* 0xFFFF                         */
    uint32_t Field4;                    /* unknown                        */
    uint16_t FileCount;                 /* number of files                */
    uint16_t NameLen;                   /* length of first filename       */
} RedHeader;
#pragma pack(pop)


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      Script Variables                                     */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef struct {
    char Name[32];                      /* variable name                 */
    char Value[MAX_PATH_LEN];           /* variable value                 */
} ScriptVar;

typedef struct {
    char Name[64];                      /* group name                     */
    int  Selected;                      /* selected by user               */
} InstGroup;

typedef struct {
    char Name[32];                      /* label name                     */
    long FilePos;                       /* position in script file        */
} ScriptLabel;

typedef struct {
    /* Project info */
    char     ProjName[64];              /* @Name value                    */
    char     ProjVersion[16];           /* @Version value                 */
    char     SubDir[MAX_PATH_LEN];      /* install subdirectory           */
    char     OutDrive;                  /* output drive letter            */
    char     InDrive;                   /* input drive letter             */

    /* Variables */
    ScriptVar Vars[MAX_VARS];           /* user-defined variables         */
    int       NumVars;                  /* variable count                 */

    /* Groups */
    InstGroup Groups[MAX_GROUPS];       /* install option groups          */
    int       NumGroups;                /* group count                    */

    /* Labels */
    ScriptLabel Labels[MAX_LABELS];     /* @Goto targets                  */
    int         NumLabels;              /* label count                    */

    /* State */
    FILE    *ScriptFp;                  /* INSTALL.DAT file handle        */
    int      UseCheckBox;               /* checkbox mode for groups       */
    int      AskOverwrite;              /* ask before overwriting files    */
    int      Aborted;                   /* user pressed ESC               */
    char     CurrentDisk[32];           /* current disk label             */

    /* v1.10.1 additions: archive-extraction state */
    char     ArchivesDir[MAX_PATH_LEN]; /* where .RED archives live       */
    char     TargetRoot[MAX_PATH_LEN];  /* where output tree gets built   */
    char     RedxPath[MAX_PATH_LEN];    /* path to redx CLI binary        */
    char     CurrentArchive[64];        /* current @BeginLib archive name */
    char     ExtractDir[MAX_PATH_LEN];  /* temp dir with extracted files  */
    int      InLibBlock;                /* 1 while inside @BeginLib...@EndLib */
    long     FilesPlaced;               /* running count of @File ops      */
    long     FilesFailed;               /* running count of failures       */

    /* v1.10.2 additions: control flow */
    char     SelectedGroups[64];                /* selected group letters, e.g. "ab"  */
    struct {
        int TakenTrue;                  /* @If expression evaluated true       */
        int InElse;                     /* currently inside @Else branch      */
    } IfStack[32];
    int      IfDepth;                   /* @If nesting depth                  */
    long     LabelPos[MAX_LABELS];      /* file position of each @Label       */
    char     LabelName[MAX_LABELS][64]; /* label names for @Goto lookup       */
    int      NumLabelDefs;              /* label-def count                    */

    /* v1.10.3 additions: working-directory state (for @ChDir/@ChDrive) */
    char     WorkingDrive;              /* @ChDrive @OutDrive result          */
    char     WorkingDir[MAX_PATH_LEN];  /* @ChDir "path" result               */

    /* v1.10.4 additions: menu items parsed from @GetGroups blocks */
    struct {
        char Letter;                    /* group letter (a-z)                 */
        char Label[128];                /* display label                      */
        int  IsCheckbox;                /* 1 if multi-select, 0 if radio       */
    } MenuItems[32];
    int      NumMenuItems;              /* accumulated across all @GetGroups   */
    int      InteractiveMenus;          /* 1 if TTY prompted, 0 if headless    */
} InstState;


/*-----------------------------------------------------------------------*/
/* inst_set_var() -- Set or create a script variable                     */
/*-----------------------------------------------------------------------*/

static void inst_set_var(InstState *St, const char *Name, const char *Value)
{
    int i;                              /* search index                   */

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
    int i;                              /* search index                   */

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
/* Also translates escaped backslashes (\\\\ -> \) and normalizes         */
/* Clark's @Foo:@Bar path style.                                         */
/*                                                                       */
/* If a `@Name(` is seen, invokes it as a function call (via the         */
/* expression evaluator) so nested constructs like                        */
/*    "@StrToken(\"@Fname\",0,\" \")"                                    */
/* embedded inside string literals get their function results            */
/* substituted, not just the surface identifier.                         */
/*-----------------------------------------------------------------------*/

/* Forward decls — the string funcs are defined later in the v1.10.2
 * eval section, but inst_expand needs them here. */
static long inst_func_strlen(InstState *St, const char *ArgsRaw);
static void inst_func_strhead(InstState *St, const char *ArgsRaw,
                               char *OutBuf, int OutSize);
static void inst_func_strtoken(InstState *St, const char *ArgsRaw,
                                char *OutBuf, int OutSize);

static void inst_expand(InstState *St, const char *Src, char *Dst, int DstSize)
{
    const char *p = Src;                /* source scan pointer            */
    int         d = 0;                  /* destination position           */

    while (*p && d < DstSize - 1) {
        if (*p == '@') {
            /* Extract identifier */
            char VarName[64];           /* variable name buffer           */
            int  v = 0;                 /* name position                  */
            const char *Val;            /* resolved value                 */

            p++;
            while (*p && (isalnum(*p) || *p == '_') && v < 63)
                VarName[v++] = *p++;
            VarName[v] = '\0';

            if (*p == '(') {
                /* Function call inside a string — invoke it via the
                 * expression evaluator to get the substituted value. */
                extern const char *inst_eval_primary_public(
                    InstState *St, const char *p, void *Out);
                /* We can't forward-call inst_eval_primary yet (defined
                 * later). Inline a minimal handler for the two most
                 * common cases here: @StrToken and @StrHead + @StrLen.
                 * Anything else falls through to variable expansion.
                 *
                 * We handle these by parsing args and calling the
                 * inst_func_* helpers directly. */
                char Args[512];
                int  Depth = 1;
                int  ai = 0;
                p++;
                while (*p && ai < 511) {
                    if (*p == '(') Depth++;
                    else if (*p == ')') { Depth--; if (Depth == 0) break; }
                    Args[ai++] = *p++;
                }
                Args[ai] = '\0';
                if (*p == ')') p++;

                if (strcasecmp(VarName, "StrToken") == 0) {
                    char Buf[512];
                    inst_func_strtoken(St, Args, Buf, sizeof(Buf));
                    { const char *bp = Buf;
                      while (*bp && d < DstSize - 1) Dst[d++] = *bp++; }
                } else if (strcasecmp(VarName, "StrHead") == 0) {
                    char Buf[512];
                    inst_func_strhead(St, Args, Buf, sizeof(Buf));
                    { const char *bp = Buf;
                      while (*bp && d < DstSize - 1) Dst[d++] = *bp++; }
                } else if (strcasecmp(VarName, "StrLen") == 0) {
                    char Buf[32];
                    long L = inst_func_strlen(St, Args);
                    snprintf(Buf, sizeof(Buf), "%ld", L);
                    { const char *bp = Buf;
                      while (*bp && d < DstSize - 1) Dst[d++] = *bp++; }
                }
                /* Other functions inside strings — silently drop */
                continue;
            }

            Val = inst_get_var(St, VarName);
            while (*Val && d < DstSize - 1)
                Dst[d++] = *Val++;
        } else if (*p == '\\' && *(p+1) == '\\') {
            /* Collapse escaped backslash pair to single */
            Dst[d++] = '\\';
            p += 2;
        } else {
            Dst[d++] = *p++;
        }
    }
    Dst[d] = '\0';
}


/*-----------------------------------------------------------------------*/
/* inst_normalize_path() -- Turn DOS-style path into host-native path    */
/*                                                                       */
/* Under Unix hosts, converts "C:\PCB\FOO.EXE" to "<TargetRoot>/FOO.EXE" */
/* (drops the drive letter, converts backslashes to forward slashes,     */
/* strips the top-level SubDir since TargetRoot already includes it).    */
/* Under DOS/Windows hosts, path is used mostly as-is.                    */
/*-----------------------------------------------------------------------*/

static void inst_normalize_path(InstState *St, const char *Src, char *Dst,
                                int DstSize)
{
    const char *p = Src;                /* scan pointer                   */
    int  d = 0;                         /* destination position           */
    char SubDirNoSlash[MAX_PATH_LEN];   /* SubDir without leading slash   */
    int  SubDirLen;                     /* how many chars to strip         */

#if PATH_SEP == '/'
    /* Cross-platform: drop drive letter, strip top-level SubDir */
    if (isalpha(*p) && *(p+1) == ':') p += 2;
    while (*p == '\\' || *p == '/') p++;

    /* If path starts with SubDir (e.g. "PCB\FOO"), strip it — TargetRoot
     * already lands us inside it. */
    strcpy(SubDirNoSlash, St->SubDir);
    {
        char *s = SubDirNoSlash;
        while (*s == '\\' || *s == '/') memmove(s, s+1, strlen(s));
        while (SubDirNoSlash[strlen(SubDirNoSlash)-1] == '\\' ||
               SubDirNoSlash[strlen(SubDirNoSlash)-1] == '/')
            SubDirNoSlash[strlen(SubDirNoSlash)-1] = '\0';
    }
    SubDirLen = (int)strlen(SubDirNoSlash);
    if (SubDirLen > 0 && strncasecmp(p, SubDirNoSlash, SubDirLen) == 0 &&
        (p[SubDirLen] == '\\' || p[SubDirLen] == '/')) {
        p += SubDirLen + 1;
    }

    /* Prepend TargetRoot */
    {
        const char *tr = St->TargetRoot;
        while (*tr && d < DstSize - 1) Dst[d++] = *tr++;
        if (d > 0 && Dst[d-1] != '/') Dst[d++] = '/';
    }
    /* Copy the rest, converting backslash to forward */
    while (*p && d < DstSize - 1) {
        Dst[d++] = (*p == '\\') ? '/' : *p;
        p++;
    }
#else
    /* DOS/Windows host: path is basically fine as-is */
    while (*p && d < DstSize - 1) Dst[d++] = *p++;
#endif
    Dst[d] = '\0';
}


/*-----------------------------------------------------------------------*/
/* inst_mkdir_p() -- Recursive mkdir (like `mkdir -p`)                  */
/*-----------------------------------------------------------------------*/

static void inst_mkdir_p(const char *Path)
{
    char Tmp[MAX_PATH_LEN];             /* working path buffer            */
    char *p;                            /* scan pointer                   */

    strncpy(Tmp, Path, sizeof(Tmp) - 1);
    Tmp[sizeof(Tmp) - 1] = '\0';

    for (p = Tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            char Save = *p;
            *p = '\0';
#ifdef _WIN32
            _mkdir(Tmp);
#elif defined(__WATCOMC__)
            mkdir(Tmp);
#else
            mkdir(Tmp, 0755);
#endif
            *p = Save;
        }
    }
#ifdef _WIN32
    _mkdir(Tmp);
#elif defined(__WATCOMC__)
    mkdir(Tmp);
#else
    mkdir(Tmp, 0755);
#endif
}


/*-----------------------------------------------------------------------*/
/* inst_copy_file() -- Byte-copy Src to Dst, optionally in append mode  */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

static int inst_copy_file(const char *Src, const char *Dst, int Append)
{
    FILE *In  = fopen(Src, "rb");
    FILE *Out;
    char  Buf[8192];                    /* copy buffer                    */
    size_t n;                           /* bytes read this iteration      */

    if (!In) return -1;

    /* Ensure destination dir exists */
    {
        char Dir[MAX_PATH_LEN];         /* dir portion of Dst             */
        char *LastSep;                  /* last separator                 */
        strncpy(Dir, Dst, sizeof(Dir) - 1);
        Dir[sizeof(Dir) - 1] = '\0';
        LastSep = strrchr(Dir, '/');
        if (!LastSep) LastSep = strrchr(Dir, '\\');
        if (LastSep) {
            *LastSep = '\0';
            inst_mkdir_p(Dir);
        }
    }

    Out = fopen(Dst, Append ? "ab" : "wb");
    if (!Out) { fclose(In); return -1; }

    while ((n = fread(Buf, 1, sizeof(Buf), In)) > 0)
        fwrite(Buf, 1, n, Out);

    fclose(In);
    fclose(Out);
    return 0;
}


/*-----------------------------------------------------------------------*/
/* inst_read_block() -- Read lines until a terminator directive          */
/*                                                                       */
/* Concatenates lines into Buf until the terminator (e.g. "@EndDisplay") */
/* is seen. Used for @Display blocks.                                    */
/*-----------------------------------------------------------------------*/

static void inst_read_block(InstState *St, const char *EndTag,
                             char *Buf, int BufSize)
{
    char Line[MAX_LINE];                /* line read buffer               */
    int  TagLen = (int)strlen(EndTag);  /* terminator length              */
    int  Pos = 0;                       /* buffer position                */

    Buf[0] = '\0';

    while (fgets(Line, sizeof(Line), St->ScriptFp)) {
        char *p = Line;                 /* line scan pointer              */

        while (*p == ' ' || *p == '\t') p++;
        if (strncasecmp(p, EndTag, TagLen) == 0)
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
    char Block[4096];                   /* display text buffer            */
    char Expanded[4096];                /* after variable expansion       */

    inst_read_block(St, "@EndDisplay", Block, sizeof(Block));
    inst_expand(St, Block, Expanded, sizeof(Expanded));

    printf("%s", Expanded);
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_define_project() -- Handle @DefineProject block             */
/*-----------------------------------------------------------------------*/

static void inst_cmd_define_project(InstState *St)
{
    char Line[MAX_LINE];                /* line read buffer               */

    while (fgets(Line, sizeof(Line), St->ScriptFp)) {
        char *p = Line;                 /* line scan pointer              */
        char  Key[64];                  /* parsed key                     */
        char  Val[256];                 /* parsed value                   */

        while (*p == ' ' || *p == '\t') p++;
        if (strncasecmp(p, "@EndProject", 11) == 0) break;

        if (*p == '@') p++;
        if (sscanf(p, "%63s = %255[^\r\n]", Key, Val) == 2) {
            /* Strip quotes */
            if (Val[0] == '"') {
                char *End;              /* closing quote pointer          */
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
    char Line[MAX_LINE];                /* line read buffer               */

    while (fgets(Line, sizeof(Line), St->ScriptFp)) {
        char *p = Line;                 /* line scan pointer              */
        char  Type[32];                 /* variable type                  */
        char  Name[64];                 /* variable name                  */
        char  Val[256] = "";            /* default value                  */

        while (*p == ' ' || *p == '\t') p++;
        if (strncasecmp(p, "@EndVars", 8) == 0) break;

        if (*p == '@') p++;
        if (sscanf(p, "%31s %63s = %255[^\r\n]", Type, Name, Val) >= 2) {
            if (Name[0] == '@') memmove(Name, Name + 1, strlen(Name));
            /* Strip quotes from value */
            if (Val[0] == '"') {
                char *End;              /* closing quote pointer          */
                memmove(Val, Val + 1, strlen(Val));
                End = strrchr(Val, '"');
                if (End) *End = '\0';
            }
            inst_set_var(St, Name, Val);
        }
    }
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                    v1.10.1 File Operations                                */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/*-----------------------------------------------------------------------*/
/* inst_make_temp_dir() -- Create a per-archive extraction scratch dir  */
/*-----------------------------------------------------------------------*/

static void inst_make_temp_dir(InstState *St, const char *ArchName)
{
    const char *TmpBase;                /* /tmp or %TEMP%                 */

#ifdef _WIN32
    TmpBase = getenv("TEMP");
    if (!TmpBase) TmpBase = "C:\\TEMP";
#else
    TmpBase = getenv("TMPDIR");
    if (!TmpBase) TmpBase = "/tmp";
#endif

    snprintf(St->ExtractDir, sizeof(St->ExtractDir),
             "%s%cpcbinst.%d.%s",
             TmpBase, PATH_SEP, (int)(long)time(NULL), ArchName);
    inst_mkdir_p(St->ExtractDir);
}


/*-----------------------------------------------------------------------*/
/* inst_remove_temp_dir() -- rm -rf the scratch dir after @EndLib       */
/*-----------------------------------------------------------------------*/

static void inst_remove_temp_dir(InstState *St)
{
    char Cmd[MAX_PATH_LEN + 32];        /* shell command buffer           */

    if (St->ExtractDir[0] == '\0') return;

#ifdef _WIN32
    snprintf(Cmd, sizeof(Cmd), "rmdir /s /q \"%s\" 2>nul", St->ExtractDir);
#else
    snprintf(Cmd, sizeof(Cmd), "rm -rf \"%s\"", St->ExtractDir);
#endif
    system(Cmd);
    St->ExtractDir[0] = '\0';
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_begin_lib() -- Handle @BeginLib <archive>                   */
/*                                                                       */
/* Shells out to redx to extract the archive into a temp dir. All @File */
/* directives until the matching @EndLib pull from this temp dir.        */
/*-----------------------------------------------------------------------*/

static int inst_cmd_begin_lib(InstState *St, const char *ArchName)
{
    char ArchPath[MAX_PATH_LEN];        /* full path to archive           */
    char Cmd[MAX_PATH_LEN * 3];         /* shell command buffer           */
    int  Rc;                            /* system() return code           */

    strncpy(St->CurrentArchive, ArchName, sizeof(St->CurrentArchive) - 1);
    St->InLibBlock = 1;

    /* Locate the archive file */
    snprintf(ArchPath, sizeof(ArchPath), "%s%c%s",
             St->ArchivesDir, PATH_SEP, ArchName);

    inst_make_temp_dir(St, ArchName);

    /* Shell out: cd <extractdir> && redx extract <archive> */
    snprintf(Cmd, sizeof(Cmd),
             "cd \"%s\" && \"%s\" extract \"%s\" > /dev/null 2>&1",
             St->ExtractDir, St->RedxPath, ArchPath);

#ifdef _WIN32
    /* Windows: use "cd /D" for cross-drive, redirect to nul */
    snprintf(Cmd, sizeof(Cmd),
             "cd /D \"%s\" && \"%s\" extract \"%s\" > nul 2>&1",
             St->ExtractDir, St->RedxPath, ArchPath);
#endif

    Rc = system(Cmd);
    if (Rc != 0) {
        printf("  WARNING: redx extract failed for %s (rc=%d)\n",
               ArchName, Rc);
        return -1;
    }

    return 0;
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_end_lib() -- Handle @EndLib                                  */
/*-----------------------------------------------------------------------*/

static void inst_cmd_end_lib(InstState *St)
{
    inst_remove_temp_dir(St);
    St->CurrentArchive[0] = '\0';
    St->InLibBlock = 0;
}


/*-----------------------------------------------------------------------*/
/* inst_find_extracted() -- Locate a source by key in ExtractDir         */
/*                                                                       */
/* Sources can be named (PCBOARD.EXE) or numeric (1, 2, ..., A, B ...).  */
/* Numeric keys map to whatever the archive listed at that position.     */
/* Returns 0 on success, -1 if not found.                                */
/*-----------------------------------------------------------------------*/

static int inst_find_extracted(InstState *St, const char *SrcKey,
                                long ExpectedSize, char *FoundPath,
                                int PathSize)
{
#ifdef _WIN32
    /* Windows: use FindFirstFile — deferred for cross-compile test */
    (void)St; (void)SrcKey; (void)ExpectedSize;
    FoundPath[0] = '\0'; (void)PathSize;
    return -1;
#else
    DIR *D = opendir(St->ExtractDir);
    struct dirent *E;
    struct stat St2;
    char Full[MAX_PATH_LEN];

    if (!D) return -1;

    /* First try: exact name match */
    while ((E = readdir(D))) {
        if (strcasecmp(E->d_name, SrcKey) == 0) {
            snprintf(Full, sizeof(Full), "%s/%s", St->ExtractDir, E->d_name);
            if (stat(Full, &St2) == 0) {
                if (ExpectedSize > 0 && St2.st_size != ExpectedSize) continue;
                strncpy(FoundPath, Full, PathSize - 1);
                FoundPath[PathSize - 1] = '\0';
                closedir(D);
                return 0;
            }
        }
    }
    rewinddir(D);

    /* Second try: exact size match (for numeric-key sources) */
    if (ExpectedSize > 0) {
        while ((E = readdir(D))) {
            if (E->d_name[0] == '.') continue;
            snprintf(Full, sizeof(Full), "%s/%s", St->ExtractDir, E->d_name);
            if (stat(Full, &St2) == 0 && St2.st_size == ExpectedSize) {
                strncpy(FoundPath, Full, PathSize - 1);
                FoundPath[PathSize - 1] = '\0';
                closedir(D);
                return 0;
            }
        }
    }

    closedir(D);
    return -1;
#endif
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_file() -- Handle @File src [@Size N] @Out|@AppendTo dst     */
/*                                                                       */
/* Parses the directive, resolves src in the current ExtractDir,         */
/* verifies @Size if given, then copies (or appends) to dst.             */
/*-----------------------------------------------------------------------*/

static void inst_cmd_file(InstState *St, const char *Line)
{
    char  SrcKey[128];                  /* source key (name or numeric)   */
    char  DstRaw[MAX_PATH_LEN];         /* destination as written         */
    char  DstExpanded[MAX_PATH_LEN];    /* after variable expansion       */
    char  DstNormalized[MAX_PATH_LEN];  /* after path normalization       */
    char  SrcPath[MAX_PATH_LEN];        /* resolved source in ExtractDir  */
    long  ExpectedSize = 0;             /* @Size value, 0 if not given    */
    int   IsAppend = 0;                 /* 1 for @AppendTo, 0 for @Out    */
    const char *p = Line;               /* scan pointer                   */
    int   n;                            /* sscanf arg count               */

    if (!St->InLibBlock) return;        /* @File only valid inside @BeginLib */

    /* Skip past "@File " */
    while (*p == ' ' || *p == '\t') p++;
    if (strncasecmp(p, "@File", 5) == 0) p += 5;
    while (*p == ' ' || *p == '\t') p++;

    /* First token: source key */
    n = 0;
    while (*p && !isspace(*p) && n < 127) SrcKey[n++] = *p++;
    SrcKey[n] = '\0';
    while (*p == ' ' || *p == '\t') p++;

    /* Optional @Size N */
    if (strncasecmp(p, "@Size", 5) == 0) {
        p += 5;
        while (*p == ' ' || *p == '\t') p++;
        ExpectedSize = strtol(p, (char **)&p, 10);
        while (*p == ' ' || *p == '\t') p++;
    }

    /* @Out or @AppendTo — missing @Out is allowed (defaults to source name).
     * Real INSTALL.DAT has lots of `@File PCBCP.EXE @Size 94729` with no
     * @Out at all inside PCBOARD2.RED (OS/2) blocks — that's v1.10.3 fix. */
    if (strncasecmp(p, "@AppendTo", 9) == 0) {
        IsAppend = 1;
        p += 9;
        while (*p == ' ' || *p == '\t') p++;
    } else if (strncasecmp(p, "@Out", 4) == 0) {
        p += 4;
        while (*p == ' ' || *p == '\t') p++;
    } else {
        /* No @Out clause — destination defaults to source key */
        strncpy(DstRaw, SrcKey, sizeof(DstRaw) - 1);
        DstRaw[sizeof(DstRaw) - 1] = '\0';
        n = (int)strlen(DstRaw);
        goto have_dst;
    }

    /* Rest is destination (may include embedded @vars) */
    n = 0;
    while (*p && *p != '\r' && *p != '\n' && n < MAX_PATH_LEN - 1)
        DstRaw[n++] = *p++;
    DstRaw[n] = '\0';
    /* Strip trailing whitespace */
    while (n > 0 && (DstRaw[n-1] == ' ' || DstRaw[n-1] == '\t'))
        DstRaw[--n] = '\0';

have_dst:

    /* Trailing "@Group X" clause?  E.g.
     *   @File N @Size 252 @Out MAIN\CNAMES.IDX         @Group n
     * means "only place this file if group letter 'n' is currently selected."
     * Detect + strip it from the destination path. */
    {
        char *GrpTag = NULL;
        int   i;
        for (i = n - 7; i >= 0; i--) {
            if (DstRaw[i] == '@' &&
                (DstRaw[i+1]=='G' || DstRaw[i+1]=='g') &&
                strncasecmp(DstRaw + i, "@Group", 6) == 0 &&
                (i == 0 || DstRaw[i-1] == ' ' || DstRaw[i-1] == '\t')) {
                GrpTag = DstRaw + i;
                break;
            }
        }
        if (GrpTag) {
            char *g = GrpTag + 6;              /* skip past "@Group" */
            char  GrpLetter;
            while (*g == ' ' || *g == '\t') g++;
            GrpLetter = *g;
            /* Trim destination at the @Group marker */
            while (GrpTag > DstRaw && (*(GrpTag - 1) == ' ' ||
                                       *(GrpTag - 1) == '\t'))
                GrpTag--;
            *GrpTag = '\0';
            /* Skip if this letter isn't in the selected groups */
            if (GrpLetter && !strchr(St->SelectedGroups, GrpLetter)) {
                return;   /* group not selected — do nothing */
            }
        }
    }

    /* v1.10.3 fix: `@Out DIR\*.*` means "place under DIR/ keeping source
     * filename". Real INSTALL.DAT uses this heavily for PCBMAIL block
     * (`@Out PCBMAIL\*.*` on every file inside). */
    {
        int dl = (int)strlen(DstRaw);
        if (dl >= 3 &&
            DstRaw[dl-3] == '*' && DstRaw[dl-2] == '.' && DstRaw[dl-1] == '*') {
            /* Trim off `*.*` and any trailing sep, then append SrcKey */
            DstRaw[dl-3] = '\0';
            dl -= 3;
            while (dl > 0 && (DstRaw[dl-1] == '/' || DstRaw[dl-1] == '\\')) {
                DstRaw[--dl] = '\0';
            }
            /* Add a separator + source name */
            if (dl > 0 && dl < MAX_PATH_LEN - 2) {
                DstRaw[dl++] = '\\';
                DstRaw[dl] = '\0';
            }
            strncat(DstRaw, SrcKey, MAX_PATH_LEN - dl - 1);
        }
    }

    /* Expand @vars in dst */
    inst_expand(St, DstRaw, DstExpanded, sizeof(DstExpanded));
    inst_normalize_path(St, DstExpanded, DstNormalized, sizeof(DstNormalized));

    /* Resolve src in ExtractDir */
    if (inst_find_extracted(St, SrcKey, ExpectedSize, SrcPath,
                             sizeof(SrcPath)) != 0) {
        printf("  MISS  %s (size=%ld) in %s\n", SrcKey, ExpectedSize,
               St->CurrentArchive);
        St->FilesFailed++;
        return;
    }

    /* Copy or append */
    if (inst_copy_file(SrcPath, DstNormalized, IsAppend) != 0) {
        printf("  FAIL  %s -> %s (%s)\n", SrcKey, DstNormalized,
               strerror(errno));
        St->FilesFailed++;
        return;
    }

    St->FilesPlaced++;
}


/*-----------------------------------------------------------------------*/
/* inst_parse_quoted_pair() -- Extract "arg1","arg2" from @Cmd("a","b") */
/*                                                                       */
/* Returns number of args parsed (0, 1, or 2).                          */
/*-----------------------------------------------------------------------*/

static int inst_parse_quoted_pair(const char *Line, char *Arg1, int A1Size,
                                    char *Arg2, int A2Size)
{
    const char *p = Line;               /* scan pointer                   */
    int  n;                             /* arg-fill index                 */

    Arg1[0] = Arg2[0] = '\0';

    /* Skip to opening paren */
    while (*p && *p != '(') p++;
    if (!*p) return 0;
    p++;
    /* Skip to opening quote */
    while (*p && *p != '"') p++;
    if (!*p) return 0;
    p++;
    /* Fill Arg1 until closing quote */
    n = 0;
    while (*p && *p != '"' && n < A1Size - 1) Arg1[n++] = *p++;
    Arg1[n] = '\0';
    if (*p != '"') return 1;
    p++;
    /* Skip to comma + opening quote */
    while (*p && *p != ',') p++;
    if (!*p) return 1;
    while (*p && *p != '"') p++;
    if (!*p) return 1;
    p++;
    /* Fill Arg2 until closing quote */
    n = 0;
    while (*p && *p != '"' && n < A2Size - 1) Arg2[n++] = *p++;
    Arg2[n] = '\0';
    return 2;
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_copy() -- Handle @Copy("src","dst")                          */
/*-----------------------------------------------------------------------*/

static void inst_cmd_copy(InstState *St, const char *Line)
{
    char SrcRaw[MAX_PATH_LEN], DstRaw[MAX_PATH_LEN];
    char SrcExp[MAX_PATH_LEN], DstExp[MAX_PATH_LEN];
    char SrcNorm[MAX_PATH_LEN], DstNorm[MAX_PATH_LEN];

    if (inst_parse_quoted_pair(Line, SrcRaw, sizeof(SrcRaw),
                                DstRaw, sizeof(DstRaw)) != 2)
        return;

    inst_expand(St, SrcRaw, SrcExp, sizeof(SrcExp));
    inst_expand(St, DstRaw, DstExp, sizeof(DstExp));
    inst_normalize_path(St, SrcExp, SrcNorm, sizeof(SrcNorm));
    inst_normalize_path(St, DstExp, DstNorm, sizeof(DstNorm));

    /* @Copy source is usually in the current dir (not in an archive) —
     * try both the ArchivesDir and normalized-in-target */
    {
        FILE *T = fopen(SrcNorm, "rb");
        if (!T) {
            /* Fall back: try SrcRaw directly (from InDrive:\SubDir) */
            char AltSrc[MAX_PATH_LEN];
            snprintf(AltSrc, sizeof(AltSrc), "%s%c%s",
                     St->ArchivesDir, PATH_SEP, SrcRaw);
            if (inst_copy_file(AltSrc, DstNorm, 0) == 0) {
                St->FilesPlaced++;
                return;
            }
            printf("  COPY-MISS  %s\n", SrcRaw);
            St->FilesFailed++;
            return;
        }
        fclose(T);
    }

    if (inst_copy_file(SrcNorm, DstNorm, 0) == 0)
        St->FilesPlaced++;
    else
        St->FilesFailed++;
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_delete() -- Handle @Delete("path")                           */
/*-----------------------------------------------------------------------*/

static void inst_cmd_delete(InstState *St, const char *Line)
{
    char PathRaw[MAX_PATH_LEN], PathExp[MAX_PATH_LEN];
    char PathNorm[MAX_PATH_LEN], Dummy[16];

    if (inst_parse_quoted_pair(Line, PathRaw, sizeof(PathRaw),
                                Dummy, sizeof(Dummy)) < 1)
        return;

    inst_expand(St, PathRaw, PathExp, sizeof(PathExp));
    inst_normalize_path(St, PathExp, PathNorm, sizeof(PathNorm));

    /* Try file delete; if it's actually a dir, that's OK — remove recursively */
    if (remove(PathNorm) != 0) {
        /* Might be a directory */
        char Cmd[MAX_PATH_LEN + 32];
#ifdef _WIN32
        snprintf(Cmd, sizeof(Cmd), "rmdir /s /q \"%s\" 2>nul", PathNorm);
#else
        snprintf(Cmd, sizeof(Cmd), "rm -rf \"%s\"", PathNorm);
#endif
        system(Cmd);
    }
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_file_attr() -- Handle @FileAttr("path","r+"|"r-")            */
/*                                                                       */
/* Sets or clears the read-only attribute. Returns 0 on success, -1 on   */
/* failure. Called both standalone and inside @If — the @If wrapper      */
/* lands in v1.10.2 (control flow).                                      */
/*-----------------------------------------------------------------------*/

static int inst_cmd_file_attr(InstState *St, const char *Line)
{
    char PathRaw[MAX_PATH_LEN], PathExp[MAX_PATH_LEN];
    char PathNorm[MAX_PATH_LEN], Mode[8];
    int  ReadOnly;                      /* 1 = set r/o, 0 = clear         */

    if (inst_parse_quoted_pair(Line, PathRaw, sizeof(PathRaw),
                                Mode, sizeof(Mode)) != 2)
        return -1;

    inst_expand(St, PathRaw, PathExp, sizeof(PathExp));
    inst_normalize_path(St, PathExp, PathNorm, sizeof(PathNorm));

    ReadOnly = (Mode[0] == 'r' && Mode[1] == '-') ? 1 : 0;

#ifdef _WIN32
    {
        DWORD attr = GetFileAttributesA(PathNorm);
        if (attr == INVALID_FILE_ATTRIBUTES) return -1;
        if (ReadOnly) attr |= FILE_ATTRIBUTE_READONLY;
        else          attr &= ~FILE_ATTRIBUTE_READONLY;
        return SetFileAttributesA(PathNorm, attr) ? 0 : -1;
    }
#else
    {
        struct stat s;
        mode_t m;
        if (stat(PathNorm, &s) != 0) return -1;
        m = s.st_mode;
        if (ReadOnly) m &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
        else          m |=   S_IWUSR;
        return chmod(PathNorm, m);
    }
#endif
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                    v1.10.2 Control Flow + Expression Eval                 */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/*-----------------------------------------------------------------------*/
/* inst_should_skip() -- Are we currently in a skipped @If branch?      */
/*-----------------------------------------------------------------------*/

static int inst_should_skip(InstState *St)
{
    int i;
    for (i = 0; i < St->IfDepth; i++) {
        int active = St->IfStack[i].InElse
                     ? !St->IfStack[i].TakenTrue
                     :  St->IfStack[i].TakenTrue;
        if (!active) return 1;
    }
    return 0;
}


/*-----------------------------------------------------------------------*/
/* inst_scan_labels() -- Pre-scan pass to build label -> filepos map    */
/*                                                                       */
/* @Goto targets are lines of form `LabelName:` (at column 0 after any  */
/* whitespace). Called once before inst_process().                       */
/*-----------------------------------------------------------------------*/

static void inst_scan_labels(InstState *St)
{
    char Line[MAX_LINE];
    long Pos;

    St->NumLabelDefs = 0;
    rewind(St->ScriptFp);

    Pos = ftell(St->ScriptFp);
    while (fgets(Line, sizeof(Line), St->ScriptFp)) {
        char *p = Line;
        char *End;
        while (*p == ' ' || *p == '\t') p++;

        /* Line format: NAME:\n where NAME is [A-Za-z_][A-Za-z0-9_]* */
        if (isalpha(*p) || *p == '_') {
            char *Start = p;
            while (isalnum(*p) || *p == '_') p++;
            if (*p == ':' && (p[1] == '\r' || p[1] == '\n' || p[1] == '\0')) {
                int Len = (int)(p - Start);
                if (Len < 63 && St->NumLabelDefs < MAX_LABELS) {
                    memcpy(St->LabelName[St->NumLabelDefs], Start, Len);
                    St->LabelName[St->NumLabelDefs][Len] = '\0';
                    St->LabelPos[St->NumLabelDefs] = ftell(St->ScriptFp);
                    St->NumLabelDefs++;
                }
            }
        }
        (void)End;
        Pos = ftell(St->ScriptFp);
    }
    (void)Pos;

    rewind(St->ScriptFp);
}


/*-----------------------------------------------------------------------*/
/* inst_goto_label() -- fseek to a named label                          */
/*                                                                       */
/* Returns 0 on success, -1 if label not found.                         */
/*-----------------------------------------------------------------------*/

static int inst_goto_label(InstState *St, const char *Name)
{
    int i;
    for (i = 0; i < St->NumLabelDefs; i++) {
        if (strcasecmp(St->LabelName[i], Name) == 0) {
            fseek(St->ScriptFp, St->LabelPos[i], SEEK_SET);
            /* Reset @If stack on goto — we jump out of any enclosing @If */
            St->IfDepth = 0;
            return 0;
        }
    }
    return -1;
}


/*-----------------------------------------------------------------------*/
/* Expression evaluator                                                  */
/*                                                                       */
/* Parses a subset of INSTALL.DAT expressions:                          */
/*   Primary: number | "string" | @Var | @Func(args) | (expr)           */
/*   Compare: == != > < >= <= [= [!                                     */
/*   Logic:   && ||                                                      */
/*                                                                       */
/* EvalVal carries both int and string interpretations. Comparisons     */
/* work on strings if either side is string, else on ints.              */
/*-----------------------------------------------------------------------*/

typedef struct {
    int  IsString;                      /* 1 if string, 0 if int          */
    long IVal;                          /* integer value                  */
    char SVal[512];                     /* string value                   */
} EvalVal;

static const char *inst_eval_expr(InstState *St, const char *p, EvalVal *Out);
static const char *inst_eval_or(InstState *St, const char *p, EvalVal *Out);
static const char *inst_eval_and(InstState *St, const char *p, EvalVal *Out);
static const char *inst_eval_cmp(InstState *St, const char *p, EvalVal *Out);
static const char *inst_eval_primary(InstState *St, const char *p, EvalVal *Out);


static const char *skip_ws(const char *p)
{
    while (*p == ' ' || *p == '\t') p++;
    return p;
}


/*-----------------------------------------------------------------------*/
/* Function-call helpers evaluated during expression parsing             */
/*-----------------------------------------------------------------------*/

static long inst_func_strlen(InstState *St, const char *ArgsRaw)
{
    char Str[512], Exp[512];
    const char *p = skip_ws(ArgsRaw);
    int  n = 0;
    if (*p == '"') p++;
    while (*p && *p != '"' && n < 511) Str[n++] = *p++;
    Str[n] = '\0';
    inst_expand(St, Str, Exp, sizeof(Exp));
    return (long)strlen(Exp);
}


static void inst_func_strhead(InstState *St, const char *ArgsRaw, char *OutBuf,
                               int OutSize)
{
    char Str[512], Exp[512];
    const char *p = skip_ws(ArgsRaw);
    int  n = 0, MaxLen;
    if (*p == '"') p++;
    while (*p && *p != '"' && n < 511) Str[n++] = *p++;
    Str[n] = '\0';
    if (*p == '"') p++;
    while (*p == ' ' || *p == '\t' || *p == ',') p++;
    MaxLen = (int)strtol(p, NULL, 10);
    if (MaxLen < 0) MaxLen = 0;
    if (MaxLen >= OutSize) MaxLen = OutSize - 1;
    inst_expand(St, Str, Exp, sizeof(Exp));
    if ((int)strlen(Exp) < MaxLen) MaxLen = (int)strlen(Exp);
    memcpy(OutBuf, Exp, MaxLen);
    OutBuf[MaxLen] = '\0';
}


static void inst_func_strtoken(InstState *St, const char *ArgsRaw,
                                char *OutBuf, int OutSize)
{
    char Str[512], Exp[512];
    const char *p = skip_ws(ArgsRaw);
    int  n = 0, Idx, i, Cur = 0;
    char Delim = ' ';
    char *Tokens[64];
    int  NumTokens = 0;
    char Work[512];

    if (*p == '"') p++;
    while (*p && *p != '"' && n < 511) Str[n++] = *p++;
    Str[n] = '\0';
    if (*p == '"') p++;
    while (*p == ' ' || *p == '\t' || *p == ',') p++;
    Idx = (int)strtol(p, (char **)&p, 10);
    while (*p == ' ' || *p == '\t' || *p == ',') p++;
    if (*p == '"') p++;
    if (*p) Delim = *p;

    inst_expand(St, Str, Exp, sizeof(Exp));

    /* Split Exp on Delim */
    strncpy(Work, Exp, sizeof(Work) - 1);
    Work[sizeof(Work) - 1] = '\0';
    Tokens[NumTokens++] = Work;
    for (i = 0; Work[i] && NumTokens < 64; i++) {
        if (Work[i] == Delim) {
            Work[i] = '\0';
            if (Work[i+1]) Tokens[NumTokens++] = &Work[i+1];
        }
    }

    if (Idx < 0 || Idx >= NumTokens) { OutBuf[0] = '\0'; return; }
    strncpy(OutBuf, Tokens[Idx], OutSize - 1);
    OutBuf[OutSize - 1] = '\0';
    (void)Cur;
}


static int inst_func_exists(InstState *St, const char *ArgsRaw)
{
    char Str[MAX_PATH_LEN], Exp[MAX_PATH_LEN], Norm[MAX_PATH_LEN];
    const char *p = skip_ws(ArgsRaw);
    int  n = 0;
    FILE *F;

    if (*p == '"') p++;
    while (*p && *p != '"' && n < MAX_PATH_LEN - 1) Str[n++] = *p++;
    Str[n] = '\0';
    inst_expand(St, Str, Exp, sizeof(Exp));
    inst_normalize_path(St, Exp, Norm, sizeof(Norm));

    F = fopen(Norm, "rb");
    if (F) { fclose(F); return 1; }

    /* Try as directory */
    {
        struct stat s;
        if (stat(Norm, &s) == 0 && (s.st_mode & S_IFDIR)) return 1;
    }
    return 0;
}


/*-----------------------------------------------------------------------*/
/* inst_eval_primary() -- number | "string" | @Var | @Func(args) | (expr) */
/*-----------------------------------------------------------------------*/

static const char *inst_eval_primary(InstState *St, const char *p, EvalVal *Out)
{
    p = skip_ws(p);
    Out->IsString = 0;
    Out->IVal = 0;
    Out->SVal[0] = '\0';

    /* Parenthesized subexpression */
    if (*p == '(') {
        p++;
        p = inst_eval_or(St, p, Out);
        p = skip_ws(p);
        if (*p == ')') p++;
        return p;
    }

    /* Negative number */
    if (*p == '-' && isdigit(p[1])) {
        char *End;
        Out->IVal = strtol(p, &End, 10);
        return End;
    }

    /* Integer literal */
    if (isdigit(*p)) {
        char *End;
        Out->IVal = strtol(p, &End, 10);
        return End;
    }

    /* Quoted string literal — track parenthesis depth so nested `"` inside
     * @Foo(...) function-call args don't terminate the literal prematurely.
     * Clark's INSTALL.DAT uses shapes like  "@StrToken(\"@Fname\",0,\" \")"
     * where the inner `"`s are inside function-call parens and must be
     * treated as literal characters, not as string terminators. */
    if (*p == '"') {
        char Raw[512];
        int  n = 0;
        int  ParenDepth = 0;
        p++;
        while (*p && n < 511) {
            if (*p == '(') ParenDepth++;
            else if (*p == ')' && ParenDepth > 0) ParenDepth--;
            else if (*p == '"' && ParenDepth == 0) break;
            Raw[n++] = *p++;
        }
        Raw[n] = '\0';
        if (*p == '"') p++;
        Out->IsString = 1;
        inst_expand(St, Raw, Out->SVal, sizeof(Out->SVal));
        return p;
    }

    /* @Var or @Func(args) or single-letter identifier */
    if (*p == '@' || isalpha(*p)) {
        char Ident[64];
        int  n = 0;
        if (*p == '@') p++;
        while (*p && (isalnum(*p) || *p == '_') && n < 63) Ident[n++] = *p++;
        Ident[n] = '\0';
        p = skip_ws(p);

        if (*p == '(') {
            /* Function call — collect args until matching ')' */
            char Args[512];
            int  Depth = 1;
            int  ai = 0;
            p++;
            while (*p && ai < 511) {
                if (*p == '(') Depth++;
                else if (*p == ')') { Depth--; if (Depth == 0) break; }
                Args[ai++] = *p++;
            }
            Args[ai] = '\0';
            if (*p == ')') p++;

            if (strcasecmp(Ident, "StrLen") == 0) {
                Out->IVal = inst_func_strlen(St, Args);
            } else if (strcasecmp(Ident, "StrHead") == 0) {
                Out->IsString = 1;
                inst_func_strhead(St, Args, Out->SVal, sizeof(Out->SVal));
            } else if (strcasecmp(Ident, "StrToken") == 0) {
                Out->IsString = 1;
                inst_func_strtoken(St, Args, Out->SVal, sizeof(Out->SVal));
            } else if (strcasecmp(Ident, "Exists") == 0) {
                Out->IVal = inst_func_exists(St, Args);
            } else if (strcasecmp(Ident, "DirExists") == 0) {
                /* v1.10.3: like @Exists but stat-check specifically for
                 * a directory (S_IFDIR). Under Unix hosts, @Exists already
                 * returns 1 for dirs, so this is effectively an alias. */
                Out->IVal = inst_func_exists(St, Args);
            } else if (strcasecmp(Ident, "Mkdir") == 0 ||
                       strcasecmp(Ident, "MkDir") == 0) {
                /* v1.10.3: actually create the directory. Return 0 on any
                 * outcome so `@If (@Mkdir(...))` empty-body pattern doesn't
                 * loop or misbehave — real INSTALL.DAT uses these calls
                 * for side effect only (`@If @Mkdir(...) @Endif` with
                 * nothing between). */
                char Str[MAX_PATH_LEN], Exp[MAX_PATH_LEN], Norm[MAX_PATH_LEN];
                const char *pp = skip_ws(Args);
                int nn = 0;
                if (*pp == '"') pp++;
                while (*pp && *pp != '"' && nn < MAX_PATH_LEN - 1)
                    Str[nn++] = *pp++;
                Str[nn] = '\0';
                inst_expand(St, Str, Exp, sizeof(Exp));
                inst_normalize_path(St, Exp, Norm, sizeof(Norm));
                inst_mkdir_p(Norm);
                Out->IVal = 0;
            } else if (strcasecmp(Ident, "FileAttr") == 0) {
                /* Two forms: query (1 arg) — v1.10.5 territory; and setter
                 * (2 args) — but the setter is already called at line
                 * dispatch. Here in expression context it's the checker.
                 * Simplification: return 0 (success). */
                Out->IVal = 0;
            } else if (strcasecmp(Ident, "System") == 0) {
                /* @System(cmd) — v1.10.5. For v1.10.2 always return 0. */
                Out->IVal = 0;
            } else if (strcasecmp(Ident, "Group") == 0) {
                /* Empty function-form of @Group — treat as string of selected groups */
                Out->IsString = 1;
                strncpy(Out->SVal, St->SelectedGroups, sizeof(Out->SVal) - 1);
                Out->SVal[sizeof(Out->SVal) - 1] = '\0';
            } else {
                /* Unknown function — return 0 */
                Out->IVal = 0;
            }
            return p;
        }

        /* Not a function — treat as variable reference (@Var or bare letter) */
        {
            const char *Val;
            if (strcasecmp(Ident, "Group") == 0) {
                Out->IsString = 1;
                strncpy(Out->SVal, St->SelectedGroups, sizeof(Out->SVal) - 1);
                Out->SVal[sizeof(Out->SVal) - 1] = '\0';
                return p;
            }
            Val = inst_get_var(St, Ident);
            if (Val && *Val) {
                Out->IsString = 1;
                strncpy(Out->SVal, Val, sizeof(Out->SVal) - 1);
                Out->SVal[sizeof(Out->SVal) - 1] = '\0';
            } else {
                /* Not a known variable — treat identifier itself as string
                 * (single-letter group ids fall here: `a [= @Group`) */
                Out->IsString = 1;
                strncpy(Out->SVal, Ident, sizeof(Out->SVal) - 1);
                Out->SVal[sizeof(Out->SVal) - 1] = '\0';
            }
            return p;
        }
    }

    return p;
}


/*-----------------------------------------------------------------------*/
/* inst_eval_cmp() -- primary [op primary]                              */
/*                                                                       */
/* Comparison operators: == != > < >= <= [= [!                          */
/*  [= means "left substring/char is contained in right string"         */
/*  [! is the negation                                                  */
/*-----------------------------------------------------------------------*/

static const char *inst_eval_cmp(InstState *St, const char *p, EvalVal *Out)
{
    EvalVal Rhs;
    char Op[4] = "";
    long Result = 0;
    int  CmpAsString = 0;

    p = inst_eval_primary(St, p, Out);
    p = skip_ws(p);

    /* Look for comparison operator */
    if (p[0] == '=' && p[1] == '=') { Op[0]='='; Op[1]='='; p += 2; }
    else if (p[0] == '!' && p[1] == '=') { Op[0]='!'; Op[1]='='; p += 2; }
    else if (p[0] == '>' && p[1] == '=') { Op[0]='>'; Op[1]='='; p += 2; }
    else if (p[0] == '<' && p[1] == '=') { Op[0]='<'; Op[1]='='; p += 2; }
    else if (p[0] == '>') { Op[0]='>'; p++; }
    else if (p[0] == '<') { Op[0]='<'; p++; }
    else if (p[0] == '[' && p[1] == '=') { Op[0]='['; Op[1]='='; p += 2; }
    else if (p[0] == '[' && p[1] == '!') { Op[0]='['; Op[1]='!'; p += 2; }
    else return p;

    p = skip_ws(p);
    p = inst_eval_primary(St, p, &Rhs);
    CmpAsString = (Out->IsString || Rhs.IsString);

    if (Op[0] == '[') {
        /* Substring / containment test */
        char Needle[512];
        const char *Hay = Rhs.IsString ? Rhs.SVal : "";
        if (Out->IsString) strncpy(Needle, Out->SVal, sizeof(Needle) - 1);
        else snprintf(Needle, sizeof(Needle), "%ld", Out->IVal);
        Needle[sizeof(Needle) - 1] = '\0';
        {
            int Found = (strstr(Hay, Needle) != NULL);
            if (Op[1] == '=') Result = Found ? 1 : 0;
            else              Result = Found ? 0 : 1;
        }
    } else if (CmpAsString) {
        const char *A = Out->IsString ? Out->SVal : "";
        const char *B = Rhs.IsString  ? Rhs.SVal  : "";
        int Cmp = strcmp(A, B);
        if (Op[0] == '=' && Op[1] == '=')      Result = (Cmp == 0);
        else if (Op[0] == '!' && Op[1] == '=') Result = (Cmp != 0);
        else if (Op[0] == '>' && Op[1] == '=') Result = (Cmp >= 0);
        else if (Op[0] == '<' && Op[1] == '=') Result = (Cmp <= 0);
        else if (Op[0] == '>')                 Result = (Cmp >  0);
        else if (Op[0] == '<')                 Result = (Cmp <  0);
    } else {
        long A = Out->IVal, B = Rhs.IVal;
        if (Op[0] == '=' && Op[1] == '=')      Result = (A == B);
        else if (Op[0] == '!' && Op[1] == '=') Result = (A != B);
        else if (Op[0] == '>' && Op[1] == '=') Result = (A >= B);
        else if (Op[0] == '<' && Op[1] == '=') Result = (A <= B);
        else if (Op[0] == '>')                 Result = (A >  B);
        else if (Op[0] == '<')                 Result = (A <  B);
    }

    Out->IsString = 0;
    Out->IVal = Result;
    Out->SVal[0] = '\0';
    return p;
}


static const char *inst_eval_and(InstState *St, const char *p, EvalVal *Out)
{
    p = inst_eval_cmp(St, p, Out);
    for (;;) {
        p = skip_ws(p);
        if (p[0] == '&' && p[1] == '&') {
            EvalVal Rhs;
            int L = Out->IsString ? (Out->SVal[0] != '\0') : (Out->IVal != 0);
            int R;
            p += 2;
            p = skip_ws(p);
            p = inst_eval_cmp(St, p, &Rhs);
            R = Rhs.IsString ? (Rhs.SVal[0] != '\0') : (Rhs.IVal != 0);
            Out->IsString = 0;
            Out->IVal = (L && R) ? 1 : 0;
            Out->SVal[0] = '\0';
        } else break;
    }
    return p;
}


static const char *inst_eval_or(InstState *St, const char *p, EvalVal *Out)
{
    p = inst_eval_and(St, p, Out);
    for (;;) {
        p = skip_ws(p);
        if (p[0] == '|' && p[1] == '|') {
            EvalVal Rhs;
            int L = Out->IsString ? (Out->SVal[0] != '\0') : (Out->IVal != 0);
            int R;
            p += 2;
            p = skip_ws(p);
            p = inst_eval_and(St, p, &Rhs);
            R = Rhs.IsString ? (Rhs.SVal[0] != '\0') : (Rhs.IVal != 0);
            Out->IsString = 0;
            Out->IVal = (L || R) ? 1 : 0;
            Out->SVal[0] = '\0';
        } else break;
    }
    return p;
}


static const char *inst_eval_expr(InstState *St, const char *p, EvalVal *Out)
{
    return inst_eval_or(St, p, Out);
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_if() -- Handle @If (expr)                                    */
/*-----------------------------------------------------------------------*/

static void inst_cmd_if(InstState *St, const char *Line)
{
    const char *p = Line;
    EvalVal V;
    int TrueVal;

    /* Skip past "@If" */
    while (*p == ' ' || *p == '\t') p++;
    if (strncasecmp(p, "@If", 3) == 0) p += 3;
    p = skip_ws(p);

    (void)inst_eval_expr(St, p, &V);
    TrueVal = V.IsString ? (V.SVal[0] != '\0') : (V.IVal != 0);

    if (St->IfDepth < 32) {
        St->IfStack[St->IfDepth].TakenTrue = TrueVal;
        St->IfStack[St->IfDepth].InElse    = 0;
        St->IfDepth++;
    }
}


static void inst_cmd_else(InstState *St)
{
    if (St->IfDepth > 0) {
        St->IfStack[St->IfDepth - 1].InElse = 1;
    }
}


static void inst_cmd_endif(InstState *St)
{
    if (St->IfDepth > 0) St->IfDepth--;
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_goto() -- Handle @Goto <label>                               */
/*-----------------------------------------------------------------------*/

static void inst_cmd_goto(InstState *St, const char *Line)
{
    const char *p = Line;
    char LabelName[64];
    int  n = 0;

    while (*p == ' ' || *p == '\t') p++;
    if (strncasecmp(p, "@Goto", 5) == 0) p += 5;
    while (*p == ' ' || *p == '\t') p++;

    while (*p && !isspace(*p) && n < 63) LabelName[n++] = *p++;
    LabelName[n] = '\0';

    if (inst_goto_label(St, LabelName) != 0) {
        printf("  GOTO-MISS  %s (label not found)\n", LabelName);
    }
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_get_groups() -- Handle @GetGroups ... @EndGroups            */
/*                                                                       */
/* v1.10.4: parses menu items from @Set-single-letter statements inside  */
/* the block. If TTY, presents a minimal interactive picker that lets    */
/* the user toggle group letters. Non-TTY runs use --groups CLI arg      */
/* (already set in state).                                               */
/*-----------------------------------------------------------------------*/

static void inst_cmd_get_groups(InstState *St, int IsCheckbox)
{
    char Line[MAX_LINE];
    int  StartCount = St->NumMenuItems;

    /* Read block content, extracting @Set X = "label" entries */
    while (fgets(Line, sizeof(Line), St->ScriptFp)) {
        char *p = Line;
        char *eq;
        char  VarName[32] = "", Val[128] = "";

        while (*p == ' ' || *p == '\t') p++;
        if (strncasecmp(p, "@EndGroups", 10) == 0) break;

        /* Also allow @CheckBox declaration inside the block */
        if (strncasecmp(p, "@CheckBox", 9) == 0) {
            IsCheckbox = 1;
            continue;
        }

        /* Look for @Set X = "..."  pattern */
        if (strncasecmp(p, "@Set", 4) == 0) {
            char *sp = p + 4;
            int n = 0;
            while (*sp == ' ' || *sp == '\t') sp++;
            while (*sp && !isspace(*sp) && *sp != '=' && n < 31)
                VarName[n++] = *sp++;
            VarName[n] = '\0';
            eq = strchr(sp, '=');
            if (eq && strlen(VarName) == 1 &&
                islower((unsigned char)VarName[0])) {
                /* Extract quoted label */
                char *q = strchr(eq, '"');
                if (q) {
                    q++;
                    n = 0;
                    while (*q && *q != '"' && n < 127) Val[n++] = *q++;
                    Val[n] = '\0';
                    /* Record menu item */
                    if (St->NumMenuItems < 32) {
                        St->MenuItems[St->NumMenuItems].Letter = VarName[0];
                        strncpy(St->MenuItems[St->NumMenuItems].Label,
                                Val, 127);
                        St->MenuItems[St->NumMenuItems].IsCheckbox =
                            IsCheckbox;
                        St->NumMenuItems++;
                    }
                }
            }
        }
    }

    /* If TTY, prompt the user to confirm / override the selection */
#ifndef __WATCOMC__
    if (isatty(fileno(stdin)) && isatty(fileno(stdout)) &&
        St->NumMenuItems > StartCount) {
        int i;
        char InLine[128];
        printf("\n Choose install %s:\n\n",
               IsCheckbox ? "options (multi-select)" :
                            "type (radio)");
        for (i = StartCount; i < St->NumMenuItems; i++) {
            char L = St->MenuItems[i].Letter;
            int Sel = (strchr(St->SelectedGroups, L) != NULL);
            printf("   [%c] %c  %s\n",
                   Sel ? 'X' : ' ',
                   L,
                   St->MenuItems[i].Label);
        }
        printf("\n Enter letters to toggle (or ENTER to accept): ");
        fflush(stdout);
        if (fgets(InLine, sizeof(InLine), stdin)) {
            char *c = InLine;
            while (*c) {
                if (islower((unsigned char)*c)) {
                    char *found = strchr(St->SelectedGroups, *c);
                    if (found) {
                        /* toggle off */
                        memmove(found, found+1, strlen(found));
                    } else {
                        /* toggle on */
                        int len = (int)strlen(St->SelectedGroups);
                        if (len < (int)sizeof(St->SelectedGroups) - 1) {
                            St->SelectedGroups[len] = *c;
                            St->SelectedGroups[len+1] = '\0';
                        }
                    }
                }
                c++;
            }
        }
        St->InteractiveMenus++;
        printf(" Groups now: %s\n\n", St->SelectedGroups);
    }
#endif
    (void)StartCount;
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_get_string() -- Real @GetString handler (v1.10.4)           */
/*                                                                       */
/* If TTY: read a line from stdin (respecting @Prompt if set), assign    */
/* to the target variable, then skip to @EndString.                      */
/* If not TTY: falls back to the v1.10.2 stub behavior (pre-filled       */
/* sensible defaults so validation loops don't spin).                    */
/*-----------------------------------------------------------------------*/

static void inst_cmd_get_string(InstState *St, const char *Line)
{
    const char *p = Line;
    char VarName[64] = "";
    int  n = 0;
    char Discard[MAX_LINE];
    char Prompt[128] = "";
    char InLine[256];

    while (*p == ' ' || *p == '\t') p++;
    if (strncasecmp(p, "@GetString", 10) == 0) p += 10;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '@') p++;
    while (*p && (isalnum(*p) || *p == '_') && n < 63) VarName[n++] = *p++;
    VarName[n] = '\0';

    /* Read block, extracting @Prompt if given */
    while (fgets(Discard, sizeof(Discard), St->ScriptFp)) {
        char *q = Discard;
        while (*q == ' ' || *q == '\t') q++;
        if (strncasecmp(q, "@EndString", 10) == 0) break;
        if (strncasecmp(q, "@Prompt", 7) == 0) {
            char *eq = strchr(q, '=');
            if (eq) {
                char *qs = strchr(eq, '"');
                if (qs) {
                    qs++;
                    n = 0;
                    while (*qs && *qs != '"' && n < 127) Prompt[n++] = *qs++;
                    Prompt[n] = '\0';
                }
            }
        }
    }

    if (!VarName[0]) return;

#ifndef __WATCOMC__
    if (isatty(fileno(stdin))) {
        if (Prompt[0]) printf("%s", Prompt);
        else printf(" Enter %s: ", VarName);
        fflush(stdout);
        if (fgets(InLine, sizeof(InLine), stdin)) {
            /* Strip newline */
            InLine[strcspn(InLine, "\r\n")] = '\0';
            /* If empty, keep existing value; else set */
            if (InLine[0]) {
                inst_set_var(St, VarName, InLine);
                return;
            }
        }
    }
#endif

    /* Headless fallback: pre-fill if empty */
    {
        const char *Cur = inst_get_var(St, VarName);
        if (!Cur || !*Cur) {
            const char *Def = "TEST";
            if (strcasecmp(VarName, "Fname") == 0)   Def = "SysOp";
            else if (strcasecmp(VarName, "Lname") == 0)   Def = "Operator";
            else if (strcasecmp(VarName, "CitySt") == 0)  Def = "Unknown, XX";
            else if (strcasecmp(VarName, "Pwd") == 0)     Def = "password";
            else if (strcasecmp(VarName, "RegCode") == 0) Def = "0";
            inst_set_var(St, VarName, Def);
        }
    }
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_set_group() / inst_cmd_clear_group() -- Modify SelectedGroups*/
/*                                                                       */
/* @SetGroup(x) — mark group letter x as selected                        */
/* @ClearGroup(x) — mark it deselected                                   */
/*-----------------------------------------------------------------------*/

static void inst_cmd_set_group(InstState *St, const char *Line)
{
    const char *p = strchr(Line, '(');
    char Letter;
    if (!p) return;
    p++;
    while (*p == ' ' || *p == '"') p++;
    Letter = *p;
    if (!islower((unsigned char)Letter)) return;
    if (!strchr(St->SelectedGroups, Letter)) {
        int len = (int)strlen(St->SelectedGroups);
        if (len < (int)sizeof(St->SelectedGroups) - 1) {
            St->SelectedGroups[len] = Letter;
            St->SelectedGroups[len+1] = '\0';
        }
    }
}


static void inst_cmd_clear_group(InstState *St, const char *Line)
{
    const char *p = strchr(Line, '(');
    char Letter;
    char *found;
    if (!p) return;
    p++;
    while (*p == ' ' || *p == '"') p++;
    Letter = *p;
    if (!islower((unsigned char)Letter)) return;
    found = strchr(St->SelectedGroups, Letter);
    if (found) memmove(found, found + 1, strlen(found));
}


static void inst_cmd_skip_block(InstState *St, const char *EndTag)
{
    char Line[MAX_LINE];
    int  TagLen = (int)strlen(EndTag);

    while (fgets(Line, sizeof(Line), St->ScriptFp)) {
        char *p = Line;
        while (*p == ' ' || *p == '\t') p++;
        if (strncasecmp(p, EndTag, TagLen) == 0) break;
    }
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_chdrive() -- Handle @ChDrive <drive> (bare directive form)  */
/*                                                                       */
/* Tracks current working drive in state. Under Unix hosts this is a     */
/* no-op filesystem-wise (no drive letters), but the state is kept for   */
/* future path resolution needs.                                         */
/*-----------------------------------------------------------------------*/

static void inst_cmd_chdrive(InstState *St, const char *Line)
{
    const char *p = Line;
    char Arg[64], Exp[64];
    int  n = 0;

    while (*p == ' ' || *p == '\t') p++;
    if (strncasecmp(p, "@ChDrive", 8) == 0) p += 8;
    while (*p == ' ' || *p == '\t') p++;
    while (*p && !isspace(*p) && n < 63) Arg[n++] = *p++;
    Arg[n] = '\0';
    inst_expand(St, Arg, Exp, sizeof(Exp));
    if (Exp[0]) St->WorkingDrive = Exp[0];
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_chdir() -- Handle @ChDir "path" (bare directive form)       */
/*-----------------------------------------------------------------------*/

static void inst_cmd_chdir(InstState *St, const char *Line)
{
    const char *p = Line;
    char Arg[MAX_PATH_LEN], Exp[MAX_PATH_LEN];
    int  n = 0;

    while (*p == ' ' || *p == '\t') p++;
    if (strncasecmp(p, "@ChDir", 6) == 0) p += 6;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '"') p++;
    while (*p && *p != '"' && *p != '\r' && *p != '\n' &&
           n < MAX_PATH_LEN - 1)
        Arg[n++] = *p++;
    Arg[n] = '\0';
    inst_expand(St, Arg, Exp, sizeof(Exp));
    strncpy(St->WorkingDir, Exp, sizeof(St->WorkingDir) - 1);
    St->WorkingDir[sizeof(St->WorkingDir) - 1] = '\0';
}


/*-----------------------------------------------------------------------*/
/* inst_cmd_set() -- Handle @Set var = value  or  @Var = value           */
/*                                                                       */
/* Line has already had "@Set" (if present) stripped; parse "var = expr" */
/*-----------------------------------------------------------------------*/

static void inst_cmd_set(InstState *St, const char *Line)
{
    const char *p = Line;
    char VarName[64];
    char Val[512];
    int  n = 0;
    EvalVal V;

    while (*p == ' ' || *p == '\t') p++;
    if (*p == '@') p++;
    while (*p && (isalnum(*p) || *p == '_') && n < 63) VarName[n++] = *p++;
    VarName[n] = '\0';

    p = skip_ws(p);
    if (*p != '=') return;
    p++;
    p = skip_ws(p);

    /* Evaluate RHS */
    inst_eval_expr(St, p, &V);
    if (V.IsString)
        strncpy(Val, V.SVal, sizeof(Val) - 1);
    else
        snprintf(Val, sizeof(Val), "%ld", V.IVal);
    Val[sizeof(Val) - 1] = '\0';

    /* Special: assignments to built-in project vars update the state */
    if (strcasecmp(VarName, "SubDir") == 0 ||
        strcasecmp(VarName, "Subdir") == 0) {
        strncpy(St->SubDir, Val, sizeof(St->SubDir) - 1);
        St->SubDir[sizeof(St->SubDir) - 1] = '\0';
    } else if (strcasecmp(VarName, "OutDrive") == 0) {
        St->OutDrive = Val[0] ? Val[0] : 'C';
    } else if (strcasecmp(VarName, "Name") == 0) {
        strncpy(St->ProjName, Val, sizeof(St->ProjName) - 1);
    } else if (strcasecmp(VarName, "Version") == 0) {
        strncpy(St->ProjVersion, Val, sizeof(St->ProjVersion) - 1);
    } else if (strcasecmp(VarName, "Label") == 0 ||
               strcasecmp(VarName, "Prompt") == 0 ||
               strcasecmp(VarName, "Files") == 0 ||
               strcasecmp(VarName, "Path") == 0) {
        /* These are metadata / v1.10.5-territory — accept but no side effect */
        inst_set_var(St, VarName, Val);
    } else if (strlen(VarName) == 1 && islower((unsigned char)VarName[0])) {
        /* Single lowercase letter: group-label declaration inside a
         * @GetGroups...@EndGroups block. NOT a normal variable — Clark's
         * script uses these as checkbox display strings, and `a [= @Group`
         * later tests the LETTER against the selected-groups state. If we
         * stored it as a variable, `a` in expressions would resolve to
         * the label text, breaking the group test. Skip. */
    } else {
        inst_set_var(St, VarName, Val);
    }
}




static int inst_process(InstState *St)
{
    char Line[MAX_LINE];                /* line read buffer               */

    while (fgets(Line, sizeof(Line), St->ScriptFp)) {
        char *p = Line;                 /* line scan pointer              */
        int   Skip;                     /* skipping due to @If (false)?   */

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

        Skip = inst_should_skip(St);

        /* When skipping, we only track @If nesting + @Else/@Endif. */
        if (Skip) {
            if (strncasecmp(p, "@If", 3) == 0 && !isalnum(p[3])) {
                if (St->IfDepth < 32) {
                    St->IfStack[St->IfDepth].TakenTrue = 0;
                    St->IfStack[St->IfDepth].InElse    = 0;
                    St->IfDepth++;
                }
            }
            else if (strncasecmp(p, "@Else", 5) == 0 && !isalnum(p[5])) {
                inst_cmd_else(St);
            }
            else if (strncasecmp(p, "@Endif", 6) == 0 ||
                     strncasecmp(p, "@EndIf", 6) == 0) {
                inst_cmd_endif(St);
            }
            continue;
        }

        /* v1.10.2: control flow */
        if (strncasecmp(p, "@If", 3) == 0 && !isalnum(p[3])) {
            inst_cmd_if(St, p);
        }
        else if (strncasecmp(p, "@Else", 5) == 0 && !isalnum(p[5])) {
            inst_cmd_else(St);
        }
        else if (strncasecmp(p, "@Endif", 6) == 0 ||
                 strncasecmp(p, "@EndIf", 6) == 0) {
            inst_cmd_endif(St);
        }
        else if (strncasecmp(p, "@Goto", 5) == 0 && !isalnum(p[5])) {
            inst_cmd_goto(St, p);
        }
        else if (strncasecmp(p, "@Set ", 5) == 0 ||
                 strncasecmp(p, "@Set\t", 5) == 0) {
            inst_cmd_set(St, p + 4);
        }

        /* v1.10.0 dispatch */
        else if (strncasecmp(p, "@DefineProject", 14) == 0)
            inst_cmd_define_project(St);
        else if (strncasecmp(p, "@DefineVars", 11) == 0)
            inst_cmd_define_vars(St);
        else if (strncasecmp(p, "@Display", 8) == 0)
            inst_cmd_display(St);
        else if (strncasecmp(p, "@Cls", 4) == 0)
            printf("\033[2J\033[H");
        else if (strncasecmp(p, "@Pause", 6) == 0) {
            printf("\n PRESS ANY KEY ");
            fflush(stdout);
#ifdef __WATCOMC__
            getch();
#else
            /* Skip actual keypress when stdin isn't a tty (for automated
             * end-to-end tests) */
            if (isatty(fileno(stdin))) getchar();
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

        /* v1.10.1 dispatch: file operations */
        else if (strncasecmp(p, "@BeginLib", 9) == 0) {
            char ArchName[64];
            const char *ap = p + 9;
            int n = 0;
            while (*ap == ' ' || *ap == '\t') ap++;
            while (*ap && !isspace(*ap) && n < 63) ArchName[n++] = *ap++;
            ArchName[n] = '\0';
            inst_cmd_begin_lib(St, ArchName);
        }
        else if (strncasecmp(p, "@EndLib", 7) == 0) {
            inst_cmd_end_lib(St);
        }
        else if (strncasecmp(p, "@File", 5) == 0 && !isalnum(p[5])) {
            inst_cmd_file(St, p);
        }
        else if (strncasecmp(p, "@Copy", 5) == 0) {
            inst_cmd_copy(St, p);
        }
        else if (strncasecmp(p, "@Delete", 7) == 0) {
            inst_cmd_delete(St, p);
        }
        else if (strncasecmp(p, "@FileAttr", 9) == 0) {
            inst_cmd_file_attr(St, p);
        }

        /* v1.10.4: real interactive handlers (fall back to headless
         * defaults when stdin isn't a TTY) */
        else if (strncasecmp(p, "@GetString", 10) == 0) {
            inst_cmd_get_string(St, p);
        }
        else if (strncasecmp(p, "@GetOutDrive", 12) == 0) {
            inst_cmd_skip_block(St, "@EndOutDrive");
        }
        else if (strncasecmp(p, "@GetSubdir", 10) == 0) {
            inst_cmd_skip_block(St, "@EndSubdir");
        }
        else if (strncasecmp(p, "@GetGroups", 10) == 0) {
            inst_cmd_get_groups(St, 0);  /* @CheckBox inside sets multi */
        }
        else if (strncasecmp(p, "@AskOverwrite", 13) == 0) {
            /* v1.10.4: accepted as directive — real prompt would ask user
             * "overwrite existing?" per file. Headless: always yes. */
        }
        else if (strncasecmp(p, "@Prompt", 7) == 0) {
            /* v1.10.4: @Prompt = "..." at top-level. Store as variable. */
            const char *eq = strchr(p, '=');
            if (eq) inst_cmd_set(St, p);
        }
        else if (strncasecmp(p, "@SetGroup", 9) == 0) {
            inst_cmd_set_group(St, p);
        }
        else if (strncasecmp(p, "@ClearGroup", 11) == 0) {
            inst_cmd_clear_group(St, p);
        }
        else if (strncasecmp(p, "@DefineDisk", 11) == 0) {
            /* v1.10.3: disk boundaries are organizational. Content inside
             * is gated by inner @If; nothing extra needed at boundary. */
        }
        else if (strncasecmp(p, "@EndDisk", 8) == 0) {
            /* v1.10.3: matching boundary end */
        }
        else if (strncasecmp(p, "@SetConfig", 10) == 0) {
            inst_cmd_skip_block(St, "@EndConfig");
        }
        else if (strncasecmp(p, "@SetAutoexec", 12) == 0) {
            inst_cmd_skip_block(St, "@EndAutoexec");
        }
        else if (strncasecmp(p, "@Finish", 7) == 0) {
            inst_cmd_skip_block(St, "@EndFinish");
        }

        /* v1.10.3 additions: filesystem state */
        else if (strncasecmp(p, "@ChDrive", 8) == 0) {
            inst_cmd_chdrive(St, p);
        }
        else if (strncasecmp(p, "@ChDir", 6) == 0) {
            inst_cmd_chdir(St, p);
        }

        /* Top-level bare-var assignment (v1.10.2 fallback):
         * @VarName = expr  outside of any @DefineVars block. */
        else if (*p == '@' && isalpha(p[1])) {
            const char *eq = strchr(p, '=');
            if (eq && *(eq + 1) != '=' && eq > p + 1) {
                char prev = *(eq - 1);
                /* Not a comparison; treat as assignment */
                if (prev != '!' && prev != '>' && prev != '<' &&
                    prev != '=') {
                    inst_cmd_set(St, p);
                }
            }
        }

        /* Silently ignore other lines (labels handled by pre-scan pass) */
    }

    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                             Main Entry                                    */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

static void usage(const char *ProgName)
{
    printf("Usage: %s [options] [INSTALL.DAT]\n", ProgName);
    printf("\n");
    printf("Options:\n");
    printf("  -a, --archives DIR   Directory containing .RED archives\n");
    printf("                       (default: current directory)\n");
    printf("  -t, --target DIR     Target root for installed files\n");
    printf("                       (default: current directory)\n");
    printf("  -r, --redx PATH      Path to redx binary\n");
    printf("                       (default: redx in PATH)\n");
    printf("  -g, --groups STR     Selected install-group letters, e.g. \"ab\"\n");
    printf("                       for First-Time + Upgrade. Default: \"abcdef\".\n");
    printf("                       (Interactive @GetGroups menu lands v1.10.4;\n");
    printf("                        until then this arg simulates the selection.)\n");
    printf("  -h, --help           This message\n");
    printf("\n");
    printf("If INSTALL.DAT is not given, defaults to ./INSTALL.DAT\n");
}


int main(int Argc, char *Argv[])
{
    InstState St;                       /* installer state                */
    const char *DatFile = "INSTALL.DAT";/* script file path               */
    int Rc;                             /* process return code            */
    int i;                              /* arg scan index                 */

    memset(&St, 0, sizeof(St));
    St.OutDrive     = 'C';
    St.InDrive      = 'A';
    St.WorkingDrive = 'C';
    strcpy(St.WorkingDir, "");
    St.AskOverwrite = 1;
    strcpy(St.ArchivesDir, ".");
    strcpy(St.TargetRoot,  ".");
    strcpy(St.RedxPath,    "redx");
    strcpy(St.SelectedGroups,      "abcdef");   /* default: all v1.10.4 checkboxes selected */

    /* Parse args */
    for (i = 1; i < Argc; i++) {
        if (strcmp(Argv[i], "-h") == 0 || strcmp(Argv[i], "--help") == 0) {
            usage(Argv[0]);
            return 0;
        }
        else if ((strcmp(Argv[i], "-a") == 0 ||
                  strcmp(Argv[i], "--archives") == 0) && i + 1 < Argc) {
            strncpy(St.ArchivesDir, Argv[++i], sizeof(St.ArchivesDir) - 1);
        }
        else if ((strcmp(Argv[i], "-t") == 0 ||
                  strcmp(Argv[i], "--target") == 0) && i + 1 < Argc) {
            strncpy(St.TargetRoot, Argv[++i], sizeof(St.TargetRoot) - 1);
        }
        else if ((strcmp(Argv[i], "-r") == 0 ||
                  strcmp(Argv[i], "--redx") == 0) && i + 1 < Argc) {
            strncpy(St.RedxPath, Argv[++i], sizeof(St.RedxPath) - 1);
        }
        else if ((strcmp(Argv[i], "-g") == 0 ||
                  strcmp(Argv[i], "--groups") == 0) && i + 1 < Argc) {
            strncpy(St.SelectedGroups, Argv[++i], sizeof(St.SelectedGroups) - 1);
        }
        else if (Argv[i][0] != '-') {
            DatFile = Argv[i];
        }
    }

    St.ScriptFp = fopen(DatFile, "r");
    if (!St.ScriptFp) {
        printf("Unable to open script file \"%s\"\n", DatFile);
        printf("The installation process cannot continue.\n");
        return 1;
    }

    printf("\n PCBoard Installation Program (install v1.10.4)\n");
    printf(" Script:    %s\n", DatFile);
    printf(" Archives:  %s\n", St.ArchivesDir);
    printf(" Target:    %s\n", St.TargetRoot);
    printf(" Redx CLI:  %s\n", St.RedxPath);
    printf(" Groups:    %s\n\n", St.SelectedGroups);

    /* v1.10.2: pre-scan for @Goto labels */
    inst_scan_labels(&St);

    Rc = inst_process(&St);

    /* Ensure any stray extract dir is cleaned up */
    if (St.ExtractDir[0]) inst_remove_temp_dir(&St);

    fclose(St.ScriptFp);

    printf("\n Files placed:  %ld\n", St.FilesPlaced);
    if (St.FilesFailed > 0)
        printf(" Files failed:  %ld\n", St.FilesFailed);
    printf(" Labels found:  %d\n", St.NumLabelDefs);

    if (Rc == 0)
        printf("\nInstallation complete.\n");

    return Rc;
}
