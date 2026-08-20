/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* modem.c -- Modem Control (AT Commands)                                   */
/*                                                                           */
/* Modem initialization, dialing, answering, and hangup. Parses modem       */
/* response strings from binary analysis:                                    */
/*   CONNECT, BUSY, NO CARRIER, NO DIALTONE, NO ANSWER,                     */
/*   RING, RINGING, VOICE, ERROR, CONNECT FAX                               */
/*                                                                           */
/* Clean-room from QFront binary analysis + standard AT command set.         */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "qfront.h"

/* Forward declarations -- serial.c */
typedef struct SerPort SerPort;
extern int  ser_read_byte(SerPort *sp, int timeout_ms);
extern int  ser_write(SerPort *sp, const void *buf, int len);
extern int  ser_write_str(SerPort *sp, const char *str);
extern void ser_set_dtr(SerPort *sp, int on);
extern void ser_flush(SerPort *sp);
extern int  ser_get_dcd(SerPort *sp);
extern int  ser_data_ready(SerPort *sp);


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                       Modem Response Codes                                */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef enum {
    MDM_NONE = 0,                       /* no response parsed yet        */
    MDM_OK,                             /* OK                            */
    MDM_CONNECT,                        /* CONNECT [speed]               */
    MDM_RING,                           /* RING                          */
    MDM_NO_CARRIER,                     /* NO CARRIER                    */
    MDM_ERROR,                          /* ERROR                         */
    MDM_NO_DIALTONE,                    /* NO DIALTONE                   */
    MDM_BUSY,                           /* BUSY                          */
    MDM_NO_ANSWER,                      /* NO ANSWER                     */
    MDM_RINGING,                        /* RINGING                       */
    MDM_VOICE,                          /* VOICE                         */
    MDM_CONNECT_FAX,                    /* CONNECT FAX                   */
    MDM_TIMEOUT                         /* no response within timeout    */
} MdmResponse;

typedef struct {
    MdmResponse Code;                   /* parsed response code          */
    uint32_t    ConnectSpeed;           /* parsed from CONNECT <speed>   */
    uint32_t    CarrierSpeed;           /* carrier vs DTE speed          */
    char        Raw[128];               /* raw response string           */
} MdmResult;


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                       Modem Configuration                                 */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef struct {
    char InitStr[128];                  /* primary init (e.g. ATS0=0M0)  */
    char InitStr2[128];                 /* secondary init                */
    char DialPrefix[32];                /* dial command (e.g. ATDT)      */
    char DialSuffix[32];                /* after phone number            */
    char AnswerCmd[32];                 /* answer command (e.g. ATA)     */
    char HangupCmd[32];                 /* hangup command (e.g. ATH0)    */
    int  InitRetries;                   /* max init attempts             */
    int  ConnectTimeout;                /* seconds to wait for CONNECT   */
    int  RedialWait;                    /* seconds between redials       */
    int  MaxRedials;                    /* max dial attempts             */
    int  ResetMinutes;                  /* auto-reset interval           */
} MdmConfig;


/*-----------------------------------------------------------------------*/
/* mdm_read_line() -- Read a line of text from the modem                */
/*                                                                       */
/* Reads characters until CR/LF, skipping leading CR/LF. Returns        */
/* MDM_TIMEOUT if no data within timeout_ms, or MDM_NONE if a           */
/* complete line was read (caller parses the string).                    */
/*-----------------------------------------------------------------------*/

static MdmResponse mdm_read_line(SerPort *Sp, char *Buf, int BufSize,
                                  int TimeoutMs)
{
    int Pos = 0;                        /* buffer write position         */
    int Ch;                             /* received character            */

    Buf[0] = '\0';

    while (Pos < BufSize - 1) {
        Ch = ser_read_byte(Sp, TimeoutMs);
        if (Ch < 0)
            return MDM_TIMEOUT;

        if (Ch == '\r' || Ch == '\n') {
            if (Pos > 0) {
                Buf[Pos] = '\0';
                break;                  /* got a complete line           */
            }
            continue;                   /* skip leading CR/LF            */
        }

        Buf[Pos++] = (char)Ch;
    }

    Buf[Pos] = '\0';
    return MDM_NONE;                    /* caller parses the string      */
}


/*-----------------------------------------------------------------------*/
/* mdm_parse_response() -- Parse a modem response string                */
/*                                                                       */
/* Identifies the response code from the raw string. For CONNECT,        */
/* also parses the speed (e.g. "CONNECT 14400" or "CONNECT 14400/ARQ"). */
/*-----------------------------------------------------------------------*/

