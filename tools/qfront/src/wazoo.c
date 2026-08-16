/* ====================================================================
 * wazoo.c — WaZOO/YooHoo Session Negotiation (FTS-0006)
 * ====================================================================
 * Implements the YooHoo handshake for FidoNet session negotiation.
 * Used as fallback when EMSI isn't supported by the remote system.
 *
 * Protocol: exchange 128-byte "hello" packets with CRC-16.
 *
 * From binary:
 *   "Incoming YooHoo"
 *   "Established YooHoo protocol"
 *   "Sending/Sent/Receiving/Received hello packet"
 *   "Bad CRC value in hello packet"
 *   "YooHoo capabilities = <hex>"
 *   "Unable to initialize WaZOO protocol"
 *
 * Clean-room from FTS-0006 (public FidoNet specification).
 * ==================================================================== */

#include "qfront.h"

/* Forward declarations — serial.c */
typedef struct SerPort SerPort;
extern int  ser_read_byte(SerPort *sp, int timeout_ms);
extern int  ser_write(SerPort *sp, const void *buf, int len);
extern int  ser_write_str(SerPort *sp, const char *str);
extern int  ser_get_dcd(SerPort *sp);

/* ---- YooHoo Hello Packet (FTS-0006 Section 3) ----
 * 128 bytes, little-endian, with CRC-16 at the end. */

#pragma pack(push, 1)
typedef struct {
    uint16_t signal;              /* 0x6F 0x6F = "oo" (YooHoo)   */
    uint16_t hello_version;       /* 1                           */
    uint16_t product_code;        /* Mailer product code          */
    uint16_t serial_number;       /* Registration serial          */
    char     sysop[20];           /* Sysop name                   */
    uint16_t my_zone;             /* Our zone number              */
    uint16_t my_net;              /* Our net number               */
    uint16_t my_node;             /* Our node number              */
    uint16_t my_point;            /* Our point number             */
    char     my_password[8];      /* Session password              */
    uint8_t  reserved1[8];        /* Reserved (zero)              */
    uint16_t capabilities;        /* Capability flags             */
    uint8_t  reserved2[12];       /* Reserved (zero)              */
    char     system_name[58];     /* System/BBS name              */
    uint16_t crc;                 /* CRC-16 of bytes 0-125        */
} YooHooHello;                    /* 128 bytes total              */
#pragma pack(pop)

/* YooHoo signal bytes */
#define YOOHOO_SIGNAL   0x6F6F    /* "oo"                        */
#define YOOHOO_VERSION  1

/* Capability flags (FTS-0006 Section 4) */
#define Y_DIETIFNA  0x0001        /* Can do FTS-0001 (basic)      */
#define Y_CAN_EMSI  0x0002        /* EMSI capable — not used here */
#define Y_ZED_ZMODEM 0x0004       /* Zmodem capable               */
#define Y_ZED_ZIPPER 0x0008       /* ZedZip capable               */
#define Y_JANUS     0x0010        /* Janus bidirectional           */
#define Y_HYDRA     0x0020        /* Hydra bidirectional           */

#define YOOHOO_MAX_RETRIES 10
#define YOOHOO_TIMEOUT_MS  20000


/* ---- CRC-16 (same as EMSI) ---- */

static uint16_t yh_crc16(const void *data, int len)
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


/* ---- Build Hello Packet ---- */

/*-----------------------------------------------------------------------*/
/* yh_build_hello() — Build a 128-byte YooHoo hello packet               */
/*                                                                         */
/* FTS-0006 Section 3: The hello packet contains our identity —          */
/* address, sysop name, system name, session password, and capability    */
/* flags (what protocols we support for file transfer).                  */
/*                                                                         */
/* The packet is exactly 128 bytes with CRC-16 at bytes 126-127.         */
/* All multi-byte fields are little-endian (Intel byte order).           */
/*                                                                         */
/* Product code 0xFE = "Unknown/custom" — registered mailers have        */
/* assigned codes (e.g. BinkleyTerm=0x11, FrontDoor=0x19).              */
/*                                                                         */
/* Capability flags we advertise:                                         */
/*   Y_DIETIFNA (0x01)  — can do basic FTS-0001 sessions                */
/*   Y_ZED_ZMODEM (0x04) — can do Zmodem file transfer                  */
/*-----------------------------------------------------------------------*/

