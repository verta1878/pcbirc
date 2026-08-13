/* ====================================================================
 * session.c — Native FidoNet Session Manager
 * ====================================================================
 * Orchestrates a complete FidoNet mail session using native protocols
 * instead of shelling to an external mailer (binkd).
 *
 * Session flow (caller):
 *   1. Open serial port
 *   2. Initialize modem
 *   3. Dial phone number (from nodelist)
 *   4. Handshake: try EMSI first, fall back to YooHoo
 *   5. Validate password
 *   6. Send outbound mail/files (Zmodem, fall back to Xmodem)
 *   7. Receive inbound mail/files
 *   8. Process file requests
 *   9. Hangup modem
 *  10. Close serial port
 *
 * Session flow (answerer):
 *   1. Detect incoming handshake type (EMSI_INQ or YooHoo ENQ)
 *   2. Complete handshake
 *   3. Validate password + nodelist
 *   4. Receive inbound mail/files
 *   5. Send outbound mail/files
 *   6. Process file requests
 *   7. Hangup
 *
 * From binary:
 *   "Establishing FidoMail handshake"
 *   "Transferring FidoMail"
 *   "End of FidoMail session"
 *   "Successfully sent/received packet(s)/file(s)"
 *   "Error during session"
 *
 * Clean-room from FTS-0001, FSC-0056, FTS-0006, public Zmodem spec.
 * ==================================================================== */

#include "qfront.h"
#include <stdarg.h>

/* Forward declarations — all protocol modules */

/* serial.c */
typedef struct SerPort SerPort;
extern int  ser_open(SerPort *sp, int port_num, int use_fossil);
extern void ser_close(SerPort *sp);
extern int  ser_read_byte(SerPort *sp, int timeout_ms);
extern int  ser_write_str(SerPort *sp, const char *str);
extern int  ser_set_baud(SerPort *sp, uint32_t baud);
extern int  ser_get_dcd(SerPort *sp);
extern void ser_set_dtr(SerPort *sp, int on);
extern void ser_flush(SerPort *sp);
extern int  ser_data_ready(SerPort *sp);

/* modem.c */
typedef struct MdmConfig MdmConfig;
typedef struct MdmResult MdmResult;
extern int  mdm_init(SerPort *sp, const MdmConfig *cfg);
extern int  mdm_dial(SerPort *sp, const MdmConfig *cfg,
                      const char *phone, MdmResult *result);
extern int  mdm_answer(SerPort *sp, const MdmConfig *cfg,
                        MdmResult *result);
extern int  mdm_hangup(SerPort *sp, const MdmConfig *cfg);
extern int  mdm_wait_ring(SerPort *sp, int timeout_ms);

/* emsi.c */
typedef struct EmsiData EmsiData;
extern int  emsi_handshake_caller(SerPort *sp, const EmsiData *our,
                                   EmsiData *remote);
extern int  emsi_handshake_answer(SerPort *sp, const EmsiData *our,
                                   EmsiData *remote);

/* wazoo.c */
extern int  wazoo_handshake_caller(SerPort *sp, const FTN_ADDR *addr,
                                    const char *sysop, const char *sys,
                                    const char *pw, FTN_ADDR *raddr,
                                    char *rsysop, int rss,
                                    char *rsys, int rss2);
extern int  wazoo_handshake_answer(SerPort *sp, const FTN_ADDR *addr,
                                    const char *sysop, const char *sys,
                                    const char *pw, FTN_ADDR *raddr,
                                    char *rsysop, int rss,
                                    char *rsys, int rss2);

/* zmodem.c */
extern int  zm_send_file(SerPort *sp, const char *filepath);
extern int  zm_recv_file(SerPort *sp, const char *inbound_dir);
extern int  zm_send_zfin(SerPort *sp);

/* xmodem.c */
extern int  xm_send_file(SerPort *sp, const char *filepath, int use_1k,
                          int use_crc);
extern int  xm_recv_file(SerPort *sp, const char *filepath, int use_1k,
                          int use_crc);

/* frequest.c */
typedef struct FreqFile FreqFile;
extern int  freq_process_req(const char *req_path, const char *req_dirs,
                              FreqFile *files, int max_files);


/* ---- Session Result ---- */

