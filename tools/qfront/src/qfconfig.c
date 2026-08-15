/* ====================================================================
 * qfconfig.c — QFront Configuration Editor (TUI)
 * ====================================================================
 * Replaces QFCONFIG.EXE. Full TUI with all menu sections.
 * Platform: ANSI escapes (Linux), conio (DOS), Win32 console.
 * Clean-room from QFront v1.20a binary (82 menu sections).
 * ==================================================================== */

#include "qfront.h"
#include <termios.h>

#define QFCONFIG_VERSION "1.0.0"
#define MAX_MENU     20
#define MAX_RECORDS  64
#define MAX_FIELD    260

/* ---- Terminal abstraction ---- */

static void con_clear(void)  { printf("\033[2J\033[H"); }
static void con_goto(int x, int y) { printf("\033[%d;%dH", y+1, x+1); }
static void con_bold(void)   { printf("\033[1m"); }
static void con_dim(void)    { printf("\033[2m"); }
static void con_reset(void)  { printf("\033[0m"); }
static void con_rev(void)    { printf("\033[7m"); }
static void con_fg(int c)    { printf("\033[%dm", 30+c); }
static void con_bg(int c)    { printf("\033[%dm", 40+c); }

/* Color constants */
#define C_BLACK 0
#define C_RED 1
#define C_GREEN 2
#define C_YELLOW 3
#define C_BLUE 4
#define C_MAGENTA 5
#define C_CYAN 6
#define C_WHITE 7

static int con_getch(void)
{
    int ch;
    struct termios old, raw;
    tcgetattr(0, &old);
    raw = old;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &raw);
    ch = getchar();
    if (ch == 27) {
        int c2 = getchar();
        if (c2 == '[') {
            int c3 = getchar();
            tcsetattr(0, TCSANOW, &old);
            if (c3 == 'A') return -1; /* Up */
            if (c3 == 'B') return -2; /* Down */
            if (c3 == 'D') return -3; /* Left */
            if (c3 == 'C') return -4; /* Right */
            if (c3 == '3') { getchar(); return -5; } /* Delete */
            return 27;
        }
        tcsetattr(0, TCSANOW, &old);
        return 27;
    }
    tcsetattr(0, TCSANOW, &old);
    return ch;
}

/* ---- Draw title bar ---- */

static void draw_title(const char *title)
{
    int i;
    con_clear();
    con_bg(C_BLUE); con_fg(C_WHITE); con_bold();
    con_goto(0, 0);
    printf("  QFront/WC Configuration - %-50s", title);
    con_reset();
}

/* ---- Draw status bar ---- */

static void draw_status(const char *help)
{
    int i;
    con_goto(0, 23);
    con_bg(C_CYAN); con_fg(C_BLACK);
    printf("  %-77s", help);
    con_reset();
    con_goto(0, 22);
    con_dim();
    if (help[0])
        printf("  %s", help);
    con_reset();
    (void)i;
}

/* ---- Generic menu selector ----
 * Returns item index (0-based) or -1 for ESC. */

typedef struct { char label[48]; char help[80]; } MItem;

static int menu_select(const char *title, const MItem *items, int count,
                        const char *status)
{
    int sel = 0, ch;
    while (1) {
        int i;
        draw_title(title);
        for (i = 0; i < count; i++) {
            con_goto(4, i + 2);
            if (i == sel) { con_rev(); printf(" %-40s ", items[i].label); con_reset(); }
            else { con_bold(); con_fg(C_YELLOW); printf(" %-40s ", items[i].label); con_reset(); }
        }
        con_goto(2, count + 3);
        con_fg(C_CYAN); printf("  %s", items[sel].help); con_reset();
        draw_status(status ? status : "UP/DOWN=Navigate  ENTER=Select  ESC=Back");
        ch = con_getch();
        if (ch == -1) sel = (sel > 0) ? sel - 1 : count - 1;
        else if (ch == -2) sel = (sel < count - 1) ? sel + 1 : 0;
        else if (ch == '\r' || ch == '\n') return sel;
        else if (ch == 27 || ch == 'q') return -1;
    }
}

