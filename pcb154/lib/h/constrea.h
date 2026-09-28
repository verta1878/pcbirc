/* constrea.h — Borland constream compatibility for Watcom */
#ifndef _CONSTREAM_H_COMPAT
#define _CONSTREAM_H_COMPAT
#ifdef __WATCOMC__
#include <stdio.h>
#include <string.h>


/* iostream manipulators for constream */
struct _constream_hex_t {};
struct _constream_dec_t {};
static _constream_hex_t _cs_hex;
static _constream_dec_t _cs_dec;
#define hex _cs_hex
#define dec _cs_dec
#define setw(n) n
#define setfill(c) c
/* Minimal constream stub — outputs to stdout */
/* endl for constream */
static const char _endl_char = '\n';
#define endl _endl_char
class constream {
public:
    constream() {}
    constream& operator<<(const char *s) { if(s) fputs(s, stdout); return *this; }
    constream& operator<<(char c) { putchar(c); return *this; }
    constream& operator<<(int n) { printf("%d", n); return *this; }
    constream& operator<<(unsigned n) { printf("%u", n); return *this; }
    constream& operator<<(long n) { printf("%ld", n); return *this; }
    constream& operator<<(unsigned long n) { printf("%lu", n); return *this; }
    constream& operator<<(void *p) { printf("%p", p); return *this; }
    constream& operator<<(_constream_hex_t) { return *this; }
    constream& operator<<(_constream_dec_t) { return *this; }
    void clrscr() {}
    void window(int,int,int,int) {}
};

/* conbuf stub for files that reference it directly */
class conbuf {
public:
    conbuf() {}
    int do_sputn(const char *s, int n) { for(int i=0;i<n;i++) putchar(s[i]); return n; }
};

#else
/* Borland: use native constrea.h */
#endif
#endif
