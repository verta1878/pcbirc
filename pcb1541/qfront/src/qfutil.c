/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* qfutil.c -- QFront Utility Commands                                      */
/*                                                                           */
/* Command-line utility for creating polls, netmail messages, file           */
/* requests, update requests, and file attaches from batch files or the      */
/* DOS command line. Replaces QFUTIL.EXE from the original QFront.           */
/*                                                                           */
/* Clean-room from QFront v1.20a binary analysis.                            */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "qfront.h"

#define QFUTIL_VERSION "1.0.0"


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                     FTS-0001 Netmail Message Header                       */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#pragma pack(push, 1)
typedef struct {
    char     FromUser[36];              /* sender name                   */
    char     ToUser[36];                /* recipient name                */
    char     Subject[72];               /* message subject               */
    char     DateTime[20];              /* date/time string              */
    uint16_t TimesRead;                 /* times message was read        */
    uint16_t DestNode;                  /* destination node number       */
    uint16_t OrigNode;                  /* originating node number       */
    uint16_t Cost;                      /* cost in cents                 */
    uint16_t OrigNet;                   /* originating net number        */
    uint16_t DestNet;                   /* destination net number        */
    uint16_t DestZone;                  /* destination zone (FTS-0001)   */
    uint16_t OrigZone;                  /* originating zone (FTS-0001)   */
    uint16_t DestPoint;                 /* destination point (FTS-0001)  */
    uint16_t OrigPoint;                 /* originating point (FTS-0001)  */
    uint16_t ReplyTo;                   /* reply linkage                 */
    uint16_t Attr;                      /* message attribute flags       */
    uint16_t NextReply;                 /* next reply linkage            */
} MsgHeader;
#pragma pack(pop)

/* Message attribute flags (FTS-0001) */
#define MSG_PRIVATE    0x0001           /* private message               */
#define MSG_CRASH      0x0002           /* crash priority                */
#define MSG_FILEATTACH 0x0010           /* file attach                   */
#define MSG_INTRANSIT  0x0020           /* in transit (forwarded)        */
#define MSG_KILLSENT   0x0080           /* kill after sent               */
#define MSG_LOCAL      0x0100           /* locally created               */
#define MSG_HOLD       0x0200           /* hold for pickup               */
#define MSG_FILEREQ    0x0800           /* file request                  */


/*-----------------------------------------------------------------------*/
/* msg_next_number() -- Find the next available .MSG number              */
/*                                                                       */
/* Scans the netmail directory for existing .MSG files and returns       */
/* highest + 1. Each netmail is a separate numbered file (1.MSG, etc.)   */
/*-----------------------------------------------------------------------*/

static int msg_next_number(const char *Dir)
{
    int Highest = 0;                    /* highest .MSG number found     */

#ifndef _WIN32
    DIR           *d;                   /* directory handle               */
    struct dirent *Ent;                 /* directory entry                */

    d = opendir(Dir);
    if (!d) return 1;

    while ((Ent = readdir(d)) != NULL) {
        int n = atoi(Ent->d_name);      /* parse number from filename    */
        if (n > Highest) Highest = n;
    }
    closedir(d);
#endif

    return Highest + 1;
}


/*-----------------------------------------------------------------------*/
/* create_netmail() -- Create a FTS-0001 netmail .MSG file               */
/*                                                                       */
/* Writes a Type 2 message header followed by INTL, FMPT, and TOPT      */
/* kludge lines (FTS-0001), then the message body. The file is stored    */
/* as <number>.MSG in the netmail directory.                              */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

