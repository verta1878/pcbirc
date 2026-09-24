/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* MKPFDATA.C - make the PACKFIDO acceptance test data.                      */
/*                                                                           */
/* pcbirc crew, 2026-09-23.  GPLv3.                                          */
/* Named after Clark's own MKPCBTXT.C, MKPCBMNU and MKUNIQUE.C: MK plus      */
/* what it makes.  It makes four files and does nothing else.                */
/*                                                                           */
/* WHAT IT MAKES, and where every byte of the layout came from               */
/* ==========================================================                */
/*                                                                           */
/*   PCBFIDO.CFG  a VERSION 2 Fido configuration - the 15.21 layout, the     */
/*                only one Clark's shipped PACKFIDO.EXE understands:         */
/*                  word  file version = 2                                   */
/*                  word  number of areas                                    */
/*                  n x   AREA_STRUCT, 243 bytes                             */
/*                  ...   a trailer PACKFIDO copies and never parses         */
/*                The trailer here is 1,031 arbitrary bytes - deliberately   */
/*                not a multiple of 1,024, so the short final chunk of the   */
/*                copy loop is exercised, and deliberately arbitrary, so     */
/*                any program that tried to parse it would fail.             */
/*   ORIG.CFG     a second, identical copy.  Both programs under test are    */
/*                run against a fresh copy of it, and it is written here     */
/*                rather than COPYed by the batch - see PFACCEPT.BAT for     */
/*                why COPY cannot be used.                                   */
/*   CNAMES.@@@   short RecSize = sizeof(oldconftype) = 548, then 548-byte   */
/*                records indexed by conference number FROM 0.               */
/*   CNAMES.ADD   no header at all, 256-byte addconftype records,            */
/*                ConfType at +127.                                          */
/*   PCBOARD.DAT  Clark's own, copied line for line from EXAMPLES\ with      */
/*                exactly four lines repointed at the test directory:        */
/*                  31   CNAMES base name                                    */
/*                  108  highest conference desired                          */
/*                  246  PCBFIDO.CFG                                         */
/*                  247  FIDOQUE.DAT                                         */
/*                                                                           */
/* The version 2 layout is not guessed.  Clark's own converter reads it      */
/* field by field - MISC\FIDOUTIL\SOURCE\CONVERT.CPP, function Areas():      */
/*     fread(&numareas,sizeof(numareas),1,oldfile);                          */
/*     for(i=0;i<numareas;i++) fread(&area,sizeof(area),1,oldfile);          */
/* with area an AREA_STRUCT.                                                 */
/*                                                                           */
/* Lines 246 and 247 are listed as "Reserved" in PCBDAT.DOC.  THEY ARE NOT.  */
/* DATAFILE.C reads FidoConfig and FidoQueue there, right after AllFilesList */
/* on 244 and EnableFido on 245, and Clark's own PCBOARD.DAT in EXAMPLES\    */
/* has PCBFIDO.CFG on 246 and FIDOQUE.DAT on 247.                            */
/*                                                                           */
/* THE CASES THE FIXTURE COVERS - every branch of the pack decision          */
/* ================================================================          */
/*   500 ordinary areas, conferences 1..500                                  */
/*   conference 0            the Main Board, a legal conference number       */
/*   conference 800          exactly NumConf - the boundary, must be KEPT    */
/*   conference 801          one past NumConf - must be DROPPED              */
/*   conference 65535        the largest word - must be DROPPED              */
/*   conference 5            has a name and a message base but ConfType 0    */
/*                           - not a Fido conference, must be DROPPED        */
/*   conference 77           ConfType 5 but NO MsgFile - must be DROPPED     */
/*   12 triplicates + 12 quadruplicates of conferences 1..12                 */
/*                           - only the FIRST of each survives               */
/*                                                                           */
/* and the whole list is SHUFFLED, so "first" is not "lowest numbered".  The */
/* shuffle is a 16-bit LCG with a fixed seed, so the fixture is identical on */
/* every compiler and every run - a byte diff is worthless otherwise.        */
/*                                                                           */
/* THE DIRECTORY MUST BE SHORT, AND THIS PROGRAM ENFORCES IT                 */
/* ========================================================                  */
/* PcbData.FidoConfig is char[33] and PcbData.CnfFile is char[32], so the    */
/* paths PCBOARD.DAT carries have 32 and 31 usable characters.  The path     */
/* this file is itself filed under,                                          */
/*                                                                           */
/*     \PCB153\SOURCE\MISC\PACKFIDO\ACCEPT\PCBFIDO.CFG    47 characters      */
/*                                                                           */
/* does not fit.  Clark's binary would silently truncate it, open nothing,   */
/* print "done." and change no file - a SILENT PASS THAT PROVES NOTHING.     */
/* That exact failure happened once during this work.  So the test runs in a */
/* short directory - PFACCEPT.BAT uses \PFTEST - and this program REFUSES,   */
/* with the arithmetic printed, rather than write a PCBOARD.DAT that cannot  */
/* work.                                                                     */
/*                                                                           */
/* usage: MKPFDATA <dir> [<pcboard.dat to copy>]                             */
/*    eg: MKPFDATA \PFTEST                                                   */
/*                                                                           */
/* A trailing backslash on <dir> is optional.  The second argument defaults  */
/* to \pcb153\SOURCE\MISC\PACKFIDO\EXAMPLES\PCBOARD.DAT, which assumes - as  */
/* every DOS script in this repo does - that the repo folder is MOUNTED AS   */
/* THE DRIVE ROOT.  It is an argument rather than a relative walk so that    */
/* nothing here contains a ..\..\ that works from one directory only.        */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>

