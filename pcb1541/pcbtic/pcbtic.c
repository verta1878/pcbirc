/*
 * pcbtic.c — TIC File Processor for PCBoard 15.4
 * Part of pcbrevival (GPL v3.0)
 *
 * Processes FidoNet .TIC files from inbound directory:
 *   - Moves described files to PCBoard file areas
 *   - Updates DIR listing files
 *   - Forwards TIC+files to downlinks (passthrough)
 *   - Hatches new files into the TIC network
 *
 * Usage: pcbtic [options]
 *   -t              Toss inbound TIC files (default action)
 *   -h file area    Hatch a file into a file echo area
 *   -i path         Inbound directory (default: from pcbis.cfg)
 *   -o path         Outbound directory
 *   -c config       Path to pcbis.cfg
 *   -l              List configured file echo areas
 *   -v              Verbose output
 *   -?              This help
 *
 * TIC file format (FTS-5006.001):
 *   Area AREANAME
 *   Origin 1:2/3
 *   From 1:2/3
 *   To 1:2/3.0
 *   File FILENAME.ZIP
 *   Size 12345
 *   Date 1234567890
 *   Desc Short description of file
 *   LDesc Long description line 1
 *   LDesc Long description line 2
 *   CRC DEADBEEF
 *   Path 1:2/3 1234567890 Wed Jan 01 00:00:00 2025
 *   Seenby 1:2/3
 *   Pw PASSWORD
 *
 * Config file: pcbtic.cfg
 *   [area]
 *   name=AREANAME
 *   dir=/path/to/pcboard/dir001
 *   dirlist=/path/to/dir001.lst
 *   passthrough=no
 *   downlinks=1:2/3 1:2/4
 *
 * Compile: gcc -o pcbtic pcbtic.c -Wall -O2
 *          wcc386 pcbtic.c -bt=dos -mf -5 -ox
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define PATH_SEP '\\'
#else
#include <unistd.h>
#include <dirent.h>
#define PATH_SEP '/'
#endif

#define MAX_AREAS    256
#define MAX_LINKS     64
#define MAX_PATH_LEN 260
#define MAX_DESC     512

/* ── TIC file data ─────────────────────────────────────────────── */

typedef struct {
    char area[64];
    char origin[40];
    char from[40];
    char to[40];
    char file[MAX_PATH_LEN];
    long size;
    unsigned long crc;
    char desc[80];
    char ldesc[MAX_DESC];
    char pw[32];
    char path[MAX_DESC];
    char seenby[MAX_DESC];
    int  replaces;
    char replaces_file[MAX_PATH_LEN];
} ticdata;

/* ── File echo area config ─────────────────────────────────────── */

typedef struct {
    char name[64];
    char dir[MAX_PATH_LEN];
    char dirlist[MAX_PATH_LEN];
    int  passthrough;
    char downlinks[MAX_DESC];
} area_cfg;

static area_cfg areas[MAX_AREAS];
static int num_areas = 0;
static char inbound[MAX_PATH_LEN]  = "";
static char outbound[MAX_PATH_LEN] = "";
static char our_addr[40]           = "";
static int  verbose = 0;

/* ── Utility ───────────────────────────────────────────────────── */

static char *trim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    char *e = s + strlen(s) - 1;
    while (e >= s && isspace((unsigned char)*e)) *e-- = 0;
    return s;
}

static void strtoupper(char *s) {
    while (*s) { *s = (char)toupper((unsigned char)*s); s++; }
}

/* ── Parse TIC file ────────────────────────────────────────────── */