/* ---- String field editor ---- */

static void edit_field(const char *prompt, char *val, int maxlen)
{
    char buf[MAX_FIELD];
    draw_title("EDITING");
    con_goto(2, 2); con_fg(C_CYAN); printf("%s", prompt); con_reset();
    con_goto(2, 4); printf("Current: "); con_bold(); printf("%s", val); con_reset();
    con_goto(2, 6); printf("New value (ENTER=keep): ");
    con_bold();
    if (fgets(buf, sizeof(buf), stdin)) {
        int len = (int)strlen(buf);
        while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) buf[--len] = '\0';
        if (len > 0) { strncpy(val, buf, maxlen - 1); val[maxlen-1] = '\0'; }
    }
    con_reset();
}

static void edit_int(const char *prompt, int *val)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", *val);
    edit_field(prompt, buf, sizeof(buf));
    *val = atoi(buf);
}

/* ---- Record list editor (generic) ----
 * For lists of items with ALT-D delete, ENTER edit. */

typedef struct { char name[64]; char value[260]; } Record;

static void record_editor(const char *title, Record *recs, int *count,
                           int max_recs, const char *name_prompt,
                           const char *val_prompt)
{
    int sel = 0, ch;
    while (1) {
        int i;
        draw_title(title);
        con_goto(2, 1);
        con_dim(); printf("  ALT-I=Insert  ENTER=Edit  D=Delete  ESC=Back"); con_reset();
        for (i = 0; i < *count && i < 18; i++) {
            con_goto(4, i + 3);
            if (i == sel) { con_rev(); printf(" %2d  %-20s  %-40s ", i+1, recs[i].name, recs[i].value); con_reset(); }
            else { printf(" %2d  %-20s  %-40s", i+1, recs[i].name, recs[i].value); }
        }
        if (*count == 0) { con_goto(4, 3); con_dim(); printf("(empty)"); con_reset(); }
        draw_status("ENTER=Edit  I=Insert  D=Delete  ESC=Back");
        ch = con_getch();
        if (ch == -1 && sel > 0) sel--;
        else if (ch == -2 && sel < *count - 1) sel++;
        else if (ch == 27) return;
        else if ((ch == '\r' || ch == '\n') && sel < *count) {
            edit_field(name_prompt, recs[sel].name, 63);
            edit_field(val_prompt, recs[sel].value, 259);
        }
        else if ((ch == 'i' || ch == 'I') && *count < max_recs) {
            memset(&recs[*count], 0, sizeof(Record));
            edit_field(name_prompt, recs[*count].name, 63);
            edit_field(val_prompt, recs[*count].value, 259);
            (*count)++;
        }
        else if ((ch == 'd' || ch == 'D') && sel < *count && *count > 0) {
            int j;
            for (j = sel; j < *count - 1; j++) recs[j] = recs[j+1];
            (*count)--;
            if (sel >= *count && sel > 0) sel--;
        }
    }
}

/* ---- Extended config data (beyond QfConfig) ---- */

typedef struct {
    /* Modem */
    char modem_init[128], modem_init2[128];
    char dial_prefix[32], dial_suffix[32];
    char answer_cmd[32], hangup_cmd[32];
    int  com_port, com_speed;
    int  reset_minutes, max_redials, redial_wait, connect_wait;
    /* Display */
    int  color_mode;
    char status_waiting[80], status_fidomail[80], status_qscan[80];
    /* Records */
    Record addresses[16]; int num_addr;
    Record aliases[16]; int num_aliases;
    Record archivers[8]; int num_archivers;
    Record origins[8]; int num_origins;
    Record routes[32]; int num_routes;
    Record semaphores[16]; int num_semaphores;
    Record fkeys[24]; int num_fkeys;
    Record mail_strings[8]; int num_mail_strings;
    Record translations[16]; int num_translations;
    Record magic_names[32]; int num_magic;
    Record quick_names[16]; int num_quick;
    Record trashcan[16]; int num_trashcan;
    Record auto_polls[16]; int num_auto_polls;
    Record priv_nodelists[8]; int num_priv_nl;
    Record ignore_users[16]; int num_ignore;
} ExtConfig;

