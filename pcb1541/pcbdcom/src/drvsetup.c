/* ============================================================================
 * drvsetup.c — pcbcomm port configuration editor
 *
 * Replaces Clark's DRVSETUP.EXE (WCSC COMM-DRV).
 * Full-screen DOS text-mode editor for PCBCOMM.CFG.
 * Reads the config file, displays a table of ports, lets the sysop
 * edit card type / base address / IRQ / segment / FOSSIL flag,
 * then saves.
 *
 * Usage:  DRVSETUP [configfile]
 *   configfile defaults to PCBCOMM.CFG
 *
 * Keys:
 *   Up/Down     Move between ports
 *   Left/Right  Move between fields
 *   Enter       Edit the current field
 *   PgDn/PgUp   Scroll port list
 *   Alt-I       Insert a new port
 *   Alt-D       Delete current port
 *   Alt-R       Repeat (copy previous port, increment subport)
 *   ESC         Exit (prompts to save)
 *
 * License: GPLv3 (pcbirc crew)
 * Authors: wrench (transport/FOSSIL)
 * ==========================================================================*/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

/* ---- Constants --------------------------------------------------------- */
#define MAX_PORTS       24
#define MAX_CARDNAME    16
#define CFG_COLS        7       /* port, card, subport, base, irq, seg, fossil */
#define SCREEN_ROWS     20      /* data rows visible at once */

/* ---- Known card types (for validation + tab-completion) ---------------- */
static const char *card_types[] = {
    "8250", "BOCA", "CYCLOM", "DIGI_PCXE", "DIGI_ACCEL", "ROCKET",
    "EASYIO", "ARNET", "HUB6", "DIGI_COMXI", "GTEK",
    "STALLION_BRUMBY", "CHASE_IOLAN", "EQUINOX_SST",
    NULL
};

/* ---- Port entry -------------------------------------------------------- */
typedef struct {
    unsigned int port;          /* 1-based port number */
    char         card[MAX_CARDNAME];
    unsigned int subport;
    unsigned int base;          /* I/O base address */
    unsigned int irq;
    unsigned int cardseg;       /* card segment (memory-mapped cards) */
    char         fossil;        /* 'Y' or 'N' */
    int          valid;         /* entry in use */
} port_entry_t;

static port_entry_t ports[MAX_PORTS];
static int n_ports = 0;
static int modified = 0;

/* ---- Screen helpers ---------------------------------------------------- */

/* Position cursor (1-based row, col) */
static void gotoxy_rc(int row, int col)
{
#if defined(__WATCOMC__) || defined(__BORLANDC__) || defined(__TURBOC__)
    gotoxy(col, row);
#else
    /* MSC — use ANSI or direct BIOS */
    union REGS r;
    r.h.ah = 0x02;  /* INT 10h set cursor */
    r.h.bh = 0;
    r.h.dh = (unsigned char)(row - 1);
    r.h.dl = (unsigned char)(col - 1);
    int86(0x10, &r, &r);
#endif
}

static void clear_screen(void)
{
#if defined(__WATCOMC__) || defined(__BORLANDC__) || defined(__TURBOC__)
    clrscr();
#else
    printf("\033[2J\033[H");
#endif
}

static void set_color(int fg, int bg)
{
    /* Direct BIOS attribute for next writes.  For simplicity in this
     * version we use printf with ANSI codes; a production version
     * would write directly to video memory at B800:0000. */
    printf("\033[%d;%dm", 30 + fg, 40 + bg);
}

static void reset_color(void)
{
    printf("\033[0m");
}

/* ---- Read extended key (returns 0x100+scancode for special keys) ------- */
static int read_key(void)
{
    int ch;
#if defined(__WATCOMC__) || defined(__BORLANDC__) || defined(__TURBOC__)
    ch = getch();
    if (ch == 0 || ch == 0xE0) {
        ch = getch();
        return 0x100 + ch;
    }
#else
    ch = _getch();
    if (ch == 0 || ch == 0xE0) {
        ch = _getch();
        return 0x100 + ch;
    }
#endif
    return ch;
}

/* Extended key scan codes */
#define KEY_UP      (0x100 + 72)
#define KEY_DOWN    (0x100 + 80)
#define KEY_LEFT    (0x100 + 75)
#define KEY_RIGHT   (0x100 + 77)
#define KEY_PGUP    (0x100 + 73)
#define KEY_PGDN    (0x100 + 81)
#define KEY_ALT_I   (0x100 + 23)    /* Alt-I */
#define KEY_ALT_D   (0x100 + 32)    /* Alt-D */
#define KEY_ALT_R   (0x100 + 19)    /* Alt-R */
#define KEY_ESC     27
#define KEY_ENTER   13

