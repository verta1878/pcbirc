/* ====================================================================
 * emsi.c — EMSI Handshake Protocol (FSC-0056)
 * ====================================================================
 * Implements the EMSI (Electronic Mail Standard Identification)
 * handshake used to negotiate FidoNet sessions.
 *
 * State machine:
 *   Caller:  send EMSI_INQ → wait EMSI_REQ → send EMSI_DAT →
 *            wait EMSI_DAT → send EMSI_ACK → session established
 *   Answer:  wait EMSI_INQ → send EMSI_REQ → wait EMSI_DAT →
 *            send EMSI_DAT → wait EMSI_ACK → session established
 *
 * Tokens (from binary):
 *   EMSI_INQ   **EMSI_INQC816     Inquiry
 *   EMSI_REQ   **EMSI_REQA77E     Request
 *   EMSI_DAT   {EMSI_DAT}{len}{data}{crc}  Data packet
 *   EMSI_ACK   **EMSI_ACKA490     Acknowledge
 *   EMSI_NAK   **EMSI_NAKE2B1     Negative ack
 *   EMSI_HBT   **EMSI_HBTEBC3     Heartbeat
 *   EMSI_CLI   **EMSI_CLI...      Client info
 *
 * Clean-room from FSC-0056 (public FidoNet specification).
 * ==================================================================== */

#include "qfront.h"

/* Forward declarations — serial.c */
typedef struct SerPort SerPort;
extern int  ser_read_byte(SerPort *sp, int timeout_ms);
extern int  ser_write(SerPort *sp, const void *buf, int len);
extern int  ser_write_str(SerPort *sp, const char *str);
extern int  ser_get_dcd(SerPort *sp);
extern int  ser_data_ready(SerPort *sp);
extern void ser_flush(SerPort *sp);

/* ---- EMSI Fixed Tokens ----
 * These include the CRC-16 appended to the token.
 * FSC-0056 Section 3: tokens are sent as "**EMSI_XXXxxxx\r" */

#define EMSI_INQ    "**EMSI_INQC816\r"
#define EMSI_REQ    "**EMSI_REQA77E\r"
#define EMSI_ACK    "**EMSI_ACKA490\r"
#define EMSI_NAK    "**EMSI_NAKE2B1\r"
#define EMSI_HBT    "**EMSI_HBTEBC3\r"

#define EMSI_MAX_RETRIES  6
#define EMSI_TIMEOUT_MS   20000   /* 20 seconds                  */
#define EMSI_DAT_MAXLEN   4096

/* ---- EMSI Session Data ---- */

typedef struct {
    FTN_ADDR addrs[16];           /* AKA list                    */
    int      num_addrs;
    char     sysop[64];           /* Sysop name                  */
    char     system_name[64];     /* System/BBS name             */
    char     location[64];        /* City, State                 */
    char     phone[40];           /* Phone number                */
    char     password[32];        /* Session password            */
    char     mailer[32];          /* Mailer name + version       */
    uint32_t link_codes;          /* Capability flags            */
    uint32_t comp_codes;          /* Compression support         */
} EmsiData;

/* Link codes (FSC-0056 Section 7) */
#define EMSI_LC_CALLERID  0x0001  /* Caller pays                 */
#define EMSI_LC_PUA       0x0002  /* Pickup all                  */
#define EMSI_LC_PUP       0x0004  /* Pickup primary address only */
#define EMSI_LC_NPU       0x0008  /* No pickup                   */
#define EMSI_LC_HAT       0x0010  /* Hold all traffic            */
#define EMSI_LC_HXT       0x0020  /* Hold compressed mail only   */
#define EMSI_LC_HRQ       0x0040  /* Hold file requests          */

/* ---- CRC-16 (CCITT) ----
 * FSC-0056 uses CRC-16/CCITT (poly 0x1021, init 0). */

