/* ====================================================================
 * modem.c — Modem Control (AT Commands)
 * ====================================================================
 * Modem initialization, dialing, answering, and hangup.
 * Parses modem response strings from binary analysis:
 *   CONNECT, BUSY, NO CARRIER, NO DIALTONE, NO ANSWER,
 *   RING, RINGING, VOICE, ERROR, CONNECT FAX
 *
 * Clean-room from QFront binary analysis + standard AT command set.
 * ==================================================================== */

#include "qfront.h"

/* Forward declaration — serial.c */
typedef struct SerPort SerPort;
extern int  ser_read_byte(SerPort *sp, int timeout_ms);
extern int  ser_write(SerPort *sp, const void *buf, int len);
extern int  ser_write_str(SerPort *sp, const char *str);
extern void ser_set_dtr(SerPort *sp, int on);
extern void ser_flush(SerPort *sp);
extern int  ser_get_dcd(SerPort *sp);
extern int  ser_data_ready(SerPort *sp);

/* ---- Modem Response Codes ---- */

typedef enum {
    MDM_NONE = 0,
    MDM_OK,                       /* OK                           */
    MDM_CONNECT,                  /* CONNECT [speed]              */
    MDM_RING,                     /* RING                         */
    MDM_NO_CARRIER,               /* NO CARRIER                   */
    MDM_ERROR,                    /* ERROR                        */
    MDM_NO_DIALTONE,              /* NO DIALTONE                  */
    MDM_BUSY,                     /* BUSY                         */
    MDM_NO_ANSWER,                /* NO ANSWER                    */
    MDM_RINGING,                  /* RINGING                      */
    MDM_VOICE,                    /* VOICE                        */
    MDM_CONNECT_FAX,              /* CONNECT FAX                  */
    MDM_TIMEOUT                   /* No response within timeout   */
} MdmResponse;

typedef struct {
    MdmResponse code;
    uint32_t    connect_speed;    /* Parsed from CONNECT <speed>  */
    uint32_t    carrier_speed;    /* Carrier vs DTE speed         */
    char        raw[128];         /* Raw response string          */
} MdmResult;

/* ---- Modem Configuration ---- */

typedef struct {
    char     init_str[128];       /* Primary init (e.g. ATS0=0M0DT) */
    char     init_str2[128];      /* Secondary init                  */
    char     dial_prefix[32];     /* Dial command (e.g. ATDT)        */
    char     dial_suffix[32];     /* After phone number              */
    char     answer_cmd[32];      /* Answer command (e.g. ATA)       */
    char     hangup_cmd[32];      /* Hangup command (e.g. ATH0)      */
    int      init_retries;        /* Max init attempts               */
    int      connect_timeout;     /* Seconds to wait for CONNECT     */
    int      redial_wait;         /* Seconds between redials         */
    int      max_redials;         /* Max dial attempts               */
    int      reset_minutes;       /* Auto-reset interval             */
} MdmConfig;


/* ---- Read a Line from Modem ---- */

static MdmResponse mdm_read_line(SerPort *sp, char *buf, int bufsize,
                                  int timeout_ms)
{
    int pos = 0;
    int ch;

    buf[0] = '\0';

    while (pos < bufsize - 1) {
        ch = ser_read_byte(sp, timeout_ms);
        if (ch < 0)
            return MDM_TIMEOUT;

        if (ch == '\r' || ch == '\n') {
            if (pos > 0) {
                buf[pos] = '\0';
                break;            /* Got a complete line          */
            }
            continue;             /* Skip leading CR/LF          */
        }

        buf[pos++] = (char)ch;
    }

    buf[pos] = '\0';
    return MDM_NONE;              /* Caller parses the string     */
}


/* ---- Parse Modem Response String ---- */