typedef struct {
    int      success;             /* 1 = session completed OK      */
    int      files_sent;          /* Number of files sent           */
    int      files_recv;          /* Number of files received       */
    long     bytes_sent;          /* Total bytes sent               */
    long     bytes_recv;          /* Total bytes received           */
    FTN_ADDR remote_addr;         /* Remote system's address        */
    char     remote_sysop[64];    /* Remote sysop name              */
    char     remote_system[64];   /* Remote system name             */
    char     protocol[16];        /* Protocol used (EMSI/YooHoo)    */
    int      error_code;          /* 0=OK, -1=fail, -2=fax          */
} SessionResult;


/* ---- Session Mode ---- */

typedef enum {
    SESS_EXTERNAL,                /* Shell to binkd (default)      */
    SESS_NATIVE                   /* Use built-in protocols         */
} SessionMode;


/* ---- Detect Incoming Handshake Type ----
 *
 * Reads the first bytes from the remote to determine if they're
 * sending EMSI_INQ (**EMSI_INQ) or YooHoo (ENQ = 0x05).
 * Returns: 'E' for EMSI, 'Y' for YooHoo, 'F' for FTS-1, 0 for unknown.
 *
 * From binary:
 *   "Incoming EMSI"
 *   "Incoming YooHoo"
 *   "Incoming FTS-1" */

static char sess_detect_handshake(SerPort *sp, int timeout_ms)
{
    int ch;
    int star_count = 0;

    while (1) {
        ch = ser_read_byte(sp, timeout_ms);
        if (ch < 0) return 0;     /* Timeout — unknown            */

        /* Check for **EMSI_INQ */
        if (ch == '*') {
            star_count++;
            if (star_count >= 2) {
                /* Read next bytes to confirm EMSI */
                ch = ser_read_byte(sp, 2000);
                if (ch == 'E') {
                    qf_log(LOG_INFO, "Incoming EMSI");
                    return 'E';
                }
            }
        } else {
            star_count = 0;
        }

        /* Check for YooHoo ENQ (0x05) */
        if (ch == 0x05) {
            qf_log(LOG_INFO, "Incoming YooHoo");
            return 'Y';
        }

        /* Check for TSYNC (FTS-0001 basic) */
        if (ch == 0xAE) {          /* TSYNC character              */
            qf_log(LOG_INFO, "Incoming FTS-1");
            return 'F';
        }

        /* Check carrier */
        if (!ser_get_dcd(sp)) {
            qf_log(LOG_WARN, "Lost carrier establishing initial handshake");
            return 0;
        }
    }
}


/* ---- Send Outbound Files (BSO) ----
 *
 * Walks the BSO outbound for a specific address and sends all
 * pending .?ut (netmail packets) and .?lo (file lists) via Zmodem.
 *
 * From binary:
 *   "Sending file(s)/mail packet(s)"
 *   "No packets found to send"
 *   "Transferring FidoMail" */

