/*
 * pcbfcfg.c — PCBoard FidoNet Configuration Utility
 * Part of pcbrevival (GPL v3.0)
 *
 * Standalone FidoNet configurator that reads/writes the same
 * DAT files as PCBSETUP's FidoNet menu (A-L). Provides a
 * focused TUI for FidoNet-only configuration without loading
 * the full PCBSETUP binary.
 *
 * Data files managed:
 *   PCBFIDO.CFG    Master FidoNet config (binary, via FCONFIG.C)
 *   AKAS.DAT       System addresses (FTN zone:net/node.point)
 *   AREAS.DAT      Echomail area → conference mapping
 *   ORIGINS.DAT    Origin line list
 *   NODEARC.DAT    Node archive type configuration
 *   PHONEX.DAT     Phone number translation table
 *   NODELIST.DAT   Nodelist file list
 *   FREQPATH.DAT   FREQ directories
 *   MAGICNAM.DAT   Magic name → file mapping
 *   FREQDENY.DAT   FREQ deny list
 *   FREQ.DAT       FREQ session/daily limits
 *
 * Usage: PCBFCFG [pcboard_root]
 *   -?           Help
 *
 * Mirrors PCBSETUP's FidoNet menu A-L plus queue viewer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <conio.h>
#define CLEAR "cls"
#elif defined(__WATCOMC__) || defined(__DOS__)
#include <conio.h>
#define CLEAR "cls"
#endif

#define MAX_PATH 80
#define MAX_AKAS 30
#define MAX_MAGIC 64
#define MAX_FREQ_PATHS 32
#define MAX_NODES 64
#define MAX_AREAS 256
#define MAX_ORIGINS 32
#define MAX_NODELISTS 16
#define MAX_DENY 32

/* ── FTN Address ── */
typedef struct {
    unsigned short zone;
    unsigned short net;
    unsigned short node;
    unsigned short point;
    char nodestr[25];
    char reserved[10];
} ftn_address;

/* ── Magic Name Entry ── */
typedef struct {
    char magic_name[20];
    char real_name[MAX_PATH];
    char password[10];
    char reserved[10];
} magic_entry;

/* ── FREQ Path Entry ── */
typedef struct {
    char path[MAX_PATH];
    char password[10];
    char reserved[10];
} freq_path;

/* ── FREQ Limits ── */
typedef struct {
    unsigned short session_time;    /* max minutes per session */
    unsigned short daily_time;      /* max minutes per day */
    unsigned long  session_bytes;   /* max KB per session */
    unsigned long  daily_bytes;     /* max KB per day */
    char           allowed;         /* A=All, L=Listed, N=Nodelist, U=Users */
    unsigned short min_baud;        /* minimum baud rate */
} freq_limits;

/* ── Origin Line ── */
typedef struct {
    char origin[80];
    char reserved[10];
} origin_entry;

/* ── Nodelist Entry ── */
typedef struct {
    char basename[80];
    char diffname[9];
    int  last_compile;
    char reserved[10];
} nodelist_entry;

/* ── Global State ── */
static char fido_path[MAX_PATH] = "";

static ftn_address   akas[MAX_AKAS];
static int           num_akas = 0;

static magic_entry   magics[MAX_MAGIC];
static int           num_magics = 0;

static freq_path     freq_paths[MAX_FREQ_PATHS];
static int           num_freq_paths = 0;

static ftn_address   deny_list[MAX_DENY];
static int           num_deny = 0;

static freq_limits   limits;

static origin_entry  origins[MAX_ORIGINS];
static int           num_origins = 0;

static nodelist_entry nodelists[MAX_NODELISTS];
static int           num_nodelists = 0;

/* ── Helpers ── */

static void clear_screen(void) {
    system(CLEAR);
}

static void read_line(const char *prompt, char *buf, int maxlen) {
    printf("  %s", prompt);
    fflush(stdout);
    if (fgets(buf, maxlen, stdin)) {
        char *nl = strchr(buf, '\n');
        if (nl) *nl = '\0';
        nl = strchr(buf, '\r');
        if (nl) *nl = '\0';
    }
}

static void ftn_to_str(ftn_address *a, char *buf) {
    if (a->point)
        sprintf(buf, "%u:%u/%u.%u", a->zone, a->net, a->node, a->point);
    else
        sprintf(buf, "%u:%u/%u", a->zone, a->net, a->node);
}

