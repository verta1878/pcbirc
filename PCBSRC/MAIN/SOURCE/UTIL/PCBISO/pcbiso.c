/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* PCBISO.C — ISO/CD-ROM File Area Indexer for PCBoard                      */
/*                                                                           */
/* Scans source paths and generates PCBoard DIR listing text files.          */
/* Uses 64 reserved bytes in CNAMES.ADD for per-conference ISO config.      */
/* No PCBOARD.DAT changes. Works on 15.4+.                                 */
/*                                                                           */
/* Author: hexadecimal                                                       */
/* License: GPLv3 (pcbrevival project)                                       */
/* pcbrevival Phase 5 — works on PCBoard 15.4+                                          */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "pcbiso.h"

#ifdef __WATCOMC__
#include <direct.h>
#include <dos.h>
#include <io.h>
#define PATH_SEP '\\'
#else
#include <dirent.h>
#include <unistd.h>
#define PATH_SEP '/'
#endif

/*-----------------------------------------------------------------------*/
/* stricmp for non-Watcom                                                 */
/*-----------------------------------------------------------------------*/

#ifndef __WATCOMC__
static int stricmp(const char *a, const char *b)
{
    while (*a && *b) {
        int ca = (*a >= 'A' && *a <= 'Z') ? *a + 32 : *a;
        int cb = (*b >= 'A' && *b <= 'Z') ? *b + 32 : *b;
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return *a - *b;
}
#endif

/*-----------------------------------------------------------------------*/
/* Trim trailing spaces from a fixed-length char field                    */
/*-----------------------------------------------------------------------*/

static void trimfield(const char *src, char *dst, int maxlen)
{
    int i;
    memcpy(dst, src, maxlen);
    dst[maxlen] = '\0';
    for (i = maxlen - 1; i >= 0 && (dst[i] == ' ' || dst[i] == '\0'); i--)
        dst[i] = '\0';
}

/*-----------------------------------------------------------------------*/
/* DIR listing writer                                                     */
/*-----------------------------------------------------------------------*/

static void write_dir_entry(FILE *fp, const char *filename, long filesize,
                            struct tm *ft, const char *desc)
{
    char datebuf[12];
    snprintf(datebuf, sizeof(datebuf), "%02d-%02d-%02d",
             ft->tm_mon + 1, ft->tm_mday, ft->tm_year % 100);
    fprintf(fp, "%-12s %9ld  %s  %s\n", filename, filesize, datebuf,
            desc ? desc : "");
}

static void write_dir_entry_desc(FILE *fp, const char *filename, long filesize,
                                 struct tm *ft, const char *desc)
{
    const char *p;
    int first, len;
    char line[MAX_DESC + 1];

    if (!desc || !desc[0]) {
        write_dir_entry(fp, filename, filesize, ft, "");
        return;
    }
    first = 1;
    p = desc;
    while (*p) {
        len = 0;
        while (*p && *p != '\n' && len < MAX_DESC)
            line[len++] = *p++;
        line[len] = '\0';
        if (*p == '\n') p++;
        if (first) {
            write_dir_entry(fp, filename, filesize, ft, line);
            first = 0;
        } else {
            fprintf(fp, "%33s%s\n", "", line);
        }
    }
}

/*-----------------------------------------------------------------------*/
/* FILE_ID.DIZ extraction from ZIP                                        */
/*-----------------------------------------------------------------------*/

#define ZIP_LOCAL_SIG  0x04034B50

static int extract_file_id_diz(const char *zippath, char *desc, int maxlen)
{
    FILE *fp;
    unsigned char hdr[30];
    unsigned long sig;
    unsigned short namelen, extralen, compmethod;
    unsigned long compsize;
    char name[256];
    const char *basename;
    const char *sl;

    fp = fopen(zippath, "rb");
    if (!fp) return 0;
    desc[0] = '\0';

    while (!feof(fp)) {
        if (fread(hdr, 1, 30, fp) != 30) break;
        sig = hdr[0] | (hdr[1] << 8) | ((unsigned long)hdr[2] << 16) |
              ((unsigned long)hdr[3] << 24);
        if (sig != ZIP_LOCAL_SIG) break;

        compmethod = hdr[8] | (hdr[9] << 8);
        compsize = hdr[18] | (hdr[19] << 8) | ((unsigned long)hdr[20] << 16) |
                   ((unsigned long)hdr[21] << 24);
        namelen = hdr[26] | (hdr[27] << 8);
        extralen = hdr[28] | (hdr[29] << 8);

        if (namelen >= sizeof(name)) {
            fseek(fp, namelen + extralen + compsize, SEEK_CUR);
            continue;
        }
        if (fread(name, 1, namelen, fp) != namelen) break;
        name[namelen] = '\0';
        if (extralen > 0) fseek(fp, extralen, SEEK_CUR);

        /* Strip path from filename inside ZIP */
        basename = name;
        for (sl = name; *sl; sl++) {
            if (*sl == '/' || *sl == '\\') basename = sl + 1;
        }

        if (stricmp(basename, "FILE_ID.DIZ") == 0) {
            if (compmethod == 0 && compsize < (unsigned long)maxlen) {
                if (fread(desc, 1, (size_t)compsize, fp) == (size_t)compsize) {
                    desc[compsize] = '\0';
                    fclose(fp);
                    return 1;
                }
            }
            break;
        }
        fseek(fp, compsize, SEEK_CUR);
    }
    fclose(fp);
    return 0;
}

/*-----------------------------------------------------------------------*/
/* Directory scanner                                                      */
/*-----------------------------------------------------------------------*/

static int path_exists(const char *path)
{
#ifdef __WATCOMC__
    struct find_t ff;
    char pat[MAX_PATH_LEN];
    snprintf(pat, sizeof(pat), "%s\\*.*", path);
    if (_dos_findfirst(pat, _A_NORMAL | _A_SUBDIR, &ff) != 0) return 0;
    _dos_findclose(&ff);
    return 1;
#else
    DIR *d = opendir(path);
    if (!d) return 0;
    closedir(d);
    return 1;
#endif
}

static int scan_directory(const char *path, FILE *dirfp, int *count)
{
    char fullpath[MAX_PATH_LEN];
    struct stat st;
    struct tm *ft;
    char desc[1024];
    int len;

#ifdef __WATCOMC__
    struct find_t ff;
    char pattern[MAX_PATH_LEN];
    snprintf(pattern, sizeof(pattern), "%s\\*.*", path);
    if (_dos_findfirst(pattern, _A_NORMAL | _A_RDONLY | _A_ARCH, &ff) != 0)
        return 0;
    do {
        if (ff.name[0] == '.') continue;
        if (ff.attrib & (_A_SUBDIR | _A_HIDDEN | _A_SYSTEM)) continue;
        snprintf(fullpath, sizeof(fullpath), "%s\\%s", path, ff.name);
        if (stat(fullpath, &st) != 0) continue;
        ft = localtime(&st.st_mtime);
        desc[0] = '\0';
        len = strlen(ff.name);
        if (len > 4 && stricmp(ff.name + len - 4, ".ZIP") == 0)
            extract_file_id_diz(fullpath, desc, sizeof(desc));
        write_dir_entry_desc(dirfp, ff.name, st.st_size, ft, desc);
        (*count)++;
    } while (_dos_findnext(&ff) == 0);
    _dos_findclose(&ff);
#else
    DIR *d;
    struct dirent *ent;
    d = opendir(path);
    if (!d) return 0;
    while ((ent = readdir(d)) != NULL) {
        if (ent->d_name[0] == '.') continue;
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, ent->d_name);
        if (stat(fullpath, &st) != 0) continue;
        if (S_ISDIR(st.st_mode)) continue;
        ft = localtime(&st.st_mtime);
        desc[0] = '\0';
        len = strlen(ent->d_name);
        if (len > 4 && stricmp(ent->d_name + len - 4, ".zip") == 0)
            extract_file_id_diz(fullpath, desc, sizeof(desc));
        write_dir_entry_desc(dirfp, ent->d_name, st.st_size, ft, desc);
        (*count)++;
    }
    closedir(d);
#endif
    return 1;
}

