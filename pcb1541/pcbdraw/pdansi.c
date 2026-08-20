/* ====================================================================
 * pdansi.c — ANSI X3.64 Escape Sequence Parser
 * ====================================================================
 * C port of sysop/0's pdansi.pas (from PabloDraw C# by cwensley).
 * Parses ANSI art files into a PDCanvas.
 *
 * Supported sequences:
 *   CSI n A       Cursor Up
 *   CSI n B       Cursor Down
 *   CSI n C       Cursor Forward
 *   CSI n D       Cursor Back
 *   CSI n E       Cursor Next Line
 *   CSI n F       Cursor Previous Line
 *   CSI n G       Cursor Horizontal Absolute
 *   CSI r;c H/f   Cursor Position
 *   CSI n J       Erase in Display
 *   CSI n K       Erase in Line
 *   CSI n S       Scroll Up
 *   CSI ... m     SGR (colors, bold, blink)
 *   CSI s         Save Cursor Position
 *   CSI u         Restore Cursor Position
 *   CSI ? 33 h/l  ICE Colors on/off
 *   CSI ? 7 h/l   Line Wrap on/off
 *
 * Copyright (C) 2026 pcbrevival contributors (GPLv3)
 * ==================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcbdraw.h"

/* ---- Internal helpers ---- */

static void clamp_cursor(AnsiParser *p)
{
    if (p->cur_x < p->clip_left)   p->cur_x = p->clip_left;
    if (p->cur_x > p->clip_right)  p->cur_x = p->clip_right;
    if (p->cur_y < p->clip_top)    p->cur_y = p->clip_top;
    if (p->cur_y > p->clip_bottom) p->cur_y = p->clip_bottom;
}

static void put_char(AnsiParser *p, unsigned char ch)
{
    PDCanvasElement e;

    if (p->cur_x >= p->clip_left && p->cur_x <= p->clip_right) {
        e.ch.ch = ch;
        e.attr = p->attr;
        canvas_set(p->canvas, p->cur_x, p->cur_y, e);
    }
    p->cur_x++;
    p->last_line_data = 1;

    if (p->cur_x > p->clip_right) {
        if (p->line_wrap) {
            p->cur_x = p->clip_left;
            p->cur_y++;
            p->last_line_data = 0;
            if (p->cur_y > p->clip_bottom) {
                canvas_scroll_up(p->canvas, 1);
                p->cur_y = p->clip_bottom;
            }
        } else {
            p->cur_x = p->clip_right;
        }
    }
}

static int parse_int(const char *s, int def)
{
    int val = 0, i = 0;
    if (!s || !s[0]) return def;
    while (s[i] >= '0' && s[i] <= '9') {
        val = val * 10 + (s[i] - '0');
        i++;
    }
    return (val == 0) ? def : val;
}

/* ---- SGR (Select Graphic Rendition) ---- */

static void process_sgr(AnsiParser *p, int *args, int argc)
{
    int i, v, tmp;

    if (argc == 0) {
        ATTR_INIT(p->attr, 7);
        return;
    }

    for (i = 0; i < argc; i++) {
        v = args[i];
        switch (v) {
        case 0:
            ATTR_INIT(p->attr, 7);
            break;
        case 1:
            p->attr.fg |= 0x08;
            break;
        case 2: case 22:
            p->attr.fg &= 0x07;
            break;
        case 5:
            p->attr.bg |= 0x08;
            break;
        case 25:
            p->attr.bg &= 0x07;
            break;
        case 7: case 27:
            tmp = ATTR_FG_ONLY(p->attr);
            p->attr.fg = (p->attr.fg & 0x08) | ATTR_BG_ONLY(p->attr);
            p->attr.bg = (p->attr.bg & 0x08) | (unsigned char)tmp;
            break;
        default:
            if (v >= 30 && v <= 37)
                p->attr.fg = (p->attr.fg & 0x08) | ansi_color_map[v - 30];
            else if (v >= 40 && v <= 47)
                p->attr.bg = (p->attr.bg & 0x08) | ansi_color_map[v - 40];
            break;
        }
    }
}

/* ---- CSI sequence parser ---- */

