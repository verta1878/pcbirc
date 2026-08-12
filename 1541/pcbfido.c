/* ====================================================================
 * pcbfido.c — PCBoard FidoNet & Transfer Operations Console
 * ====================================================================
 * Text-mode console for monitoring and controlling FidoNet operations.
 * Called from pcbis_ui as the "FidoNet Console" screen or run standalone.
 *
 * Features:
 *   - 5,000-line ring buffer scrollback with ↑↓ PgUp/PgDn Home/End
 *   - Tagged output filtering: [B]inkP [T]IC [E]cho [F]REQ [X]fer [A]ll
 *   - Spawns child processes: pcbbinkp, pcbtic, pcbtoss, pcbftp
 *   - Live status bar with node info, queue counts, transfer progress
 *   - Interactive menu: poll, status, config, filter, verbose toggle
 *
 * All child tools write tagged stdout lines:
 *   [BINKP] [TIC] [TOSS] [FREQ] [FTP]
 * This console captures, stores, filters, and displays them.
 *
 * Builds with OpenWatcom: wcc386 -bt=os2 -5r -oxs pcbfido.c
 *
 * Copyright (C) 2026 pcbrevival contributors
 * License: GPLv3
 * ==================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef __OS2__
 #define INCL_DOS
 #define INCL_DOSQUEUES
 #define INCL_DOSERRORS
 #define INCL_KBD
 #include <os2.h>
#elif defined(__NT__) || defined(__WINDOWS__)
 #include <windows.h>
 #include <io.h>
 #include <process.h>
#else
 /* Linux/Unix */
 #include <unistd.h>
 #include <sys/wait.h>
 #include <fcntl.h>
 #include <termios.h>
 #include <sys/ioctl.h>
 #include <signal.h>
#endif

/* ====================================================================
 * ANSI Escape Helpers
 * ==================================================================== */

#define CSI     "\033["
#define CLS     CSI "2J" CSI "H"
#define BOLD    CSI "1m"
#define DIM     CSI "2m"
#define RESET   CSI "0m"
#define CYAN    CSI "36m"
#define YELLOW  CSI "33m"
#define WHITE   CSI "37m"
#define GREEN   CSI "32m"
#define RED     CSI "31m"
#define MAGENTA CSI "35m"
#define BGBLUE  CSI "44m"
#define BGBLACK CSI "40m"
#define REVERSE CSI "7m"

static void gotoxy(int x, int y) { printf(CSI "%d;%dH", y, x); }
static void clrline(void)        { printf(CSI "2K"); }

/* ====================================================================
 * Terminal I/O
 * ==================================================================== */

#if !defined(_WIN32) && !defined(__WATCOMC__) && !defined(__OS2__)
static struct termios g_orig_term;

static void term_raw(void) {
    struct termios t;
    tcgetattr(0, &g_orig_term);
    t = g_orig_term;
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN]  = 0;
    t.c_cc[VTIME] = 1;     /* 100ms timeout for non-blocking */
    tcsetattr(0, TCSANOW, &t);
}

static void term_restore(void) {
    tcsetattr(0, TCSANOW, &g_orig_term);
}

static int readkey(void) {
    int c = getchar();
    if (c == EOF)
        return 0;
    if (c == 27) {
        int c2 = getchar();
        if (c2 == EOF) return 27;       /* bare ESC */
        if (c2 == '[') {
            int c3 = getchar();
            switch (c3) {
            case 'A': return 0x148;     /* Up    */
            case 'B': return 0x150;     /* Down  */
            case 'C': return 0x14D;     /* Right */
            case 'D': return 0x14B;     /* Left  */
            case '5': getchar(); return 0x149; /* PgUp  */
            case '6': getchar(); return 0x151; /* PgDn  */
            case 'H': return 0x147;     /* Home  */
            case 'F': return 0x14F;     /* End   */
            default:  return 0;
            }
        }
        return 0;
    }
    return c;
}

#elif defined(_WIN32) || defined(__NT__)

static void term_raw(void) { /* Windows: conio handles it */ }
static void term_restore(void) { }

static int readkey(void) {
    int c;
    if (!_kbhit())
        return 0;
    c = _getch();
    if (c == 0 || c == 0xE0) {
        c = _getch();
        return 0x100 + c;      /* extended key */
    }
    return c;
}

