/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* emsi.c -- EMSI Handshake Protocol (FSC-0056)                             */
/*                                                                           */
/* Implements the EMSI (Electronic Mail Standard Identification)             */
/* handshake used to negotiate FidoNet sessions.                             */
/*                                                                           */
/* State machine:                                                            */
/*   Caller:  send EMSI_INQ -> wait EMSI_REQ -> send EMSI_DAT ->           */
/*            wait EMSI_DAT -> send EMSI_ACK -> session established          */
/*   Answer:  wait EMSI_INQ -> send EMSI_REQ -> wait EMSI_DAT ->           */
/*            send EMSI_DAT -> wait EMSI_ACK -> session established          */
/*                                                                           */
/* Tokens (from binary):                                                     */
/*   EMSI_INQ   **EMSI_INQC816     Inquiry                                 */
/*   EMSI_REQ   **EMSI_REQA77E     Request                                 */
/*   EMSI_DAT   {EMSI_DAT}{len}{data}{crc}  Data packet                    */
/*   EMSI_ACK   **EMSI_ACKA490     Acknowledge                             */
/*   EMSI_NAK   **EMSI_NAKE2B1     Negative ack                            */
/*   EMSI_HBT   **EMSI_HBTEBC3     Heartbeat                               */
/*                                                                           */
/* Clean-room from FSC-0056 (public FidoNet specification).                  */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "qfront.h"

/* Forward declarations -- serial.c */
typedef struct SerPort SerPort;
extern int  ser_read_byte(SerPort *sp, int timeout_ms);
extern int  ser_write(SerPort *sp, const void *buf, int len);
extern int  ser_write_str(SerPort *sp, const char *str);
extern int  ser_get_dcd(SerPort *sp);
extern int  ser_data_ready(SerPort *sp);
extern void ser_flush(SerPort *sp);


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                        EMSI Fixed Tokens                                  */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* These include the CRC-16 appended to the token.
 * FSC-0056 Section 3: tokens are sent as "**EMSI_XXXxxxx\r" */

#define EMSI_INQ    "**EMSI_INQC816\r"  /* inquiry                       */
#define EMSI_REQ    "**EMSI_REQA77E\r"  /* request                       */
#define EMSI_ACK    "**EMSI_ACKA490\r"  /* acknowledge                   */
#define EMSI_NAK    "**EMSI_NAKE2B1\r"  /* negative ack                  */
#define EMSI_HBT    "**EMSI_HBTEBC3\r"  /* heartbeat                     */

#define EMSI_MAX_RETRIES  6             /* max handshake attempts        */
#define EMSI_TIMEOUT_MS   20000         /* 20 second timeout             */
#define EMSI_DAT_MAXLEN   4096          /* max DAT packet size           */


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                       EMSI Session Data                                   */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef struct {
    FTN_ADDR Addrs[16];                 /* AKA list                      */
    int      NumAddrs;                  /* number of AKAs                */
    char     Sysop[64];                 /* sysop name                    */
    char     SystemName[64];            /* system/BBS name               */
    char     Location[64];              /* city, state                   */
    char     Phone[40];                 /* phone number                  */
    char     Password[32];              /* session password              */
    char     Mailer[32];                /* mailer name + version         */
    uint32_t LinkCodes;                 /* capability flags              */
    uint32_t CompCodes;                 /* compression support           */
} EmsiData;

/* Link codes (FSC-0056 Section 7) */
#define EMSI_LC_CALLERID  0x0001        /* caller pays                   */
#define EMSI_LC_PUA       0x0002        /* pickup all                    */
#define EMSI_LC_PUP       0x0004        /* pickup primary address only   */
#define EMSI_LC_NPU       0x0008        /* no pickup                     */
#define EMSI_LC_HAT       0x0010        /* hold all traffic              */
#define EMSI_LC_HXT       0x0020        /* hold compressed mail only     */
#define EMSI_LC_HRQ       0x0040        /* hold file requests            */


/*-----------------------------------------------------------------------*/
/* emsi_crc16() -- CRC-16/CCITT                                         */
/*                                                                       */
/* FSC-0056 uses CRC-16/CCITT (polynomial 0x1021, initial value 0).     */
/*-----------------------------------------------------------------------*/

