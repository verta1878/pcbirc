/*
 * ppedecrypt.c — Decrypt and dump PPE file contents.
 *
 * Implements Clark's PPE encryption from LIB/SOURCE/MISC/CRYPT.C:
 *   decrypt2: seed 0xDB24, XOR/ROR chain (PPL 3.01+)
 *   decrypt3: XOR with "DECOMPILERS SUCK!" key (PPL 3.30 only)
 *
 * Per-chunk decryption: each sVARINFO, string value, and cVARVAL
 * is encrypted separately with a fresh seed. NOT whole-file.
 *
 * File format (from NEWSCR.CPP open/save):
 *   48 bytes: header ("PCBoard Programming Language Executable  X.XX\r\n\x1a")
 *    2 bytes: varCount (unsigned short)
 *   For each variable:
 *     11 bytes: sVARINFO (encrypted per-chunk)
 *     If type == vtSTRING (7):
 *       2 bytes: string length (NOT encrypted)
 *       N bytes: string data (encrypted per-chunk)
 *     Else if type == vtFUNCTION (15) or vtPROCEDURE (16):
 *       12 bytes: function info (encrypted per-chunk)
 *     Else:
 *       12 bytes: cVARVAL (encrypted per-chunk)
 *    2 bytes: scriptSize (NOT encrypted)
 *    N bytes: script data (encrypted in 2047-byte chunks)
 *
 * Usage:
 *   ppedecrypt input.ppe           — dump variable table
 *   ppedecrypt -x input.ppe out.bin — extract decrypted bytecode
 *
 * Source: LIB/SOURCE/MISC/CRYPT.C, LIB/H/MISC.H, NEWSCR.CPP open()
 * hexadecimal, 2026-09-07 — pcbirc project
 * GPLv3 + Clark banner
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HDR_SIZE       48
#define VARINFO_SIZE   11
#define VARVAL_SIZE    12
#define SCRIPT_CHUNK   2047

/* Type definitions from VAR.HPP */
#define vtBOOLEAN    0
#define vtUNSIGNED   1
#define vtDATE       2
#define vtEDATE      3
#define vtINTEGER    4
#define vtMONEY      5
#define vtREAL       6
#define vtSTRING     7
#define vtTIME       8
#define vtBYTE       9
#define vtWORD      10
#define vtSBYTE     11
#define vtSWORD     12
#define vtBIGSTR    13
#define vtDREAL     14
#define vtFUNCTION  15
#define vtPROCEDURE 16
#define vtDDATE     17

static const char *type_names[] = {
    "BOOLEAN",  "UNSIGNED", "DATE",     "EDATE",
    "INTEGER",  "MONEY",    "REAL",     "STRING",
    "TIME",     "BYTE",     "WORD",     "SBYTE",
    "SWORD",    "BIGSTR",   "DREAL",    "FUNCTION",
    "PROCEDURE","DDATE"
};

/* "DECOMPILERS SUCK!" obfuscated (CRYPT.C line 361) */
#define SUCKLEN 17
static unsigned char Suck[SUCKLEN] = {
    0x8C,0x53,0xB8,0xA7,0x9E,0x0F,0x0A,0xCB,0x28,
    0x62,0x2D,0x50,0x7E,0x05,0x3D,0x4E,0x35
};

/*
 * decrypt2 — from CRYPT.C ASM version (line 308).
 * Each call uses a fresh seed 0xDB24.
 */
static void decrypt2(unsigned char *buf, int len)
{
    unsigned short bx = 0xDB24;  /* seed */
    int dx = len >> 1;           /* word count */
    unsigned char cl = 0;
    int i;

    for (i = 0; dx > 0; i++, dx--) {
        int wo = i * 2;
        unsigned short ax = buf[wo] | (buf[wo+1] << 8);
        unsigned short bp_save = ax;

        cl = ((bx & 0xFF) + (dx & 0xFF)) & 0xFF;

        /* ror ax, cl */
        {
            int rot = cl & 15;
            ax = (ax >> rot) | (ax << (16 - rot));
            ax &= 0xFFFF;
        }

        /* xor al,dl; xor ah,dl */
        {
            unsigned char dl = dx & 0xFF;
            unsigned char al = (ax & 0xFF) ^ dl;
            unsigned char ah = ((ax >> 8) & 0xFF) ^ dl;
            ax = al | (ah << 8);
        }

        /* xor ax, bx */
        ax ^= bx;

        buf[wo]   = ax & 0xFF;
        buf[wo+1] = (ax >> 8) & 0xFF;
        bx = bp_save;
    }

    /* odd byte */
    if (len & 1) {
        unsigned char al = buf[len-1];
        int rot;
        al ^= (bx & 0xFF);
        rot = cl & 7;
        al = (al >> rot) | (al << (8 - rot));
        al &= 0xFF;
        buf[len-1] = al;
    }
}