#elif defined(__OS2__)

static void term_raw(void) { }
static void term_restore(void) { }

static int readkey(void) {
    KBDKEYINFO ki;
    ki.fbStatus = 0;
    KbdCharIn(&ki, IO_NOWAIT, 0);
    if (!(ki.fbStatus & 0x40))
        return 0;
    if (ki.chChar == 0 || ki.chChar == 0xE0)
        return 0x100 + ki.chScan;
    return ki.chChar;
}
#endif

/* ====================================================================
 * Scrollback Ring Buffer
 * ==================================================================== */

#define SCROLLBACK_SIZE 5000
#define LINE_MAXLEN     256

typedef struct {
    char  text[LINE_MAXLEN];
    char  tag;          /* 'B'=BinkP, 'T'=TIC, 'E'=Echo, 'F'=FREQ, 'X'=FTP, ' '=untagged */
    time_t timestamp;
} ScrollLine;

static ScrollLine g_scroll[SCROLLBACK_SIZE];
static int g_scroll_head  = 0;         /* next write position                */
static int g_scroll_count = 0;         /* total lines stored                 */
static int g_scroll_view  = 0;         /* view offset from newest (0=bottom) */

/* Current filter: 'A'=all, 'B', 'T', 'E', 'F', 'X' */
static char g_filter = 'A';

/* Screen dimensions */
static int g_screen_rows = 25;
static int g_screen_cols = 80;

/* Status */
static int g_binkp_queue  = 0;
static int g_tic_pending  = 0;
static int g_child_active = 0;
static char g_status_msg[128] = "Ready";

static char detect_tag(const char *line)
{
    if (strncmp(line, "[BINKP]", 7) == 0) return 'B';
    if (strncmp(line, "[TIC]",   5) == 0) return 'T';
    if (strncmp(line, "[TOSS]",  6) == 0) return 'E';
    if (strncmp(line, "[FREQ]",  6) == 0) return 'F';
    if (strncmp(line, "[FTP]",   5) == 0) return 'X';
    return ' ';
}

static void scroll_add(const char *line)
{
    ScrollLine *sl = &g_scroll[g_scroll_head];

    strncpy(sl->text, line, LINE_MAXLEN - 1);
    sl->text[LINE_MAXLEN - 1] = '\0';
    sl->tag = detect_tag(line);
    sl->timestamp = time(NULL);

    g_scroll_head = (g_scroll_head + 1) % SCROLLBACK_SIZE;
    if (g_scroll_count < SCROLLBACK_SIZE)
        g_scroll_count++;
}

/* Get the Nth line from bottom (0=newest) that matches filter.
 * Returns NULL if no match. */
static ScrollLine *scroll_get_filtered(int n)
{
    int found = 0;
    int idx;
    int i;

    for (i = 0; i < g_scroll_count; i++) {
        idx = (g_scroll_head - 1 - i + SCROLLBACK_SIZE) % SCROLLBACK_SIZE;
        if (g_filter == 'A' || g_scroll[idx].tag == g_filter) {
            if (found == n)
                return &g_scroll[idx];
            found++;
        }
    }
    return NULL;
}

/* Count filtered lines */
static int scroll_filtered_count(void)
{
    int count = 0;
    int i, idx;

    if (g_filter == 'A')
        return g_scroll_count;

    for (i = 0; i < g_scroll_count; i++) {
        idx = (g_scroll_head - 1 - i + SCROLLBACK_SIZE) % SCROLLBACK_SIZE;
        if (g_scroll[idx].tag == g_filter)
            count++;
    }
    return count;
}

/* ====================================================================
 * Screen Drawing
 * ==================================================================== */

static const char *tag_color(char tag)
{
    switch (tag) {
    case 'B': return CYAN;
    case 'T': return GREEN;
    case 'E': return YELLOW;
    case 'F': return MAGENTA;
    case 'X': return WHITE;
    default:  return DIM;
    }
}

static const char *filter_name(char f)
{
    switch (f) {
    case 'A': return "All";
    case 'B': return "BinkP";
    case 'T': return "TIC";
    case 'E': return "Echo";
    case 'F': return "FREQ";
    case 'X': return "FTP";
    default:  return "?";
    }
}