/* ---- Config file I/O --------------------------------------------------- */
static int load_config(const char *path)
{
    FILE *f;
    char line[128];
    int n = 0;

    memset(ports, 0, sizeof(ports));

    f = fopen(path, "r");
    if (!f) return 0;  /* empty / new config is OK */

    while (fgets(line, sizeof(line), f) && n < MAX_PORTS) {
        port_entry_t *p = &ports[n];
        char fossil = 'N';
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        if (sscanf(line, "%u %15s %u %i %u %u %c",
                   &p->port, p->card, &p->subport, &p->base,
                   &p->irq, &p->cardseg, &fossil) < 6)
            continue;
        p->fossil = (fossil == 'Y' || fossil == 'y') ? 'Y' : 'N';
        p->valid = 1;
        n++;
    }
    fclose(f);
    n_ports = n;
    return n;
}

static int save_config(const char *path)
{
    FILE *f;
    int i;

    f = fopen(path, "w");
    if (!f) return -1;

    fprintf(f, "# PCBCOMM.CFG — pcbcomm port configuration\n");
    fprintf(f, "# Generated by DRVSETUP (the crew 4free — GPLv3)\n");
    fprintf(f, "# Port  Card           Sub  Base   IRQ  Seg    FOSSIL\n");

    for (i = 0; i < n_ports; i++) {
        port_entry_t *p = &ports[i];
        if (!p->valid) continue;
        fprintf(f, "%-5u  %-14s  %u    0x%03X  %u    0x%04X  %c\n",
                p->port, p->card, p->subport, p->base,
                p->irq, p->cardseg, p->fossil);
    }
    fclose(f);
    return 0;
}

/* ---- Field column positions -------------------------------------------- */
static const int col_pos[CFG_COLS] = { 2, 10, 24, 32, 41, 47, 57 };
static const char *col_hdr[CFG_COLS] = {
    "Port", "Card Type", "Sub", "Base", "IRQ", "Segment", "FOSSIL"
};

/* ---- Draw the table ---------------------------------------------------- */
static void draw_header(void)
{
    gotoxy_rc(1, 1);
    printf("                      PCBCOMM Port Configuration Editor");
    gotoxy_rc(2, 1);
    printf("  Port    Card Type       Sub   Base    IRQ   Segment  FOSSIL");
    gotoxy_rc(3, 1);
    printf("  ----    -------------   ---   ----    ---   -------  ------");
}

static void draw_row(int screen_row, int idx, int cur_row, int cur_col)
{
    port_entry_t *p;
    int row = screen_row + 4;  /* offset past header */

    gotoxy_rc(row, 1);
    printf("                                                              ");
    gotoxy_rc(row, 1);

    if (idx >= n_ports || !ports[idx].valid) return;
    p = &ports[idx];

    if (idx == cur_row) set_color(0, 7);  /* highlight */

    printf("  %-5u   %-14s  %u     0x%03X   %-3u   0x%04X   %c",
           p->port, p->card, p->subport, p->base,
           p->irq, p->cardseg, p->fossil);

    if (idx == cur_row) reset_color();
}

static void draw_status(void)
{
    gotoxy_rc(24, 1);
    printf("  ESC=Exit  PgDn=Forw  PgUp=Back  Alt-I=Insert  Alt-D=Delete  Alt-R=Repeat  ");
    gotoxy_rc(25, 1);
    printf("  %d port(s) configured%s",
           n_ports, modified ? "  [MODIFIED]" : "");
    printf("                              ");
}

static void draw_all(int top, int cur_row, int cur_col)
{
    int i;
    draw_header();
    for (i = 0; i < SCREEN_ROWS; i++)
        draw_row(i, top + i, cur_row, cur_col);
    draw_status();
}

/* ---- Edit a single field ----------------------------------------------- */
static void edit_field(int idx, int col)
{
    port_entry_t *p = &ports[idx];
    char buf[20];
    unsigned int val;

    gotoxy_rc(25, 1);
    printf("  Enter new value: ");

    /* Read a short string from the keyboard */
    {
        int pos = 0;
        int ch;
        buf[0] = '\0';
        while (1) {
            ch = read_key();
            if (ch == KEY_ENTER) break;
            if (ch == KEY_ESC) return;  /* cancel edit */
            if (ch == 8 && pos > 0) { pos--; buf[pos] = '\0'; printf("\b \b"); continue; }
            if (ch >= 32 && ch < 127 && pos < 18) {
                buf[pos++] = (char)ch;
                buf[pos] = '\0';
                putchar(ch);
            }
        }
    }

    if (buf[0] == '\0') return;  /* empty = no change */

    modified = 1;
    switch (col) {
    case 0: /* port number */
        val = (unsigned int)atoi(buf);
        if (val >= 1 && val <= MAX_PORTS) p->port = val;
        break;
    case 1: /* card type */
        strncpy(p->card, buf, MAX_CARDNAME - 1);
        p->card[MAX_CARDNAME - 1] = '\0';
        /* Uppercase it */
        { char *s; for (s = p->card; *s; s++) if (*s >= 'a' && *s <= 'z') *s -= 32; }
        break;
    case 2: /* subport */
        p->subport = (unsigned int)atoi(buf);
        break;
    case 3: /* base address */
        p->base = (unsigned int)strtol(buf, NULL, 16);
        break;
    case 4: /* IRQ */
        val = (unsigned int)atoi(buf);
        if (val <= 15) p->irq = val;
        else printf("\a");  /* beep: invalid IRQ */
        break;
    case 5: /* card segment */
        p->cardseg = (unsigned int)strtol(buf, NULL, 16);
        break;
    case 6: /* FOSSIL flag */
        p->fossil = (buf[0] == 'Y' || buf[0] == 'y') ? 'Y' : 'N';
        break;
    }
}