static int parse_tic(const char *path, ticdata *tic) {
    FILE *f = fopen(path, "r");
    char line[1024], key[64], *val;

    if (!f) return -1;
    memset(tic, 0, sizeof(*tic));

    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == 0 || *p == ';') continue;

        /* Extract key */
        int i = 0;
        while (*p && !isspace((unsigned char)*p) && i < 63)
            key[i++] = *p++;
        key[i] = 0;
        val = trim(p);
        strtoupper(key);

        if      (!strcmp(key, "AREA"))    strncpy(tic->area, val, 63);
        else if (!strcmp(key, "ORIGIN"))  strncpy(tic->origin, val, 39);
        else if (!strcmp(key, "FROM"))    strncpy(tic->from, val, 39);
        else if (!strcmp(key, "TO"))      strncpy(tic->to, val, 39);
        else if (!strcmp(key, "FILE"))    strncpy(tic->file, val, MAX_PATH_LEN-1);
        else if (!strcmp(key, "SIZE"))    tic->size = atol(val);
        else if (!strcmp(key, "DESC"))    strncpy(tic->desc, val, 79);
        else if (!strcmp(key, "CRC"))     tic->crc = strtoul(val, NULL, 16);
        else if (!strcmp(key, "PW"))      strncpy(tic->pw, val, 31);
        else if (!strcmp(key, "REPLACES")) {
            tic->replaces = 1;
            strncpy(tic->replaces_file, val, MAX_PATH_LEN-1);
        }
        else if (!strcmp(key, "LDESC")) {
            if (tic->ldesc[0]) strncat(tic->ldesc, "\n", MAX_DESC-strlen(tic->ldesc)-1);
            strncat(tic->ldesc, val, MAX_DESC-strlen(tic->ldesc)-1);
        }
        else if (!strcmp(key, "PATH")) {
            if (tic->path[0]) strncat(tic->path, "\n", MAX_DESC-strlen(tic->path)-1);
            strncat(tic->path, val, MAX_DESC-strlen(tic->path)-1);
        }
        else if (!strcmp(key, "SEENBY")) {
            if (tic->seenby[0]) strncat(tic->seenby, " ", MAX_DESC-strlen(tic->seenby)-1);
            strncat(tic->seenby, val, MAX_DESC-strlen(tic->seenby)-1);
        }
    }
    fclose(f);
    return (tic->area[0] && tic->file[0]) ? 0 : -1;
}

/* ── Find area config ──────────────────────────────────────────── */

static area_cfg *find_area(const char *name) {
    int i;
    char upper[64];
    strncpy(upper, name, 63); upper[63] = 0;
    strtoupper(upper);
    for (i = 0; i < num_areas; i++) {
        char cmp[64];
        strncpy(cmp, areas[i].name, 63); cmp[63] = 0;
        strtoupper(cmp);
        if (!strcmp(upper, cmp)) return &areas[i];
    }
    return NULL;
}

/* ── Update DIR listing ────────────────────────────────────────── */

static void update_dirlist(const char *listpath, const char *filename,
                           long filesize, const char *desc) {
    FILE *f;
    char datebuf[12];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    sprintf(datebuf, "%02d-%02d-%02d", t->tm_mon+1, t->tm_mday, t->tm_year % 100);

    f = fopen(listpath, "a");
    if (!f) {
        fprintf(stderr, "pcbtic: cannot update %s\n", listpath);
        return;
    }
    /* PCBoard DIR format: FILENAME.EXT  SIZE  DATE  DESCRIPTION */
    fprintf(f, "%-12s %8ld  %s  %s\n", filename, filesize, datebuf, desc);
    fclose(f);

    if (verbose) printf("  Updated: %s\n", listpath);
}

/* ── Copy file ─────────────────────────────────────────────────── */

static int copy_file(const char *src, const char *dst) {
    FILE *in, *out;
    char buf[8192];
    size_t n;

    in = fopen(src, "rb");
    if (!in) return -1;
    out = fopen(dst, "wb");
    if (!out) { fclose(in); return -1; }

    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
        fwrite(buf, 1, n, out);

    fclose(in);
    fclose(out);
    return 0;
}

/* ── Write outbound TIC ───────────────────────────────────────── */

static void write_tic(const char *dir, ticdata *tic, const char *downlink) {
    char path[MAX_PATH_LEN];
    FILE *f;

    sprintf(path, "%s%c%s", dir, PATH_SEP, tic->file);
    /* Change extension to .TIC */
    char *dot = strrchr(path, '.');
    if (dot) strcpy(dot, ".TIC");
    else strcat(path, ".TIC");

    f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "Area %s\r\n", tic->area);
    fprintf(f, "Origin %s\r\n", tic->origin[0] ? tic->origin : our_addr);
    fprintf(f, "From %s\r\n", our_addr);
    fprintf(f, "To %s\r\n", downlink);
    fprintf(f, "File %s\r\n", tic->file);
    if (tic->size) fprintf(f, "Size %ld\r\n", tic->size);
    if (tic->desc[0]) fprintf(f, "Desc %s\r\n", tic->desc);
    if (tic->crc) fprintf(f, "CRC %08lX\r\n", tic->crc);
    if (tic->path[0]) {
        char *line = strtok(tic->path, "\n");
        while (line) {
            fprintf(f, "Path %s\r\n", line);
            line = strtok(NULL, "\n");
        }
    }
    /* Add our path line */
    {
        time_t now = time(NULL);
        char tbuf[64];
        strftime(tbuf, sizeof(tbuf), "%a %b %d %H:%M:%S %Y", gmtime(&now));
        fprintf(f, "Path %s %ld %s\r\n", our_addr, (long)now, tbuf);
    }
    if (tic->seenby[0]) fprintf(f, "Seenby %s\r\n", tic->seenby);
    fprintf(f, "Seenby %s\r\n", our_addr);

    fclose(f);
    if (verbose) printf("  Forwarded TIC to %s: %s\n", downlink, path);
}