static void yh_build_hello(YooHooHello *hello, const FTN_ADDR *addr,
                            const char *sysop, const char *system_name,
                            const char *password, uint16_t caps)
{
    memset(hello, 0, sizeof(*hello));
    qf_log(LOG_DEBUG, "yh_build_hello: %d:%d/%d.%d caps=0x%04X",
           addr->zone, addr->net, addr->node, addr->point, caps);

    hello->signal       = YOOHOO_SIGNAL;
    hello->hello_version = YOOHOO_VERSION;
    hello->product_code = 0xFE;   /* "Unknown/custom"            */
    hello->my_zone      = addr->zone;
    hello->my_net       = addr->net;
    hello->my_node      = addr->node;
    hello->my_point     = addr->point;
    hello->capabilities = caps;

    strncpy(hello->sysop, sysop, sizeof(hello->sysop) - 1);
    strncpy(hello->system_name, system_name, sizeof(hello->system_name) - 1);

    if (password && password[0])
        strncpy(hello->my_password, password, sizeof(hello->my_password));

    /* CRC-16 over bytes 0-125 (everything except the CRC field) */
    hello->crc = yh_crc16(hello, 126);
}


/* ---- Send Hello Packet ---- */

static int yh_send_hello(SerPort *sp, const YooHooHello *hello)
{
    qf_log(LOG_DEBUG, "Sending hello packet");

    /* Send the ENQ (0x05) to signal YooHoo start */
    {
        unsigned char enq = 0x05;
        ser_write(sp, &enq, 1);
    }

    /* Send the 128-byte hello packet */
    ser_write(sp, hello, sizeof(*hello));

    qf_log(LOG_DEBUG, "Sent hello packet");
    return 0;
}


/* ---- Receive Hello Packet ---- */

static int yh_recv_hello(SerPort *sp, YooHooHello *hello, int timeout_ms)
{
    int i, ch;
    unsigned char *buf = (unsigned char *)hello;
    uint16_t calc_crc;

    qf_log(LOG_DEBUG, "Receiving hello packet");

    /* Wait for ENQ (0x05) */
    while (1) {
        ch = ser_read_byte(sp, timeout_ms);
        if (ch < 0) {
            qf_log(LOG_WARN, "Timeout receiving hello packet");
            return -1;
        }
        if (ch == 0x05) break;    /* Got ENQ                     */

        if (!ser_get_dcd(sp)) {
            qf_log(LOG_WARN, "Lost carrier receiving hello packet");
            return -2;
        }
    }

    /* Read 128 bytes */
    for (i = 0; i < 128; i++) {
        ch = ser_read_byte(sp, 5000);
        if (ch < 0) {
            qf_log(LOG_WARN, "Timeout in hello packet at byte %d", i);
            return -1;
        }
        buf[i] = (unsigned char)ch;
    }

    /* Validate signal */
    if (hello->signal != YOOHOO_SIGNAL) {
        qf_log(LOG_WARN, "Invalid YooHoo signal: 0x%04X", hello->signal);
        return -1;
    }

    /* Validate CRC */
    calc_crc = yh_crc16(hello, 126);
    if (calc_crc != hello->crc) {
        qf_log(LOG_WARN, "Bad CRC value in hello packet "
               "(got %04X, expected %04X)", hello->crc, calc_crc);
        return -1;
    }

    qf_log(LOG_DEBUG, "Received hello packet: %d:%d/%d.%d \"%s\" (\"%s\")",
           hello->my_zone, hello->my_net, hello->my_node, hello->my_point,
           hello->system_name, hello->sysop);
    qf_log(LOG_DEBUG, "  caps=0x%04X product=0x%02X version=%d",
           hello->capabilities, hello->product_code, hello->hello_version);
    return 0;
}


/* ---- YooHoo Handshake (Caller) ---- */