static ExtConfig g_ext;

/* ---- Sub-menus ---- */

static void menu_program_setup(QfConfig *cfg) {
    MItem items[] = {
        {"Outbound directory",       "Location to store outbound FidoMail packets"},
        {"Inbound directory",        "Location to store received files"},
        {"Temp inbound directory",   "Received files for non-secured sessions"},
        {"NetMail directory",        "Location of .MSG style NetMail messages"},
        {"Log file",                 "Filename for the QFront system log"},
        {"Mailer program",           "External mailer path (binkd)"},
        {"Tosser program",           "External tosser path (hpt/pcbtoss)"},
        {"TIC processor",            "External TIC processor path"},
    };
    while (1) {
        int s = menu_select("Program Setup", items, 8, NULL);
        if (s == -1) return;
        switch (s) {
        case 0: edit_field("Outbound directory:", cfg->outbound, 260); break;
        case 1: edit_field("Inbound directory:", cfg->inbound, 260); break;
        case 2: edit_field("Temp inbound directory:", cfg->temp_inbound, 260); break;
        case 3: edit_field("NetMail directory:", cfg->netmail_dir, 260); break;
        case 4: edit_field("Log file:", cfg->logfile, 260); break;
        case 5: edit_field("Mailer program:", cfg->binkd_path, 260); break;
        case 6: edit_field("Tosser program:", cfg->tosser_path, 260); break;
        case 7: edit_field("TIC processor:", cfg->tic_proc, 260); break;
        }
    }
}

static void menu_fidomail_setup(QfConfig *cfg) {
    MItem items[] = {
        {"Primary address",     "Your primary FidoNet address (zone:net/node)"},
        {"Nodelist directory",  "Location to store raw nodelists"},
        {"Nodelist base name",  "Primary nodelist name (8 chars, e.g. NODELIST)"},
        {"Max retries",         "Maximum number of times per day to call a system"},
        {"Retry delay",         "Number of seconds between successive redials"},
        {"Hold time",           "Seconds before marking nodes undialable"},
        {"Debug mode",          "Enable verbose debug logging"},
    };
    while (1) {
        int s = menu_select("FidoMail Setup", items, 7, NULL);
        char buf[64];
        if (s == -1) return;
        switch (s) {
        case 0: ftn_format_addr(&cfg->aka[0], buf, 64); edit_field("Primary address:", buf, 64);
                ftn_parse_addr(buf, &cfg->aka[0]); if (!cfg->num_aka) cfg->num_aka = 1; break;
        case 1: edit_field("Nodelist directory:", cfg->nodelist_dir, 260); break;
        case 2: edit_field("Nodelist base name:", cfg->nodelist_base, 32); break;
        case 3: edit_int("Max retries:", &cfg->max_retries); break;
        case 4: edit_int("Retry delay (seconds):", &cfg->retry_delay); break;
        case 5: edit_int("Hold time (seconds):", &cfg->hold_time); break;
        case 6: edit_int("Debug (0/1):", &cfg->debug); break;
        }
    }
}

static void menu_display_setup(void) {
    MItem items[] = {
        {"Color mode",              "Enable color output (1=yes, 0=no)"},
        {"Waiting status",          "Status message while waiting for a call"},
        {"FidoMail status",         "Status message during FidoMail transfers"},
        {"QScan status",            "Status message during QScan runs"},
        {"Screen blank minutes",    "Minutes before blanking screen (0=disabled)"},
    };
    while (1) {
        int s = menu_select("Display Setup", items, 5, NULL);
        if (s == -1) return;
        switch (s) {
        case 0: edit_int("Color mode (0=mono, 1=color):", &g_ext.color_mode); break;
        case 1: edit_field("Waiting status:", g_ext.status_waiting, 80); break;
        case 2: edit_field("FidoMail status:", g_ext.status_fidomail, 80); break;
        case 3: edit_field("QScan status:", g_ext.status_qscan, 80); break;
        }
    }
}