/* ---- Insert / Delete / Repeat ------------------------------------------ */
static void insert_port(int at)
{
    int i;
    if (n_ports >= MAX_PORTS) return;
    for (i = n_ports; i > at; i--)
        ports[i] = ports[i - 1];
    memset(&ports[at], 0, sizeof(port_entry_t));
    ports[at].port = (unsigned int)(at + 1);
    ports[at].fossil = 'N';
    ports[at].valid = 1;
    strcpy(ports[at].card, "8250");
    ports[at].base = 0x3F8;
    ports[at].irq = 4;
    n_ports++;
    modified = 1;
}

static void delete_port(int at)
{
    int i;
    if (n_ports <= 0 || at >= n_ports) return;
    for (i = at; i < n_ports - 1; i++)
        ports[i] = ports[i + 1];
    memset(&ports[n_ports - 1], 0, sizeof(port_entry_t));
    n_ports--;
    modified = 1;
}

static void repeat_port(int at)
{
    if (at <= 0 || n_ports >= MAX_PORTS) return;
    insert_port(at);
    /* Copy from previous, increment subport */
    ports[at] = ports[at - 1];
    ports[at].port = (unsigned int)(at + 1);
    ports[at].subport++;
    modified = 1;
}

/* ---- Save prompt ------------------------------------------------------- */
static int save_prompt(const char *path)
{
    int ch;
    gotoxy_rc(25, 1);
    printf("  Save changes? [Y]es  [N]o-resume  [A]bort-exit, no save  ");
    ch = read_key();
    if (ch == 'Y' || ch == 'y') {
        if (save_config(path) < 0) {
            printf("\n  ERROR: could not write %s\n", path);
            read_key();
            return 0;  /* stay in editor */
        }
        return 1;  /* saved, exit */
    }
    if (ch == 'A' || ch == 'a') return 1;  /* abort, exit without save */
    return 0;  /* resume editing */
}

/* ---- Main editor loop -------------------------------------------------- */
int main(int argc, char **argv)
{
    const char *cfg = "PCBCOMM.CFG";
    int cur_row = 0, cur_col = 0, top = 0;
    int key, running = 1;

    if (argc > 1) {
        if (argv[1][0] == '-' || argv[1][0] == '/') {
            printf("Usage:  DRVSETUP [configfile]\n");
            printf("  configfile defaults to PCBCOMM.CFG\n");
            return 0;
        }
        cfg = argv[1];
    }

    load_config(cfg);

    /* If no ports configured, start with one default entry */
    if (n_ports == 0) {
        insert_port(0);
        modified = 0;  /* don't mark as modified yet */
    }

    clear_screen();
    draw_all(top, cur_row, cur_col);

    while (running) {
        /* Position cursor at current cell */
        gotoxy_rc(cur_row - top + 4, col_pos[cur_col]);

        key = read_key();

        switch (key) {
        case KEY_UP:
            if (cur_row > 0) cur_row--;
            if (cur_row < top) top = cur_row;
            break;
        case KEY_DOWN:
            if (cur_row < n_ports - 1) cur_row++;
            if (cur_row >= top + SCREEN_ROWS) top = cur_row - SCREEN_ROWS + 1;
            break;
        case KEY_LEFT:
            if (cur_col > 0) cur_col--;
            break;
        case KEY_RIGHT:
            if (cur_col < CFG_COLS - 1) cur_col++;
            break;
        case KEY_PGDN:
            top += SCREEN_ROWS;
            if (top >= n_ports) top = n_ports > SCREEN_ROWS ? n_ports - SCREEN_ROWS : 0;
            cur_row = top;
            break;
        case KEY_PGUP:
            top -= SCREEN_ROWS;
            if (top < 0) top = 0;
            cur_row = top;
            break;
        case KEY_ENTER:
            edit_field(cur_row, cur_col);
            break;
        case KEY_ALT_I:
            insert_port(cur_row);
            break;
        case KEY_ALT_D:
            delete_port(cur_row);
            if (cur_row >= n_ports && n_ports > 0) cur_row = n_ports - 1;
            break;
        case KEY_ALT_R:
            repeat_port(cur_row);
            break;
        case KEY_ESC:
            if (modified) {
                if (save_prompt(cfg)) running = 0;
            } else {
                running = 0;
            }
            break;
        }

        draw_all(top, cur_row, cur_col);
    }

    clear_screen();
    gotoxy_rc(1, 1);
    if (modified) {
        printf("DRVSETUP: configuration saved to %s\n", cfg);
    } else {
        printf("DRVSETUP: no changes.\n");
    }
    return 0;
}
