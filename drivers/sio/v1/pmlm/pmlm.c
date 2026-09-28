/* ====================================================================
 * PMLM.EXE — Poor Man's Line Monitor for OS/2
 * ====================================================================
 * Clean-room from SIOREF.TXT description.
 * Monitors COM port traffic and displays RX/TX bytes with color coding.
 *
 * Usage: PMLM portnum [tracefile]
 *
 * Display:
 *   Upper area: traffic (RX = white on blue, TX = yellow on violet)
 *   Lower area: signal status (CTS, DSR, DCD, RI)
 *               buffer counts (Rx Chars, Tx Chars)
 *   Bottom:     keyboard shortcuts
 * ====================================================================
 */

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_VIO
#define INCL_KBD
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IOCTL_ASYNC         0x0001
#define ASYNC_GETMODEMINPUT 0x0067
#define ASYNC_GETINQUECOUNT 0x0068
#define ASYNC_GETOUTQUECOUNT 0x0069

/* VIO color attributes */
#define ATTR_RX     0x1F    /* White on blue */
#define ATTR_TX     0x5E    /* Yellow on violet */
#define ATTR_SIG_ON 0x4F    /* Bright white on red */
#define ATTR_SIG_OFF 0x47   /* Dim white on red */
#define ATTR_STATUS 0x07    /* Normal */
#define ATTR_KEYS   0x70    /* Reverse video */

static BYTE attrRx     = ATTR_RX;
static BYTE attrTx     = ATTR_TX;
static BYTE attrStatus = ATTR_STATUS;
static BYTE attrKeys   = ATTR_KEYS;

static HFILE    hCom = (HFILE)-1;
static FILE    *traceFile = NULL;
static int      running = 1;
static int      displayRow = 0;
static int      maxDisplayRow = 19;

static void     DrawStatusBar(void);
static void     DrawSignals(void);
static void     DrawBufferCounts(void);
static void     DrawKeyBar(void);
static void     DisplayByte(BYTE b, BYTE attr);
static void     WriteTrace(BYTE b, char direction);
static void     PollPort(void);

int main(int argc, char *argv[])
{
    int     portNum;
    ULONG   action;
    APIRET  rc;
    char    name[16];
    KBDKEYINFO ki;

    if (argc < 2) {
        printf("Usage: PMLM portnum [tracefile]\n");
        return 1;
    }

    portNum = atoi(argv[1]);
    sprintf(name, "COM%d", portNum);

    /* Open trace file if specified */
    if (argc >= 3) {
        traceFile = fopen(argv[2], "wb");
        if (!traceFile) {
            printf("Warning: Cannot create trace file %s\n", argv[2]);
        }
    }

    /* Open COM port */
    rc = DosOpen(name, &hCom, &action, 0, 0,
                 OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE, NULL);
    if (rc != 0) {
        printf("Error: Cannot open %s (rc=%lu)\n", name, rc);
        return 1;
    }

    /* Clear screen and set up display */
    {
        BYTE blank[2] = { ' ', ATTR_STATUS };
        VioScrollUp(0, 0, -1, -1, -1, blank, 0);
    }

    /* Draw initial screen layout */
    {
        char title[80];
        sprintf(title, " PMLM — Monitoring COM%d ", portNum);
        VioWrtCharStrAtt(title, strlen(title), 0, 0, &attrKeys, 0);
    }

    DrawKeyBar();

    /* Main loop */
    while (running) {
        /* Check for keyboard input */
        KbdCharIn(&ki, IO_NOWAIT, 0);
        if (ki.fbStatus & 0x40) {  /* Key available */
            switch (ki.chChar) {
            case 'q': case 'Q': case 27:  /* ESC */
                running = 0;
                break;
            case 'c': case 'C':
                /* Clear display */
                {
                    BYTE blank[2] = { ' ', ATTR_STATUS };
                    VioScrollUp(1, 0, maxDisplayRow, 79, -1, blank, 0);
                    displayRow = 1;
                }
                break;
            }
        }

        /* Poll port for data and status */
        PollPort();

        /* Update signal and buffer display */
        DrawSignals();
        DrawBufferCounts();

        /* Small delay to prevent CPU spin */
        DosSleep(10);
    }

    /* Cleanup */
    DosClose(hCom);
    if (traceFile) fclose(traceFile);

    /* Restore screen */
    {
        BYTE blank[2] = { ' ', 0x07 };
        VioScrollUp(0, 0, -1, -1, -1, blank, 0);
    }

    return 0;
}