static int str_to_ftn(const char *s, ftn_address *a) {
    unsigned z=0, ne=0, no=0, p=0;
    if (sscanf(s, "%u:%u/%u.%u", &z, &ne, &no, &p) >= 3) {
        a->zone = z; a->net = ne; a->node = no; a->point = p;
        return 1;
    }
    return 0;
}

/* ── File I/O ── */
static void make_path(char *out, const char *filename) {
    sprintf(out, "%s%s%s", fido_path,
            (fido_path[0] && fido_path[strlen(fido_path)-1] != '/' &&
             fido_path[strlen(fido_path)-1] != '\\') ? "/" : "",
            filename);
}

static int load_binary_file(const char *filename, void *data, int recsize, int maxrecs) {
    char path[MAX_PATH*2];
    FILE *f;
    int count = 0;
    make_path(path, filename);
    f = fopen(path, "rb");
    if (!f) return 0;
    while (count < maxrecs && fread((char*)data + count*recsize, recsize, 1, f) == 1)
        count++;
    fclose(f);
    return count;
}

static int save_binary_file(const char *filename, void *data, int recsize, int count) {
    char path[MAX_PATH*2];
    FILE *f;
    make_path(path, filename);
    f = fopen(path, "wb");
    if (!f) { printf("  Error: cannot write %s\n", path); return 0; }
    fwrite(data, recsize, count, f);
    fclose(f);
    return 1;
}

static void load_all(void) {
    num_akas = load_binary_file("AKAS.DAT", akas, sizeof(ftn_address), MAX_AKAS);
    num_magics = load_binary_file("MAGICNAM.DAT", magics, sizeof(magic_entry), MAX_MAGIC);
    num_freq_paths = load_binary_file("FREQPATH.DAT", freq_paths, sizeof(freq_path), MAX_FREQ_PATHS);
    num_deny = load_binary_file("FREQDENY.DAT", deny_list, sizeof(ftn_address), MAX_DENY);
    num_origins = load_binary_file("ORIGINS.DAT", origins, sizeof(origin_entry), MAX_ORIGINS);
    num_nodelists = load_binary_file("NODELIST.DAT", nodelists, sizeof(nodelist_entry), MAX_NODELISTS);

    if (load_binary_file("FREQ.DAT", &limits, sizeof(freq_limits), 1) == 0)
        memset(&limits, 0, sizeof(limits));
}

/* ── Screen: System Address (C) ── */
static void screen_akas(void) {
    char buf[80], addr[40];
    int i, ch, n;
    for (;;) {
        clear_screen();
        printf("  ╔═══════════════════════════════════════════════╗\n");
        printf("  ║         System Addresses (AKAs)               ║\n");
        printf("  ╠═══════════════════════════════════════════════╣\n");
        for (i = 0; i < num_akas; i++) {
            ftn_to_str(&akas[i], addr);
            printf("  ║  %2d) %-40s ║\n", i+1, addr);
        }
        if (num_akas == 0)
            printf("  ║  (none configured)                            ║\n");
        printf("  ╠═══════════════════════════════════════════════╣\n");
        printf("  ║  [A]dd  [D]elete  [ESC] Back                  ║\n");
        printf("  ╚═══════════════════════════════════════════════╝\n");

        ch = toupper(getch());
        if (ch == 27) break;
        if (ch == 'A' && num_akas < MAX_AKAS) {
            read_line("Address (zone:net/node[.point]): ", buf, sizeof(buf));
            if (str_to_ftn(buf, &akas[num_akas]))
                num_akas++;
        }
        if (ch == 'D' && num_akas > 0) {
            read_line("Delete # (1-N): ", buf, sizeof(buf));
            n = atoi(buf);
            if (n >= 1 && n <= num_akas) {
                memmove(&akas[n-1], &akas[n], (num_akas-n)*sizeof(ftn_address));
                num_akas--;
            }
        }
    }
    save_binary_file("AKAS.DAT", akas, sizeof(ftn_address), num_akas);
}