#define NUMCONF        800
#define NBASE          500
#define NDUP            12

#define AREA_SIZE       60
#define MAXFLEN         66
#define V2_RECSIZE     243          /* sizeof(AREA_STRUCT)  */
#define OLDCONF_SIZE   548          /* sizeof(oldconftype)  */
#define ADDCONF_SIZE   256          /* sizeof(addconftype)  */
#define OC_MSGFILE_OFF  27
#define AC_TYPE_OFF    127

#define MAXAREAS  (NBASE + 6 + NDUP * 2)

static unsigned int  AConf[MAXAREAS];
static char          AName[MAXAREAS][32];
static int           NAreas;

/* A 16-bit LCG, so the fixture is identical on every compiler and run.     */
static unsigned int Seed = 1878U;
static unsigned int lcg(void)
{
    Seed = (unsigned int)(Seed * 25173U + 13849U);
    return Seed;
}

static void add(unsigned int conf, char *name)
{
    if (NAreas >= MAXAREAS) return;
    AConf[NAreas] = conf;
    strncpy(AName[NAreas], name, 31);
    AName[NAreas][31] = '\0';
    NAreas++;
}

static void build_list(void)
{
    char tmp[32];
    int  i, j;
    unsigned int c;

    for (i = 1; i <= NBASE; i++) {
        sprintf(tmp, "AREA%d", i);
        add((unsigned int) i, tmp);
    }
    add(0U,              "MAIN-BOARD");
    add((unsigned int) NUMCONF,     "BOUNDARY-EXACT");
    add((unsigned int) NUMCONF + 1, "BOUNDARY-OVER");
    add(65535U,          "MAX-WORD");
    add(5U,              "NOT-A-FIDO-CONF");
    add(77U,             "NAME-BUT-NO-MSGFILE");
    for (i = 1; i <= NDUP; i++) { sprintf(tmp, "TRIPLICATE-%d", i); add((unsigned int) i, tmp); }
    for (i = 1; i <= NDUP; i++) { sprintf(tmp, "QUADRUPLE-%d",  i); add((unsigned int) i, tmp); }

    /* Fisher-Yates with the LCG - deterministic, but not in file order.    */
    for (i = NAreas - 1; i > 0; i--) {
        j = (int)(lcg() % (unsigned int)(i + 1));
        c = AConf[i]; AConf[i] = AConf[j]; AConf[j] = c;
        strcpy(tmp, AName[i]); strcpy(AName[i], AName[j]); strcpy(AName[j], tmp);
    }
}