static uint16_t emsi_crc16(const void *Data, int Len)
{
    const unsigned char *p = (const unsigned char *)Data;
    uint16_t Crc = 0;                   /* running CRC value             */
    int i, j;                           /* byte and bit loop indices     */

    for (i = 0; i < Len; i++) {
        Crc ^= (uint16_t)p[i] << 8;
        for (j = 0; j < 8; j++) {
            if (Crc & 0x8000)
                Crc = (Crc << 1) ^ 0x1021;
            else
                Crc <<= 1;
        }
    }

    return Crc;
}


/*-----------------------------------------------------------------------*/
/* emsi_build_dat() -- Build an EMSI_DAT packet                         */
/*                                                                       */
/* FSC-0056 Section 5:                                                   */
/*   {EMSI_DAT}{<hex_len>}{<data>}{<CRC16>}                             */
/*                                                                       */
/* Data is a series of brace-delimited fields:                           */
/*   {system_name}{location}{sysop}{phone}{baud}{mailer}                 */
/*   {<addr_list>}{password}{link_codes}{comp_codes}                     */
/*                                                                       */
/* Returns length of packet written to Buf.                              */
/*-----------------------------------------------------------------------*/

static int emsi_build_dat(const EmsiData *Data, char *Buf, int BufSize)
{
    char     Payload[2048];             /* packet payload                */
    char     Addrs[256];                /* formatted address list        */
    uint16_t Crc;                       /* payload CRC                   */
    int      PLen;                      /* payload length                */
    int      i;                         /* address loop index            */

    /* Build address list */
    Addrs[0] = '\0';
    {
        int APos = 0;                   /* address string position       */

        for (i = 0; i < Data->NumAddrs; i++) {
            char AddrBuf[64];           /* single formatted address      */

            ftn_format_addr(&Data->Addrs[i], AddrBuf, sizeof(AddrBuf));
            if (APos > 0 && APos < 250)
                APos += snprintf(Addrs + APos, sizeof(Addrs) - APos, " ");
            if (APos < 250)
                APos += snprintf(Addrs + APos, sizeof(Addrs) - APos,
                                 "%s", AddrBuf);
        }
    }

    /* Build payload: {field}{field}... */
    PLen = snprintf(Payload, sizeof(Payload),
        "{EMSI}{%s}{%s}{%s}{%s}{%s}{8N1,PC}{%s}{%s}{%s}{}",
        Data->SystemName,
        Data->Location,
        Data->Sysop,
        Data->Phone[0] ? Data->Phone : "-Unpublished-",
        "115200",                       /* speed -- always report max    */
        Data->Mailer,
        Addrs,
        Data->Password);

    /* Calculate CRC over the payload */
    Crc = emsi_crc16(Payload, PLen);

    /* Build complete packet: {EMSI_DAT}{hex_len}{payload}{hex_crc} */
    return snprintf(Buf, BufSize, "{EMSI_DAT}%04X%s%04X\r",
                    PLen, Payload, Crc);
}


/*-----------------------------------------------------------------------*/
/* emsi_parse_dat() -- Parse an EMSI_DAT packet                         */
/*                                                                       */
/* Extracts brace-delimited fields from the packet. Field positions      */
/* are defined by FSC-0056 Section 5.                                    */
/*                                                                       */
/* Returns 0 on success (at least one address parsed), -1 on error.     */
/*-----------------------------------------------------------------------*/