int wazoo_handshake_caller(SerPort *sp,
                            const FTN_ADDR *our_addr,
                            const char *sysop,
                            const char *system_name,
                            const char *password,
                            FTN_ADDR *remote_addr,
                            char *remote_sysop, int sysop_size,
                            char *remote_system, int system_size)
{
    YooHooHello our_hello, their_hello;
    int attempt;

    qf_log(LOG_INFO, "Establishing FidoMail handshake (YooHoo)");

    /* Build our hello packet */
    yh_build_hello(&our_hello, our_addr, sysop, system_name,
                    password, Y_DIETIFNA | Y_ZED_ZMODEM);

    /* Send our hello, wait for theirs */
    for (attempt = 0; attempt < YOOHOO_MAX_RETRIES; attempt++) {
        yh_send_hello(sp, &our_hello);

        if (yh_recv_hello(sp, &their_hello, YOOHOO_TIMEOUT_MS) == 0)
            break;

        if (!ser_get_dcd(sp)) {
            qf_log(LOG_WARN, "Lost carrier initiating outgoing YooHoo");
            return -1;
        }
    }

    if (attempt >= YOOHOO_MAX_RETRIES) {
        qf_log(LOG_WARN, "Max retries exceeded initiating outgoing YooHoo");
        qf_log(LOG_WARN, "Unable to initialize WaZOO protocol");
        return -1;
    }

    /* Extract remote info */
    remote_addr->zone  = their_hello.my_zone;
    remote_addr->net   = their_hello.my_net;
    remote_addr->node  = their_hello.my_node;
    remote_addr->point = their_hello.my_point;

    strncpy(remote_sysop, their_hello.sysop, sysop_size - 1);
    strncpy(remote_system, their_hello.system_name, system_size - 1);

    /* Check password */
    if (password && password[0]) {
        if (strncmp(their_hello.my_password, password, 8) != 0) {
            qf_log(LOG_WARN, "[*** Invalid session password! ***]");
            qf_log(LOG_WARN, "[*** Check security setup! ***]");
        } else {
            qf_log(LOG_INFO, "Secured (password protected) mail session");
        }
    }

    {
        char buf[64];
        ftn_format_addr(remote_addr, buf, sizeof(buf));
        qf_log(LOG_INFO, "YooHoo capabilities = %04X",
               their_hello.capabilities);
        qf_log(LOG_INFO, "Established YooHoo protocol with %s (%s)",
               remote_system, buf);
    }

    return 0;
}


/* ---- YooHoo Handshake (Answerer) ---- */

int wazoo_handshake_answer(SerPort *sp,
                            const FTN_ADDR *our_addr,
                            const char *sysop,
                            const char *system_name,
                            const char *password,
                            FTN_ADDR *remote_addr,
                            char *remote_sysop, int sysop_size,
                            char *remote_system, int system_size)
{
    YooHooHello our_hello, their_hello;
    int attempt;

    qf_log(LOG_INFO, "Incoming YooHoo");

    yh_build_hello(&our_hello, our_addr, sysop, system_name,
                    password, Y_DIETIFNA | Y_ZED_ZMODEM);

    /* Wait for their hello first */
    for (attempt = 0; attempt < YOOHOO_MAX_RETRIES; attempt++) {
        if (yh_recv_hello(sp, &their_hello, YOOHOO_TIMEOUT_MS) == 0)
            break;

        if (!ser_get_dcd(sp)) {
            qf_log(LOG_WARN, "Lost carrier initiating incoming YooHoo");
            return -1;
        }
    }

    if (attempt >= YOOHOO_MAX_RETRIES) {
        qf_log(LOG_WARN, "Max retries exceeded initiating incoming YooHoo");
        return -1;
    }

    /* Send our hello */
    yh_send_hello(sp, &our_hello);

    /* Extract remote info */
    remote_addr->zone  = their_hello.my_zone;
    remote_addr->net   = their_hello.my_net;
    remote_addr->node  = their_hello.my_node;
    remote_addr->point = their_hello.my_point;

    strncpy(remote_sysop, their_hello.sysop, sysop_size - 1);
    strncpy(remote_system, their_hello.system_name, system_size - 1);

    if (password && password[0]) {
        if (strncmp(their_hello.my_password, password, 8) != 0) {
            qf_log(LOG_WARN, "[*** Invalid session password! ***]");
        } else {
            qf_log(LOG_INFO, "Secured (password protected) mail session");
        }
    }

    {
        char buf[64];
        ftn_format_addr(remote_addr, buf, sizeof(buf));
        qf_log(LOG_INFO, "YooHoo capabilities = %04X",
               their_hello.capabilities);
        qf_log(LOG_INFO, "Established YooHoo protocol with %s (%s)",
               remote_system, buf);
    }

    return 0;
}
