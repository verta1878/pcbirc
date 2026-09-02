/* ============================================================================
 * redx.c — WCSC .RED archive extractor (v1.0)
 *
 * Extracts records from PCBoard install disks' .RED archives (COMMDRV.RED,
 * PCBOARD.RED, etc.). Supports:
 *   Method 0x0001 (STORED) — verified byte-perfect
 *   Method 0x000B (LHA-lh5 variant) — 9/10 test vectors byte-perfect
 *                                     (see red_decompress.c for known issues)
 *
 * Usage:
 *   redx list <archive.RED>              list contents
 *   redx extract <archive.RED>           extract all to cwd
 *   redx extract <archive.RED> <name>    extract single record
 *
 * License: GPLv3
 * ==========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Prototype from red_decompress.c */
int red_decompress(const uint8_t *cmp, size_t cmp_len,
                   uint8_t *out, size_t out_len);

/* Prototypes from red_pack.c */
int red_pack_file(FILE *out, const char *disk_path, const char *store_name,
                  int prefer_lha, const char *lha_path);

typedef struct {
    uint32_t cmp_size;
    uint32_t unc_size;
    uint16_t crc16;
    uint16_t method;
    char     filename[128];
    size_t   payload_offset;
    size_t   next_offset;
} red_record_t;

static uint32_t rd_le32(const uint8_t *p) {
    return p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24);
}
static uint16_t rd_le16(const uint8_t *p) {
    return p[0] | (p[1]<<8);
}

static uint8_t *slurp(const char *path, size_t *len_out) {
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    size_t n;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    n = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = malloc(n);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, n, f) != n) { free(buf); fclose(f); return NULL; }
    fclose(f);
    *len_out = n;
    return buf;
}

/* WCSC .RED record: fixed 41-byte header slot + variable payload.
 *   0-1:   'RR' magic
 *   2:     format version
 *   3-7:   5-byte DOS timestamp
 *   8-11:  cmp_size (LE32)
 *   12-15: unc_size (LE32)
 *   16-17: 0xffff
 *   18-19: crc16 (LE16)
 *   20-23: reserved/flags
 *   24-25: method (LE16, 0x0001=STORED, 0x000B=LHA-lh5)
 *   26-40: 15-byte filename slot (null-terminated, zero-padded).
 * Payload begins at pos+41 (fixed) for exactly cmp_size bytes.
 * Next record header starts at pos+41+cmp_size.
 */
static int parse_next(const uint8_t *data, size_t len, size_t pos,
                      red_record_t *rec) {
    size_t fn_start, fn_end, i;
    if (pos + 41 > len) return -1;
    if (data[pos] != 'R' || data[pos+1] != 'R') return -1;
    rec->cmp_size = rd_le32(data + pos + 8);
    rec->unc_size = rd_le32(data + pos + 12);
    rec->crc16    = rd_le16(data + pos + 18);
    rec->method   = rd_le16(data + pos + 24);
    fn_start = pos + 26;
    fn_end = fn_start;
    while (fn_end < pos + 41 && data[fn_end] != 0) fn_end++;
    if (fn_end >= pos + 41) return -1;
    i = fn_end - fn_start;
    if (i >= sizeof(rec->filename)) i = sizeof(rec->filename) - 1;
    memcpy(rec->filename, data + fn_start, i);
    rec->filename[i] = 0;
    rec->payload_offset = pos + 41;
    rec->next_offset = pos + 41 + rec->cmp_size;
    return 0;
}

static void cmd_list(const char *path) {
    size_t len;
    uint8_t *data = slurp(path, &len);
    size_t pos = 0;
    red_record_t rec;
    int count = 0;

    if (!data) { perror(path); return; }
    printf("%-20s %-8s %10s %10s %6s\n",
           "FILENAME", "METHOD", "CMP", "UNC", "CRC16");
    while (parse_next(data, len, pos, &rec) == 0) {
        const char *m = rec.method == 0x0001 ? "STORED" :
                        rec.method == 0x000B ? "LHA-lh5" : "?";
        printf("%-20s %-8s %10u %10u  %04x\n",
               rec.filename, m, rec.cmp_size, rec.unc_size, rec.crc16);
        count++;
        pos = rec.next_offset;
    }
    printf("%d record(s)\n", count);
    free(data);
}