static void process_escape(AnsiParser *p, const unsigned char *buf,
                           long len, long *pos)
{
    char param[128];
    int plen = 0;
    unsigned char ch;
    int args[16];
    char arg_strs[16][16];
    int argc = 0;
    int v, i, start;

    /* Read parameter bytes until letter */
    while (*pos < len) {
        ch = buf[*pos];
        (*pos)++;
        if (ch >= 'A' && ch <= 'z')
            break;
        if (plen < 126) {
            param[plen++] = (char)ch;
        }
    }
    param[plen] = '\0';

    /* Split by ';' */
    argc = 0;
    start = 0;
    for (i = 0; i <= plen; i++) {
        if (i == plen || param[i] == ';') {
            if (argc < 16) {
                int slen = i - start;
                if (slen > 15) slen = 15;
                memcpy(arg_strs[argc], param + start, slen);
                arg_strs[argc][slen] = '\0';
                args[argc] = atoi(arg_strs[argc]);
                argc++;
            }
            start = i + 1;
        }
    }

    /* Handle empty param string — no args */
    if (plen == 0) argc = 0;

    switch (ch) {
    case 'A': /* Cursor Up */
        v = (argc > 0) ? parse_int(arg_strs[0], 1) : 1;
        p->cur_y -= v;
        if (p->cur_y < p->clip_top) p->cur_y = p->clip_top;
        break;

    case 'B': /* Cursor Down */
        v = (argc > 0) ? parse_int(arg_strs[0], 1) : 1;
        p->cur_y += v;
        if (p->cur_y > p->clip_bottom) p->cur_y = p->clip_bottom;
        break;

    case 'C': /* Cursor Forward */
        v = (argc > 0) ? parse_int(arg_strs[0], 1) : 1;
        p->cur_x += v;
        if (p->cur_x > p->clip_right) p->cur_x = p->clip_right;
        break;

    case 'D': /* Cursor Back */
        v = (argc > 0) ? parse_int(arg_strs[0], 1) : 1;
        p->cur_x -= v;
        if (p->cur_x < p->clip_left) p->cur_x = p->clip_left;
        break;

    case 'E': /* Cursor Next Line */
        p->cur_x = p->clip_left;
        v = (argc > 0) ? parse_int(arg_strs[0], 1) : 1;
        p->cur_y += v;
        while (p->cur_y > p->clip_bottom) {
            canvas_scroll_up(p->canvas, 1);
            p->cur_y--;
        }
        break;

    case 'F': /* Cursor Previous Line */
        p->cur_x = p->clip_left;
        v = (argc > 0) ? parse_int(arg_strs[0], 1) : 1;
        p->cur_y -= v;
        if (p->cur_y < p->clip_top) p->cur_y = p->clip_top;
        break;

    case 'G': /* Cursor Horizontal Absolute */
        v = (argc > 0) ? parse_int(arg_strs[0], 1) : 1;
        p->cur_x = p->clip_left + v - 1;
        clamp_cursor(p);
        break;

    case 'H': case 'f': /* Cursor Position */
        p->cur_y = p->clip_top +
            ((argc > 0 && arg_strs[0][0]) ? parse_int(arg_strs[0], 1) : 1) - 1;
        p->cur_x = p->clip_left +
            ((argc > 1 && arg_strs[1][0]) ? parse_int(arg_strs[1], 1) : 1) - 1;
        clamp_cursor(p);
        break;

    case 'J': /* Erase in Display */
        v = (argc > 0) ? args[0] : 0;
        if (v == 2) {
            ATTR_INIT(p->attr, 7);
            canvas_clear(p->canvas);
            p->cur_x = p->clip_left;
            p->cur_y = p->clip_top;
        }
        break;

    case 'K': /* Erase in Line */
        {
            PDCanvasElement clr;
            clr.ch.ch = 32;
            ATTR_INIT(clr.attr, 7);
            for (i = p->cur_x; i <= p->clip_right; i++)
                canvas_set(p->canvas, i, p->cur_y, clr);
        }
        break;

    case 'S': /* Scroll Up */
        v = (argc > 0) ? parse_int(arg_strs[0], 1) : 1;
        canvas_scroll_up(p->canvas, v);
        break;

    case 'h': /* Set Mode */
        for (i = 0; i < argc; i++) {
            if (strcmp(arg_strs[i], "?33") == 0) {
                p->ice_colors = 1;
                p->ice_detected = 1;
            }
            if (strcmp(arg_strs[i], "?7") == 0)
                p->line_wrap = 1;
        }
        break;

    case 'l': /* Reset Mode */
        for (i = 0; i < argc; i++) {
            if (strcmp(arg_strs[i], "?33") == 0) {
                p->ice_colors = 0;
                p->ice_detected = 1;
            }
            if (strcmp(arg_strs[i], "?7") == 0)
                p->line_wrap = 0;
        }
        break;

    case 'm': /* SGR */
        process_sgr(p, args, argc);
        break;

    case 's': /* Save Cursor */
        if (argc == 0) {
            p->save_x = p->cur_x;
            p->save_y = p->cur_y;
        }
        break;

    case 'u': /* Restore Cursor */
        if (argc == 0) {
            p->cur_x = p->save_x;
            p->cur_y = p->save_y;
        }
        break;
    }
}

/* ---- Public API ---- */

void ansi_init(AnsiParser *p)
{
    memset(p, 0, sizeof(AnsiParser));
    p->line_wrap = 1;
}

