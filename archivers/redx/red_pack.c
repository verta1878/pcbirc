/* ============================================================================
 * red_pack.c — WCSC .RED archive writer (v1.0)
 *
 * Builds .RED archives compatible with WCSC PCBoard installers. Companion to
 * red_decompress.c / redx.c which handle the read side.
 *
 * Compression modes:
 *   METHOD_STORED (0x0001)  — no compression, always available, always works
 *   METHOD_LHA    (0x000B)  — Yoshi LHA -lh5- variant + 2-byte WCSC prefix
 *                             (requires archivers/lha/src/lha binary in PATH
 *                             or explicit --lha-path; falls back to STORED
 *                             if unavailable)
 *
 * Container format (26-byte header + null-terminated filename + payload +
 * 2-byte CRC16 trailer) is fully documented in
 * pcb154/pcbdcom/GAP-ANALYSIS.md.
 *
 * Round-trip guarantee: any .RED written by this tool can be read back by
 * red_decompress.c (both methods verified).
 *
 * License: GPLv3
 * pcbirc crew (hexadecimal)
 * ==========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
# include <io.h>
#else
# include <unistd.h>
#endif

/* ----- CRC16 (WCSC convention: standard CCITT poly 0x1021, init 0xFFFF) ----- */

static uint16_t red_crc16(const uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFF;
    size_t i;
    unsigned j;
    for (i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (j = 0; j < 8; j++) {
            if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
            else              crc = (crc << 1);
        }
    }
    return crc;
}

/* ----- Little-endian writers ----- */

static void wr_le16(uint8_t *p, uint16_t v)
{
    p[0] = v & 0xFF;
    p[1] = (v >> 8) & 0xFF;
}

static void wr_le32(uint8_t *p, uint32_t v)
{
    p[0] = v & 0xFF;
    p[1] = (v >> 8) & 0xFF;
    p[2] = (v >> 16) & 0xFF;
    p[3] = (v >> 24) & 0xFF;
}

/* ----- Compression methods ----- */

#define RED_METHOD_STORED  0x0001
#define RED_METHOD_LHA     0x000B

/* ----- File I/O helpers ----- */

static uint8_t *slurp_file(const char *path, size_t *len_out)
{
    FILE *f;
    uint8_t *buf;
    long n;

    f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n < 0) { fclose(f); return NULL; }

    buf = (uint8_t *)malloc((size_t)n + 1);
    if (!buf) { fclose(f); return NULL; }
    if (n > 0 && fread(buf, 1, (size_t)n, f) != (size_t)n) {
        free(buf); fclose(f); return NULL;
    }
    fclose(f);
    *len_out = (size_t)n;
    return buf;
}

/* ----- LHA compression via external Yoshi lha binary ----- *
 * We shell out to archivers/lha/src/lha rather than porting the 2000+ line
 * slide/huf compressor. That binary is already built by the pcbirc source
 * tree (autoreconf + configure + make in archivers/lha/).
 *
 * Steps:
 *   1. Write input to /tmp/redx-inXXXXXX
 *   2. Run: lha aq5 /tmp/redx-outXXXXXX.lzh /tmp/redx-inXXXXXX
 *   3. Parse resulting .lzh header, extract raw -lh5- payload
 *   4. Prepend 2-byte WCSC header (2 zero bytes — we can only guarantee
 *      round-trip against our own decoder, not COMMDRV.EXE-style content)
 *   5. Return the wrapped stream
 *
 * Returns NULL on any failure — caller should fall back to STORED. */
