/*
 * pcbis_ui.c — PCBoard Installation System Configuration UI
 * Inspired by Mystic BBS MCFG (GPL v3.0, James Coyle)
 * Part of pcbrevival (GPL v3.0)
 *
 * Text UI for configuring PCBoard 15.4 + netmodem2irc stack.
 * Uses ANSI escape codes for box drawing and menu navigation.
 * Compiles with gcc (Linux) and Watcom (DOS).
 *
 * Usage: pcbis_ui [pcbis.cfg]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32) || defined(__WATCOMC__)
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

/* ── ANSI helpers ──────────────────────────────────────────────── */

#define CSI "\033["
#define CLS CSI "2J" CSI "H"
#define BOLD CSI "1m"
#define DIM CSI "2m"
#define RESET CSI "0m"
#define CYAN CSI "36m"
#define YELLOW CSI "33m"
#define WHITE CSI "37m"
#define BGBLUE CSI "44m"
#define BGBLACK CSI "40m"
#define REVERSE CSI "7m"

static void gotoxy(int x, int y) { printf(CSI "%d;%dH", y, x); }
static void setcolor(const char *c) { printf("%s", c); }
static void clearscreen(void) { printf(CLS); }

/* ── Raw terminal ──────────────────────────────────────────────── */

#if !defined(_WIN32) && !defined(__WATCOMC__)
static struct termios orig_term;
static void term_raw(void) {
    struct termios t;
    tcgetattr(0, &orig_term);
    t = orig_term;
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &t);
}
static void term_restore(void) { tcsetattr(0, TCSANOW, &orig_term); }
static int readkey(void) {
    int c = getchar();
    if (c == 27) {
        int c2 = getchar();
        if (c2 == '[') {
            int c3 = getchar();
            switch (c3) {
                case 'A': return 256; /* up */
                case 'B': return 257; /* down */
                case 'C': return 258; /* right */
                case 'D': return 259; /* left */
            }
        }
        return 27;
    }
    return c;
}
#else
static void term_raw(void) {}
static void term_restore(void) {}
static int readkey(void) {
    int c = getch();
    if (c == 0 || c == 0xE0) {
        c = getch();
        switch (c) {
            case 72: return 256;
            case 80: return 257;
            case 77: return 258;
            case 75: return 259;
        }
    }
    return c;
}
#endif

#define KEY_UP    256
#define KEY_DOWN  257
#define KEY_RIGHT 258
#define KEY_LEFT  259
#define KEY_ENTER 13
#define KEY_ESC   27
#define KEY_TAB   9

/* ── Box drawing ───────────────────────────────────────────────── */

static void draw_box(int x, int y, int w, int h, const char *title, const char *color) {
    int i, j;
    setcolor(color);
    gotoxy(x, y);
    printf("\xc9"); for (i = 0; i < w-2; i++) printf("\xcd"); printf("\xbb");
    for (i = 1; i < h-1; i++) {
        gotoxy(x, y+i);
        printf("\xba");
        gotoxy(x+w-1, y+i);
        printf("\xba");
    }
    gotoxy(x, y+h-1);
    printf("\xc8"); for (i = 0; i < w-2; i++) printf("\xcd"); printf("\xbc");
    if (title && *title) {
        int tlen; tlen = (int)strlen(title);
        gotoxy(x + (w - tlen - 4) / 2, y);
        printf("\xcd\xb5 %s%s%s %s\xc6\xcd", BOLD WHITE, title, RESET, color);
    }
    setcolor(RESET);
}

/* Use UTF-8 box drawing if terminal supports it */
static void draw_box_utf8(int x, int y, int w, int h, const char *title) {
    int i, j;
    setcolor(CYAN);
    gotoxy(x, y);
    printf("┌"); for (i = 0; i < w-2; i++) printf("─"); printf("┐");
    for (i = 1; i < h-1; i++) {
        gotoxy(x, y+i);
        printf("│");
        for (j = 0; j < w-2; j++) printf(" ");
        printf("│");
    }
    gotoxy(x, y+h-1);
    printf("└"); for (i = 0; i < w-2; i++) printf("─"); printf("┘");
    if (title && *title) {
        int tlen; tlen = (int)strlen(title);
        gotoxy(x + (w - tlen - 4) / 2, y);
        printf("─┤ " BOLD WHITE "%s" RESET CYAN " ├─", title);
    }
    setcolor(RESET);
}