int ansi_load_buffer(AnsiParser *p, const unsigned char *buf, long len,
                     PDCanvas *canvas)
{
    long pos = 0;
    long data_len = len;
    unsigned char b, b2;

    p->canvas = canvas;
    p->clip_left = 0;
    p->clip_top = 0;
    p->clip_right = canvas->width - 1;
    p->clip_bottom = canvas->height - 1;
    ATTR_INIT(p->attr, 7);
    p->cur_x = 0;
    p->cur_y = 0;
    p->save_x = 0;
    p->save_y = 0;
    p->last_line_data = 0;

    /* Check for SAUCE — don't parse it as ANSI */
    if (data_len > 128) {
        if (memcmp(buf + data_len - 128, "SAUCE", 5) == 0)
            data_len -= 129;    /* exclude SAUCE + EOF byte */
    }

    while (pos < data_len) {
        b = buf[pos++];

        if (b == 27) {  /* ESC */
            if (pos >= data_len) break;
            b2 = buf[pos++];
            if (b2 == '[')
                process_escape(p, buf, data_len, &pos);
            else {
                /* Not CSI — output both bytes as characters */
                if (b != 10 && b != 13 && b != 26) put_char(p, b);
                if (b2 != 10 && b2 != 13 && b2 != 26) put_char(p, b2);
            }
        } else if (b == 10) {
            /* Line Feed */
            p->cur_y++;
            p->last_line_data = 0;
            if (p->cur_y > p->clip_bottom) {
                canvas_scroll_up(p->canvas, 1);
                p->cur_y = p->clip_bottom;
            }
            p->cur_x = p->clip_left;
        } else if (b == 13 || b == 26) {
            /* CR or EOF — ignore */
        } else {
            put_char(p, b);
        }
    }

    return 0;
}

int ansi_load_file(AnsiParser *p, const char *filename, PDCanvas *canvas)
{
    FILE *f;
    long len;
    unsigned char *buf;
    int rc;

    f = fopen(filename, "rb");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    buf = (unsigned char *)malloc(len);
    if (!buf) { fclose(f); return -1; }

    if ((long)fread(buf, 1, len, f) != len) {
        free(buf);
        fclose(f);
        return -1;
    }
    fclose(f);

    rc = ansi_load_buffer(p, buf, len, canvas);
    free(buf);
    return rc;
}

int ansi_get_final_y(AnsiParser *p)
{
    return p->last_line_data ? p->cur_y + 1 : p->cur_y;
}

/* ---- PCBoard @X codes ---- */

int pcboard_load_file(const char *filename, PDCanvas *canvas)
{
    FILE *f;
    int x = 0, y = 0;
    int ch;
    PDCanvasElement e;
    PDAttribute attr;

    ATTR_INIT(attr, 7);

    f = fopen(filename, "rb");
    if (!f) return -1;

    while ((ch = fgetc(f)) != EOF) {
        if (ch == '@' && (ch = fgetc(f)) == 'X') {
            /* @Xab — a=bg, b=fg in hex */
            int hi = fgetc(f);
            int lo = fgetc(f);
            if (hi != EOF && lo != EOF) {
                int hv = 0, lv = 0;
                if (hi >= '0' && hi <= '9') hv = hi - '0';
                else if (hi >= 'A' && hi <= 'F') hv = hi - 'A' + 10;
                else if (hi >= 'a' && hi <= 'f') hv = hi - 'a' + 10;
                if (lo >= '0' && lo <= '9') lv = lo - '0';
                else if (lo >= 'A' && lo <= 'F') lv = lo - 'A' + 10;
                else if (lo >= 'a' && lo <= 'f') lv = lo - 'a' + 10;
                ATTR_INIT(attr, (unsigned char)((hv << 4) | lv));
            }
            continue;
        }

        if (ch == '\n') {
            x = 0; y++;
            continue;
        }
        if (ch == '\r') continue;

        e.ch.ch = (short)ch;
        e.attr = attr;
        canvas_set(canvas, x, y, e);
        x++;
        if (x >= canvas->width) { x = 0; y++; }
    }

    fclose(f);
    return 0;
}

/* ---- Binary (raw char+attr pairs) ---- */

int binary_load_file(const char *filename, PDCanvas *canvas, int width)
{
    FILE *f;
    int x = 0, y = 0;
    unsigned char pair[2];
    PDCanvasElement e;

    f = fopen(filename, "rb");
    if (!f) return -1;

    while (fread(pair, 1, 2, f) == 2) {
        e.ch.ch = pair[0];
        ATTR_INIT(e.attr, pair[1]);
        canvas_set(canvas, x, y, e);
        x++;
        if (x >= width) { x = 0; y++; }
    }

    fclose(f);
    return 0;
}
