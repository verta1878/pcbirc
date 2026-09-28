/* ====================================================================
 * VIEWPMLM.EXE — PMLM Trace File Viewer
 * ====================================================================
 * Clean-room from SIOREF.TXT. Views trace files created by PMLM.
 * Works in DOS, OS/2 DOS sessions, and native OS/2.
 * Navigation: UP, DOWN, PAGEUP, PAGEDOWN, HOME, END, ESC=quit
 * ====================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define MAX_LINES       10000
#define LINE_WIDTH      80
#define SCREEN_ROWS     23

/* Trace record: direction byte + data byte */
typedef struct {
    char    dir;        /* 'R' = received, 'T' = transmitted */
    unsigned char data;
} TRACERECORD;

static TRACERECORD *records = NULL;
static long         numRecords = 0;
static long         topLine = 0;

static void LoadTraceFile(const char *filename);
static void DisplayPage(void);
static void SetColor(int attr);

int main(int argc, char *argv[])
{
    int key;

    if (argc < 2) {
        printf("Usage: VIEWPMLM tracefile\n");
        return 1;
    }

    LoadTraceFile(argv[1]);
    if (numRecords == 0) {
        printf("No trace data found in %s\n", argv[1]);
        return 1;
    }

    printf("VIEWPMLM — %ld records loaded. Use arrow keys to navigate, ESC to quit.\n",
           numRecords);

    /* Display loop */
    topLine = 0;
    DisplayPage();

    while (1) {
        key = getch();

        if (key == 27) break;           /* ESC */
        if (key == 0 || key == 0xE0) {  /* Extended key */
            key = getch();
            switch (key) {
            case 72:    /* UP */
                if (topLine > 0) topLine--;
                break;
            case 80:    /* DOWN */
                if (topLine < numRecords - SCREEN_ROWS)
                    topLine++;
                break;
            case 73:    /* PAGEUP */
                topLine -= SCREEN_ROWS;
                if (topLine < 0) topLine = 0;
                break;
            case 81:    /* PAGEDOWN */
                topLine += SCREEN_ROWS;
                if (topLine > numRecords - SCREEN_ROWS)
                    topLine = numRecords - SCREEN_ROWS;
                if (topLine < 0) topLine = 0;
                break;
            case 71:    /* HOME */
                topLine = 0;
                break;
            case 79:    /* END */
                topLine = numRecords - SCREEN_ROWS;
                if (topLine < 0) topLine = 0;
                break;
            }
        }

        DisplayPage();
    }

    free(records);
    printf("\n");
    return 0;
}

static void LoadTraceFile(const char *filename)
{
    FILE *f;
    long  fileSize;
    int   ch1, ch2;

    f = fopen(filename, "rb");
    if (!f) {
        printf("Error: Cannot open %s\n", filename);
        return;
    }

    /* Get file size */
    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    /* Allocate records (each is 2 bytes) */
    numRecords = fileSize / 2;
    if (numRecords > MAX_LINES) numRecords = MAX_LINES;

    records = (TRACERECORD *)malloc(numRecords * sizeof(TRACERECORD));
    if (!records) {
        printf("Error: Out of memory\n");
        fclose(f);
        numRecords = 0;
        return;
    }

    /* Read records */
    {
        long i;
        for (i = 0; i < numRecords; i++) {
            ch1 = fgetc(f);
            ch2 = fgetc(f);
            if (ch1 == EOF || ch2 == EOF) {
                numRecords = i;
                break;
            }
            records[i].dir  = (char)ch1;
            records[i].data = (unsigned char)ch2;
        }
    }

    fclose(f);
}

static void DisplayPage(void)
{
    long i, j;
    long end = topLine + SCREEN_ROWS;
    int  lineRecords = 16;  /* Records per display line */
    long lineStart;

    if (end > numRecords) end = numRecords;

    printf("\033[H");   /* ANSI: cursor home */
    printf("\033[37;40m");  /* White on black */

    /* Header */
    printf("  Offset  Dir  ");
    for (j = 0; j < lineRecords; j++) printf("%02X ", (int)j);
    printf(" ASCII\n");
    printf("  ------  ---  ");
    for (j = 0; j < lineRecords; j++) printf("-- ");
    printf(" ----------------\n");

    /* Display records grouped by direction changes */
    for (i = topLine; i < end; i++) {
        char ch;
        const char *color;

        if (records[i].dir == 'R') {
            color = "\033[37;44m";      /* White on blue = RX */
        } else {
            color = "\033[33;45m";      /* Yellow on magenta = TX */
        }

        ch = (records[i].data >= 32 && records[i].data < 127)
             ? (char)records[i].data : '.';

        printf("%s%6ld  %cX  %02X %c \033[0m\n",
               color, i,
               records[i].dir,
               (unsigned)records[i].data,
               ch);
    }

    /* Fill remaining */
    for (i = end - topLine; i < SCREEN_ROWS - 2; i++) {
        printf("\033[K\n");
    }

    printf("\033[7m %ld-%ld of %ld  UP/DN/PGUP/PGDN/HOME/END  ESC=Quit \033[0m",
           topLine + 1, end, numRecords);
    fflush(stdout);
}