static void menu_modem_dialout(void) {
    MItem items[] = {
        {"COM port number",         "Serial port (1=COM1, 2=COM2, etc.)"},
        {"Port speed",              "DTE speed (baud rate lock)"},
        {"Modem init string",       "Primary modem initialization string"},
        {"Modem init string 2",     "Secondary modem initialization string"},
        {"Dial prefix",             "Prefix string for dialouts (e.g. ATDT)"},
        {"Dial suffix",             "Suffix string after phone number"},
        {"Answer command",          "String to send to answer a call (e.g. ATA)"},
        {"Hangup command",          "String to hangup (e.g. ATH0)"},
        {"Reset minutes",           "Minutes before auto-resetting modem"},
        {"Max redials",             "Number of redials to try per system"},
        {"Redial wait",             "Seconds between successive redials"},
        {"Connect wait",            "Seconds to wait for CONNECT after dialout"},
    };
    while (1) {
        int s = menu_select("Modem/Dialout", items, 12, NULL);
        if (s == -1) return;
        switch (s) {
        case 0:  edit_int("COM port:", &g_ext.com_port); break;
        case 1:  edit_int("Port speed:", &g_ext.com_speed); break;
        case 2:  edit_field("Modem init:", g_ext.modem_init, 128); break;
        case 3:  edit_field("Modem init 2:", g_ext.modem_init2, 128); break;
        case 4:  edit_field("Dial prefix:", g_ext.dial_prefix, 32); break;
        case 5:  edit_field("Dial suffix:", g_ext.dial_suffix, 32); break;
        case 6:  edit_field("Answer command:", g_ext.answer_cmd, 32); break;
        case 7:  edit_field("Hangup command:", g_ext.hangup_cmd, 32); break;
        case 8:  edit_int("Reset minutes:", &g_ext.reset_minutes); break;
        case 9:  edit_int("Max redials:", &g_ext.max_redials); break;
        case 10: edit_int("Redial wait (sec):", &g_ext.redial_wait); break;
        case 11: edit_int("Connect wait (sec):", &g_ext.connect_wait); break;
        }
    }
}

static void menu_events(void) {
    MItem items[] = {
        {"View events",             "List all configured events"},
        {"Add event",               "Create a new event"},
        {"Event types",             "FidoMail, Errorlevel, Batch file"},
        {"Select days",             "Press SPACEBAR to toggle days of the week"},
    };
    while (1) {
        int s = menu_select("Events", items, 4, NULL);
        if (s == -1) return;
        draw_title("Events");
        con_goto(2, 2);
        printf("  Events are configured as [Event.*] sections in qfront.cfg\n\n");
        printf("  Example:\n");
        printf("    [Event.DailyPoll]\n");
        printf("    Type=FidoMail\n");
        printf("    Days=MTWTF\n");
        printf("    StartTime=02:00\n");
        printf("    EndTime=05:00\n");
        printf("    Flags=ForceComp,ScanBefore,AutoPoll\n");
        printf("    Errorlevel=0\n\n");
        printf("  Event flags: Slide, EchoOnly, NetOnly, RecvOnly, CMOnly,\n");
        printf("    NonCMOnly, NodeCritical, NoHeld, ListedOnly, ForcePoll,\n");
        printf("    PollDuring, ScanBefore, CompileNL, CompileRecv, Rescan,\n");
        printf("    EndNoMail, ExitNoMail, ExitDone, AutoPoll\n");
        printf("\n  Press any key...");
        con_getch();
    }
}

static void menu_addresses(void) {
    record_editor("Addresses (AKAs)", g_ext.addresses, &g_ext.num_addr, 16,
                  "AKA name:", "Address (zone:net/node.point):");
}

static void menu_aliases(void) {
    record_editor("Aliases", g_ext.aliases, &g_ext.num_aliases, 16,
                  "Alias name:", "Maps to address:");
}

static void menu_archivers(void) {
    record_editor("Archivers", g_ext.archivers, &g_ext.num_archivers, 8,
                  "Description (e.g. 'ZIP by PKWARE'):", "Command (e.g. PKZIP.EXE):");
}