static void mdm_parse_response(const char *line, MdmResult *result)
{
    memset(result, 0, sizeof(*result));
    strncpy(result->raw, line, sizeof(result->raw) - 1);

    if (strcmp(line, "OK") == 0) {
        result->code = MDM_OK;
    }
    else if (strncmp(line, "CONNECT FAX", 11) == 0) {
        result->code = MDM_CONNECT_FAX;
    }
    else if (strncmp(line, "CONNECT", 7) == 0) {
        result->code = MDM_CONNECT;
        /* Parse speed: "CONNECT 14400" or "CONNECT 14400/ARQ" */
        if (line[7] == ' ')
            result->connect_speed = (uint32_t)atol(line + 8);
        else
            result->connect_speed = 300;  /* Plain "CONNECT" = 300 */
    }
    else if (strcmp(line, "RING") == 0) {
        result->code = MDM_RING;
    }
    else if (strcmp(line, "NO CARRIER") == 0) {
        result->code = MDM_NO_CARRIER;
    }
    else if (strcmp(line, "ERROR") == 0) {
        result->code = MDM_ERROR;
    }
    else if (strcmp(line, "NO DIALTONE") == 0 ||
             strcmp(line, "NO DIAL TONE") == 0) {
        result->code = MDM_NO_DIALTONE;
    }
    else if (strcmp(line, "BUSY") == 0) {
        result->code = MDM_BUSY;
    }
    else if (strcmp(line, "NO ANSWER") == 0) {
        result->code = MDM_NO_ANSWER;
    }
    else if (strcmp(line, "RINGING") == 0) {
        result->code = MDM_RINGING;
    }
    else if (strcmp(line, "VOICE") == 0) {
        result->code = MDM_VOICE;
    }
}


/* ---- Send AT Command and Wait for Response ---- */

static MdmResponse mdm_send_cmd(SerPort *sp, const char *cmd,
                                  MdmResult *result, int timeout_ms)
{
    char line[128];
    MdmResponse rc;

    ser_flush(sp);

    /* Send command + CR */
    ser_write_str(sp, cmd);
    ser_write_str(sp, "\r");

    /* Read response lines until we get a result code or timeout */
    while (1) {
        rc = mdm_read_line(sp, line, sizeof(line), timeout_ms);
        if (rc == MDM_TIMEOUT) {
            if (result) {
                result->code = MDM_TIMEOUT;
                strncpy(result->raw, "(timeout)", sizeof(result->raw) - 1);
            }
            return MDM_TIMEOUT;
        }

        /* Skip echo of our command */
        if (strncmp(line, "AT", 2) == 0 || line[0] == '\0')
            continue;

        /* Parse the response */
        if (result)
            mdm_parse_response(line, result);
        else {
            MdmResult tmp;
            mdm_parse_response(line, &tmp);
            return tmp.code;
        }

        if (result->code != MDM_NONE)
            return result->code;
    }
}


/* ---- Initialize Modem ---- */

int mdm_init(SerPort *sp, const MdmConfig *cfg)
{
    MdmResult result;
    int attempt;

    qf_log(LOG_INFO, "Initializing modem");

    for (attempt = 1; attempt <= cfg->init_retries; attempt++) {
        /* Send primary init string */
        if (mdm_send_cmd(sp, cfg->init_str, &result, 5000) == MDM_OK) {
            /* Send secondary init if configured */
            if (cfg->init_str2[0]) {
                if (mdm_send_cmd(sp, cfg->init_str2, &result, 5000) != MDM_OK) {
                    qf_log(LOG_WARN, "Secondary init failed: %s", result.raw);
                }
            }
            qf_log(LOG_INFO, "Modem initialized");
            return 0;
        }

        qf_log(LOG_WARN, "Modem initialization error, retry %d", attempt);
    }

    qf_log(LOG_ERROR, "Unable to initialize modem");
    return -1;
}


/* ---- Dial a Phone Number ---- */