static void cmd_extract(const char *path, const char *filter) {
    size_t len;
    uint8_t *data = slurp(path, &len);
    size_t pos = 0;
    red_record_t rec;
    int extracted = 0;

    if (!data) { perror(path); return; }
    while (parse_next(data, len, pos, &rec) == 0) {
        if (filter && strcmp(filter, rec.filename) != 0) {
            pos = rec.next_offset;
            continue;
        }
        uint8_t *out = malloc(rec.unc_size ? rec.unc_size : 1);
        int rc = 0;
        if (rec.method == 0x0001) {
            if (rec.cmp_size == rec.unc_size)
                memcpy(out, data + rec.payload_offset, rec.unc_size);
            else
                rc = -1;
        } else if (rec.method == 0x000B) {
            rc = red_decompress(data + rec.payload_offset, rec.cmp_size,
                                out, rec.unc_size);
        } else {
            rc = -2;
        }
        if (rc == 0) {
            FILE *f = fopen(rec.filename, "wb");
            if (f) {
                fwrite(out, 1, rec.unc_size, f);
                fclose(f);
                printf("  OK   %s (%u bytes)\n", rec.filename, rec.unc_size);
                extracted++;
            } else {
                printf("  FAIL %s: cannot create file\n", rec.filename);
            }
        } else {
            printf("  FAIL %s: decompress rc=%d\n", rec.filename, rc);
        }
        free(out);
        pos = rec.next_offset;
    }
    printf("Extracted %d file(s)\n", extracted);
    free(data);
}


static void cmd_pack(int argc, char **argv) {
    /* argv[0] = out.RED, then flags and files */
    const char *out_path = NULL;
    const char *lha_path = NULL;
    int prefer_lha = 0;
    int i, first_file = -1;
    FILE *out;
    int count = 0;

    for (i = 0; i < argc; i++) {
        if (!strncmp(argv[i], "--lha=", 6)) {
            prefer_lha = 1;
            lha_path = argv[i] + 6;
        } else if (!strcmp(argv[i], "--stored")) {
            prefer_lha = 0;
        } else if (!out_path) {
            out_path = argv[i];
        } else {
            if (first_file < 0) first_file = i;
        }
    }

    if (!out_path || first_file < 0) {
        fprintf(stderr,
            "usage: redx pack [--stored|--lha=<path>] <out.RED> <file>...\n");
        return;
    }

    out = fopen(out_path, "wb");
    if (!out) {
        fprintf(stderr, "cannot create %s\n", out_path);
        return;
    }

    for (i = first_file; i < argc; i++) {
        const char *base = strrchr(argv[i], '/');
        base = base ? base + 1 : argv[i];
        if (red_pack_file(out, argv[i], base, prefer_lha, lha_path) < 0) {
            fclose(out);
            return;
        }
        printf("  added: %s\n", base);
        count++;
    }
    fclose(out);
    printf("wrote %s (%d records)\n", out_path, count);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr,
                "usage:\n"
                "  redx list <archive.RED>\n"
                "  redx extract <archive.RED> [name]\n"
                "  redx pack [--stored|--lha=<path>] <out.RED> <file>...\n");
        return 1;
    }
    if (!strcmp(argv[1], "list"))
        cmd_list(argv[2]);
    else if (!strcmp(argv[1], "extract"))
        cmd_extract(argv[2], argc > 3 ? argv[3] : NULL);
    else if (!strcmp(argv[1], "pack"))
        cmd_pack(argc - 2, argv + 2);
    else {
        fprintf(stderr, "unknown command: %s\n", argv[1]);
        return 1;
    }
    return 0;
}