static uint16_t emsi_crc16(const void *data, int len)
{
    const unsigned char *p = (const unsigned char *)data;
    uint16_t crc = 0;
    int i, j;

    for (i = 0; i < len; i++) {
        crc ^= (uint16_t)p[i] << 8;
        for (j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }

    return crc;
}


/* ---- Build EMSI_DAT Packet ----
 *
 * FSC-0056 Section 5:
 *   {EMSI_DAT}{<hex_len>}{<data>}{<CRC32>}
 *
 * Data is a series of brace-delimited fields:
 *   {system_name}{location}{sysop}{phone}{baud}{mailer}
 *   {<addr_list>}{password}{link_codes}{comp_codes}
 *
 * We use CRC-16 (the simpler variant allowed by FSC-0056). */

static int emsi_build_dat(const EmsiData *data, char *buf, int bufsize)
{
    char payload[2048];
    char addrs[256];
    uint16_t crc;
    int plen, i;

    /* Build address list */
    addrs[0] = '\0';
    {
        int apos = 0;
        for (i = 0; i < data->num_addrs; i++) {
            char a[64];
            ftn_format_addr(&data->addrs[i], a, sizeof(a));
            if (apos > 0 && apos < 250)
                apos += snprintf(addrs + apos, sizeof(addrs) - apos, " ");
            if (apos < 250)
                apos += snprintf(addrs + apos, sizeof(addrs) - apos, "%s", a);
        }
    }

    /* Build payload: {field}{field}... */
    plen = snprintf(payload, sizeof(payload),
        "{EMSI}{%s}{%s}{%s}{%s}{%s}{8N1,PC}{%s}{%s}{%s}{}",
        data->system_name,
        data->location,
        data->sysop,
        data->phone[0] ? data->phone : "-Unpublished-",
        "115200",                 /* Speed — always report max   */
        data->mailer,
        addrs,
        data->password);

    /* Calculate CRC over the payload */
    crc = emsi_crc16(payload, plen);

    /* Build complete packet: {EMSI_DAT}{hex_len}{payload}{hex_crc} */
    return snprintf(buf, bufsize, "{EMSI_DAT}%04X%s%04X\r",
                    plen, payload, crc);
}


/* ---- Parse EMSI_DAT Packet ---- */

static int emsi_parse_dat(const char *pkt, int pktlen, EmsiData *data)
{
    const char *p = pkt;
    char field[256];
    int fi, fcount;

    memset(data, 0, sizeof(*data));

    /* Skip {EMSI_DAT} header and hex length */
    if (strncmp(p, "{EMSI_DAT}", 10) == 0)
        p += 10;

    /* Skip 4-digit hex length */
    if (strlen(p) >= 4) {
        /* Validate length if needed */
        p += 4;
    }

    /* Parse brace-delimited fields */
    fcount = 0;
    while (*p && fcount < 20) {
        if (*p != '{') { p++; continue; }
        p++;                      /* Skip opening brace          */

        /* Read field content */
        fi = 0;
        while (*p && *p != '}' && fi < 255)
            field[fi++] = *p++;
        field[fi] = '\0';

        if (*p == '}') p++;       /* Skip closing brace          */

        /* Map fields by position (FSC-0056 Section 5) */
        switch (fcount) {
        case 0:                   /* "EMSI" tag — skip           */
            break;
        case 1:                   /* System name                 */
            strncpy(data->system_name, field, 63);
            break;
        case 2:                   /* Location                    */
            strncpy(data->location, field, 63);
            break;
        case 3:                   /* Sysop                       */
            strncpy(data->sysop, field, 63);
            break;
        case 4:                   /* Phone                       */
            strncpy(data->phone, field, 39);
            break;
        case 5:                   /* Speed — ignore              */
            break;
        case 6:                   /* Flags — ignore              */
            break;
        case 7:                   /* Mailer                      */
            strncpy(data->mailer, field, 31);
            break;
        case 8:                   /* Address list                */
            {
                char *tok = strtok(field, " ");
                while (tok && data->num_addrs < 16) {
                    ftn_parse_addr(tok, &data->addrs[data->num_addrs++]);
                    tok = strtok(NULL, " ");
                }
            }
            break;
        case 9:                   /* Password                    */
            strncpy(data->password, field, 31);
            break;
        }

        fcount++;
    }

    return (data->num_addrs > 0) ? 0 : -1;
}


/* ---- Wait for EMSI Token ----
 * Reads from serial port looking for "**EMSI_XXX" tokens. */

static int emsi_wait_token(SerPort *sp, char *token, int toksize,
                            int timeout_ms)
{
    int pos = 0;
    int ch;
    int star_count = 0;

    token[0] = '\0';

    while (1) {
        ch = ser_read_byte(sp, timeout_ms);
        if (ch < 0) return -1;   /* Timeout                     */

        /* Look for ** prefix */
        if (ch == '*') {
            star_count++;
            if (star_count == 2) {
                /* Found ** — read rest of token until CR */
                token[0] = '*';
                token[1] = '*';
                pos = 2;
                while (pos < toksize - 1) {
                    ch = ser_read_byte(sp, 2000);
                    if (ch < 0 || ch == '\r' || ch == '\n') break;
                    token[pos++] = (char)ch;
                }
                token[pos] = '\0';
                return 0;
            }
        } else if (ch == '{') {
            /* EMSI_DAT packet starts with { */
            token[0] = '{';
            pos = 1;
            while (pos < toksize - 1) {
                ch = ser_read_byte(sp, 2000);
                if (ch < 0 || ch == '\r') break;
                token[pos++] = (char)ch;
            }
            token[pos] = '\0';
            return 0;
        } else {
            star_count = 0;
        }

        /* Check carrier */
        if (!ser_get_dcd(sp)) {
            qf_log(LOG_WARN, "Lost carrier during EMSI handshake");
            return -2;
        }
    }
}


/* ---- Save EMSI Data to File ---- */

static void emsi_save_dat(const char *filename, const EmsiData *data)
{
    FILE *f = fopen(filename, "w");
    int i;
    char buf[64];

    if (!f) return;

    fprintf(f, "System: %s\n", data->system_name);
    fprintf(f, "Sysop:  %s\n", data->sysop);
    fprintf(f, "Location: %s\n", data->location);
    fprintf(f, "Phone:  %s\n", data->phone);
    fprintf(f, "Mailer: %s\n", data->mailer);
    fprintf(f, "Password: %s\n", data->password);

    for (i = 0; i < data->num_addrs; i++) {
        ftn_format_addr(&data->addrs[i], buf, sizeof(buf));
        fprintf(f, "AKA %d: %s\n", i, buf);
    }

    fclose(f);
}


/* ---- EMSI Handshake (Caller) ----
 * We are calling out — we initiate. */

int emsi_handshake_caller(SerPort *sp, const EmsiData *our_data,
                           EmsiData *remote_data)
{
    char token[EMSI_DAT_MAXLEN];
    char dat_pkt[EMSI_DAT_MAXLEN];
    int attempt;

    qf_log(LOG_INFO, "Establishing FidoMail handshake");

    /* Send EMSI_INQ repeatedly until we get EMSI_REQ */
    for (attempt = 0; attempt < EMSI_MAX_RETRIES; attempt++) {
        qf_log(LOG_DEBUG, "Sending EMSI_INQ (attempt %d)", attempt + 1);
        ser_write_str(sp, EMSI_INQ);

        if (emsi_wait_token(sp, token, sizeof(token), EMSI_TIMEOUT_MS) == 0) {
            if (strncmp(token, "**EMSI_REQ", 10) == 0)
                break;            /* Got EMSI_REQ                */
            if (strncmp(token, "{EMSI_DAT}", 10) == 0) {
                /* Remote jumped straight to DAT — parse it */
                goto got_remote_dat;
            }
        }
    }

    if (attempt >= EMSI_MAX_RETRIES) {
        qf_log(LOG_WARN, "Unable to establish initial handshake");
        return -1;
    }

    /* Send our EMSI_DAT */
    emsi_build_dat(our_data, dat_pkt, sizeof(dat_pkt));
    qf_log(LOG_DEBUG, "Sending EMSI packet");
    ser_write_str(sp, dat_pkt);
    qf_log(LOG_DEBUG, "Sent EMSI packet");

    /* Wait for remote's EMSI_DAT */
    for (attempt = 0; attempt < EMSI_MAX_RETRIES; attempt++) {
        if (emsi_wait_token(sp, token, sizeof(token), EMSI_TIMEOUT_MS) == 0) {
            if (strncmp(token, "{EMSI_DAT}", 10) == 0)
                break;
            if (strncmp(token, "**EMSI_NAK", 10) == 0) {
                qf_log(LOG_DEBUG, "Resending EMSI packet");
                ser_write_str(sp, dat_pkt);
                continue;
            }
        }
    }

    if (attempt >= EMSI_MAX_RETRIES) {
        qf_log(LOG_WARN, "Timeout receiving EMSI packet");
        return -1;
    }

got_remote_dat:
    /* Parse remote's EMSI_DAT */
    qf_log(LOG_DEBUG, "Receiving EMSI packet");
    if (emsi_parse_dat(token, (int)strlen(token), remote_data) != 0) {
        qf_log(LOG_WARN, "Bad CRC receiving EMSI packet");
        ser_write_str(sp, EMSI_NAK);
        return -1;
    }

    qf_log(LOG_DEBUG, "Received EMSI packet");

    /* Save received data */
    emsi_save_dat("EMSI-IN.DAT", remote_data);
    emsi_save_dat("EMSI-OUT.DAT", our_data);

    /* Send EMSI_ACK */
    ser_write_str(sp, EMSI_ACK);
    ser_write_str(sp, EMSI_ACK);  /* Send twice per FSC-0056     */

    {
        char buf[64];
        ftn_format_addr(&remote_data->addrs[0], buf, sizeof(buf));
        qf_log(LOG_INFO, "Established EMSI protocol with %s (%s)",
               remote_data->system_name, buf);
    }

    return 0;
}


/* ---- EMSI Handshake (Answerer) ----
 * We answered a call — remote initiates. */

int emsi_handshake_answer(SerPort *sp, const EmsiData *our_data,
                           EmsiData *remote_data)
{
    char token[EMSI_DAT_MAXLEN];
    char dat_pkt[EMSI_DAT_MAXLEN];
    int attempt;

    qf_log(LOG_INFO, "Incoming EMSI");

    /* Send EMSI_REQ to acknowledge their INQ */
    ser_write_str(sp, EMSI_REQ);

    /* Wait for remote's EMSI_DAT */
    for (attempt = 0; attempt < EMSI_MAX_RETRIES; attempt++) {
        if (emsi_wait_token(sp, token, sizeof(token), EMSI_TIMEOUT_MS) == 0) {
            if (strncmp(token, "{EMSI_DAT}", 10) == 0)
                break;
            if (strncmp(token, "**EMSI_INQ", 10) == 0) {
                /* Resend REQ */
                ser_write_str(sp, EMSI_REQ);
                continue;
            }
        }
    }

    if (attempt >= EMSI_MAX_RETRIES) {
        qf_log(LOG_WARN, "Timeout receiving EMSI packet");
        return -1;
    }

    /* Parse remote's EMSI_DAT */
    qf_log(LOG_DEBUG, "Receiving EMSI packet");
    if (emsi_parse_dat(token, (int)strlen(token), remote_data) != 0) {
        ser_write_str(sp, EMSI_NAK);
        return -1;
    }
    qf_log(LOG_DEBUG, "Received EMSI packet");

    emsi_save_dat("EMSI-IN.DAT", remote_data);

    /* Send our EMSI_DAT */
    emsi_build_dat(our_data, dat_pkt, sizeof(dat_pkt));
    qf_log(LOG_DEBUG, "Sending EMSI packet");
    ser_write_str(sp, dat_pkt);
    qf_log(LOG_DEBUG, "Sent EMSI packet");

    /* Wait for EMSI_ACK */
    for (attempt = 0; attempt < EMSI_MAX_RETRIES; attempt++) {
        if (emsi_wait_token(sp, token, sizeof(token), EMSI_TIMEOUT_MS) == 0) {
            if (strncmp(token, "**EMSI_ACK", 10) == 0)
                break;
            if (strncmp(token, "**EMSI_NAK", 10) == 0) {
                ser_write_str(sp, dat_pkt);
                continue;
            }
        }
    }

    emsi_save_dat("EMSI-OUT.DAT", our_data);

    {
        char buf[64];
        ftn_format_addr(&remote_data->addrs[0], buf, sizeof(buf));
        qf_log(LOG_INFO, "Established EMSI protocol with %s (%s)",
               remote_data->system_name, buf);
    }

    return 0;
}
