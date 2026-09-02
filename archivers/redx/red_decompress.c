/* ============================================================================
 * red_decompress.c — WCSC .RED method 0x000B decompressor
 *
 * C port of Yoshi Watazaki's LHA -lh5- (huf.c + bitio.c) with the WCSC-
 * specific 2/3-byte prefix skip. Byte-perfect on 9/10 test vectors from
 * COMMDRV.RED extracted files. See refwork/decompress_v1.0.py for details
 * on the reverse-engineering and known issues.
 *
 * Public API:
 *     int red_decompress(const uint8_t *cmp, size_t cmp_len,
 *                        uint8_t *out, size_t out_len);
 *     Returns 0 on success, negative on error.
 *
 * License: GPLv3
 * Based on LHA source (public domain) by Haruyasu Yoshizaki.
 * ==========================================================================*/

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

/* --- Constants (match Yoshi's lha_macro.h / huf.h for -lh5-) --- */
#define REDX_DICBIT    13
#define REDX_DICSIZ    (1 << REDX_DICBIT)     /* 8192 */
#define REDX_NC        510
#define REDX_CBIT      9
#define REDX_NT        19
#define REDX_TBIT      5
#define REDX_NP        (REDX_DICBIT + 1)      /* 14 for lh5 */
#define REDX_PBIT      4
#define REDX_THRESHOLD 3

/* --- Bit I/O state (mirrors Yoshi's bitio.c static globals) --- */
typedef struct {
    const uint8_t *data;
    size_t         data_len;
    size_t         pos;
    uint16_t       bitbuf;
    uint8_t        subbitbuf;
    uint8_t        bitcount;
} redx_bitio_t;

static void redx_fillbuf(redx_bitio_t *b, unsigned int n) {
    while (n > b->bitcount) {
        n -= b->bitcount;
        b->bitbuf = (uint16_t)((b->bitbuf << b->bitcount) |
                               (b->subbitbuf >> (8 - b->bitcount)));
        b->subbitbuf = (b->pos < b->data_len) ? b->data[b->pos++] : 0;
        b->bitcount = 8;
    }
    b->bitcount = (uint8_t)(b->bitcount - n);
    b->bitbuf = (uint16_t)((b->bitbuf << n) |
                           (b->subbitbuf >> (8 - n)));
    b->subbitbuf = (uint8_t)(b->subbitbuf << n);
}

static uint16_t redx_getbits(redx_bitio_t *b, unsigned int n) {
    uint16_t x = (uint16_t)(b->bitbuf >> (16 - n));
    redx_fillbuf(b, n);
    return x;
}

static uint16_t redx_peekbits(redx_bitio_t *b, unsigned int n) {
    return (uint16_t)(b->bitbuf >> (16 - n));
}

static void redx_bitio_init(redx_bitio_t *b, const uint8_t *data, size_t len) {
    b->data = data; b->data_len = len; b->pos = 0;
    b->bitbuf = 0; b->subbitbuf = 0; b->bitcount = 0;
    redx_fillbuf(b, 16);
}

/* --- Huffman tree: array-of-nodes, allocated up-front --- *
 * Each node = 3 uint16: [left_child_idx, right_child_idx, value_or_0xFFFF]
 * Root = index 0. value == 0xFFFF means internal node.
 */
#define REDX_LEAF_MARK 0xFFFF

typedef struct {
    uint16_t *nodes;       /* 3 * capacity uint16 */
    unsigned int capacity;
    unsigned int next;
    int single_valid;
    uint16_t single_val;
} redx_tree_t;