/*-----------------------------------------------------------------------*/
/* CNAMES reader                                                          */
/*-----------------------------------------------------------------------*/

typedef struct {
    int  confnum;
    char name[15];        /* from oldconftype.Name */
    char name2[49];       /* from addconftype.Name2 */
    char dirnameloc[34];  /* path to DIR.LST */
    char pthnameloc[34];  /* path to DLPATH.LST */
    char reserved_raw[64]; /* raw Reserved[64] from CNAMES.ADD */
} conf_info;

static int read_conf_info(const char *cnames_old, const char *cnames_add,
                          conf_info *confs, int maxconfs)
{
    FILE *fp_old, *fp_add;
    oldconftype orec;
    addconftype arec;
    int count, i;

    fp_old = fopen(cnames_old, "rb");
    if (!fp_old) {
        fprintf(stderr, "PCBISO: Cannot open %s\n", cnames_old);
        return -1;
    }
    fp_add = fopen(cnames_add, "rb");
    if (!fp_add) {
        fprintf(stderr, "PCBISO: Cannot open %s\n", cnames_add);
        fclose(fp_old);
        return -1;
    }

    count = 0;
    for (i = 0; count < maxconfs; i++) {
        if (fread(&orec, sizeof(oldconftype), 1, fp_old) != 1) break;
        if (fread(&arec, sizeof(addconftype), 1, fp_add) != 1) break;

        /* Skip empty conferences (no name) */
        if (orec.Name[0] == '\0' || orec.Name[0] == ' ') continue;

        confs[count].confnum = i;
        trimfield(orec.Name, confs[count].name, 14);
        trimfield(arec.Name2, confs[count].name2, 48);
        trimfield(orec.DirNameLoc, confs[count].dirnameloc, 33);
        trimfield(orec.PthNameLoc, confs[count].pthnameloc, 33);
        memcpy(confs[count].reserved_raw, arec.FilebaseFlags, 64);
        count++;
    }

    fclose(fp_old);
    fclose(fp_add);
    return count;
}

