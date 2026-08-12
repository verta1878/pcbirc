/* ====================================================================
 * pdcanvas.c — Canvas, Palette, SAUCE (C port of pdtypes.pas/pdsauce.pas)
 * ====================================================================
 * Copyright (C) 2026 pcbrevival contributors (GPLv3)
 * ==================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcbdraw.h"

/* ---- Default EGA/VGA palette ---- */

const PDColor default_palette[16] = {
    {  0,   0,   0},    /* 0  Black        */
    {  0,   0, 170},    /* 1  Blue         */
    {  0, 170,   0},    /* 2  Green        */
    {  0, 170, 170},    /* 3  Cyan         */
    {170,   0,   0},    /* 4  Red          */
    {170,   0, 170},    /* 5  Magenta      */
    {170,  85,   0},    /* 6  Brown        */
    {170, 170, 170},    /* 7  Light Gray   */
    { 85,  85,  85},    /* 8  Dark Gray    */
    { 85,  85, 255},    /* 9  Light Blue   */
    { 85, 255,  85},    /* 10 Light Green  */
    { 85, 255, 255},    /* 11 Light Cyan   */
    {255,  85,  85},    /* 12 Light Red    */
    {255,  85, 255},    /* 13 Light Magenta*/
    {255, 255,  85},    /* 14 Yellow       */
    {255, 255, 255}     /* 15 White        */
};

/* ---- ANSI SGR → DOS color map ---- */

const unsigned char ansi_color_map[8] = {0, 4, 2, 6, 1, 5, 3, 7};

/* ---- Canvas ---- */

PDCanvas *canvas_create(int width, int height)
{
    PDCanvas *c = (PDCanvas *)malloc(sizeof(PDCanvas));
    if (!c) return NULL;
    c->width = width;
    c->height = height;
    c->data = (PDCanvasElement *)calloc(width * height, sizeof(PDCanvasElement));
    if (!c->data) { free(c); return NULL; }
    canvas_clear(c);
    return c;
}

void canvas_free(PDCanvas *c)
{
    if (c) {
        if (c->data) free(c->data);
        free(c);
    }
}

void canvas_resize(PDCanvas *c, int width, int height)
{
    if (!c) return;
    if (c->data) free(c->data);
    c->width = width;
    c->height = height;
    c->data = (PDCanvasElement *)calloc(width * height, sizeof(PDCanvasElement));
    canvas_clear(c);
}

void canvas_clear(PDCanvas *c)
{
    int i;
    if (!c || !c->data) return;
    for (i = 0; i < c->width * c->height; i++) {
        c->data[i].ch.ch = 32;
        ATTR_INIT(c->data[i].attr, 7);
    }
}

void canvas_fill(PDCanvas *c, short ch, unsigned char attr)
{
    int i;
    if (!c || !c->data) return;
    for (i = 0; i < c->width * c->height; i++) {
        c->data[i].ch.ch = ch;
        ATTR_INIT(c->data[i].attr, attr);
    }
}

void canvas_scroll_up(PDCanvas *c, int lines)
{
    int i, move_count;
    if (!c || !c->data) return;
    if (lines >= c->height) { canvas_clear(c); return; }

    move_count = (c->height - lines) * c->width;
    for (i = 0; i < move_count; i++)
        c->data[i] = c->data[i + lines * c->width];

    for (i = move_count; i < c->height * c->width; i++) {
        c->data[i].ch.ch = 32;
        ATTR_INIT(c->data[i].attr, 7);
    }
}

void canvas_trim_height(PDCanvas *c, int new_height)
{
    if (!c) return;
    if (new_height < 1) new_height = 1;
    if (new_height < c->height)
        c->height = new_height;
    /* Don't realloc — just reduce logical height */
}

PDCanvasElement canvas_get(PDCanvas *c, int x, int y)
{
    PDCanvasElement e;
    if (c && c->data && x >= 0 && x < c->width && y >= 0 && y < c->height)
        return c->data[y * c->width + x];
    e.ch.ch = 32;
    ATTR_INIT(e.attr, 0);
    return e;
}

void canvas_set(PDCanvas *c, int x, int y, PDCanvasElement e)
{
    if (c && c->data && x >= 0 && x < c->width && y >= 0 && y < c->height)
        c->data[y * c->width + x] = e;
}

/* ---- SAUCE ---- */

int sauce_load(const char *filename, SauceRecord *sauce)
{
    FILE *f;
    long fsize;

    memset(sauce, 0, sizeof(SauceRecord));

    f = fopen(filename, "rb");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    if (fsize < 128) { fclose(f); return 0; }

    fseek(f, fsize - 128, SEEK_SET);
    if (fread(sauce, 1, 128, f) != 128) { fclose(f); return 0; }
    fclose(f);

    if (memcmp(sauce->id, "SAUCE", 5) != 0) {
        memset(sauce, 0, sizeof(SauceRecord));
        return 0;
    }

    return 1;
}

int sauce_get_width(const SauceRecord *s)
{
    if (s->datatype == 1 && s->tinfo1 > 0)
        return s->tinfo1;
    return 0;
}

int sauce_get_height(const SauceRecord *s)
{
    if (s->datatype == 1 && s->tinfo2 > 0)
        return s->tinfo2;
    return 0;
}

int sauce_get_ice(const SauceRecord *s)
{
    return (s->tflags & 0x01) ? 1 : 0;
}