static void mdm_parse_response(const char *Line, MdmResult *Result)
{
    memset(Result, 0, sizeof(*Result));
    strncpy(Result->Raw, Line, sizeof(Result->Raw) - 1);

    if (strcmp(Line, "OK") == 0) {
        Result->Code = MDM_OK;
    }
    else if (strncmp(Line, "CONNECT FAX", 11) == 0) {
        Result->Code = MDM_CONNECT_FAX;
    }
    else if (strncmp(Line, "CONNECT", 7) == 0) {
        Result->Code = MDM_CONNECT;
        /* Parse speed: "CONNECT 14400" or "CONNECT 14400/ARQ" */
        if (Line[7] == ' ')
            Result->ConnectSpeed = (uint32_t)atol(Line + 8);
        else
            Result->ConnectSpeed = 300; /* plain "CONNECT" = 300 bps     */
    }
    else if (strcmp(Line, "RING") == 0) {
        Result->Code = MDM_RING;
    }
    else if (strcmp(Line, "NO CARRIER") == 0) {
        Result->Code = MDM_NO_CARRIER;
    }
    else if (strcmp(Line, "ERROR") == 0) {
        Result->Code = MDM_ERROR;
    }
    else if (strcmp(Line, "NO DIALTONE") == 0 ||
             strcmp(Line, "NO DIAL TONE") == 0) {
        Result->Code = MDM_NO_DIALTONE;
    }
    else if (strcmp(Line, "BUSY") == 0) {
        Result->Code = MDM_BUSY;
    }
    else if (strcmp(Line, "NO ANSWER") == 0) {
        Result->Code = MDM_NO_ANSWER;
    }
    else if (strcmp(Line, "RINGING") == 0) {
        Result->Code = MDM_RINGING;
    }
    else if (strcmp(Line, "VOICE") == 0) {
        Result->Code = MDM_VOICE;
    }
}


/*-----------------------------------------------------------------------*/
/* mdm_send_cmd() -- Send an AT command and wait for response           */
/*                                                                       */
/* Flushes the serial buffer, sends the command with CR, then reads      */
/* response lines until a result code is found or timeout.               */
/* Skips command echo lines (starting with "AT").                        */
/*-----------------------------------------------------------------------*/