int mdm_dial(SerPort *sp, const MdmConfig *cfg, const char *phone,
              MdmResult *result)
{
    char cmd[256];
    MdmResponse rc;

    snprintf(cmd, sizeof(cmd), "%s%s%s",
             cfg->dial_prefix, phone, cfg->dial_suffix);

    qf_log(LOG_INFO, "Dialing %s", phone);

    rc = mdm_send_cmd(sp, cmd, result, cfg->connect_timeout * 1000);

    switch (rc) {
    case MDM_CONNECT:
        qf_log(LOG_INFO, "Connect speed = %lu", 
               (unsigned long)result->connect_speed);
        qf_log(LOG_INFO, "Connect string = \"%s\"", result->raw);
        return 0;

    case MDM_BUSY:
        qf_log(LOG_INFO, "Modem response - BUSY");
        qf_log(LOG_INFO, "Called modem is busy");
        return -1;

    case MDM_NO_CARRIER:
        qf_log(LOG_INFO, "Modem response - NO CARRIER");
        return -1;

    case MDM_NO_DIALTONE:
        qf_log(LOG_INFO, "Modem response - NO DIALTONE");
        return -1;

    case MDM_NO_ANSWER:
        qf_log(LOG_INFO, "Modem response - NO ANSWER");
        return -1;

    case MDM_VOICE:
        qf_log(LOG_INFO, "Modem response - VOICE");
        qf_log(LOG_INFO, "Call is VOICE");
        return -1;

    case MDM_CONNECT_FAX:
        qf_log(LOG_INFO, "CONNECT FAX");
        return -2;                /* Special: fax call            */

    case MDM_TIMEOUT:
        qf_log(LOG_INFO, "Dial result: Timed out");
        qf_log(LOG_DEBUG, "Maximum redials reached dialing");
        return -1;

    default:
        qf_log(LOG_INFO, "Dial result: Unknown");
        return -1;
    }
}


/*-----------------------------------------------------------------------*/
/* mdm_answer() — Answer an incoming call                                */
/*                                                                         */
/* Called after mdm_wait_ring() returns 0 (RING detected).               */
/* Sends the answer command (typically "ATA") and waits for the modem    */
/* to report the result.                                                   */
/*                                                                         */
/* The modem will respond with one of:                                   */
/*   CONNECT xxxxx  — data call connected at xxxxx bps                   */
/*   CONNECT FAX    — incoming fax call                                  */
/*   NO CARRIER     — caller hung up before connect                      */
/*   ERROR          — modem error (bad command, etc.)                    */
/*                                                                         */
/* Returns:                                                               */
/*    0 = CONNECT (data call — proceed to handshake detection)           */
/*   -1 = failed to connect (NO CARRIER, ERROR, timeout)                */
/*   -2 = FAX call detected (CONNECT FAX)                                */
/*                                                                         */
/* The connect_timeout field (typically 60 seconds) controls how long    */
/* we wait for CONNECT. This covers the modem negotiation time —         */
/* V.32bis/V.34 modems can take 15-30 seconds to train.                  */
/*-----------------------------------------------------------------------*/

int mdm_answer(SerPort *sp, const MdmConfig *cfg, MdmResult *result)
{
    MdmResponse rc;

    qf_log(LOG_INFO, "Answering call — sending \"%s\"", cfg->answer_cmd);
    qf_log(LOG_DEBUG, "Connect timeout = %d seconds", cfg->connect_timeout);

    rc = mdm_send_cmd(sp, cfg->answer_cmd, result,
                       cfg->connect_timeout * 1000);

    qf_log(LOG_DEBUG, "Modem response code = %d, raw = \"%s\"",
           (int)rc, result->raw);

    if (rc == MDM_CONNECT) {
        /* Data call — modem negotiated a connection.
         * connect_speed is parsed from "CONNECT 14400" etc.
         * This could be a FidoNet mailer OR a human with a terminal. */
        qf_log(LOG_INFO, "CONNECT at %lu bps — data call",
               (unsigned long)result->connect_speed);
        return 0;
    }

    if (rc == MDM_CONNECT_FAX) {
        /* FAX call — modem detected CNG (calling) tone.
         * We don't handle fax; return -2 so the caller can
         * route to fax software via BOARD.BAT errorlevel 5. */
        qf_log(LOG_INFO, "CONNECT FAX — fax call detected");
        return -2;
    }

    /* Failed — NO CARRIER, BUSY, ERROR, or timeout.
     * Most common: caller hung up during modem negotiation. */
    qf_log(LOG_WARN, "Failed to answer: \"%s\" (code=%d)",
           result->raw, (int)rc);
    return -1;
}