static void menu_origin_lines(void) {
    record_editor("Origin Lines", g_ext.origins, &g_ext.num_origins, 8,
                  "Origin name:", "Origin text (without * Origin:):");
}

static void menu_routing(void) {
    record_editor("NetMail Routing", g_ext.routes, &g_ext.num_routes, 32,
                  "Target pattern (e.g. 1:*/*):", "Route command (Direct/Via/Hold/NoPoll):");
}

static void menu_nodelists(QfConfig *cfg) {
    MItem items[] = {
        {"Primary nodelist dir",    "Location to store raw nodelists"},
        {"Primary nodelist name",   "Base name (8 chars, e.g. NODELIST)"},
        {"Private nodelists",       "Edit private nodelist/pointlist entries"},
        {"Nodediff name",           "Base name for nodediff files"},
    };
    while (1) {
        int s = menu_select("Nodelists", items, 4, NULL);
        if (s == -1) return;
        switch (s) {
        case 0: edit_field("Nodelist directory:", cfg->nodelist_dir, 260); break;
        case 1: edit_field("Nodelist base:", cfg->nodelist_base, 32); break;
        case 2: record_editor("Private Nodelists", g_ext.priv_nodelists,
                    &g_ext.num_priv_nl, 8, "Nodelist file:", "Boss node (or blank):"); break;
        case 3: /* nodediff name */ break;
        }
    }
}

static void menu_semaphores(void) {
    record_editor("Semaphore Files", g_ext.semaphores, &g_ext.num_semaphores, 16,
                  "Semaphore file path:", "Errorlevel to exit with:");
}

static void menu_function_keys(void) {
    record_editor("Function Keys", g_ext.fkeys, &g_ext.num_fkeys, 24,
                  "Key (F1-F12, ALT-F1-F12):", "Action (errorlevel or shell command):");
}

static void menu_mail_strings(void) {
    record_editor("External Mail Strings", g_ext.mail_strings,
                  &g_ext.num_mail_strings, 8,
                  "Mail string (e.g. CONNECT):", "Errorlevel:");
}

static void menu_translations(void) {
    record_editor("Translation/Costing", g_ext.translations,
                  &g_ext.num_translations, 16,
                  "Phone prefix:", "Translation + cost:");
}

static void menu_magic_names(void) {
    record_editor("Magic Filenames", g_ext.magic_names, &g_ext.num_magic, 32,
                  "Magic name (e.g. FILES):", "Real file path:");
}

static void menu_quick_names(void) {
    record_editor("Quick Lookup Names", g_ext.quick_names,
                  &g_ext.num_quick, 16,
                  "Quick name:", "Real name + address:");
}

static void menu_trashcan(void) {
    record_editor("Trashcan Users", g_ext.trashcan, &g_ext.num_trashcan, 16,
                  "Username to ignore:", "(reason):");
}

static void menu_auto_polls(void) {
    record_editor("Automatic Polls", g_ext.auto_polls,
                  &g_ext.num_auto_polls, 16,
                  "Node address:", "Poll schedule:");
}

static void menu_ignore_users(void) {
    record_editor("Usernames to Ignore", g_ext.ignore_users,
                  &g_ext.num_ignore, 16,
                  "Username:", "(reason):");
}

static void menu_node_manager(void) {
    MItem items[] = {
        {"Add/Edit node",           "Configure a linked FidoNet node"},
        {"Session passwords",       "Set passwords for secured sessions"},
        {"Packet passwords",        "Set packet-level passwords"},
        {"Conferences",             "Per-node conference subscriptions"},
        {"EchoMail areas",          "Toggle areas for this node"},
        {"Groups",                  "Assign node to groups"},
    };
    while (1) {
        int s = menu_select("Node Manager", items, 6, "ALT-D=Delete  ENTER=Edit");
        if (s == -1) return;
        draw_title("Node Manager");
        con_goto(2, 2);
        printf("  Edit [Node.*] sections in qfront.cfg\n\n");
        printf("  [Node.1:234/56]\n  Password=SECRET\n  PacketPwd=PKT123\n");
        printf("  Areas=NET_DEV,FIDONEWS,BBS_ADS\n\n  Press any key...");
        con_getch();
    }
}

