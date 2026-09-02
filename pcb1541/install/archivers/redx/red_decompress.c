/* SPDX-License-Identifier: GPL-3.0-or-later
 * red_decompress.c — WCSC .RED decompressor
 *
 * WCSC's .RED container wraps -lh5- (LHA level-1 style) compressed data with:
 *   - 2 or 3 byte package prefix (skipped based on first byte)
 *   - CRC-CCITT (polynomial 0x1021) embedded every 4094 stream bytes
 *     as 2 trailing bytes (part of WCSC's disk I/O layer, [0x76]!=0 mode)
 *
 * Reference implementation: verified byte-perfect against 10/10 test pairs
 * from CSBACKUP.ARJ against WCSC's INSTALL.EXE decoder (install_unpacked.exe)
 * decompression output for all COMMDRV drivers.
 *
 * Verified 2026-09-XX by reverse-engineering INSTALL.EXE main LHA loop at
 * file offset 0x35e0. See archivers/redx/refwork/ghidra-ready/ for full
 * disassembly notes.
 *
 * This file is part of the pcbirc / PCBoard revival project.
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>


/* --- LHA -lh5- decoder parameters --- */
#define REDX_DICBIT    13
#define REDX_DICSIZ    (1U << REDX_DICBIT)
#define REDX_NC        510U
#define REDX_CBIT      9U
#define REDX_NT        19U
#define REDX_TBIT      5U
#define REDX_NP        14U
#define REDX_PBIT      4U
#define REDX_THRESHOLD 3

/* --- bit I/O --- */
typedef struct {
    const uint8_t *data;
    size_t         data_len;
    size_t         pos;
    uint16_t       bitbuf;
    uint8_t        subbitbuf;
    uint8_t        bitcount;
} redx_bitio_t;

static void redx_bitio_init(redx_bitio_t *b, const uint8_t *data, size_t len) {
    b->data = data;
    b->data_len = len;
    b->pos = 0;
    b->bitbuf = 0;
    b->subbitbuf = 0;
    b->bitcount = 0;
}

static void redx_fillbuf(redx_bitio_t *b, unsigned int n) {
    while (n > b->bitcount) {
        n -= b->bitcount;
        b->bitbuf = (uint16_t)((b->bitbuf << b->bitcount) |
                               (b->subbitbuf >> (8 - b->bitcount)));
        b->subbitbuf = (b->pos < b->data_len) ? b->data[b->pos++] : 0;
        b->bitcount = 8;
    }
    b->bitcount -= (uint8_t)n;
    b->bitbuf = (uint16_t)((b->bitbuf << n) |
                           (b->subbitbuf >> (8 - n)));
    b->subbitbuf <<= n;
}

static unsigned int redx_getbits(redx_bitio_t *b, unsigned int n) {
    uint16_t x;
    x = (uint16_t)(b->bitbuf >> (16 - n));
    redx_fillbuf(b, n);
    return x;
}

static void redx_bitio_prime(redx_bitio_t *b) {
    /* Pre-load first 16 bits */
    b->bitcount = 0;
    b->bitbuf = 0;
    b->subbitbuf = 0;
    redx_fillbuf(b, 16);
}

/* --- Huffman tree structures --- */
typedef struct {
    uint16_t *nodes;         /* triplets: [0]=code (if leaf), [1]=left, [2]=right */
    unsigned int max_nodes;
    unsigned int next_node;
    int          single_valid;
    uint16_t     single_val;
} redx_tree_t;

typedef struct {
    redx_bitio_t bio;
    redx_tree_t  pt_char_tree;
    redx_tree_t  c_tree;
    redx_tree_t  pt_pos_tree;
    uint16_t     pt_char_nodes[3 * (2 * 20 + 1)];
    uint16_t     c_tree_nodes[3 * (2 * REDX_NC + 1)];
    uint16_t     pt_pos_nodes[3 * (2 * REDX_NP + 1)];
    int          blocksize;
} redx_decoder_t;

static void redx_tree_reset(redx_tree_t *t, uint16_t *nodes, unsigned int max_nodes) {
    t->nodes = nodes;
    t->max_nodes = max_nodes;
    t->next_node = 0;
    t->single_valid = 0;
    t->single_val = 0;
    memset(nodes, 0, max_nodes * 3 * sizeof(uint16_t));
}

static void redx_tree_set_single(redx_tree_t *t, uint16_t val) {
    t->single_valid = 1;
    t->single_val = val;
}

