/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* MKACCEPT.C - build the acceptance fixture for PACKFIDO.                   */
/*                                                                           */
/* pcbirc crew, 2026-09-23.  GPLv3.                                          */
/*                                                                           */
/* The acceptance test for PACKFIDO.C is not "it compiles".  It is: run the  */
/* reconstruction and Clark's shipped PACKFIDO.EXE over the SAME input and   */
/* diff the two outputs byte for byte.  For two years that test could not    */
/* be run, because the only Fido data we hold is version 3 and the shipped   */
/* binary only understands version 2.                                        */
/*                                                                           */
/* This program removes that excuse.  The version 2 layout is fully          */
/* specified by Clark's own converter - MISC\FIDOUTIL\SOURCE\CONVERT.CPP     */
/* reads a version 2 PCBFIDO.CFG field by field - so a well-formed one can   */
/* be written from scratch.  Nothing here is guessed:                        */
/*                                                                           */
/*   PCBFIDO.CFG  word version=2, word count, count x AREA_STRUCT(243),      */
/*                then a trailer PACKFIDO copies and never parses.  The      */
/*                trailer here is deliberately arbitrary bytes at an odd     */
/*                length, so any program that tried to parse it would fail.  */
/*   CNAMES.@@@   short RecSize=sizeof(oldconftype)=548, then 548-byte       */
/*                records indexed by conference number from 0.               */
/*   CNAMES.ADD   no header, 256-byte addconftype records, ConfType at +127. */
/*   PCBOARD.DAT  Clark's own from ..\EXAMPLES, with four lines repointed:   */
/*                  31   CNAMES base name                                    */
/*                  108  highest conference desired                          */
/*                  246  PCBFIDO.CFG                                         */
/*                  247  FIDOQUE.DAT                                         */
/*                Lines 246/247 are listed as "Reserved" in PCBDAT.DOC.      */
/*                They are not reserved: DATAFILE.C reads FidoConfig and     */
/*                FidoQueue there, right after AllFilesList on 244 and       */
/*                EnableFido on 245, and Clark's own PCBOARD.DAT has         */
/*                PCBFIDO.CFG and FIDOQUE.DAT on exactly those two lines.    */
/*                                                                           */
/* THE CASES THE FIXTURE COVERS - every branch of the pack decision:         */
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
/* and the whole list is shuffled, so "first" is not "lowest numbered".      */
/*                                                                           */
/* THE DIRECTORY MUST BE SHORT, AND THIS PROGRAM ENFORCES IT                 */
/* PcbData.FidoConfig is char[33] and PcbData.CnfFile is char[32], so the    */
/* paths PCBOARD.DAT carries have 32 and 31 usable characters.  The path     */
/* this file is filed under,                                                 */
/*     \PCB153\SOURCE\MISC\PACKFIDO\ACCEPT\PCBFIDO.CFG    47 characters      */
/* does not fit.  Clark's binary would silently truncate it and then open    */
/* nothing - printing "done." and changing no file, with no error at all.    */
/* So the test runs in a SHORT directory (ACCEPT.BAT uses \PFTEST) and this  */
/* program REFUSES rather than write a PCBOARD.DAT that cannot work.         */
/*                                                                           */
/* usage: MKACCEPT <dir> [<pcboard.dat to copy>]                             */
/*    eg: MKACCEPT \PFTEST \pcb153\SOURCE\MISC\PACKFIDO\examples\PCBOARD.DAT */
/*                                                                           */
/* A trailing backslash on <dir> is optional.  The second argument defaults  */
/* to that same absolute path, which assumes - as every DOS script in this   */
/* repo does - that the repo folder is MOUNTED AS THE DRIVE ROOT.  It is     */
/* passed in rather than reached by a relative walk so that neither this     */
/* program nor ACCEPT.BAT contains a ..\..\ that only works from one exact   */
/* directory and breaks the moment the tree is reorganised.                  */
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

static int write_cfg(char *dir)
{
    char          path[80];
    unsigned char rec[V2_RECSIZE];
    unsigned int  w;
    int           fd, i;

    sprintf(path, "%sPCBFIDO.CFG", dir);
    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IWRITE);
    if (fd == -1) { printf("cannot write %s\n", path); return 0; }

    w = 2U;                  write(fd, &w, 2);      /* file version        */
    w = (unsigned int) NAreas; write(fd, &w, 2);    /* number of areas     */

    for (i = 0; i < NAreas; i++) {
        memset(rec, 0, sizeof(rec));
        *(unsigned int *)rec = AConf[i];
        strcpy((char *)rec + 2, AName[i]);          /* Area_Name[60]       */
        strcpy((char *)rec + 2 + AREA_SIZE, "MSGS.FID");           /* Mreserved  */
        strcpy((char *)rec + 2 + AREA_SIZE + MAXFLEN, "1:100/100");/* aka        */
        strcpy((char *)rec + 2 + AREA_SIZE + MAXFLEN + 25,
               "* Origin: acceptance fixture");                    /* origin     */
        write(fd, rec, V2_RECSIZE);
    }

    /* The trailer.  1031 bytes - not a multiple of 1024, on purpose, so    */
    /* the final short chunk of the copy loop is exercised too.             */
    for (i = 0; i < 1031; i++) {
        unsigned char b = (unsigned char)(lcg() >> 5);
        write(fd, &b, 1);
    }

    close(fd);
    printf("PCBFIDO.CFG  %d areas\n", NAreas);
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

