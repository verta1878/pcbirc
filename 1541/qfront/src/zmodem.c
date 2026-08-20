/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* zmodem.c -- Zmodem File Transfer Protocol                                */
/*                                                                           */
/* Implements Zmodem send and receive with:                                  */
/*   - 8K block size ("Using Zmodem 8k block size")                         */
/*   - CRC-16 and CRC-32                                                    */
/*   - 4 subpacket types (CrcE, CrcG, CrcQ, CrcW)                          */
/*   - Resume support ("Attempting resume")                                 */
/*   - Skip support ("Zmodem - skip file")                                  */
/*   - Progress display (CPS, blocks, bytes remaining)                      */
/*                                                                           */
/* From binary:                                                              */
/*   "Zmodem - bad file position"                                           */
/*   "Zmodem - specified file does not exist"                               */
/*   "Zmodem - not allowed to overwrite file"                               */
/*   "Zmodem - never got proper handshake"                                  */
/*   "Zmodem - got CrcE/CrcG/CrcQ/CrcW DataSubpacket"                      */
/*   "Zmodem - got garbage from remote"                                     */
/*   "Zmodem - no files to receive"                                         */
/*   "Zmodem - skip file"                                                   */
/*                                                                           */
/* Clean-room from public Zmodem specification (Chuck Forsberg, 1986).       */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "qfront.h"

/* Forward declarations -- serial.c */
typedef struct SerPort SerPort;
extern int  ser_read_byte(SerPort *sp, int timeout_ms);
extern int  ser_write(SerPort *sp, const void *buf, int len);
extern int  ser_get_dcd(SerPort *sp);
extern int  ser_data_ready(SerPort *sp);
extern void ser_flush(SerPort *sp);


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                       Zmodem Constants                                    */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#define ZPAD        '*'                 /* padding character             */
#define ZDLE        0x18                /* escape character (CAN)        */
#define ZDLEE       0x58                /* escaped ZDLE                  */
#define ZBIN        'A'                 /* binary header (CRC-16)        */
#define ZHEX        'B'                 /* hex header (CRC-16)           */
#define ZBIN32      'C'                 /* binary header (CRC-32)        */

/* Frame types */
#define ZRQINIT     0                   /* request receiver init         */
#define ZRINIT      1                   /* receiver init                 */
#define ZSINIT      2                   /* sender init (send setup)      */
#define ZACK        3                   /* acknowledge                   */
#define ZFILE       4                   /* file header (name, size)      */
#define ZSKIP       5                   /* skip this file                */
#define ZNAK        6                   /* negative acknowledge          */
#define ZABORT      7                   /* abort session                 */
#define ZFIN        8                   /* finish session                */
#define ZRPOS       9                   /* resume position               */
#define ZDATA       10                  /* data packet follows           */
#define ZEOF        11                  /* end of file                   */
#define ZFERR       12                  /* file error (read/write)       */
#define ZCRC        13                  /* CRC request                   */
#define ZCHALLENGE  14                  /* challenge                     */
#define ZCOMPL      15                  /* complete                      */
#define ZCAN        16                  /* cancel (5 CAN chars)          */
#define ZFREECNT    17                  /* free disk space request       */
#define ZCOMMAND    18                  /* command                       */

/* Subpacket types */
#define ZCRCW       'k'                 /* CRC + wait (expect ZACK)      */
#define ZCRCG       'h'                 /* CRC + go (streaming, no ack)  */
#define ZCRCQ       'j'                 /* CRC + query (expect ZACK)     */
#define ZCRCE       'i'                 /* CRC + end (last subpacket)    */

/* ZRINIT capability flags */
#define CANFDX      0x01                /* full duplex                   */
#define CANOVIO     0x02                /* overlapped I/O                */
#define CANBRK      0x04                /* break signal                  */
#define CANCRY      0x08                /* encryption                    */
#define CANLZW      0x10                /* LZW compression               */
#define CANFC32     0x20                /* CRC-32 capable                */
#define ESCCTL      0x40                /* escape control chars          */
#define ESC8        0x80                /* escape 8-bit chars            */