/*-----------------------------------------------------------------------*/
/* Mount table (PCBISO.DAT)                                               */
/*-----------------------------------------------------------------------*/

static pcbiso_dat g_mounts;
static char g_dat_path[MAX_PATH_LEN] = "PCBISO.DAT";

static void mount_table_init(void)
{
    memset(&g_mounts, 0, sizeof(g_mounts));
    memcpy(g_mounts.sig, PCBISO_DAT_SIG, 8);
}

static int mount_table_load(void)
{
    FILE *fp;
    fp = fopen(g_dat_path, "rb");
    if (!fp) {
        mount_table_init();
        return 0;
    }
    if (fread(&g_mounts, sizeof(pcbiso_dat), 1, fp) != 1 ||
        memcmp(g_mounts.sig, PCBISO_DAT_SIG, 8) != 0) {
        fclose(fp);
        mount_table_init();
        return 0;
    }
    fclose(fp);
    return 1;
}

static int mount_table_save(void)
{
    FILE *fp;
    fp = fopen(g_dat_path, "wb");
    if (!fp) {
        fprintf(stderr, "PCBISO: Cannot write %s\n", g_dat_path);
        return 0;
    }
    fwrite(&g_mounts, sizeof(pcbiso_dat), 1, fp);
    fclose(fp);
    return 1;
}

static char mount_find_free_letter(void)
{
    char used[26];
    int i;
    memset(used, 0, sizeof(used));
    for (i = 0; i < MAX_MOUNT; i++) {
        if (g_mounts.mounts[i].drive >= 'A' && g_mounts.mounts[i].drive <= 'Z')
            used[g_mounts.mounts[i].drive - 'A'] = 1;
    }
    /* Start from E: (skip A-D for floppies and primary drives) */
    for (i = 4; i < 26; i++) {
        if (!used[i]) return (char)('A' + i);
    }
    return 0;
}

