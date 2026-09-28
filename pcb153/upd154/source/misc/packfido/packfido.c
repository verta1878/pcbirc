/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* PACKFIDO.C - compact AREAS.DAT, the 15.22-and-later Fido area file.       */
/*                                                                           */
/* pcbirc crew, 2026-09-23.  GPLv3.                                          */
/*                                                                           */
/* THIS IS THE UPGRADE 15.4 NEVER INCLUDED                                   */
/* ======================================                                    */
/* Clark shipped a PACKFIDO.EXE in the 15.41 install set and never updated   */
/* it.  His own DOC\PACK.DOC, copyright 1995, describes a utility "written   */
/* for PCBoard 15.21 to pack the FIDO configuration file PCBFIDO.CFG" - and  */
/* PCBFIDO.CFG stopped holding the areas at 15.22.  Three independent        */
/* pieces of evidence, none of them inference:                               */
/*                                                                           */
/*   1. VERSION STRINGS.  Every binary in that install set that touches      */
/*      current data carries 15.3 - PCBOARD, PCBOARDM, PCBSM, PCBPACK,       */
/*      PCBSETUP, MKPCBTXT, UUIN, UUOUT, UUXFER, PCBMODEM, PPLC330.          */
/*      PACKFIDO.EXE carries 15.0 and nothing later.                         */
/*   2. FIDOUTIL.EXE, the program that CONVERTS 15.21 data to 15.22,         */
/*      carries 15.0, 15.21 AND 15.22.  It knew about the new format.  The   */
/*      program whose whole job is packing the file FIDOUTIL had just        */
/*      converted never learned it.                                          */
/*   3. CONVERT.CPP, the converter's own source, has the call to packfido's  */
/*      one exported symbol commented out:  //do_pack();                     */
/*      The pack step was disabled at conversion time rather than fixed.     */
/*                                                                           */
/* So a 15.3 or 15.4 sysop had a PACKFIDO.EXE on disk, documented as a       */
/* 15.21 tool, that could not read the file it was pointed at.  This file    */
/* is the successor that was never written.                                  */
/*                                                                           */
/* ONE SOURCE, TWO TREES, TWO COMPILERS                                      */
/* ------------------------------------                                      */
/* This exact file is in two places and the two copies are byte identical:    */
/*                                                                           */
/*   pcb153\upd154\SOURCE\MISC\PACKFIDO\   Borland C++ 3.1, small model      */
/*   pcb154\MAIN\SOURCE\MISC\PACKFIDO\     OpenWatcom - DOS 16, OS/2, DOS32  */
/*                                                                           */
/* upd154 is Clark's 15.4 upgrade to 15.3 in source form - 15.3 is the base,  */
/* upd154 is the delta, the same shape the 15.4 binary patches had.  A        */
/* PACKFIDO that reads the current data structure IS a 15.4 upgrade to a      */
/* 15.3 board, so it belongs in the delta; pcb153\SOURCE stays pure.  And     */
/* 15.3 is where the evidence that it was needed lives - the shipped          */
/* PACKFIDO.EXE, DOC\PACK.DOC calling it a 15.21 tool, and CONVERT.CPP with   */
/* the pack step commented out.  The problem is documented in the base; the   */
/* fix is filed in the upgrade.                                               */
/*                                                                           */
/* Their output over the same AREAS.DAT is byte identical, which is the only  */
/* cross-check this program can have - see NO ORACLE below.                   */
/*                                                                           */
/* THE OTHER HALF                                                            */
/* --------------                                                            */
/* pcb153\SOURCE\MISC\PACKFIDO\PACKFIDO.C exists to come out byte for byte   */
/* identical to the PACKFIDO.EXE Clark shipped.  That binary is dated        */
/* 11-10-94 and reads 243-byte AREA_STRUCT records out of PCBFIDO.CFG - the  */
/* 15.21 layout.  15.22 moved the areas into their own file with a different */
/* record, so the shipped binary DOES NOTHING USEFUL on a 15.3 or 15.4       */
/* board, and a rebuild that matches it inherits that exactly.               */
/*                                                                           */
/* So this file is the other half: same program, current data structure.     */
/*                                                                           */
/*   AREAS.DAT                                                               */
/*     word   file version = 3                                               */
/*     n x    NAREA_STRUCT, records to end of file                           */
/*                                                                           */
/* No count and no trailer - cCONFIGBASE in DATA.HPP seeks by record number  */
/* past a 2-byte header and reads to EOF.  Nothing else is in the file.      */
/*                                                                           */
/* THE RECORD SIZE IS PROBED, NOT ASSUMED                                    */
/* --------------------------------------                                    */
/* Three sources give three answers and the file wins:                       */
/*                                                                           */
/*   STRUCTS.H, sizeof(NAREA_STRUCT)                       81                */
/*   FIDO.DOC, the record laid out over offsets 2..82      81                */
/*   Clark's own AREAS.DAT in pcb153\SOURCE\MISC\FIDOUTIL  71                */
/*                                                                           */
/*     48,566 - 2 = 48,564 ;  48,564 / 71 = 684.0000 exactly                 */
/*     684 of 684 records have a sane conference number and a printable,     */
/*     NUL-padded area tag.  At 81: 45 bytes left over, 9 of 599 sane.       */
/*                                                                           */
/* The 10 bytes are the trailing reserved[10].  It is in the header and in   */
/* the document; it is not in the data.  probe_v3() tries 81 first - what    */
/* both of those say - then 71, and takes the first size that divides the    */
/* file evenly AND scans clean.  It prints which one it used.                */
/*                                                                           */
/* That is also why the pack loop never declares a record variable.  It      */
/* reads recsize RAW BYTES and touches only the conference number at +0 and  */
/* the tag at +2.  Every byte it does not understand - including a           */
/* reserved[10] that may or may not be there - is copied through untouched.  */
/* A struct would have destroyed exactly the bytes in dispute.               */
/*                                                                           */
/* WHAT GETS DROPPED - the same three reasons as the original               */
/* ---------------------------------------------------------                */
/*   a. conference number above NumConf                                      */
/*   b. the conference is not a Fido conference                              */
/*   c. an area for that conference was ALREADY KEPT - first one wins,       */
/*      every later duplicate goes                                           */
/*                                                                           */
/* (c) was read out of the bit-CLEAR at 0x2F75 in the shipped binary and     */
/* then confirmed by experiment: given twelve conferences three and four     */
/* times in shuffled order, Clark's binary drops exactly the later ones.     */
/*                                                                           */
/* A conference is a Fido conference when it has a name, has a message base, */
/* and has ConfType 5.  Name and MsgFile come from CNAMES.@@@ (oldconftype,  */
/* 548 bytes, after a 2-byte RecSize header, indexed by conference number    */
/* FROM 0); ConfType comes from CNAMES.ADD (addconftype, 256 bytes, no       */
/* header).  pcbconftype's 739 bytes are a MERGE of the two - there is no    */
/* 739-byte file anywhere, and an earlier draft that read one was reading a  */
/* file that has never existed.                                              */
/*                                                                           */
/* NO ORACLE, AND SAYING SO                                                   */
/* ------------------------                                                  */
/* No shipped binary ever packed version 3, so there is nothing to diff      */
/* this against.  It is checked against Clark's own AREAS.DAT instead: 684   */
/* records at 71 bytes probed correctly, /R leaves the file untouched, an    */
/* injected duplicate is the record that dies, and the survivors match the   */
/* expected set byte for byte.  That is weaker than a byte diff and is       */
/* labelled as one.                                                          */
/*                                                                           */
/* usage: PACKFIDO <areas.dat> <cnames> <numconf> [/R]                       */
/*                                                                           */
/* Arguments rather than PCBOARD.DAT, deliberately: this one is not trying   */
/* to match anybody's binary, and a utility you can point at a copy of a     */
/* file is a utility you can test.  /R reports and writes nothing.           */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

