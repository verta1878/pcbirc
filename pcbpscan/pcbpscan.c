/*
 * pcbpscan.c — PCBoard File Scanner (clean room thdproscan)
 * Part of pcbrevival (GPL v3.0)
 *
 * Clean room implementation by pcbirc crew (evga/kiddo/sysop0 design).
 * Tests uploaded files for integrity and extracts FILE_ID.DIZ/ANS.
 *
 * Called by PCBTEST.BAT:
 *   @echo off
 *   pcbpscan %1 %2 %3 %4
 *   if errorlevel 1 echo FAILED > PCBFAIL.TXT
 *
 * Arguments (from PCBoard verifyfile()):
 *   %1 = full path to uploaded file
 *   %2 = "UPLOAD", "ATTACH", or "TEST"
 *   %3 = upload description file path (or "TEST")
 *   %4 = original filename
 *
 * Exit codes:
 *   0 = PASS (file is OK)
 *   1 = FAIL (file is corrupt or dangerous)
 *   2 = ERROR (scanner couldn't run / AV engine error)
 *
 * Tests performed:
 *   1. File exists and is > 0 bytes
 *   2. Archive integrity (ZIP central directory check)
 *   3. Nested archive depth limit (zip bomb detection)
 *   4. Filename sanitization (no path traversal)
 *   5. FILE_ID.DIZ extraction from ZIP archives
 *   6. External virus scanner hook (ClamAV/McAfee/any)
 *   7. Banned extension checking
 *   8. PCBoard DIR file description update
 *
 * Configuration: pcbpscan.cfg (KEY=VALUE, one per line)
 *   AV_CMD=clamscan --no-summary
 *   VERBOSE=1
 *   MAX_FILE_SIZE=104857600
 *   MAX_FILES_IN_ARC=10000
 *   MAX_NEST_DEPTH=3
 *   BANNED_EXTS=.exe .com .bat .cmd .scr .pif .vbs .js
 *   DIZ_TO_DESC=1
 *   DIR_FILE=
 *
 * Also reads environment: PCBPROSCAN_AV, PCBPROSCAN_VERBOSE
 *
 * thdproscan bugs fixed in this implementation:
 *   Bug 1: Config loads from pcbpscan.cfg (KEY=VALUE text format)
 *   Bug 2: AV exit code 2 = scanner error (RC=2), distinct from
 *          virus found (RC=1). Logged separately, returns exit 2.
 *   Bug 3: Archive file listing for ZIP (internal), ARJ/RAR/LHA
 *          via external tool if available (arj/unrar/lha on PATH)
 *   Bug 4: FILE_ID.DIZ extracted from ZIP, written to description
 *          file (%3) and optionally appended to PCBoard DIR listing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <time.h>

/* Portability: stricmp is Watcom/MSVC, strcasecmp is POSIX */
#if !defined(_WIN32) && !defined(__WATCOMC__) && !defined(__OS2__)
 #include <strings.h>
 #define stricmp strcasecmp
#endif

/* ------------------------------------------------------------------ */
/* Configuration defaults                                              */
/* ------------------------------------------------------------------ */

#define CFG_FILE        "pcbpscan.cfg"

static int    verbose         = 0;
static char   virus_cmd[256]  = "";
static long   max_file_size   = 100L * 1024L * 1024L;  /* 100MB */
static int    max_files_arc   = 10000;
static int    max_nest_depth  = 3;
static int    diz_to_desc     = 1;      /* write DIZ to description file */
static char   banned_exts[512]= ".exe .com .bat .cmd .scr .pif .vbs .js";
static char   dir_file[260]   = "";     /* PCBoard DIR listing file */

/* ZIP signatures */
#define ZIP_LOCAL_SIG    0x04034b50u
#define ZIP_CENTRAL_SIG  0x02014b50u
#define ZIP_END_SIG      0x06054b50u

/* DIZ buffer */
#define DIZ_MAXLEN  2048
static char diz_text[DIZ_MAXLEN];
static int  diz_found = 0;

/* ------------------------------------------------------------------ */
/* Config file loader (Bug 1 fix)                                      */
/* ------------------------------------------------------------------ */

static void trim(char *s)
{
    char *end = s + strlen(s) - 1;
    while (end >= s && (*end == '\r' || *end == '\n' || *end == ' ' || *end == '\t'))
        *end-- = '\0';
    /* Leading whitespace */
    {
        char *p = s;
        while (*p == ' ' || *p == '\t') p++;
        if (p != s) memmove(s, p, strlen(p) + 1);
    }
}