/* ── Toss one TIC file ─────────────────────────────────────────── */

static int toss_tic(const char *ticpath) {
    ticdata tic;
    area_cfg *area;
    char srcfile[MAX_PATH_LEN], dstfile[MAX_PATH_LEN];
    struct stat st;

    if (parse_tic(ticpath, &tic) != 0) {
        fprintf(stderr, "pcbtic: invalid TIC file: %s\n", ticpath);
        return -1;
    }

    if (verbose) printf("Processing: %s → area %s, file %s\n", ticpath, tic.area, tic.file);

    area = find_area(tic.area);
    if (!area) {
        fprintf(stderr, "pcbtic: unknown area '%s' in %s\n", tic.area, ticpath);
        return -1;
    }

    /* Find the actual file in inbound */
    sprintf(srcfile, "%s%c%s", inbound, PATH_SEP, tic.file);
    if (stat(srcfile, &st) != 0) {
        fprintf(stderr, "pcbtic: file not found: %s\n", srcfile);
        return -1;
    }
    if (tic.size == 0) tic.size = (long)st.st_size;

    if (!area->passthrough) {
        /* Move file to area directory */
        sprintf(dstfile, "%s%c%s", area->dir, PATH_SEP, tic.file);
        if (copy_file(srcfile, dstfile) != 0) {
            fprintf(stderr, "pcbtic: cannot copy %s → %s\n", srcfile, dstfile);
            return -1;
        }
        if (verbose) printf("  Copied: %s → %s\n", srcfile, dstfile);

        /* Update DIR listing */
        if (area->dirlist[0]) {
            update_dirlist(area->dirlist, tic.file, tic.size,
                           tic.desc[0] ? tic.desc : "No description");
        }

        /* Handle REPLACES — remove old file */
        if (tic.replaces) {
            char oldfile[MAX_PATH_LEN];
            sprintf(oldfile, "%s%c%s", area->dir, PATH_SEP, tic.replaces_file);
            unlink(oldfile);
            if (verbose) printf("  Replaced: %s\n", tic.replaces_file);
        }
    }

    /* Forward to downlinks */
    if (area->downlinks[0]) {
        char links[MAX_DESC];
        char *link;
        strncpy(links, area->downlinks, MAX_DESC-1);
        link = strtok(links, " ,");
        while (link) {
            link = trim(link);
            if (*link && strcmp(link, tic.from)) {
                /* Copy file to outbound */
                sprintf(dstfile, "%s%c%s", outbound, PATH_SEP, tic.file);
                copy_file(srcfile, dstfile);
                write_tic(outbound, &tic, link);
            }
            link = strtok(NULL, " ,");
        }
    }

    /* Clean up inbound */
    unlink(srcfile);
    unlink(ticpath);

    printf("Tossed: %s [%s] %s (%ld bytes)\n",
           tic.area, tic.file, tic.desc, tic.size);
    return 0;
}

/* ── Hatch a file ──────────────────────────────────────────────── */