static USHORT lastTxCount = 0;

static void PollPort(void)
{
    ULONG parmLen = 0, dataLen;
    BYTE  data[4];
    ULONG actual;

    /* Check RX buffer — read and display incoming data */
    dataLen = 4;
    if (DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_GETINQUECOUNT,
                    NULL, 0, &parmLen, data, 4, &dataLen) == 0) {
        USHORT count = *(USHORT *)&data[0];
        if (count > 0) {
            BYTE buf[256];
            ULONG toRead = (count > 256) ? 256 : count;
            if (DosRead(hCom, buf, toRead, &actual) == 0 && actual > 0) {
                ULONG i;
                for (i = 0; i < actual; i++) {
                    DisplayByte(buf[i], attrRx);
                    WriteTrace(buf[i], 'R');
                }
            }
        }
    }

    /* Monitor TX buffer count for outgoing traffic indication */
    dataLen = 4;
    if (DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_GETOUTQUECOUNT,
                    NULL, 0, &parmLen, data, 4, &dataLen) == 0) {
        USHORT txCount = *(USHORT *)&data[0];
        if (txCount > lastTxCount) {
            /* New data was written to TX — indicate with marker */
            DisplayByte('>', attrTx);
        }
        lastTxCount = txCount;
    }
}

static int displayCol = 0;

static void DisplayByte(BYTE b, BYTE attr)
{
    char ch;
    char hexBuf[4];
    BYTE hexAttr;

    /* Display printable chars, dots for control */
    if (b >= 32 && b < 127) {
        ch = (char)b;
    } else {
        ch = '.';
    }

    /* Display as character with color */
    VioWrtCharStrAtt(&ch, 1, displayRow + 1, displayCol, &attr, 0);
    displayCol++;

    /* Wrap at column 80 */
    if (displayCol >= 80) {
        displayCol = 0;
        displayRow++;
        if (displayRow >= maxDisplayRow) {
            /* Scroll up */
            BYTE blank[2];
            blank[0] = ' ';
            blank[1] = ATTR_STATUS;
            VioScrollUp(1, 0, maxDisplayRow, 79, 1, blank, 0);
            displayRow = maxDisplayRow - 1;
        }
    }
}

static void WriteTrace(BYTE b, char direction)
{
    if (traceFile) {
        fputc(direction, traceFile);
        fputc(b, traceFile);
    }
}

static void DrawSignals(void)
{
    ULONG parmLen = 0, dataLen = 1;
    BYTE  data[1];
    char  buf[80];

    if (DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_GETMODEMINPUT,
                    NULL, 0, &parmLen, data, 1, &dataLen) == 0) {
        sprintf(buf, " CTS:%s  DSR:%s  RI:%s  DCD:%s ",
                (data[0] & 0x10) ? "ON " : "OFF",
                (data[0] & 0x20) ? "ON " : "OFF",
                (data[0] & 0x40) ? "ON " : "OFF",
                (data[0] & 0x80) ? "ON " : "OFF");
        VioWrtCharStrAtt(buf, strlen(buf), 21, 0, &attrStatus, 0);
    }
}

static void DrawBufferCounts(void)
{
    ULONG parmLen = 0, dataLen = 4;
    BYTE  data[4];
    char  buf[80];

    if (DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_GETINQUECOUNT,
                    NULL, 0, &parmLen, data, 4, &dataLen) == 0) {
        sprintf(buf, " Rx Chars: %5u ", *(USHORT *)&data[0]);
        VioWrtCharStrAtt(buf, strlen(buf), 22, 0, &attrStatus, 0);
    }

    dataLen = 4;
    if (DosDevIOCtl(hCom, IOCTL_ASYNC, ASYNC_GETOUTQUECOUNT,
                    NULL, 0, &parmLen, data, 4, &dataLen) == 0) {
        sprintf(buf, " Tx Chars: %5u ", *(USHORT *)&data[0]);
        VioWrtCharStrAtt(buf, strlen(buf), 22, 20, &attrStatus, 0);
    }
}

static void DrawKeyBar(void)
{
    char *bar = " Q=Quit  C=Clear                                                                ";
    VioWrtCharStrAtt(bar, 80, 24, 0, &attrKeys, 0);
}