/* PCBFIDO.CFG and ORIG.CFG are written together, byte for byte, in one     */
/* pass.  ORIG.CFG is the pristine copy both programs under test are run     */
/* against, and it is made HERE rather than COPYed by the batch because      */
/* COPY does not work from inside a batch file under DOSBox-X - see the      */
/* note in PFACCEPT.BAT.                                                     */

static int put2(int a, int b, void *buf, unsigned int n)
{
    if (write(a, buf, n) != (int) n) return 0;
    if (write(b, buf, n) != (int) n) return 0;
    return 1;
}

static int write_cfg(char *dir)
{
    char          path[80];
    unsigned char rec[V2_RECSIZE];
    unsigned char b;
    unsigned int  w;
    int           f1, f2, i, ok;

    sprintf(path, "%sPCBFIDO.CFG", dir);
    f1 = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IWRITE);
    if (f1 == -1) { printf("cannot write %s\n", path); return 0; }

    sprintf(path, "%sORIG.CFG", dir);
    f2 = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IWRITE);
    if (f2 == -1) { printf("cannot write %s\n", path); close(f1); return 0; }

    ok  = 1;
    w   = 2U;                   ok = ok && put2(f1, f2, &w, 2);  /* version */
    w   = (unsigned int) NAreas; ok = ok && put2(f1, f2, &w, 2); /* count   */

    for (i = 0; ok && i < NAreas; i++) {
        memset(rec, 0, sizeof(rec));
        *(unsigned int *)rec = AConf[i];
        strcpy((char *)rec + 2, AName[i]);          /* Area_Name[60]       */
        strcpy((char *)rec + 2 + AREA_SIZE, "MSGS.FID");           /* Mreserved  */
        strcpy((char *)rec + 2 + AREA_SIZE + MAXFLEN, "1:100/100");/* aka        */
        strcpy((char *)rec + 2 + AREA_SIZE + MAXFLEN + 25,
               "* Origin: acceptance fixture");                    /* origin     */
        ok = put2(f1, f2, rec, V2_RECSIZE);
    }

    /* The trailer.  1031 bytes - not a multiple of 1024, on purpose, so    */
    /* the final short chunk of the copy loop is exercised too.             */
    for (i = 0; ok && i < 1031; i++) {
        b  = (unsigned char)(lcg() >> 5);
        ok = put2(f1, f2, &b, 1);
    }

    close(f1);
    close(f2);
    if (! ok) { printf("short write - is the disk full?\n"); return 0; }
    printf("PCBFIDO.CFG and ORIG.CFG  %d areas\n", NAreas);
    return 1;
}

static int is_fido(int c)
{
    if (c == 5) return 0;                      /* ConfType 0 on purpose    */
    if (c == 77) return 1;                     /* Fido, but no MsgFile     */
    if (c == 0 || c == NUMCONF) return 1;
    return (c >= 1 && c <= NBASE);
}

static int write_cnames(char *dir)
{
    char          path[80];
    unsigned char rec[OLDCONF_SIZE];
    unsigned char add_[ADDCONF_SIZE];
    unsigned int  w;
    int           f1, f2, c;

    sprintf(path, "%sCNAMES.@@@", dir);
    f1 = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IWRITE);
    if (f1 == -1) { printf("cannot write %s\n", path); return 0; }
    sprintf(path, "%sCNAMES.ADD", dir);
    f2 = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IWRITE);
    if (f2 == -1) { printf("cannot write %s\n", path); close(f1); return 0; }

    w = OLDCONF_SIZE;
    write(f1, &w, 2);                          /* the RecSize header       */

    for (c = 0; c <= NUMCONF; c++) {
        memset(rec, 0, sizeof(rec));
        sprintf((char *)rec, "CONF%d", c);      /* Name[14]                 */
        if (c != 77)
            strcpy((char *)rec + OC_MSGFILE_OFF, "C:\\PCB\\MSG");
        write(f1, rec, OLDCONF_SIZE);

        memset(add_, 0, sizeof(add_));
        add_[AC_TYPE_OFF] = (unsigned char)(is_fido(c) ? 5 : 0);
        write(f2, add_, ADDCONF_SIZE);
    }

    close(f1);
    close(f2);
    printf("CNAMES.@@@ / CNAMES.ADD  %d conferences\n", NUMCONF + 1);
    return 1;
}