static int create_netmail(const char *Dir, const FTN_ADDR *From,
    const FTN_ADDR *To, const char *FromName, const char *ToName,
    const char *Subject, const char *Body, uint16_t Attr)
{
    MsgHeader  Hdr;                     /* message header                */
    char       Path[260];               /* output file path              */
    FILE      *f;                       /* output file handle            */
    int        Num;                     /* message number                */
    time_t     Now;                     /* current time                  */
    struct tm *Tm;                      /* broken-down time              */

    memset(&Hdr, 0, sizeof(Hdr));
    strncpy(Hdr.FromUser, FromName, 35);
    strncpy(Hdr.ToUser, ToName, 35);
    strncpy(Hdr.Subject, Subject, 71);

    Now = time(NULL);
    Tm  = localtime(&Now);
    strftime(Hdr.DateTime, sizeof(Hdr.DateTime), "%d %b %y  %H:%M:%S", Tm);

    Hdr.OrigZone  = From->zone;
    Hdr.OrigNet   = From->net;
    Hdr.OrigNode  = From->node;
    Hdr.OrigPoint = From->point;
    Hdr.DestZone  = To->zone;
    Hdr.DestNet   = To->net;
    Hdr.DestNode  = To->node;
    Hdr.DestPoint = To->point;
    Hdr.Attr      = Attr | MSG_LOCAL;

    Num = msg_next_number(Dir);
    snprintf(Path, sizeof(Path), "%s%c%d.MSG", Dir, PATH_SEP, Num);

    f = fopen(Path, "wb");
    if (!f) {
        fprintf(stderr, "Error creating %s\n", Path);
        return -1;
    }

    /* Write header */
    fwrite(&Hdr, sizeof(Hdr), 1, f);

    /* Write INTL kludge (FTS-0001 zone routing) */
    fprintf(f, "\x01""INTL %u:%u/%u %u:%u/%u\r",
        To->zone, To->net, To->node,
        From->zone, From->net, From->node);

    /* Write FMPT kludge if originating from a point */
    if (From->point)
        fprintf(f, "\x01""FMPT %u\r", From->point);

    /* Write TOPT kludge if destined for a point */
    if (To->point)
        fprintf(f, "\x01""TOPT %u\r", To->point);

    /* Write message body */
    if (Body && Body[0]) {
        fputs(Body, f);
        if (Body[strlen(Body) - 1] != '\r')
            fputc('\r', f);
    }

    fputc('\0', f);                     /* null terminator (FTS-0001)    */
    fclose(f);

    printf("Created netmail #%d: %s -> %s (%s)\n",
           Num, FromName, ToName, Path);
    return 0;
}


/*-----------------------------------------------------------------------*/
/* create_poll() -- Create a poll for a node                             */
/*                                                                       */
/* Creates a zero-length .ilo (immediate priority) flow file in the      */
/* BSO outbound. QFront will call this node on the next scan.            */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

static int create_poll(const char *Outbound, const FTN_ADDR *Addr)
{
    char  Path[260];                    /* flow file path                */
    char  AddrBuf[64];                  /* formatted address for display */
    FILE *f;                            /* flow file handle              */

    snprintf(Path, sizeof(Path), "%s%c%04x%04x.ilo",
             Outbound, PATH_SEP, Addr->net, Addr->node);

    f = fopen(Path, "ab");             /* create or touch               */
    if (!f) {
        fprintf(stderr, "Error creating poll %s\n", Path);
        return -1;
    }
    fclose(f);

    ftn_format_addr(Addr, AddrBuf, sizeof(AddrBuf));
    printf("Poll created for %s\n", AddrBuf);
    return 0;
}


/*-----------------------------------------------------------------------*/
/* create_freq() -- Create a file request                                */
/*                                                                       */
/* Appends a filename to a .req file in the BSO outbound. Also creates   */
/* a poll so QFront will call the node to make the request.              */
/*                                                                       */
/* FTS-0006 Section 6: .REQ file format:                                 */
/*   FILENAME             -- request this file                           */
/*   FILENAME +            -- update request (only if newer)             */
/*   FILENAME !password    -- password-protected request                 */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