static int hatch_file(const char *filepath, const char *areaname, const char *desc) {
    area_cfg *area;
    ticdata tic;
    struct stat st;
    char *basename_p;
    char dstfile[MAX_PATH_LEN];

    area = find_area(areaname);
    if (!area) {
        fprintf(stderr, "pcbtic: unknown area '%s'\n", areaname);
        return -1;
    }
    if (stat(filepath, &st) != 0) {
        fprintf(stderr, "pcbtic: file not found: %s\n", filepath);
        return -1;
    }

    basename_p = strrchr(filepath, PATH_SEP);
    if (!basename_p) basename_p = strrchr(filepath, '/');
    basename_p = basename_p ? basename_p + 1 : (char *)filepath;

    memset(&tic, 0, sizeof(tic));
    strncpy(tic.area, areaname, 63);
    strncpy(tic.file, basename_p, MAX_PATH_LEN-1);
    strncpy(tic.origin, our_addr, 39);
    strncpy(tic.from, our_addr, 39);
    tic.size = (long)st.st_size;
    if (desc) strncpy(tic.desc, desc, 79);
    else strncpy(tic.desc, "Hatched file", 79);

    /* Copy file to area directory */
    if (!area->passthrough) {
        sprintf(dstfile, "%s%c%s", area->dir, PATH_SEP, basename_p);
        copy_file(filepath, dstfile);
        if (area->dirlist[0])
            update_dirlist(area->dirlist, basename_p, tic.size, tic.desc);
    }

    /* Forward to downlinks */
    if (area->downlinks[0]) {
        char links[MAX_DESC];
        char *link;
        sprintf(dstfile, "%s%c%s", outbound, PATH_SEP, basename_p);
        copy_file(filepath, dstfile);
        strncpy(links, area->downlinks, MAX_DESC-1);
        link = strtok(links, " ,");
        while (link) {
            link = trim(link);
            if (*link) write_tic(outbound, &tic, link);
            link = strtok(NULL, " ,");
        }
    }

    printf("Hatched: %s → %s [%s] (%ld bytes)\n",
           basename_p, areaname, tic.desc, tic.size);
    return 0;
}

/* ── Load config ───────────────────────────────────────────────── */

static void load_config(const char *path) {
    FILE *f = fopen(path, "r");
    char line[512];
    area_cfg *cur = NULL;

    if (!f) return;
    while (fgets(line, sizeof(line), f)) {
        char *p = trim(line);
        if (*p == '#' || *p == ';' || *p == 0) continue;

        if (!strcmp(p, "[area]")) {
            if (num_areas < MAX_AREAS) {
                cur = &areas[num_areas++];
                memset(cur, 0, sizeof(*cur));
            }
            continue;
        }

        char key[64], val[256];
        if (sscanf(p, "%63[^=]=%255[^\n]", key, val) == 2) {
            char *k = trim(key);
            char *v = trim(val);

            if (!cur) {
                /* Global settings */
                if (!strcmp(k, "inbound"))  strncpy(inbound, v, MAX_PATH_LEN-1);
                else if (!strcmp(k, "outbound")) strncpy(outbound, v, MAX_PATH_LEN-1);
                else if (!strcmp(k, "address"))  strncpy(our_addr, v, 39);
            } else {
                if (!strcmp(k, "name"))        strncpy(cur->name, v, 63);
                else if (!strcmp(k, "dir"))    strncpy(cur->dir, v, MAX_PATH_LEN-1);
                else if (!strcmp(k, "dirlist")) strncpy(cur->dirlist, v, MAX_PATH_LEN-1);
                else if (!strcmp(k, "passthrough")) cur->passthrough = (v[0]=='y'||v[0]=='Y'||v[0]=='1');
                else if (!strcmp(k, "downlinks")) strncpy(cur->downlinks, v, MAX_DESC-1);
            }
        }
    }
    fclose(f);
}

/* ── Toss all TIC files in inbound ─────────────────────────────── */

static int toss_inbound(void) {
    int count = 0;
#ifdef _WIN32
    /* Windows: _findfirst/_findnext */
    char pattern[MAX_PATH_LEN];
    struct _finddata_t fd;
    long h;
    sprintf(pattern, "%s%c*.TIC", inbound, PATH_SEP);
    h = _findfirst(pattern, &fd);
    if (h != -1) {
        do {
            char fullpath[MAX_PATH_LEN];
            sprintf(fullpath, "%s%c%s", inbound, PATH_SEP, fd.name);
            if (toss_tic(fullpath) == 0) count++;
        } while (_findnext(h, &fd) == 0);
        _findclose(h);
    }
#else
    DIR *d = opendir(inbound);
    struct dirent *ent;
    if (!d) {
        fprintf(stderr, "pcbtic: cannot open inbound: %s\n", inbound);
        return -1;
    }
    while ((ent = readdir(d)) != NULL) {
        char *ext = strrchr(ent->d_name, '.');
        if (ext && (!strcasecmp(ext, ".tic") || !strcasecmp(ext, ".TIC"))) {
            char fullpath[MAX_PATH_LEN];
            sprintf(fullpath, "%s/%s", inbound, ent->d_name);
            if (toss_tic(fullpath) == 0) count++;
        }
    }
    closedir(d);
#endif
    return count;
}