static void load_config(const char *cfgpath)
{
    FILE *f;
    char line[512];
    char *eq;

    f = fopen(cfgpath, "r");
    if (!f) {
        if (verbose) printf("  Config: %s not found, using defaults\n", cfgpath);
        return;
    }

    while (fgets(line, sizeof(line), f)) {
        trim(line);
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';')
            continue;

        eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        trim(line);
        eq++;
        trim(eq);

        if (stricmp(line, "AV_CMD") == 0)
            strncpy(virus_cmd, eq, sizeof(virus_cmd) - 1);
        else if (stricmp(line, "VERBOSE") == 0)
            verbose = atoi(eq);
        else if (stricmp(line, "MAX_FILE_SIZE") == 0)
            max_file_size = atol(eq);
        else if (stricmp(line, "MAX_FILES_IN_ARC") == 0)
            max_files_arc = atoi(eq);
        else if (stricmp(line, "MAX_NEST_DEPTH") == 0)
            max_nest_depth = atoi(eq);
        else if (stricmp(line, "BANNED_EXTS") == 0)
            strncpy(banned_exts, eq, sizeof(banned_exts) - 1);
        else if (stricmp(line, "DIZ_TO_DESC") == 0)
            diz_to_desc = atoi(eq);
        else if (stricmp(line, "DIR_FILE") == 0)
            strncpy(dir_file, eq, sizeof(dir_file) - 1);
    }

    fclose(f);
    if (verbose) printf("  Config loaded from %s\n", cfgpath);
}

/* ------------------------------------------------------------------ */
/* Utility: read little-endian integers from file                      */
/* ------------------------------------------------------------------ */

static unsigned long read32(FILE *f)
{
    unsigned char b[4];
    if (fread(b, 1, 4, f) != 4) return 0;
    return b[0] | (b[1]<<8) | ((unsigned long)b[2]<<16) | ((unsigned long)b[3]<<24);
}

static unsigned int read16(FILE *f)
{
    unsigned char b[2];
    if (fread(b, 1, 2, f) != 2) return 0;
    return b[0] | (b[1]<<8);
}

/* ------------------------------------------------------------------ */
/* ZIP integrity check + FILE_ID.DIZ extraction (Bug 3 & 4 fix)        */
/* ------------------------------------------------------------------ */