/*-----------------------------------------------------------------------*/
/* Commands                                                               */
/*-----------------------------------------------------------------------*/

static int cmd_index(const char *srcpath, const char *dirfile, int rebuild)
{
    FILE *fp;
    int count;

    if (!path_exists(srcpath)) {
        fprintf(stderr, "PCBISO: %s not found — path does not exist.\n", srcpath);
        return 1;
    }

    if (rebuild) {
        remove(dirfile);
        printf("PCBISO: Rebuilding %s from %s\n", dirfile, srcpath);
    } else {
        printf("PCBISO: Indexing %s to %s\n", srcpath, dirfile);
    }

    fp = fopen(dirfile, "w");
    if (!fp) {
        fprintf(stderr, "PCBISO: Cannot create %s\n", dirfile);
        return 1;
    }

    count = 0;
    scan_directory(srcpath, fp, &count);
    fclose(fp);
    printf("PCBISO: %d files indexed.\n", count);
    return 0;
}

static int cmd_list(const char *cnames_old, const char *cnames_add)
{
    conf_info confs[1024];
    int count, i;
    const char *type;

    count = read_conf_info(cnames_old, cnames_add, confs, 1024);
    if (count < 0) return 1;

    printf("Conf   Name                 DIR.LST                          Type\n");
    printf("-----  -------------------  -------------------------------  --------\n");

    for (i = 0; i < count; i++) {
        if (confs[i].dirnameloc[0] == '\0') continue;

        {
            pcb_conf_ext *ext = (pcb_conf_ext *)confs[i].reserved_raw;
            int has_iso = 0, j;
            for (j = 0; j < 64; j++) {
                if (ext->FilebaseFlags[j] != 0) { has_iso = 1; break; }
            }
            if (has_iso)
                type = "ISO";
            else if (confs[i].pthnameloc[0] != '\0')
                type = "Local";
            else
                type = "-";
        }

        printf("%-5d  %-19s  %-31s  %s\n",
               confs[i].confnum,
               confs[i].name2[0] ? confs[i].name2 : confs[i].name,
               confs[i].dirnameloc,
               type);
    }

    /* Also show mount table */
    mount_table_load();
    if (g_mounts.count > 0) {
        printf("\nMounted ISOs:\n");
        printf("Drive  ISO Path\n");
        printf("-----  --------\n");
        for (i = 0; i < MAX_MOUNT; i++) {
            if (g_mounts.mounts[i].drive != 0)
                printf("%c:     %s\n", g_mounts.mounts[i].drive,
                       g_mounts.mounts[i].isopath);
        }
    }
    return 0;
}

static int cmd_status(const char *cnames_old, const char *cnames_add)
{
    conf_info confs[1024];
    int count, i, files;
    const char *type;
    struct stat st;
    char timebuf[20];
    struct tm *ft;

    count = read_conf_info(cnames_old, cnames_add, confs, 1024);
    if (count < 0) return 1;

    printf("Conf   Name                 Files  Last Indexed      Status\n");
    printf("-----  -------------------  -----  ----------------  --------\n");

    for (i = 0; i < count; i++) {
        if (confs[i].dirnameloc[0] == '\0') continue;

        {
            pcb_conf_ext *ext = (pcb_conf_ext *)confs[i].reserved_raw;
            int has_iso = 0, j;
            for (j = 0; j < 64; j++) {
                if (ext->FilebaseFlags[j] != 0) { has_iso = 1; break; }
            }
            type = has_iso ? "ISO" : "Local";
        }

        /* Count files in DIR listing and get mod time */
        files = 0;
        timebuf[0] = '\0';
        if (stat(confs[i].dirnameloc, &st) == 0) {
            FILE *df = fopen(confs[i].dirnameloc, "r");
            if (df) {
                char line[256];
                while (fgets(line, sizeof(line), df)) {
                    if (line[0] != ' ' && line[0] != '\n' && line[0] != '\r')
                        files++;
                }
                fclose(df);
            }
            ft = localtime(&st.st_mtime);
            snprintf(timebuf, sizeof(timebuf), "%04d-%02d-%02d %02d:%02d",
                     ft->tm_year + 1900, ft->tm_mon + 1, ft->tm_mday,
                     ft->tm_hour, ft->tm_min);
        } else {
            snprintf(timebuf, sizeof(timebuf), "(not indexed)");
        }

        printf("%-5d  %-19s  %5d  %-16s  %s\n",
               confs[i].confnum,
               confs[i].name2[0] ? confs[i].name2 : confs[i].name,
               files, timebuf, type);
    }

    return 0;
}

