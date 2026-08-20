/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* wazoo.c -- WaZOO/YooHoo Session Negotiation (FTS-0006)                   */
/*                                                                           */
/* Implements the YooHoo handshake for FidoNet session negotiation.          */
/* Used as fallback when EMSI isn't supported by the remote system.          */
/*                                                                           */
/* Protocol: exchange 128-byte "hello" packets with CRC-16.                  */
/*                                                                           */
/* From binary:                                                              */
/*   "Incoming YooHoo"                                                      */
/*   "Established YooHoo protocol"                                          */
/*   "Sending/Sent/Receiving/Received hello packet"                         */
/*   "Bad CRC value in hello packet"                                        */
/*   "YooHoo capabilities = <hex>"                                          */
/*   "Unable to initialize WaZOO protocol"                                  */
/*                                                                           */
/* Clean-room from FTS-0006 (public FidoNet specification).                  */
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


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*              YooHoo Hello Packet (FTS-0006 Section 3)                     */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* 128 bytes, little-endian, with CRC-16 at the end. */

#pragma pack(push, 1)
typedef struct {
    uint16_t Signal;                    /* 0x6F6F = "oo" (YooHoo)        */
    uint16_t HelloVersion;              /* 1                             */
    uint16_t ProductCode;               /* mailer product code           */
    uint16_t SerialNumber;              /* registration serial           */
    char     Sysop[20];                 /* sysop name                    */
    uint16_t MyZone;                    /* our zone number               */
    uint16_t MyNet;                     /* our net number                */
    uint16_t MyNode;                    /* our node number               */
    uint16_t MyPoint;                   /* our point number              */
    char     MyPassword[8];             /* session password              */
    uint8_t  Reserved1[8];              /* reserved (zero)               */
    uint16_t Capabilities;              /* capability flags              */
    uint8_t  Reserved2[12];             /* reserved (zero)               */
    char     SystemName[58];            /* system/BBS name               */
    uint16_t Crc;                       /* CRC-16 of bytes 0-125         */
} YooHooHello;                          /* 128 bytes total               */
#pragma pack(pop)

/* YooHoo signal bytes */
#define YOOHOO_SIGNAL   0x6F6F          /* "oo"                          */
#define YOOHOO_VERSION  1               /* hello packet version          */

/* Capability flags (FTS-0006 Section 4) */
#define Y_DIETIFNA   0x0001             /* can do FTS-0001 (basic)       */
#define Y_CAN_EMSI   0x0002             /* EMSI capable -- not used here */
#define Y_ZED_ZMODEM 0x0004             /* Zmodem capable                */
#define Y_ZED_ZIPPER 0x0008             /* ZedZip capable                */
#define Y_JANUS      0x0010             /* Janus bidirectional           */
#define Y_HYDRA      0x0020             /* Hydra bidirectional           */

#define YOOHOO_MAX_RETRIES 10           /* max handshake attempts        */
#define YOOHOO_TIMEOUT_MS  20000        /* hello packet timeout (ms)     */


/*-----------------------------------------------------------------------*/
/* yh_crc16() -- CRC-16/CCITT (same as EMSI)                            */
/*                                                                       */
/* Polynomial 0x1021, initial value 0x0000. Used to validate hello       */
/* packets. CRC covers bytes 0-125 (everything except the CRC field).   */
/*-----------------------------------------------------------------------*/

static uint16_t yh_crc16(const void *Data, int Len)
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
/* yh_build_hello() -- Build a 128-byte YooHoo hello packet              */
/*                                                                       */
/* FTS-0006 Section 3: The hello packet contains our identity --         */
/* address, sysop name, system name, session password, and capability    */
/* flags (what protocols we support for file transfer).                  */
/*                                                                       */
/* The packet is exactly 128 bytes with CRC-16 at bytes 126-127.        */
/* All multi-byte fields are little-endian (Intel byte order).           */
/*                                                                       */
/* Product code 0xFE = "Unknown/custom" -- registered mailers have       */
/* assigned codes (e.g. BinkleyTerm=0x11, FrontDoor=0x19).              */
/*                                                                       */
/* Capability flags we advertise:                                        */
/*   Y_DIETIFNA (0x01)   -- can do basic FTS-0001 sessions              */
/*   Y_ZED_ZMODEM (0x04) -- can do Zmodem file transfer                 */
/*-----------------------------------------------------------------------*/