/*-----------------------------------------------------------------------*/
/* mdm_hangup() — Hang up the modem and drop the call                    */
/*                                                                         */
/* Uses two methods for maximum compatibility:                           */
/*                                                                         */
/*   Method 1: Drop DTR for 1 second.                                   */
/*     Most modems are configured (AT&D2) to hang up when DTR drops.     */
/*     This is the preferred method — fast and reliable.                 */
/*     After 1 second, raise DTR again so the modem is ready.            */
/*                                                                         */
/*   Method 2: Send "+++" escape sequence, then ATH0.                    */
/*     Fallback for modems where DTR drop doesn't work (AT&D0 mode).     */
/*     The 1.1 second guard time before and after "+++" is required      */
/*     by the Hayes AT command set — the modem ignores "+++" if there    */
/*     is data within the guard time on either side.                     */
/*                                                                         */
/* Both methods are always used because we don't know the modem's        */
/* AT&D setting. One of them will work. If both fail, the modem will     */
/* eventually time out and hang up on its own (S0 register timeout).     */
/*-----------------------------------------------------------------------*/

int mdm_hangup(SerPort *sp, const MdmConfig *cfg)
{
    MdmResult result;

    qf_log(LOG_DEBUG, "Hanging up — dropping DTR");

    /* Method 1: Drop DTR (1 second) */
    ser_set_dtr(sp, 0);
#ifdef _WIN32
    Sleep(1000);
#elif defined(__MSDOS__) || defined(__DOS__)
    delay(1000);
#else
    sleep(1);
#endif
    ser_set_dtr(sp, 1);

    qf_log(LOG_DEBUG, "DTR raised — sending +++ escape sequence");

    /* Method 2: Send +++ then hangup command (ATH0)
     * Guard time: 1.1 seconds of silence before and after +++ */
#ifdef _WIN32
    Sleep(1100);
#elif defined(__MSDOS__) || defined(__DOS__)
    delay(1100);
#else
    usleep(1100000);
#endif
    ser_write_str(sp, "+++");
#ifdef _WIN32
    Sleep(1100);
#elif defined(__MSDOS__) || defined(__DOS__)
    delay(1100);
#else
    usleep(1100000);
#endif

    qf_log(LOG_DEBUG, "Sending hangup command: \"%s\"", cfg->hangup_cmd);
    mdm_send_cmd(sp, cfg->hangup_cmd, &result, 3000);

    qf_log(LOG_DEBUG, "Modem hangup complete (response: \"%s\")",
           result.raw);
    return 0;
}


/*-----------------------------------------------------------------------*/
/* mdm_wait_ring() — Wait for the phone to ring                          */
/*                                                                         */
/* Sits on the serial port reading modem responses, looking for "RING".  */
/* Returns 0 as soon as RING is detected, or -1 on timeout.              */
/*                                                                         */
/* This is where QFront spends most of its idle time in front-end mode.  */
/* The timeout is typically retry_delay seconds from config (30-300).     */
/* Between calls to this function, the main loop checks for outbound     */
/* mail to send.                                                           */
/*                                                                         */
/* Note: Some modems send other unsolicited responses that are NOT ring: */
/*   "OK"           — modem completed a previous command                 */
/*   "DATE = ..."   — Caller ID date (if enabled)                       */
/*   "NMBR = ..."   — Caller ID number (if enabled)                     */
/*   "NAME = ..."   — Caller ID name (if enabled)                       */
/* These are all ignored — we only care about "RING".                    */
/*                                                                         */
/* timeout_ms: milliseconds to wait before giving up (0 = forever)       */
/* Returns: 0 = RING detected, -1 = timeout                              */
/*-----------------------------------------------------------------------*/

int mdm_wait_ring(SerPort *sp, int timeout_ms)
{
    char line[128];
    MdmResult result;
    MdmResponse rc;

    qf_log(LOG_DEBUG, "mdm_wait_ring: listening (timeout=%d ms)",
           timeout_ms);

    while (1) {
        rc = mdm_read_line(sp, line, sizeof(line), timeout_ms);

        if (rc == MDM_TIMEOUT) {
            /* Normal — no call within timeout window */
            qf_log(LOG_DEBUG, "mdm_wait_ring: timeout — no ring");
            return -1;
        }

        mdm_parse_response(line, &result);

        if (result.code == MDM_RING) {
            qf_log(LOG_INFO, "Ring detected");
            qf_log(LOG_DEBUG, "Ring string = \"%s\"", line);
            return 0;
        }

        /* Not RING — log and continue waiting.
         * Could be Caller ID data, OK from previous command, etc. */
        if (line[0] != '\0') {
            qf_log(LOG_DEBUG, "mdm_wait_ring: ignoring \"%s\" (not RING)",
                   line);
        }
    }
}
