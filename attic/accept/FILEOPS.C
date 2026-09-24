/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* FILEOPS.C - copy a file, or compare two files, setting ERRORLEVEL.        */
/*                                                                           */
/* pcbirc crew, 2026-09-23.  GPLv3.                                          */
/*                                                                           */
/* WHY THIS EXISTS AT ALL                                                    */
/* ======================                                                    */
/* DOS already has COPY and FC.  Neither can be used by a test harness that  */
/* has to give a trustworthy answer:                                         */
/*                                                                           */
/*   COPY  is an INTERNAL command, and under DOSBox-X it works from the      */
/*         autoexec and SILENTLY DOES NOTHING from inside a batch file - no  */
/*         error, no file, ERRORLEVEL unchanged.  Measured with MKDIR        */
/*         markers, not assumed.  A harness built on it goes on to compare   */
/*         whatever the previous step left lying around and reports PASS     */
/*         for the wrong reason.  That is worse than failing.                */
/*                                                                           */
/*   FC    is an EXTERNAL command.  It is not on every DOS, it is not in     */
/*         DOSBox-X, and capturing its verdict means redirecting its output  */
/*         to a file, which is unreliable from batch for the same reason     */
/*         COPY is.                                                          */
/*                                                                           */
/* So PFACCEPT.BAT calls this instead, and uses nothing but EXE calls, IF    */
/* ERRORLEVEL, IF EXIST, CD and ECHO.  No COPY, no REN, no DEL, no FC, and   */
/* no redirection anywhere.                                                  */
/*                                                                           */
/* It is deliberately generic and knows nothing about PACKFIDO.  Any other   */
/* DOS build or test in this repo that needs a dependable copy or a          */
/* dependable byte compare can call it.                                      */
/*                                                                           */
/* usage:  FILEOPS /C <from> <to>      copy                                  */
/*         FILEOPS /D <file1> <file2>  compare                               */
/*                                                                           */
/* ERRORLEVEL 0  copied, or the two files are identical                      */
/*            1  they differ, or the copy failed                             */
/*            2  a file could not be opened                                  */
/*                                                                           */
/* /D prints the offset and the two bytes at the FIRST difference, so a      */
/* failing acceptance run says where it went wrong rather than just "no".    */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>

#define CHUNK  1024

static int filecopy(char *from, char *to)
{
    int            a, b, n;
    unsigned char  buf[CHUNK];

    a = open(from, O_RDONLY | O_BINARY);
    if (a == -1) { printf("FILEOPS: cannot open %s\n", from); return 2; }

    b = open(to, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IWRITE);
    if (b == -1) {
        printf("FILEOPS: cannot write %s\n", to);
        close(a);
        return 2;
    }

    while ((n = read(a, buf, CHUNK)) > 0) {
        if (write(b, buf, n) != n) {
            printf("FILEOPS: short write to %s - is the disk full?\n", to);
            close(a); close(b);
            return 1;
        }
    }
    if (n < 0) {
        printf("FILEOPS: read error on %s\n", from);
        close(a); close(b);
        return 1;
    }

    close(a);
    close(b);
    return 0;
}

static int filediff(char *fa, char *fb)
{
    int            a, b, na, nb, i;
    long           off = 0L;
    unsigned char  ba[CHUNK], bb[CHUNK];

    a = open(fa, O_RDONLY | O_BINARY);
    if (a == -1) { printf("FILEOPS: cannot open %s\n", fa); return 2; }
    b = open(fb, O_RDONLY | O_BINARY);
    if (b == -1) { printf("FILEOPS: cannot open %s\n", fb); close(a); return 2; }

    for (;;) {
        na = read(a, ba, CHUNK);
        nb = read(b, bb, CHUNK);

        if (na < 0 || nb < 0) {
            printf("FILEOPS: read error\n");
            close(a); close(b);
            return 2;
        }
        if (na != nb) {
            printf("FILEOPS: %s and %s are different lengths\n", fa, fb);
            close(a); close(b);
            return 1;
        }
        if (na == 0) break;

        for (i = 0; i < na; i++) {
            if (ba[i] != bb[i]) {
                printf("FILEOPS: first difference at offset %ld - "
                       "%02X in %s, %02X in %s\n",
                       off + (long) i, ba[i], fa, bb[i], fb);
                close(a); close(b);
                return 1;
            }
        }
        off += (long) na;
    }

    close(a);
    close(b);
    printf("FILEOPS: %ld bytes, identical\n", off);
    return 0;
}

int main(int argc, char *argv[])
{
    char sw;

    if (argc == 4 && (argv[1][0] == '/' || argv[1][0] == '-')) {
        sw = argv[1][1];
        if (sw == 'c' || sw == 'C') return filecopy(argv[2], argv[3]);
        if (sw == 'd' || sw == 'D') return filediff(argv[2], argv[3]);
    }

    printf("usage: FILEOPS /C <from> <to>      copy\n"
           "       FILEOPS /D <file1> <file2>  compare\n"
           "\n"
           "Sets ERRORLEVEL: 0 ok or identical, 1 differ or copy failed,\n"
           "2 a file could not be opened.  Exists because DOS COPY does not\n"
           "work from inside a batch file under DOSBox-X and FC is not\n"
           "always there - see the note at the top of FILEOPS.C.\n");
    return 2;
}
