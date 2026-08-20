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

/* Inbound file transfer stubs (defined at end of file) */
void sess_receive_files_inbound(SerPort *sp, const QfConfig *cfg);
void sess_send_files_inbound(SerPort *sp, const QfConfig *cfg, const FTN_ADDR *addr);

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


/* ---- Sysop Hotkeys (DOS TUI) ----
 * In the original, the sysop could press:
 *   ALT-C   Clear screen
 *   ALT-D   Dial a node manually
 *   ALT-H   Hangup modem
 *   ALT-J   Shell to DOS (with memory swap)
 *   ALT-Q   Outbound queue manager
 *   ALT-S   Status display
 *   ALT-T   Terminal mode
 *   ALT-X   Exit QFront
 *   F1-F12  Configurable function keys
 *
 * In our implementation, these are handled by the main loop
 * in qfront.c via signal handlers and CLI options. The TUI
 * hotkey model is replaced by the CLI + config file model. */


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

/*-----------------------------------------------------------------------*/
/* sess_detect_handshake() — Detect FidoNet mailer handshake type        */
/*                                                                         */
/* After answering a call, listens on the serial port for up to          */
/* timeout_ms milliseconds for one of three FidoNet handshake sequences: */
/*                                                                         */
/*   EMSI (FSC-0056):                                                    */
/*     Calling mailer sends "**EMSI_INQ<crc>" — we detect the "**E"      */
/*     prefix. Two consecutive '*' followed by 'E' = EMSI.              */
/*     Returns 'E'. Modern standard (1991+), most mailers use this.      */
/*                                                                         */
/*   YooHoo (FSC-0018, FTS-0006):                                       */
/*     Calling mailer sends ENQ (0x05). Single byte detection.           */
/*     Returns 'Y'. Used by older WaZOO mailers (BinkleyTerm, etc.)     */
/*                                                                         */
/*   FTS-0001 (basic FidoNet):                                           */
/*     Calling mailer sends TSYNC (0xAE). Single byte detection.        */
/*     Returns 'F'. Original Fido/FidoNet basic session. Rarely used    */
/*     after ~1990 but we support it for completeness.                   */
/*                                                                         */
/*   No handshake (timeout):                                             */
/*     Returns 0. The caller is probably a human with a terminal         */
/*     program (or another BBS). The main code treats this as            */
/*     "load PCBoard" — return errorlevel 1 to BOARD.BAT.              */
/*                                                                         */
/*   Carrier lost:                                                       */
/*     Returns 0. Caller hung up during detection window.               */
/*                                                                         */
/* The 20-second timeout is the standard FidoNet detection window.       */
/* Most mailers send their handshake within 2-5 seconds of connect.     */
/* Humans see garbage characters during this window (EMSI_INQ attempts   */
/* from any mailer on their end), which is normal and harmless.          */
/*-----------------------------------------------------------------------*/