/* ── Screen: FREQ Path List (I) ── */
static void screen_freq_paths(void) {
    char buf[MAX_PATH];
    int i, ch, n;
    for (;;) {
        clear_screen();
        printf("  ╔═══════════════════════════════════════════════════════════════╗\n");
        printf("  ║         FREQ Path List                                        ║\n");
        printf("  ╠═══════════════════════════════════════════════════════════════╣\n");
        printf("  ║  #   Path                                        Password    ║\n");
        printf("  ║  ──  ──────────────────────────────────────────── ────────── ║\n");
        for (i = 0; i < num_freq_paths; i++) {
            printf("  ║  %2d  %-48.48s %-10.10s ║\n",
                   i+1, freq_paths[i].path, freq_paths[i].password);
        }
        if (num_freq_paths == 0)
            printf("  ║  (none configured)                                            ║\n");
        printf("  ╠═══════════════════════════════════════════════════════════════╣\n");
        printf("  ║  [A]dd  [D]elete  [E]dit  [ESC] Back                         ║\n");
        printf("  ╚═══════════════════════════════════════════════════════════════╝\n");

        ch = toupper(getch());
        if (ch == 27) break;
        if (ch == 'A' && num_freq_paths < MAX_FREQ_PATHS) {
            read_line("Path: ", freq_paths[num_freq_paths].path, MAX_PATH);
            read_line("Password (blank=none): ", freq_paths[num_freq_paths].password, 10);
            if (freq_paths[num_freq_paths].path[0])
                num_freq_paths++;
        }
        if (ch == 'D' && num_freq_paths > 0) {
            read_line("Delete # (1-N): ", buf, sizeof(buf));
            n = atoi(buf);
            if (n >= 1 && n <= num_freq_paths) {
                memmove(&freq_paths[n-1], &freq_paths[n], (num_freq_paths-n)*sizeof(freq_path));
                num_freq_paths--;
            }
        }
        if (ch == 'E' && num_freq_paths > 0) {
            read_line("Edit # (1-N): ", buf, sizeof(buf));
            n = atoi(buf);
            if (n >= 1 && n <= num_freq_paths) {
                printf("  Current: %s\n", freq_paths[n-1].path);
                read_line("New path (blank=keep): ", buf, sizeof(buf));
                if (buf[0]) strncpy(freq_paths[n-1].path, buf, MAX_PATH-1);
                read_line("New password (blank=keep): ", buf, sizeof(buf));
                if (buf[0]) strncpy(freq_paths[n-1].password, buf, 9);
            }
        }
    }
    save_binary_file("FREQPATH.DAT", freq_paths, sizeof(freq_path), num_freq_paths);
}

/* ── Screen: FREQ Magic Names (K) ── */
static void screen_magic_names(void) {
    char buf[MAX_PATH];
    int i, ch, n;
    for (;;) {
        clear_screen();
        printf("  ╔════════════════════════════════════════════════════════════════════╗\n");
        printf("  ║         FREQ Magic Names                                           ║\n");
        printf("  ╠════════════════════════════════════════════════════════════════════╣\n");
        printf("  ║  #   Magic Name          Filename                       Password  ║\n");
        printf("  ║  ──  ──────────────────── ──────────────────────────── ────────── ║\n");
        for (i = 0; i < num_magics; i++) {
            printf("  ║  %2d  %-20.20s %-30.30s %-10.10s ║\n",
                   i+1, magics[i].magic_name, magics[i].real_name, magics[i].password);
        }
        if (num_magics == 0)
            printf("  ║  (none configured)                                                 ║\n");
        printf("  ╠════════════════════════════════════════════════════════════════════╣\n");
        printf("  ║  [A]dd  [D]elete  [E]dit  [ESC] Back                              ║\n");
        printf("  ╚════════════════════════════════════════════════════════════════════╝\n");

        ch = toupper(getch());
        if (ch == 27) break;
        if (ch == 'A' && num_magics < MAX_MAGIC) {
            read_line("Magic name (e.g. NODELIST): ", magics[num_magics].magic_name, 20);
            read_line("Real filename/path: ", magics[num_magics].real_name, MAX_PATH);
            read_line("Password (blank=none): ", magics[num_magics].password, 10);
            if (magics[num_magics].magic_name[0] && magics[num_magics].real_name[0])
                num_magics++;
        }
        if (ch == 'D' && num_magics > 0) {
            read_line("Delete # (1-N): ", buf, sizeof(buf));
            n = atoi(buf);
            if (n >= 1 && n <= num_magics) {
                memmove(&magics[n-1], &magics[n], (num_magics-n)*sizeof(magic_entry));
                num_magics--;
            }
        }
        if (ch == 'E' && num_magics > 0) {
            read_line("Edit # (1-N): ", buf, sizeof(buf));
            n = atoi(buf);
            if (n >= 1 && n <= num_magics) {
                printf("  Current: %s → %s\n", magics[n-1].magic_name, magics[n-1].real_name);
                read_line("New magic name (blank=keep): ", buf, sizeof(buf));
                if (buf[0]) strncpy(magics[n-1].magic_name, buf, 19);
                read_line("New filename (blank=keep): ", buf, sizeof(buf));
                if (buf[0]) strncpy(magics[n-1].real_name, buf, MAX_PATH-1);
                read_line("New password (blank=keep): ", buf, sizeof(buf));
                if (buf[0]) strncpy(magics[n-1].password, buf, 9);
            }
        }
    }
    save_binary_file("MAGICNAM.DAT", magics, sizeof(magic_entry), num_magics);
}

