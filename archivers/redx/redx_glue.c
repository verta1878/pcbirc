/*
 * redx_glue.c — minimal stubs for LHA library symbols we don't need
 *
 * LHA's lharc.c defines lots of globals (bitbuf, subbitbuf_len, infile,
 * outfile, dicbit, origsize, compsize, decode_count, loc, dtext, etc.)
 * and helpers (xmalloc, xfopen, fatal_error, error). We're linking
 * bitio/huf/shuf/dhuf/slide/larc/maketbl/maketree without lharc.c, so
 * we provide the pieces those files reference.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lha.h"

/* Globals used by the decoder (owned normally by lharc.c) */
unsigned short  bitbuf;
unsigned char   subbitbuf, bitcount;
FILE           *infile, *outfile;
long            compsize, origsize;
int             quiet = 1;
int             quiet_mode = 1;
int             verbose = 0;
int             noexec = 0;
int             text_mode = 0;
int             extract_broken_archive = 0;
int             dump_lzss = 0;
char           *writing_filename = NULL;
off_t           decode_count;
long            reading_size;
unsigned char  *dtext;
unsigned int    dicsiz;
unsigned long   loc;
FILE           *fout = NULL;

void *xmalloc(size_t sz) {
    void *p = malloc(sz);
    if (!p) { fprintf(stderr, "xmalloc(%zu) failed\n", sz); exit(1); }
    return p;
}

void *xrealloc(void *p, size_t sz) {
    void *q = realloc(p, sz);
    if (!q) { fprintf(stderr, "xrealloc(%zu) failed\n", sz); exit(1); }
    return q;
}

FILE *xfopen(char *name, char *mode) {
    FILE *f = fopen(name, mode);
    if (!f) { perror(name); exit(1); }
    return f;
}

void fatal_error(char *fmt, ...) {
    va_list ap;
    fprintf(stderr, "fatal: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error(char *fmt, ...) {
    va_list ap;
    fprintf(stderr, "error: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

void warning(char *fmt, ...) {
    va_list ap;
    fprintf(stderr, "warning: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

/* Progress indicators — no-ops for our test harness */
void put_indicator(long int count) { (void)count; }
void start_indicator(char *name, long int size, char *msg, long int def_indicator_count) {
    (void)name; (void)size; (void)msg; (void)def_indicator_count;
}
void finish_indicator2(char *name, char *msg, int pos) {
    (void)name; (void)msg; (void)pos;
}
void finish_indicator(char *name, char *msg) { (void)name; (void)msg; }


/* More stubs */
int unpackable;
unsigned short dicbit;  /* actually a global variable, not const */

void lha_exit(int code) { exit(code); }

/* More LHA globals — encoder-side but referenced from decoder init paths */
unsigned short  maxmatch;
unsigned int    n_max;
unsigned char *text;

int verify_mode = 0;

unsigned int crctable[UCHAR_MAX + 1];
unsigned int crc;