/* ── Config data ───────────────────────────────────────────────── */

typedef struct {
    char bbs_name[80];
    char sysop_name[40];
    int  telnet_port;
    int  nodes;
    char pcb_root[256];
    char pcb_data[256];
    char fossil_driver[80];
    char netmodem_path[256];
    int  dosbox_cycles;
    char dosbox_output[20];
    /* FidoNet */
    int  fido_enabled;
    char fido_address[40];
    char fido_hub[80];
    int  fido_binkp_port;
    char fido_inbound[256];
    char fido_outbound[256];
    char fido_nodelist[256];
    /* Web Server */
    int  web_enabled;
    int  web_port;
    char web_root[256];
    char web_title[80];
    int  web_ansi_preview;
    int  web_file_browser;
    /* FTP Server */
    int  ftp_enabled;
    int  ftp_port;
    char ftp_root[256];
    int  ftp_anonymous;
    int  ftp_max_connections;
    char ftp_banner[80];
} pcbis_config;

static pcbis_config cfg;

static void config_defaults(pcbis_config *c) {
    memset(c, 0, sizeof(*c));
    strcpy(c->bbs_name, "PCBoard BBS");
    strcpy(c->sysop_name, "SYSOP");
    c->telnet_port = 23;
    c->nodes = 1;
    strcpy(c->pcb_root, "/home/pcboard");
    strcpy(c->pcb_data, "/home/pcboard/data");
    strcpy(c->fossil_driver, "ADF");
    strcpy(c->netmodem_path, "/home/pcboard/netmodem");
    c->dosbox_cycles = 0; /* max */
    strcpy(c->dosbox_output, "surface");
    c->fido_enabled = 0;
    strcpy(c->fido_address, "1:1/1.0");
    strcpy(c->fido_hub, "");
    c->fido_binkp_port = 24554;
    strcpy(c->fido_inbound, "/home/pcboard/fido/inbound");
    strcpy(c->fido_outbound, "/home/pcboard/fido/outbound");
    strcpy(c->fido_nodelist, "/home/pcboard/fido/nodelist");
    /* Web */
    c->web_enabled = 0;
    c->web_port = 8080;
    strcpy(c->web_root, "/home/pcboard/DATA/default/www");
    strcpy(c->web_title, "PCBoard BBS");
    c->web_ansi_preview = 1;
    c->web_file_browser = 1;
    /* FTP */
    c->ftp_enabled = 0;
    c->ftp_port = 21;
    strcpy(c->ftp_root, "/home/pcboard/files");
    c->ftp_anonymous = 0;
    c->ftp_max_connections = 10;
    strcpy(c->ftp_banner, "PCBoard 15.4 FTP Server");
}

