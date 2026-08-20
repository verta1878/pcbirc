/* qfutil.c — QFront Utility Commands
 * Replaces QFUTIL.EXE. CLI for polls, netmail, file requests.
 * Clean-room from QFront v1.20a binary analysis. */

#include "qfront.h"

#define QFUTIL_VERSION "1.0.0"

#pragma pack(push, 1)
typedef struct {
    char     from[36], to[36], subject[72], datetime[20];
    uint16_t times_read, dest_node, orig_node, cost;
    uint16_t orig_net, dest_net, dest_zone, orig_zone;
    uint16_t dest_point, orig_point, reply_to, attr, next_reply;
} MsgHeader;
#pragma pack(pop)

#define MSG_PRIVATE    0x0001
#define MSG_CRASH      0x0002
#define MSG_FILEATTACH 0x0010
#define MSG_INTRANSIT  0x0020
#define MSG_KILLSENT   0x0080
#define MSG_LOCAL      0x0100
#define MSG_HOLD       0x0200
#define MSG_FILEREQ    0x0800

static int msg_next_number(const char *dir) {
    int highest = 0;
#ifndef _WIN32
    DIR *d = opendir(dir);
    struct dirent *ent;
    if (!d) return 1;
    while ((ent = readdir(d)) != NULL) {
        int n = atoi(ent->d_name);
        if (n > highest) highest = n;
    }
    closedir(d);
#endif
    return highest + 1;
}

static int create_netmail(const char *dir, const FTN_ADDR *from,
    const FTN_ADDR *to, const char *from_name, const char *to_name,
    const char *subject, const char *body, uint16_t attr) {
    MsgHeader hdr; char path[260]; FILE *f; int num; time_t now; struct tm *tm;
    memset(&hdr, 0, sizeof(hdr));
    strncpy(hdr.from, from_name, 35); strncpy(hdr.to, to_name, 35);
    strncpy(hdr.subject, subject, 71);
    now = time(NULL); tm = localtime(&now);
    strftime(hdr.datetime, sizeof(hdr.datetime), "%d %b %y  %H:%M:%S", tm);
    hdr.orig_zone = from->zone; hdr.orig_net = from->net;
    hdr.orig_node = from->node; hdr.orig_point = from->point;
    hdr.dest_zone = to->zone; hdr.dest_net = to->net;
    hdr.dest_node = to->node; hdr.dest_point = to->point;
    hdr.attr = attr | MSG_LOCAL;
    num = msg_next_number(dir);
    snprintf(path, sizeof(path), "%s%c%d.MSG", dir, PATH_SEP, num);
    f = fopen(path, "wb"); if (!f) { fprintf(stderr, "Error creating %s\n", path); return -1; }
    fwrite(&hdr, sizeof(hdr), 1, f);
    fprintf(f, "\x01""INTL %u:%u/%u %u:%u/%u\r",
        to->zone, to->net, to->node, from->zone, from->net, from->node);
    if (from->point) fprintf(f, "\x01""FMPT %u\r", from->point);
    if (to->point) fprintf(f, "\x01""TOPT %u\r", to->point);
    if (body && body[0]) { fputs(body, f); if (body[strlen(body)-1] != '\r') fputc('\r', f); }
    fputc('\0', f); fclose(f);
    printf("Created netmail #%d: %s -> %s (%s)\n", num, from_name, to_name, path);
    return 0;
}

static int create_poll(const char *outbound, const FTN_ADDR *addr) {
    char path[260], buf[64]; FILE *f;
    snprintf(path, sizeof(path), "%s%c%04x%04x.ilo", outbound, PATH_SEP, addr->net, addr->node);
    f = fopen(path, "ab"); if (!f) { fprintf(stderr, "Error creating poll %s\n", path); return -1; }
    fclose(f); ftn_format_addr(addr, buf, sizeof(buf));
    printf("Poll created for %s\n", buf); return 0;
}

static int create_freq(const char *outbound, const FTN_ADDR *addr,
    const char *filename, int update, const char *pwd) {
    char path[260], buf[64]; FILE *f;
    snprintf(path, sizeof(path), "%s%c%04x%04x.req", outbound, PATH_SEP, addr->net, addr->node);
    f = fopen(path, "a"); if (!f) return -1;
    if (update) fprintf(f, "%s +\n", filename);
    else if (pwd && pwd[0]) fprintf(f, "%s !%s\n", filename, pwd);
    else fprintf(f, "%s\n", filename);
    fclose(f); ftn_format_addr(addr, buf, sizeof(buf));
    printf("%s request for \"%s\" from %s\n", update ? "Update" : "File", filename, buf);
    create_poll(outbound, addr); return 0;
}

