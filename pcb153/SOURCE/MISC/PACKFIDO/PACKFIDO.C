/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* PACKFIDO.C - compact the PCBoard FidoNet area configuration.              */
/*                                                                           */
/* RECONSTRUCTION, 2026-09-23, pcbirc crew.  GPLv3.                          */
/*                                                                           */
/* THE TARGET IS A BYTE-EXACT REBUILD, NOT A WORKALIKE                       */
/* ==================================================                        */
/* This is the 15.3 build: it is meant to come out byte for byte identical   */
/* to the PACKFIDO.EXE Clark shipped -                                       */
/*                                                                           */
/*   pcb1541\install\dist\target\PACKFIDO.EXE   23,214 bytes                 */
/*   sha256 ef584fc957a28d051a7c40937ed5eed916dfd043bc9ae7ca3f2996fc6a8f63a8 */
/*   Borland C++ banner 1991, internal date string 11-10-94                  */
/*                                                                           */
/* Nothing else counts as done.  An earlier draft of this file took the      */
/* config name, the CNAMES name and the conference count on the command      */
/* line and proved itself with a behavioural harness.  That was the wrong    */
/* target: Clark's program takes NO arguments, and a program that takes      */
/* arguments can never match his binary however well it behaves.             */
/*                                                                           */
/* WHAT THE BINARY IS, MEASURED                                              */
/* ----------------------------                                              */
/*   8 relocations, 512-byte header  -> SMALL MODEL, like Clark's other      */
/*                                      utilities (MKPCBTXT.EXE has 10)      */
/*   "Unable to open PCBOARD.DAT"    -> links the kit: readdatfile()         */
/*   "15.0" and no "14.5"            -> built WITHOUT -DLIB, the #else arm   */
/*                                      of the version check in DATAFIL2.C   */
/*   the three CNAMES messages       -> NOT in any library we hold, so they  */
/*                                      are in PACKFIDO.OBJ itself: the      */
/*                                      program opens CNAMES.@@@ / .ADD and  */
/*                                      checks the RecSize header on its own */
/*   "print scanf : floating point formats not linked"                       */
/*                                   -> -f-, no floating point              */
/*                                                                           */
/* So the shape is: readdatfile() for PCBOARD.DAT, then this file's own      */
/* CNAMES scan, then the pack.  No argv anywhere.  PcbData.CnfFile,          */
/* PcbData.FidoConfig and PcbData.NumConf come from PCBOARD.DAT lines 31,    */
/* 246 and 108 - and 246 is one of three lines PCBDAT.DOC calls "Reserved"   */
/* while DATAFILE.C reads FidoConfig there.                                  */
/*                                                                           */
/* BUILD - Clark's own utility recipe, from MKPCBTXT.MAK                     */
/* ----------------------------------------------------                     */
/*   BCC -c -ms -P -Oebglmptv -f- -ff- -C -K -G -O -Z -k- -d ...             */
/*   TLINK /x/c c0s.obj+PACKFIDO.OBJ,PACKFIDO.EXE,,<libs>+cs.lib             */
/*                                                                           */
/* -P matters: the kit libraries are C++ objects, so a C compile leaves      */
/* every kit symbol undefined.  See PACKFIDO.MAK.                            */
/*                                                                           */
/* WHERE IT STANDS - 1,276 BYTES SHORT, AND WE KNOW WHY                      */
/* ---------------------------------------------------                      */
/*   ours    21,938 bytes   small model, 512-byte header                     */
/*   Clark's 23,214 bytes                                                    */
/*                                                                           */
/* It does not link yet.  Four symbols are missing, all of them the KIT'S    */
/* own dependencies on category libraries that do not exist in small model:  */
/*                                                                           */
/*   retrycount()        MISC, wanted by CHKAPPEN                            */
/*   findstartofname()   MISC, wanted by DATAFIL2                            */
/*   _int23hnd           DOS,  wanted by HANDLERS                            */
/*   _int24hnd           DOS,  wanted by HANDLERS                            */
/*                                                                           */
/* toolkit\pwa153\bc31\lib holds MISC_L, DOS_L and the rest in LARGE model    */
/* only, plus PCBKBCS/C/M/L.  The small-model category libraries are the     */
/* "4 models" leg of the SDK matrix that is still open - TK.CFG has -ml      */
/* baked in, so MODEL is currently cosmetic.  Build those and this links.    */
/*                                                                           */
/* THIS FILE READS THE 15.21 LAYOUT ON PURPOSE                               */
/* -------------------------------------------                               */
/* Clark's shipped binary reads 243-byte AREA_STRUCT records out of          */
/* PCBFIDO.CFG - the 15.21 layout - and that is what a byte-exact rebuild    */
/* has to do too, even though 15.22 moved the areas into AREAS.DAT with a    */
/* different record.  A binary that matches his cannot also be the one you   */
/* run on a 15.3 board.  The working version lives at                        */
/*                                                                           */
/*   pcb154\MAIN\SOURCE\MISC\PACKFIDO\PACKFIDO.C                             */
/*                                                                           */
/* and reads AREAS.DAT.  Two files, two jobs, no pretending one is both.     */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mem.h>
#include <pcbtools.h>