static MdmResponse mdm_send_cmd(SerPort *Sp, const char *Cmd,
                                  MdmResult *Result, int TimeoutMs)
{
    char        Line[128];              /* response line buffer          */
    MdmResponse Rc;                     /* response from mdm_read_line   */

    ser_flush(Sp);

    /* Send command + CR */
    ser_write_str(Sp, Cmd);
    ser_write_str(Sp, "\r");

    /* Read response lines until we get a result code or timeout */
    while (1) {
        Rc = mdm_read_line(Sp, Line, sizeof(Line), TimeoutMs);
        if (Rc == MDM_TIMEOUT) {
            if (Result) {
                Result->Code = MDM_TIMEOUT;
                strncpy(Result->Raw, "(timeout)", sizeof(Result->Raw) - 1);
            }
            return MDM_TIMEOUT;
        }

        /* Skip echo of our command */
        if (strncmp(Line, "AT", 2) == 0 || Line[0] == '\0')
            continue;

        /* Parse the response */
        if (Result)
            mdm_parse_response(Line, Result);
        else {
            MdmResult Tmp;              /* temp result for code only     */
            mdm_parse_response(Line, &Tmp);
            return Tmp.Code;
        }

        if (Result->Code != MDM_NONE)
            return Result->Code;
    }
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      Modem Operations                                     */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* mdm_init() -- Initialize the modem                                   */
/*                                                                       */
/* Sends the primary init string (and optional secondary) up to          */
/* InitRetries times. The init string typically sets: S0=0 (no auto-    */
/* answer), M0 (speaker off), DT (tone dial mode).                      */
/*                                                                       */
/* Returns 0 on success, -1 if modem won't respond to init.             */
/*-----------------------------------------------------------------------*/

int mdm_init(SerPort *Sp, const MdmConfig *Cfg)
{
    MdmResult Result;                   /* modem response                */
    int       Attempt;                  /* retry counter                 */

    qf_log(LOG_INFO, "Initializing modem");

    for (Attempt = 1; Attempt <= Cfg->InitRetries; Attempt++) {
        /* Send primary init string */
        if (mdm_send_cmd(Sp, Cfg->InitStr, &Result, 5000) == MDM_OK) {
            /* Send secondary init if configured */
            if (Cfg->InitStr2[0]) {
                if (mdm_send_cmd(Sp, Cfg->InitStr2, &Result, 5000) != MDM_OK)
                    qf_log(LOG_WARN, "Secondary init failed: %s", Result.Raw);
            }
            qf_log(LOG_INFO, "Modem initialized");
            return 0;
        }

        qf_log(LOG_WARN, "Modem initialization error, retry %d", Attempt);
    }

    qf_log(LOG_ERROR, "Unable to initialize modem");
    return -1;
}


/*-----------------------------------------------------------------------*/
/* mdm_dial() -- Dial a phone number                                    */
/*                                                                       */
/* Constructs the dial command from prefix + phone + suffix and sends    */
/* it. Waits up to ConnectTimeout seconds for the modem response.       */
/*                                                                       */
/* Returns:                                                              */
/*    0 = CONNECT (call connected)                                       */
/*   -1 = failed (BUSY, NO CARRIER, NO DIALTONE, NO ANSWER, timeout)    */
/*   -2 = FAX call detected (CONNECT FAX)                                */
/*-----------------------------------------------------------------------*/

int mdm_dial(SerPort *Sp, const MdmConfig *Cfg, const char *Phone,
              MdmResult *Result)
{
    char        Cmd[256];               /* complete dial command          */
    MdmResponse Rc;                     /* modem response code           */

    snprintf(Cmd, sizeof(Cmd), "%s%s%s",
             Cfg->DialPrefix, Phone, Cfg->DialSuffix);

    qf_log(LOG_INFO, "Dialing %s", Phone);

    Rc = mdm_send_cmd(Sp, Cmd, Result, Cfg->ConnectTimeout * 1000);

    switch (Rc) {
    case MDM_CONNECT:
        qf_log(LOG_INFO, "Connect speed = %lu",
               (unsigned long)Result->ConnectSpeed);
        qf_log(LOG_INFO, "Connect string = \"%s\"", Result->Raw);
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
        return -2;                      /* special: fax call             */

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
/* mdm_answer() -- Answer an incoming call                              */
/*                                                                       */
/* Called after mdm_wait_ring() returns 0 (RING detected). Sends the    */
/* answer command (typically "ATA") and waits for the modem to report    */
/* the result.                                                           */
/*                                                                       */
/* The modem will respond with one of:                                   */
/*   CONNECT xxxxx  -- data call connected at xxxxx bps                  */
/*   CONNECT FAX    -- incoming fax call                                 */
/*   NO CARRIER     -- caller hung up before connect                     */
/*   ERROR          -- modem error (bad command, etc.)                   */
/*                                                                       */
/* Returns:                                                              */
/*    0 = CONNECT (data call -- proceed to handshake detection)          */
/*   -1 = failed to connect (NO CARRIER, ERROR, timeout)                */
/*   -2 = FAX call detected (CONNECT FAX)                                */
/*                                                                       */
/* The ConnectTimeout field (typically 60 seconds) controls how long     */
/* we wait for CONNECT. This covers modem negotiation time --            */
/* V.32bis/V.34 modems can take 15-30 seconds to train.                 */
/*-----------------------------------------------------------------------*/

int mdm_answer(SerPort *Sp, const MdmConfig *Cfg, MdmResult *Result)
{
    MdmResponse Rc;                     /* modem response code           */

    qf_log(LOG_INFO, "Answering call -- sending \"%s\"", Cfg->AnswerCmd);
    qf_log(LOG_DEBUG, "Connect timeout = %d seconds", Cfg->ConnectTimeout);

    Rc = mdm_send_cmd(Sp, Cfg->AnswerCmd, Result,
                       Cfg->ConnectTimeout * 1000);

    qf_log(LOG_DEBUG, "Modem response code = %d, raw = \"%s\"",
           (int)Rc, Result->Raw);

    if (Rc == MDM_CONNECT) {
        qf_log(LOG_INFO, "CONNECT at %lu bps -- data call",
               (unsigned long)Result->ConnectSpeed);
        return 0;
    }

    if (Rc == MDM_CONNECT_FAX) {
        qf_log(LOG_INFO, "CONNECT FAX -- fax call detected");
        return -2;
    }

    qf_log(LOG_WARN, "Failed to answer: \"%s\" (code=%d)",
           Result->Raw, (int)Rc);
    return -1;
}


/*-----------------------------------------------------------------------*/
/* mdm_hangup() -- Hang up the modem and drop the call                  */
/*                                                                       */
/* Uses two methods for maximum compatibility:                           */
/*                                                                       */
/*   Method 1: Drop DTR for 1 second.                                   */
/*     Most modems are configured (AT&D2) to hang up when DTR drops.     */
/*     This is the preferred method -- fast and reliable.                */
/*     After 1 second, raise DTR again so the modem is ready.            */
/*                                                                       */
/*   Method 2: Send "+++" escape sequence, then ATH0.                    */
/*     Fallback for modems where DTR drop doesn't work (AT&D0 mode).     */
/*     The 1.1 second guard time before and after "+++" is required      */
/*     by the Hayes AT command set -- the modem ignores "+++" if there   */
/*     is data within the guard time on either side.                     */
/*                                                                       */
/* Both methods are always used because we don't know the modem's        */
/* AT&D setting. One of them will work.                                  */
/*-----------------------------------------------------------------------*/

int mdm_hangup(SerPort *Sp, const MdmConfig *Cfg)
{
    MdmResult Result;                   /* hangup command response       */

    qf_log(LOG_DEBUG, "Hanging up -- dropping DTR");

    /* Method 1: Drop DTR (1 second) */
    ser_set_dtr(Sp, 0);
#ifdef _WIN32
    Sleep(1000);
#elif defined(__MSDOS__) || defined(__DOS__)
    delay(1000);
#else
    sleep(1);
#endif
    ser_set_dtr(Sp, 1);

    qf_log(LOG_DEBUG, "DTR raised -- sending +++ escape sequence");

    /* Method 2: Send +++ then hangup command (ATH0)
     * Guard time: 1.1 seconds of silence before and after +++ */
#ifdef _WIN32
    Sleep(1100);
#elif defined(__MSDOS__) || defined(__DOS__)
    delay(1100);
#else
    usleep(1100000);
#endif
    ser_write_str(Sp, "+++");
#ifdef _WIN32
    Sleep(1100);
#elif defined(__MSDOS__) || defined(__DOS__)
    delay(1100);
#else
    usleep(1100000);
#endif

    qf_log(LOG_DEBUG, "Sending hangup command: \"%s\"", Cfg->HangupCmd);
    mdm_send_cmd(Sp, Cfg->HangupCmd, &Result, 3000);

    qf_log(LOG_DEBUG, "Modem hangup complete (response: \"%s\")",
           Result.Raw);
    return 0;
}


/*-----------------------------------------------------------------------*/
/* mdm_wait_ring() -- Wait for the phone to ring                        */
/*                                                                       */
/* Sits on the serial port reading modem responses, looking for "RING".  */
/* Returns 0 as soon as RING is detected, or -1 on timeout.              */
/*                                                                       */
/* This is where QFront spends most of its idle time in front-end mode.  */
/* The timeout is typically retry_delay seconds from config (30-300).     */
/* Between calls to this function, the main loop checks for outbound     */
/* mail to send.                                                          */
/*                                                                       */
/* Note: Some modems send other unsolicited responses that are NOT ring: */
/*   "OK"           -- modem completed a previous command                */
/*   "DATE = ..."   -- Caller ID date (if enabled)                       */
/*   "NMBR = ..."   -- Caller ID number (if enabled)                     */
/*   "NAME = ..."   -- Caller ID name (if enabled)                       */
/* These are all ignored -- we only care about "RING".                   */
/*                                                                       */
/* TimeoutMs: milliseconds to wait before giving up (0 = forever)        */
/* Returns: 0 = RING detected, -1 = timeout                              */
/*-----------------------------------------------------------------------*/

int mdm_wait_ring(SerPort *Sp, int TimeoutMs)
{
    char        Line[128];              /* response line buffer          */
    MdmResult   Result;                 /* parsed response               */
    MdmResponse Rc;                     /* read result                   */

    qf_log(LOG_DEBUG, "mdm_wait_ring: listening (timeout=%d ms)",
           TimeoutMs);

    while (1) {
        Rc = mdm_read_line(Sp, Line, sizeof(Line), TimeoutMs);

        if (Rc == MDM_TIMEOUT) {
            /* Normal -- no call within timeout window */
            qf_log(LOG_DEBUG, "mdm_wait_ring: timeout -- no ring");
            return -1;
        }

        mdm_parse_response(Line, &Result);

        if (Result.Code == MDM_RING) {
            qf_log(LOG_INFO, "Ring detected");
            qf_log(LOG_DEBUG, "Ring string = \"%s\"", Line);
            return 0;
        }

        /* Not RING -- log and continue waiting.
         * Could be Caller ID data, OK from previous command, etc. */
        if (Line[0] != '\0') {
            qf_log(LOG_DEBUG, "mdm_wait_ring: ignoring \"%s\" (not RING)",
                   Line);
        }
    }
}
