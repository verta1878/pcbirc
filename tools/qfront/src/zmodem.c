/* ====================================================================
 * zmodem.c — Zmodem File Transfer Protocol
 * ====================================================================
 * Implements Zmodem send and receive with:
 *   - 8K block size ("Using Zmodem 8k block size")
 *   - CRC-16 and CRC-32
 *   - 4 subpacket types (CrcE, CrcG, CrcQ, CrcW)
 *   - Resume support ("Attempting resume")
 *   - Skip support ("Zmodem - skip file")
 *   - Progress display (CPS, blocks, bytes remaining)
 *
 * From binary:
 *   "Zmodem - bad file position"
 *   "Zmodem - specified file does not exist"
 *   "Zmodem - not allowed to overwrite file"
 *   "Zmodem - never got proper handshake"
 *   "Zmodem - got CrcE/CrcG/CrcQ/CrcW DataSubpacket"
 *   "Zmodem - got garbage from remote"
 *   "Zmodem - no files to receive"
 *   "Zmodem - skip file"
 *
 * Clean-room from public Zmodem specification (Chuck Forsberg, 1986).
 * ==================================================================== */

#include "qfront.h"

/* Forward declarations — serial.c */
typedef struct SerPort SerPort;
extern int  ser_read_byte(SerPort *sp, int timeout_ms);
extern int  ser_write(SerPort *sp, const void *buf, int len);
extern int  ser_get_dcd(SerPort *sp);
extern int  ser_data_ready(SerPort *sp);
extern void ser_flush(SerPort *sp);

/* ---- Zmodem Constants ---- */

#define ZPAD        '*'           /* Padding character             */
#define ZDLE        0x18          /* Escape character (CAN)        */
#define ZDLEE       0x58          /* Escaped ZDLE                  */
#define ZBIN        'A'           /* Binary header (CRC-16)        */
#define ZHEX        'B'           /* Hex header (CRC-16)           */
#define ZBIN32      'C'           /* Binary header (CRC-32)        */

/* Frame types */
#define ZRQINIT     0             /* Request receiver init          */
#define ZRINIT      1             /* Receiver init                  */
#define ZSINIT      2             /* Sender init (send setup)       */
#define ZACK        3             /* Acknowledge                    */
#define ZFILE       4             /* File header (name, size, etc.) */
#define ZSKIP       5             /* Skip this file                 */
#define ZNAK        6             /* Negative acknowledge           */
#define ZABORT      7             /* Abort session                  */
#define ZFIN        8             /* Finish session                 */
#define ZRPOS       9             /* Resume position                */
#define ZDATA       10            /* Data packet follows            */
#define ZEOF        11            /* End of file                    */
#define ZFERR       12            /* File error (read/write)        */
#define ZCRC        13            /* CRC request                    */
#define ZCHALLENGE  14            /* Challenge                      */
#define ZCOMPL      15            /* Complete                       */
#define ZCAN        16            /* Cancel (5 CAN chars)           */
#define ZFREECNT    17            /* Free disk space request        */
#define ZCOMMAND    18            /* Command                        */

/* Subpacket types */
#define ZCRCW       'k'           /* CRC + Wait (expect ZACK)       */
#define ZCRCG       'h'           /* CRC + Go (streaming, no ack)   */
#define ZCRCQ       'j'           /* CRC + Query (expect ZACK)      */
#define ZCRCE       'i'           /* CRC + End (last subpacket)     */

/* ZRINIT capability flags */
#define CANFDX      0x01          /* Full duplex                    */
#define CANOVIO     0x02          /* Overlapped I/O                 */
#define CANBRK      0x04          /* Break signal                   */
#define CANCRY      0x08          /* Encryption                     */
#define CANLZW      0x10          /* LZW compression                */
#define CANFC32     0x20          /* CRC-32 capable                 */
#define ESCCTL      0x40          /* Escape control chars            */
#define ESC8        0x80          /* Escape 8-bit chars              */

#define ZM_BLOCK_SIZE  8192       /* 8K blocks                      */
#define ZM_TIMEOUT     10000      /* 10 second timeout               */
#define ZM_MAX_ERRORS  10         /* Max errors before abort         */
#define ZM_CAN_COUNT   5          /* 5 CANs to abort                 */