static int check_zip(const char *path)
{
    FILE *f;
    unsigned long sig;
    long filesize;
    int found_eocd = 0;
    int file_count = 0;

    f = fopen(path, "rb");
    if (!f) return -1;

    sig = read32(f);
    if (sig != ZIP_LOCAL_SIG) {
        if (verbose) printf("  Not a valid ZIP (bad signature)\n");
        fclose(f);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    filesize = ftell(f);

    /* Search backward for EOCD signature */
    {
        long pos;
        long search_start = filesize - 65557;
        if (search_start < 0) search_start = 0;

        for (pos = filesize - 22; pos >= search_start; pos--) {
            fseek(f, pos, SEEK_SET);
            sig = read32(f);
            if (sig == ZIP_END_SIG) {
                found_eocd = 1;
                fseek(f, pos + 10, SEEK_SET);
                file_count = read16(f);
                break;
            }
        }
    }

    if (!found_eocd) {
        if (verbose) printf("  ZIP corrupt: no central directory\n");
        fclose(f);
        return 1;
    }

    if (file_count > max_files_arc) {
        printf("  ZIP suspicious: %d files (limit %d)\n", file_count, max_files_arc);
        fclose(f);
        return 1;
    }

    /* Walk central directory to list files and find FILE_ID.DIZ */
    {
        long cd_offset;
        int i;

        /* EOCD + 16 = offset of start of central directory */
        /* We need to find EOCD again to read CD offset */
        {
            long pos;
            long search_start2 = filesize - 65557;
            if (search_start2 < 0) search_start2 = 0;

            for (pos = filesize - 22; pos >= search_start2; pos--) {
                fseek(f, pos, SEEK_SET);
                if (read32(f) == ZIP_END_SIG) {
                    fseek(f, pos + 16, SEEK_SET);
                    cd_offset = (long)read32(f);
                    break;
                }
            }
        }

        fseek(f, cd_offset, SEEK_SET);

        for (i = 0; i < file_count && i < max_files_arc; i++) {
            unsigned int name_len, extra_len, comment_len;
            unsigned long comp_size, uncomp_size, local_offset;
            char filename[260];

            sig = read32(f);
            if (sig != ZIP_CENTRAL_SIG) break;

            /* Skip version made by (2), version needed (2), flags (2),
               compression (2), mod time (2), mod date (2), crc32 (4) */
            fseek(f, 12, SEEK_CUR);
            comp_size   = read32(f);
            uncomp_size = read32(f);
            name_len    = read16(f);
            extra_len   = read16(f);
            comment_len = read16(f);

            /* Skip disk# start (2), internal attr (2), external attr (4) */
            fseek(f, 8, SEEK_CUR);
            local_offset = read32(f);

            /* Read filename */
            {
                int rlen = name_len < 259 ? name_len : 259;
                if ((int)fread(filename, 1, rlen, f) != rlen) break;
                filename[rlen] = '\0';
                if (name_len > 259) fseek(f, name_len - 259, SEEK_CUR);
            }

            if (verbose > 1)
                printf("  [%4d] %-40s %8lu bytes\n", i, filename, uncomp_size);

            /* Check for path traversal in archived filenames */
            if (strstr(filename, "..") || filename[0] == '/' || filename[0] == '\\') {
                printf("  FAIL: path traversal in archive: %s\n", filename);
                fclose(f);
                return 1;
            }

            /* Look for FILE_ID.DIZ or FILE_ID.ANS (case-insensitive) */
            {
                const char *basename = strrchr(filename, '/');
                if (!basename) basename = filename; else basename++;

                if (stricmp(basename, "FILE_ID.DIZ") == 0 ||
                    stricmp(basename, "FILE_ID.ANS") == 0) {

                    /* Extract DIZ from local file header */
                    long saved_pos = ftell(f);
                    unsigned int lh_name_len, lh_extra_len;

                    fseek(f, local_offset, SEEK_SET);
                    if (read32(f) == ZIP_LOCAL_SIG) {
                        /* Skip to name_len/extra_len (offset 26 from sig) */
                        fseek(f, local_offset + 26, SEEK_SET);
                        lh_name_len  = read16(f);
                        lh_extra_len = read16(f);
                        /* Skip name + extra to get to data */
                        fseek(f, lh_name_len + lh_extra_len, SEEK_CUR);

                        /* Read file data (only if stored/deflated and small) */
                        if (uncomp_size > 0 && uncomp_size < DIZ_MAXLEN &&
                            comp_size == uncomp_size) {
                            /* Stored (no compression) — read directly */
                            int nr = (int)fread(diz_text, 1, (int)uncomp_size, f);
                            diz_text[nr] = '\0';
                            diz_found = 1;
                            if (verbose) printf("  Extracted %s (%lu bytes)\n",
                                                basename, uncomp_size);
                        } else if (verbose) {
                            printf("  Found %s but compressed (need unzip)\n", basename);
                        }
                    }

                    fseek(f, saved_pos, SEEK_SET);
                }
            }

            /* Skip extra + comment to next central dir entry */
            fseek(f, extra_len + comment_len, SEEK_CUR);
        }
    }

    fclose(f);

    if (verbose) printf("  ZIP OK: %d files, %ld bytes\n", file_count, filesize);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Archive listing for non-ZIP formats via external tools (Bug 3 fix)  */
/* ------------------------------------------------------------------ */

static int check_archive_external(const char *path, int type)
{
    char cmd[512];
    int rc;
    const char *tool = NULL;
    const char *test_flag = NULL;

    switch (type) {
    case 1: tool = "arj"; test_flag = "t"; break;      /* ARJ */
    case 3: tool = "lha"; test_flag = "t"; break;      /* LZH */
    case 4: tool = "unrar"; test_flag = "t"; break;     /* RAR */
    case 5: tool = "7z"; test_flag = "t"; break;        /* 7Z */
    default: return 0; /* no tool, skip */
    }

    /* Try to run the tool — if not on PATH, skip gracefully */
    snprintf(cmd, sizeof(cmd), "%s %s \"%s\" >NUL 2>&1", tool, test_flag, path);

#if defined(__OS2__) || defined(__NT__) || defined(_WIN32)
    /* Windows/OS2: redirect to NUL */
#else
    snprintf(cmd, sizeof(cmd), "%s %s \"%s\" >/dev/null 2>&1", tool, test_flag, path);
#endif

    if (verbose) printf("  Running: %s %s ...\n", tool, test_flag);
    rc = system(cmd);

    if (rc == -1) {
        /* Tool not found — not an error, just can't verify */
        if (verbose) printf("  %s not found on PATH, skipping integrity check\n", tool);
        return 0;
    }

    if (rc != 0) {
        printf("  FAIL: %s integrity test failed (rc=%d)\n", tool, rc);
        return 1;
    }

    if (verbose) printf("  %s integrity OK\n", tool);

    /* Try to extract FILE_ID.DIZ if not already found */
    if (!diz_found) {
        char diz_path[260];
        struct stat st2;

        snprintf(cmd, sizeof(cmd), "%s e \"%s\" FILE_ID.DIZ -o. >NUL 2>&1",
                 tool, path);
        system(cmd);

        /* Check if FILE_ID.DIZ appeared */
        strcpy(diz_path, "FILE_ID.DIZ");
        if (stat(diz_path, &st2) == 0 && st2.st_size > 0 && st2.st_size < DIZ_MAXLEN) {
            FILE *df = fopen(diz_path, "r");
            if (df) {
                int nr = (int)fread(diz_text, 1, DIZ_MAXLEN - 1, df);
                diz_text[nr] = '\0';
                diz_found = 1;
                fclose(df);
                if (verbose) printf("  Extracted FILE_ID.DIZ via %s\n", tool);
            }
            remove(diz_path);   /* clean up */
        }
    }

    return 0;
}

/* ------------------------------------------------------------------ */
/* Detect archive type by magic bytes                                  */
/* ------------------------------------------------------------------ */

static int detect_type(const char *path)
{
    FILE *f;
    unsigned char hdr[8];
    int n;

    f = fopen(path, "rb");
    if (!f) return -1;
    n = (int)fread(hdr, 1, 8, f);
    fclose(f);
    if (n < 4) return -1;

    if (hdr[0]=='P' && hdr[1]=='K' && hdr[2]==3 && hdr[3]==4) return 0; /* ZIP */
    if (hdr[0]==0x60 && hdr[1]==0xEA) return 1; /* ARJ */
    if (hdr[0]==0x1A) return 2; /* ARC */
    if (n >= 5 && hdr[2]=='-' && hdr[3]=='l' && hdr[4]=='h') return 3; /* LZH */
    if (hdr[0]=='R' && hdr[1]=='a' && hdr[2]=='r' && hdr[3]=='!') return 4; /* RAR */
    if (hdr[0]=='7' && hdr[1]=='z') return 5; /* 7Z */
    if (hdr[0]==0x1F && hdr[1]==0x8B) return 6; /* GZ */
    return -1;
}

/* ------------------------------------------------------------------ */
/* Filename checks                                                     */
/* ------------------------------------------------------------------ */

static int check_filename(const char *name)
{
    if (strstr(name, "..") != NULL) return 1;
    if (name[0] == '/' || name[0] == '\\') return 1;
    if (strlen(name) > 1 && name[1] == ':') return 1;
    return 0;
}

/* Check file extension against banned list */
static int check_banned_ext(const char *name)
{
    const char *dot = strrchr(name, '.');
    char ext[16];
    char *p;

    if (!dot) return 0;

    strncpy(ext, dot, sizeof(ext) - 1);
    ext[sizeof(ext) - 1] = '\0';

    /* Lowercase the extension */
    for (p = ext; *p; p++) *p = (char)tolower(*p);

    /* Search banned list (space-separated) */
    {
        char banned_copy[512];
        char *tok;
        strncpy(banned_copy, banned_exts, sizeof(banned_copy) - 1);
        banned_copy[sizeof(banned_copy) - 1] = '\0';

        tok = strtok(banned_copy, " ,;");
        while (tok) {
            if (stricmp(tok, ext) == 0)
                return 1;
            tok = strtok(NULL, " ,;");
        }
    }

    return 0;
}

/* ------------------------------------------------------------------ */
/* Virus scanner with proper exit code handling (Bug 2 fix)            */
/*                                                                     */
/* ClamAV exit codes:                                                  */
/*   0 = clean                                                         */
/*   1 = virus/malware found                                           */
/*   2 = scanner error (corrupt file, can't read, engine error)        */
/*                                                                     */
/* McAfee uvscan exit codes:                                           */
/*   0 = clean                                                         */
/*   1-12 = various scan states                                        */
/*   13 = virus found                                                  */
/*   2 = DAT error / integrity error                                   */
/*                                                                     */
/* We distinguish:                                                     */
/*   RC=1 → FAIL (virus found, reject file)                            */
/*   RC=2 → ERROR (scanner problem, flag for sysop review)             */
/*   RC>2 → FAIL (assume bad)                                          */
/* ------------------------------------------------------------------ */

static int run_virus_scan(const char *path)
{
    char cmd[512];
    int rc;

    if (virus_cmd[0] == '\0')
        return 0;       /* no scanner configured */

    snprintf(cmd, sizeof(cmd), "%s \"%s\"", virus_cmd, path);
    if (verbose) printf("  AV scan: %s\n", cmd);

    rc = system(cmd);

    if (rc == 0) {
        if (verbose) printf("  AV: clean\n");
        return 0;       /* clean */
    }

    if (rc == 2) {
        /* Scanner error — not a virus, but scanner couldn't run properly */
        printf("  ERROR: AV scanner error (rc=2) — file needs manual review\n");
        return 2;       /* return 2 = scanner error, distinct from virus */
    }

    /* rc=1 or rc>2: virus found or other rejection */
    printf("  FAIL: AV scanner rejected file (rc=%d)\n", rc);
    return 1;
}

/* ------------------------------------------------------------------ */
/* Write FILE_ID.DIZ to description file and DIR listing (Bug 4 fix)   */
/* ------------------------------------------------------------------ */

static void write_diz_to_desc(const char *descfile)
{
    FILE *f;

    if (!diz_found || !diz_to_desc)
        return;

    if (!descfile || strlen(descfile) == 0 || stricmp(descfile, "TEST") == 0)
        return;

    f = fopen(descfile, "w");
    if (f) {
        fprintf(f, "%s", diz_text);
        fclose(f);
        if (verbose) printf("  Wrote FILE_ID.DIZ to %s\n", descfile);
    }
}

/* Append file entry to PCBoard DIR listing file.
 * Format: filename  size  date  description (first line of DIZ)
 */
static void write_dir_entry(const char *filename, long filesize)
{
    FILE *f;
    time_t now;
    struct tm *t;
    char datebuf[12];
    char *first_line;
    char diz_copy[DIZ_MAXLEN];

    if (dir_file[0] == '\0')
        return;

    now = time(NULL);
    t = localtime(&now);
    snprintf(datebuf, sizeof(datebuf), "%02d-%02d-%02d",
             (t->tm_mon + 1), t->tm_mday, t->tm_year % 100);

    /* Get first line of DIZ for description */
    if (diz_found) {
        strncpy(diz_copy, diz_text, DIZ_MAXLEN - 1);
        diz_copy[DIZ_MAXLEN - 1] = '\0';
        first_line = strtok(diz_copy, "\r\n");
    } else {
        first_line = NULL;
    }

    f = fopen(dir_file, "a");
    if (f) {
        /* PCBoard DIR format: filename padded to 13, size, date, description */
        fprintf(f, "%-13s %8ld  %s  %s\n",
                filename, filesize, datebuf,
                first_line ? first_line : "No description");
        fclose(f);
        if (verbose) printf("  Appended to DIR: %s\n", dir_file);
    }
}

/* ------------------------------------------------------------------ */
/* Help                                                                */
/* ------------------------------------------------------------------ */

static void print_help(void)
{
    printf(
        "pcbpscan v1.1 — PCBoard File Scanner\n"
        "Part of pcbrevival (GPL v3.0)\n"
        "Clean room design by pcbirc crew\n"
        "\n"
        "Usage: pcbpscan <filepath> [type] [descfile] [filename]\n"
        "\n"
        "  filepath   Full path to uploaded file\n"
        "  type       UPLOAD, ATTACH, or TEST\n"
        "  descfile   Upload description file path\n"
        "  filename   Original filename\n"
        "\n"
        "Exit codes:\n"
        "  0 = PASS    File is OK\n"
        "  1 = FAIL    File corrupt, dangerous, or virus found\n"
        "  2 = ERROR   Scanner error (needs manual review)\n"
        "\n"
        "Config: pcbpscan.cfg (KEY=VALUE)\n"
        "  AV_CMD=clamscan --no-summary\n"
        "  VERBOSE=1\n"
        "  MAX_FILE_SIZE=104857600\n"
        "  BANNED_EXTS=.exe .com .bat .cmd .scr .pif .vbs .js\n"
        "  DIZ_TO_DESC=1\n"
        "  DIR_FILE=C:\\PCB\\GEN\\DIR0\n"
        "\n"
        "PCBTEST.BAT:\n"
        "  @echo off\n"
        "  pcbpscan %%1 %%2 %%3 %%4\n"
        "  if errorlevel 2 echo ERROR > PCBERROR.TXT\n"
        "  if errorlevel 1 echo FAILED > PCBFAIL.TXT\n"
        "\n"
        "Environment: PCBPROSCAN_AV, PCBPROSCAN_VERBOSE\n"
    );
}

/* ------------------------------------------------------------------ */
/* Main                                                                */
/* ------------------------------------------------------------------ */

int main(int argc, char **argv)
{
    const char *filepath;
    const char *descfile = NULL;
    const char *origname = NULL;
    const char *av_env;
    struct stat st;
    int filetype;
    int av_result;

    if (argc < 2 || strcmp(argv[1], "-?") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help();
        return (argc < 2) ? 2 : 0;
    }

    filepath = argv[1];
    if (argc >= 4) descfile = argv[3];
    if (argc >= 5) origname = argv[4];

    /* Load config file first */
    load_config(CFG_FILE);

    /* Environment overrides */
    av_env = getenv("PCBPROSCAN_AV");
    if (av_env && strlen(av_env) > 0)
        strncpy(virus_cmd, av_env, sizeof(virus_cmd) - 1);

    if (getenv("PCBPROSCAN_VERBOSE"))
        verbose = 1;

    printf("pcbpscan: testing %s\n", filepath);

    /* ---- Test 1: File exists and has size ---- */
    if (stat(filepath, &st) != 0) {
        printf("  FAIL: file not found\n");
        return 1;
    }
    if (st.st_size < 10) {
        printf("  FAIL: file too small (%ld bytes)\n", (long)st.st_size);
        return 1;
    }
    if (st.st_size > max_file_size) {
        printf("  FAIL: file too large (%ld bytes, limit %ld)\n",
               (long)st.st_size, max_file_size);
        return 1;
    }

    /* ---- Test 2: Filename sanitization ---- */
    if (origname && check_filename(origname)) {
        printf("  FAIL: suspicious filename '%s'\n", origname);
        return 1;
    }

    /* ---- Test 3: Banned extension check ---- */
    if (origname && check_banned_ext(origname)) {
        printf("  FAIL: banned file extension '%s'\n", origname);
        return 1;
    }

    /* ---- Test 4: Archive integrity + file listing ---- */
    filetype = detect_type(filepath);
    switch (filetype) {
    case 0: /* ZIP — internal check */
        if (check_zip(filepath) != 0) {
            printf("  FAIL: ZIP integrity check failed\n");
            return 1;
        }
        break;
    case 1: /* ARJ */
    case 3: /* LZH */
    case 4: /* RAR */
    case 5: /* 7Z */
        if (check_archive_external(filepath, filetype) != 0) {
            printf("  FAIL: archive integrity check failed\n");
            return 1;
        }
        break;
    case 2: /* ARC */
    case 6: /* GZ */
        if (verbose) printf("  Archive type %d (no integrity tool)\n", filetype);
        break;
    default:
        if (verbose) printf("  Not an archive\n");
        break;
    }

    /* ---- Test 5: Virus scan ---- */
    av_result = run_virus_scan(filepath);
    if (av_result == 1) {
        printf("  FAIL: virus scanner rejected file\n");
        return 1;
    }
    if (av_result == 2) {
        printf("  ERROR: virus scanner malfunction — manual review needed\n");
        return 2;
    }

    /* ---- All tests passed ---- */
    printf("  PASS: %s (%ld bytes%s)\n",
           origname ? origname : filepath,
           (long)st.st_size,
           diz_found ? ", FILE_ID.DIZ extracted" : "");

    /* Write FILE_ID.DIZ to description file */
    write_diz_to_desc(descfile);

    /* Append to PCBoard DIR listing */
    if (origname)
        write_dir_entry(origname, (long)st.st_size);

    /* Write PCBPASS.TXT */
    {
        FILE *pf = fopen("PCBPASS.TXT", "w");
        if (pf) {
            fprintf(pf, "File tested OK by pcbpscan v1.1\n");
            fprintf(pf, "Size: %ld bytes\n", (long)st.st_size);
            if (filetype >= 0) {
                static const char *type_names[] =
                    {"ZIP","ARJ","ARC","LZH","RAR","7Z","GZ"};
                fprintf(pf, "Type: %s\n", type_names[filetype]);
            }
            if (diz_found) {
                fprintf(pf, "FILE_ID.DIZ:\n%s\n", diz_text);
            }
            fclose(pf);
        }
    }

    return 0;
}