static void draw_titlebar(void)
{
    gotoxy(1, 1);
    printf(BOLD BGBLUE WHITE);
    printf(" PCBoard FidoNet Console");
    /* Pad to screen width */
    {
        int i;
        for (i = 24; i < g_screen_cols; i++) putchar(' ');
    }
    printf(RESET);
}

static void draw_statusbar(void)
{
    gotoxy(1, g_screen_rows - 1);
    printf(BOLD BGBLUE WHITE);
    clrline();
    printf(" Filter:[%s]  Queue:%d  TIC:%d  %s%s",
           filter_name(g_filter),
           g_binkp_queue, g_tic_pending,
           g_child_active ? "ACTIVE " : "",
           g_status_msg);
    {
        int i;
        int len = 30 + (int)strlen(filter_name(g_filter)) +
                  (int)strlen(g_status_msg);
        for (i = len; i < g_screen_cols; i++) putchar(' ');
    }
    printf(RESET);
}

static void draw_menubar(void)
{
    gotoxy(1, g_screen_rows);
    printf(BOLD BGBLUE WHITE);
    clrline();
    printf(" A"YELLOW"ddr "WHITE"N"YELLOW"odes "WHITE"B"YELLOW"inkP "
           WHITE"T"YELLOW"IC "WHITE"E"YELLOW"cho "WHITE"M"YELLOW"agic "
           WHITE"Q"YELLOW"ueue "WHITE"S"YELLOW"tat "WHITE"P"YELLOW"oll "
           WHITE"F"YELLOW"iltr "WHITE"ESC"YELLOW" Quit");
    printf(RESET);
}

static void draw_scrollback(void)
{
    int view_lines = g_screen_rows - 3;  /* title + status + menu */
    int i;

    for (i = 0; i < view_lines; i++) {
        int line_idx = g_scroll_view + (view_lines - 1 - i);
        ScrollLine *sl = scroll_get_filtered(line_idx);

        gotoxy(1, i + 2);
        clrline();

        if (sl) {
            printf("%s%s" RESET, tag_color(sl->tag), sl->text);
        }
    }
}

static void draw_screen(void)
{
    draw_titlebar();
    draw_scrollback();
    draw_statusbar();
    draw_menubar();
    fflush(stdout);
}

/* ====================================================================
 * Child Process Management
 * ==================================================================== */

#ifdef __OS2__

/* OS/2: spawn child, capture stdout via pipe */
static int run_child(const char *cmd)
{
    HFILE hRead, hWrite;
    RESULTCODES rc;
    char buf[512];
    ULONG bytesRead;
    APIRET api_rc;

    /* Create pipe */
    api_rc = DosCreatePipe(&hRead, &hWrite, 4096);
    if (api_rc != 0) {
        scroll_add("[FIDO] ERROR: DosCreatePipe failed");
        return -1;
    }

    /* TODO: full OS/2 DosExecPgm with stdout redirect to pipe */
    scroll_add("[FIDO] OS/2 child spawn not yet implemented");
    DosClose(hRead);
    DosClose(hWrite);
    return -1;
}

#elif defined(__NT__) || defined(__WINDOWS__)

/* Windows: spawn child, capture stdout via pipe */
static int run_child(const char *cmd)
{
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    char buf[512];
    DWORD bytesRead;
    char cmdbuf[512];

    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        scroll_add("[FIDO] ERROR: CreatePipe failed");
        return -1;
    }

    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.hStdOutput = hWritePipe;
    si.hStdError  = hWritePipe;
    si.dwFlags = STARTF_USESTDHANDLES;

    strncpy(cmdbuf, cmd, sizeof(cmdbuf) - 1);
    cmdbuf[sizeof(cmdbuf) - 1] = '\0';

    memset(&pi, 0, sizeof(pi));

    g_child_active = 1;
    snprintf(buf, sizeof(buf), "[FIDO] Running: %s", cmd);
    scroll_add(buf);

    if (!CreateProcess(NULL, cmdbuf, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        scroll_add("[FIDO] ERROR: CreateProcess failed");
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        g_child_active = 0;
        return -1;
    }

    CloseHandle(hWritePipe);

    /* Read child output line by line */
    {
        char linebuf[LINE_MAXLEN];
        int linepos = 0;

        while (ReadFile(hReadPipe, buf, sizeof(buf) - 1, &bytesRead, NULL) && bytesRead > 0) {
            DWORD i;
            for (i = 0; i < bytesRead; i++) {
                if (buf[i] == '\n' || buf[i] == '\r') {
                    if (linepos > 0) {
                        linebuf[linepos] = '\0';
                        scroll_add(linebuf);
                        linepos = 0;
                    }
                } else if (linepos < LINE_MAXLEN - 1) {
                    linebuf[linepos++] = buf[i];
                }
            }
            /* Redraw scrollback periodically */
            draw_scrollback();
            draw_statusbar();
            fflush(stdout);
        }

        if (linepos > 0) {
            linebuf[linepos] = '\0';
            scroll_add(linebuf);
        }
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hReadPipe);

    g_child_active = 0;
    scroll_add("[FIDO] Child process complete");
    return 0;
}