static void config_load(pcbis_config *c, const char *path) {
    FILE *f = fopen(path, "r");
    char line[512], key[64], val[256];
    if (!f) return;
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        if (sscanf(line, "%63[^=]=%255[^\n]", key, val) == 2) {
            /* trim */
            char *p = key + strlen(key) - 1;
            while (p >= key && *p == ' ') *p-- = 0;
            p = val; while (*p == ' ') p++;
            if (!strcmp(key, "bbs_name")) strncpy(c->bbs_name, p, 79);
            else if (!strcmp(key, "sysop_name")) strncpy(c->sysop_name, p, 39);
            else if (!strcmp(key, "telnet_port")) c->telnet_port = atoi(p);
            else if (!strcmp(key, "nodes")) c->nodes = atoi(p);
            else if (!strcmp(key, "pcb_root")) strncpy(c->pcb_root, p, 255);
            else if (!strcmp(key, "pcb_data")) strncpy(c->pcb_data, p, 255);
            else if (!strcmp(key, "fossil_driver")) strncpy(c->fossil_driver, p, 79);
            else if (!strcmp(key, "netmodem_path")) strncpy(c->netmodem_path, p, 255);
            else if (!strcmp(key, "dosbox_cycles")) c->dosbox_cycles = atoi(p);
            else if (!strcmp(key, "dosbox_output")) strncpy(c->dosbox_output, p, 19);
            else if (!strcmp(key, "fido_enabled")) c->fido_enabled = atoi(p);
            else if (!strcmp(key, "fido_address")) strncpy(c->fido_address, p, 39);
            else if (!strcmp(key, "fido_hub")) strncpy(c->fido_hub, p, 79);
            else if (!strcmp(key, "fido_binkp_port")) c->fido_binkp_port = atoi(p);
            else if (!strcmp(key, "fido_inbound")) strncpy(c->fido_inbound, p, 255);
            else if (!strcmp(key, "fido_outbound")) strncpy(c->fido_outbound, p, 255);
            else if (!strcmp(key, "fido_nodelist")) strncpy(c->fido_nodelist, p, 255);
            else if (!strcmp(key, "web_enabled")) c->web_enabled = atoi(p);
            else if (!strcmp(key, "web_port")) c->web_port = atoi(p);
            else if (!strcmp(key, "web_root")) strncpy(c->web_root, p, 255);
            else if (!strcmp(key, "web_title")) strncpy(c->web_title, p, 79);
            else if (!strcmp(key, "web_ansi_preview")) c->web_ansi_preview = atoi(p);
            else if (!strcmp(key, "web_file_browser")) c->web_file_browser = atoi(p);
            else if (!strcmp(key, "ftp_enabled")) c->ftp_enabled = atoi(p);
            else if (!strcmp(key, "ftp_port")) c->ftp_port = atoi(p);
            else if (!strcmp(key, "ftp_root")) strncpy(c->ftp_root, p, 255);
            else if (!strcmp(key, "ftp_anonymous")) c->ftp_anonymous = atoi(p);
            else if (!strcmp(key, "ftp_max_connections")) c->ftp_max_connections = atoi(p);
            else if (!strcmp(key, "ftp_banner")) strncpy(c->ftp_banner, p, 79);
        }
    }
    fclose(f);
}

static void config_save(pcbis_config *c, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) { printf("Error: cannot write %s\n", path); return; }
    fprintf(f, "# pcbis.cfg — PCBoard Installation System\n");
    fprintf(f, "# Generated by pcbis -cfg\n\n");
    fprintf(f, "# BBS Settings\n");
    fprintf(f, "bbs_name=%s\n", c->bbs_name);
    fprintf(f, "sysop_name=%s\n", c->sysop_name);
    fprintf(f, "telnet_port=%d\n", c->telnet_port);
    fprintf(f, "nodes=%d\n", c->nodes);
    fprintf(f, "\n# Paths\n");
    fprintf(f, "pcb_root=%s\n", c->pcb_root);
    fprintf(f, "pcb_data=%s\n", c->pcb_data);
    fprintf(f, "fossil_driver=%s\n", c->fossil_driver);
    fprintf(f, "netmodem_path=%s\n", c->netmodem_path);
    fprintf(f, "\n# DOSBox\n");
    fprintf(f, "dosbox_cycles=%d\n", c->dosbox_cycles);
    fprintf(f, "dosbox_output=%s\n", c->dosbox_output);
    fprintf(f, "\n# FidoNet\n");
    fprintf(f, "fido_enabled=%d\n", c->fido_enabled);
    fprintf(f, "fido_address=%s\n", c->fido_address);
    fprintf(f, "fido_hub=%s\n", c->fido_hub);
    fprintf(f, "fido_binkp_port=%d\n", c->fido_binkp_port);
    fprintf(f, "fido_inbound=%s\n", c->fido_inbound);
    fprintf(f, "fido_outbound=%s\n", c->fido_outbound);
    fprintf(f, "fido_nodelist=%s\n", c->fido_nodelist);
    fprintf(f, "\n# Web Server\n");
    fprintf(f, "web_enabled=%d\n", c->web_enabled);
    fprintf(f, "web_port=%d\n", c->web_port);
    fprintf(f, "web_root=%s\n", c->web_root);
    fprintf(f, "web_title=%s\n", c->web_title);
    fprintf(f, "web_ansi_preview=%d\n", c->web_ansi_preview);
    fprintf(f, "web_file_browser=%d\n", c->web_file_browser);
    fprintf(f, "\n# FTP Server\n");
    fprintf(f, "ftp_enabled=%d\n", c->ftp_enabled);
    fprintf(f, "ftp_port=%d\n", c->ftp_port);
    fprintf(f, "ftp_root=%s\n", c->ftp_root);
    fprintf(f, "ftp_anonymous=%d\n", c->ftp_anonymous);
    fprintf(f, "ftp_max_connections=%d\n", c->ftp_max_connections);
    fprintf(f, "ftp_banner=%s\n", c->ftp_banner);
    fclose(f);
}