/* ── Screen: FREQ Restrictions (J) ── */
static void screen_freq_limits(void) {
    char buf[40];
    const char *allowed_desc;
    int ch;
    for (;;) {
        clear_screen();
        switch (limits.allowed) {
            case 'L': allowed_desc = "Listed nodes only"; break;
            case 'N': allowed_desc = "Nodelist nodes only"; break;
            case 'U': allowed_desc = "User file nodes only"; break;
            default:  allowed_desc = "All nodes"; limits.allowed = 'A'; break;
        }
        printf("  ╔═══════════════════════════════════════════════╗\n");
        printf("  ║         FREQ Restrictions                     ║\n");
        printf("  ╠═══════════════════════════════════════════════╣\n");
        printf("  ║  1) Session Max Time  : %-5u minutes         ║\n", limits.session_time);
        printf("  ║  2) Session Max KBytes: %-8lu              ║\n", limits.session_bytes);
        printf("  ║  3) Daily Max Time    : %-5u minutes         ║\n", limits.daily_time);
        printf("  ║  4) Daily Max KBytes  : %-8lu              ║\n", limits.daily_bytes);
        printf("  ║  5) Allowed Nodes     : %c (%s)    \n", limits.allowed, allowed_desc);
        printf("  ║  6) Min Allowed Baud  : %-5u                 ║\n", limits.min_baud);
        printf("  ╠═══════════════════════════════════════════════╣\n");
        printf("  ║  Enter field # to edit, [ESC] Back            ║\n");
        printf("  ╚═══════════════════════════════════════════════╝\n");

        ch = getch();
        if (ch == 27) break;
        switch (ch) {
            case '1': read_line("Session Max Time (min): ", buf, sizeof(buf));
                      if (buf[0]) limits.session_time = atoi(buf); break;
            case '2': read_line("Session Max KBytes: ", buf, sizeof(buf));
                      if (buf[0]) limits.session_bytes = atol(buf); break;
            case '3': read_line("Daily Max Time (min): ", buf, sizeof(buf));
                      if (buf[0]) limits.daily_time = atoi(buf); break;
            case '4': read_line("Daily Max KBytes: ", buf, sizeof(buf));
                      if (buf[0]) limits.daily_bytes = atol(buf); break;
            case '5': read_line("Allowed (A=All, L=Listed, N=Nodelist, U=Users): ", buf, sizeof(buf));
                      if (buf[0]) limits.allowed = toupper(buf[0]); break;
            case '6': read_line("Min Baud: ", buf, sizeof(buf));
                      if (buf[0]) limits.min_baud = atoi(buf); break;
        }
    }
    save_binary_file("FREQ.DAT", &limits, sizeof(freq_limits), 1);
}