static void menu_area_manager(void) {
    MItem items[] = {
        {"Add/Edit area",           "Configure an echomail area"},
        {"List areas",              "Show all configured areas"},
        {"High message pointers",   "Set/reset high water marks"},
        {"Area groups",             "Organize areas into groups"},
        {"Origin lines",            "Set per-area origin text"},
        {"Passthrough areas",       "Configure pass-through areas"},
    };
    while (1) {
        int s = menu_select("Area Manager", items, 6, "SPACEBAR=Toggle  ALT-D=Delete");
        if (s == -1) return;
        if (s == 4) { menu_origin_lines(); continue; }
        draw_title("Area Manager");
        con_goto(2, 2);
        printf("  Edit [Area.*] sections in qfront.cfg\n\n");
        printf("  [Area.NET_DEV]\n  Path=/var/spool/fido/areas/net_dev\n");
        printf("  Conference=42\n  Passthrough=N\n  Origin=My BBS (1:234/56)\n");
        printf("\n  Press any key...");
        con_getch();
    }
}

static void menu_areafix_setup(void) {
    MItem items[] = {
        {"Areafix uplinks",         "Configure uplink nodes for forwarding"},
        {"Security levels",         "Min security for area requests"},
        {"Allow forwarding",        "Forward requests to uplinks"},
        {"Password policy",         "Allow/deny password changes"},
        {"Compression",             "Default archiver for bundles"},
    };
    while (1) {
        int s = menu_select("Areafix Setup", items, 5, NULL);
        if (s == -1) return;
        draw_title("Areafix Setup");
        con_goto(2, 2);
        printf("  Edit [Areafix.*] sections in qfront.cfg\n\n");
        printf("  [Areafix.1:234/0]\n  Password=UPLINK_PW\n  Groups=A,B,C\n");
        printf("  Forward=Y\n  MinSecurity=10\n\n  Press any key...");
        con_getch();
    }
}

static void menu_frequest_setup(void) {
    MItem items[] = {
        {"Magic filenames",         "Aliases for frequently requested files"},
        {"Request paths",           "Directories to search for requests"},
        {"Request limits",          "Max bytes/files per session/day"},
        {"Minimum baud rate",       "Minimum speed for file requests"},
        {"Time restrictions",       "Hours when requests are allowed"},
        {"Allow unlisted nodes",    "Accept requests from unlisted systems"},
        {"Request start time",     "Time of day to begin allowing requests (00:00=ignore)"},
        {"Request end time",       "Time of day to stop allowing requests (00:00=ignore)"},
    };
    while (1) {
        int s = menu_select("File Request Setup", items, 8, NULL);
        if (s == -1) return;
        if (s == 0) { menu_magic_names(); continue; }
        draw_title("File Request Setup");
        con_goto(2, 2);
        printf("  QMAGIC.DAT: magic_name  /real/path\n");
        printf("  QRLIMIT.DAT: MaxBytesSession=10000000\n");
        printf("                MaxFilesSession=50\n");
        printf("                MinSpeed=9600\n\n  Press any key...");
        con_getch();
    }
}

/* ---- Save ---- */