/* ── Help ──────────────────────────────────────────────────────── */

static void print_help(void) {
    printf(
        "pcbtic — TIC File Processor for PCBoard 15.4\n"
        "Part of pcbrevival (GPL v3.0)\n"
        "\n"
        "Usage: pcbtic [options]\n"
        "\n"
        "  -t              Toss inbound .TIC files (default)\n"
        "  -h file area    Hatch a file into a file echo area\n"
        "     -d desc      Description for hatched file\n"
        "  -i path         Inbound directory\n"
        "  -o path         Outbound directory\n"
        "  -c config       Config file (default: pcbtic.cfg)\n"
        "  -l              List configured file echo areas\n"
        "  -v              Verbose output\n"
        "  -?              This help\n"
        "\n"
        "Config file: pcbtic.cfg\n"
        "\n"
        "  # Global settings\n"
        "  address=1:234/56.0\n"
        "  inbound=/home/pcboard/fido/inbound\n"
        "  outbound=/home/pcboard/fido/outbound\n"
        "\n"
        "  # File echo areas\n"
        "  [area]\n"
        "  name=BBS_UTILS\n"
        "  dir=/home/pcboard/dirs/bbsutil\n"
        "  dirlist=/home/pcboard/dirs/bbsutil.lst\n"
        "  passthrough=no\n"
        "  downlinks=1:2/3 1:2/4\n"
        "\n"
        "  [area]\n"
        "  name=DOORWARE\n"
        "  dir=/home/pcboard/dirs/doors\n"
        "  dirlist=/home/pcboard/dirs/doors.lst\n"
        "\n"
        "TIC format: FTS-5006.001\n"
        "Reference: https://ftsc.org/docs/fts-5006.001\n"
    );
}

/* ── Main ──────────────────────────────────────────────────────── */

int main(int argc, char **argv) {
    const char *cfgpath = "pcbtic.cfg";
    int mode = 0; /* 0=toss, 1=hatch, 2=list */
    const char *hatch_path = NULL, *hatch_area = NULL, *hatch_desc = NULL;
    int i;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-t")) mode = 0;
        else if (!strcmp(argv[i], "-h") && i+2 < argc) {
            mode = 1;
            hatch_path = argv[++i];
            hatch_area = argv[++i];
        }
        else if (!strcmp(argv[i], "-d") && i+1 < argc) hatch_desc = argv[++i];
        else if (!strcmp(argv[i], "-i") && i+1 < argc) strncpy(inbound, argv[++i], MAX_PATH_LEN-1);
        else if (!strcmp(argv[i], "-o") && i+1 < argc) strncpy(outbound, argv[++i], MAX_PATH_LEN-1);
        else if (!strcmp(argv[i], "-c") && i+1 < argc) cfgpath = argv[++i];
        else if (!strcmp(argv[i], "-l")) mode = 2;
        else if (!strcmp(argv[i], "-v")) verbose = 1;
        else if (!strcmp(argv[i], "-?") || !strcmp(argv[i], "--help")) {
            print_help();
            return 0;
        }
    }

    load_config(cfgpath);

    if (!inbound[0])  strcpy(inbound, ".");
    if (!outbound[0]) strcpy(outbound, ".");

    switch (mode) {
        case 0: {
            int n = toss_inbound();
            if (n >= 0) printf("pcbtic: %d TIC file(s) processed\n", n);
            break;
        }
        case 1:
            if (!hatch_path || !hatch_area) {
                fprintf(stderr, "pcbtic: -h requires file and area\n");
                return 1;
            }
            return hatch_file(hatch_path, hatch_area, hatch_desc);
        case 2:
            printf("Configured file echo areas:\n");
            for (i = 0; i < num_areas; i++)
                printf("  %-20s %s%s\n", areas[i].name, areas[i].dir,
                       areas[i].passthrough ? " (passthrough)" : "");
            if (!num_areas) printf("  (none — add [area] sections to %s)\n", cfgpath);
            break;
    }

    return 0;
}