#define AREA_SIZE        60
#define MAXFLEN          66
#define FIDO_CONFERENCE   5

#define OLDCONF_SIZE    548
#define ADDCONF_SIZE    256
#define OC_MSGFILE_OFF   27
#define AC_TYPE_OFF     127

#define TMPNAME     "tmp.cfg"
#define CHUNK       1024

void LIBENTRY readdatfile(void);
void LIBENTRY errorexittodos(char *Str);

#pragma pack(1)
typedef struct {
  unsigned int  PCB_Conference;
  char          Area_Name[AREA_SIZE];
  char          Mreserved[MAXFLEN];
  char          default_aka[25];
  char          origin[70];
  char          HighAscii;
  char          reserved[19];
} AREA_STRUCT;
#pragma pack()

static unsigned char *ConfMap;
static unsigned int   NumConf;

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

static void scanconferences(void)
{
    DOSFILE       cf, af;
    char          name[80];
    unsigned int  recsize, i;
    int           haveadd;
    unsigned char oc[OLDCONF_SIZE];
    unsigned char ac[ADDCONF_SIZE];
    char          conftype;

    strcpy(name, PcbData.CnfFile);
    strcat(name, ".@@@");
    if (dosfopen(name, OPEN_READ | OPEN_DENYNONE, &cf) == -1)
        errorexittodos("Unable to open CNAMES.@@@ file");

    if (dosfread(&recsize, sizeof(recsize), &cf) != sizeof(recsize) ||
        recsize != OLDCONF_SIZE)
        errorexittodos("CNAMES.@@@ file is formatted wrong - run PCBSETUP!");

    strcpy(name, PcbData.CnfFile);
    strcat(name, ".ADD");
    haveadd = (dosfopen(name, OPEN_READ | OPEN_DENYNONE, &af) != -1);
    if (!haveadd)
        errorexittodos("Unable to open CNAMES.ADD file");

    for (i = 0; i <= NumConf; i++) {
        dosfseek(&cf, (long) i * OLDCONF_SIZE + sizeof(short), SEEK_SET);
        if (dosfread(oc, OLDCONF_SIZE, &cf) != OLDCONF_SIZE)
            break;

        dosfseek(&af, (long) i * ADDCONF_SIZE, SEEK_SET);
        conftype = '\0';
        if (dosfread(ac, ADDCONF_SIZE, &af) == ADDCONF_SIZE)
            conftype = (char) ac[AC_TYPE_OFF];

        if (oc[0] != '\0' && oc[OC_MSGFILE_OFF] != '\0' &&
            conftype == FIDO_CONFERENCE)
            conf_set(i);
    }

    dosfclose(&cf);
    dosfclose(&af);
}

static void do_pack(void)
{
    DOSFILE       src, dst;
    unsigned int  version, count, kept, i;
    long          countpos;
    AREA_STRUCT   rec;
    char          buf[CHUNK];
    int           n;

    if (fileexist(PcbData.FidoConfig) == 255)
        return;

    if (dosfopen(PcbData.FidoConfig, OPEN_READ | OPEN_DENYNONE, &src) == -1)
        return;

    if (dosfopen(TMPNAME, OPEN_RDWR | OPEN_CREATE, &dst) == -1) {
        dosfclose(&src);
        return;
    }

    dosfread(&version, sizeof(version), &src);
    dosfwrite(&version, sizeof(version), &dst);

    countpos = dosfseek(&dst, 0L, SEEK_CUR);

    dosfread(&count, sizeof(count), &src);
    dosfwrite(&count, sizeof(count), &dst);

    kept = 0;
    for (i = 0; i < count; i++) {
        if (dosfread(&rec, sizeof(rec), &src) != sizeof(rec))
            break;

        if (rec.PCB_Conference <= NumConf && conf_avail(rec.PCB_Conference)) {
            dosfwrite(&rec, sizeof(rec), &dst);
            kept++;
            conf_take(rec.PCB_Conference);
        } else {
            printf("\nremoved %5d %s...", rec.PCB_Conference, rec.Area_Name);
        }
    }

    while ((n = dosfread(buf, CHUNK, &src)) > 0)
        dosfwrite(buf, n, &dst);

    dosfseek(&dst, countpos, SEEK_SET);
    dosfwrite(&kept, sizeof(kept), &dst);

    dosfclose(&src);
    dosfclose(&dst);

    unlink(PcbData.FidoConfig);
    rename(TMPNAME, PcbData.FidoConfig);
}

void main(void)
{
    readdatfile();

    NumConf = PcbData.NumConf;

    ConfMap = (unsigned char *) calloc((NumConf >> 3) + 2, 1);
    if (ConfMap == NULL) {
        printf("Unable to allocate memory");
        exit(1);
    }
    memset(ConfMap, 0, (NumConf >> 3) + 2);

    printf("\nscanning conference configuration...");
    scanconferences();

    printf("\npacking the fido configuration file...");
    do_pack();

    free(ConfMap);

    printf("\ndone.\n");
}