static int cmd_mount(const char *isopath)
{
    char drive;
    int i, slot;
    struct stat st;

    if (stat(isopath, &st) != 0) {
        fprintf(stderr, "PCBISO: %s not found.\n", isopath);
        return 1;
    }

    mount_table_load();

    /* Check if already mounted */
    for (i = 0; i < MAX_MOUNT; i++) {
        if (g_mounts.mounts[i].drive != 0 &&
            stricmp(g_mounts.mounts[i].isopath, isopath) == 0) {
            printf("PCBISO: %s already mounted as %c:\n",
                   isopath, g_mounts.mounts[i].drive);
            return 0;
        }
    }

    drive = mount_find_free_letter();
    if (drive == 0) {
        fprintf(stderr, "PCBISO: No free drive letters (E-Z all in use).\n");
        return 1;
    }

    /* Find empty slot */
    slot = -1;
    for (i = 0; i < MAX_MOUNT; i++) {
        if (g_mounts.mounts[i].drive == 0) { slot = i; break; }
    }
    if (slot < 0) {
        fprintf(stderr, "PCBISO: Mount table full.\n");
        return 1;
    }

    g_mounts.mounts[slot].drive = drive;
    strncpy(g_mounts.mounts[slot].isopath, isopath, MAX_PATH_LEN - 1);
    g_mounts.mounts[slot].isopath[MAX_PATH_LEN - 1] = '\0';
    g_mounts.count++;

    mount_table_save();

    printf("PCBISO: Mounted %s as %c:\n", isopath, drive);
    printf("NOTE: Use your OS to mount the ISO to %c: then run PCBISO /INDEX %c:\n",
           drive, drive);
    return 0;
}

static int cmd_unmount(const char *drive_str)
{
    char drive;
    int i;

    if (!drive_str[0]) {
        fprintf(stderr, "Usage: PCBISO /UNMOUNT <drive_letter>\n");
        return 1;
    }

    drive = drive_str[0];
    if (drive >= 'a' && drive <= 'z') drive -= 32;

    mount_table_load();

    for (i = 0; i < MAX_MOUNT; i++) {
        if (g_mounts.mounts[i].drive == drive) {
            printf("PCBISO: Unmounted %c: (%s)\n", drive, g_mounts.mounts[i].isopath);
            g_mounts.mounts[i].drive = 0;
            g_mounts.mounts[i].isopath[0] = '\0';
            g_mounts.count--;
            mount_table_save();
            return 0;
        }
    }

    fprintf(stderr, "PCBISO: %c: is not in the mount table.\n", drive);
    return 1;
}

static int cmd_index_all(const char *cnames_old, const char *cnames_add)
{
    conf_info confs[1024];
    int count, i, total, errors;

    count = read_conf_info(cnames_old, cnames_add, confs, 1024);
    if (count < 0) return 1;

    total = 0;
    errors = 0;
    for (i = 0; i < count; i++) {
        {
            pcb_conf_ext *ext = (pcb_conf_ext *)confs[i].reserved_raw;
            int has_iso = 0, j;
            for (j = 0; j < 64; j++) {
                if (ext->FilebaseFlags[j] != 0) { has_iso = 1; break; }
            }
            if (!has_iso) continue;
        }
        if (confs[i].dirnameloc[0] == '\0' || confs[i].pthnameloc[0] == '\0') continue;

        if (cmd_index(confs[i].pthnameloc, confs[i].dirnameloc, 0) == 0)
            total++;
        else
            errors++;
    }

    printf("\nPCBISO: %d conferences indexed, %d errors.\n", total, errors);
    return errors > 0 ? 1 : 0;
}