static void yh_build_hello(YooHooHello *Hello, const FTN_ADDR *Addr,
                            const char *SysopName, const char *SysName,
                            const char *Password, uint16_t Caps)
{
    memset(Hello, 0, sizeof(*Hello));
    qf_log(LOG_DEBUG, "yh_build_hello: %d:%d/%d.%d caps=0x%04X",
           Addr->zone, Addr->net, Addr->node, Addr->point, Caps);

    Hello->Signal       = YOOHOO_SIGNAL;
    Hello->HelloVersion = YOOHOO_VERSION;
    Hello->ProductCode  = 0xFE;         /* "Unknown/custom"              */
    Hello->MyZone       = Addr->zone;
    Hello->MyNet        = Addr->net;
    Hello->MyNode       = Addr->node;
    Hello->MyPoint      = Addr->point;
    Hello->Capabilities = Caps;

    strncpy(Hello->Sysop, SysopName, sizeof(Hello->Sysop) - 1);
    strncpy(Hello->SystemName, SysName, sizeof(Hello->SystemName) - 1);

    if (Password && Password[0])
        strncpy(Hello->MyPassword, Password, sizeof(Hello->MyPassword));

    /* CRC-16 over bytes 0-125 (everything except the CRC field) */
    Hello->Crc = yh_crc16(Hello, 126);
}


/*-----------------------------------------------------------------------*/
/* yh_send_hello() -- Send hello packet over serial port                 */
/*                                                                       */
/* Sends ENQ (0x05) to signal YooHoo start, then the 128-byte packet.   */
/*                                                                       */
/* Returns 0 on success.                                                 */
/*-----------------------------------------------------------------------*/

static int yh_send_hello(SerPort *Sp, const YooHooHello *Hello)
{
    unsigned char Enq = 0x05;           /* YooHoo start signal           */

    qf_log(LOG_DEBUG, "Sending hello packet");
    ser_write(Sp, &Enq, 1);
    ser_write(Sp, Hello, sizeof(*Hello));
    qf_log(LOG_DEBUG, "Sent hello packet");

    return 0;
}


/*-----------------------------------------------------------------------*/
/* yh_recv_hello() -- Receive hello packet from serial port              */
/*                                                                       */
/* Waits for ENQ (0x05), then reads 128 bytes. Validates the signal      */
/* field (must be 0x6F6F) and CRC-16.                                    */
/*                                                                       */
/* Returns 0 on success, -1 on timeout, -2 on carrier loss.             */
/*-----------------------------------------------------------------------*/