/* ------------------------------------------------------------------------ */
/* PORTABILITY - Borland C++ 3.1, Turbo C 2.01, Microsoft C 7.0, OpenWatcom  */
/*                                                                           */
/* pcbword is EVERY field that is two bytes in the file.  It is not          */
/* decoration.  "unsigned int" is 2 bytes under Borland C 3.1, Turbo C, MSC  */
/* and OpenWatcom's 16-bit wcc, and 4 bytes under wcc386 and every 32-bit    */
/* target - so a conference number read through an "unsigned int *" would    */
/* swallow the next two bytes of the record the moment this is built for     */
/* OS/2 or 32-bit DOS, silently, with no diagnostic.  main() refuses to run  */
/* if pcbword is not 2 bytes.                                                */
/* ------------------------------------------------------------------------ */

typedef unsigned short pcbword;

#ifndef O_BINARY                /* some hosts have no text/binary distinction */
#define O_BINARY 0
#endif

#define AREA_SIZE           60      /* DEFINES.H:294  */
#define FIDO_CONFERENCE      5      /* DEFINES.H:72   */

#define OLDCONF_SIZE       548      /* sizeof(oldconftype), NEWDATA.H      */
#define ADDCONF_SIZE       256      /* sizeof(addconftype), NEWDATA.H      */
#define OC_MSGFILE_OFF    0x01B     /* char MsgFile[32] in CNAMES.@@@ (27) */
#define AC_TYPE_OFF       0x07F     /* char ConfType    in CNAMES.ADD (127)*/

