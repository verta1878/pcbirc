/* ====================================================================
 * xmodem.c — Xmodem / Xmodem-1K / XmodemCRC / SEAlink
 * ====================================================================
 * Legacy file transfer protocols used as Zmodem fallback.
 *
 * Variants (from binary):
 *   "Xmodem"     128-byte blocks, simple checksum
 *   "Xmodem1K"   1024-byte blocks, CRC-16
 *   "XmodemCRC"  128-byte blocks, CRC-16
 *   "SEAlink"    Xmodem with sliding window (FidoNet extension)
 *
 * From binary:
 *   "Xmodem init failed"
 *   "Xmodem init was canceled on request"
 *   "Duplicate block received"
 *   "Wrong block number received"
 *   "Block shorter than requested"
 *
 * Clean-room from public Xmodem specification (Ward Christensen, 1977).
 * ==================================================================== */

#include "qfront.h"

/* Forward declarations — serial.c */
typedef struct SerPort SerPort;
extern int  ser_read_byte(SerPort *sp, int timeout_ms);
extern int  ser_write(SerPort *sp, const void *buf, int len);
extern int  ser_get_dcd(SerPort *sp);
extern void ser_flush(SerPort *sp);

/* ---- Xmodem Constants ---- */

#define SOH         0x01          /* Start of 128-byte block       */
#define STX         0x02          /* Start of 1024-byte block      */
#define EOT         0x04          /* End of transmission            */
#define ACK         0x06          /* Acknowledge                    */
#define NAK         0x15          /* Negative acknowledge           */
#define CAN         0x18          /* Cancel (2 CANs = abort)        */
#define CPMEOF      0x1A          /* CP/M EOF padding               */

#define XM_128      128           /* Standard block size            */
#define XM_1K       1024          /* 1K block size                  */
#define XM_MAX_RETRY 10           /* Max retries per block          */
#define XM_TIMEOUT   10000        /* 10 second timeout              */
#define XM_INIT_TIMEOUT 60000     /* 60 seconds for initial sync    */

/* ---- CRC-16 (CCITT) ---- */

static uint16_t xm_crc16(const void *data, int len)
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

/* ---- Simple Checksum ---- */

static unsigned char xm_checksum(const void *data, int len)
{
    const unsigned char *p = (const unsigned char *)data;
    unsigned char sum = 0;
    int i;
    for (i = 0; i < len; i++)
        sum += p[i];
    return sum;
}


/* ---- Send File via Xmodem ---- */

/*-----------------------------------------------------------------------*/
/* xm_send_file() — Send a file via Xmodem / Xmodem-1K / Xmodem-CRC    */
/*                                                                         */
/* Xmodem is the fallback protocol when Zmodem negotiation fails.       */
/* It's much slower than Zmodem (half-duplex, 128 or 1024 byte blocks,  */
/* ACK required for every block) but nearly universally supported.      */
/*                                                                         */
/* Parameters:                                                            */
/*   use_1k:  1 = Xmodem-1K (1024-byte blocks), 0 = classic (128-byte) */
/*   use_crc: 1 = CRC-16 error checking, 0 = simple checksum           */
/*                                                                         */
/* The receiver initiates by sending NAK (checksum mode) or 'C' (CRC    */
/* mode). We detect which one and adapt accordingly.                     */
/*                                                                         */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

