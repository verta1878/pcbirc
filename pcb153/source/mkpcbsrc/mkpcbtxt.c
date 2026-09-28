/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* mkpcbtxt.c                                                                */
/*                                                                           */
/* Reverse-engineered PCBoard 15.4 PCBTEXT upgrade utility.                  */
/*                                                                           */
/* Not byte-identical to the original 15.4b MKPCBTXT.EXE — this is a         */
/* behavioral recreation based on:                                           */
/*                                                                           */
/*   - The PCBTEXT file format documented in PCBTEXT.H (81-byte records,     */
/*     stored on disk as 80 bytes each: 1 byte color + 79 byte text)         */
/*   - The record 0 version stamp mechanism used by readpcbtextfile() in     */
/*     DISPLAY/PCBTEXT.C                                                     */
/*   - Command line strings extracted from MKPCBTXT.EXE:                     */
/*         "/UPGRADE"                        (mode switch, but implicit)     */
/*         "/I:"                             (input path prefix)             */
/*         "%s does not exist"               (input-not-found error)         */
/*         "%s has been upgraded"            (per-file success)              */
/*         "Record #%d has been upgraded in %s."   (per-record progress)     */
/*                                                                           */
/* Behaviour: reads an existing PCBTEXT.<ext> file, walks its records to     */
/* determine how many are present, then appends any records missing at the   */
/* tail up to TXT_NUMPROMPTS_154.  Rewrites record 0 with the 15.4 version   */
/* stamp so the runtime accepts it.                                          */
/*                                                                           */
/* Build:                                                                    */
/*   bcc -ml -c mkpcbtxt.c                                                   */
/*   tlink /L\B\C31\LIB;\PCBSRC\LIB\BCDOS\BC31  C0L mkpcbtxt.obj +           */
/*     misc_l.386 pcb_l.386 dos_l.386, mkpcbtxt.exe,, cl.lib                 */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "PCBTXT154.H"        /* generated table of 15.4 records */

/* On-disk PCBTEXT record: 1 byte color + 79 byte text, no NUL, space padded */
#define PCBTEXT_RECSIZE     80

/* Version stamp written into record 0.  Verbatim from the 15.4b MKPCBTXT
 * binary at data offset 0x07d7e.  readpcbtextfile() uses strstr(record0.Str,
 * "15.4") to validate the file version, so this string satisfies checks for
 * every PCBoard version from 14.5 through 15.4.
 */
static const char VERSION_STAMP[] =
    "PCBoard version 14.5 & 15.0 & 15.2 & 15.3 & 15.4 PCBTEXT file";

/* Copyright banner shown to stdout on start-up */
static const char BANNER[] =
    "PCBoard 15.4 PCBTEXT Upgrade Utility\n"
    "Copyright (C) 1988-1997 Clark Development Company, Inc.\n";


/*---------------------------------------------------------------------------
 *  Pad a PCBTEXT record: color byte + text, blank-padded to 79 bytes.
 *  buf must point to PCBTEXT_RECSIZE bytes.
 */
static void pack_record(char *buf, unsigned char color, const char *text)
{
    int len = strlen(text);
    if (len > PCBTEXT_RECSIZE - 1)
        len = PCBTEXT_RECSIZE - 1;
    buf[0] = (char) color;
    memcpy(&buf[1], text, len);
    memset(&buf[1 + len], ' ', PCBTEXT_RECSIZE - 1 - len);
}


/*---------------------------------------------------------------------------
 *  Count how many complete PCBTEXT records are stored in the file.  Used to
 *  determine where to start appending the 15.4 records.
 */
static long record_count(int fd)
{
    long size = lseek(fd, 0L, SEEK_END);
    if (size < 0)
        return -1;
    return size / PCBTEXT_RECSIZE;
}


/*---------------------------------------------------------------------------
 *  Upgrade one PCBTEXT.<ext> file in place.  Returns 0 on success, non-zero
 *  on any I/O failure.
 */
static int upgrade_pcbtext(const char *path)
{
    int  fd;
    long have, need, i;
    char buf[PCBTEXT_RECSIZE];

    if (access(path, 0) != 0) {                   /* file must already exist */
        printf("%s does not exist\n", path);
        return 1;
    }

    fd = open(path, O_RDWR | O_BINARY);
    if (fd < 0) {
        printf("Cannot open %s\n", path);
        return 2;
    }

    have = record_count(fd);
    need = TXT_NUMPROMPTS_154 + 1;                /* record 0 + N prompts */

    /* Rewrite record 0 with the 15.4 version stamp so the runtime accepts */
    /* the file after upgrade.                                             */
    lseek(fd, 0L, SEEK_SET);
    pack_record(buf, 0, VERSION_STAMP);
    write(fd, buf, PCBTEXT_RECSIZE);

    /* Append any missing records at the tail.  Records that already exist */
    /* from 15.3 are left untouched — sysops customize them.               */
    lseek(fd, 0L, SEEK_END);
    for (i = have; i < need; i++) {
        const pcbtext_default_t *def = &Pcbtext154Table[i];
        pack_record(buf, def->color, def->text);
        if (write(fd, buf, PCBTEXT_RECSIZE) != PCBTEXT_RECSIZE) {
            printf("Write error at record %ld in %s\n", i, path);
            close(fd);
            return 3;
        }
        printf("Record #%ld has been upgraded in %s.\n", i, path);
    }

    close(fd);
    printf("%s has been upgraded\n", path);
    return 0;
}


/*---------------------------------------------------------------------------
 *  Command line: MKPCBTXT PCBTEXT[.ext] [PCBTEXT[.ext] ...]
 *
 *  Undocumented switches (found in the 15.4b binary but not user-facing):
 *      /UPGRADE         - explicit upgrade mode (implicit if omitted)
 *      /I:<path>        - input file path prefix
 */
int main(int argc, char *argv[])
{
    int i;
    int rc = 0;
    int filecount = 0;

    fputs(BANNER, stdout);

    if (argc < 2) {
        fputs("Usage: MKPCBTXT PCBTEXT[.ext] [PCBTEXT[.ext] ...]\n", stdout);
        return 1;
    }

    for (i = 1; i < argc; i++) {
        /* Skip switches; only /I: adjusts subsequent behavior in the OG,
         * and /UPGRADE is a no-op since upgrade is the only mode here.
         */
        if (argv[i][0] == '/' || argv[i][0] == '-') {
            /* switches are accepted for command-line compatibility only */
            continue;
        }
        rc |= upgrade_pcbtext(argv[i]);
        filecount++;
    }

    if (filecount == 0) {
        fputs("No PCBTEXT files specified.\n", stdout);
        return 1;
    }
    return rc;
}
