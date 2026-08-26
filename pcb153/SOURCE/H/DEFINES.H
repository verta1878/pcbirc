/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* The source code in this module is proprietary software belonging to       */
/* Clark Development Company and is part of the PCBoard source code library. */
/* You are granted the right to use this source code for the building of any */
/* of the PCBoard products you have licensed.  Any other usage is forbidden  */
/* without prior written consent from Clark Development Company, Inc.        */
/*                                                                           */
/* Be sure to read the source code license agreement before utilizing any    */
/* of the source code found herein.                                          */
/*                                                                           */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


#ifndef DEFINES_H
#define DEFINES_H

/****************************************************************************/
/* Version Numbers                                                          */

#define         VERSION_MAJOR   "15"
#define         VERSION_MINOR   "3"
#define         CONFIG_VERSION   2

/****************************************************************************/
/* Protocol Flag..                                                          */

#define   XMODEM        1
#define   ZMODEM        2

/****************************************************************************/
/* Queue flags.                                                             */

#define   Q_POLL        1
#define   Q_FILEREQ     2
#define   Q_KILLSENT    4
#define   Q_HOLD        8
#define   Q_CRASH       16
#define   Q_NORMAL      32
#define   Q_POLLED      64
#define   Q_FILESEND    128
#define   Q_OUTBOUND    256
#define   Q_FREQED      1024

/****************************************************************************/
/* Consts for the Event flags.                                              */

const   int  POLL        = 1;
const   int  CRASH       = 16;
const   int  HOLD        = 8;
const   int  ALLOW_ROUTE = 512;

/****************************************************************************/
/*Flags to identify queue_record fields                                     */
const   int  FLAG       = 0x01;
const   int  NODESTR    = 0x02;

/****************************************************************************/
/* Product Code info                                                        */

#define         W_PRODUCT_CODE          0x0D
#define         PROTCODE                0x0D
#define         PROD_FILE               "FTSCPROD.DAT"

/****************************************************************************/
/* Misc. Defines...                                                         */

#define         INBOUND                 1
#define         OUTBOUND                2
#define         END_OF_LINE             0xFF
#define         LINE_LENGTH             80
#define         FIDO_CONFERENCE         5         /* ConfType flag for fido */
#define         MAXLEN                  200
#define         NUM_ZONES               50
#define         ABORT_LEN               8
#define         ABORT_STRING           ""
#define         LINE_LEN                79
#define         HDR_SIZE                9
#define         MNU_TOPX                22  /* For FIDO display menus */
#define         MNU_TOPY                5
#define         MNU_BOTX                57
#define         MNU_BOTY                19

/****************************************************************************/
/* Return values for EMSI/YooHoo Detect functions                           */

#define         NOTHING_THERE           0
#define         NOT_OPUS                0
#define         WAZOO_CONNECT           1
#define         FIDO_CONNECT            2
#define         EMSI_CONNECT            3
#define         FIDO_SYS                4

/****************************************************************************/
/* Defines of different archive types supported by the program              */

#define     ZIP_ARCH            0               /* ZIP Archive          */
#define     ARJ_ARCH            1               /* ARJ Archive          */
#define     ARC_ARCH            2               /* ARC Archive          */
#define     LZH_ARCH            3               /* LZH Archive          */
#define     ARCH_UNKNOWN        -1              /* unknown archive type */
#define     ERROR               -2                                                      /* Error return                                 */

/****************************************************************************/
/* Filename defines.                                                        */

#define     CONFIG_FILE         PcbData.FidoConfig
#define     WORK_DIR            directory_info.work_directory
#define     IN_DIR              directory_info.incoming_packets
/*#define     QUEUE_FILE          "FIDOQUE.DAT"       /* Name of the Queue    */ */
/*#define     MASTER_QUEUE_FILE   "MASTER.DAT" */
#define     LOGFILE             "FIDOLOG.IN"        /* log file             */
#define     INCOMING_MASK       "*.PKT"             /* Packet files         */
/*#define     QUEUE_FILE          "FIDOQUE.DAT" */
#define     OUTBOUND_FILE       "PCBDSZ.LST"
#define     ZM_SEND             "ZMSEND.EXE"
#define     ZM_RECV             "ZMRECV.EXE"
#define     NODELIST_NDX        "NODELIST.NDX"
#define     TMP_NODE            "NODES.TMP"
#define     RESULT_FILE         "RESULTS.FIX"
#define     AREAFIX_HELP_FILE   "AREAFIX.HLP"
#define     F_LIMIT_FILE        "FREQ.DAT"

/****************************************************************************/
/* Keys...                                                                  */

#define         CARRIAGE_RETURN         13
#define         CR                      13
#define         LINE_FEED               10
#define         LF                      10
#define         ESC_KEY                 0x1b

/****************************************************************************/
/* These are the places where to write information                          */

#define START_COL       11
#define STATUS_LINE     4
#define MESSAGES        5
#define LAST_ERROR      6
#define LAST_COL        79

/****************************************************************************/
/* Connection states                                                        */

#define         CONNECTED               0
#define         BUSY                    1
#define         NO_MORE                 2
#define         NO_CONNECTION           3
#define         USER_ABORT              4

/****************************************************************************/
/* Fields in the NODELIST data line and what they mean                      */

#define         HEIRARCHY               0
#define         NUMBER                  1
#define         BBS_NAME                2
#define         LOCATION                3
#define         F_SYSOP                 4
#define         PHONE                   5
#define         SPEED                   6
#define         FLAGS                   7