int xm_send_file(SerPort *sp, const char *filepath, int use_1k,
                  int use_crc)
{
    FILE *f;
    unsigned char block[XM_1K + 6];  /* Max: STX + blk + ~blk + 1024 + crc16 */
    int blk_size = use_1k ? XM_1K : XM_128;
    int blk_num = 1;
    int ch, retry, n;
    long sent = 0;

    f = fopen(filepath, "rb");
    if (!f) {
        qf_log(LOG_WARN, "Error sending %s", filepath);
        return -1;
    }

    qf_log(LOG_INFO, "Protocol      : %s",
           use_1k ? "Xmodem1K" : (use_crc ? "XmodemCRC" : "Xmodem"));

    /* Wait for receiver's init character */
    /* NAK = checksum mode, 'C' = CRC mode */
    for (retry = 0; retry < XM_MAX_RETRY; retry++) {
        ch = ser_read_byte(sp, XM_INIT_TIMEOUT);
        if (ch < 0) continue;
        if (ch == 'C') { use_crc = 1; break; }
        if (ch == NAK) { use_crc = 0; break; }
        if (ch == CAN) {
            qf_log(LOG_WARN, "Xmodem init was canceled on request");
            fclose(f);
            return -1;
        }
    }

    if (retry >= XM_MAX_RETRY) {
        qf_log(LOG_WARN, "Xmodem init failed");
        fclose(f);
        return -1;
    }

    /* Send blocks */
    while ((n = (int)fread(block + 3, 1, blk_size, f)) > 0) {
        int pkt_len;

        /* Pad last block with CPMEOF */
        while (n < blk_size)
            block[3 + n++] = CPMEOF;

        /* Build packet header */
        block[0] = (blk_size == XM_1K) ? STX : SOH;
        block[1] = (unsigned char)(blk_num & 0xFF);
        block[2] = (unsigned char)(~blk_num & 0xFF);

        /* Append check bytes */
        if (use_crc) {
            uint16_t crc = xm_crc16(block + 3, blk_size);
            block[3 + blk_size]     = (unsigned char)(crc >> 8);
            block[3 + blk_size + 1] = (unsigned char)(crc & 0xFF);
            pkt_len = 3 + blk_size + 2;
        } else {
            block[3 + blk_size] = xm_checksum(block + 3, blk_size);
            pkt_len = 3 + blk_size + 1;
        }

        /* Send with retry */
        for (retry = 0; retry < XM_MAX_RETRY; retry++) {
            ser_write(sp, block, pkt_len);

            ch = ser_read_byte(sp, XM_TIMEOUT);
            if (ch == ACK) break;
            if (ch == CAN) {
                qf_log(LOG_WARN, "Receiver cancelled transfer");
                fclose(f);
                return -1;
            }
            if (ch == NAK) {
                qf_log(LOG_DEBUG, "Block %d NAK, retrying", blk_num);
                continue;
            }

            if (!ser_get_dcd(sp)) {
                qf_log(LOG_WARN, "Lost carrier");
                fclose(f);
                return -1;
            }
        }

        if (retry >= XM_MAX_RETRY) {
            qf_log(LOG_WARN, "Maximum protocol error count reached");
            fclose(f);
            return -1;
        }

        sent += blk_size;
        blk_num++;
    }

    fclose(f);

    /* Send EOT */
    for (retry = 0; retry < XM_MAX_RETRY; retry++) {
        unsigned char eot = EOT;
        ser_write(sp, &eot, 1);
        ch = ser_read_byte(sp, XM_TIMEOUT);
        if (ch == ACK) break;
    }

    qf_log(LOG_INFO, "Successfully sent %s (%ld bytes)",
           filepath, sent);
    return 0;
}


/* ---- Receive File via Xmodem ---- */

/*-----------------------------------------------------------------------*/
/* xm_recv_file() — Receive a file via Xmodem / Xmodem-1K / Xmodem-CRC */
/*                                                                         */
/* We initiate by sending 'C' (CRC mode request) or NAK (checksum mode).*/
/* The sender responds with blocks containing SOH (128-byte) or STX     */
/* (1024-byte) headers, a block number, the data, and error check bytes.*/
/*                                                                         */
/* Block numbers wrap at 255 — block 256 is numbered 0 again.           */
/* Duplicate blocks (retransmissions) are ACKed but not written to disk. */
/*                                                                         */
/* EOT from the sender signals end of file. We ACK the EOT and return.  */
/*                                                                         */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