#define DEFAULT_PCBDAT  "\\pcb153\\SOURCE\\MISC\\PACKFIDO\\examples\\PCBOARD.DAT"

#define FIDOCONFIG_MAX  32          /* sizeof(PcbData.FidoConfig) - 1 */
#define CNFFILE_MAX     31          /* sizeof(PcbData.CnfFile)    - 1 */

/* --- /C and /D : copy and compare, because the shell cannot be trusted --- */
/*                                                                           */
/* These are not decoration.  Two things a batch file would normally use are */
/* not dependable here:                                                      */
/*                                                                           */
/*   FC     an EXTERNAL command.  Not on every DOS, and capturing its output */
/*          means a redirect, which is the other problem.                    */
/*   COPY   an INTERNAL command that, measured under DOSBox-X, works from    */
/*          the autoexec and SILENTLY DOES NOTHING from inside a batch file  */
/*          - no error, no file, and ERRORLEVEL unchanged.  A harness built  */
/*          on it reports whatever the previous step left lying around.      */
/*                                                                           */
/* So ACCEPT.BAT calls these instead and uses nothing but EXE calls, IF      */
/* ERRORLEVEL, IF EXIST, CD and ECHO.  No COPY, no REN, no DEL, no FC, no    */
/* redirection anywhere.                                                     */

static int filecopy(char *fa, char *fb)
{
    int            a, b, n;
    unsigned char  buf[1024];

    a = open(fa, O_RDONLY | O_BINARY);
    if (a == -1) { printf("cannot open %s\n", fa); return 1; }
    b = open(fb, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IWRITE);
    if (b == -1) { printf("cannot write %s\n", fb); close(a); return 1; }

    while ((n = read(a, buf, sizeof(buf))) > 0) {
        if (write(b, buf, n) != n) {
            printf("short write to %s\n", fb);
            close(a); close(b); return 1;
        }
    }
    close(a);
    close(b);
    return 0;
}

static int filediff(char *fa, char *fb)
{
    int            a, b, na, nb, i;
    long           off = 0L;
    unsigned char  ba[1024], bb[1024];

    a = open(fa, O_RDONLY | O_BINARY);
    if (a == -1) { printf("cannot open %s\n", fa); return 2; }
    b = open(fb, O_RDONLY | O_BINARY);
    if (b == -1) { printf("cannot open %s\n", fb); close(a); return 2; }

    for (;;) {
        na = read(a, ba, sizeof(ba));
        nb = read(b, bb, sizeof(bb));
        if (na != nb) {
            printf("%s and %s are different lengths\n", fa, fb);
            close(a); close(b); return 1;
        }
        if (na <= 0) break;
        for (i = 0; i < na; i++) {
            if (ba[i] != bb[i]) {
                printf("first difference at offset %ld: %02X vs %02X\n",
                       off + (long) i, ba[i], bb[i]);
                close(a); close(b); return 1;
            }
        }
        off += (long) na;
    }
    close(a);
    close(b);
    printf("%ld bytes, identical\n", off);
    return 0;
}

int main(int argc, char *argv[])
{
    char dir[64];
    char src[80];
    int  ncfg, ncnf;

    if (argc == 4 && (argv[1][0] == '/' || argv[1][0] == '-')) {
        if (argv[1][1] == 'd' || argv[1][1] == 'D')
            return filediff(argv[2], argv[3]);
        if (argv[1][1] == 'c' || argv[1][1] == 'C')
            return filecopy(argv[2], argv[3]);
    }

    if (argc < 2) {
        printf("usage: MKACCEPT <dir> [<pcboard.dat to copy>]\n"
               "       MKACCEPT /C <from> <to>       copy, sets ERRORLEVEL\n"
               "       MKACCEPT /D <file1> <file2>   compare, sets ERRORLEVEL\n"
               "   eg: MKACCEPT \\PFTEST\n"
               "\n"
               "The directory must be SHORT - see the note at the top of\n"
               "MKACCEPT.C.  The default source PCBOARD.DAT is\n"
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
    if (! write_cfg(dir))    return 1;
    {   /* ORIG.CFG - the pristine copy both programs are run against.       */
        char a[80], b[80];
        sprintf(a, "%sPCBFIDO.CFG", dir);
        sprintf(b, "%sORIG.CFG",    dir);
        if (filecopy(a, b) != 0) return 1;
    }
    if (! write_cnames(dir)) return 1;
    if (! write_pcbdat(dir, src)) return 1;

    printf("fixture written to %s\n", dir);
    return 0;
}