#define ZM_BLOCK_SIZE  8192             /* 8K blocks                     */
#define ZM_TIMEOUT     10000            /* 10 second timeout             */
#define ZM_MAX_ERRORS  10               /* max errors before abort       */
#define ZM_CAN_COUNT   5                /* 5 CANs to abort               */


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                          CRC Tables                                       */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

static uint32_t ZmCrc32Table[256];      /* CRC-32 lookup table           */
static int      ZmCrc32Init = 0;        /* table initialized flag        */


/*-----------------------------------------------------------------------*/
/* zm_build_crc32() -- Build the CRC-32 lookup table                    */
/*                                                                       */
/* Polynomial 0xEDB88320 (reversed representation of the standard       */
/* CRC-32 polynomial used by Ethernet, PKZip, etc.).                    */
/*-----------------------------------------------------------------------*/

static void zm_build_crc32(void)
{
    uint32_t Crc;                       /* working CRC value             */
    int      i, j;                      /* byte and bit loop indices     */

    for (i = 0; i < 256; i++) {
        Crc = (uint32_t)i;
        for (j = 0; j < 8; j++) {
            if (Crc & 1)
                Crc = (Crc >> 1) ^ 0xEDB88320;
            else
                Crc >>= 1;
        }
        ZmCrc32Table[i] = Crc;
    }
    ZmCrc32Init = 1;
}


/*-----------------------------------------------------------------------*/
/* zm_crc32() -- Calculate CRC-32 over a data block                     */
/*-----------------------------------------------------------------------*/

static uint32_t zm_crc32(const void *Data, int Len)
{
    const unsigned char *p = (const unsigned char *)Data;
    uint32_t Crc = 0xFFFFFFFF;          /* initial CRC value             */
    int i;                              /* byte loop index               */

    if (!ZmCrc32Init) zm_build_crc32();

    for (i = 0; i < Len; i++)
        Crc = ZmCrc32Table[(Crc ^ p[i]) & 0xFF] ^ (Crc >> 8);

    return Crc ^ 0xFFFFFFFF;
}


/*-----------------------------------------------------------------------*/
/* zm_update_crc32() -- Update CRC-32 with a single byte                */
/*-----------------------------------------------------------------------*/

static uint32_t zm_update_crc32(uint32_t Crc, unsigned char Byte)
{
    if (!ZmCrc32Init) zm_build_crc32();
    return ZmCrc32Table[(Crc ^ Byte) & 0xFF] ^ (Crc >> 8);
}


/*-----------------------------------------------------------------------*/
/* zm_crc16() -- CRC-16/CCITT                                          */
/*-----------------------------------------------------------------------*/

static uint16_t zm_crc16(const void *Data, int Len)
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


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                       Zmodem Header Structure                             */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef struct {
    int      Type;                      /* frame type (ZRQINIT, etc.)    */
    uint32_t Pos;                       /* position / flags (4 bytes)    */
    int      UseCrc32;                  /* CRC-32 mode?                  */
} ZmHeader;


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                   ZDLE Encoding / Decoding                                */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* zm_is_escape() -- Check if a byte needs ZDLE escaping                */
/*                                                                       */
/* Zmodem escapes: DLE(0x10), XON(0x11), XOFF(0x13), ZDLE(0x18),       */
/* and their high-bit equivalents (0x90, 0x91, 0x93), plus 0xFF.        */
/*-----------------------------------------------------------------------*/

static int zm_is_escape(unsigned char Ch)
{
    return (Ch == 0x10 || Ch == 0x11 || Ch == 0x13 || Ch == 0x18 ||
            Ch == 0x90 || Ch == 0x91 || Ch == 0x93 || Ch == 0xFF);
}


/*-----------------------------------------------------------------------*/
/* zm_send_escaped() -- Send data with ZDLE escaping                    */
/*-----------------------------------------------------------------------*/

static int zm_send_escaped(SerPort *Sp, const void *Data, int Len)
{
    const unsigned char *p = (const unsigned char *)Data;
    unsigned char Esc[2];               /* escape pair buffer            */
    int i;                              /* byte loop index               */

    for (i = 0; i < Len; i++) {
        if (zm_is_escape(p[i])) {
            Esc[0] = ZDLE;
            Esc[1] = p[i] ^ 0x40;
            ser_write(Sp, Esc, 2);
        } else {
            ser_write(Sp, &p[i], 1);
        }
    }
    return Len;
}