/* Borland 3.1 has no pack(push)/pack(pop); OpenWatcom has both.  pack(1)  */
/* followed by pack() restores the default on all four compilers.           */
#pragma pack(1)
typedef struct {
  pcbword        ConfNum;
  char           AreaTag[AREA_SIZE];
  pcbword        AkaIndex;
  pcbword        OriginIndex;
  char           HighAscii;
  pcbword        LastActivityDate;
  char           AllowPrivate;
  char           AllowFileAttach;
  char           reserved[10];
} NAREA_STRUCT;                     /* 81 in the header; 71 on disk        */
#pragma pack()

#define RECSIZE_HDR      81
#define RECSIZE_DSK      71
#define MAXRECSIZE       81

#define REC_CONF(b)     (*(pcbword *)(b))
#define REC_TAG(b)      ((char *)(b) + 2)

#define TMPNAME     "tmp.dat"

static unsigned int    NumConf;
static unsigned char  *ConfMap;
static int             ReportOnly;

static void conf_set(unsigned int c)
{
    ConfMap[c >> 3] |= (unsigned char)(1 << (c & 7));
}

static int conf_avail(unsigned int c)
{
    return (ConfMap[c >> 3] >> (c & 7)) & 1;
}

static void conf_take(unsigned int c)
{
    ConfMap[c >> 3] &= (unsigned char) ~(1 << (c & 7));
}

/* --- CNAMES.@@@ + CNAMES.ADD: set a bit per Fido conference -------------- */
/* 1 ok, 0 could not open, -1 wrong format (already reported).              */