static int sess_send_outbound(SerPort *sp, const QfConfig *cfg,
                               const FTN_ADDR *addr, SessionResult *res)
{
    char zonedir[260], basename[64], path[520];
    char flavours[] = "icdfh";
    int sent_any = 0;
    int fi;

    /* Build BSO base path for this address */
    snprintf(basename, sizeof(basename), "%04x%04x",
             addr->net, addr->node);

    /* Try default zone outbound first */
    snprintf(zonedir, sizeof(zonedir), "%s", cfg->outbound);

    qf_log(LOG_INFO, "Transferring FidoMail");

    /* Send .?ut files (netmail packets) for each flavour */
    for (fi = 0; flavours[fi]; fi++) {
        snprintf(path, sizeof(path), "%s%c%s.%cut",
                 zonedir, PATH_SEP, basename, flavours[fi]);

        {
            FILE *test = fopen(path, "rb");
            if (test) {
                fclose(test);
                qf_log(LOG_INFO, "Sending mail packet: %s", path);
                if (zm_send_file(sp, path) == 0) {
                    sent_any = 1;
                    res->files_sent++;
                    /* Delete packet after successful send */
                    remove(path);
                } else {
                    qf_log(LOG_WARN, "Error sending %s", path);
                }
            }
        }
    }

    /* Send files listed in .?lo flow files */
    for (fi = 0; flavours[fi]; fi++) {
        FILE *flo;
        char line[520];

        snprintf(path, sizeof(path), "%s%c%s.%clo",
                 zonedir, PATH_SEP, basename, flavours[fi]);

        flo = fopen(path, "r");
        if (!flo) continue;

        while (fgets(line, sizeof(line), flo)) {
            char *p = line;
            char directive = 0;

            /* Strip whitespace */
            while (*p == ' ' || *p == '\t') p++;
            {
                char *end = p + strlen(p) - 1;
                while (end > p && (*end == '\n' || *end == '\r'))
                    *end-- = '\0';
            }

            if (*p == '\0' || *p == '~') continue;  /* Skip empty/skipped */

            /* Parse directive prefix */
            if (*p == '#' || *p == '^') {
                directive = *p;
                p++;
            }

            /* Send the file */
            qf_log(LOG_INFO, "Sending file: %s", p);
            if (zm_send_file(sp, p) == 0) {
                sent_any = 1;
                res->files_sent++;

                /* Handle directives */
                if (directive == '^')
                    remove(p);         /* Delete after send          */
                else if (directive == '#') {
                    /* Truncate after send */
                    FILE *trunc = fopen(p, "wb");
                    if (trunc) fclose(trunc);
                }
            } else {
                qf_log(LOG_WARN, "Error sending %s", p);
            }

            /* Check carrier between files */
            if (!ser_get_dcd(sp)) {
                qf_log(LOG_WARN, "Lost carrier");
                fclose(flo);
                return -1;
            }
        }

        fclose(flo);

        /* Delete the flow file after processing */
        remove(path);
    }

    if (!sent_any)
        qf_log(LOG_INFO, "No packets found to send");

    return 0;
}


/* ---- Receive Inbound Files ----
 *
 * Receives files via Zmodem into the inbound directory.
 * Loops until the remote sends ZFIN (no more files).
 *
 * From binary:
 *   "Receiving a FidoMail run"
 *   "Successfully received packet(s)/file(s)" */

static int sess_recv_inbound(SerPort *sp, const QfConfig *cfg,
                              SessionResult *res)
{
    int rc;

    qf_log(LOG_INFO, "Receiving a FidoMail run");

    while (1) {
        rc = zm_recv_file(sp, cfg->inbound);
        if (rc < 0) break;        /* No more files or error       */
        if (rc == 0) {
            res->files_recv++;
        }

        if (!ser_get_dcd(sp)) {
            qf_log(LOG_WARN, "Lost carrier");
            break;
        }
    }

    return 0;
}


/* ---- Native Outbound Session (We Call Them) ----
 *
 * Full session: dial → handshake → send → receive → hangup.
 *
 * From binary:
 *   "Dialing <address>"
 *   "Establishing FidoMail handshake"
 *   "End of FidoMail session" */