/* ── Menu items ────────────────────────────────────────────────── */

typedef enum { FT_STR, FT_INT, FT_BOOL, FT_PATH } field_type;

typedef struct {
    char        hotkey;
    const char *label;
    field_type  type;
    void       *data;
    int         maxlen;
    const char *help;
} menu_field;

static void edit_string(int x, int y, int maxlen, char *buf) {
    int pos, k;
    pos = (int)strlen(buf);
    gotoxy(x, y);
    setcolor(BGBLUE WHITE);
    printf("%-*s", maxlen, buf);
    gotoxy(x + pos, y);
    fflush(stdout);
    while (1) {
        k = readkey();
        if (k == KEY_ENTER || k == KEY_ESC || k == KEY_TAB) break;
        if (k == 8 || k == 127) { /* backspace */
            if (pos > 0) { buf[--pos] = 0; }
        } else if (k >= 32 && k < 127 && pos < maxlen - 1) {
            buf[pos++] = (char)k;
            buf[pos] = 0;
        }
        gotoxy(x, y);
        printf("%-*s", maxlen, buf);
        gotoxy(x + pos, y);
        fflush(stdout);
    }
    setcolor(RESET);
}

static void edit_int(int x, int y, int *val) {
    char buf[16];
    sprintf(buf, "%d", *val);
    edit_string(x, y, 8, buf);
    *val = atoi(buf);
}

static void edit_bool(int x, int y, int *val) {
    *val = !(*val);
}

static void draw_field(int x, int y, menu_field *fld, int selected) {
    char valbuf[80];
    gotoxy(x, y);
    if (selected) setcolor(REVERSE CYAN);
    else setcolor(WHITE);
    printf(" %c ", fld->hotkey);
    setcolor(selected ? BOLD WHITE : DIM WHITE);
    printf("%-20s ", fld->label);
    setcolor(selected ? BOLD YELLOW : WHITE);
    switch (fld->type) {
        case FT_STR:
        case FT_PATH:
            printf("%-40.40s", (char *)fld->data);
            break;
        case FT_INT:
            printf("%-40d", *(int *)fld->data);
            break;
        case FT_BOOL:
            printf("%-40s", *(int *)fld->data ? "Yes" : "No");
            break;
    }
    setcolor(RESET);
}

/* ── Main menu screens ─────────────────────────────────────────── */