/****************************************************************************/
/* Wazoo flags.                                                             */

#define         Y_DIETIFNA              0x0001  /* Can do fast LoTek    */
#define         FTB_USER                0x0002  /* Reserved by Opus-CBCS*/
#define         ZED_ZIPPER              0x0004  /* Does Zmodem - 1K     */
#define         ZED_ZAPPER              0x0008  /* Does Zmodem - variant*/
#define         DOES_JANUS              0x0010  /* Can do Janus         */
#define         Bit_5                   0x0020  /* Reserved by FTSC     */
#define         Bit_6                   0x0040  /* Reserved by FTSC     */
#define         Bit_7                   0x0080  /* Reserved by FTSC     */
#define         Bit_8                   0x0100  /* Reserved by FTSC     */
#define         Bit_9                   0x0200  /* Reserved by FTSC     */
#define         Bit_a                   0x0400  /* Reserved by FTSC     */
#define         Bit_b                   0x0800  /* Reserved by FTSC     */
#define         Bit_c                   0x1000  /* Reserved by FTSC     */
#define         Bit_d                   0x2000  /* Reserved by FTSC     */
#define         DO_DOMAIN               0x4000  /* Contains domain      */
#define         WZ_FREQ                 0x8000  /* WZ file req. ok      */

/****************************************************************************/
/* Connection states                                                        */

#define         CONNECTED               0
#define         BUSY                    1
#define         NO_MORE                 2
#define         NO_CONNECTION           3
#define         USER_ABORT              4

/****************************************************************************/
/* Fido message attribute bits                                                     */

#define         MSG_PRIVATE           0x0001
#define         MSG_CRASH             0x0002
#define         MSG_RECEIVED          0x0004
#define         MSG_SENT              0x0008
#define         MSG_FILEATT           0x0010
#define         MSG_IN_TRANSIT        0x0020
#define         MSG_ORPHAN            0x0040
#define         MSG_KILLSENT          0x0080
#define         MSG_LOCAL             0x0100
#define         MSG_HOLD              0x0200
#define         UNUSED                0x0400          /* this one is not used */
#define         MSG_FILEREQ     0x0800
#define         MSG_RETRECREQ   0x1000
#define         MSG_ISRETREC    0x2000
#define         MSG_AUDITREQ    0x4000
#define         MSG_FILEUPDATE  0x8000

#define                 MAX_ARCHIVERS           4

/*****************************************************************************/

#define         LMR_FILE    "PCBFIDO.LMR"       /* LMR Pointer File     */

        /* Fido Imbedded Message Text Information     */

#define         HARD_RETURN     0x0D            /* must PRESERVE these  */
#define         SOFT_RETURN     0x8D            /* can throw these out  */
#define         MSG_COMMAND     0x01            /* begin control info   */

        /* PCBoard Special Functions    */

#define         PCB_RETURN      0xE3            /* PCBoard carriage return */

        /* Sizes of various records     */

#define         FIDO_MSG_HDR_SIZE       sizeof(FIDO_MESSAGE_HDR)
#define         FIDO_FIXED_SIZE         34
#define         NUM_EXPORTS             20   /* Number of Exports allowed  */
#define         LINE_LEN                79
#define         HDR_SIZE                9

/****************************************************************************/
/* Protocol commands                                                        */

#define         SOH               0x01
#define         SYN               0x16
#define         STX               0x02
#define         EOT               0x04
#define         ENQ               0x05
#define         ACK               0x06
#define         NAK               0x15
#define         YOOHOO            0xf1
#define         HEADER            0x1f
#define         TSYNCH            0xae
#define         HELLO_SIZE        128
#define         LINE_STATUS       "8N1,PUA"
#define         COMPAT_CODES      "ZMO,XMA,FNC"
#define         PRODUCT_CODE      "0D"
#define         PRODUCT_NAME      "PCBoard"
#define         SERIAL_NUMBER     "9902131"

/****************************************************************************/
/* Connection states                                                        */

#define         CONNECTED               0
#define         BUSY                    1
#define         NO_MORE                 2
#define         NO_CONNECTION           3
#define         USER_ABORT              4

/****************************************************************************/
/* FCONFIG.C control flags.                                                 */

#define     AREA_RECS     1
#define     ADDR_RECS     2
#define     NODE_RECS     4
#define     XLAT_RECS     8
#define     LIST_RECS     16
#define     FREQ_RECS     32
#define     MAGIC_RECS    64
#define     DENY_RECS     128


/****************************************************************************/
/* Logging flags                                                            */

#define SEND 1
#define RECV 2
#define FREQ 3
#define BLOCK_START 4
#define BLOCK_END   5
#define BLOCK       6


#define MAX_FIDO_USERS 100
#define FROM_MENU      1
#define FROM_TIMER     2
#define NUMPKTS        100

#define AREA_SIZE      60

/***************************************************************************/
/* Types of message responses */

#define FREQFAIL     0
#define FREQINFO     1
#define AFIXFAIL     2
#define AFIXLIST     3
#define AFIXHELP     4
#define AFIXAVAL     5
#define AFIXRESPONSE 6
#define AFIXFORWARD  7

/* Maximum file.directory length */
#define MAXFLEN      66  /* 66 from the DOS version of Borland MAXDIR */

#define SEENBYS      1
#define PRESENT      2
#endif