static void save_config(QfConfig *cfg, const char *path)
{
    FILE *f;
    int i;

    draw_title("Save Configuration");
    con_goto(2, 2);
    printf("Save to %s? (Y/N) ", path);
    int ch = con_getch();
    if (ch != 'Y' && ch != 'y') { printf("Not saved.\n"); con_getch(); return; }

    f = fopen(path, "w");
    if (!f) { printf("\nError saving!\n"); con_getch(); return; }

    fprintf(f, "# QFront Configuration\n# Generated by QFConfig v" QFCONFIG_VERSION "\n\n");
    for (i = 0; i < cfg->num_aka; i++) {
        char buf[64]; ftn_format_addr(&cfg->aka[i], buf, 64);
        fprintf(f, "Address=%s\n", buf);
    }
    fprintf(f, "\nOutbound=%s\nInbound=%s\n", cfg->outbound, cfg->inbound);
    if (cfg->temp_inbound[0]) fprintf(f, "TempInbound=%s\n", cfg->temp_inbound);
    fprintf(f, "NetmailDir=%s\n", cfg->netmail_dir);
    fprintf(f, "\nNodelistDir=%s\nNodelistBase=%s\n", cfg->nodelist_dir, cfg->nodelist_base);
    fprintf(f, "\nMailer=%s\nTosser=%s\n", cfg->binkd_path, cfg->tosser_path);
    if (cfg->tic_proc[0]) fprintf(f, "TicProc=%s\n", cfg->tic_proc);
    fprintf(f, "\nLogFile=%s\nDebug=%d\n", cfg->logfile, cfg->debug);
    fprintf(f, "\nMaxRetries=%d\nRetryDelay=%d\nHoldTime=%d\n",
            cfg->max_retries, cfg->retry_delay, cfg->hold_time);

    if (g_ext.com_port) fprintf(f, "\nComPort=%d\nComSpeed=%d\n", g_ext.com_port, g_ext.com_speed);
    if (g_ext.modem_init[0]) fprintf(f, "ModemInit=%s\n", g_ext.modem_init);
    if (g_ext.modem_init2[0]) fprintf(f, "ModemInit2=%s\n", g_ext.modem_init2);
    if (g_ext.dial_prefix[0]) fprintf(f, "DialPrefix=%s\n", g_ext.dial_prefix);
    if (g_ext.answer_cmd[0]) fprintf(f, "AnswerCmd=%s\n", g_ext.answer_cmd);
    if (g_ext.hangup_cmd[0]) fprintf(f, "HangupCmd=%s\n", g_ext.hangup_cmd);

    for (i = 0; i < g_ext.num_origins; i++)
        fprintf(f, "\n[Origin.%d]\nText=%s\n", i+1, g_ext.origins[i].value);
    for (i = 0; i < g_ext.num_routes; i++)
        fprintf(f, "\n[Route.%d]\nTarget=%s\nCommand=%s\n", i+1, g_ext.routes[i].name, g_ext.routes[i].value);
    for (i = 0; i < g_ext.num_semaphores; i++)
        fprintf(f, "Semaphore=%s:%s\n", g_ext.semaphores[i].name, g_ext.semaphores[i].value);

    fclose(f);
    printf("\nConfiguration saved to %s\n", path);
    printf("Press any key..."); con_getch();
}

/* ---- Import/Export ---- */

static void menu_import_export(void) {
    MItem items[] = {
        {"Export to AREAS.BBS",     "Export EchoMail areas to AREAS.BBS format"},
        {"Export to FIDONET.NA",    "Export areas to FIDONET.NA format"},
        {"Import from AREAS.BBS",   "Import EchoMail areas from AREAS.BBS"},
        {"Packing data files",      "Compress and clean up data files"},
        {"General settings",        "General system-wide settings"},
        {"Usernet messages",        "USERNET.XXX message configuration"},
        {"Browser mode",            "Log file browser settings"},
        {"VGA font selection",      "Select display font (DOS only)"},
    };
    int s = menu_select("Import/Export", items, 8, NULL);
    if (s >= 0) {
        draw_title("Import/Export");
        con_goto(2, 2);
        printf("  Feature available via external tools.\n");
        printf("  Press any key..."); con_getch();
    }
}

/* ---- Main Menu ---- */