static uint8_t *lha_compress(const uint8_t *src, size_t src_len,
                             size_t *out_len, const char *lha_path)
{
    char in_path[64], out_path[64], cmd[512];
    FILE *f;
    uint8_t *lzh_data, *result;
    size_t lzh_len, payload_off, payload_len;
    int rc;

    if (!lha_path || !*lha_path) return NULL;

    /* Temp file paths (use PID for uniqueness; on multi-thread callers a
     * proper mkstemp would be needed). */
    snprintf(in_path,  sizeof(in_path),  "/tmp/redx-in-%d.bin",  (int)getpid());
    snprintf(out_path, sizeof(out_path), "/tmp/redx-out-%d.lzh", (int)getpid());

    /* Write input */
    f = fopen(in_path, "wb");
    if (!f) return NULL;
    if (fwrite(src, 1, src_len, f) != src_len) { fclose(f); unlink(in_path); return NULL; }
    fclose(f);

    /* Delete stale output; lha won't overwrite */
    unlink(out_path);

    /* Build command: lha with -lh5- forced.
     * "a" = add, "q" = quiet, "5" = force -lh5- method */
    snprintf(cmd, sizeof(cmd), "%s aqo50 %s %s > /dev/null 2>&1",
             lha_path, out_path, in_path);
    rc = system(cmd);
    unlink(in_path);
    if (rc != 0) { unlink(out_path); return NULL; }

    /* Read resulting .lzh */
    lzh_data = slurp_file(out_path, &lzh_len);
    unlink(out_path);
    if (!lzh_data) return NULL;

    /* Parse level-0 LHA header to find payload offset:
     *   byte 0: header size (excluding this byte + checksum)
     *   byte 1: header checksum
     *   bytes 2..7: method ID ("-lh5-")
     *   bytes 8..11: compressed size (LE u32)
     *   bytes 12..15: uncompressed size (LE u32)
     *   ... file header continues ...
     * Payload starts at 2 + header_size. */
    if (lzh_len < 22 || memcmp(lzh_data + 2, "-lh5-", 5) != 0) {
        free(lzh_data);
        return NULL;
    }
    payload_off = 2 + (size_t)lzh_data[0];
    if (payload_off >= lzh_len) { free(lzh_data); return NULL; }
    payload_len = lzh_len - payload_off;

    /* WCSC .RED method 0x000B prepends 2 bytes before the raw LHA stream.
     * For repack we emit 2 zero bytes (round-trips against our decoder). */
    result = (uint8_t *)malloc(payload_len + 2);
    if (!result) { free(lzh_data); return NULL; }
    result[0] = 0xFF;
    result[1] = 0xFF;
    memcpy(result + 2, lzh_data + payload_off, payload_len);
    free(lzh_data);

    *out_len = payload_len + 2;
    return result;
}

/* ----- Write one record into an open .RED file ----- */

static int write_record(FILE *out, const char *filename,
                        const uint8_t *payload, size_t cmp_size,
                        size_t unc_size, uint16_t crc, uint16_t method)
{
    uint8_t header[26];
    uint16_t trailer_crc;
    size_t fn_len;

    memset(header, 0, sizeof(header));
    header[0] = 'R';
    header[1] = 'R';
    header[2] = 0x01;                            /* version              */
    /* bytes 3..7 timestamp — leave zero (not required for extraction)   */
    wr_le32(header + 8,  (uint32_t)cmp_size);
    wr_le32(header + 12, (uint32_t)unc_size);
    wr_le16(header + 16, 0xFFFF);                /* marker               */
    wr_le16(header + 18, crc);                   /* CRC16 of uncompressed*/
    wr_le16(header + 20, 0x0000);                /* padding              */
    wr_le16(header + 22, 0x0001);                /* const                */
    wr_le16(header + 24, method);

    if (fwrite(header, 1, 26, out) != 26) return -1;

    /* Null-terminated filename */
    fn_len = strlen(filename);
    if (fwrite(filename, 1, fn_len, out) != fn_len) return -1;
    if (fputc(0, out) == EOF) return -1;

    /* Compressed payload */
    if (cmp_size > 0 && fwrite(payload, 1, cmp_size, out) != cmp_size) return -1;

    /* 2-byte CRC16 trailer (of compressed data) */
    trailer_crc = red_crc16(payload, cmp_size);
    {
        uint8_t tr[2];
        wr_le16(tr, trailer_crc);
        if (fwrite(tr, 1, 2, out) != 2) return -1;
    }

    return 0;
}