int xm_recv_file(SerPort *sp, const char *filepath, int use_1k,
                  int use_crc)
{
    FILE *f;
    unsigned char block[XM_1K + 6];
    int expected_blk = 1;
    int retry, ch, blk_size;
    long received = 0;

    f = fopen(filepath, "wb");
    if (!f) {
        qf_log(LOG_WARN, "Error receiving %s (cannot create file)", filepath);
        return -1;
    }

    qf_log(LOG_INFO, "Protocol      : %s",
           use_1k ? "Xmodem1K" : (use_crc ? "XmodemCRC" : "Xmodem"));

    /* Send init character: 'C' for CRC, NAK for checksum */
    for (retry = 0; retry < XM_MAX_RETRY; retry++) {
        unsigned char init_ch = use_crc ? 'C' : NAK;
        ser_write(sp, &init_ch, 1);

        ch = ser_read_byte(sp, XM_INIT_TIMEOUT);
        if (ch == SOH || ch == STX) break;
        if (ch == EOT) {
            /* Empty transfer */
            fclose(f);
            return 0;
        }
        if (ch == CAN) {
            qf_log(LOG_WARN, "Xmodem init was canceled on request");
            fclose(f);
            return -1;
        }
    }

    if (retry >= XM_MAX_RETRY) {
        qf_log(LOG_WARN, "Xmodem init failed");
        fclose(f);
        return -1;
    }

    /* Receive blocks */
    while (1) {
        unsigned char blk_num, blk_inv;
        int valid;

        /* We already have SOH/STX from init or previous iteration */
        blk_size = (ch == STX) ? XM_1K : XM_128;

        /* Read block number and complement */
        blk_num = (unsigned char)ser_read_byte(sp, XM_TIMEOUT);
        blk_inv = (unsigned char)ser_read_byte(sp, XM_TIMEOUT);

        /* Read data block */
        {
            int i;
            for (i = 0; i < blk_size; i++) {
                int b = ser_read_byte(sp, XM_TIMEOUT);
                if (b < 0) {
                    qf_log(LOG_WARN, "Block shorter than requested");
                    goto send_nak;
                }
                block[i] = (unsigned char)b;
            }
        }

        /* Read check bytes */
        valid = 1;
        if (use_crc) {
            uint16_t recv_crc, calc_crc;
            recv_crc = (uint16_t)ser_read_byte(sp, XM_TIMEOUT) << 8;
            recv_crc |= (uint16_t)ser_read_byte(sp, XM_TIMEOUT);
            calc_crc = xm_crc16(block, blk_size);
            if (recv_crc != calc_crc) {
                qf_log(LOG_DEBUG, "Incorrect CRC or checksum received");
                valid = 0;
            }
        } else {
            unsigned char recv_sum, calc_sum;
            recv_sum = (unsigned char)ser_read_byte(sp, XM_TIMEOUT);
            calc_sum = xm_checksum(block, blk_size);
            if (recv_sum != calc_sum) {
                qf_log(LOG_DEBUG, "Incorrect CRC or checksum received");
                valid = 0;
            }
        }

        /* Validate block number */
        if ((blk_num ^ blk_inv) != 0xFF) {
            qf_log(LOG_DEBUG, "Wrong block number received");
            valid = 0;
        }

        if (blk_num == (unsigned char)((expected_blk - 1) & 0xFF)) {
            /* Duplicate block — ACK but don't write */
            qf_log(LOG_DEBUG, "Duplicate block received");
            {
                unsigned char ack = ACK;
                ser_write(sp, &ack, 1);
            }
            goto next_block;
        }

        if (blk_num != (unsigned char)(expected_blk & 0xFF)) {
            qf_log(LOG_DEBUG, "Wrong block number received");
            valid = 0;
        }

        if (!valid) {
send_nak:
            {
                unsigned char nak = NAK;
                ser_write(sp, &nak, 1);
            }
            goto next_block;
        }

        /* Write block to file */
        fwrite(block, 1, blk_size, f);
        received += blk_size;
        expected_blk++;

        /* ACK */
        {
            unsigned char ack = ACK;
            ser_write(sp, &ack, 1);
        }

next_block:
        /* Wait for next block header */
        ch = ser_read_byte(sp, XM_TIMEOUT);
        if (ch == EOT) {
            /* End of transmission — ACK it */
            unsigned char ack = ACK;
            ser_write(sp, &ack, 1);
            break;
        }
        if (ch == CAN) {
            /* Check for double CAN */
            int ch2 = ser_read_byte(sp, 1000);
            if (ch2 == CAN) {
                qf_log(LOG_WARN, "Receiver cancelled transfer");
                break;
            }
        }
        if (ch != SOH && ch != STX) {
            qf_log(LOG_DEBUG, "Unexpected char during protocol");
            continue;
        }

        if (!ser_get_dcd(sp)) {
            qf_log(LOG_WARN, "Lost carrier");
            break;
        }
    }

    fclose(f);

    qf_log(LOG_INFO, "Successfully received %s (%ld bytes)",
           filepath, received);
    return 0;
}