static void screen_general(void) {
    menu_field fields[] = {
        {'N', "BBS Name",       FT_STR,  cfg.bbs_name,      79, "Name of your BBS"},
        {'S', "Sysop Name",     FT_STR,  cfg.sysop_name,    39, "Sysop login name"},
        {'P', "Telnet Port",    FT_INT,  &cfg.telnet_port,    0, "TCP port for telnet"},
        {'O', "Nodes",          FT_INT,  &cfg.nodes,          0, "Simultaneous callers"},
    };
    int nf, sel, k, i;
    nf = 4; sel = 0;
    while (1) {
        draw_box_utf8(2, 2, 72, nf + 4, "General Settings");
        for (i = 0; i < nf; i++)
            draw_field(4, 4 + i, &fields[i], i == sel);
        gotoxy(3, 4 + nf + 1);
        setcolor(DIM);
        printf(" %s", fields[sel].help);
        printf("                              ");
        setcolor(RESET);
        fflush(stdout);
        k = readkey();
        if (k == KEY_ESC || k == 'q') break;
        if (k == KEY_UP && sel > 0) sel--;
        if (k == KEY_DOWN && sel < nf - 1) sel++;
        if (k == KEY_ENTER) {
            switch (fields[sel].type) {
                case FT_STR:
                case FT_PATH: edit_string(28, 4 + sel, 40, (char *)fields[sel].data); break;
                case FT_INT:  edit_int(28, 4 + sel, (int *)fields[sel].data); break;
                case FT_BOOL: edit_bool(28, 4 + sel, (int *)fields[sel].data); break;
            }
        }
        if (k >= 'A' && k <= 'Z') k = tolower(k);
        for (i = 0; i < nf; i++)
            if (tolower(fields[i].hotkey) == k) { sel = i; break; }
    }
}

static void screen_paths(void) {
    menu_field fields[] = {
        {'R', "PCBoard Root",   FT_PATH, cfg.pcb_root,     255, "PCBoard installation directory"},
        {'D', "Data Directory", FT_PATH, cfg.pcb_data,     255, "PCBoard data files"},
        {'F', "FOSSIL Driver",  FT_STR,  cfg.fossil_driver,  79, "ADF, X00, BNU, or netfosdl"},
        {'M', "NetModem Path",  FT_PATH, cfg.netmodem_path,255, "netmodem2irc location"},
    };
    int nf, sel, k, i;
    nf = 4; sel = 0;
    while (1) {
        draw_box_utf8(2, 2, 72, nf + 4, "Paths & Directories");
        for (i = 0; i < nf; i++)
            draw_field(4, 4 + i, &fields[i], i == sel);
        gotoxy(3, 4 + nf + 1);
        setcolor(DIM);
        printf(" %s", fields[sel].help);
        printf("                              ");
        setcolor(RESET);
        fflush(stdout);
        k = readkey();
        if (k == KEY_ESC || k == 'q') break;
        if (k == KEY_UP && sel > 0) sel--;
        if (k == KEY_DOWN && sel < nf - 1) sel++;
        if (k == KEY_ENTER) {
            edit_string(28, 4 + sel, 40, (char *)fields[sel].data);
        }
        if (k >= 'A' && k <= 'Z') k = tolower(k);
        for (i = 0; i < nf; i++)
            if (tolower(fields[i].hotkey) == k) { sel = i; break; }
    }
}

