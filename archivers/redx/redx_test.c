/*
 * redx_test.c — proof-of-concept .RED single-record extractor
 *
 * Reads COMMDRV.RED, seeks to the first record (COMMDV00.DRV @ offset 26),
 * pulls its 792-byte compressed payload, feeds it through LHA's LH5
 * decoder, writes the 1130-byte result to disk.
 *
 * If output matches the shipped COMMDV00.DRV byte-for-byte (or CRC16
 * matches Clark's header value 0x85F3), we've cracked the .RED format
 * with vendored LHA — and every other record in every .RED file
 * unlocks with the same code path.
 *
 * Build: gcc -o redx_test redx_test.c redx_glue.c \
 *          lha/src/{bitio,huf,shuf,dhuf,slide,larc,maketbl,maketree}.c \
 *          -I lha/src -DHAVE_CONFIG_H=0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lha.h"

int main(int argc, char **argv) {
    const char *red_path = argc > 1 ? argv[1] : "/tmp/commdrv-work/COMMDRV.RED";
    const char *out_path = "/tmp/commdrv-work/extract/COMMDV00_via_redx.bin";

    /* Record 1: COMMDV00.DRV
     *   name offset in file: 26
     *   header offset:       2
     *   compressed size:     792  (dword @ header+6, i.e. file+8)
     *   uncompressed size:   1130 (dword @ header+10, i.e. file+12)
     *   expected CRC16:      0x85F3
     *   method byte:         0x0B (LH5-family)
     *   compressed data at:  26 + 12 = 38 (name length is 12: "COMMDV00.DRV")
     */

    FILE *red = fopen(red_path, "rb");
    if (!red) { perror(red_path); return 1; }

    /* Skip past name to compressed payload */
    if (fseek(red, 38, SEEK_SET) != 0) { perror("seek"); return 1; }

    FILE *out = fopen(out_path, "wb");
    if (!out) { perror(out_path); return 1; }

    struct interfacing iface;
    iface.infile   = red;
    iface.outfile  = out;
    iface.original = 1130;
    iface.packed   = 792;
    iface.read_size = 0;
    iface.dicbit   = LZHUFF5_DICBIT;   /* 13 = 8KB dictionary */
    iface.method   = LZHUFF5_METHOD_NUM; /* method 5 */

    unsigned int crc = decode(&iface);

    fclose(red);
    fclose(out);

    printf("decode returned CRC16 = 0x%04x (Clark's header says 0x85F3)\n", crc);
    printf("wrote %s\n", out_path);
    return crc == 0x85F3 ? 0 : 2;
}