static void redx_tree_reset(redx_tree_t *t, uint16_t *storage, unsigned int cap) {
    t->nodes = storage;
    t->capacity = cap;
    t->next = 1;                    /* index 0 is root */
    t->single_valid = 0;
    t->nodes[0] = 0;                /* left */
    t->nodes[1] = 0;                /* right */
    t->nodes[2] = REDX_LEAF_MARK;
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
        /* All-zero: no tree, decode returns 0 */
        t->single_valid = 1;
        t->single_val = 0;
        return 0;
    }
    if (max_len > 16) return -1;

    for (i = 0; i < n; i++)
        if (bitlen[i] > 0) bl_count[bitlen[i]]++;

    code = 0;
    for (i = 1; i <= max_len; i++) {
        code = (uint16_t)((code + bl_count[i - 1]) << 1);
        next_code[i] = code;
    }

    /* Reset root */
    t->next = 1;
    t->nodes[0] = 0;
    t->nodes[1] = 0;
    t->nodes[2] = REDX_LEAF_MARK;

    for (i = 0; i < n; i++) {
        cl = bitlen[i];
        if (cl == 0) continue;
        code = next_code[cl]++;

        node = 0;
        for (bit_i = cl; bit_i > 0; bit_i--) {
            bit = (code >> (bit_i - 1)) & 1;
            if (bit_i == 1) {
                /* Leaf */
                child = node * 3 + bit;
                if (t->nodes[t->nodes[child]] == 0 && t->nodes[t->nodes[child] + 1] == 0) {
                    /* fresh */
                }
                /* Simpler: allocate new node for leaf, mark it */
                if (t->nodes[child] == 0) {
                    if (t->next >= t->capacity) return -2;
                    t->nodes[child] = (uint16_t)t->next;
                    t->nodes[t->next * 3 + 0] = 0;
                    t->nodes[t->next * 3 + 1] = 0;
                    t->nodes[t->next * 3 + 2] = (uint16_t)i;
                    t->next++;
                } else {
                    t->nodes[t->nodes[child] * 3 + 2] = (uint16_t)i;
                }
            } else {
                /* Internal */
                child = node * 3 + bit;
                if (t->nodes[child] == 0) {
                    if (t->next >= t->capacity) return -3;
                    t->nodes[child] = (uint16_t)t->next;
                    t->nodes[t->next * 3 + 0] = 0;
                    t->nodes[t->next * 3 + 1] = 0;
                    t->nodes[t->next * 3 + 2] = REDX_LEAF_MARK;
                    t->next++;
                }
                node = t->nodes[child];
            }
        }
    }
    return 0;
}

static int redx_tree_set_single(redx_tree_t *t, uint16_t val) {
    t->single_valid = 1;
    t->single_val = val;
    return 0;
}

static int redx_tree_decode(redx_bitio_t *b, redx_tree_t *t) {
    unsigned int node = 0;
    uint16_t child_idx;
    if (t->single_valid) return t->single_val;

    for (;;) {
        if (t->nodes[node * 3 + 2] != REDX_LEAF_MARK)
            return t->nodes[node * 3 + 2];
        {
            unsigned int bit = redx_getbits(b, 1);
            child_idx = t->nodes[node * 3 + bit];
            if (child_idx == 0) return -1;
            node = child_idx;
        }
    }
}

/* --- Decoder state --- */
typedef struct {
    redx_bitio_t bio;
    uint16_t     blocksize;

    redx_tree_t  c_tree;
    redx_tree_t  pt_char_tree;
    redx_tree_t  pt_pos_tree;

    uint16_t     c_tree_nodes[REDX_NC * 4 * 3];      /* generous */
    uint16_t     pt_char_nodes[REDX_NT * 4 * 3];
    uint16_t     pt_pos_nodes[REDX_NP * 4 * 3];
} redx_decoder_t;

static int redx_read_pt_len(redx_decoder_t *d, redx_tree_t *tree,
                             uint16_t *nodes_storage, unsigned int nodes_cap,
                             short nn, short nbit, short i_special) {
    unsigned short n, c;
    unsigned short mask;
    unsigned int i;
    uint8_t pt_len[32] = {0};   /* NPT max = 19 */

    n = redx_getbits(&d->bio, nbit);
    redx_tree_reset(tree, nodes_storage, nodes_cap);

    if (n == 0) {
        c = redx_getbits(&d->bio, nbit);
        redx_tree_set_single(tree, c);
        return 0;
    }

    i = 0;
    while (i < n) {
        c = redx_peekbits(&d->bio, 3);
        if (c != 7) {
            redx_fillbuf(&d->bio, 3);
        } else {
            mask = (uint16_t)(1 << (16 - 4));
            while (mask & d->bio.bitbuf) {
                mask >>= 1;
                c++;
            }
            redx_fillbuf(&d->bio, c - 3);
        }
        pt_len[i++] = (uint8_t)c;
        if ((short)i == i_special) {
            int skip = redx_getbits(&d->bio, 2);
            /* Yoshi's pre-decrement while (--skip >= 0 && i < NPT) */
            while (1) {
                skip--;
                if (skip < 0 || (short)i >= nn) break;
                pt_len[i++] = 0;
            }
        }
    }
    /* zero rest — pt_len already 0 */
    return redx_tree_build(tree, pt_len, nn);
}