/* ── Screen: FREQ Deny List ── */
static void screen_freq_deny(void) {
    char buf[80], addr[40];
    int i, ch, n;
    for (;;) {
        clear_screen();
        printf("  ╔═══════════════════════════════════════════════╗\n");
        printf("  ║         FREQ Deny List                        ║\n");
        printf("  ╠═══════════════════════════════════════════════╣\n");
        for (i = 0; i < num_deny; i++) {
            ftn_to_str(&deny_list[i], addr);
            printf("  ║  %2d) %-40s ║\n", i+1, addr);
        }
        if (num_deny == 0)
            printf("  ║  (none — all nodes may FREQ)                  ║\n");
        printf("  ╠═══════════════════════════════════════════════╣\n");
        printf("  ║  [A]dd  [D]elete  [ESC] Back                  ║\n");
        printf("  ╚═══════════════════════════════════════════════╝\n");

        ch = toupper(getch());
        if (ch == 27) break;
        if (ch == 'A' && num_deny < MAX_DENY) {
            read_line("Deny address (zone:net/node): ", buf, sizeof(buf));
            if (str_to_ftn(buf, &deny_list[num_deny]))
                num_deny++;
        }
        if (ch == 'D' && num_deny > 0) {
            read_line("Delete # (1-N): ", buf, sizeof(buf));
            n = atoi(buf);
            if (n >= 1 && n <= num_deny) {
                memmove(&deny_list[n-1], &deny_list[n], (num_deny-n)*sizeof(ftn_address));
                num_deny--;
            }
        }
    }
    save_binary_file("FREQDENY.DAT", deny_list, sizeof(ftn_address), num_deny);
}

/* ── Screen: Origin Lines ── */
static void screen_origins(void) {
    char buf[80];
    int i, ch, n;
    for (;;) {
        clear_screen();
        printf("  ╔═══════════════════════════════════════════════════════════════════╗\n");
        printf("  ║         Origin Lines                                              ║\n");
        printf("  ╠═══════════════════════════════════════════════════════════════════╣\n");
        for (i = 0; i < num_origins; i++) {
            printf("  ║  %2d) %-62.62s ║\n", i+1, origins[i].origin);
        }
        if (num_origins == 0)
            printf("  ║  (none configured)                                                ║\n");
        printf("  ╠═══════════════════════════════════════════════════════════════════╣\n");
        printf("  ║  [A]dd  [D]elete  [E]dit  [ESC] Back                             ║\n");
        printf("  ╚═══════════════════════════════════════════════════════════════════╝\n");

        ch = toupper(getch());
        if (ch == 27) break;
        if (ch == 'A' && num_origins < MAX_ORIGINS) {
            read_line("Origin line: ", origins[num_origins].origin, 80);
            if (origins[num_origins].origin[0]) num_origins++;
        }
        if (ch == 'D' && num_origins > 0) {
            read_line("Delete # (1-N): ", buf, sizeof(buf));
            n = atoi(buf);
            if (n >= 1 && n <= num_origins) {
                memmove(&origins[n-1], &origins[n], (num_origins-n)*sizeof(origin_entry));
                num_origins--;
            }
        }
        if (ch == 'E' && num_origins > 0) {
            read_line("Edit # (1-N): ", buf, sizeof(buf));
            n = atoi(buf);
            if (n >= 1 && n <= num_origins) {
                printf("  Current: %s\n", origins[n-1].origin);
                read_line("New origin: ", buf, sizeof(buf));
                if (buf[0]) strncpy(origins[n-1].origin, buf, 79);
            }
        }
    }
    save_binary_file("ORIGINS.DAT", origins, sizeof(origin_entry), num_origins);
}

/* ── Screen: Nodelist Configuration (H) ── */
static void screen_nodelists(void) {
    char buf[MAX_PATH];
    int i, ch, n;
    for (;;) {
        clear_screen();
        printf("  ╔═══════════════════════════════════════════════════════════════╗\n");
        printf("  ║         Nodelist Configuration                                ║\n");
        printf("  ╠═══════════════════════════════════════════════════════════════╣\n");
        printf("  ║  #   Base Name                                     Diff Name ║\n");
        printf("  ║  ──  ────────────────────────────────────────────── ──────── ║\n");
        for (i = 0; i < num_nodelists; i++) {
            printf("  ║  %2d  %-50.50s %-8.8s ║\n",
                   i+1, nodelists[i].basename, nodelists[i].diffname);
        }
        if (num_nodelists == 0)
            printf("  ║  (none configured)                                            ║\n");
        printf("  ╠═══════════════════════════════════════════════════════════════╣\n");
        printf("  ║  [A]dd  [D]elete  [ESC] Back                                 ║\n");
        printf("  ╚═══════════════════════════════════════════════════════════════╝\n");

        ch = toupper(getch());
        if (ch == 27) break;
        if (ch == 'A' && num_nodelists < MAX_NODELISTS) {
            read_line("Nodelist base (e.g. NODELIST): ", nodelists[num_nodelists].basename, 80);
            read_line("Diff name (e.g. NODEDIFF): ", nodelists[num_nodelists].diffname, 9);
            if (nodelists[num_nodelists].basename[0]) num_nodelists++;
        }
        if (ch == 'D' && num_nodelists > 0) {
            read_line("Delete # (1-N): ", buf, sizeof(buf));
            n = atoi(buf);
            if (n >= 1 && n <= num_nodelists) {
                memmove(&nodelists[n-1], &nodelists[n], (num_nodelists-n)*sizeof(nodelist_entry));
                num_nodelists--;
            }
        }
    }
    save_binary_file("NODELIST.DAT", nodelists, sizeof(nodelist_entry), num_nodelists);
}

