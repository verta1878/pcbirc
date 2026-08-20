/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* xmodem.c -- Xmodem / Xmodem-1K / XmodemCRC / SEAlink                    */
/*                                                                           */
/* Legacy file transfer protocols used as Zmodem fallback.                   */
/*                                                                           */
/* Variants (from binary):                                                   */
/*   "Xmodem"     128-byte blocks, simple checksum                          */
/*   "Xmodem1K"   1024-byte blocks, CRC-16                                 */
/*   "XmodemCRC"  128-byte blocks, CRC-16                                   */
/*   "SEAlink"    Xmodem with sliding window (FidoNet extension)            */
/*                                                                           */
/* From binary:                                                              */
/*   "Xmodem init failed"                                                   */
/*   "Xmodem init was canceled on request"                                  */
/*   "Duplicate block received"                                             */
/*   "Wrong block number received"                                          */
/*   "Block shorter than requested"                                         */
/*                                                                           */
/* Clean-room from public Xmodem specification (Ward Christensen, 1977).     */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "qfront.h"

/* Forward declarations -- serial.c */
typedef struct SerPort SerPort;
extern int  ser_read_byte(SerPort *sp, int timeout_ms);
extern int  ser_write(SerPort *sp, const void *buf, int len);
extern int  ser_get_dcd(SerPort *sp);
extern void ser_flush(SerPort *sp);


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                       Xmodem Constants                                    */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#define SOH         0x01                /* start of 128-byte block       */
#define STX         0x02                /* start of 1024-byte block      */
#define EOT         0x04                /* end of transmission           */
#define ACK         0x06                /* acknowledge                   */
#define NAK         0x15                /* negative acknowledge          */
#define CAN         0x18                /* cancel (2 CANs = abort)       */
#define CPMEOF      0x1A                /* CP/M EOF padding              */

#define XM_128      128                 /* standard block size           */
#define XM_1K       1024                /* 1K block size                 */
#define XM_MAX_RETRY 10                 /* max retries per block         */
#define XM_TIMEOUT   10000              /* 10 second timeout             */
#define XM_INIT_TIMEOUT 60000           /* 60 seconds for initial sync   */


/*-----------------------------------------------------------------------*/
/* xm_crc16() -- CRC-16/CCITT for Xmodem                               */
/*-----------------------------------------------------------------------*/

static uint16_t xm_crc16(const void *Data, int Len)
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
/* xm_checksum() -- Simple 8-bit checksum for classic Xmodem            */
/*-----------------------------------------------------------------------*/

