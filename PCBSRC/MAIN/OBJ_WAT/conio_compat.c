/*
 * conio_compat.c — Borland conio functions for Watcom DOS4G
 * Uses BIOS INT 10h for text-mode screen operations.
 * pcbrevival Phase 0, August 2026
 */
#include <dos.h>
#include <string.h>
#include <i86.h>

static unsigned char _cur_x = 1, _cur_y = 1;  /* 1-based */
static unsigned char _win_x1=1, _win_y1=1, _win_x2=80, _win_y2=25;

/* Read cursor position via BIOS */
static void _bios_getpos(unsigned char *row, unsigned char *col) {
    union REGS r;
    r.h.ah = 0x03; r.h.bh = 0;
    int386(0x10, &r, &r);
    *row = r.h.dh; *col = r.h.dl;
}

/* Set cursor position via BIOS */
static void _bios_setpos(unsigned char row, unsigned char col) {
    union REGS r;
    r.h.ah = 0x02; r.h.bh = 0; r.h.dh = row; r.h.dl = col;
    int386(0x10, &r, &r);
}

void gotoxy(int x, int y) {
    _cur_x = (unsigned char)x;
    _cur_y = (unsigned char)y;
    _bios_setpos(y - 1, x - 1);  /* BIOS is 0-based */
}

int wherex(void) {
    unsigned char r, c;
    _bios_getpos(&r, &c);
    return c + 1;
}

int wherey(void) {
    unsigned char r, c;
    _bios_getpos(&r, &c);
    return r + 1;
}

/* clreol — clear to end of line */
void clreol(void) {
    union REGS r;
    unsigned char row, col;
    _bios_getpos(&row, &col);
    r.h.ah = 0x09;  /* write char+attr at cursor */
    r.h.al = ' ';
    r.h.bh = 0;
    r.h.bl = 0x07;  /* normal attribute */
    r.w.cx = _win_x2 - col;
    int386(0x10, &r, &r);
}

/* _setcursortype — 0=hidden, 1=normal, 2=block */
void _setcursortype(int type) {
    union REGS r;
    r.h.ah = 0x01;
    if (type == 0) { r.h.ch = 0x20; r.h.cl = 0x00; }       /* hidden */
    else if (type == 2) { r.h.ch = 0x00; r.h.cl = 0x07; }   /* block */
    else { r.h.ch = 0x06; r.h.cl = 0x07; }                    /* normal */
    int386(0x10, &r, &r);
}

/* Direct video memory access for puttext/movetext */
static unsigned short *_vidmem(void) {
    /* DOS4G flat model — video memory at 0xB8000 */
    return (unsigned short *)0xB8000;
}

/* puttext — copy buffer to screen rectangle (1-based coords) */
int puttext(int left, int top, int right, int bottom, void *buf) {
    unsigned short *vid = _vidmem();
    unsigned short *src = (unsigned short *)buf;
    int r, c, cols = right - left + 1;
    for (r = top - 1; r <= bottom - 1; r++) {
        for (c = left - 1; c <= right - 1; c++) {
            vid[r * 80 + c] = *src++;
        }
    }
    return 1;
}

/* movetext — copy screen rectangle to another position */
int movetext(int left, int top, int right, int bottom, int newleft, int newtop) {
    unsigned short *vid = _vidmem();
    unsigned short buf[80*50];
    int r, c, cols = right - left + 1, rows = bottom - top + 1;
    /* copy out */
    for (r = 0; r < rows; r++)
        for (c = 0; c < cols; c++)
            buf[r * cols + c] = vid[(top - 1 + r) * 80 + (left - 1 + c)];
    /* copy in */
    for (r = 0; r < rows; r++)
        for (c = 0; c < cols; c++)
            vid[(newtop - 1 + r) * 80 + (newleft - 1 + c)] = buf[r * cols + c];
    return 1;
}