#else /* Linux/Unix */

static int run_child(const char *cmd)
{
    int pipefd[2];
    pid_t pid;
    char buf[512];

    if (pipe(pipefd) < 0) {
        scroll_add("[FIDO] ERROR: pipe() failed");
        return -1;
    }

    pid = fork();
    if (pid < 0) {
        scroll_add("[FIDO] ERROR: fork() failed");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {
        /* Child */
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        _exit(127);
    }

    /* Parent */
    close(pipefd[1]);
    g_child_active = 1;

    snprintf(buf, sizeof(buf), "[FIDO] Running: %s", cmd);
    scroll_add(buf);

    /* Read child output */
    {
        char linebuf[LINE_MAXLEN];
        int linepos = 0;
        int n;

        /* Set non-blocking on pipe */
        fcntl(pipefd[0], F_SETFL, O_NONBLOCK);

        while (1) {
            n = read(pipefd[0], buf, sizeof(buf) - 1);
            if (n > 0) {
                int i;
                for (i = 0; i < n; i++) {
                    if (buf[i] == '\n' || buf[i] == '\r') {
                        if (linepos > 0) {
                            linebuf[linepos] = '\0';
                            scroll_add(linebuf);
                            linepos = 0;
                        }
                    } else if (linepos < LINE_MAXLEN - 1) {
                        linebuf[linepos++] = buf[i];
                    }
                }
                draw_scrollback();
                draw_statusbar();
                fflush(stdout);
            } else if (n == 0) {
                break;  /* EOF */
            } else {
                /* EAGAIN — check if child still alive */
                int status;
                if (waitpid(pid, &status, WNOHANG) != 0)
                    break;
                usleep(50000);  /* 50ms */
            }
        }

        if (linepos > 0) {
            linebuf[linepos] = '\0';
            scroll_add(linebuf);
        }
    }

    close(pipefd[0]);
    waitpid(pid, NULL, 0);

    g_child_active = 0;
    scroll_add("[FIDO] Child process complete");
    return 0;
}
#endif

/* ====================================================================
 * Menu Commands
 * ==================================================================== */

/* Poll a node — prompts for address or uses default */
static void cmd_poll(void)
{
    char addr[80];
    char cmd[260];

    gotoxy(1, g_screen_rows);
    clrline();
    printf(BOLD WHITE " Poll address (z:n/n[.p]): " RESET);
    fflush(stdout);

    /* Switch to line mode briefly */
    term_restore();
    if (fgets(addr, sizeof(addr), stdin) == NULL) {
        term_raw();
        return;
    }
    term_raw();

    /* Trim */
    {
        char *p = addr + strlen(addr) - 1;
        while (p >= addr && (*p == '\r' || *p == '\n' || *p == ' '))
            *p-- = '\0';
    }

    if (strlen(addr) == 0)
        return;

    snprintf(cmd, sizeof(cmd), "pcbbinkp poll %s", addr);
    run_child(cmd);
    draw_screen();
}

/* Show queue status */
static void cmd_status(void)
{
    run_child("pcbbinkp status");
    draw_screen();
}

/* Cycle filter */
static void cmd_filter(void)
{
    static const char filters[] = "ABTEFX";
    char *p = strchr(filters, g_filter);
    if (p && *(p + 1))
        g_filter = *(p + 1);
    else
        g_filter = 'A';

    g_scroll_view = 0;     /* reset view to bottom */

    {
        char msg[64];
        snprintf(msg, sizeof(msg), "[FIDO] Filter: %s", filter_name(g_filter));
        scroll_add(msg);
    }
}

/* ====================================================================
 * Main Console Loop
 * ==================================================================== */

int pcbfido_main(int argc, char *argv[])
{
    int running = 1;
    int key;
    int view_lines;

    /* Detect screen size */
#if !defined(_WIN32) && !defined(__WATCOMC__) && !defined(__OS2__)
    {
        struct winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
            g_screen_rows = ws.ws_row;
            g_screen_cols = ws.ws_col;
        }
    }
#elif defined(__NT__) || defined(_WIN32)
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            g_screen_cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            g_screen_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        }
    }