/* Copy Clark's own PCBOARD.DAT, repointing the four lines that matter.     */

static int write_pcbdat(char *dir, char *src)
{
    char  path[80], line[256];
    FILE *in, *out;
    int   n;

    sprintf(path, "%sPCBOARD.DAT", dir);
    in = fopen(src, "rt");
    if (in == NULL) {
        printf("cannot read %s\n", src);
        return 0;
    }
    out = fopen(path, "wt");
    if (out == NULL) { printf("cannot write %s\n", path); fclose(in); return 0; }

    n = 0;
    while (fgets(line, sizeof(line), in) != NULL) {
        n++;
        switch (n) {
          case  31: fprintf(out, "%sCNAMES\n", dir);       break;
          case 108: fprintf(out, "%d\n", NUMCONF);         break;
          case 246: fprintf(out, "%sPCBFIDO.CFG\n", dir);  break;
          case 247: fprintf(out, "%sFIDOQUE.DAT\n", dir);  break;
          default:  fputs(line, out);                      break;
        }
    }
    fclose(in);
    fclose(out);
    printf("PCBOARD.DAT  %d lines\n", n);
    return (n >= 247);
}

#define DEFAULT_PCBDAT  "\\pcb153\\SOURCE\\MISC\\PACKFIDO\\EXAMPLES\\PCBOARD.DAT"

#define FIDOCONFIG_MAX  32          /* sizeof(PcbData.FidoConfig) - 1 */
#define CNFFILE_MAX     31          /* sizeof(PcbData.CnfFile)    - 1 */

int main(int argc, char *argv[])
{
    char dir[64];
    char src[80];
    int  ncfg, ncnf;

    if (argc < 2) {
        printf("usage: MKPFDATA <dir> [<pcboard.dat to copy>]\n"
               "   eg: MKPFDATA \\PFTEST\n"
               "\n"
               "Makes the PACKFIDO acceptance fixture: a version 2\n"
               "PCBFIDO.CFG, a matching ORIG.CFG, CNAMES.@@@, CNAMES.ADD\n"
               "and a PCBOARD.DAT pointed at <dir>.\n"
               "\n"
               "<dir> must be SHORT - see the note at the top of MKPFDATA.C.\n"
               "The default source PCBOARD.DAT is\n"
               "   " DEFAULT_PCBDAT "\n"
               "which assumes the repo folder is mounted as the drive root.\n");
        return 1;
    }
    strncpy(dir, argv[1], sizeof(dir) - 2);
    dir[sizeof(dir) - 2] = '\0';
    if (dir[0] != '\0' && dir[strlen(dir) - 1] != '\\')
        strcat(dir, "\\");

    if (argc > 2) {
        strncpy(src, argv[2], sizeof(src) - 1);
        src[sizeof(src) - 1] = '\0';
    } else {
        strcpy(src, DEFAULT_PCBDAT);
    }

    /* Refuse rather than write a PCBOARD.DAT whose paths get truncated.    */
    ncfg = (int) strlen(dir) + 11;              /* dir + "PCBFIDO.CFG"      */
    ncnf = (int) strlen(dir) + 6;               /* dir + "CNAMES"           */
    if (ncfg > FIDOCONFIG_MAX || ncnf > CNFFILE_MAX) {
        printf("\n%s is too long a directory for this test.\n", dir);
        printf("  %sPCBFIDO.CFG is %d characters; PcbData.FidoConfig holds %d\n",
               dir, ncfg, FIDOCONFIG_MAX);
        printf("  %sCNAMES      is %d characters; PcbData.CnfFile    holds %d\n",
               dir, ncnf, CNFFILE_MAX);
        printf("\nClark's PACKFIDO.EXE would truncate the path, open nothing,\n"
               "print \"done.\" and change no file - a silent pass that proves\n"
               "nothing.  Use a short directory, eg \\PFTEST.\n");
        return 1;
    }

    build_list();
    if (! write_cfg(dir))         return 1;
    if (! write_cnames(dir))      return 1;
    if (! write_pcbdat(dir, src)) return 1;

    printf("fixture written to %s\n", dir);
    return 0;
}