static void screen_fidonet(void) {
    menu_field fields[] = {
        {'E', "FidoNet Enabled",FT_BOOL, &cfg.fido_enabled,   0, "Enable FidoNet echomail"},
        {'A', "FTN Address",    FT_STR,  cfg.fido_address,   39, "Your FidoNet address (zone:net/node.point)"},
        {'H', "Hub Address",    FT_STR,  cfg.fido_hub,       79, "Uplink hub address or hostname"},
        {'B', "Binkp Port",     FT_INT,  &cfg.fido_binkp_port, 0, "BinkP TCP port (default 24554)"},
        {'I', "Inbound Path",   FT_PATH, cfg.fido_inbound,  255, "Incoming .PKT directory"},
        {'O', "Outbound Path",  FT_PATH, cfg.fido_outbound, 255, "Outgoing .PKT directory"},
        {'N', "Nodelist Path",  FT_PATH, cfg.fido_nodelist,  255, "NODELIST.DBF/NDX directory"},
    };
    int nf, sel, k, i; nf = 7; sel = 0;
    while (1) {
        draw_box_utf8(2, 2, 72, nf + 4, "FidoNet Configuration");
        for (i = 0; i < nf; i++)
            draw_field(4, 4 + i, &fields[i], i == sel);
        gotoxy(3, 4 + nf + 1);
        setcolor(DIM);
        printf(" %s", fields[sel].help);
        printf("                              ");
        setcolor(RESET);
        fflush(stdout);
        k = readkey();
        if (k == KEY_ESC || k == 'q') break;
        if (k == KEY_UP && sel > 0) sel--;
        if (k == KEY_DOWN && sel < nf - 1) sel++;
        if (k == KEY_ENTER) {
            switch (fields[sel].type) {
                case FT_STR:
                case FT_PATH: edit_string(28, 4 + sel, 40, (char *)fields[sel].data); break;
                case FT_INT:  edit_int(28, 4 + sel, (int *)fields[sel].data); break;
                case FT_BOOL: edit_bool(28, 4 + sel, (int *)fields[sel].data); break;
            }
        }
        if (k >= 'A' && k <= 'Z') k = tolower(k);
        for (i = 0; i < nf; i++)
            if (tolower(fields[i].hotkey) == k) { sel = i; break; }
    }
}

static void screen_dosbox(void) {
    menu_field fields[] = {
        {'C', "CPU Cycles",     FT_INT,  &cfg.dosbox_cycles,  0, "0=max, or fixed cycle count"},
        {'O', "Video Output",   FT_STR,  cfg.dosbox_output,  19, "surface, overlay, opengl"},
    };
    int nf, sel, k, i; nf = 2; sel = 0;
    while (1) {
        draw_box_utf8(2, 2, 72, nf + 4, "DOSBox Settings");
        for (i = 0; i < nf; i++)
            draw_field(4, 4 + i, &fields[i], i == sel);
        gotoxy(3, 4 + nf + 1);
        setcolor(DIM);
        printf(" %s", fields[sel].help);
        printf("                              ");
        setcolor(RESET);
        fflush(stdout);
        k = readkey();
        if (k == KEY_ESC || k == 'q') break;
        if (k == KEY_UP && sel > 0) sel--;
        if (k == KEY_DOWN && sel < nf - 1) sel++;
        if (k == KEY_ENTER) {
            switch (fields[sel].type) {
                case FT_STR: edit_string(28, 4 + sel, 40, (char *)fields[sel].data); break;
                case FT_INT: edit_int(28, 4 + sel, (int *)fields[sel].data); break;
                default: break;
            }
        }
    }
}


static void screen_web(void) {
    menu_field fields[] = {
        {'E', "Web Enabled",     FT_BOOL, &cfg.web_enabled,     0, "Enable built-in web server"},
        {'P', "HTTP Port",       FT_INT,  &cfg.web_port,        0, "Web server TCP port"},
        {'R', "Web Root",        FT_PATH, cfg.web_root,       255, "Web content directory"},
        {'T', "Page Title",      FT_STR,  cfg.web_title,       79, "HTML page title"},
        {'A', "ANSI Preview",    FT_BOOL, &cfg.web_ansi_preview, 0, "Show ANSI art previews in browser"},
        {'F', "File Browser",    FT_BOOL, &cfg.web_file_browser, 0, "Enable web file area browser"},
    };
    int nf, sel, k, i; nf = 6; sel = 0;
    while (1) {
        draw_box_utf8(2, 2, 72, nf + 4, "Web Server Configuration");
        for (i = 0; i < nf; i++)
            draw_field(4, 4 + i, &fields[i], i == sel);
        gotoxy(3, 4 + nf + 1);
        setcolor(DIM);
        printf(" %s", fields[sel].help);
        printf("                                       ");
        setcolor(RESET);
        fflush(stdout);
        k = readkey();
        if (k == KEY_ESC || k == 'q') break;
        if (k == KEY_UP && sel > 0) sel--;
        if (k == KEY_DOWN && sel < nf - 1) sel++;
        if (k == KEY_ENTER) {
            switch (fields[sel].type) {
                case FT_STR:
                case FT_PATH: edit_string(28, 4 + sel, 40, (char *)fields[sel].data); break;
                case FT_INT:  edit_int(28, 4 + sel, (int *)fields[sel].data); break;
                case FT_BOOL: edit_bool(28, 4 + sel, (int *)fields[sel].data); break;
            }
        }
        if (k >= 'A' && k <= 'Z') k = tolower(k);
        for (i = 0; i < nf; i++)
            if (tolower(fields[i].hotkey) == k) { sel = i; break; }
    }
}