/* ---- SEAlink Send (Sliding Window Xmodem) ----
 * SEAlink uses a sliding window of up to 6 blocks ahead
 * without waiting for ACK. This improves throughput on
 * high-latency connections (satellite, packet radio). */

int sea_send_file(SerPort *sp, const char *filepath)
{
    FILE *f;
    unsigned char block[XM_128 + 6];
    int blk_num = 1;
    int window = 0;
    int max_window = 6;           /* SEAlink window size           */
    int ch, n;
    long sent = 0;

    f = fopen(filepath, "rb");
    if (!f) return -1;

    qf_log(LOG_INFO, "Protocol      : SEAlink");

    /* Wait for NAK or 'C' */
    ch = ser_read_byte(sp, XM_INIT_TIMEOUT);
    if (ch != NAK && ch != 'C') {
        qf_log(LOG_WARN, "Xmodem init failed");
        fclose(f);
        return -1;
    }

    while ((n = (int)fread(block + 3, 1, XM_128, f)) > 0) {
        int pkt_len;
        uint16_t crc;

        /* Pad */
        while (n < XM_128)
            block[3 + n++] = CPMEOF;

        /* Header */
        block[0] = SOH;
        block[1] = (unsigned char)(blk_num & 0xFF);
        block[2] = (unsigned char)(~blk_num & 0xFF);

        /* CRC-16 */
        crc = xm_crc16(block + 3, XM_128);
        block[3 + XM_128]     = (unsigned char)(crc >> 8);
        block[3 + XM_128 + 1] = (unsigned char)(crc & 0xFF);
        pkt_len = 3 + XM_128 + 2;

        /* Send block without waiting (sliding window) */
        ser_write(sp, block, pkt_len);
        window++;
        blk_num++;
        sent += XM_128;

        /* Drain ACKs when window is full */
        if (window >= max_window) {
            while (window > 0) {
                ch = ser_read_byte(sp, XM_TIMEOUT);
                if (ch == ACK) window--;
                else if (ch == NAK) { window = 0; break; }
                else if (ch < 0) break;
            }
        }

        if (!ser_get_dcd(sp)) {
            qf_log(LOG_WARN, "Lost carrier");
            break;
        }
    }

    fclose(f);

    /* Drain remaining ACKs */
    while (window > 0) {
        ch = ser_read_byte(sp, XM_TIMEOUT);
        if (ch == ACK) window--;
        else break;
    }

    /* EOT */
    {
        unsigned char eot = EOT;
        ser_write(sp, &eot, 1);
        ser_read_byte(sp, XM_TIMEOUT);  /* Wait for ACK */
    }

    qf_log(LOG_INFO, "Successfully sent %s (%ld bytes)", filepath, sent);
    return 0;
}