/*-----------------------------------------------------------------------*/
/* /SETISO and /CLEARISO — set/clear ISO flag in CNAMES.ADD              */
/*-----------------------------------------------------------------------*/

static int cmd_setiso(const char *cnames_add, int fbnum, int confnum)
{
    FILE *fp;
    addconftype arec;
    long offset;
    pcb_conf_ext *ext;

    if (fbnum < 0 || fbnum > 511) {
        fprintf(stderr, "PCBISO: Filebase number must be 0-511.\n");
        return 1;
    }

    fp = fopen(cnames_add, "r+b");
    if (!fp) {
        fprintf(stderr, "PCBISO: Cannot open %s\n", cnames_add);
        return 1;
    }

    offset = (long)confnum * sizeof(addconftype);
    if (fseek(fp, offset, SEEK_SET) != 0) {
        fprintf(stderr, "PCBISO: Conference %d not found in %s\n", confnum, cnames_add);
        fclose(fp);
        return 1;
    }

    if (fread(&arec, sizeof(addconftype), 1, fp) != 1) {
        fprintf(stderr, "PCBISO: Conference %d not found in %s\n", confnum, cnames_add);
        fclose(fp);
        return 1;
    }

    ext = (pcb_conf_ext *)arec.FilebaseFlags;
    FB_SET_FLAG(ext, fbnum, FBFLAG_ISO);

    /* Write back */
    fseek(fp, offset, SEEK_SET);
    if (fwrite(&arec, sizeof(addconftype), 1, fp) != 1) {
        fprintf(stderr, "PCBISO: Write failed to %s\n", cnames_add);
        fclose(fp);
        return 1;
    }

    fclose(fp);
    printf("PCBISO: Filebase %d in conference %d marked as ISO-backed.\n", fbnum, confnum);
    return 0;
}

static int cmd_cleariso(const char *cnames_add, int fbnum, int confnum)
{
    FILE *fp;
    addconftype arec;
    long offset;
    pcb_conf_ext *ext;

    if (fbnum < 0 || fbnum > 511) {
        fprintf(stderr, "PCBISO: Filebase number must be 0-511.\n");
        return 1;
    }

    fp = fopen(cnames_add, "r+b");
    if (!fp) {
        fprintf(stderr, "PCBISO: Cannot open %s\n", cnames_add);
        return 1;
    }

    offset = (long)confnum * sizeof(addconftype);
    if (fseek(fp, offset, SEEK_SET) != 0) {
        fprintf(stderr, "PCBISO: Conference %d not found in %s\n", confnum, cnames_add);
        fclose(fp);
        return 1;
    }

    if (fread(&arec, sizeof(addconftype), 1, fp) != 1) {
        fprintf(stderr, "PCBISO: Conference %d not found in %s\n", confnum, cnames_add);
        fclose(fp);
        return 1;
    }

    ext = (pcb_conf_ext *)arec.FilebaseFlags;
    FB_CLR_FLAG(ext, fbnum, FBFLAG_ISO);

    fseek(fp, offset, SEEK_SET);
    if (fwrite(&arec, sizeof(addconftype), 1, fp) != 1) {
        fprintf(stderr, "PCBISO: Write failed to %s\n", cnames_add);
        fclose(fp);
        return 1;
    }

    fclose(fp);
    printf("PCBISO: Filebase %d in conference %d ISO flag cleared.\n", fbnum, confnum);
    return 0;
}

/*-----------------------------------------------------------------------*/
/* Usage                                                                  */
/*-----------------------------------------------------------------------*/