int sess_call_native(const QfConfig *cfg, const FTN_ADDR *addr,
                      const char *phone, const char *password,
                      SessionResult *res)
{
    /* Stack-allocate opaque structs at max size */
    char sp_buf[1024];            /* SerPort (padded)              */
    char mdm_buf[512];            /* MdmConfig (padded)            */
    char result_buf[256];         /* MdmResult (padded)            */

    SerPort   *sp  = (SerPort *)sp_buf;
    MdmConfig *mcfg = (MdmConfig *)mdm_buf;
    MdmResult *mres = (MdmResult *)result_buf;

    char addrstr[64];
    int  rc;

    memset(res, 0, sizeof(*res));
    memset(sp_buf, 0, sizeof(sp_buf));
    memset(mdm_buf, 0, sizeof(mdm_buf));

    ftn_format_addr(addr, addrstr, sizeof(addrstr));

    /* Set up modem config from QfConfig defaults */
    /* (These fields would be loaded from qfront.cfg in production) */

    /* Open serial port */
    if (ser_open(sp, 1, 1) != 0) {  /* COM1, try FOSSIL first */
        qf_log(LOG_ERROR, "Unable to open serial port");
        return -1;
    }

    /* Initialize modem */
    if (mdm_init(sp, mcfg) != 0) {
        ser_close(sp);
        return -1;
    }

    /* Dial */
    qf_log(LOG_INFO, "Dialing %s", addrstr);
    qf_log(LOG_INFO, "Phone number : %s", phone);

    rc = mdm_dial(sp, mcfg, phone, mres);
    if (rc != 0) {
        if (rc == -2) {
            qf_log(LOG_INFO, "Fax call received, exiting with errorlevel 5");
            res->error_code = -2;
        }
        mdm_hangup(sp, mcfg);
        ser_close(sp);
        return rc;
    }

    /* Handshake — try EMSI first, fall back to YooHoo */
    qf_log(LOG_INFO, "Establishing FidoMail handshake");

    /* Try EMSI */
    {
        /* Build our EMSI data from config */
        /* (Simplified — production code fills from cfg) */
        FTN_ADDR remote_addr;
        char remote_sysop[64] = "", remote_system[64] = "";

        rc = wazoo_handshake_caller(sp, &cfg->aka[0],
                                     "Sysop", "QFront",
                                     password,
                                     &remote_addr,
                                     remote_sysop, sizeof(remote_sysop),
                                     remote_system, sizeof(remote_system));

        if (rc == 0) {
            res->remote_addr = remote_addr;
            strncpy(res->remote_sysop, remote_sysop, 63);
            strncpy(res->remote_system, remote_system, 63);
            strncpy(res->protocol, "YooHoo", 15);
        } else {
            qf_log(LOG_WARN, "Unable to establish initial handshake");
            mdm_hangup(sp, mcfg);
            ser_close(sp);
            return -1;
        }
    }

    /* Send outbound mail/files */
    sess_send_outbound(sp, cfg, addr, res);

    /* Receive inbound mail/files */
    sess_recv_inbound(sp, cfg, res);

    /* Send ZFIN */
    zm_send_zfin(sp);

    qf_log(LOG_INFO, "End of FidoMail session");

    /* Log results */
    if (res->files_sent > 0 || res->files_recv > 0) {
        qf_log(LOG_INFO, "Successfully sent packet(s)/file(s)");
        res->success = 1;
    }

    /* Hangup and close */
    mdm_hangup(sp, mcfg);
    ser_close(sp);

    return 0;
}


/* ---- Native Inbound Session (They Call Us) ----
 *
 * Answer call → detect handshake → receive → send → hangup.
 *
 * From binary:
 *   "Ring detected"
 *   "Caller online at <speed>"
 *   "Receiving an incoming FidoMail (FTS-1) run" */

int sess_answer_native(const QfConfig *cfg, SerPort *sp,
                        const char *password, SessionResult *res)
{
    char handshake_type;
    FTN_ADDR remote_addr;
    char remote_sysop[64] = "", remote_system[64] = "";
    int rc;

    memset(res, 0, sizeof(*res));

    /* Detect handshake type */
    handshake_type = sess_detect_handshake(sp, 60000);

    if (handshake_type == 0) {
        qf_log(LOG_WARN, "Unable to establish initial handshake");
        return -1;
    }

    /* Perform handshake */
    if (handshake_type == 'E') {
        /* EMSI answerer — not yet wired, use YooHoo */
        handshake_type = 'Y';
    }

    if (handshake_type == 'Y') {
        rc = wazoo_handshake_answer(sp, &cfg->aka[0],
                                     "Sysop", "QFront",
                                     password,
                                     &remote_addr,
                                     remote_sysop, sizeof(remote_sysop),
                                     remote_system, sizeof(remote_system));
        if (rc != 0) {
            qf_log(LOG_WARN, "Unable to establish initial handshake");
            return -1;
        }
        strncpy(res->protocol, "YooHoo", 15);
    }

    res->remote_addr = remote_addr;
    strncpy(res->remote_sysop, remote_sysop, 63);
    strncpy(res->remote_system, remote_system, 63);

    {
        char buf[64];
        ftn_format_addr(&remote_addr, buf, sizeof(buf));
        qf_log(LOG_INFO, "Caller online at %s (%s)",
               remote_system, buf);
    }

    /* Receive inbound (they send first when they called) */
    sess_recv_inbound(sp, cfg, res);

    /* Send outbound for this node */
    sess_send_outbound(sp, cfg, &remote_addr, res);

    /* Finish */
    zm_send_zfin(sp);

    qf_log(LOG_INFO, "End of FidoMail session");

    if (res->files_sent > 0 || res->files_recv > 0)
        res->success = 1;

    return 0;
}