static void screen_ftp(void) {
    menu_field fields[] = {
        {'E', "FTP Enabled",     FT_BOOL, &cfg.ftp_enabled,       0, "Enable built-in FTP server"},
        {'P', "FTP Port",        FT_INT,  &cfg.ftp_port,          0, "FTP server TCP port"},
        {'R', "FTP Root",        FT_PATH, cfg.ftp_root,         255, "FTP file root directory"},
        {'A', "Anonymous Login", FT_BOOL, &cfg.ftp_anonymous,     0, "Allow anonymous FTP access"},
        {'M', "Max Connections", FT_INT,  &cfg.ftp_max_connections,0, "Maximum simultaneous FTP sessions"},
        {'B', "Banner",          FT_STR,  cfg.ftp_banner,        79, "FTP welcome banner message"},
    };
    int nf, sel, k, i; nf = 6; sel = 0;
    while (1) {
        draw_box_utf8(2, 2, 72, nf + 4, "FTP Server Configuration");
        for (i = 0; i < nf; i++)
            draw_field(4, 4 + i, &fields[i], i == sel);
        gotoxy(3, 4 + nf + 1);
        setcolor(DIM);
        printf(" %s", fields[sel].help);
        printf("                                       ");
        setcolor(RESET);
        fflush(stdout);
        k = readkey();
        if (k == KEY_ESC || k == 'q') break;
        if (k == KEY_UP && sel > 0) sel--;
        if (k == KEY_DOWN && sel < nf - 1) sel++;
        if (k == KEY_ENTER) {
            switch (fields[sel].type) {
                case FT_STR:
                case FT_PATH: edit_string(28, 4 + sel, 40, (char *)fields[sel].data); break;
                case FT_INT:  edit_int(28, 4 + sel, (int *)fields[sel].data); break;
                case FT_BOOL: edit_bool(28, 4 + sel, (int *)fields[sel].data); break;
            }
        }
        if (k >= 'A' && k <= 'Z') k = tolower(k);
        for (i = 0; i < nf; i++)
            if (tolower(fields[i].hotkey) == k) { sel = i; break; }
    }
}
/* ── Main menu ─────────────────────────────────────────────────── */