/* ---- CRC-32 Table ---- */

static uint32_t zm_crc32_table[256];
static int      zm_crc32_init = 0;

static void zm_build_crc32(void)
{
    uint32_t crc;
    int i, j;

    for (i = 0; i < 256; i++) {
        crc = (uint32_t)i;
        for (j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
        zm_crc32_table[i] = crc;
    }
    zm_crc32_init = 1;
}

static uint32_t zm_crc32(const void *data, int len)
{
    const unsigned char *p = (const unsigned char *)data;
    uint32_t crc = 0xFFFFFFFF;
    int i;

    if (!zm_crc32_init) zm_build_crc32();

    for (i = 0; i < len; i++)
        crc = zm_crc32_table[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);

    return crc ^ 0xFFFFFFFF;
}

static uint32_t zm_update_crc32(uint32_t crc, unsigned char byte)
{
    if (!zm_crc32_init) zm_build_crc32();
    return zm_crc32_table[(crc ^ byte) & 0xFF] ^ (crc >> 8);
}


/* ---- CRC-16 (CCITT) ---- */

static uint16_t zm_crc16(const void *data, int len)
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


/* ---- Zmodem Header ---- */

typedef struct {
    int      type;                /* Frame type (ZRQINIT, etc.)    */
    uint32_t pos;                 /* Position / flags (4 bytes)    */
    int      use_crc32;           /* CRC-32 mode?                  */
} ZmHeader;


/* ---- ZDLE Encoding/Decoding ----
 * Zmodem escapes certain bytes with ZDLE prefix. */

static int zm_is_escape(unsigned char c)
{
    /* Escape: DLE(0x10), XON(0x11), XOFF(0x13), ZDLE(0x18),
     * DLE|0x80, XON|0x80, XOFF|0x80, 0xFF, CR after @ */
    return (c == 0x10 || c == 0x11 || c == 0x13 || c == 0x18 ||
            c == 0x90 || c == 0x91 || c == 0x93 || c == 0xFF);
}

static int zm_send_escaped(SerPort *sp, const void *data, int len)
{
    const unsigned char *p = (const unsigned char *)data;
    unsigned char buf[2];
    int i;

    for (i = 0; i < len; i++) {
        if (zm_is_escape(p[i])) {
            buf[0] = ZDLE;
            buf[1] = p[i] ^ 0x40;
            ser_write(sp, buf, 2);
        } else {
            ser_write(sp, &p[i], 1);
        }
    }
    return len;
}


/* ---- Read ZDLE-decoded byte ---- */

static int zm_read_byte(SerPort *sp, int timeout_ms)
{
    int ch = ser_read_byte(sp, timeout_ms);
    if (ch < 0) return -1;

    if (ch == ZDLE) {
        ch = ser_read_byte(sp, timeout_ms);
        if (ch < 0) return -1;
        if (ch == ZDLEE) return ZDLE;
        return ch ^ 0x40;
    }
    return ch;
}


/* ---- Send Hex Header ---- */

static void zm_send_hex_header(SerPort *sp, const ZmHeader *hdr)
{
    unsigned char raw[5];
    uint16_t crc;
    char hex[32];
    int i;

    raw[0] = (unsigned char)hdr->type;
    raw[1] = (unsigned char)(hdr->pos & 0xFF);
    raw[2] = (unsigned char)((hdr->pos >> 8) & 0xFF);
    raw[3] = (unsigned char)((hdr->pos >> 16) & 0xFF);
    raw[4] = (unsigned char)((hdr->pos >> 24) & 0xFF);

    crc = zm_crc16(raw, 5);

    /* Format: ZPAD ZPAD ZDLE ZHEX type[2] p0[2] p1[2] p2[2] p3[2] crc[4] CR LF */
    i = 0;
    hex[i++] = ZPAD;
    hex[i++] = ZPAD;
    hex[i++] = ZDLE;
    hex[i++] = ZHEX;

    /* Hex-encode the 5 data bytes + 2 CRC bytes */
    {
        int j;
        unsigned char bytes[7];
        memcpy(bytes, raw, 5);
        bytes[5] = (unsigned char)(crc >> 8);
        bytes[6] = (unsigned char)(crc & 0xFF);

        for (j = 0; j < 7; j++) {
            hex[i++] = "0123456789abcdef"[(bytes[j] >> 4) & 0xF];
            hex[i++] = "0123456789abcdef"[bytes[j] & 0xF];
        }
    }

    hex[i++] = '\r';
    hex[i++] = '\n';

    ser_write(sp, hex, i);
}


/* ---- Send Binary Header (CRC-32) ---- */

static void zm_send_bin32_header(SerPort *sp, const ZmHeader *hdr)
{
    unsigned char preamble[4] = { ZPAD, ZPAD, ZDLE, ZBIN32 };
    unsigned char raw[5];
    uint32_t crc;

    raw[0] = (unsigned char)hdr->type;
    raw[1] = (unsigned char)(hdr->pos & 0xFF);
    raw[2] = (unsigned char)((hdr->pos >> 8) & 0xFF);
    raw[3] = (unsigned char)((hdr->pos >> 16) & 0xFF);
    raw[4] = (unsigned char)((hdr->pos >> 24) & 0xFF);

    crc = zm_crc32(raw, 5);

    ser_write(sp, preamble, 4);
    zm_send_escaped(sp, raw, 5);

    {
        unsigned char crc_bytes[4];
        crc_bytes[0] = (unsigned char)(crc & 0xFF);
        crc_bytes[1] = (unsigned char)((crc >> 8) & 0xFF);
        crc_bytes[2] = (unsigned char)((crc >> 16) & 0xFF);
        crc_bytes[3] = (unsigned char)((crc >> 24) & 0xFF);
        zm_send_escaped(sp, crc_bytes, 4);
    }
}


/* ---- Receive Header ---- */

static int zm_recv_header(SerPort *sp, ZmHeader *hdr, int timeout_ms)
{
    int ch;
    unsigned char raw[5];
    int i;

    memset(hdr, 0, sizeof(*hdr));

    /* Wait for ZPAD ZPAD ZDLE */
    while (1) {
        ch = ser_read_byte(sp, timeout_ms);
        if (ch < 0) return -1;
        if (ch != ZPAD) continue;

        ch = ser_read_byte(sp, timeout_ms);
        if (ch < 0) return -1;
        if (ch == ZPAD) {
            ch = ser_read_byte(sp, timeout_ms);
            if (ch < 0) return -1;
            if (ch == ZDLE) break;
        }
    }

    /* Read encoding type */
    ch = ser_read_byte(sp, timeout_ms);
    if (ch < 0) return -1;

    if (ch == ZHEX) {
        /* Hex header — read 14 hex chars (7 bytes) */
        char hexbuf[16];
        for (i = 0; i < 14; i++) {
            ch = ser_read_byte(sp, timeout_ms);
            if (ch < 0) return -1;
            hexbuf[i] = (char)ch;
        }
        hexbuf[14] = '\0';

        /* Parse hex */
        for (i = 0; i < 5; i++) {
            unsigned int val;
            sscanf(hexbuf + i * 2, "%2x", &val);
            raw[i] = (unsigned char)val;
        }
        hdr->use_crc32 = 0;

        /* Consume trailing CR LF */
        ser_read_byte(sp, 100);
        ser_read_byte(sp, 100);

    } else if (ch == ZBIN || ch == ZBIN32) {
        hdr->use_crc32 = (ch == ZBIN32);

        /* Binary header — read 5 ZDLE-decoded bytes */
        for (i = 0; i < 5; i++) {
            ch = zm_read_byte(sp, timeout_ms);
            if (ch < 0) return -1;
            raw[i] = (unsigned char)ch;
        }

        /* Read and verify CRC */
        if (hdr->use_crc32) {
            for (i = 0; i < 4; i++)
                zm_read_byte(sp, timeout_ms);  /* CRC-32 bytes */
        } else {
            zm_read_byte(sp, timeout_ms);      /* CRC-16 high */
            zm_read_byte(sp, timeout_ms);      /* CRC-16 low */
        }
    } else {
        qf_log(LOG_DEBUG, "Zmodem - got garbage from remote");
        return -1;
    }

    hdr->type = raw[0];
    hdr->pos  = raw[1] | ((uint32_t)raw[2] << 8) |
                ((uint32_t)raw[3] << 16) | ((uint32_t)raw[4] << 24);

    return 0;
}


/* ---- Send Data Subpacket ---- */

static void zm_send_data(SerPort *sp, const void *data, int len,
                          int subtype)
{
    unsigned char end[2];
    uint32_t crc;

    zm_send_escaped(sp, data, len);

    /* Subpacket terminator: ZDLE + subtype */
    end[0] = ZDLE;
    end[1] = (unsigned char)subtype;
    ser_write(sp, end, 2);

    /* CRC-32 over data + subtype byte */
    crc = zm_crc32(data, len);
    crc = zm_update_crc32(crc ^ 0xFFFFFFFF, (unsigned char)subtype)
          ^ 0xFFFFFFFF;

    {
        unsigned char crc_bytes[4];
        crc_bytes[0] = (unsigned char)(crc & 0xFF);
        crc_bytes[1] = (unsigned char)((crc >> 8) & 0xFF);
        crc_bytes[2] = (unsigned char)((crc >> 16) & 0xFF);
        crc_bytes[3] = (unsigned char)((crc >> 24) & 0xFF);
        zm_send_escaped(sp, crc_bytes, 4);
    }
}


/* ---- Send Cancel (5 CANs + 5 Backspaces) ---- */

static void zm_send_cancel(SerPort *sp)
{
    unsigned char cancel[10];
    int i;
    for (i = 0; i < 5; i++) cancel[i] = 0x18;      /* CAN */
    for (i = 5; i < 10; i++) cancel[i] = 0x08;      /* BS */
    ser_write(sp, cancel, 10);
}


/* ---- Send File via Zmodem ---- */

/*-----------------------------------------------------------------------*/
/* zm_send_file() — Send a single file via Zmodem batch protocol         */
/*                                                                         */
/* Implements the Zmodem sender state machine:                           */
/*   1. Send ZRQINIT (request receiver to initialize)                    */
/*   2. Wait for ZRINIT from receiver                                    */
/*   3. Send ZFILE header with filename, size, timestamp                 */
/*   4. Wait for ZRPOS (receiver's position — 0 for new, >0 for resume) */
/*   5. Send file data in ZCRCG subpackets (streaming, no ack per block) */
/*   6. Send ZEOF when all data sent                                     */
/*   7. Wait for ZRINIT (receiver ready for next file)                   */
/*                                                                         */
/* Block size starts at 8K ("Using Zmodem 8k block size" from binary)    */
/* and may be reduced on error for noisy lines. CRC-32 is used for      */
/* data integrity.                                                        */
/*                                                                         */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

int zm_send_file(SerPort *sp, const char *filepath)
{
    qf_log(LOG_DEBUG, "zm_send_file: %s", filepath);
    FILE *f;
    ZmHeader hdr;
    unsigned char block[ZM_BLOCK_SIZE];
    char fileinfo[1024];
    long file_size, sent = 0;
    int n, errors = 0;
    time_t start_time;
    const char *filename;

    /* Extract filename from path */
    filename = strrchr(filepath, PATH_SEP);
    filename = filename ? filename + 1 : filepath;

    f = fopen(filepath, "rb");
    if (!f) {
        qf_log(LOG_WARN, "Zmodem - specified file does not exist");
        return -1;
    }

    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    qf_log(LOG_INFO, "Protocol      : Zmodem");
    qf_log(LOG_INFO, "Using Zmodem 8k block size");

    start_time = time(NULL);

    /* Send ZRQINIT */
    memset(&hdr, 0, sizeof(hdr));
    hdr.type = ZRQINIT;
    zm_send_hex_header(sp, &hdr);

    /* Wait for ZRINIT */
    if (zm_recv_header(sp, &hdr, ZM_TIMEOUT) != 0 ||
        hdr.type != ZRINIT) {
        qf_log(LOG_WARN, "Zmodem - never got proper handshake");
        fclose(f);
        return -1;
    }

    /* Send ZFILE header */
    hdr.type = ZFILE;
    hdr.pos = 0;
    zm_send_bin32_header(sp, &hdr);

    /* Send file info subpacket: "name\0size date\0" */
    n = snprintf(fileinfo, sizeof(fileinfo), "%s", filename);
    n++;  /* Include null terminator */
    n += snprintf(fileinfo + n, sizeof(fileinfo) - n, "%ld", file_size);
    zm_send_data(sp, fileinfo, n + 1, ZCRCW);

    /* Wait for ZRPOS (resume) or ZSKIP */
    if (zm_recv_header(sp, &hdr, ZM_TIMEOUT) != 0) {
        fclose(f);
        return -1;
    }

    if (hdr.type == ZSKIP) {
        qf_log(LOG_INFO, "Zmodem - skip file");
        fclose(f);
        return 0;
    }

    if (hdr.type == ZRPOS) {
        /* Resume from position */
        if (hdr.pos > 0) {
            qf_log(LOG_INFO, "Attempting resume at offset %lu",
                   (unsigned long)hdr.pos);
            fseek(f, (long)hdr.pos, SEEK_SET);
            sent = (long)hdr.pos;
        }
    }

    /* Send ZDATA header */
    hdr.type = ZDATA;
    hdr.pos = (uint32_t)sent;
    zm_send_bin32_header(sp, &hdr);

    /* Send file data in blocks */
    while ((n = (int)fread(block, 1, ZM_BLOCK_SIZE, f)) > 0) {
        int subtype;

        /* Use CrcG (streaming) for middle blocks, CrcE for last */
        if (feof(f))
            subtype = ZCRCE;       /* Last block                  */
        else
            subtype = ZCRCG;       /* Streaming — no ack needed   */

        zm_send_data(sp, block, n, subtype);
        sent += n;

        /* Progress display */
        {
            time_t elapsed = time(NULL) - start_time;
            long cps = (elapsed > 0) ? sent / elapsed : 0;
            long remain = file_size - sent;
            qf_log(LOG_DEBUG, "Bytes %ld/%ld  %ld CPS  %ld remaining",
                   sent, file_size, cps, remain);
        }

        /* Check for ZACK/ZNAK if using CrcQ/CrcW */
        if (subtype == ZCRCW || subtype == ZCRCQ) {
            if (zm_recv_header(sp, &hdr, ZM_TIMEOUT) != 0) {
                errors++;
                if (errors >= ZM_MAX_ERRORS) {
                    qf_log(LOG_WARN,
                           "Maximum protocol error count reached");
                    break;
                }
            }

            /* Handle ZRPOS — resend from requested position */
            if (hdr.type == ZRPOS) {
                qf_log(LOG_DEBUG, "Zmodem - bad file position");
                fseek(f, (long)hdr.pos, SEEK_SET);
                sent = (long)hdr.pos;

                hdr.type = ZDATA;
                hdr.pos = (uint32_t)sent;
                zm_send_bin32_header(sp, &hdr);
            }
        }

        /* Check carrier */
        if (!ser_get_dcd(sp)) {
            qf_log(LOG_WARN, "Lost carrier");
            break;
        }
    }

    fclose(f);

    /* Send ZEOF */
    hdr.type = ZEOF;
    hdr.pos = (uint32_t)sent;
    zm_send_hex_header(sp, &hdr);

    /* Wait for ZRINIT (ready for next file) */
    zm_recv_header(sp, &hdr, ZM_TIMEOUT);

    {
        time_t elapsed = time(NULL) - start_time;
        long cps = (elapsed > 0) ? sent / elapsed : 0;
        qf_log(LOG_INFO, "Successfully sent %s (%ld bytes, %ld CPS)",
               filename, sent, cps);
    }

    return 0;
}


/* ---- Send ZFIN (End Session) ---- */

int zm_send_zfin(SerPort *sp)
{
    ZmHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.type = ZFIN;
    zm_send_hex_header(sp, &hdr);

    /* Wait for ZFIN from remote */
    if (zm_recv_header(sp, &hdr, ZM_TIMEOUT) == 0 &&
        hdr.type == ZFIN) {
        /* Send OO (Over and Out) */
        ser_write(sp, "OO", 2);
    }

    qf_log(LOG_INFO, "End of transmitted file");
    return 0;
}


/* ---- Receive File via Zmodem ---- */

/*-----------------------------------------------------------------------*/
/* zm_recv_file() — Receive a single file via Zmodem batch protocol      */
/*                                                                         */
/* Implements the Zmodem receiver state machine:                         */
/*   1. Send ZRINIT (we're ready to receive)                             */
/*   2. Wait for ZFILE header (contains filename, size, timestamp)       */
/*   3. Send ZRPOS with file offset (0 for new, filesize for resume)    */
/*   4. Receive data subpackets, write to disk                           */
/*   5. On ZEOF, verify file size matches                                */
/*   6. Send ZRINIT to request next file                                 */
/*                                                                         */
/* Files are written to inbound_dir. If a partial file exists with the   */
/* same name, Zmodem resume is attempted ("Attempting resume").          */
/*                                                                         */
/* Returns 0 on success (file received), -1 on error or no more files.  */
/*-----------------------------------------------------------------------*/

int zm_recv_file(SerPort *sp, const char *inbound_dir)
{
    qf_log(LOG_DEBUG, "zm_recv_file: inbound=%s", inbound_dir);
    ZmHeader hdr;
    FILE *f = NULL;
    char filepath[520];
    char filename[260];
    long file_size = 0, received = 0;
    int errors = 0;
    time_t start_time;

    start_time = time(NULL);

    /* Send ZRINIT */
    memset(&hdr, 0, sizeof(hdr));
    hdr.type = ZRINIT;
    hdr.pos = CANFDX | CANOVIO | CANFC32;
    zm_send_hex_header(sp, &hdr);

    /* Wait for ZFILE or ZFIN */
    while (1) {
        if (zm_recv_header(sp, &hdr, ZM_TIMEOUT * 3) != 0) {
            errors++;
            if (errors >= ZM_MAX_ERRORS) {
                qf_log(LOG_WARN, "Zmodem - never got proper handshake");
                return -1;
            }
            /* Resend ZRINIT */
            hdr.type = ZRINIT;
            hdr.pos = CANFDX | CANOVIO | CANFC32;
            zm_send_hex_header(sp, &hdr);
            continue;
        }

        if (hdr.type == ZFIN) {
            qf_log(LOG_INFO, "Zmodem - no files to receive");
            hdr.type = ZFIN;
            zm_send_hex_header(sp, &hdr);
            return 0;
        }

        if (hdr.type == ZFILE)
            break;
    }

    /* Read ZFILE data subpacket — contains filename and size */
    {
        unsigned char info[1024];
        int pos = 0, ch;

        while (pos < 1023) {
            ch = zm_read_byte(sp, ZM_TIMEOUT);
            if (ch < 0) break;
            if (ch == ZDLE) {
                ch = ser_read_byte(sp, ZM_TIMEOUT);
                if (ch >= 'h' && ch <= 'k') break;  /* Subpacket end */
            }
            info[pos++] = (unsigned char)ch;
        }
        info[pos] = '\0';

        /* Parse: "filename\0size date ..." */
        strncpy(filename, (char *)info, sizeof(filename) - 1);
        /* Sanitize: strip path components, reject ".." */
        {
            char *slash = strrchr(filename, '/');
            char *bslash = strrchr(filename, '\\');
            if (bslash > slash) slash = bslash;
            if (slash) memmove(filename, slash + 1, strlen(slash + 1) + 1);
            if (strstr(filename, "..") || filename[0] == '\0') {
                qf_log(LOG_WARN, "Zmodem: rejecting unsafe filename");
                return -1;
            }
        }
        {
            const char *p = (const char *)info + strlen(filename) + 1;
            file_size = atol(p);
        }
    }

    if (file_size <= 0) {
        qf_log(LOG_WARN, "Zmodem: rejecting %s (invalid size %ld)", filename, file_size);
        hdr.type = ZSKIP;
        zm_send_hex_header(sp, &hdr);
        return -1;
    }

    qf_log(LOG_INFO, "Receiving %s (%ld bytes)", filename, file_size);

    /* Build output path */
    snprintf(filepath, sizeof(filepath), "%s%c%s",
             inbound_dir, PATH_SEP, filename);

    /* Check for existing file — resume support */
    f = fopen(filepath, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        received = ftell(f);
        fclose(f);

        if (received > 0 && received < file_size) {
            qf_log(LOG_INFO, "Resuming partial transfer at %ld", received);
            f = fopen(filepath, "ab");
        } else if (received >= file_size) {
            qf_log(LOG_INFO, "Receiver skipped file %s (already complete)",
                   filename);
            hdr.type = ZSKIP;
            zm_send_hex_header(sp, &hdr);
            return 0;
        } else {
            f = fopen(filepath, "wb");
            received = 0;
        }
    } else {
        f = fopen(filepath, "wb");
        received = 0;
    }

    if (!f) {
        qf_log(LOG_WARN, "Error receiving %s (cannot create file)", filename);
        hdr.type = ZSKIP;
        zm_send_hex_header(sp, &hdr);
        return -1;
    }

    /* Send ZRPOS (start/resume position) */
    hdr.type = ZRPOS;
    hdr.pos = (uint32_t)received;
    zm_send_hex_header(sp, &hdr);

    /* Receive data */
    while (1) {
        if (zm_recv_header(sp, &hdr, ZM_TIMEOUT) != 0) {
            errors++;
            if (errors >= ZM_MAX_ERRORS) {
                qf_log(LOG_WARN, "Too many errors received during protocol");
                break;
            }
            continue;
        }

        if (hdr.type == ZEOF) {
            qf_log(LOG_DEBUG, "End of transmitted file");
            break;
        }

        if (hdr.type == ZDATA) {
            /* Receive data blocks until subpacket end */
            unsigned char block[ZM_BLOCK_SIZE];
            int bpos = 0;
            int ch;

            while (bpos < ZM_BLOCK_SIZE) {
                ch = zm_read_byte(sp, ZM_TIMEOUT);
                if (ch < 0) break;

                /* Check for subpacket terminator */
                if (ch == ZDLE) {
                    ch = ser_read_byte(sp, ZM_TIMEOUT);
                    if (ch >= 'h' && ch <= 'k') {
                        /* CrcE/G/Q/W — end of subpacket */
                        /* Read and discard CRC */
                        zm_read_byte(sp, ZM_TIMEOUT);
                        zm_read_byte(sp, ZM_TIMEOUT);
                        zm_read_byte(sp, ZM_TIMEOUT);
                        zm_read_byte(sp, ZM_TIMEOUT);
                        break;
                    }
                    ch ^= 0x40;  /* Unescape */
                }

                block[bpos++] = (unsigned char)ch;
            }

            if (bpos > 0) {
                fwrite(block, 1, bpos, f);
                received += bpos;
            }

            /* Send ZACK for CrcW/CrcQ subpackets */
            /* CrcG is streaming — no ack needed */
        }

        if (!ser_get_dcd(sp)) {
            qf_log(LOG_WARN, "Lost carrier");
            break;
        }
    }

    fclose(f);

    /* Send ZRINIT to signal ready for next file */
    hdr.type = ZRINIT;
    hdr.pos = CANFDX | CANOVIO | CANFC32;
    zm_send_hex_header(sp, &hdr);

    {
        time_t elapsed = time(NULL) - start_time;
        long cps = (elapsed > 0) ? received / elapsed : 0;

        if (received >= file_size)
            qf_log(LOG_INFO, "Successfully received %s (%ld bytes, %ld CPS)",
                   filename, received, cps);
        else
            qf_log(LOG_WARN,
                   "Attempt to receive file(s) was unsuccessful");
    }

    return (received >= file_size) ? 0 : -1;
}