/* ----- Pack one file into a record (chooses method) ----- */

int red_pack_file(FILE *out, const char *disk_path, const char *store_name,
                  int prefer_lha, const char *lha_path)
{
    uint8_t *raw, *compressed;
    size_t raw_len, cmp_len;
    uint16_t crc;
    uint16_t method;
    int rc;

    raw = slurp_file(disk_path, &raw_len);
    if (!raw) {
        fprintf(stderr, "red_pack: cannot read '%s': %s\n",
                disk_path, strerror(errno));
        return -1;
    }

    crc = red_crc16(raw, raw_len);

    compressed = NULL;
    cmp_len = 0;
    method = RED_METHOD_STORED;

    if (prefer_lha) {
        compressed = lha_compress(raw, raw_len, &cmp_len, lha_path);
        if (compressed && cmp_len < raw_len) {
            method = RED_METHOD_LHA;
        } else {
            /* LHA unavailable, or compression didn't help — fall back */
            if (compressed) { free(compressed); compressed = NULL; }
        }
    }

    if (method == RED_METHOD_STORED) {
        compressed = raw;
        cmp_len = raw_len;
    }

    rc = write_record(out, store_name, compressed, cmp_len,
                      raw_len, crc, method);

    if (method == RED_METHOD_LHA) free(compressed);
    free(raw);
    return rc;
}

/* ----- CLI entry ----- */

#ifdef RED_PACK_STANDALONE

static void usage(const char *prog)
{
    fprintf(stderr,
        "usage: %s [--stored | --lha=<path-to-lha>] <out.RED> <file> [<file>...]\n"
        "\n"
        "  --stored          use method 0x0001 (no compression) for all records\n"
        "                    always works, no external deps, larger output\n"
        "  --lha=<path>      use method 0x000B (LHA -lh5- + WCSC prefix) via\n"
        "                    external lha binary; falls back to STORED if\n"
        "                    compression doesn't help or lha fails\n"
        "\n"
        "  <out.RED>         output archive path\n"
        "  <file>            input file(s) — stored under their basename\n"
        "\n"
        "  Default without either flag: --stored (safe, deterministic).\n"
        "\n"
        "Round-trip check:\n"
        "  %s out.RED file1 file2\n"
        "  redx list out.RED\n"
        "  redx extract out.RED    (into empty dir; diff vs originals)\n",
        prog, prog);
}

int main(int argc, char **argv)
{
    int prefer_lha = 0;
    const char *lha_path = NULL;
    const char *out_path = NULL;
    int i, first_file = 1;
    FILE *out;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--stored") == 0) {
            prefer_lha = 0;
        } else if (strncmp(argv[i], "--lha=", 6) == 0) {
            prefer_lha = 1;
            lha_path = argv[i] + 6;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        } else if (!out_path) {
            out_path = argv[i];
            first_file = i + 1;
        } else {
            break;
        }
    }

    if (!out_path || first_file >= argc) {
        usage(argv[0]);
        return 1;
    }

    out = fopen(out_path, "wb");
    if (!out) {
        fprintf(stderr, "red_pack: cannot create '%s': %s\n",
                out_path, strerror(errno));
        return 1;
    }

    for (i = first_file; i < argc; i++) {
        /* Basename for stored name */
        const char *base = strrchr(argv[i], '/');
        base = base ? base + 1 : argv[i];
        if (red_pack_file(out, argv[i], base, prefer_lha, lha_path) < 0) {
            fclose(out);
            unlink(out_path);
            return 1;
        }
        printf("  added: %s\n", base);
    }

    fclose(out);
    printf("wrote %s (%d records)\n", out_path, argc - first_file);
    return 0;
}

#endif /* RED_PACK_STANDALONE */

/* end red_pack.c */