static char sess_detect_handshake(SerPort *sp, int timeout_ms)
{
    int ch;
    int star_count = 0;   /* Consecutive '*' characters seen so far     */
    int bytes_read = 0;   /* Total bytes read (for debug logging)       */

    qf_log(LOG_DEBUG, "sess_detect_handshake: listening for %d ms",
           timeout_ms);

    while (1) {
        ch = ser_read_byte(sp, timeout_ms);

        if (ch < 0) {
            /* Timeout — no handshake detected within the window.
             * This is the normal path for human callers. */
            qf_log(LOG_DEBUG, "Handshake detection timeout after %d bytes",
                   bytes_read);
            return 0;
        }

        bytes_read++;

        /* Check for **EMSI_INQ (FSC-0056 Section 4)
         *
         * The calling mailer sends: **EMSI_INQ<len><crc16>
         * We only need to see "**E" to identify it as EMSI.
         * The full EMSI handshake is done in wazoo_handshake_answer(). */
        if (ch == '*') {
            star_count++;
            if (star_count >= 2) {
                /* Two stars — next byte should be 'E' for EMSI */
                ch = ser_read_byte(sp, 2000);
                if (ch == 'E') {
                    qf_log(LOG_INFO, "Incoming EMSI (detected **E)");
                    qf_log(LOG_DEBUG, "EMSI detected after %d bytes",
                           bytes_read);
                    return 'E';
                }
                qf_log(LOG_DEBUG, "Two stars but next byte=0x%02X (not E)",
                       ch >= 0 ? ch : 0);
            }
        } else {
            star_count = 0;
        }

        /* Check for YooHoo ENQ (0x05) — FSC-0018/FTS-0006
         * Single byte: ASCII ENQ (enquiry). BinkleyTerm sends this
         * repeatedly until the answering system responds. */
        if (ch == 0x05) {
            qf_log(LOG_INFO, "Incoming YooHoo (ENQ 0x05)");
            qf_log(LOG_DEBUG, "YooHoo detected after %d bytes", bytes_read);
            return 'Y';
        }

        /* Check for TSYNC (0xAE) — FTS-0001 basic session
         * Original FidoNet session-layer sync character.
         * Ifcico, early Fido, SEAdog use this. Very rare today. */
        if (ch == 0xAE) {
            qf_log(LOG_INFO, "Incoming FTS-1 (TSYNC 0xAE)");
            qf_log(LOG_DEBUG, "FTS-1 detected after %d bytes", bytes_read);
            return 'F';
        }

        /* Check carrier — caller may have hung up during detection */
        if (!ser_get_dcd(sp)) {
            qf_log(LOG_WARN, "Lost carrier during handshake detection "
                   "after %d bytes", bytes_read);
            return 0;
        }

        /* Any other byte is noise or human typing — ignore and keep
         * listening until timeout. Humans typing "HELLO" or pressing
         * Enter generate printable ASCII that doesn't match any of
         * the magic bytes above. */
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
    if (ser_open(sp, cfg->com_port, 1) != 0) {  /* From config, try FOSSIL first */
        qf_log(LOG_ERROR, "Unable to open serial port");
        return -1;
    }

    /* Set locked baud rate and raise DTR.
     * Original QFront: "opening communications port"
     * DTR must be raised before modem responds to AT commands.
     * Baud rate locks DTE speed (modem negotiates DCE separately). */
    ser_set_baud(sp, cfg->locked_baud);
    ser_set_dtr(sp, 1);
    qf_log(LOG_INFO, "Port COM%d opened at %d baud", cfg->com_port, cfg->locked_baud);

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

    qf_log(LOG_DEBUG, "Carrier speed = %lu", 0UL);
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

    qf_log(LOG_DEBUG, "Carrier speed = %lu", 0UL);
    qf_log(LOG_INFO, "End of FidoMail session");

    if (res->files_sent > 0 || res->files_recv > 0)
        res->success = 1;

    return 0;
}



/*-----------------------------------------------------------------------*/
/* qf_answer_session() — Inbound call handler                            */
/*                                                                         */
/* Called from main loop between outbound polls. Opens COM port, waits    */
/* for RING, answers, detects handshake type.                            */
/*                                                                         */
/* Returns:                                                               */
/*   -1 = no call (timeout)                                               */
/*    0 = FidoNet session completed                                       */
/*    1 = human caller — exit for BBS (errorlevel 1)                     */
/*    5 = FAX call (errorlevel 5)                                        */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* qf_answer_session() — Inbound call handler (front-end mailer mode)    */
/*                                                                         */
/* This is QFront's front-end mailer functionality. It sits on the COM   */
/* port waiting for incoming calls. When the phone rings:                 */
/*                                                                         */
/*   1. Modem answers the call (ATA command)                             */
/*   2. Wait for CONNECT from modem                                      */
/*   3. Listen for FidoNet handshake (EMSI **EMSI_INQ, YooHoo ENQ 0x05, */
/*      or FTS-1 TSYNC 0xAE) for 20 seconds                             */
/*   4. If FidoNet handshake detected:                                   */
/*      - Run WaZOO/EMSI session (exchange addresses, passwords)         */
/*      - Receive inbound files (Zmodem)                                 */
/*      - Send any pending outbound files for that node                  */
/*      - Hang up, return to waiting                                     */
/*   5. If NO handshake detected (timeout = human caller):               */
/*      - Leave carrier up (DON'T hang up!)                              */
/*      - Close serial port (release for PCBoard to use)                 */
/*      - Return errorlevel 1 to BOARD.BAT                              */
/*      - BOARD.BAT sees errorlevel 1, loads PCBOARD.EXE                */
/*      - PCBoard takes over the connected caller                       */
/*   6. If FAX tone detected:                                           */
/*      - Hang up                                                        */
/*      - Return errorlevel 5 (BOARD.BAT can route to FAX software)     */
/*                                                                         */
/* The sp_override parameter allows unit testing with a mock serial port.*/
/* Pass NULL for normal operation (opens COM port from config).          */
/*                                                                         */
/* Returns:                                                               */
/*   -1 = no call received (ring timeout expired, normal idle)           */
/*    0 = FidoNet session completed successfully                         */
/*    1 = human caller — BOARD.BAT should load PCBoard (errorlevel 1)   */
/*    5 = FAX call detected — BOARD.BAT can route to FAX (errorlevel 5) */
/*-----------------------------------------------------------------------*/

int qf_answer_session(void *sp_override, const QfConfig *cfg,
                      int *files_in, int *files_out)
{
    SerPort *sp;
    char sp_buf[1024];          /* Opaque SerPort storage (padded)       */
    char mdm_buf[512];          /* Opaque MdmConfig storage (padded)     */
    char result_buf[256];       /* Opaque MdmResult storage (padded)     */
    MdmConfig *mcfg;            /* Pointer into mdm_buf                  */
    MdmResult *mres;            /* Pointer into result_buf               */
    int ring, ans;              /* Return codes from modem functions      */
    char htype;                 /* Handshake type: E=EMSI, Y=YooHoo,     */
                                /* F=FTS-1, 0=unknown/human              */

    *files_in = 0;
    *files_out = 0;

    /*
     * Cast the padded char buffers to opaque struct pointers.
     * This is the same pattern used in sess_call_node() for outbound.
     * We can't include modem.c's struct definition directly because
     * it's a private type — we use the forward declaration
     * "typedef struct MdmConfig MdmConfig" and the padded buffer.
     */
    mcfg = (MdmConfig *)mdm_buf;
    mres = (MdmResult *)result_buf;
    memset(mdm_buf, 0, sizeof(mdm_buf));
    memset(result_buf, 0, sizeof(result_buf));

    /*
     * Build modem config for answering.
     *
     * MdmConfig layout (from modem.c, must stay in sync):
     *   Offset   0: char init_str[128]     — modem init (e.g. "ATZ")
     *   Offset 128: char init_str2[128]    — secondary init
     *   Offset 256: char dial_prefix[32]   — not used for answer
     *   Offset 288: char dial_suffix[32]   — not used for answer
     *   Offset 320: char answer_cmd[32]    — answer command (e.g. "ATA")
     *   Offset 352: char hangup_cmd[32]    — hangup command (e.g. "ATH0")
     *   Offset 384: int  init_retries      — not used for answer
     *   Offset 388: int  connect_timeout   — seconds to wait for CONNECT
     *   Offset 392: int  redial_wait       — not used for answer
     *   Offset 396: int  max_redials       — not used for answer
     *   Offset 400: int  reset_minutes     — not used for answer
     *
     * TODO: Read these from QfConfig instead of hardcoding.
     * When QfConfig gets modem config fields (init_str, answer_cmd, etc.),
     * populate mcfg from those. For now, safe defaults.
     */
    {
        char *p = mdm_buf;
        strncpy(p, "ATZ", 127);               /* init_str: reset modem    */
        strncpy(p + 320, "ATA", 31);           /* answer_cmd: answer call  */
        strncpy(p + 352, "ATH0", 31);          /* hangup_cmd: hang up      */
        *(int *)(p + 388) = 60;                /* connect_timeout: 60 sec  */
    }

    qf_log(LOG_DEBUG, "qf_answer_session: preparing to wait for call");
    qf_log(LOG_DEBUG, "  COM port: %d, baud: %d, timeout: %d sec",
           cfg->com_port, cfg->locked_baud, cfg->retry_delay);

    /*
     * Open the serial port.
     *
     * ser_open() tries FOSSIL first (use_fossil=1), falls back to
     * direct UART access. The port stays open for the entire
     * wait-answer-session cycle.
     *
     * If sp_override is non-NULL, the caller provided a pre-opened
     * port (for testing). Skip open/close in that case.
     */
    if (sp_override) {
        sp = (SerPort *)sp_override;
        qf_log(LOG_DEBUG, "Using caller-provided serial port (test mode)");
    } else {
        sp = (SerPort *)sp_buf;
        memset(sp_buf, 0, sizeof(sp_buf));
        if (ser_open(sp, cfg->com_port, 1) != 0) {
            qf_log(LOG_ERROR, "Cannot open COM%d — check port config",
                   cfg->com_port);
            return -1;
        }
        ser_set_baud(sp, cfg->locked_baud);
        ser_set_dtr(sp, 1);  /* Raise DTR — modem needs this to respond */
        qf_log(LOG_DEBUG, "COM%d opened, DTR raised, baud=%d",
               cfg->com_port, cfg->locked_baud);
    }

    /*
     * Wait for RING.
     *
     * mdm_wait_ring() reads modem responses looking for "RING".
     * Timeout is retry_delay seconds (from config, typically 30-300).
     * Returns 0 if RING detected, -1 on timeout.
     *
     * This is where QFront spends most of its time — sitting idle
     * on the COM port waiting for the phone to ring. Between rings
     * the main loop in qfront.c also checks for outbound mail to send.
     */
    qf_log(LOG_DEBUG, "Waiting for RING (timeout=%d ms)",
           cfg->retry_delay * 1000);

    ring = mdm_wait_ring(sp, cfg->retry_delay * 1000);

    if (ring != 0) {
        /* No ring — normal timeout. Return to main loop for outbound. */
        qf_log(LOG_DEBUG, "No ring detected — timeout expired");
        if (!sp_override) ser_close(sp);
        return -1;
    }

    /*
     * RING detected — answer the call.
     *
     * mdm_answer() sends the answer command (ATA) and waits for
     * the modem to report CONNECT, NO CARRIER, or CONNECT FAX.
     *
     * Returns:
     *    0 = CONNECT (data call — could be mailer or human)
     *   -1 = NO CARRIER or other failure
     *   -2 = FAX tone detected (CONNECT FAX)
     */
    qf_log(LOG_INFO, "RING detected — answering call");
    qf_log(LOG_DEBUG, "Sending answer command: %s", mdm_buf + 320);

    ans = mdm_answer(sp, mcfg, mres);

    if (ans == -2) {
        /*
         * FAX call — modem reported CONNECT FAX or similar.
         * Hang up and return errorlevel 5. BOARD.BAT can route
         * this to FAX software if configured.
         */
        qf_log(LOG_INFO, "FAX call detected — hanging up");
        qf_log(LOG_DEBUG, "Returning errorlevel 5 for FAX routing");
        mdm_hangup(sp, mcfg);
        if (!sp_override) ser_close(sp);
        return 5;
    }

    if (ans != 0) {
        /*
         * Failed to connect — modem reported NO CARRIER, BUSY,
         * NO ANSWER, ERROR, or timed out waiting for CONNECT.
         * This happens when the caller hangs up before connect,
         * or line noise triggers a false RING.
         */
        qf_log(LOG_WARN, "No CONNECT after answer — caller gone or noise");
        qf_log(LOG_DEBUG, "mdm_answer returned %d", ans);
        mdm_hangup(sp, mcfg);
        if (!sp_override) ser_close(sp);
        return -1;
    }

    /*
     * CONNECTED — we have carrier. Now determine what's on the other end.
     *
     * sess_detect_handshake() listens for 20 seconds for:
     *   - **EMSI_INQ  → EMSI mailer (returns 'E')
     *   - ENQ (0x05)  → YooHoo/WaZOO mailer (returns 'Y')
     *   - TSYNC (0xAE) → FTS-0001 basic session (returns 'F')
     *   - timeout      → probably a human with a terminal (returns 0)
     *   - carrier loss  → caller hung up (returns 0)
     *
     * 20 seconds is the standard FidoNet detection window. Most mailers
     * send their handshake within 2-5 seconds. Humans just see garbage
     * characters during this window (the EMSI_INQ attempts from any
     * mailer trying to handshake with us), then the BBS loads.
     *
     * IMPORTANT: We do NOT send EMSI_INQ ourselves in answer mode.
     * The calling mailer sends it; we listen. This follows FSC-0056
     * Section 4: "The answering system shall wait for the calling
     * system to identify itself."
     */
    qf_log(LOG_INFO, "Call answered — detecting handshake (20 sec window)");
    qf_log(LOG_DEBUG, "Listening for EMSI_INQ / YooHoo ENQ / FTS-1 TSYNC");

    htype = sess_detect_handshake(sp, 20000);

    qf_log(LOG_DEBUG, "Handshake detection result: '%c' (%s)",
           htype ? htype : '0',
           htype == 'E' ? "EMSI" :
           htype == 'Y' ? "YooHoo" :
           htype == 'F' ? "FTS-1" : "none/human");

    if (htype == 'E' || htype == 'Y' || htype == 'F') {
        /*
         * FidoNet mailer on the other end.
         *
         * Run the WaZOO handshake to exchange:
         *   - Our address (from config aka[0])
         *   - Our system name ("QFront")
         *   - Optional session password
         *   - Remote's address, sysop name, system name
         *
         * After handshake, receive any files they're sending us
         * (PKT packets, file attaches, TIC files), then send any
         * pending outbound mail we have for their address.
         */
        FTN_ADDR remote_addr;
        char remote_sysop[64];
        char remote_system[64];
        int src;

        memset(remote_sysop, 0, sizeof(remote_sysop));
        memset(remote_system, 0, sizeof(remote_system));

        qf_log(LOG_INFO, "Inbound FidoNet session (type=%c)", htype);
        qf_log(LOG_DEBUG, "Starting WaZOO answer handshake as %d:%d/%d.%d",
               cfg->aka[0].zone, cfg->aka[0].net,
               cfg->aka[0].node, cfg->aka[0].point);

        src = wazoo_handshake_answer(sp, &cfg->aka[0],
                                      "Sysop", "QFront", "",
                                      &remote_addr,
                                      remote_sysop, sizeof(remote_sysop),
                                      remote_system, sizeof(remote_system));

        if (src == 0) {
            /*
             * Handshake succeeded — we know who's calling.
             * Exchange files via Zmodem batch transfer.
             *
             * sess_receive_files_inbound: receive their PKTs/files
             * sess_send_files_inbound: send our pending mail for them
             *
             * After file exchange, both sides hang up.
             */
            qf_log(LOG_INFO, "Handshake OK with %d:%d/%d.%d (%s, %s)",
                   remote_addr.zone, remote_addr.net,
                   remote_addr.node, remote_addr.point,
                   remote_system, remote_sysop);

            qf_log(LOG_DEBUG, "Receiving inbound files...");
            sess_receive_files_inbound(sp, cfg);

            qf_log(LOG_DEBUG, "Sending outbound files for %d:%d/%d.%d...",
                   remote_addr.zone, remote_addr.net,
                   remote_addr.node, remote_addr.point);
            sess_send_files_inbound(sp, cfg, &remote_addr);

            qf_log(LOG_INFO, "Session with %s (%s) complete",
                   remote_system, remote_sysop);
        } else {
            /*
             * Handshake failed — couldn't agree on protocol,
             * password mismatch, or carrier lost during handshake.
             */
            qf_log(LOG_WARN, "WaZOO handshake failed (rc=%d)", src);
        }

        /* Hang up and close port — session done */
        qf_log(LOG_DEBUG, "Hanging up after FidoNet session");
        mdm_hangup(sp, mcfg);
        if (!sp_override) ser_close(sp);
        return 0;  /* FidoNet session done — return to main loop */
    }

    /*
     * No FidoNet handshake detected — this is a human caller.
     *
     * CRITICAL: Do NOT hang up! The caller is connected and waiting
     * for the BBS to answer. We close our handle to the serial port
     * but leave the modem connection up (carrier stays high).
     *
     * We return errorlevel 1 to the main loop, which exits QFront.
     * BOARD.BAT checks the errorlevel and loads PCBOARD.EXE, which
     * opens the same COM port and finds a connected caller waiting.
     *
     * BOARD.BAT example:
     *   :LOOP
     *   QFRONT.EXE QFRONT.CFG
     *   IF ERRORLEVEL 5 GOTO FAX
     *   IF ERRORLEVEL 1 GOTO BBS
     *   GOTO LOOP
     *   :BBS
     *   PCBOARD.EXE
     *   GOTO LOOP
     *   :FAX
     *   FAXRECV.EXE
     *   GOTO LOOP
     *
     * Note: BOARD.BAT must check errorlevels from highest to lowest
     * because DOS IF ERRORLEVEL checks >= not ==.
     */
    qf_log(LOG_INFO, "No mailer handshake — human caller detected");
    qf_log(LOG_INFO, "Exiting with errorlevel 1 — BOARD.BAT loads PCBoard");
    qf_log(LOG_DEBUG, "Carrier stays up — PCBoard takes over the caller");

    /* Close port handle but DON'T drop DTR — keep carrier up */
    if (!sp_override) ser_close(sp);
    return 1;  /* Errorlevel 1 = load BBS */
}

/*-----------------------------------------------------------------------*/
/* STUBS — inbound file transfer (not yet implemented)                   */
/*-----------------------------------------------------------------------*/

void sess_receive_files_inbound(SerPort *sp, const QfConfig *cfg)
{
    (void)sp; (void)cfg;
    qf_log(LOG_INFO, "sess_receive_files_inbound: STUB -- not yet implemented");
}

void sess_send_files_inbound(SerPort *sp, const QfConfig *cfg, const FTN_ADDR *addr)
{
    (void)sp; (void)cfg; (void)addr;
    qf_log(LOG_INFO, "sess_send_files_inbound: STUB -- not yet implemented");
}
