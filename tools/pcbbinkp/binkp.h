/* ====================================================================
 * binkp.h — BinkP/1.1 Protocol Definitions for PCBoard
 * ====================================================================
 * Clean-room implementation for pcbbinkp.exe
 * Reference: FTS-1026 (BinkP/1.0), FSP-1024 (CRAM-MD5 extension)
 *
 * Copyright (C) 2026 pcbrevival contributors
 * License: GPLv3
 * ==================================================================== */

#ifndef BINKP_H
#define BINKP_H

/* --------------------------------------------------------------------
 * BinkP Frame Format
 * --------------------------------------------------------------------
 * Every frame: 2-byte header (big-endian) + payload
 *   Bit 15 of header = type flag:
 *     0 = data frame (payload is file data)
 *     1 = command frame (first byte of payload = command ID)
 *   Bits 14..0 = payload length (0..32767)
 * -------------------------------------------------------------------- */

#define BINKP_MAX_BLOCK     32767       /* max payload per frame          */
#define BINKP_HDR_SIZE      2           /* frame header                   */
#define BINKP_FLAG_CMD      0x8000u     /* bit 15 = command frame         */
#define BINKP_DEFAULT_PORT  24554       /* IANA registered for BinkP      */

/* BinkP command IDs (FTS-1026 Table 1) */
#define M_NUL   0       /* site info / option negotiation             */
#define M_ADR   1       /* FTN address list                           */
#define M_PWD   2       /* session password (plain or CRAM-MD5)       */
#define M_FILE  3       /* file header: name size time offset          */
#define M_OK    4       /* password accepted (secure session)          */
#define M_EOB   5       /* end of batch                                */
#define M_GOT   6       /* file received OK: name size time            */
#define M_ERR   7       /* fatal error — session terminates            */
#define M_BSY   8       /* all addresses busy — try later              */
#define M_GET   9       /* request file from offset                    */
#define M_SKIP  10      /* skip this file for now                      */
#define M_COUNT 11      /* number of defined commands                  */

/* Session state machine */
typedef enum {
    STATE_INIT = 0,     /* pre-connection                              */
    STATE_WAIT_ADR,     /* sent our ADR, waiting for remote ADR        */
    STATE_WAIT_PWD,     /* got ADR, waiting for PWD (answering side)   */
    STATE_WAIT_OK,      /* sent PWD, waiting for OK/ERR (calling side) */
    STATE_AUTH_OK,      /* authenticated — ready for file transfer     */
    STATE_TRANSFER,     /* sending/receiving files                     */
    STATE_WAIT_EOB,     /* sent EOB, waiting for remote EOB            */
    STATE_DONE,         /* session complete                            */
    STATE_ERROR         /* unrecoverable error                         */
} BinkpState;

/* Session direction */
#define BINKP_ORIGINATE  0  /* we called them (calling/originating)    */
#define BINKP_ANSWER     1  /* they called us (answering)              */

/* NR/ND/CRYPT negotiation flags */
#define OPT_NR       0x0001 /* Non-Reliable mode                      */
#define OPT_ND       0x0002 /* No-Dupes mode                          */
#define OPT_CRYPT    0x0004 /* Session encryption (CRC32-based)        */
#define OPT_CRAM     0x0008 /* CRAM-MD5 authentication available       */

/* FTN address — 4D: zone:net/node.point */
typedef struct {
    unsigned short zone;
    unsigned short net;
    unsigned short node;
    unsigned short point;
} FtnAddr;

/* File transfer tracking */
typedef struct {
    char     name[256];     /* filename                                */
    long     size;          /* file size in bytes                      */
    long     time;          /* Unix timestamp                          */
    long     offset;        /* resume offset                           */
    FILE    *fp;            /* open file handle                        */
} BinkpFile;

/* Per-session state */
typedef struct {
    /* Connection */
    int           sock;             /* TCP socket                      */
    int           direction;        /* ORIGINATE or ANSWER             */
    BinkpState    state;            /* state machine                   */

    /* Identity */
    FtnAddr       local_addr;       /* our primary FTN address         */
    FtnAddr       remote_addr;      /* their primary FTN address       */
    char          password[64];     /* session password                */

    /* I/O buffers */
    unsigned char ibuf[BINKP_MAX_BLOCK + BINKP_HDR_SIZE + 1];
    int           isize;            /* bytes remaining to read         */
    int           iptr;             /* read position in ibuf           */
    unsigned char obuf[BINKP_MAX_BLOCK + BINKP_HDR_SIZE + 1];
    int           optr;             /* write position in obuf          */

    /* File transfer */
    BinkpFile     in_file;          /* file being received             */
    BinkpFile     out_file;         /* file being sent                 */
    int           rx_eob;           /* remote sent EOB                 */
    int           tx_eob;           /* we sent EOB                     */

    /* Authentication */
    unsigned short options;         /* negotiated OPT_ flags           */
    char          challenge[64];    /* CRAM-MD5 challenge string       */
    int           md5_ok;           /* CRAM-MD5 auth succeeded         */

    /* Statistics */
    long          bytes_sent;
    long          bytes_rcvd;
    int           files_sent;
    int           files_rcvd;

    /* BSO paths */
    char          outbound[260];    /* BSO outbound directory          */
    char          inbound[260];     /* inbound directory               */
    char          temp_inbound[260];/* temp inbound (partial files)    */
} BinkpSession;

/* --------------------------------------------------------------------
 * Function prototypes
 * -------------------------------------------------------------------- */

/* binkp.c — protocol core */
int  binkp_init(BinkpSession *s, int sock, int direction);
int  binkp_run(BinkpSession *s);
void binkp_cleanup(BinkpSession *s);

/* Frame I/O */
int  binkp_send_cmd(BinkpSession *s, int cmd, const char *data);
int  binkp_send_data(BinkpSession *s, const unsigned char *data, int len);
int  binkp_recv_frame(BinkpSession *s, int *is_cmd, int *cmd,
                      unsigned char *data, int *len);

/* Command handlers */
int  binkp_handle_nul(BinkpSession *s, const char *data);
int  binkp_handle_adr(BinkpSession *s, const char *data);
int  binkp_handle_pwd(BinkpSession *s, const char *data);
int  binkp_handle_file(BinkpSession *s, const char *data);
int  binkp_handle_ok(BinkpSession *s, const char *data);
int  binkp_handle_eob(BinkpSession *s);
int  binkp_handle_got(BinkpSession *s, const char *data);
int  binkp_handle_err(BinkpSession *s, const char *data);

/* binkpauth.c — CRAM-MD5 */
void binkp_make_challenge(char *buf, int bufsize);
int  binkp_build_digest(const char *password, const char *challenge,
                        char *out, int outsize);
int  binkp_verify_digest(const char *password, const char *challenge,
                         const char *digest);

/* FTN address helpers */
int  ftn_parse(const char *str, FtnAddr *addr);
int  ftn_format(const FtnAddr *addr, char *buf, int bufsize);
int  ftn_match(const FtnAddr *a, const FtnAddr *b);

/* BSO (Binkley-Style Outbound) scanning */
int  bso_scan_outbound(BinkpSession *s, const FtnAddr *remote);
int  bso_next_file(BinkpSession *s);
void bso_mark_sent(BinkpSession *s, const char *filename);

/* Tagged output for pcbis_ui */
void binkp_log(int level, const char *fmt, ...);

#endif /* BINKP_H */