static int create_attach(const char *outbound, const FTN_ADDR *addr,
    const char *filepath, int crash) {
    char path[260]; FILE *f;
    snprintf(path, sizeof(path), "%s%c%04x%04x.%clo", outbound, PATH_SEP,
        addr->net, addr->node, crash ? 'c' : 'f');
    f = fopen(path, "a"); if (!f) return -1;
    fprintf(f, "%s\n", filepath); fclose(f);
    printf("File attach: %s\n", filepath); return 0;
}

static const char *parse_opt(int argc, char *argv[], const char *opt) {
    int i, len = (int)strlen(opt);
    for (i = 1; i < argc; i++)
        if (strncasecmp(argv[i], opt, len) == 0)
            return argv[i][len] == ':' ? argv[i]+len+1 : "";
    return NULL;
}
static int has_opt(int argc, char *argv[], const char *o) { return parse_opt(argc,argv,o) != NULL; }

static void usage(void) {
    /* /COLOR and /MONO parsed but cosmetic-only */
    printf("QFUtil v" QFUTIL_VERSION " — QFront Utility\n\n"
        "  /POLL /ADDR:<addr>                        Create poll\n"
        "  /NETMAIL /ADDR:<addr> /TO:<name> [opts]    Create netmail\n"
        "  /FREQ /ADDR:<addr> /FILE:<name>            File request\n"
        "  /UREQUEST /ADDR:<addr> /FILE:<name>        Update request\n"
        "  /FORWARD /ADDR:<addr> /TO:<name>           Forward netmail\n"
        "  /HELP                                      This help\n\n"
        "Options: /FROM: /SUBJ: /FILE: /FLAGS: /PWRD:\n");
}

int main(int argc, char *argv[]) {
    QfConfig cfg; FTN_ADDR target; uint16_t attr = 0;
    const char *a, *from, *to, *subj, *file, *flags, *pwd;
    if (argc < 2 || has_opt(argc,argv,"/HELP") || has_opt(argc,argv,"-h")) { usage(); return 0; }
    if (qf_config_load("qfront.cfg", &cfg) != 0) { fprintf(stderr, "ERROR: Unable to read configuration file.\n"); return 1; }
    a = parse_opt(argc,argv,"/ADDR");
    if (!a || !a[0]) { fprintf(stderr, "ERROR: No address was specified.\n"); return 1; }
    if (ftn_parse_addr(a, &target) != 0) { fprintf(stderr, "ERROR: Invalid address: %s\n", a); return 1; }
    from = parse_opt(argc,argv,"/FROM"); to = parse_opt(argc,argv,"/TO");
    subj = parse_opt(argc,argv,"/SUBJ"); file = parse_opt(argc,argv,"/FILE");
    flags = parse_opt(argc,argv,"/FLAGS"); pwd = parse_opt(argc,argv,"/PWRD");
    if (flags) {
        if (strstr(flags,"PVT")) attr |= MSG_PRIVATE;
        if (strstr(flags,"CRA")) attr |= MSG_CRASH;
        if (strstr(flags,"K/S")) attr |= MSG_KILLSENT;
        if (strstr(flags,"HLD")) attr |= MSG_HOLD;
    }
    if (has_opt(argc,argv,"/POLL")) return create_poll(cfg.outbound, &target);
    if (has_opt(argc,argv,"/NETMAIL")) {
        if (file && file[0]) {
            attr |= MSG_FILEATTACH;
            create_netmail(cfg.netmail_dir, &cfg.aka[0], &target,
                from?from:"Sysop", to?to:"Sysop", file, "File attached.\r", attr);
            return create_attach(cfg.outbound, &target, file, attr & MSG_CRASH);
        }
        return create_netmail(cfg.netmail_dir, &cfg.aka[0], &target,
            from?from:"Sysop", to?to:"Sysop", subj?subj:"(no subject)", "Automatic message\r", attr);
    }
    if (has_opt(argc,argv,"/FORWARD"))
        return create_netmail(cfg.netmail_dir, &cfg.aka[0], &target,
            from?from:"Sysop", to?to:"Sysop", subj?subj:"Forwarded", "Forwarded netmail.\r", attr|MSG_INTRANSIT);
    if (has_opt(argc,argv,"/FREQ") || has_opt(argc,argv,"/REQUEST")) {
        if (!file||!file[0]) { fprintf(stderr, "ERROR: No filenames were specified.\n"); return 1; }
        return create_freq(cfg.outbound, &target, file, 0, pwd);
    }
    if (has_opt(argc,argv,"/UREQUEST")) {
        if (!file||!file[0]) { fprintf(stderr, "ERROR: No filenames were specified.\n"); return 1; }
        return create_freq(cfg.outbound, &target, file, 1, pwd);
    }
    fprintf(stderr, "ERROR: Nothing to do!\n"); usage(); return 1;
}