static unsigned char xm_checksum(const void *Data, int Len)
{
    const unsigned char *p = (const unsigned char *)Data;
    unsigned char Sum = 0;              /* running checksum              */
    int i;                              /* byte loop index               */

    for (i = 0; i < Len; i++)
        Sum += p[i];
    return Sum;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                          Xmodem Send                                      */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* xm_send_file() -- Send a file via Xmodem / Xmodem-1K / Xmodem-CRC   */
/*                                                                       */
/* Xmodem is the fallback protocol when Zmodem negotiation fails.        */
/* It's much slower than Zmodem (half-duplex, 128 or 1024 byte blocks,  */
/* ACK required for every block) but nearly universally supported.       */
/*                                                                       */
/* Parameters:                                                           */
/*   Use1K:   1 = Xmodem-1K (1024-byte blocks), 0 = classic (128-byte) */
/*   UseCrc:  1 = CRC-16 error checking, 0 = simple checksum            */
/*                                                                       */
/* The receiver initiates by sending NAK (checksum mode) or 'C' (CRC    */
/* mode). We detect which one and adapt accordingly.                     */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

int xm_send_file(SerPort *Sp, const char *Filepath, int Use1K,
                  int UseCrc)
{
    FILE         *f;                    /* input file handle             */
    unsigned char Block[XM_1K + 6];     /* block buffer (max 1K + hdr)   */
    int           BlkSize;              /* current block size            */
    int           BlkNum = 1;           /* block sequence number         */
    int           Ch;                   /* received character            */
    int           Retry;                /* retry counter                 */
    int           BytesRead;            /* bytes read from file          */
    long          Sent = 0;             /* total bytes sent              */

    BlkSize = Use1K ? XM_1K : XM_128;

    f = fopen(Filepath, "rb");
    if (!f) {
        qf_log(LOG_WARN, "Error sending %s", Filepath);
        return -1;
    }

    qf_log(LOG_INFO, "Protocol      : %s",
           Use1K ? "Xmodem1K" : (UseCrc ? "XmodemCRC" : "Xmodem"));

    /* Wait for receiver's init character:
     * NAK = checksum mode, 'C' = CRC mode */
    for (Retry = 0; Retry < XM_MAX_RETRY; Retry++) {
        Ch = ser_read_byte(Sp, XM_INIT_TIMEOUT);
        if (Ch < 0) continue;
        if (Ch == 'C')   { UseCrc = 1; break; }
        if (Ch == NAK)   { UseCrc = 0; break; }
        if (Ch == CAN) {
            qf_log(LOG_WARN, "Xmodem init was canceled on request");
            fclose(f);
            return -1;
        }
    }

    if (Retry >= XM_MAX_RETRY) {
        qf_log(LOG_WARN, "Xmodem init failed");
        fclose(f);
        return -1;
    }

    /* Send blocks */
    while ((BytesRead = (int)fread(Block + 3, 1, BlkSize, f)) > 0) {
        int PktLen;                     /* total packet length           */

        /* Pad last block with CPMEOF */
        while (BytesRead < BlkSize)
            Block[3 + BytesRead++] = CPMEOF;

        /* Build packet header */
        Block[0] = (BlkSize == XM_1K) ? STX : SOH;
        Block[1] = (unsigned char)(BlkNum & 0xFF);
        Block[2] = (unsigned char)(~BlkNum & 0xFF);

        /* Append check bytes */
        if (UseCrc) {
            uint16_t Crc = xm_crc16(Block + 3, BlkSize);
            Block[3 + BlkSize]     = (unsigned char)(Crc >> 8);
            Block[3 + BlkSize + 1] = (unsigned char)(Crc & 0xFF);
            PktLen = 3 + BlkSize + 2;
        } else {
            Block[3 + BlkSize] = xm_checksum(Block + 3, BlkSize);
            PktLen = 3 + BlkSize + 1;
        }

        /* Send with retry */
        for (Retry = 0; Retry < XM_MAX_RETRY; Retry++) {
            ser_write(Sp, Block, PktLen);

            Ch = ser_read_byte(Sp, XM_TIMEOUT);
            if (Ch == ACK) break;
            if (Ch == CAN) {
                qf_log(LOG_WARN, "Receiver cancelled transfer");
                fclose(f);
                return -1;
            }
            if (Ch == NAK) {
                qf_log(LOG_DEBUG, "Block %d NAK, retrying", BlkNum);
                continue;
            }

            if (!ser_get_dcd(Sp)) {
                qf_log(LOG_WARN, "Lost carrier");
                fclose(f);
                return -1;
            }
        }

        if (Retry >= XM_MAX_RETRY) {
            qf_log(LOG_WARN, "Maximum protocol error count reached");
            fclose(f);
            return -1;
        }

        Sent += BlkSize;
        BlkNum++;
    }

    fclose(f);

    /* Send EOT */
    for (Retry = 0; Retry < XM_MAX_RETRY; Retry++) {
        unsigned char EotByte = EOT;
        ser_write(Sp, &EotByte, 1);
        Ch = ser_read_byte(Sp, XM_TIMEOUT);
        if (Ch == ACK) break;
    }

    qf_log(LOG_INFO, "Successfully sent %s (%ld bytes)", Filepath, Sent);
    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                         Xmodem Receive                                    */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* xm_recv_file() -- Receive a file via Xmodem / Xmodem-1K / XmodemCRC */
/*                                                                       */
/* We initiate by sending 'C' (CRC mode request) or NAK (checksum).     */
/* The sender responds with SOH (128-byte) or STX (1024-byte) blocks.   */
/*                                                                       */
/* Block numbers wrap at 255 -- block 256 is numbered 0 again.           */
/* Duplicate blocks (retransmissions) are ACKed but not written.         */
/*                                                                       */
/* EOT from the sender signals end of file. We ACK the EOT and return.  */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

int xm_recv_file(SerPort *Sp, const char *Filepath, int Use1K,
                  int UseCrc)
{
    FILE         *f;                    /* output file handle            */
    unsigned char Block[XM_1K + 6];     /* block buffer                  */
    int           ExpectedBlk = 1;      /* expected block number         */
    int           Retry;                /* retry counter                 */
    int           Ch;                   /* received character            */
    int           BlkSize;              /* current block size            */
    long          Received = 0;         /* total bytes received          */

    (void)Use1K;                        /* detected from SOH/STX         */

    f = fopen(Filepath, "wb");
    if (!f) {
        qf_log(LOG_WARN, "Error receiving %s (cannot create file)", Filepath);
        return -1;
    }

    qf_log(LOG_INFO, "Protocol      : %s",
           Use1K ? "Xmodem1K" : (UseCrc ? "XmodemCRC" : "Xmodem"));

    /* Send init character: 'C' for CRC, NAK for checksum */
    for (Retry = 0; Retry < XM_MAX_RETRY; Retry++) {
        unsigned char InitCh = UseCrc ? 'C' : NAK;
        ser_write(Sp, &InitCh, 1);

        Ch = ser_read_byte(Sp, XM_INIT_TIMEOUT);
        if (Ch == SOH || Ch == STX) break;
        if (Ch == EOT) {
            fclose(f);                  /* empty transfer                */
            return 0;
        }
        if (Ch == CAN) {
            qf_log(LOG_WARN, "Xmodem init was canceled on request");
            fclose(f);
            return -1;
        }
    }

    if (Retry >= XM_MAX_RETRY) {
        qf_log(LOG_WARN, "Xmodem init failed");
        fclose(f);
        return -1;
    }

    /* Receive blocks */
    while (1) {
        unsigned char BlkNum;           /* received block number         */
        unsigned char BlkInv;           /* received block complement     */
        int           Valid;            /* block validity flag           */

        /* We already have SOH/STX from init or previous iteration */
        BlkSize = (Ch == STX) ? XM_1K : XM_128;

        /* Read block number and complement */
        BlkNum = (unsigned char)ser_read_byte(Sp, XM_TIMEOUT);
        BlkInv = (unsigned char)ser_read_byte(Sp, XM_TIMEOUT);

        /* Read data block */
        {
            int i;                      /* byte read index               */
            for (i = 0; i < BlkSize; i++) {
                int b = ser_read_byte(Sp, XM_TIMEOUT);
                if (b < 0) {
                    qf_log(LOG_WARN, "Block shorter than requested");
                    goto send_nak;
                }
                Block[i] = (unsigned char)b;
            }
        }

        /* Read and validate check bytes */
        Valid = 1;
        if (UseCrc) {
            uint16_t RecvCrc;           /* CRC from sender               */
            uint16_t CalcCrc;           /* CRC we calculated             */

            RecvCrc  = (uint16_t)ser_read_byte(Sp, XM_TIMEOUT) << 8;
            RecvCrc |= (uint16_t)ser_read_byte(Sp, XM_TIMEOUT);
            CalcCrc  = xm_crc16(Block, BlkSize);
            if (RecvCrc != CalcCrc) {
                qf_log(LOG_DEBUG, "Incorrect CRC or checksum received");
                Valid = 0;
            }
        } else {
            unsigned char RecvSum;      /* checksum from sender          */
            unsigned char CalcSum;      /* checksum we calculated        */

            RecvSum = (unsigned char)ser_read_byte(Sp, XM_TIMEOUT);
            CalcSum = xm_checksum(Block, BlkSize);
            if (RecvSum != CalcSum) {
                qf_log(LOG_DEBUG, "Incorrect CRC or checksum received");
                Valid = 0;
            }
        }

        /* Validate block number */
        if ((BlkNum ^ BlkInv) != 0xFF) {
            qf_log(LOG_DEBUG, "Wrong block number received");
            Valid = 0;
        }

        if (BlkNum == (unsigned char)((ExpectedBlk - 1) & 0xFF)) {
            /* Duplicate block -- ACK but don't write */
            qf_log(LOG_DEBUG, "Duplicate block received");
            {
                unsigned char AckByte = ACK;
                ser_write(Sp, &AckByte, 1);
            }
            goto next_block;
        }

        if (BlkNum != (unsigned char)(ExpectedBlk & 0xFF)) {
            qf_log(LOG_DEBUG, "Wrong block number received");
            Valid = 0;
        }

        if (!Valid) {
send_nak:
            {
                unsigned char NakByte = NAK;
                ser_write(Sp, &NakByte, 1);
            }
            goto next_block;
        }

        /* Write block to file */
        fwrite(Block, 1, BlkSize, f);
        Received += BlkSize;
        ExpectedBlk++;

        /* ACK */
        {
            unsigned char AckByte = ACK;
            ser_write(Sp, &AckByte, 1);
        }

next_block:
        /* Wait for next block header */
        Ch = ser_read_byte(Sp, XM_TIMEOUT);
        if (Ch == EOT) {
            /* End of transmission -- ACK it */
            unsigned char AckByte = ACK;
            ser_write(Sp, &AckByte, 1);
            break;
        }
        if (Ch == CAN) {
            /* Check for double CAN */
            int Ch2 = ser_read_byte(Sp, 1000);
            if (Ch2 == CAN) {
                qf_log(LOG_WARN, "Receiver cancelled transfer");
                break;
            }
        }
        if (Ch != SOH && Ch != STX) {
            qf_log(LOG_DEBUG, "Unexpected char during protocol");
            continue;
        }

        if (!ser_get_dcd(Sp)) {
            qf_log(LOG_WARN, "Lost carrier");
            break;
        }
    }

    fclose(f);

    qf_log(LOG_INFO, "Successfully received %s (%ld bytes)",
           Filepath, Received);
    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                    SEAlink Send (Sliding Window Xmodem)                   */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* sea_send_file() -- Send a file via SEAlink protocol                  */
/*                                                                       */
/* SEAlink uses a sliding window of up to 6 blocks ahead without         */
/* waiting for ACK. This improves throughput on high-latency             */
/* connections (satellite, packet radio).                                */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

int sea_send_file(SerPort *Sp, const char *Filepath)
{
    FILE         *f;                    /* input file handle             */
    unsigned char Block[XM_128 + 6];    /* block buffer                  */
    int           BlkNum = 1;           /* block sequence number         */
    int           Window = 0;           /* blocks sent without ACK       */
    int           MaxWindow = 6;        /* SEAlink window size           */
    int           Ch;                   /* received character            */
    int           BytesRead;            /* bytes read from file          */
    long          Sent = 0;             /* total bytes sent              */

    f = fopen(Filepath, "rb");
    if (!f) return -1;

    qf_log(LOG_INFO, "Protocol      : SEAlink");

    /* Wait for NAK or 'C' */
    Ch = ser_read_byte(Sp, XM_INIT_TIMEOUT);
    if (Ch != NAK && Ch != 'C') {
        qf_log(LOG_WARN, "Xmodem init failed");
        fclose(f);
        return -1;
    }

    while ((BytesRead = (int)fread(Block + 3, 1, XM_128, f)) > 0) {
        int      PktLen;                /* total packet length           */
        uint16_t Crc;                   /* CRC-16 of block data          */

        /* Pad last block with CPMEOF */
        while (BytesRead < XM_128)
            Block[3 + BytesRead++] = CPMEOF;

        /* Header */
        Block[0] = SOH;
        Block[1] = (unsigned char)(BlkNum & 0xFF);
        Block[2] = (unsigned char)(~BlkNum & 0xFF);

        /* CRC-16 */
        Crc = xm_crc16(Block + 3, XM_128);
        Block[3 + XM_128]     = (unsigned char)(Crc >> 8);
        Block[3 + XM_128 + 1] = (unsigned char)(Crc & 0xFF);
        PktLen = 3 + XM_128 + 2;

        /* Send block without waiting (sliding window) */
        ser_write(Sp, Block, PktLen);
        Window++;
        BlkNum++;
        Sent += XM_128;

        /* Drain ACKs when window is full */
        if (Window >= MaxWindow) {
            while (Window > 0) {
                Ch = ser_read_byte(Sp, XM_TIMEOUT);
                if (Ch == ACK) Window--;
                else if (Ch == NAK) { Window = 0; break; }
                else if (Ch < 0) break;
            }
        }

        if (!ser_get_dcd(Sp)) {
            qf_log(LOG_WARN, "Lost carrier");
            break;
        }
    }

    fclose(f);

    /* Drain remaining ACKs */
    while (Window > 0) {
        Ch = ser_read_byte(Sp, XM_TIMEOUT);
        if (Ch == ACK) Window--;
        else break;
    }

    /* EOT */
    {
        unsigned char EotByte = EOT;
        ser_write(Sp, &EotByte, 1);
        ser_read_byte(Sp, XM_TIMEOUT);  /* wait for ACK                  */
    }

    qf_log(LOG_INFO, "Successfully sent %s (%ld bytes)", Filepath, Sent);
    return 0;
}