static int emsi_parse_dat(const char *Pkt, int PktLen, EmsiData *Data)
{
    const char *p = Pkt;                /* packet scan pointer           */
    char        Field[256];             /* current field buffer          */
    int         Fi;                     /* field character index          */
    int         FieldNum;               /* field position counter        */

    (void)PktLen;
    memset(Data, 0, sizeof(*Data));

    /* Skip {EMSI_DAT} header and hex length */
    if (strncmp(p, "{EMSI_DAT}", 10) == 0)
        p += 10;

    /* Skip 4-digit hex length */
    if (strlen(p) >= 4)
        p += 4;

    /* Parse brace-delimited fields */
    FieldNum = 0;
    while (*p && FieldNum < 20) {
        if (*p != '{') { p++; continue; }
        p++;                            /* skip opening brace            */

        /* Read field content */
        Fi = 0;
        while (*p && *p != '}' && Fi < 255)
            Field[Fi++] = *p++;
        Field[Fi] = '\0';

        if (*p == '}') p++;             /* skip closing brace            */

        /* Map fields by position (FSC-0056 Section 5) */
        switch (FieldNum) {
        case 0:                         /* "EMSI" tag -- skip            */
            break;
        case 1:                         /* system name                   */
            strncpy(Data->SystemName, Field, 63);
            break;
        case 2:                         /* location                      */
            strncpy(Data->Location, Field, 63);
            break;
        case 3:                         /* sysop                         */
            strncpy(Data->Sysop, Field, 63);
            break;
        case 4:                         /* phone                         */
            strncpy(Data->Phone, Field, 39);
            break;
        case 5:                         /* speed -- ignore               */
            break;
        case 6:                         /* flags -- ignore               */
            break;
        case 7:                         /* mailer                        */
            strncpy(Data->Mailer, Field, 31);
            break;
        case 8:                         /* address list                  */
            {
                char *Tok = strtok(Field, " ");
                while (Tok && Data->NumAddrs < 16) {
                    ftn_parse_addr(Tok, &Data->Addrs[Data->NumAddrs++]);
                    Tok = strtok(NULL, " ");
                }
            }
            break;
        case 9:                         /* password                      */
            strncpy(Data->Password, Field, 31);
            break;
        }

        FieldNum++;
    }

    return (Data->NumAddrs > 0) ? 0 : -1;
}


/*-----------------------------------------------------------------------*/
/* emsi_wait_token() -- Wait for an EMSI token on the serial port       */
/*                                                                       */
/* Reads bytes looking for "**EMSI_XXX" tokens or "{EMSI_DAT}" packets. */
/*                                                                       */
/* Returns 0 on success (token read into Token buffer),                  */
/*        -1 on timeout, -2 on carrier loss.                             */
/*-----------------------------------------------------------------------*/

static int emsi_wait_token(SerPort *Sp, char *Token, int TokSize,
                            int TimeoutMs)
{
    int Pos = 0;                        /* token buffer position         */
    int Ch;                             /* received character            */
    int StarCount = 0;                  /* consecutive '*' count         */

    Token[0] = '\0';

    while (1) {
        Ch = ser_read_byte(Sp, TimeoutMs);
        if (Ch < 0) return -1;          /* timeout                       */

        /* Look for ** prefix */
        if (Ch == '*') {
            StarCount++;
            if (StarCount == 2) {
                /* Found ** -- read rest of token until CR */
                Token[0] = '*';
                Token[1] = '*';
                Pos = 2;
                while (Pos < TokSize - 1) {
                    Ch = ser_read_byte(Sp, 2000);
                    if (Ch < 0 || Ch == '\r' || Ch == '\n') break;
                    Token[Pos++] = (char)Ch;
                }
                Token[Pos] = '\0';
                return 0;
            }
        } else if (Ch == '{') {
            /* EMSI_DAT packet starts with { */
            Token[0] = '{';
            Pos = 1;
            while (Pos < TokSize - 1) {
                Ch = ser_read_byte(Sp, 2000);
                if (Ch < 0 || Ch == '\r') break;
                Token[Pos++] = (char)Ch;
            }
            Token[Pos] = '\0';
            return 0;
        } else {
            StarCount = 0;
        }

        /* Check carrier */
        if (!ser_get_dcd(Sp)) {
            qf_log(LOG_WARN, "Lost carrier during EMSI handshake");
            return -2;
        }
    }
}


/*-----------------------------------------------------------------------*/
/* emsi_save_dat() -- Save EMSI session data to a text file             */
/*                                                                       */
/* Writes system info and AKA list to EMSI-IN.DAT or EMSI-OUT.DAT      */
/* for diagnostic purposes.                                              */
/*-----------------------------------------------------------------------*/