/*-----------------------------------------------------------------------*/
/* zm_read_byte() -- Read a ZDLE-decoded byte                           */
/*-----------------------------------------------------------------------*/

static int zm_read_byte(SerPort *Sp, int TimeoutMs)
{
    int Ch;                             /* received character            */

    Ch = ser_read_byte(Sp, TimeoutMs);
    if (Ch < 0) return -1;

    if (Ch == ZDLE) {
        Ch = ser_read_byte(Sp, TimeoutMs);
        if (Ch < 0) return -1;
        if (Ch == ZDLEE) return ZDLE;
        return Ch ^ 0x40;
    }
    return Ch;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                    Header Send / Receive                                  */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* zm_send_hex_header() -- Send a Zmodem hex header                     */
/*                                                                       */
/* Format: ZPAD ZPAD ZDLE ZHEX type[2] p0[2] p1[2] p2[2] p3[2]         */
/*         crc[4] CR LF                                                  */
/*-----------------------------------------------------------------------*/

static void zm_send_hex_header(SerPort *Sp, const ZmHeader *Hdr)
{
    unsigned char Raw[5];               /* header data bytes             */
    uint16_t      Crc;                  /* CRC-16 of header              */
    char          Hex[32];              /* hex-encoded output            */
    int           i, j;                 /* output and byte indices       */

    Raw[0] = (unsigned char)Hdr->Type;
    Raw[1] = (unsigned char)(Hdr->Pos & 0xFF);
    Raw[2] = (unsigned char)((Hdr->Pos >> 8) & 0xFF);
    Raw[3] = (unsigned char)((Hdr->Pos >> 16) & 0xFF);
    Raw[4] = (unsigned char)((Hdr->Pos >> 24) & 0xFF);

    Crc = zm_crc16(Raw, 5);

    i = 0;
    Hex[i++] = ZPAD;
    Hex[i++] = ZPAD;
    Hex[i++] = ZDLE;
    Hex[i++] = ZHEX;

    /* Hex-encode 5 data bytes + 2 CRC bytes */
    {
        unsigned char Bytes[7];         /* data + CRC bytes              */
        memcpy(Bytes, Raw, 5);
        Bytes[5] = (unsigned char)(Crc >> 8);
        Bytes[6] = (unsigned char)(Crc & 0xFF);

        for (j = 0; j < 7; j++) {
            Hex[i++] = "0123456789abcdef"[(Bytes[j] >> 4) & 0xF];
            Hex[i++] = "0123456789abcdef"[Bytes[j] & 0xF];
        }
    }

    Hex[i++] = '\r';
    Hex[i++] = '\n';

    ser_write(Sp, Hex, i);
}


/*-----------------------------------------------------------------------*/
/* zm_send_bin32_header() -- Send a binary CRC-32 header                */
/*-----------------------------------------------------------------------*/

static void zm_send_bin32_header(SerPort *Sp, const ZmHeader *Hdr)
{
    unsigned char Preamble[4] = { ZPAD, ZPAD, ZDLE, ZBIN32 };
    unsigned char Raw[5];               /* header data bytes             */
    uint32_t      Crc;                  /* CRC-32 of header              */

    Raw[0] = (unsigned char)Hdr->Type;
    Raw[1] = (unsigned char)(Hdr->Pos & 0xFF);
    Raw[2] = (unsigned char)((Hdr->Pos >> 8) & 0xFF);
    Raw[3] = (unsigned char)((Hdr->Pos >> 16) & 0xFF);
    Raw[4] = (unsigned char)((Hdr->Pos >> 24) & 0xFF);

    Crc = zm_crc32(Raw, 5);

    ser_write(Sp, Preamble, 4);
    zm_send_escaped(Sp, Raw, 5);

    {
        unsigned char CrcBytes[4];      /* CRC-32 in little-endian       */
        CrcBytes[0] = (unsigned char)(Crc & 0xFF);
        CrcBytes[1] = (unsigned char)((Crc >> 8) & 0xFF);
        CrcBytes[2] = (unsigned char)((Crc >> 16) & 0xFF);
        CrcBytes[3] = (unsigned char)((Crc >> 24) & 0xFF);
        zm_send_escaped(Sp, CrcBytes, 4);
    }
}


/*-----------------------------------------------------------------------*/
/* zm_recv_header() -- Receive a Zmodem header (hex or binary)          */
/*                                                                       */
/* Waits for ZPAD ZPAD ZDLE, then reads either hex or binary header.    */
/*                                                                       */
/* Returns 0 on success, -1 on timeout or error.                         */
/*-----------------------------------------------------------------------*/

static int zm_recv_header(SerPort *Sp, ZmHeader *Hdr, int TimeoutMs)
{
    int           Ch;                   /* received character            */
    unsigned char Raw[5];               /* header data bytes             */
    int           i;                    /* byte read index               */

    memset(Hdr, 0, sizeof(*Hdr));

    /* Wait for ZPAD ZPAD ZDLE */
    while (1) {
        Ch = ser_read_byte(Sp, TimeoutMs);
        if (Ch < 0) return -1;
        if (Ch != ZPAD) continue;

        Ch = ser_read_byte(Sp, TimeoutMs);
        if (Ch < 0) return -1;
        if (Ch == ZPAD) {
            Ch = ser_read_byte(Sp, TimeoutMs);
            if (Ch < 0) return -1;
            if (Ch == ZDLE) break;
        }
    }

    /* Read encoding type */
    Ch = ser_read_byte(Sp, TimeoutMs);
    if (Ch < 0) return -1;

    if (Ch == ZHEX) {
        /* Hex header -- read 14 hex chars (7 bytes) */
        char HexBuf[16];               /* hex character buffer          */

        for (i = 0; i < 14; i++) {
            Ch = ser_read_byte(Sp, TimeoutMs);
            if (Ch < 0) return -1;
            HexBuf[i] = (char)Ch;
        }
        HexBuf[14] = '\0';

        for (i = 0; i < 5; i++) {
            unsigned int Val;           /* parsed hex value              */
            sscanf(HexBuf + i * 2, "%2x", &Val);
            Raw[i] = (unsigned char)Val;
        }
        Hdr->UseCrc32 = 0;

        /* Consume trailing CR LF */
        ser_read_byte(Sp, 100);
        ser_read_byte(Sp, 100);

    } else if (Ch == ZBIN || Ch == ZBIN32) {
        Hdr->UseCrc32 = (Ch == ZBIN32);

        /* Binary header -- read 5 ZDLE-decoded bytes */
        for (i = 0; i < 5; i++) {
            Ch = zm_read_byte(Sp, TimeoutMs);
            if (Ch < 0) return -1;
            Raw[i] = (unsigned char)Ch;
        }

        /* Read and discard CRC bytes */
        if (Hdr->UseCrc32) {
            for (i = 0; i < 4; i++)
                zm_read_byte(Sp, TimeoutMs);
        } else {
            zm_read_byte(Sp, TimeoutMs);
            zm_read_byte(Sp, TimeoutMs);
        }
    } else {
        qf_log(LOG_DEBUG, "Zmodem - got garbage from remote");
        return -1;
    }

    Hdr->Type = Raw[0];
    Hdr->Pos  = Raw[1] | ((uint32_t)Raw[2] << 8) |
                ((uint32_t)Raw[3] << 16) | ((uint32_t)Raw[4] << 24);

    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                     Data Subpacket / Cancel                               */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* zm_send_data() -- Send a data subpacket with CRC-32                  */
/*                                                                       */
/* Subpacket terminator: ZDLE + SubType byte. CRC-32 covers both the    */
/* data and the subtype byte.                                            */
/*-----------------------------------------------------------------------*/

static void zm_send_data(SerPort *Sp, const void *Data, int Len,
                          int SubType)
{
    unsigned char End[2];               /* ZDLE + subtype pair           */
    uint32_t      Crc;                  /* CRC-32 of data + subtype      */

    zm_send_escaped(Sp, Data, Len);

    End[0] = ZDLE;
    End[1] = (unsigned char)SubType;
    ser_write(Sp, End, 2);

    /* CRC-32 over data + subtype byte */
    Crc = zm_crc32(Data, Len);
    Crc = zm_update_crc32(Crc ^ 0xFFFFFFFF, (unsigned char)SubType)
          ^ 0xFFFFFFFF;

    {
        unsigned char CrcBytes[4];      /* CRC-32 in little-endian       */
        CrcBytes[0] = (unsigned char)(Crc & 0xFF);
        CrcBytes[1] = (unsigned char)((Crc >> 8) & 0xFF);
        CrcBytes[2] = (unsigned char)((Crc >> 16) & 0xFF);
        CrcBytes[3] = (unsigned char)((Crc >> 24) & 0xFF);
        zm_send_escaped(Sp, CrcBytes, 4);
    }
}


/*-----------------------------------------------------------------------*/
/* zm_send_cancel() -- Send cancel sequence (5 CANs + 5 backspaces)    */
/*-----------------------------------------------------------------------*/

static void zm_send_cancel(SerPort *Sp)
{
    unsigned char Cancel[10];           /* CAN + BS sequence             */
    int i;                              /* loop index                    */

    for (i = 0; i < 5; i++) Cancel[i] = 0x18;     /* CAN               */
    for (i = 5; i < 10; i++) Cancel[i] = 0x08;    /* BS                */
    ser_write(Sp, Cancel, 10);
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                          Zmodem Send                                      */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* zm_send_file() -- Send a single file via Zmodem batch protocol       */
/*                                                                       */
/* Implements the Zmodem sender state machine:                           */
/*   1. Send ZRQINIT (request receiver to initialize)                    */
/*   2. Wait for ZRINIT from receiver                                    */
/*   3. Send ZFILE header with filename, size, timestamp                 */
/*   4. Wait for ZRPOS (0 for new, >0 for resume)                       */
/*   5. Send file data in ZCRCG subpackets (streaming, no ack per block) */
/*   6. Send ZEOF when all data sent                                     */
/*   7. Wait for ZRINIT (receiver ready for next file)                   */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

int zm_send_file(SerPort *Sp, const char *Filepath)
{
    FILE         *f;                    /* input file handle             */
    ZmHeader      Hdr;                  /* protocol header               */
    unsigned char Block[ZM_BLOCK_SIZE]; /* data block buffer             */
    char          FileInfo[1024];       /* filename + size string        */
    long          FileSize;             /* total file size               */
    long          Sent = 0;             /* bytes sent so far             */
    int           n;                    /* bytes read from file          */
    int           Errors = 0;           /* error counter                 */
    time_t        StartTime;            /* transfer start time           */
    const char   *FileName;             /* basename for display          */

    qf_log(LOG_DEBUG, "zm_send_file: %s", Filepath);

    /* Extract filename from path */
    FileName = strrchr(Filepath, PATH_SEP);
    FileName = FileName ? FileName + 1 : Filepath;

    f = fopen(Filepath, "rb");
    if (!f) {
        qf_log(LOG_WARN, "Zmodem - specified file does not exist");
        return -1;
    }

    fseek(f, 0, SEEK_END);
    FileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    qf_log(LOG_INFO, "Protocol      : Zmodem");
    qf_log(LOG_INFO, "Using Zmodem 8k block size");

    StartTime = time(NULL);

    /* Send ZRQINIT */
    memset(&Hdr, 0, sizeof(Hdr));
    Hdr.Type = ZRQINIT;
    zm_send_hex_header(Sp, &Hdr);

    /* Wait for ZRINIT */
    if (zm_recv_header(Sp, &Hdr, ZM_TIMEOUT) != 0 ||
        Hdr.Type != ZRINIT) {
        qf_log(LOG_WARN, "Zmodem - never got proper handshake");
        fclose(f);
        return -1;
    }

    /* Send ZFILE header */
    Hdr.Type = ZFILE;
    Hdr.Pos  = 0;
    zm_send_bin32_header(Sp, &Hdr);

    /* Send file info subpacket: "name\0size\0" */
    n = snprintf(FileInfo, sizeof(FileInfo), "%s", FileName);
    n++;                                /* include null terminator       */
    n += snprintf(FileInfo + n, sizeof(FileInfo) - n, "%ld", FileSize);
    zm_send_data(Sp, FileInfo, n + 1, ZCRCW);

    /* Wait for ZRPOS (resume) or ZSKIP */
    if (zm_recv_header(Sp, &Hdr, ZM_TIMEOUT) != 0) {
        fclose(f);
        return -1;
    }

    if (Hdr.Type == ZSKIP) {
        qf_log(LOG_INFO, "Zmodem - skip file");
        fclose(f);
        return 0;
    }

    if (Hdr.Type == ZRPOS) {
        if (Hdr.Pos > 0) {
            qf_log(LOG_INFO, "Attempting resume at offset %lu",
                   (unsigned long)Hdr.Pos);
            fseek(f, (long)Hdr.Pos, SEEK_SET);
            Sent = (long)Hdr.Pos;
        }
    }

    /* Send ZDATA header */
    Hdr.Type = ZDATA;
    Hdr.Pos  = (uint32_t)Sent;
    zm_send_bin32_header(Sp, &Hdr);

    /* Send file data in blocks */
    while ((n = (int)fread(Block, 1, ZM_BLOCK_SIZE, f)) > 0) {
        int SubType;                    /* subpacket type                */

        SubType = feof(f) ? ZCRCE : ZCRCG;
        zm_send_data(Sp, Block, n, SubType);
        Sent += n;

        /* Progress display */
        {
            time_t Elapsed = time(NULL) - StartTime;
            long   Cps     = (Elapsed > 0) ? Sent / Elapsed : 0;
            long   Remain  = FileSize - Sent;
            qf_log(LOG_DEBUG, "Bytes %ld/%ld  %ld CPS  %ld remaining",
                   Sent, FileSize, Cps, Remain);
        }

        /* Handle ZACK/ZNAK for CrcW/CrcQ subpackets */
        if (SubType == ZCRCW || SubType == ZCRCQ) {
            if (zm_recv_header(Sp, &Hdr, ZM_TIMEOUT) != 0) {
                Errors++;
                if (Errors >= ZM_MAX_ERRORS) {
                    qf_log(LOG_WARN,
                           "Maximum protocol error count reached");
                    break;
                }
            }

            if (Hdr.Type == ZRPOS) {
                qf_log(LOG_DEBUG, "Zmodem - bad file position");
                fseek(f, (long)Hdr.Pos, SEEK_SET);
                Sent = (long)Hdr.Pos;

                Hdr.Type = ZDATA;
                Hdr.Pos  = (uint32_t)Sent;
                zm_send_bin32_header(Sp, &Hdr);
            }
        }

        if (!ser_get_dcd(Sp)) {
            qf_log(LOG_WARN, "Lost carrier");
            break;
        }
    }

    fclose(f);

    /* Send ZEOF */
    Hdr.Type = ZEOF;
    Hdr.Pos  = (uint32_t)Sent;
    zm_send_hex_header(Sp, &Hdr);

    /* Wait for ZRINIT (ready for next file) */
    zm_recv_header(Sp, &Hdr, ZM_TIMEOUT);

    {
        time_t Elapsed = time(NULL) - StartTime;
        long   Cps     = (Elapsed > 0) ? Sent / Elapsed : 0;
        qf_log(LOG_INFO, "Successfully sent %s (%ld bytes, %ld CPS)",
               FileName, Sent, Cps);
    }

    return 0;
}


/*-----------------------------------------------------------------------*/
/* zm_send_zfin() -- Send ZFIN (end session) and wait for reply         */
/*-----------------------------------------------------------------------*/

int zm_send_zfin(SerPort *Sp)
{
    ZmHeader Hdr;                       /* protocol header               */

    memset(&Hdr, 0, sizeof(Hdr));
    Hdr.Type = ZFIN;
    zm_send_hex_header(Sp, &Hdr);

    if (zm_recv_header(Sp, &Hdr, ZM_TIMEOUT) == 0 &&
        Hdr.Type == ZFIN) {
        ser_write(Sp, "OO", 2);         /* Over and Out                  */
    }

    qf_log(LOG_INFO, "End of transmitted file");
    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                         Zmodem Receive                                    */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* zm_recv_file() -- Receive a single file via Zmodem batch protocol    */
/*                                                                       */
/* Implements the Zmodem receiver state machine:                         */
/*   1. Send ZRINIT (we're ready to receive)                             */
/*   2. Wait for ZFILE header (filename, size, timestamp)                */
/*   3. Send ZRPOS with file offset (0 for new, >0 for resume)          */
/*   4. Receive data subpackets, write to disk                           */
/*   5. On ZEOF, verify file size matches                                */
/*   6. Send ZRINIT to request next file                                 */
/*                                                                       */
/* Returns 0 on success (file received), -1 on error or no more files.  */
/*-----------------------------------------------------------------------*/

int zm_recv_file(SerPort *Sp, const char *InboundDir)
{
    ZmHeader      Hdr;                  /* protocol header               */
    FILE         *f = NULL;             /* output file handle            */
    char          Filepath[520];        /* full output path              */
    char          FileName[260];        /* parsed filename               */
    long          FileSize = 0;         /* expected file size            */
    long          Received = 0;         /* bytes received so far         */
    int           Errors = 0;           /* error counter                 */
    time_t        StartTime;            /* transfer start time           */

    qf_log(LOG_DEBUG, "zm_recv_file: inbound=%s", InboundDir);

    StartTime = time(NULL);

    /* Send ZRINIT */
    memset(&Hdr, 0, sizeof(Hdr));
    Hdr.Type = ZRINIT;
    Hdr.Pos  = CANFDX | CANOVIO | CANFC32;
    zm_send_hex_header(Sp, &Hdr);

    /* Wait for ZFILE or ZFIN */
    while (1) {
        if (zm_recv_header(Sp, &Hdr, ZM_TIMEOUT * 3) != 0) {
            Errors++;
            if (Errors >= ZM_MAX_ERRORS) {
                qf_log(LOG_WARN, "Zmodem - never got proper handshake");
                return -1;
            }
            /* Resend ZRINIT */
            Hdr.Type = ZRINIT;
            Hdr.Pos  = CANFDX | CANOVIO | CANFC32;
            zm_send_hex_header(Sp, &Hdr);
            continue;
        }

        if (Hdr.Type == ZFIN) {
            qf_log(LOG_INFO, "Zmodem - no files to receive");
            Hdr.Type = ZFIN;
            zm_send_hex_header(Sp, &Hdr);
            return 0;
        }

        if (Hdr.Type == ZFILE)
            break;
    }

    /* Read ZFILE data subpacket -- filename and size */
    {
        unsigned char Info[1024];       /* file info buffer              */
        int           InfoPos = 0;      /* buffer position               */
        int           Ch;               /* received character            */

        while (InfoPos < 1023) {
            Ch = zm_read_byte(Sp, ZM_TIMEOUT);
            if (Ch < 0) break;
            if (Ch == ZDLE) {
                Ch = ser_read_byte(Sp, ZM_TIMEOUT);
                if (Ch >= 'h' && Ch <= 'k') break;  /* subpacket end     */
            }
            Info[InfoPos++] = (unsigned char)Ch;
        }
        Info[InfoPos] = '\0';

        /* Parse: "filename\0size date ..." */
        strncpy(FileName, (char *)Info, sizeof(FileName) - 1);

        /* Sanitize: strip path components, reject ".." */
        {
            char *Slash;                /* last forward slash            */
            char *BSlash;               /* last backslash                */

            Slash  = strrchr(FileName, '/');
            BSlash = strrchr(FileName, '\\');
            if (BSlash > Slash) Slash = BSlash;
            if (Slash)
                memmove(FileName, Slash + 1, strlen(Slash + 1) + 1);
            if (strstr(FileName, "..") || FileName[0] == '\0') {
                qf_log(LOG_WARN, "Zmodem: rejecting unsafe filename");
                return -1;
            }
        }

        {
            const char *p;              /* size string pointer           */
            p = (const char *)Info + strlen(FileName) + 1;
            FileSize = atol(p);
        }
    }

    if (FileSize <= 0) {
        qf_log(LOG_WARN, "Zmodem: rejecting %s (invalid size %ld)",
               FileName, FileSize);
        Hdr.Type = ZSKIP;
        zm_send_hex_header(Sp, &Hdr);
        return -1;
    }

    qf_log(LOG_INFO, "Receiving %s (%ld bytes)", FileName, FileSize);

    /* Build output path */
    snprintf(Filepath, sizeof(Filepath), "%s%c%s",
             InboundDir, PATH_SEP, FileName);

    /* Check for existing file -- resume support */
    f = fopen(Filepath, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        Received = ftell(f);
        fclose(f);

        if (Received > 0 && Received < FileSize) {
            qf_log(LOG_INFO, "Resuming partial transfer at %ld", Received);
            f = fopen(Filepath, "ab");
        } else if (Received >= FileSize) {
            qf_log(LOG_INFO,
                   "Receiver skipped file %s (already complete)", FileName);
            Hdr.Type = ZSKIP;
            zm_send_hex_header(Sp, &Hdr);
            return 0;
        } else {
            f = fopen(Filepath, "wb");
            Received = 0;
        }
    } else {
        f = fopen(Filepath, "wb");
        Received = 0;
    }

    if (!f) {
        qf_log(LOG_WARN, "Error receiving %s (cannot create file)",
               FileName);
        Hdr.Type = ZSKIP;
        zm_send_hex_header(Sp, &Hdr);
        return -1;
    }

    /* Send ZRPOS (start/resume position) */
    Hdr.Type = ZRPOS;
    Hdr.Pos  = (uint32_t)Received;
    zm_send_hex_header(Sp, &Hdr);

    /* Receive data */
    while (1) {
        if (zm_recv_header(Sp, &Hdr, ZM_TIMEOUT) != 0) {
            Errors++;
            if (Errors >= ZM_MAX_ERRORS) {
                qf_log(LOG_WARN,
                       "Too many errors received during protocol");
                break;
            }
            continue;
        }

        if (Hdr.Type == ZEOF) {
            qf_log(LOG_DEBUG, "End of transmitted file");
            break;
        }

        if (Hdr.Type == ZDATA) {
            /* Receive data blocks until subpacket end */
            unsigned char Block[ZM_BLOCK_SIZE];  /* data block buffer     */
            int           BPos = 0;              /* block position        */
            int           Ch;                    /* received character    */

            while (BPos < ZM_BLOCK_SIZE) {
                Ch = zm_read_byte(Sp, ZM_TIMEOUT);
                if (Ch < 0) break;

                /* Check for subpacket terminator */
                if (Ch == ZDLE) {
                    Ch = ser_read_byte(Sp, ZM_TIMEOUT);
                    if (Ch >= 'h' && Ch <= 'k') {
                        /* CrcE/G/Q/W -- end of subpacket */
                        /* Read and discard CRC-32 */
                        zm_read_byte(Sp, ZM_TIMEOUT);
                        zm_read_byte(Sp, ZM_TIMEOUT);
                        zm_read_byte(Sp, ZM_TIMEOUT);
                        zm_read_byte(Sp, ZM_TIMEOUT);
                        break;
                    }
                    Ch ^= 0x40;         /* unescape                      */
                }

                Block[BPos++] = (unsigned char)Ch;
            }

            if (BPos > 0) {
                fwrite(Block, 1, BPos, f);
                Received += BPos;
            }
        }

        if (!ser_get_dcd(Sp)) {
            qf_log(LOG_WARN, "Lost carrier");
            break;
        }
    }

    fclose(f);

    /* Send ZRINIT to signal ready for next file */
    Hdr.Type = ZRINIT;
    Hdr.Pos  = CANFDX | CANOVIO | CANFC32;
    zm_send_hex_header(Sp, &Hdr);

    {
        time_t Elapsed = time(NULL) - StartTime;
        long   Cps     = (Elapsed > 0) ? Received / Elapsed : 0;

        if (Received >= FileSize)
            qf_log(LOG_INFO,
                   "Successfully received %s (%ld bytes, %ld CPS)",
                   FileName, Received, Cps);
        else
            qf_log(LOG_WARN,
                   "Attempt to receive file(s) was unsuccessful");
    }

    return (Received >= FileSize) ? 0 : -1;
}
