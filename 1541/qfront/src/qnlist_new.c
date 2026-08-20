/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* qnlist.c -- Nodelist Compiler                                            */
/*                                                                           */
/* Replaces QNLIST.EXE (255 KB). Compiles FidoNet nodelists:                 */
/*   /COMPILE        Compile primary + all private nodelists                 */
/*   /COMPILENEW     Compile only if new diffs found                         */
/*                                                                           */
/* Operations:                                                               */
/*   1. Find archived nodediff in inbound                                    */
/*   2. Unarchive nodediff                                                   */
/*   3. Verify old nodelist CRC                                              */
/*   4. Apply nodediff to produce new nodelist                               */
/*   5. Verify new nodelist CRC                                              */
/*   6. Compile nodelist to binary index (.NDX)                              */
/*   7. Process private nodelists/pointlists                                 */
/*   8. Move/archive processed files                                         */
/*                                                                           */
/* From binary:                                                              */
/*   "Checking CRC of old nodelist...DONE"                                  */
/*   "Applying nodediff"                                                    */
/*   "Processing nodelist update...DONE"                                    */
/*   "New nodelist fails CRC check after nodediff update"                   */
/*   "Old nodelist fails CRC check"                                         */
/*   "No new nodelists/nodediffs found"                                     */
/*                                                                           */
/* Clean-room from FTS-5001 (nodelist format) + binary analysis.             */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "qfront.h"

#define QNLIST_VERSION "1.0.0"


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                     CRC-16 Nodelist Validation                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* nl_crc16() -- CRC-16/CCITT for nodelist validation                   */
/*-----------------------------------------------------------------------*/

static uint16_t nl_crc16(const void *Data, int Len)
{
    const unsigned char *p = (const unsigned char *)Data;
    uint16_t Crc = 0;                   /* running CRC value             */
    int i, j;                           /* byte and bit loop indices     */

    for (i = 0; i < Len; i++) {
        Crc ^= (uint16_t)p[i] << 8;
        for (j = 0; j < 8; j++) {
            if (Crc & 0x8000)
                Crc = (Crc << 1) ^ 0x1021;
            else
                Crc <<= 1;
        }
    }
    return Crc;
}


/*-----------------------------------------------------------------------*/
/* nl_file_crc() -- Calculate CRC of a nodelist file                    */
/*                                                                       */
/* FTS-5001: CRC is over all bytes except the first line (which          */
/* contains the CRC itself). The first line format is:                   */
/*   ";A ... -- Day number : XXXXX"                                      */
/* where XXXXX is the stored CRC value.                                  */
/*-----------------------------------------------------------------------*/

static uint16_t nl_file_crc(const char *Path, uint16_t *StoredCrc)
{
    FILE    *f;                         /* nodelist file handle           */
    char     Line[512];                 /* line read buffer              */
    uint16_t Crc = 0;                   /* running CRC                   */
    int      First = 1;                 /* first line flag               */

    f = fopen(Path, "r");
    if (!f) return 0xFFFF;

    while (fgets(Line, sizeof(Line), f)) {
        int Len = (int)strlen(Line);    /* line length                   */

        /* Strip trailing CR/LF for CRC calculation */
        while (Len > 0 && (Line[Len-1] == '\n' || Line[Len-1] == '\r'))
            Len--;

        if (First) {
            /* First line: extract stored CRC from last field */
            First = 0;
            if (StoredCrc) {
                char *Colon = strrchr(Line, ':');
                if (Colon) {
                    while (*Colon == ':' || *Colon == ' ') Colon++;
                    *StoredCrc = (uint16_t)strtoul(Colon, NULL, 10);
                }
            }
            continue;
        }

        /* Skip comment lines (;) for CRC */
        if (Line[0] == ';') continue;

        Crc = nl_crc16(Line, Len);
    }

    fclose(f);
    return Crc;
}
