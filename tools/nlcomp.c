/*
 * nlcomp.c — FidoNet Nodelist Compiler for PCBoard 15.4
 *
 * Reads a raw FidoNet nodelist (NODELIST.###) and compiles it into
 * NODELIST.DBF + NODELIST.NDX in CodeBase dBASE IV format, as expected
 * by PCBoard's built-in FidoNet support (PCBTOSS/FIDOMISC).
 *
 * Part of pcbrevival — PCBoard 15.4 Source Recovery Project
 * License: GPL v3.0
 *
 * Usage: nlcomp NODELIST.299 [output_path]
 *
 * Nodelist format (FTS-5000):
 *   Zone,1,North_America,Salt_Lake_City_UT,Sysop_Name,1-801-555-1234,9600,flags
 *   ,2,Region_2,...
 *   ,10,Net_10,...
 *   ,100,Hub_100,...
 *   ,200,Node_200,...
 *
 * Output:
 *   NODELIST.DBF — dBASE IV file with fields:
 *     ZONE(N5), NET(N5), NODE(N5), BBS_NAME(C70), LOCATION(C20),
 *     SYSOP_NAME(C25), BBS_PHONE(C50), MAXBAUD(N5), CONFIG(C4),
 *     COMP(C4), CAPABLE(C4)
 *   NODELIST.NDX — index on STR(ZONE,5)+STR(NET,5)+STR(NODE,5)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

/* --- dBASE III file structures --- */

#pragma pack(1)
typedef struct {
    unsigned char version;
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned int num_records;
    unsigned short header_size;
    unsigned short record_size;
    unsigned char reserved[20];
} DBF_HEADER;

typedef struct {
    char name[11];
    char type;
    unsigned int reserved1;
    unsigned char length;
    unsigned char decimal;
    unsigned char reserved2[14];
} DBF_FIELD;
#pragma pack()

/* Field definitions for NODELIST.DBF */
typedef struct {
    const char *name;
    char type;
    int  length;
    int  decimal;
} FIELD_DEF;

static FIELD_DEF field_defs[] = {
    { "ZONE",       'N',  5, 0 },
    { "NET",        'N',  5, 0 },
    { "NODE",       'N',  5, 0 },
    { "BBS_NAME",   'C', 70, 0 },
    { "LOCATION",   'C', 20, 0 },
    { "SYSOP_NAME", 'C', 25, 0 },
    { "BBS_PHONE",  'C', 50, 0 },
    { "MAXBAUD",    'N',  5, 0 },
    { "CONFIG",     'C',  4, 0 },
    { "COMP",       'C',  4, 0 },
    { "CAPABLE",    'C',  4, 0 },
};
#define NUM_FIELDS (sizeof(field_defs) / sizeof(field_defs[0]))
/* 1 delete flag + 5+5+5+70+20+25+50+5+4+4+4 = 198 */
#define RECORD_SIZE 198

/* --- NDX file structures (dBASE III+ .NDX format) --- */

#pragma pack(1)
typedef struct {
    unsigned int root_page;
    unsigned int total_pages;
    unsigned int reserved1;
    unsigned short key_length;
    unsigned short keys_per_page;
    unsigned short key_type;
    unsigned int  entry_size;
    unsigned char  reserved2;
    unsigned char  unique;
    char           key_expr[256];
} NDX_HEADER;
#pragma pack()

#define NDX_PAGE_SIZE 512
#define NDX_KEY_LEN   15
#define NDX_ENTRY_SIZE 24   /* 4 lower_page + 4 recno + 16 key (padded) */

/* --- Helpers --- */

static void underscore_to_space(char *s) {
    while (*s) {
        if (*s == '_') *s = ' ';
        s++;
    }
}

static void safe_copy(char *dst, const char *src, int maxlen) {
    int i;
    for (i = 0; i < maxlen - 1 && src[i]; i++)
        dst[i] = src[i];
    dst[i] = '\0';
}

static int parse_csv_line(const char *line, char out[][256], int max_fields) {
    int count = 0;
    const char *p = line;
    while (count < max_fields) {
        const char *comma = strchr(p, ',');
        if (comma) {
            int len = (int)(comma - p);
            if (len > 255) len = 255;
            memcpy(out[count], p, len);
            out[count][len] = '\0';
            count++;
            p = comma + 1;
        } else {
            safe_copy(out[count], p, 256);
            /* trim trailing whitespace */
            char *e = out[count] + strlen(out[count]) - 1;
            while (e >= out[count] && (*e == '\r' || *e == '\n' || *e == ' '))
                *e-- = '\0';
            count++;
            break;
        }
    }
    return count;
}