static int scan_conferences(char *cnfbase)
{
    char           name1[80], name2[80], *p;
    int            fdc, fda, haveadd;
    unsigned int   i;
    pcbword        recsize;
    unsigned char  oc[OLDCONF_SIZE];
    unsigned char  ac[ADDCONF_SIZE];
    char           conftype;

    strncpy(name1, cnfbase, sizeof(name1) - 5);
    name1[sizeof(name1) - 5] = '\0';
    p = strrchr(name1, '.');
    if (p != NULL && (stricmp(p, ".@@@") == 0 || stricmp(p, ".ADD") == 0))
        *p = '\0';
    strcpy(name2, name1);
    strcat(name1, ".@@@");
    strcat(name2, ".ADD");

    fdc = open(name1, O_RDONLY | O_BINARY);
    if (fdc == -1) return 0;

    if (read(fdc, &recsize, 2) != 2) { close(fdc); return 0; }
    if (recsize != OLDCONF_SIZE) {
        printf("\n%s file is formatted wrong - run PCBSETUP!\n", name1);
        close(fdc);
        return -1;
    }

    fda     = open(name2, O_RDONLY | O_BINARY);
    haveadd = (fda != -1);
    if (!haveadd)
        printf("\nno %s - no conference has a ConfType, so none is Fido", name2);

    for (i = 0; i <= NumConf; i++) {
        if (lseek(fdc, (long) i * OLDCONF_SIZE + 2L, SEEK_SET) == -1L) break;
        if (read(fdc, oc, OLDCONF_SIZE) != OLDCONF_SIZE) break;

        conftype = '\0';
        if (haveadd) {
            if (lseek(fda, (long) i * ADDCONF_SIZE, SEEK_SET) != -1L &&
                read(fda, ac, ADDCONF_SIZE) == ADDCONF_SIZE)
                conftype = (char) ac[AC_TYPE_OFF];
        }

        if (oc[0] != '\0' && oc[OC_MSGFILE_OFF] != '\0' &&
            conftype == FIDO_CONFERENCE)
            conf_set(i);
    }

    if (haveadd) close(fda);
    close(fdc);
    return 1;
}

/* --- probe the record size ---------------------------------------------- */

static int sane_at(int fd, long body, pcbword size)
{
    unsigned char  buf[MAXRECSIZE];
    pcbword        conf;
    long           recs, i;
    int            n, j, seen_nul;
    char          *tag;

    if (body <= 0L || (body % (long) size) != 0L) return 0;
    recs = body / (long) size;

    if (lseek(fd, 2L, SEEK_SET) == -1L) return 0;

    for (i = 0; i < recs; i++) {
        n = read(fd, buf, size);
        if (n != (int) size) return 0;

        conf = REC_CONF(buf);
        if (conf == 0 || conf > 0x7FFFU) return 0;

        tag = REC_TAG(buf);
        if (tag[0] == '\0') return 0;

        seen_nul = 0;
        for (j = 0; j < AREA_SIZE; j++) {
            if (tag[j] == '\0') { seen_nul = 1; continue; }
            if (seen_nul) return 0;
            if ((unsigned char) tag[j] < ' ') return 0;
        }
    }
    return 1;
}

static pcbword probe_v3(int fd, long filesize)
{
    long body = filesize - 2L;

    if (sane_at(fd, body, RECSIZE_HDR)) return RECSIZE_HDR;
    if (sane_at(fd, body, RECSIZE_DSK)) return RECSIZE_DSK;
    return 0;
}

/* --- the pack ----------------------------------------------------------- */

static long pack_areas(char *areafile)
{
    int            src, dst;
    pcbword        version, recsize;
    unsigned int   kept, count, i;
    pcbword        conf;
    unsigned long  total;
    long           filesize, dropped = 0L;
    unsigned char  rec[MAXRECSIZE];

    if (access(areafile, 0) != 0) return -1L;

    src = open(areafile, O_RDONLY | O_BINARY);
    if (src == -1) return -1L;

    filesize = filelength(src);

    if (read(src, &version, 2) != 2) { close(src); return -1L; }

    if (version != 3) {
        printf("\n%s has file version %u.  This packs version 3, the 15.22"
               "\nand later AREAS.DAT.  Version 2 is PCBFIDO.CFG and belongs"
               "\nto the 15.21 program in pcb153.\n", areafile, (unsigned) version);
        close(src);
        return -2L;
    }

    recsize = probe_v3(src, filesize);
    if (recsize == 0) {
        printf("\n%s is version 3 but its %ld byte record area divides by"
               "\nneither %d nor %d - refusing to touch it.\n",
               areafile, filesize - 2L, RECSIZE_HDR, RECSIZE_DSK);
        close(src);
        return -2L;
    }

    total = (unsigned long)((filesize - 2L) / (long) recsize);
    if (total > 0xFFFFUL) {
        printf("\n%s holds %lu records - more than a 16-bit count can"
               "\naddress.  Refusing to touch it.\n", areafile, total);
        close(src);
        return -2L;
    }
    count = (unsigned int) total;
    lseek(src, 2L, SEEK_SET);

    printf("\nfile version %u, %u byte records",
           (unsigned) version, (unsigned) recsize);
    if (recsize == RECSIZE_DSK)
        printf(" (no trailing reserved[10])");

    if (ReportOnly) {
        dst = -1;
    } else {
        dst = open(TMPNAME, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IWRITE);
        if (dst == -1) { close(src); return -1L; }
        write(dst, &version, 2);
    }

    kept = 0;
    for (i = 0; i < count; i++) {
        if (read(src, rec, recsize) != (int) recsize) {
            close(src); if (dst != -1) close(dst); return -1L;
        }

        conf = REC_CONF(rec);

        if (conf <= NumConf && conf_avail(conf)) {
            if (!ReportOnly) write(dst, rec, recsize);
            kept++;
            conf_take(conf);
        } else {
            printf("\nremoved %5d %s...", (int) conf, REC_TAG(rec));
            dropped++;
        }
    }

    close(src);

    if (ReportOnly) {
        printf("\n\nreport only - %s not modified", areafile);
        printf("\n%u records read, %u kept, %ld removed", count, kept, dropped);
        return dropped;
    }

    close(dst);

    unlink(areafile);
    rename(TMPNAME, areafile);

    return dropped;
}