static int redx_read_c_len(redx_decoder_t *d) {
    unsigned short n, c;
    unsigned int i;
    static uint8_t c_len[REDX_NC];   /* static to avoid stack blowup */

    memset(c_len, 0, sizeof(c_len));
    n = redx_getbits(&d->bio, REDX_CBIT);
    redx_tree_reset(&d->c_tree, d->c_tree_nodes,
                    sizeof(d->c_tree_nodes) / (3 * sizeof(uint16_t)));

    if (n == 0) {
        c = redx_getbits(&d->bio, REDX_CBIT);
        redx_tree_set_single(&d->c_tree, c);
        return 0;
    }

    i = 0;
    while (i < n && i < REDX_NC) {
        int dec = redx_tree_decode(&d->bio, &d->pt_char_tree);
        if (dec < 0) return -1;
        c = (unsigned short)dec;
        if (c <= 2) {
            int cnt;
            if (c == 0)      cnt = 1;
            else if (c == 1) cnt = redx_getbits(&d->bio, 4) + 3;
            else             cnt = redx_getbits(&d->bio, REDX_CBIT) + 20;
            while (1) {
                cnt--;
                if (cnt < 0 || i >= REDX_NC) break;
                c_len[i++] = 0;
            }
        } else {
            c_len[i++] = (uint8_t)(c - 2);
        }
    }
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
    if (d->blocksize == 0) {
        int rc = redx_start_new_block(d);
        if (rc < 0) return rc;
    }
    d->blocksize--;
    return redx_tree_decode(&d->bio, &d->c_tree);
}

static int redx_decode_p(redx_decoder_t *d) {
    int j = redx_tree_decode(&d->bio, &d->pt_pos_tree);
    if (j < 0) return -1;
    if (j != 0)
        j = (1 << (j - 1)) + redx_getbits(&d->bio, j - 1);
    return j;
}

/* --- Public API --- */
int red_decompress(const uint8_t *cmp, size_t cmp_len,
                   uint8_t *out, size_t out_len) {
    static redx_decoder_t d;   /* static: avoid stack; not thread-safe */
    uint8_t ring[REDX_DICSIZ];
    unsigned int ring_pos = 0;
    size_t out_pos = 0;
    unsigned int skip;

    if (cmp_len < 3) return -1;

    /* WCSC 2/3-byte prefix: 3 bytes if first byte is 0x00, else 2 */
    skip = (cmp[0] == 0) ? 3 : 2;
    if (cmp_len <= skip) return -2;

    memset(&d, 0, sizeof(d));
    d.blocksize = 0;
    redx_bitio_init(&d.bio, cmp + skip, cmp_len - skip);

    memset(ring, ' ', sizeof(ring));

    while (out_pos < out_len) {
        int c = redx_decode_c(&d);
        int p;
        int count, i;
        unsigned int src;

        if (c < 0) return -3;

        if (c < 256) {
            out[out_pos++] = (uint8_t)c;
            ring[ring_pos] = (uint8_t)c;
            ring_pos = (ring_pos + 1) & (REDX_DICSIZ - 1);
        } else {
            count = c - 256 + REDX_THRESHOLD;
            p = redx_decode_p(&d);
            if (p < 0) return -4;
            src = (ring_pos - (unsigned int)p - 1) & (REDX_DICSIZ - 1);
            for (i = 0; i < count; i++) {
                uint8_t b = ring[(src + i) & (REDX_DICSIZ - 1)];
                out[out_pos++] = b;
                ring[ring_pos] = b;
                ring_pos = (ring_pos + 1) & (REDX_DICSIZ - 1);
                if (out_pos >= out_len) break;
            }
        }
    }
    return 0;
}

#ifdef REDX_STANDALONE_TEST
/* Test harness: reads payload + oracle, compares.
 * Compile: cc -o red_test red_decompress.c -DREDX_STANDALONE_TEST
 * Run:     ./red_test payload.bin oracle.bin
 */
#include <stdio.h>

static uint8_t *read_file(const char *path, size_t *len_out) {
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    size_t n;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    n = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = (uint8_t *)malloc(n);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, n, f) != n) { free(buf); fclose(f); return NULL; }
    fclose(f);
    *len_out = n;
    return buf;
}

int main(int argc, char **argv) {
    size_t cmp_len, or_len;
    uint8_t *cmp, *oracle, *out;
    int rc;
    size_t i, match = 0;

    if (argc != 3) {
        fprintf(stderr, "usage: %s payload.bin oracle.bin\n", argv[0]);
        return 1;
    }

    cmp = read_file(argv[1], &cmp_len);
    oracle = read_file(argv[2], &or_len);
    if (!cmp || !oracle) return 2;

    out = (uint8_t *)malloc(or_len);
    rc = red_decompress(cmp, cmp_len, out, or_len);
    if (rc < 0) {
        fprintf(stderr, "decompress error: %d\n", rc);
        return 3;
    }

    for (i = 0; i < or_len; i++) {
        if (out[i] != oracle[i]) break;
        match++;
    }
    if (match == or_len)
        printf("OK: %zu bytes byte-perfect match\n", or_len);
    else
        printf("MISMATCH: first %zu bytes match, then diverge\n", match);
    return (match == or_len) ? 0 : 4;
}
#endif