#endif

    view_lines = g_screen_rows - 3;

    term_raw();
    printf(CLS);

    scroll_add("[FIDO] PCBoard FidoNet Console started");
    scroll_add("[FIDO] Press F or TAB to cycle filter, P to poll, S for status");
    scroll_add("[FIDO] ESC to exit");

    draw_screen();

    while (running) {
        key = readkey();
        if (key == 0) {
            /* No key — could check for child output here */
#if !defined(_WIN32) && !defined(__WATCOMC__)
            usleep(50000);      /* 50ms idle */
#elif defined(__NT__) || defined(_WIN32)
            Sleep(50);
#elif defined(__OS2__)
            DosSleep(50);
#endif
            continue;
        }

        switch (key) {
        case 27:        /* ESC — quit */
            running = 0;
            break;

        case 'p': case 'P':
            cmd_poll();
            break;

        case 's': case 'S':
            cmd_status();
            break;

        case 'f': case 'F': case 9:    /* F or TAB = cycle filter */
            cmd_filter();
            draw_screen();
            break;

        /* Direct filter keys */
        case 'b': case 'B':
            g_filter = 'B'; g_scroll_view = 0; draw_screen();
            break;
        case 't': case 'T':
            g_filter = 'T'; g_scroll_view = 0; draw_screen();
            break;
        case 'e': case 'E':
            g_filter = 'E'; g_scroll_view = 0; draw_screen();
            break;
        case 'a': case 'A':
            g_filter = 'A'; g_scroll_view = 0; draw_screen();
            break;
        case 'x': case 'X':
            g_filter = 'X'; g_scroll_view = 0; draw_screen();
            break;
        case 'm': case 'M':
            g_filter = 'F'; g_scroll_view = 0; draw_screen();
            break;

        case 'q': case 'Q':    /* Queue editor (future) */
            scroll_add("[FIDO] Queue editor not yet implemented");
            draw_screen();
            break;

        /* Navigation */
        case 0x148:     /* Up */
            if (g_scroll_view < scroll_filtered_count() - view_lines)
                g_scroll_view++;
            draw_scrollback();
            fflush(stdout);
            break;

        case 0x150:     /* Down */
            if (g_scroll_view > 0)
                g_scroll_view--;
            draw_scrollback();
            fflush(stdout);
            break;

        case 0x149:     /* PgUp */
            g_scroll_view += view_lines;
            if (g_scroll_view > scroll_filtered_count() - view_lines)
                g_scroll_view = scroll_filtered_count() - view_lines;
            if (g_scroll_view < 0) g_scroll_view = 0;
            draw_scrollback();
            fflush(stdout);
            break;

        case 0x151:     /* PgDn */
            g_scroll_view -= view_lines;
            if (g_scroll_view < 0) g_scroll_view = 0;
            draw_scrollback();
            fflush(stdout);
            break;

        case 0x147:     /* Home */
            g_scroll_view = scroll_filtered_count() - view_lines;
            if (g_scroll_view < 0) g_scroll_view = 0;
            draw_scrollback();
            fflush(stdout);
            break;

        case 0x14F:     /* End */
            g_scroll_view = 0;
            draw_scrollback();
            fflush(stdout);
            break;
        }
    }

    term_restore();
    printf(CLS);
    printf("FidoNet Console exited.\n");

    return 0;
}

/* ====================================================================
 * Standalone entry point
 * When built as standalone EXE, main() calls pcbfido_main().
 * When linked into pcbis_ui, pcbis_ui calls pcbfido_main() directly.
 * ==================================================================== */

#ifndef PCBFIDO_LIBRARY
int main(int argc, char *argv[])
{
    return pcbfido_main(argc, argv);
}
#endif