int main(int argc, char *argv[])
{
    long  dropped;
    int   a, npos, rc;
    char *areafile = NULL;
    char *cnames   = NULL;
    char *numconf  = NULL;

    if (sizeof(pcbword) != 2) {
        fprintf(stderr,
            "PACKFIDO: pcbword is %u bytes, not 2.  Every on-disk field in\n"
            "          AREAS.DAT is two bytes; refusing to touch it.\n",
            (unsigned) sizeof(pcbword));
        return 2;
    }

    if (sizeof(NAREA_STRUCT) != RECSIZE_HDR) {
        fprintf(stderr,
            "PACKFIDO: NAREA_STRUCT is %u bytes, not %d - wrong packing, or\n"
            "          AREA_SIZE changed.  Refusing to touch AREAS.DAT.\n",
            (unsigned) sizeof(NAREA_STRUCT), RECSIZE_HDR);
        return 2;
    }

    npos = 0;
    for (a = 1; a < argc; a++) {
        if (argv[a][0] == '/' || argv[a][0] == '-') {
            if (argv[a][1] == 'r' || argv[a][1] == 'R') { ReportOnly = 1; continue; }
            fprintf(stderr, "PACKFIDO: unknown switch %s\n", argv[a]);
            return 1;
        }
        switch (++npos) {
          case 1: areafile = argv[a]; break;
          case 2: cnames   = argv[a]; break;
          case 3: numconf  = argv[a]; break;
          default: break;
        }
    }

    if (npos < 3) {
        fprintf(stderr,
            "usage: PACKFIDO <areas.dat> <cnames> <numconf> [/R]\n"
            "\n"
            "  <areas.dat>  the 15.22-and-later Fido area file, version 3\n"
            "  <cnames>     the CNAMES base name, with or without an\n"
            "               extension; .@@@ and .ADD are both read\n"
            "  <numconf>    number of conferences\n"
            "  /R           report only, write nothing\n");
        return 1;
    }
    NumConf = (unsigned int) atoi(numconf);

    ConfMap = (unsigned char *) calloc((NumConf >> 3) + 2, 1);
    if (ConfMap == NULL) {
        printf("Unable to allocate memory");
        return 1;
    }
    memset(ConfMap, 0, (NumConf >> 3) + 2);

    printf("\nscanning conference configuration...");
    rc = scan_conferences(cnames);
    if (rc != 1) {
        if (rc == 0) printf("\nUnable to open %s\n", cnames);
        free(ConfMap);
        return 1;
    }

    printf("\npacking the fido area file...");
    dropped = pack_areas(areafile);

    free(ConfMap);

    if (dropped == -2L) return 1;
    if (dropped < 0L) {
        printf("\nUnable to open %s\n", areafile);
        return 1;
    }

    printf("\ndone.\n");
    return 0;
}