/* ── Main Menu ── */
static void main_menu(void) {
    char addr[40];
    int ch;
    for (;;) {
        clear_screen();
        printf("  ╔═══════════════════════════════════════════════╗\n");
        printf("  ║   PCBFCFG — PCBoard FidoNet Configuration     ║\n");
        printf("  ║   PCBoard 15.41 / pcbrevival                  ║\n");
        printf("  ╠═══════════════════════════════════════════════╣\n");
        printf("  ║                                               ║\n");
        if (num_akas > 0) {
            ftn_to_str(&akas[0], addr);
            printf("  ║   Primary Address: %-26s ║\n", addr);
        } else {
            printf("  ║   Primary Address: (not configured)            ║\n");
        }
        printf("  ║   AKAs: %-3d  Areas: (in conferences)           ║\n", num_akas);
        printf("  ║   Magic Names: %-3d  FREQ Paths: %-3d             ║\n", num_magics, num_freq_paths);
        printf("  ║   Origin Lines: %-3d  Nodelists: %-3d             ║\n", num_origins, num_nodelists);
        printf("  ║   Deny List: %-3d                                ║\n", num_deny);
        printf("  ║                                               ║\n");
        printf("  ╠═══════════════════════════════════════════════╣\n");
        printf("  ║   C  System Address (AKAs)                    ║\n");
        printf("  ║   H  Nodelist Configuration                   ║\n");
        printf("  ║   I  FREQ Path List                           ║\n");
        printf("  ║   J  FREQ Restrictions                        ║\n");
        printf("  ║   K  FREQ Magic Names                         ║\n");
        printf("  ║   L  FREQ Deny List                           ║\n");
        printf("  ║   O  Origin Lines                             ║\n");
        printf("  ║                                               ║\n");
        printf("  ║   Q  Quit                                     ║\n");
        printf("  ╚═══════════════════════════════════════════════╝\n");
        printf("  Selection: ");
        fflush(stdout);

        ch = toupper(getch());
        printf("%c\n", ch);

        switch (ch) {
            case 'C': screen_akas(); break;
            case 'H': screen_nodelists(); break;
            case 'I': screen_freq_paths(); break;
            case 'J': screen_freq_limits(); break;
            case 'K': screen_magic_names(); break;
            case 'L': screen_freq_deny(); break;
            case 'O': screen_origins(); break;
            case 'Q': case 27: return;
        }
    }
}

/* ── Entry Point ── */
int main(int argc, char *argv[]) {
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "/?") == 0) {
            printf("PCBFCFG — PCBoard FidoNet Configuration Utility\n");
            printf("Part of PCBoard 15.41 / pcbrevival\n\n");
            printf("Usage: PCBFCFG [fido_data_path]\n\n");
            printf("Reads/writes the same DAT files as PCBSETUP's\n");
            printf("FidoNet menu (items A-L). Provides focused\n");
            printf("FidoNet configuration without loading PCBSETUP.\n\n");
            printf("Data files: AKAS.DAT, MAGICNAM.DAT, FREQPATH.DAT,\n");
            printf("  FREQDENY.DAT, FREQ.DAT, ORIGINS.DAT, NODELIST.DAT\n");
            return 0;
        }
        strncpy(fido_path, argv[i], MAX_PATH-1);
    }

    if (!fido_path[0]) {
        /* Default: look in current directory */
        strcpy(fido_path, ".");
    }

    load_all();
    main_menu();

    printf("\n  PCBFCFG — Configuration saved.\n");
    return 0;
}