static void emsi_save_dat(const char *Filename, const EmsiData *Data)
{
    FILE *f;                            /* output file handle            */
    int   i;                            /* AKA loop index                */
    char  AddrBuf[64];                  /* formatted address             */

    f = fopen(Filename, "w");
    if (!f) return;

    fprintf(f, "System: %s\n", Data->SystemName);
    fprintf(f, "Sysop:  %s\n", Data->Sysop);
    fprintf(f, "Location: %s\n", Data->Location);
    fprintf(f, "Phone:  %s\n", Data->Phone);
    fprintf(f, "Mailer: %s\n", Data->Mailer);
    fprintf(f, "Password: %s\n", Data->Password);

    for (i = 0; i < Data->NumAddrs; i++) {
        ftn_format_addr(&Data->Addrs[i], AddrBuf, sizeof(AddrBuf));
        fprintf(f, "AKA %d: %s\n", i, AddrBuf);
    }

    fclose(f);
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                   EMSI Handshake (Caller Side)                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* emsi_handshake_caller() -- Initiate EMSI handshake as caller         */
/*                                                                       */
/* We are calling out -- we initiate with EMSI_INQ, wait for EMSI_REQ,  */
/* exchange EMSI_DAT packets, and confirm with EMSI_ACK.                */
/*                                                                       */
/* Returns 0 on success, -1 on failure.                                  */
/*-----------------------------------------------------------------------*/

int emsi_handshake_caller(SerPort *Sp, const EmsiData *OurData,
                           EmsiData *RemoteData)
{
    char Token[EMSI_DAT_MAXLEN];        /* received token buffer         */
    char DatPkt[EMSI_DAT_MAXLEN];       /* our EMSI_DAT packet           */
    int  Attempt;                       /* retry counter                 */

    qf_log(LOG_INFO, "Establishing FidoMail handshake");

    /* Send EMSI_INQ repeatedly until we get EMSI_REQ */
    for (Attempt = 0; Attempt < EMSI_MAX_RETRIES; Attempt++) {
        qf_log(LOG_DEBUG, "Sending EMSI_INQ (attempt %d)", Attempt + 1);
        ser_write_str(Sp, EMSI_INQ);

        if (emsi_wait_token(Sp, Token, sizeof(Token),
                             EMSI_TIMEOUT_MS) == 0) {
            if (strncmp(Token, "**EMSI_REQ", 10) == 0)
                break;                  /* got EMSI_REQ                  */
            if (strncmp(Token, "{EMSI_DAT}", 10) == 0)
                goto got_remote_dat;    /* remote jumped to DAT          */
        }
    }

    if (Attempt >= EMSI_MAX_RETRIES) {
        qf_log(LOG_WARN, "Unable to establish initial handshake");
        return -1;
    }

    /* Send our EMSI_DAT */
    emsi_build_dat(OurData, DatPkt, sizeof(DatPkt));
    qf_log(LOG_DEBUG, "Sending EMSI packet");
    ser_write_str(Sp, DatPkt);
    qf_log(LOG_DEBUG, "Sent EMSI packet");

    /* Wait for remote's EMSI_DAT */
    for (Attempt = 0; Attempt < EMSI_MAX_RETRIES; Attempt++) {
        if (emsi_wait_token(Sp, Token, sizeof(Token),
                             EMSI_TIMEOUT_MS) == 0) {
            if (strncmp(Token, "{EMSI_DAT}", 10) == 0)
                break;
            if (strncmp(Token, "**EMSI_NAK", 10) == 0) {
                qf_log(LOG_DEBUG, "Resending EMSI packet");
                ser_write_str(Sp, DatPkt);
                continue;
            }
        }
    }

    if (Attempt >= EMSI_MAX_RETRIES) {
        qf_log(LOG_WARN, "Timeout receiving EMSI packet");
        return -1;
    }

got_remote_dat:
    /* Parse remote's EMSI_DAT */
    qf_log(LOG_DEBUG, "Receiving EMSI packet");
    if (emsi_parse_dat(Token, (int)strlen(Token), RemoteData) != 0) {
        qf_log(LOG_WARN, "Bad CRC receiving EMSI packet");
        ser_write_str(Sp, EMSI_NAK);
        return -1;
    }
    qf_log(LOG_DEBUG, "Received EMSI packet");

    /* Save received data for diagnostics */
    emsi_save_dat("EMSI-IN.DAT", RemoteData);
    emsi_save_dat("EMSI-OUT.DAT", OurData);

    /* Send EMSI_ACK (twice per FSC-0056) */
    ser_write_str(Sp, EMSI_ACK);
    ser_write_str(Sp, EMSI_ACK);

    {
        char AddrBuf[64];               /* formatted address for log     */

        ftn_format_addr(&RemoteData->Addrs[0], AddrBuf, sizeof(AddrBuf));
        qf_log(LOG_INFO, "Established EMSI protocol with %s (%s)",
               RemoteData->SystemName, AddrBuf);
    }

    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                   EMSI Handshake (Answerer Side)                          */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* emsi_handshake_answer() -- Accept EMSI handshake as answerer         */
/*                                                                       */
/* We answered a call -- remote initiates with EMSI_INQ. We respond     */
/* with EMSI_REQ, exchange EMSI_DAT packets, wait for EMSI_ACK.        */
/*                                                                       */
/* Returns 0 on success, -1 on failure.                                  */
/*-----------------------------------------------------------------------*/

int emsi_handshake_answer(SerPort *Sp, const EmsiData *OurData,
                           EmsiData *RemoteData)
{
    char Token[EMSI_DAT_MAXLEN];        /* received token buffer         */
    char DatPkt[EMSI_DAT_MAXLEN];       /* our EMSI_DAT packet           */
    int  Attempt;                       /* retry counter                 */

    qf_log(LOG_INFO, "Incoming EMSI");

    /* Send EMSI_REQ to acknowledge their INQ */
    ser_write_str(Sp, EMSI_REQ);

    /* Wait for remote's EMSI_DAT */
    for (Attempt = 0; Attempt < EMSI_MAX_RETRIES; Attempt++) {
        if (emsi_wait_token(Sp, Token, sizeof(Token),
                             EMSI_TIMEOUT_MS) == 0) {
            if (strncmp(Token, "{EMSI_DAT}", 10) == 0)
                break;
            if (strncmp(Token, "**EMSI_INQ", 10) == 0) {
                /* Resend REQ */
                ser_write_str(Sp, EMSI_REQ);
                continue;
            }
        }
    }

    if (Attempt >= EMSI_MAX_RETRIES) {
        qf_log(LOG_WARN, "Timeout receiving EMSI packet");
        return -1;
    }

    /* Parse remote's EMSI_DAT */
    qf_log(LOG_DEBUG, "Receiving EMSI packet");
    if (emsi_parse_dat(Token, (int)strlen(Token), RemoteData) != 0) {
        ser_write_str(Sp, EMSI_NAK);
        return -1;
    }
    qf_log(LOG_DEBUG, "Received EMSI packet");

    emsi_save_dat("EMSI-IN.DAT", RemoteData);

    /* Send our EMSI_DAT */
    emsi_build_dat(OurData, DatPkt, sizeof(DatPkt));
    qf_log(LOG_DEBUG, "Sending EMSI packet");
    ser_write_str(Sp, DatPkt);
    qf_log(LOG_DEBUG, "Sent EMSI packet");

    /* Wait for EMSI_ACK */
    for (Attempt = 0; Attempt < EMSI_MAX_RETRIES; Attempt++) {
        if (emsi_wait_token(Sp, Token, sizeof(Token),
                             EMSI_TIMEOUT_MS) == 0) {
            if (strncmp(Token, "**EMSI_ACK", 10) == 0)
                break;
            if (strncmp(Token, "**EMSI_NAK", 10) == 0) {
                ser_write_str(Sp, DatPkt);
                continue;
            }
        }
    }

    emsi_save_dat("EMSI-OUT.DAT", OurData);

    {
        char AddrBuf[64];               /* formatted address for log     */

        ftn_format_addr(&RemoteData->Addrs[0], AddrBuf, sizeof(AddrBuf));
        qf_log(LOG_INFO, "Established EMSI protocol with %s (%s)",
               RemoteData->SystemName, AddrBuf);
    }

    return 0;
}