/*
 * decrypt3 = encrypt3 (XOR is self-inverse).
 * From CRYPT.C line 364.
 */
static void decrypt3(unsigned char *buf, int len)
{
    int recycle = SUCKLEN;
    unsigned char *p = Suck;
    int i;

    for (i = 0; i < len; i++) {
        buf[i] ^= (*p + ((len - i) & 0xFF)) & 0xFF;
        recycle--;
        if (recycle)
            p++;
        else {
            p = Suck;
            recycle = SUCKLEN;
        }
    }
}

static const char *get_type_name(int type)
{
    if (type >= 0 && type <= 17)
        return type_names[type];
    return "UNKNOWN";
}

static unsigned short read_u16(const unsigned char *p)
{
    return p[0] | (p[1] << 8);
}

static short read_s16(const unsigned char *p)
{
    return (short)(p[0] | (p[1] << 8));
}

static long read_s32(const unsigned char *p)
{
    return (long)(p[0] | (p[1]<<8) | ((long)p[2]<<16) | ((long)p[3]<<24));
}

int main(int argc, char *argv[])
{
    FILE *fp;
    unsigned char *data;
    long fsize;
    int extract = 0;
    const char *infile, *outfile = NULL;
    unsigned char header[HDR_SIZE];
    unsigned short var_count;
    int ppe_ver;
    int offset, i;

    if (argc < 2) {
        fprintf(stderr, "Usage: ppedecrypt input.ppe\n");
        fprintf(stderr, "       ppedecrypt -x input.ppe out.bin\n");
        return 1;
    }

    if (strcmp(argv[1], "-x") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: ppedecrypt -x input.ppe out.bin\n");
            return 1;
        }
        extract = 1;
        infile = argv[2];
        outfile = argv[3];
    } else {
        infile = argv[1];
    }

    fp = fopen(infile, "rb");
    if (!fp) {
        fprintf(stderr, "Cannot open %s\n", infile);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    data = (unsigned char *)malloc(fsize);
    if (!data) {
        fprintf(stderr, "Out of memory\n");
        fclose(fp);
        return 1;
    }
    fread(data, 1, fsize, fp);
    fclose(fp);

    /* Parse header */
    memcpy(header, data, HDR_SIZE);
    var_count = read_u16(data + HDR_SIZE);

    /* Extract version: "...  X.XX\r\n\x1a" at offset 42 */
    {
        char vbuf[8];
        memcpy(vbuf, header + 42, 4);
        vbuf[4] = '\0';
        /* "3.20" or " 3.20" — handle leading space */
        if (header[40] == ' ') header[40] = '0';
        ppe_ver = atoi((char*)header+40) * 100 + atoi((char*)header+43);
    }

    printf("File:      %s\n", infile);
    printf("Size:      %ld bytes\n", fsize);
    printf("Version:   %d.%02d\n", ppe_ver / 100, ppe_ver % 100);
    printf("Variables: %d\n\n", var_count);

    offset = HDR_SIZE + 2;  /* after header + varCount */

    for (i = 0; i < var_count && offset + VARINFO_SIZE <= fsize; i++) {
        unsigned char info[VARINFO_SIZE];
        short vid;
        unsigned char dims, vtype, flags;

        /* Decrypt sVARINFO (per-chunk, fresh seed) */
        memcpy(info, data + offset, VARINFO_SIZE);
        if (ppe_ver == 330) decrypt3(info, VARINFO_SIZE);
        if (ppe_ver >= 301) decrypt2(info, VARINFO_SIZE);
        offset += VARINFO_SIZE;

        vid   = read_s16(info);
        dims  = info[2];
        vtype = info[9];
        flags = info[10];

        if (vtype == vtSTRING) {
            /* String: 2-byte length (NOT encrypted) + data (encrypted) */
            short slen = read_s16(data + offset);
            offset += 2;

            if (slen > 0 && slen < 2048 && offset + slen <= fsize) {
                unsigned char *str = (unsigned char *)malloc(slen + 1);
                memcpy(str, data + offset, slen);
                if (ppe_ver == 330) decrypt3(str, slen);
                if (ppe_ver >= 301) decrypt2(str, slen);
                str[slen] = '\0';
                /* Trim trailing null for display */
                {
                    int dlen = slen;
                    while (dlen > 0 && str[dlen-1] == '\0') dlen--;
                    printf("  [%2d] id=%3d %-10s = \"%.60s\"\n",
                           i, vid, get_type_name(vtype), str);
                }
                free(str);
                offset += slen;
            } else {
                printf("  [%2d] id=%3d %-10s (empty)\n",
                       i, vid, get_type_name(vtype));
            }

        } else if (vtype == vtFUNCTION || vtype == vtPROCEDURE) {
            /* Function/Procedure: 12 bytes (encrypted) */
            unsigned char val[VARVAL_SIZE];
            memcpy(val, data + offset, VARVAL_SIZE);
            if (ppe_ver == 330) decrypt3(val, VARVAL_SIZE);
            if (ppe_ver >= 301) decrypt2(val, VARVAL_SIZE);
            offset += VARVAL_SIZE;

            printf("  [%2d] id=%3d %-10s args=%d vars=%d start=%d\n",
                   i, vid, get_type_name(vtype),
                   val[4], val[5],
                   read_u16(val + 6));

        } else {
            /* All other types: 12 bytes cVARVAL (encrypted) */
            unsigned char val[VARVAL_SIZE];
            long content = 0;
            memcpy(val, data + offset, VARVAL_SIZE);
            if (ppe_ver == 330) decrypt3(val, VARVAL_SIZE);
            if (ppe_ver >= 301) decrypt2(val, VARVAL_SIZE);
            offset += VARVAL_SIZE;

            content = read_s32(val + 4);

            if (vtype == vtBOOLEAN)
                printf("  [%2d] id=%3d %-10s = %s\n",
                       i, vid, get_type_name(vtype),
                       content ? "TRUE" : "FALSE");
            else if (vtype == vtINTEGER)
                printf("  [%2d] id=%3d %-10s = %ld\n",
                       i, vid, get_type_name(vtype), content);
            else
                printf("  [%2d] id=%3d %-10s\n",
                       i, vid, get_type_name(vtype));
        }
    }

    /* Script size (2 bytes, NOT encrypted) */
    if (offset + 2 <= fsize) {
        unsigned short script_size = read_u16(data + offset);
        offset += 2;
        printf("\nscriptSize: %d bytes\n", script_size);
        printf("Remaining:  %ld bytes (bytecode)\n", fsize - offset);

        if (extract && outfile) {
            /* Decrypt script data in 2047-byte chunks and write */
            FILE *out = fopen(outfile, "wb");
            if (!out) {
                fprintf(stderr, "Cannot create %s\n", outfile);
                free(data);
                return 1;
            }

            /* Write header + varcount unmodified */
            fwrite(data, 1, HDR_SIZE + 2, out);

            /* TODO: write decrypted variable table */

            /* Decrypt and write script chunks */
            {
                int remaining = fsize - offset;
                int pos = offset;
                while (remaining > 0) {
                    int chunk = remaining > SCRIPT_CHUNK ? SCRIPT_CHUNK : remaining;
                    unsigned char *buf = (unsigned char *)malloc(chunk);
                    memcpy(buf, data + pos, chunk);
                    if (ppe_ver == 330) decrypt3(buf, chunk);
                    if (ppe_ver >= 301) decrypt2(buf, chunk);
                    fwrite(buf, 1, chunk, out);
                    free(buf);
                    pos += chunk;
                    remaining -= chunk;
                    /* Handle RLE zero byte at chunk boundary */
                    if (chunk == SCRIPT_CHUNK && remaining > 0
                        && data[pos-1] == 0) {
                        pos++;
                        remaining--;
                    }
                }
            }

            fclose(out);
            printf("Decrypted bytecode written to %s\n", outfile);
        }
    }

    free(data);
    return 0;
}