static void usage(void)
{
    printf("PCBISO v%s — ISO/CD-ROM File Area Indexer for PCBoard\n", PCBISO_VERSION);
    printf("pcbrevival Phase 5 — GPLv3\n\n");
    printf("Usage:\n");
    printf("  PCBISO /INDEX <source_path> <dir_file>    Index files to DIR listing\n");
    printf("  PCBISO /INDEX ALL <cnames> <cnamesadd>    Index all ISO-backed conferences\n");
    printf("  PCBISO /REBUILD <source_path> <dir_file>  Delete and rebuild DIR listing\n");
    printf("  PCBISO /REBUILD ALL <cnames> <cnamesadd>  Rebuild all ISO conferences\n");
    printf("  PCBISO /LIST <cnames> <cnamesadd>         List configured file areas\n");
    printf("  PCBISO /STATUS <cnames> <cnamesadd>       Detailed status of file areas\n");
    printf("  PCBISO /SETISO <conf> <fb> <cnamesadd>    Mark filebase as ISO-backed\n");
    printf("  PCBISO /CLEARISO <conf> <fb> <cnamesadd>  Clear ISO flag on filebase\n");
    printf("  PCBISO /MOUNT <iso_file>                  Register ISO in mount table\n");
    printf("  PCBISO /UNMOUNT <drive>                   Remove from mount table\n");
}

/*-----------------------------------------------------------------------*/
/* Main                                                                   */
/*-----------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    if (argc < 2) {
        usage();
        return 1;
    }

    if (stricmp(argv[1], "/INDEX") == 0) {
        if (argc >= 3 && stricmp(argv[2], "ALL") == 0) {
            if (argc != 5) {
                fprintf(stderr, "Usage: PCBISO /INDEX ALL <cnames> <cnamesadd>\n");
                return 1;
            }
            return cmd_index_all(argv[3], argv[4]);
        }
        if (argc != 4) {
            fprintf(stderr, "Usage: PCBISO /INDEX <source_path> <dir_file>\n");
            return 1;
        }
        return cmd_index(argv[2], argv[3], 0);

    } else if (stricmp(argv[1], "/REBUILD") == 0) {
        if (argc >= 3 && stricmp(argv[2], "ALL") == 0) {
            if (argc != 5) {
                fprintf(stderr, "Usage: PCBISO /REBUILD ALL <cnames> <cnamesadd>\n");
                return 1;
            }
            /* TODO: rebuild all */
            printf("PCBISO: /REBUILD ALL — not yet implemented\n");
            return 1;
        }
        if (argc != 4) {
            fprintf(stderr, "Usage: PCBISO /REBUILD <source_path> <dir_file>\n");
            return 1;
        }
        return cmd_index(argv[2], argv[3], 1);

    } else if (stricmp(argv[1], "/LIST") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: PCBISO /LIST <cnames> <cnamesadd>\n");
            return 1;
        }
        return cmd_list(argv[2], argv[3]);

    } else if (stricmp(argv[1], "/STATUS") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: PCBISO /STATUS <cnames> <cnamesadd>\n");
            return 1;
        }
        return cmd_status(argv[2], argv[3]);

    } else if (stricmp(argv[1], "/SETISO") == 0) {
        if (argc != 5) {
            fprintf(stderr, "Usage: PCBISO /SETISO <conf_num> <filebase_num> <cnamesadd>\n");
            return 1;
        }
        return cmd_setiso(argv[4], atoi(argv[3]), atoi(argv[2]));

    } else if (stricmp(argv[1], "/CLEARISO") == 0) {
        if (argc != 5) {
            fprintf(stderr, "Usage: PCBISO /CLEARISO <conf_num> <filebase_num> <cnamesadd>\n");
            return 1;
        }
        return cmd_cleariso(argv[4], atoi(argv[3]), atoi(argv[2]));

    } else if (stricmp(argv[1], "/MOUNT") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: PCBISO /MOUNT <iso_file>\n");
            return 1;
        }
        return cmd_mount(argv[2]);

    } else if (stricmp(argv[1], "/UNMOUNT") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: PCBISO /UNMOUNT <drive_letter>\n");
            return 1;
        }
        return cmd_unmount(argv[2]);

    } else {
        fprintf(stderr, "PCBISO: Unknown command: %s\n", argv[1]);
        usage();
        return 1;
    }
}