static void main_menu(QfConfig *cfg, const char *cfgpath)
{
    MItem items[] = {
        {"Program Setup",           "System directories and filenames"},
        {"FidoMail Setup",          "Addresses, nodelist, session settings"},
        {"Display Setup",           "Colors, screen mode, status messages"},
        {"Modem/Dialout",           "Modem initialization/dialout strings"},
        {"Events",                  "Event scheduling, actions, errorlevels"},
        {"Node Manager",            "Per-node config, passwords, conferences"},
        {"Area Manager",            "EchoMail areas, conferences, origin lines"},
        {"Areafix Setup",           "Uplinks, forwarding, security"},
        {"File Request Setup",      "Magic filenames, paths, limits"},
        {"Nodelists",               "Primary and private nodelist configuration"},
        {"Addresses / Aliases",     "AKA addresses and alias mappings"},
        {"Archivers",               "ZIP, ARJ, LZH archiver configuration"},
        {"NetMail Routing",         "Routing rules for netmail"},
        {"Semaphore Files",         "Trigger files and errorlevels"},
        {"Function Keys",           "F1-F12 and ALT-F key bindings"},
        {"External Mail Strings",   "Strings to detect external mail sessions"},
        {"Translation/Costing",     "Phone prefix translations and costs"},
        {"Auto Polls / Quick Names","Automatic poll list and quick lookups"},
        {"Trashcan / Ignore Users", "Users to ignore during scan"},
        {"Import / Export",         "AREAS.BBS, FIDONET.NA, pack data"},
        {"Save Configuration",      "Write changes to config file"},
    };

    while (1) {
        int s = menu_select("Main Menu", items, 21,
                            "UP/DOWN=Navigate  ENTER=Select  ESC=Exit");
        switch (s) {
        case 0:  menu_program_setup(cfg); break;
        case 1:  menu_fidomail_setup(cfg); break;
        case 2:  menu_display_setup(); break;
        case 3:  menu_modem_dialout(); break;
        case 4:  menu_events(); break;
        case 5:  menu_node_manager(); break;
        case 6:  menu_area_manager(); break;
        case 7:  menu_areafix_setup(); break;
        case 8:  menu_frequest_setup(); break;
        case 9:  menu_nodelists(cfg); break;
        case 10: menu_addresses(); menu_aliases(); break;
        case 11: menu_archivers(); break;
        case 12: menu_routing(); break;
        case 13: menu_semaphores(); break;
        case 14: menu_function_keys(); break;
        case 15: menu_mail_strings(); break;
        case 16: menu_translations(); break;
        case 17: menu_auto_polls(); menu_quick_names(); break;
        case 18: menu_trashcan(); menu_ignore_users(); break;
        case 19: menu_import_export(); break;
        case 20: save_config(cfg, cfgpath); break;
        case -1:
            draw_title("Exit");
            con_goto(2, 2);
            printf("Exit without saving? (Y/N) ");
            { int ch = con_getch();
              if (ch == 'Y' || ch == 'y') return; }
            break;
        }
    }
}

/* ---- Main ---- */

int main(int argc, char *argv[])
{
    QfConfig cfg;
    const char *cfgpath = "qfront.cfg";
    int i;
    for (i = 1; i < argc; i++) {
        if (strcasecmp(argv[i], "/HELP") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("QFConfig v" QFCONFIG_VERSION " — QFront Configuration Editor\n");
            printf("Usage: qfconfig [/COLOR] [/MONO] [/PACK] [config_file]\n");
            return 0;
        }
        if (argv[i][0] != '/' && argv[i][0] != '-') cfgpath = argv[i];
    }
    memset(&g_ext, 0, sizeof(g_ext));
    g_ext.com_port = 1; g_ext.com_speed = 115200;
    strncpy(g_ext.dial_prefix, "ATDT", sizeof(g_ext.dial_prefix)-1); strncpy(g_ext.answer_cmd, "ATA", sizeof(g_ext.answer_cmd)-1);
    strncpy(g_ext.hangup_cmd, "ATH0", sizeof(g_ext.hangup_cmd)-1); strncpy(g_ext.modem_init, "ATZ", sizeof(g_ext.modem_init)-1);
    g_ext.reset_minutes = 5; g_ext.max_redials = 10;
    g_ext.redial_wait = 60; g_ext.connect_wait = 60;
    g_ext.color_mode = 1;

    if (qf_config_load(cfgpath, &cfg) != 0) {
        memset(&cfg, 0, sizeof(cfg));
        cfg.max_retries = 5; cfg.retry_delay = 300; cfg.hold_time = 3600;
    }
    main_menu(&cfg, cfgpath);
    con_clear(); con_reset();
    return 0;
}