int main(int argc, char **argv) {
    const char *cfgpath;
    int sel, k, i, nitems;
    static const char *items[] = {
        "G  General Settings      BBS name, sysop, port, nodes",
        "P  Paths & Directories   PCBoard root, data, FOSSIL, netmodem",
        "W  Web Server            HTTP port, web root, ANSI preview",
        "T  FTP Server            FTP port, file root, anonymous access",
        "F  FidoNet Configuration Address, hub, binkp, packet dirs",
        "D  DOSBox Settings       CPU cycles, video output",
        "",
        "S  Save Configuration    Write pcbis.cfg",
        "I  Initialize            Run pcbis_initv (first-time setup)",
        "Q  Quit                  Exit without saving",
    };
    cfgpath = "pcbis.cfg";
    sel = 0; nitems = 10;
    if (argc > 1 && (!strcmp(argv[1], "-?") || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) {
        printf(
            "pcbis -cfg — PCBoard Installation System Configuration\n"
            "Part of pcbrevival (GPL v3.0)\n"
            "\n"
            "Usage: pcbis_ui [pcbis.cfg]\n"
            "       pcbis -cfg          (via pcbis launcher script)\n"
            "\n"
            "Configuration screens:\n"
            "  G  General Settings     BBS name, sysop name, telnet port, nodes\n"
            "  P  Paths & Directories  PCBoard root, data dir, FOSSIL, netmodem\n"
            "  W  Web Server           HTTP port, web root, ANSI preview, file browser\n"
            "  T  FTP Server           FTP port, file root, anonymous, max connections\n"
            "  F  FidoNet              FTN address, hub, binkp port, in/outbound\n"
            "  D  DOSBox Settings      CPU cycles, video output mode\n"
            "\n"
            "Navigation:\n"
            "  Arrow keys    Move selection\n"
            "  Enter         Edit selected field\n"
            "  Letter key    Jump to menu item by hotkey\n"
            "  Backspace     Delete character while editing\n"
            "  Escape / Q    Back to main menu / quit\n"
            "  S             Save configuration to pcbis.cfg\n"
            "  I             Run first-time setup (pcbis_initv)\n"
            "\n"
            "Config file: pcbis.cfg (key=value format)\n"
            "  Located at: $PCBIS_ROOT/pcbis.cfg or ./pcbis.cfg\n"
            "\n"
            "Related commands:\n"
            "  pcbis_initv     First-time directory setup\n"
            "  pcbis_startup   Start PCBoard + netmodem2irc\n"
            "  pcbis_shutdown  Graceful shutdown\n"
            "  pcbtic          TIC file processor (file echoes)\n"
        );
        return 0;
    }

    if (argc > 1) cfgpath = argv[1];

    config_defaults(&cfg);
    config_load(&cfg, cfgpath);

    term_raw();
    clearscreen();

    while (1) {
        clearscreen();
        draw_box_utf8(2, 1, 72, 3, "PCBoard Installation System");

        gotoxy(4, 2);
        setcolor(BOLD WHITE);
        printf("  General  Paths  Web  FTP  FidoNet  DOSBox        Save & Exit");
        setcolor(RESET);

        draw_box_utf8(2, 5, 72, 14, "Main Menu");


        for (i = 0; i < nitems; i++) {
            gotoxy(6, 7 + i);
            if (items[i][0] == 0) continue;
            if (i == sel) setcolor(REVERSE CYAN);
            else setcolor(WHITE);
            printf(" %-66s ", items[i]);
            setcolor(RESET);
        }

        gotoxy(4, 20);
        setcolor(DIM);
        printf(" Use arrows to navigate, Enter to select, Q to quit");
        setcolor(RESET);
        fflush(stdout);

        k = readkey();
        if (k == KEY_UP) { do { if (sel > 0) sel--; } while (sel < nitems && items[sel][0] == 0); }
        if (k == KEY_DOWN) { do { if (sel < nitems - 1) sel++; } while (sel < nitems && items[sel][0] == 0); }

        if (k == KEY_ENTER || (k >= 'a' && k <= 'z') || (k >= 'A' && k <= 'Z')) {
            char ch = (k == KEY_ENTER) ? items[sel][0] : (char)toupper(k);
            switch (ch) {
                case 'G': clearscreen(); screen_general(); break;
                case 'P': clearscreen(); screen_paths(); break;
                case 'W': clearscreen(); screen_web(); break;
                case 'T': clearscreen(); screen_ftp(); break;
                case 'F': clearscreen(); screen_fidonet(); break;
                case 'D': clearscreen(); screen_dosbox(); break;
                case 'S':
                    config_save(&cfg, cfgpath);
                    gotoxy(4, 20);
                    setcolor(BOLD YELLOW);
                    printf(" Configuration saved to %s", cfgpath);
                    setcolor(RESET);
                    fflush(stdout);
                    readkey();
                    break;
                case 'I':
                    term_restore();
                    clearscreen();
                    system("./pcbis_initv");
                    term_raw();
                    readkey();
                    break;
                case 'Q': case 0:
                    goto done;
            }
        }
        if (k == KEY_ESC || k == 'q' || k == 'Q') goto done;
    }

done:
    term_restore();
    clearscreen();
    printf("pcbis configuration complete.\n");
    return 0;
}