static void write_numeric(FILE *f, int value, int width) {
    char buf[32];
    sprintf(buf, "%*d", width, value);
    fwrite(buf, 1, width, f);
}

static void write_char_field(FILE *f, const char *s, int width) {
    int len = (int)strlen(s);
    int i;
    if (len > width) len = width;
    fwrite(s, 1, len, f);
    for (i = len; i < width; i++)
        fputc(' ', f);
}

/* --- Main --- */

int main(int argc, char **argv) {
    FILE *fin, *fdbf, *fndx;
    char line[1024];
    char outpath[256] = "";
    char dbf_path[512], ndx_path[512];
    unsigned int cur_zone = 0, cur_net = 0;
    unsigned long record_count = 0;
    int i;

    printf("nlcomp - FidoNet Nodelist Compiler for PCBoard 15.4\n");
    printf("Part of pcbrevival (GPL v3.0)\n\n");

    if (argc < 2) {
        printf("Usage: nlcomp NODELIST.### [output_path/]\n");
        printf("\nReads a raw FidoNet nodelist and creates NODELIST.DBF + NODELIST.NDX\n");
        printf("in the format expected by PCBoard's built-in FidoNet tosser.\n");
        return 1;
    }

    fin = fopen(argv[1], "r");
    if (!fin) {
        printf("Error: cannot open %s\n", argv[1]);
        return 1;
    }

    if (argc > 2) {
        safe_copy(outpath, argv[2], sizeof(outpath));
        /* ensure trailing separator */
        i = (int)strlen(outpath);
        if (i > 0 && outpath[i-1] != '/' && outpath[i-1] != '\\')
            strcat(outpath, "/");
    }

    sprintf(dbf_path, "%sNODELIST.DBF", outpath);
    sprintf(ndx_path, "%sNODELIST.NDX", outpath);

    fdbf = fopen(dbf_path, "wb");
    if (!fdbf) {
        printf("Error: cannot create %s\n", dbf_path);
        fclose(fin);
        return 1;
    }

    /* Write DBF header (placeholder) */
    DBF_HEADER hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.version = 0x03;
    hdr.year = 26;
    hdr.month = 8;
    hdr.day = 2;
    hdr.header_size = (unsigned short)(sizeof(DBF_HEADER) + NUM_FIELDS * sizeof(DBF_FIELD) + 1);
    hdr.record_size = RECORD_SIZE;
    fwrite(&hdr, sizeof(hdr), 1, fdbf);

    /* Write field descriptors */
    for (i = 0; i < (int)NUM_FIELDS; i++) {
        DBF_FIELD fld;
        memset(&fld, 0, sizeof(fld));
        strncpy(fld.name, field_defs[i].name, 11);
        fld.type = field_defs[i].type;
        fld.length = (unsigned char)field_defs[i].length;
        fld.decimal = (unsigned char)field_defs[i].decimal;
        fwrite(&fld, sizeof(fld), 1, fdbf);
    }
    fputc(0x0D, fdbf); /* header terminator */

    /* Parse nodelist and write records */
    while (fgets(line, sizeof(line), fin)) {
        char flds[10][256];
        int nf;
        char *keyword;
        int nodenum;

        if (line[0] == ';' || line[0] == '\r' || line[0] == '\n')
            continue;

        memset(flds, 0, sizeof(flds));
        nf = parse_csv_line(line, flds, 10);
        if (nf < 6) continue;

        keyword = flds[0];
        nodenum = atoi(flds[1]);

        if (strcasecmp(keyword, "Zone") == 0) {
            cur_zone = nodenum;
            cur_net = nodenum;
        } else if (strcasecmp(keyword, "Region") == 0) {
            cur_net = nodenum;
        } else if (strcasecmp(keyword, "Host") == 0) {
            cur_net = nodenum;
            nodenum = 0;
        } else if (strcasecmp(keyword, "Hub") == 0) {
            /* hub is a regular node */
        } else if (strcasecmp(keyword, "Pvt") == 0 ||
                   strcasecmp(keyword, "Hold") == 0 ||
                   strcasecmp(keyword, "Down") == 0) {
            /* status flags — node number stays */
        } else if (keyword[0] == '\0') {
            /* regular node */
        } else {
            continue;
        }

        /* Convert underscores to spaces in text fields */
        underscore_to_space(flds[2]);
        underscore_to_space(flds[3]);
        underscore_to_space(flds[4]);

        /* Write DBF record */
        fputc(' ', fdbf); /* delete flag */
        write_numeric(fdbf, cur_zone, 5);
        write_numeric(fdbf, cur_net, 5);
        write_numeric(fdbf, nodenum, 5);
        write_char_field(fdbf, flds[2], 70);  /* BBS_NAME */
        write_char_field(fdbf, flds[3], 20);  /* LOCATION */
        write_char_field(fdbf, flds[4], 25);  /* SYSOP_NAME */
        write_char_field(fdbf, flds[5], 50);  /* BBS_PHONE */
        write_numeric(fdbf, (nf > 6) ? atoi(flds[6]) : 9600, 5); /* MAXBAUD */
        write_char_field(fdbf, "0", 4);       /* CONFIG */
        write_char_field(fdbf, "0", 4);       /* COMP */
        write_char_field(fdbf, "0", 4);       /* CAPABLE */

        record_count++;
    }

    fputc(0x1A, fdbf); /* EOF marker */

    /* Rewrite header with correct record count */
    fseek(fdbf, 0, SEEK_SET);
    hdr.num_records = record_count;
    fwrite(&hdr, sizeof(hdr), 1, fdbf);
    fclose(fdbf);
    fclose(fin);

    /* --- Build NDX index --- */
    fdbf = fopen(dbf_path, "rb");
    fndx = fopen(ndx_path, "wb");

    if (!fdbf || !fndx) {
        printf("Error creating index\n");
        if (fdbf) fclose(fdbf);
        if (fndx) fclose(fndx);
        return 1;
    }

    {
        NDX_HEADER nh;
        unsigned long keys_per_page, total_pages, rec_num, num_keys;
        char page_buf[NDX_PAGE_SIZE];
        char rec[RECORD_SIZE];
        char *ptr;

        memset(&nh, 0, sizeof(nh));
        nh.key_length = NDX_KEY_LEN;
        nh.key_type = 0;
        nh.entry_size = NDX_ENTRY_SIZE;
        nh.keys_per_page = (NDX_PAGE_SIZE - 4) / NDX_ENTRY_SIZE;
        nh.unique = 1;
        strcpy(nh.key_expr, "STR(ZONE,5)+STR(NET,5)+STR(NODE,5)");

        keys_per_page = nh.keys_per_page;
        total_pages = (record_count + keys_per_page - 1) / keys_per_page;
        if (total_pages == 0) total_pages = 1;
        nh.root_page = 1;
        nh.total_pages = total_pages + 1;

        /* Write header page (512 bytes) */
        memset(page_buf, 0, NDX_PAGE_SIZE);
        memcpy(page_buf, &nh, sizeof(nh));
        fwrite(page_buf, 1, NDX_PAGE_SIZE, fndx);

        /* Skip DBF header */
        fseek(fdbf, hdr.header_size, SEEK_SET);

        /* Write leaf pages */
        rec_num = 0;
        while (rec_num < record_count) {
            memset(page_buf, 0, NDX_PAGE_SIZE);
            num_keys = 0;
            ptr = page_buf + 4;

            while (num_keys < keys_per_page && rec_num < record_count) {
                if (fread(rec, 1, RECORD_SIZE, fdbf) != RECORD_SIZE) break;

                /* lower_page = 0 (leaf) */
                memset(ptr, 0, 4);
                /* record number (1-based) */
                {
                    unsigned long rn = rec_num + 1;
                    memcpy(ptr + 4, &rn, 4);
                }
                /* key = zone(5)+net(5)+node(5) from bytes 1-15 */
                memcpy(ptr + 8, &rec[1], 15);
                ptr[8 + 15] = '\0';

                ptr += NDX_ENTRY_SIZE;
                num_keys++;
                rec_num++;
            }

            memcpy(page_buf, &num_keys, 4);
            fwrite(page_buf, 1, NDX_PAGE_SIZE, fndx);
        }
    }

    fclose(fdbf);
    fclose(fndx);

    printf("Compiled %lu nodes from %s\n", record_count, argv[1]);
    printf("Created: %s (%lu records)\n", dbf_path, record_count);
    printf("Created: %s\n", ndx_path);

    return 0;
}