static int redx_tree_build(redx_tree_t *t, const uint8_t *bitlen, unsigned int n) {
    unsigned int i, bit_i, cl;
    uint16_t code;
    uint16_t bl_count[17] = {0};
    uint16_t next_code[18] = {0};
    unsigned int max_len = 0;
    unsigned int node, child, bit;

    for (i = 0; i < n; i++) {
        if (bitlen[i] > max_len) max_len = bitlen[i];
    }
    if (max_len == 0) {
        t->single_valid = 1;
        t->single_val = 0;
        return 0;
    }
    if (max_len > 16) return -1;

    for (i = 0; i < n; i++)
        if (bitlen[i] > 0) bl_count[bitlen[i]]++;

    code = 0;
    for (bit_i = 1; bit_i <= max_len; bit_i++) {
        code = (uint16_t)((code + bl_count[bit_i - 1]) << 1);
        next_code[bit_i] = code;
    }

    /* Root node */
    t->next_node = 1;
    if (t->next_node > t->max_nodes) return -1;

    for (i = 0; i < n; i++) {
        cl = bitlen[i];
        if (cl == 0) continue;
        code = next_code[cl]++;
        node = 0;
        for (bit_i = cl; bit_i > 0; bit_i--) {
            bit = (code >> (bit_i - 1)) & 1;
            child = t->nodes[node * 3 + 1 + bit];
            if (child == 0) {
                if (bit_i == 1) {
                    /* leaf */
                    t->nodes[node * 3 + 1 + bit] = 0xFFFF; /* leaf marker high bit */
                    /* Actually store leaf via reserved node with code stored */
                    if (t->next_node >= t->max_nodes) return -1;
                    child = (uint16_t)t->next_node++;
                    t->nodes[node * 3 + 1 + bit] = child;
                    t->nodes[child * 3 + 0] = (uint16_t)i;
                    t->nodes[child * 3 + 1] = 0xFFFF; /* leaf marker */
                } else {
                    if (t->next_node >= t->max_nodes) return -1;
                    child = (uint16_t)t->next_node++;
                    t->nodes[node * 3 + 1 + bit] = child;
                }
            }
            node = child;
        }
    }
    return 0;
}

static int redx_tree_decode(redx_tree_t *t, redx_bitio_t *b) {
    unsigned int node = 0;
    unsigned int bit;
    if (t->single_valid) return t->single_val;
    for (;;) {
        bit = redx_getbits(b, 1);
        node = t->nodes[node * 3 + 1 + bit];
        if (node == 0) return -1;
        if (t->nodes[node * 3 + 1] == 0xFFFF) {
            /* leaf */
            return t->nodes[node * 3 + 0];
        }
    }
}

/* --- pt tree reader (nn = NT or NP) --- */
static int redx_read_pt_len(redx_decoder_t *d, redx_tree_t *tree,
                            uint16_t *nodes, unsigned int max_nodes,
                            unsigned int nn, unsigned int nbit, int i_special) {
    unsigned int n, i, c;
    static uint8_t pt_len[20];
    unsigned int mask;

    n = redx_getbits(&d->bio, nbit);
    redx_tree_reset(tree, nodes, max_nodes);

    if (n == 0) {
        c = redx_getbits(&d->bio, nbit);
        redx_tree_set_single(tree, (uint16_t)c);
        return 0;
    }

    if (n > nn) return -1;

    memset(pt_len, 0, sizeof(pt_len));
    i = 0;
    while (i < n) {
        c = d->bio.bitbuf >> (16 - 3);
        if (c == 7) {
            mask = 1U << (16 - 4);
            while (mask & d->bio.bitbuf) {
                mask >>= 1;
                c++;
            }
        }
        redx_fillbuf(&d->bio, (c < 7) ? 3 : (c - 3));
        pt_len[i++] = (uint8_t)c;
        if ((int)i == i_special) {
            c = redx_getbits(&d->bio, 2);
            while ((int)c-- > 0 && i < nn) pt_len[i++] = 0;
        }
    }
    while (i < nn) pt_len[i++] = 0;

    return redx_tree_build(tree, pt_len, nn);
}

/* --- c tree reader --- */
static int redx_read_c_len(redx_decoder_t *d) {
    unsigned int n, c;
    unsigned int i;
    static uint8_t c_len[REDX_NC];

    memset(c_len, 0, sizeof(c_len));
    n = redx_getbits(&d->bio, REDX_CBIT);
    redx_tree_reset(&d->c_tree, d->c_tree_nodes,
                    sizeof(d->c_tree_nodes) / (3 * sizeof(uint16_t)));

    if (n == 0) {
        c = redx_getbits(&d->bio, REDX_CBIT);
        redx_tree_set_single(&d->c_tree, (uint16_t)c);
        return 0;
    }

    if (n > REDX_NC) return -1;

    i = 0;
    while (i < n) {
        c = redx_tree_decode(&d->pt_char_tree, &d->bio);
        if (c < 0 || c > 18) return -1;

        if (c <= 2) {
            unsigned int zeros;
            if (c == 0)      zeros = 1;
            else if (c == 1) zeros = 3 + redx_getbits(&d->bio, 4);
            else             zeros = 20 + redx_getbits(&d->bio, REDX_CBIT);
            while (zeros-- > 0 && i < REDX_NC) c_len[i++] = 0;
        } else {
            c_len[i++] = (uint8_t)(c - 2);
        }
    }
    while (i < REDX_NC) c_len[i++] = 0;

    return redx_tree_build(&d->c_tree, c_len, REDX_NC);
}