static int yh_recv_hello(SerPort *Sp, YooHooHello *Hello, int TimeoutMs)
{
    int            i;                   /* byte read index               */
    int            Ch;                  /* received byte                 */
    unsigned char *Buf;                 /* hello packet as byte array    */
    uint16_t       CalcCrc;             /* calculated CRC for validation */

    Buf = (unsigned char *)Hello;
    qf_log(LOG_DEBUG, "Receiving hello packet");

    /* Wait for ENQ (0x05) */
    while (1) {
        Ch = ser_read_byte(Sp, TimeoutMs);
        if (Ch < 0) {
            qf_log(LOG_WARN, "Timeout receiving hello packet");
            return -1;
        }
        if (Ch == 0x05) break;          /* got ENQ                       */

        if (!ser_get_dcd(Sp)) {
            qf_log(LOG_WARN, "Lost carrier receiving hello packet");
            return -2;
        }
    }

    /* Read 128 bytes */
    for (i = 0; i < 128; i++) {
        Ch = ser_read_byte(Sp, 5000);
        if (Ch < 0) {
            qf_log(LOG_WARN, "Timeout in hello packet at byte %d", i);
            return -1;
        }
        Buf[i] = (unsigned char)Ch;
    }

    /* Validate signal */
    if (Hello->Signal != YOOHOO_SIGNAL) {
        qf_log(LOG_WARN, "Invalid YooHoo signal: 0x%04X", Hello->Signal);
        return -1;
    }

    /* Validate CRC */
    CalcCrc = yh_crc16(Hello, 126);
    if (CalcCrc != Hello->Crc) {
        qf_log(LOG_WARN, "Bad CRC value in hello packet "
               "(got %04X, expected %04X)", Hello->Crc, CalcCrc);
        return -1;
    }

    qf_log(LOG_DEBUG, "Received hello packet: %d:%d/%d.%d \"%s\" (\"%s\")",
           Hello->MyZone, Hello->MyNet, Hello->MyNode, Hello->MyPoint,
           Hello->SystemName, Hello->Sysop);
    qf_log(LOG_DEBUG, "  caps=0x%04X product=0x%02X version=%d",
           Hello->Capabilities, Hello->ProductCode, Hello->HelloVersion);

    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                     YooHoo Handshake (Caller Side)                        */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* wazoo_handshake_caller() -- Initiate YooHoo handshake as caller       */
/*                                                                       */
/* We send our hello first, then wait for theirs. Retries up to          */
/* YOOHOO_MAX_RETRIES times. On success, the remote system's address,    */
/* sysop name, and system name are returned.                             */
/*                                                                       */
/* Returns 0 on success, -1 on failure.                                  */
/*-----------------------------------------------------------------------*/

int wazoo_handshake_caller(SerPort *Sp,
                            const FTN_ADDR *OurAddr,
                            const char *SysopName,
                            const char *SysName,
                            const char *Password,
                            FTN_ADDR *RemoteAddr,
                            char *RemoteSysop, int SysopBufSize,
                            char *RemoteSystem, int SystemBufSize)
{
    YooHooHello OurHello;               /* our outgoing hello packet     */
    YooHooHello TheirHello;             /* their incoming hello packet   */
    int         Attempt;                /* retry counter                 */

    qf_log(LOG_INFO, "Establishing FidoMail handshake (YooHoo)");

    /* Build our hello packet */
    yh_build_hello(&OurHello, OurAddr, SysopName, SysName,
                    Password, Y_DIETIFNA | Y_ZED_ZMODEM);

    /* Send our hello, wait for theirs */
    for (Attempt = 0; Attempt < YOOHOO_MAX_RETRIES; Attempt++) {
        yh_send_hello(Sp, &OurHello);

        if (yh_recv_hello(Sp, &TheirHello, YOOHOO_TIMEOUT_MS) == 0)
            break;

        if (!ser_get_dcd(Sp)) {
            qf_log(LOG_WARN, "Lost carrier initiating outgoing YooHoo");
            return -1;
        }
    }

    if (Attempt >= YOOHOO_MAX_RETRIES) {
        qf_log(LOG_WARN, "Max retries exceeded initiating outgoing YooHoo");
        qf_log(LOG_WARN, "Unable to initialize WaZOO protocol");
        return -1;
    }

    /* Extract remote info */
    RemoteAddr->zone  = TheirHello.MyZone;
    RemoteAddr->net   = TheirHello.MyNet;
    RemoteAddr->node  = TheirHello.MyNode;
    RemoteAddr->point = TheirHello.MyPoint;

    strncpy(RemoteSysop, TheirHello.Sysop, SysopBufSize - 1);
    strncpy(RemoteSystem, TheirHello.SystemName, SystemBufSize - 1);

    /* Check password */
    if (Password && Password[0]) {
        if (strncmp(TheirHello.MyPassword, Password, 8) != 0) {
            qf_log(LOG_WARN, "[*** Invalid session password! ***]");
            qf_log(LOG_WARN, "[*** Check security setup! ***]");
        } else {
            qf_log(LOG_INFO, "Secured (password protected) mail session");
        }
    }

    {
        char AddrBuf[64];               /* formatted address for log     */

        ftn_format_addr(RemoteAddr, AddrBuf, sizeof(AddrBuf));
        qf_log(LOG_INFO, "YooHoo capabilities = %04X",
               TheirHello.Capabilities);
        qf_log(LOG_INFO, "Established YooHoo protocol with %s (%s)",
               RemoteSystem, AddrBuf);
    }

    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                    YooHoo Handshake (Answerer Side)                       */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* wazoo_handshake_answer() -- Accept YooHoo handshake as answerer       */
/*                                                                       */
/* We wait for their hello first, then send ours. This is the reverse    */
/* of the caller side. Used when we answered the phone and detected      */
/* a YooHoo signal.                                                      */
/*                                                                       */
/* Returns 0 on success, -1 on failure.                                  */
/*-----------------------------------------------------------------------*/

int wazoo_handshake_answer(SerPort *Sp,
                            const FTN_ADDR *OurAddr,
                            const char *SysopName,
                            const char *SysName,
                            const char *Password,
                            FTN_ADDR *RemoteAddr,
                            char *RemoteSysop, int SysopBufSize,
                            char *RemoteSystem, int SystemBufSize)
{
    YooHooHello OurHello;               /* our outgoing hello packet     */
    YooHooHello TheirHello;             /* their incoming hello packet   */
    int         Attempt;                /* retry counter                 */

    qf_log(LOG_INFO, "Incoming YooHoo");

    yh_build_hello(&OurHello, OurAddr, SysopName, SysName,
                    Password, Y_DIETIFNA | Y_ZED_ZMODEM);

    /* Wait for their hello first */
    for (Attempt = 0; Attempt < YOOHOO_MAX_RETRIES; Attempt++) {
        if (yh_recv_hello(Sp, &TheirHello, YOOHOO_TIMEOUT_MS) == 0)
            break;

        if (!ser_get_dcd(Sp)) {
            qf_log(LOG_WARN, "Lost carrier initiating incoming YooHoo");
            return -1;
        }
    }

    if (Attempt >= YOOHOO_MAX_RETRIES) {
        qf_log(LOG_WARN,
               "Max retries exceeded initiating incoming YooHoo");
        return -1;
    }

    /* Send our hello */
    yh_send_hello(Sp, &OurHello);

    /* Extract remote info */
    RemoteAddr->zone  = TheirHello.MyZone;
    RemoteAddr->net   = TheirHello.MyNet;
    RemoteAddr->node  = TheirHello.MyNode;
    RemoteAddr->point = TheirHello.MyPoint;

    strncpy(RemoteSysop, TheirHello.Sysop, SysopBufSize - 1);
    strncpy(RemoteSystem, TheirHello.SystemName, SystemBufSize - 1);

    if (Password && Password[0]) {
        if (strncmp(TheirHello.MyPassword, Password, 8) != 0) {
            qf_log(LOG_WARN, "[*** Invalid session password! ***]");
        } else {
            qf_log(LOG_INFO, "Secured (password protected) mail session");
        }
    }

    {
        char AddrBuf[64];               /* formatted address for log     */

        ftn_format_addr(RemoteAddr, AddrBuf, sizeof(AddrBuf));
        qf_log(LOG_INFO, "YooHoo capabilities = %04X",
               TheirHello.Capabilities);
        qf_log(LOG_INFO, "Established YooHoo protocol with %s (%s)",
               RemoteSystem, AddrBuf);
    }

    return 0;
}