static int create_freq(const char *Outbound, const FTN_ADDR *Addr,
    const char *Filename, int IsUpdate, const char *Password)
{
    char  Path[260];                    /* .req file path                */
    char  AddrBuf[64];                  /* formatted address for display */
    FILE *f;                            /* .req file handle              */

    snprintf(Path, sizeof(Path), "%s%c%04x%04x.req",
             Outbound, PATH_SEP, Addr->net, Addr->node);

    f = fopen(Path, "a");
    if (!f) return -1;

    if (IsUpdate)
        fprintf(f, "%s +\n", Filename);
    else if (Password && Password[0])
        fprintf(f, "%s !%s\n", Filename, Password);
    else
        fprintf(f, "%s\n", Filename);

    fclose(f);

    ftn_format_addr(Addr, AddrBuf, sizeof(AddrBuf));
    printf("%s request for \"%s\" from %s\n",
           IsUpdate ? "Update" : "File", Filename, AddrBuf);

    create_poll(Outbound, Addr);
    return 0;
}


/*-----------------------------------------------------------------------*/
/* create_attach() -- Create a file attach in BSO outbound              */
/*                                                                       */
/* Appends a filepath to a .?lo flow file. Crash priority uses .clo,     */
/* normal uses .flo.                                                     */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

static int create_attach(const char *Outbound, const FTN_ADDR *Addr,
    const char *Filepath, int IsCrash)
{
    char  Path[260];                    /* flow file path                */
    FILE *f;                            /* flow file handle              */

    snprintf(Path, sizeof(Path), "%s%c%04x%04x.%clo",
             Outbound, PATH_SEP, Addr->net, Addr->node,
             IsCrash ? 'c' : 'f');

    f = fopen(Path, "a");
    if (!f) return -1;

    fprintf(f, "%s\n", Filepath);
    fclose(f);

    printf("File attach: %s\n", Filepath);
    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      Command Line Parsing                                 */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* parse_opt() -- Find a command-line option and return its value        */
/*                                                                       */
/* Searches argv for an option matching the given prefix (e.g. "/ADDR"). */
/* If found and followed by ':', returns the value after the colon.      */
/* If found without ':', returns empty string. If not found, returns     */
/* NULL.                                                                 */
/*-----------------------------------------------------------------------*/

static const char *parse_opt(int Argc, char *Argv[], const char *Opt)
{
    int i;                              /* argument index                */
    int Len = (int)strlen(Opt);         /* option prefix length          */

    for (i = 1; i < Argc; i++) {
        if (strncasecmp(Argv[i], Opt, Len) == 0)
            return Argv[i][Len] == ':' ? Argv[i] + Len + 1 : "";
    }
    return NULL;
}


/*-----------------------------------------------------------------------*/
/* has_opt() -- Check if a command-line option is present                */
/*-----------------------------------------------------------------------*/

static int has_opt(int Argc, char *Argv[], const char *Opt)
{
    return parse_opt(Argc, Argv, Opt) != NULL;
}


/*-----------------------------------------------------------------------*/
/* usage() -- Display help text                                          */
/*-----------------------------------------------------------------------*/

static void usage(void)
{
    printf("QFUtil v" QFUTIL_VERSION " -- QFront Utility\n\n"
        "  /POLL /ADDR:<addr>                        Create poll\n"
        "  /NETMAIL /ADDR:<addr> /TO:<n> [opts]    Create netmail\n"
        "  /FREQ /ADDR:<addr> /FILE:<n>            File request\n"
        "  /UREQUEST /ADDR:<addr> /FILE:<n>        Update request\n"
        "  /FORWARD /ADDR:<addr> /TO:<n>           Forward netmail\n"
        "  /HELP                                      This help\n\n"
        "Options: /FROM: /SUBJ: /FILE: /FLAGS: /PWRD:\n");
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                             Main Entry                                    */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

int main(int Argc, char *Argv[])
{
    QfConfig    Cfg;                    /* loaded configuration          */
    FTN_ADDR    Target;                 /* destination address           */
    uint16_t    Attr = 0;              /* message attribute flags       */
    const char *AddrOpt;                /* /ADDR: value                  */
    const char *FromOpt;                /* /FROM: value                  */
    const char *ToOpt;                  /* /TO: value                    */
    const char *SubjOpt;                /* /SUBJ: value                  */
    const char *FileOpt;                /* /FILE: value                  */
    const char *FlagsOpt;               /* /FLAGS: value                 */
    const char *PwdOpt;                 /* /PWRD: value                  */

    if (Argc < 2 || has_opt(Argc, Argv, "/HELP") ||
        has_opt(Argc, Argv, "-h")) {
        usage();
        return 0;
    }

    if (qf_config_load("qfront.cfg", &Cfg) != 0) {
        fprintf(stderr, "ERROR: Unable to read configuration file.\n");
        return 1;
    }

    /* Parse address */
    AddrOpt = parse_opt(Argc, Argv, "/ADDR");
    if (!AddrOpt || !AddrOpt[0]) {
        fprintf(stderr, "ERROR: No address was specified.\n");
        return 1;
    }
    if (ftn_parse_addr(AddrOpt, &Target) != 0) {
        fprintf(stderr, "ERROR: Invalid address: %s\n", AddrOpt);
        return 1;
    }

    /* Parse optional arguments */
    FromOpt  = parse_opt(Argc, Argv, "/FROM");
    ToOpt    = parse_opt(Argc, Argv, "/TO");
    SubjOpt  = parse_opt(Argc, Argv, "/SUBJ");
    FileOpt  = parse_opt(Argc, Argv, "/FILE");
    FlagsOpt = parse_opt(Argc, Argv, "/FLAGS");
    PwdOpt   = parse_opt(Argc, Argv, "/PWRD");

    /* Parse message flags */
    if (FlagsOpt) {
        if (strstr(FlagsOpt, "PVT")) Attr |= MSG_PRIVATE;
        if (strstr(FlagsOpt, "CRA")) Attr |= MSG_CRASH;
        if (strstr(FlagsOpt, "K/S")) Attr |= MSG_KILLSENT;
        if (strstr(FlagsOpt, "HLD")) Attr |= MSG_HOLD;
    }

    /* Dispatch command */
    if (has_opt(Argc, Argv, "/POLL"))
        return create_poll(Cfg.outbound, &Target);

    if (has_opt(Argc, Argv, "/NETMAIL")) {
        if (FileOpt && FileOpt[0]) {
            /* Netmail with file attach */
            Attr |= MSG_FILEATTACH;
            create_netmail(Cfg.netmail_dir, &Cfg.aka[0], &Target,
                FromOpt ? FromOpt : "Sysop",
                ToOpt   ? ToOpt   : "Sysop",
                FileOpt, "File attached.\r", Attr);
            return create_attach(Cfg.outbound, &Target,
                                 FileOpt, Attr & MSG_CRASH);
        }
        return create_netmail(Cfg.netmail_dir, &Cfg.aka[0], &Target,
            FromOpt ? FromOpt : "Sysop",
            ToOpt   ? ToOpt   : "Sysop",
            SubjOpt ? SubjOpt : "(no subject)",
            "Automatic message\r", Attr);
    }

    if (has_opt(Argc, Argv, "/FORWARD"))
        return create_netmail(Cfg.netmail_dir, &Cfg.aka[0], &Target,
            FromOpt ? FromOpt : "Sysop",
            ToOpt   ? ToOpt   : "Sysop",
            SubjOpt ? SubjOpt : "Forwarded",
            "Forwarded netmail.\r", Attr | MSG_INTRANSIT);

    if (has_opt(Argc, Argv, "/FREQ") ||
        has_opt(Argc, Argv, "/REQUEST")) {
        if (!FileOpt || !FileOpt[0]) {
            fprintf(stderr, "ERROR: No filenames were specified.\n");
            return 1;
        }
        return create_freq(Cfg.outbound, &Target, FileOpt, 0, PwdOpt);
    }

    if (has_opt(Argc, Argv, "/UREQUEST")) {
        if (!FileOpt || !FileOpt[0]) {
            fprintf(stderr, "ERROR: No filenames were specified.\n");
            return 1;
        }
        return create_freq(Cfg.outbound, &Target, FileOpt, 1, PwdOpt);
    }

    fprintf(stderr, "ERROR: Nothing to do!\n");
    usage();
    return 1;
}