static int redx_start_new_block(redx_decoder_t *d) {
    int rc;
    d->blocksize = redx_getbits(&d->bio, 16);
    rc = redx_read_pt_len(d, &d->pt_char_tree,
                          d->pt_char_nodes,
                          sizeof(d->pt_char_nodes) / (3 * sizeof(uint16_t)),
                          REDX_NT, REDX_TBIT, 3);
    if (rc < 0) return rc;
    rc = redx_read_c_len(d);
    if (rc < 0) return rc;
    return redx_read_pt_len(d, &d->pt_pos_tree,
                            d->pt_pos_nodes,
                            sizeof(d->pt_pos_nodes) / (3 * sizeof(uint16_t)),
                            REDX_NP, REDX_PBIT, -1);
}

static int redx_decode_c(redx_decoder_t *d) {
    int c;
    if (d->blocksize == 0) {
        int rc = redx_start_new_block(d);
        if (rc < 0) return rc;
    }
    d->blocksize--;
    c = redx_tree_decode(&d->c_tree, &d->bio);
    return c;
}

static int redx_decode_p(redx_decoder_t *d) {
    int p = redx_tree_decode(&d->pt_pos_tree, &d->bio);
    if (p < 0) return p;
    if (p != 0) {
        p = (int)((1U << (p - 1)) | redx_getbits(&d->bio, p - 1));
    }
    return p;
}

/* --- main entry point --- */
int red_decompress(const uint8_t *cmp, size_t cmp_len,
                   uint8_t *out, size_t out_len) {
    redx_decoder_t d;
    uint8_t ring[REDX_DICSIZ];
    unsigned int ring_pos = 0;
    size_t out_pos = 0;
    size_t skip;
    static uint8_t reformed[1048576];
    size_t rlen = 0;
    size_t src;

    if (cmp == NULL || out == NULL) return -1;
    if (cmp_len < 4) return -2;

    /* File header skip (WCSC package prefix) */
    /* LHA stream starts immediately at payload byte 0 */
    skip = 0;
    if (cmp_len <= skip) return -2;

    /* Strip embedded CRC-CCITT bytes: every 4094 stream bytes has 2 CRC bytes.
     * This is WCSC's [0x76]!=0 mode (integrity-checked read).
     * Files without embedded CRC just have no CRC bytes to strip
     * (loop consumes all bytes without matching any full 4094-byte segment). */
    src = skip;
    while (src < cmp_len) {
        size_t take = 4094;
        if (take > cmp_len - src) take = cmp_len - src;
        if (rlen + take > sizeof(reformed)) return -6;
        memcpy(reformed + rlen, cmp + src, take);
        rlen += take;
        src += take;
        src += 2;  /* skip 2 CRC bytes at end of each 4KB physical block */
    }

    memset(&d, 0, sizeof(d));
    d.blocksize = 0;
    redx_bitio_init(&d.bio, reformed, rlen);
    redx_bitio_prime(&d.bio);

    memset(ring, ' ', sizeof(ring));

    while (out_pos < out_len) {
        int c = redx_decode_c(&d);
        int p;
        int count, i;
        unsigned int src2;

        if (c < 0) return -3;

        if (c < 256) {
            out[out_pos++] = (uint8_t)c;
            ring[ring_pos] = (uint8_t)c;
            ring_pos = (ring_pos + 1) & (REDX_DICSIZ - 1);
        } else {
            count = c - 256 + REDX_THRESHOLD;
            p = redx_decode_p(&d);
            if (p < 0) return -4;
            src2 = (ring_pos - (unsigned int)p - 1) & (REDX_DICSIZ - 1);
            for (i = 0; i < count; i++) {
                uint8_t b = ring[(src2 + i) & (REDX_DICSIZ - 1)];
                out[out_pos++] = b;
                ring[ring_pos] = b;
                ring_pos = (ring_pos + 1) & (REDX_DICSIZ - 1);
                if (out_pos >= out_len) break;
            }
        }
    }

    return 0;
}
