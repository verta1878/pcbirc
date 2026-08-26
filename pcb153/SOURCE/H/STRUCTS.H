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


#ifndef STRUCTS_DOT_H
#define STRUCTS_DOT_H

#include "misc.h"
#include "dosfunc.h"
#include "defines.h"

#ifdef __BORLANDC__
  #include <dir.h>
#else
  #include <direct.h>
#endif

#include "types.hpp"

#define PacketType sint


/* For capabilities flags. */
typedef struct
{
    bool ZAP;
    bool ZMO;
    bool ARC;
    bool XMA;
    bool FNC;
    bool JAN;
    bool KER;
    bool NCP;
    bool NRQ;
}CAPABILITIES;

typedef struct
{
  char  syn;
  char  block;
  char  compblock;
  long  file_length;
  sint   date;
  sint  time;
  char  filename[16];
  char  header_version;
  char  send_prog[16];
  char  crc_mode;
  char  fill[86];
  uint  crc;
/*  char  crc1; */
/*  char  crc2; */
/*char  checksum; */
}TELINK_BLOCK;

typedef struct
{
  sint           signal;                 /* always 'o' (0x6f)    */
  sint           hello_version;          /* currently 1 (0x01)   */
  sint           product;                /* product code         */
  sint           product_maj;            /* major revision       */
  sint           product_min;            /* minor revision       */
  char          bbs_name[60];           /* Other end's name     */
  char          sysop[20];              /* sysop's name         */
  sint           my_zone;                /* 0== not supported    */
  sint           my_net;                 /* our net number       */
  sint           my_node;                /* our node             */
  sint           my_point;               /* 0== not supported    */
  char          my_password[8];         /* only 6 are sig.      */
  char          reserved2[8];           /* reserved by Opus     */
  uint          capabilities;           /* see below            */
  char          reserved3[12];          /* for non-opus         */
} HELLO_T;

#pragma pack(1)
typedef struct
{
  sint           Orig_Node;              /* originating node of packet   */
  sint           Dest_Node;              /* destination node of packet   */
  sint           Year;                   /* year packet was created      */
  sint           Month;                  /* month packet was created     */
  sint           Day;                    /* day packet was created       */
  sint           Hour;                   /* hour packet was created      */
  sint           Minute;                 /* minute packet was created    */
  sint           Second;                 /* second packet was created    */
  sint           Baud;                   /* max baud rate of orig/dest   */
  PacketType    Packet_Type;            /* Should be 0x0002             */
  sint           Orig_Net;               /* originating net of packet    */
  sint           Dest_Net;               /* destination net of packet    */
  char          ProtCode;               /* product code                 */
  char          SerialNo;               /* Serial number                */
  char          Password[8];            /* Session password (or NULL)   */
  sint           Orig_Zone;              /* zone of packet sender (NULL) */
  sint           Dest_Zone;              /* zone of dest  receiver (NULL)*/
  char          Reserved[20];           /* Fill bytes                   */
} FIDO_PACKET_HEADER;  /* Stone Age Packets */
#pragma pack()

#define         FIDO_HDR_SIZE   sizeof(FIDO_PACKET_HEADER)

#pragma pack(1)
typedef struct
{
  sint           Orig_Node;              /* originating node of packet   */
  sint           Dest_Node;              /* destination node of packet   */
  sint           Year;                   /* year packet was created      */
  sint           Month;                  /* month packet was created     */
  sint           Day;                    /* day packet was created       */
  sint           Hour;                   /* hour packet was created      */
  sint           Minute;                 /* minute packet was created    */
  sint           Second;                 /* second packet was created    */
  sint           Baud;                   /* max baud rate of orig/dest   */
  PacketType    Packet_Type;            /* Should be 0x0002             */
  sint           Orig_Net;               /* originating net of packet    */
  sint           Dest_Net;               /* destination net of packet    */
  char          ProtCode;               /* product code                 */
  char          SerialNo;               /* Serial number                */
  char          Password[8];            /* Session password (or NULL)   */
  sint           Orig_Zone;              /* zone of packet sender (NULL) */
  sint           Dest_Zone;              /* zone of dest receiver (NULL) */
  sint           auxnet;                 /* Origin Net if from a Point   */
  sint           CWValid;                /* CW Validation Copy, Byte-Swapped */
  char          ProdCodeH;              /* FTSC Product Code, Hi        */
  char          ProdCodeM;              /* FTSC Product Code, Minor     */
  sint           CW;                     /* Compatibility Word           */
  sint           OrigZone;               /* Origin Zone                  */
  sint           DestZone;               /* Destination Zone             */
  sint           OrigPoint;              /* Origin Point                 */
  sint           DestPoint;              /* Destination Point            */
  long int      ProdData;               /* Product Specific Data        */
} FIDO_PACKET_HEADER_TYPE2;   /* New Type 2+ Packet */
#pragma pack()

#define         FIDO_HDR_SIZE_TYPE2   sizeof(FIDO_PACKET_HEADER_TYPE2)

#pragma pack(1)
typedef struct
{
  PacketType    Packet_Type;            /* Should be 0x0002             */
  sint           Orig_Node;              /* Originating node of message  */
  sint           Dest_Node;              /* Destination node of message  */
  sint           Orig_Net;               /* Net # of originating node    */
  sint           Dest_Net;               /* Net # of destinating node    */
  sint           Attribute;              /* Message attribute            */
  sint           Msg_Cost;               /* Est. cost of sending msg     */
  char          DateTime[20];
  char          To_User[36];
  char          From_User[36];
  char          Subject[72];
} FIDO_MESSAGE_HDR;
#pragma pack()

#pragma pack(1)
typedef struct
{
  char          From_User[36];          /* User msg is from             */
  char          To_User[36];            /* Who msg is to                */
  char          Subject[72];            /* Subject of message           */
  char          Date_Time[20];          /* date and time of msg         */
  uint           Times_Read;             /* # times msg has been read    */
  uint           Dest_Node;              /* destination node of msg      */
  uint           Orig_Node;              /* originating node of msg      */
  sint           Cost;                   /* cost of sending msg          */
  uint           Orig_Net;               /* originating net of msg       */
  uint           Dest_Net;               /* Destination net of msg       */
  uint           Dest_Zone;              /* destination zone of msg      */
  uint           Orig_Zone;              /* originating zone of msg      */
  uint           Dest_Point;             /* Destination point of msg     */
  uint           Orig_Point;             /* originating point of msg     */
  uint           Reply_To;               /* msg # msg is reply to        */
  uint           Attribute;              /* Attribute of msg             */
  uint           Next_Reply;             /* Msg # of next reply          */
} STORED_FIDO_MSG;
#pragma pack()

#pragma pack(1)
typedef struct
{
  bassngl       high_number;            /* high message number in base  */
  bassngl       low_number;             /* low message number in base   */
  bassngl       num_active;             /* number of active messages    */
  bassngl       num_callers;            /* main base only               */
  char          lockfeild[6];           /* "LOCKED" Field for 14.2      */
  char          reserved[106];          /* Reserved for future use      */
} PCB_BASE_HEADER;
#pragma pack()

#pragma pack(1)
typedef struct
{
  char          Status;                 /* Message status               */
  bassngl       Number;                 /* message number               */
  bassngl       Reference;              /* Referenced message number    */
  char          Num_Blocks;             /* number of blocks in msg      */
  char          Date[8];                /* Date of message              */
  char          Time[5];                /* Time of message              */
  char          To[25];                 /* Who message is to            */
  bassngl       Reply_Date;             /* Date message was replied to  */
  char          Reply_Time[5];          /* Time message was replied to  */
  char          Reply_Status;           /* Does this message have reply?*/
  char          From[25];               /* Who message is from          */
  char          Subject[25];            /* Subject of message           */
  char          Password[12];           /* Password                     */
  char          Active_Status;          /* Is msg active?               */
  char          Echo;                   /* Echo message?                */
  char          Reserved[6];            /* Reserved for future use      */
} PCB_MSG_HEADER;
#pragma pack()

#pragma pack(1)
typedef struct
{
  uint          Conf_Num;
  char          Filename[MAXFLEN];
  long          high_num;
  long          low_num;
  long          active_msgs;
  DOSFILE       base_handle;
  DOSFILE       base_header_handle;
  sint           index_handle_145;
  sint           index_handle_150;
  bool          opened;
} PCB_MSG_BASE;
#pragma pack()

typedef enum {BAUD_300,BAUD_1200,BAUD_2400,BAUD_4800,BAUD_9600,BAUD_14400,
              BAUD_16800} baud_types;

#pragma pack(1)
typedef struct {
  sint                   region;
  unsigned long         offset;
} REGION_T;
#pragma pack()

#pragma pack(1)
typedef struct {
  uint          crash_mailer:1; /* Accepts mail 24 hours a day */
  uint          no_humans:1;    /* Does not accept human callers */
  uint          listed_only:1;  /* Calls from listed systems only */
} SysFlags1;
#pragma pack()

#pragma pack(1)
typedef struct {
  uint          V21:1;  /* CCITT v.21 -   300bps full duplex    */
  uint          V22:1;  /* CCITT v.22 -  1200bps full duplex    */
  uint          V29:1;  /* CCITT v.29 -  9600bps half duplex    */
  uint          V32:1;  /* CCITT v.32 -  9600bps full duplex    */
  uint          V32B:1; /* CCITT v.32bis - 14400bps full duplex */
  uint          V33:1;  /* CCITT v.33                           */
  uint          V34:1;  /* CCITT v.34                           */
  uint          V42:1;  /* LAP-M error correction - fallback MNP*/
  uint          V42B:1; /* CCITT v.42bis                        */
  uint          MNP:1;  /* Microcom Networking Protocol         */
  uint          H96:1;  /* Hayes V9600                          */
  uint          HST:1;  /* USR Courier HST                      */
  uint          MAX:1;  /* Microcom AX/96xx series              */
  uint          PEP:1;  /* Packet Ensemble Protocol             */
  uint          CSP:1;  /* Compucom Speedmodem                  */
  uint          MN:1;   /* No compression supported             */
} SysFlags2;
#pragma pack()

#pragma pack(1)
typedef struct {
  uint          XA:1;   /*                                      */
  uint          XB:1;   /*                                      */
  uint          XC:1;   /*      listed below                    */
  uint          XP:1;   /*                                      */
  uint          XR:1;   /*                                      */
  uint          XW:1;   /*                                      */
  uint          XX:1;   /*                                      */
  uint          Gateway:1;      /* System is a gateway          */
} SysFlags3;
#pragma pack()


#pragma pack(1)
typedef struct {
  uint                  zone;
  uint                  net;
  uint                  node;
  char                  BBS_name[70];
  char                  location[20];
  char                  Sysop_name[25];
  char                  phone[50];
  baud_types            maximum_baud;
  SysFlags1             ConfigFlags;
  SysFlags2             CompFlags;
  SysFlags3             Capabilities;
} NODE_REC;
#pragma pack()

/************************************************************************/
/* The following flags indicate the types of                            */
/* file/update requests supported.                                      */
/* |--------------------------------------------------|                 */
/* |      |         Bark        |        WaZOO        |                 */
/* |      |---------------------|---------------------|                 */
/* |      |   File   |  Update  |   File   |  Update  |                 */
/* | Flag | Requests | Requests | Requests | Requests |                 */
/* |------|----------|----------|----------|----------|                 */
/* | XA   |    Yes   |    Yes   |    Yes   |    Yes   |                 */
/* | XB   |    Yes   |    Yes   |    Yes   |    No    |                 */
/* | XC   |    Yes   |    No    |    Yes   |    Yes   |                 */
/* | XP   |    Yes   |    Yes   |    No    |    No    |                 */
/* | XR   |    Yes   |    No    |    Yes   |    No    |                 */
/* | XW   |    No    |    No    |    Yes   |    No    |                 */
/* | XX   |    No    |    No    |    Yes   |    Yes   |                 */
/* |--------------------------------------------------|                 */
/************************************************************************/


#pragma pack(1)
typedef struct {
  uint           zone,
                 net;
  unsigned long  offset;
} OFFSET;
#pragma pack()

#pragma pack(1)
typedef struct
{
  uint            zone,
                  net,
                  node;
  char            sysop[50],
                  bbs_name[50],
                  bbs_phone[50],
                  bbs_loc[50],
                  baud[30];
  unsigned long   offset;
} INDEX;
#pragma pack()

#pragma pack(1)
typedef struct
{
  uint          zone;
  char          listname[80];
} NODELIST_HEADER;
#pragma pack()

#pragma pack(1)
typedef struct
{
  char          basename[80],
                diffname[9];
  sint          last_compile;
  char          reserved[10];
} NODELIST;
#pragma pack()

#pragma pack(1)
typedef struct
{
  char          archivers[MAX_ARCHIVERS][MAXFLEN];
  char          archiver_switches[MAX_ARCHIVERS][80];
  char          unarchivers[MAX_ARCHIVERS][MAXFLEN];
  char          unarchiver_switches[MAX_ARCHIVERS][80];
} ARCHIVERS;
#pragma pack()

#pragma pack(1)
typedef struct
{
  char          incoming_packets[MAXFLEN];
  char          outgoing_packets[MAXFLEN];
  char          outgoing_msg[MAXFLEN];
  char          bad_packets[MAXFLEN];
  char          nodelist_path[MAXFLEN];
  char          work_directory[MAXFLEN];
  char          passthrough[MAXFLEN];
  char          securemail[MAXFLEN];
  char          messages[MAXFLEN];
} DIRECTORIES;
#pragma pack()

#pragma pack(1)
typedef struct
{
  sint           zone;                   /* zone .PKT file goes to       */
  sint           net;                    /* net .PKT file goes to        */
  sint           node;                   /* node .PKT file goes to       */
  sint           point;                  /* point .PKT file goes to      */
  char           filename[MAXFLEN];       /* .PKT path and filename       */
  bool           archived;               /* has .PKT been archived?      */
} ARCHIVE_RECORD;
#pragma()

#pragma pack(1)
typedef struct
{
  char  nodestr[25];
  uint  this_zone;
  uint  this_net;
  uint  this_node;
  uint  this_point;
  char  reserved[10];
} THIS_ADDRESS;
#pragma pack()

/* New this_address used with new configuration modules */
#pragma pack(1)
typedef struct
{
  uint  zone;
  uint  net;
  uint  node;
  uint  point;
  bool  primary;
  bool  inseenby;
  bool  present;
  char  UpOrDownLink;
  char  range[70];
  char  reserved[10];
} NADDRESS;
#pragma pack()

#pragma pack(1)
typedef struct
{
  char  nodestr[25];
  uint  zone;
  uint  net;
  uint  node;
  uint  point;
  uint  pkt_type;
  sint  archiver_index;
  char  reserved[10];
} NODE_T;
#pragma pack()

/* New node_t struct for new configuration files */
#pragma, pack(1)
typedef struct
{
  uint  zone;
  uint  net;
  uint  node;
  uint  point;
  uint  pkt_type;
  sint  archiver_index;
  char  reserved[10];
} NNODE_T;
#pragma pack()

#pragma pack(1)
typedef struct
{
  char          in[30];
  char          out[30];
  char          reserved[10];
} TRANSLATE;
#pragma pack()

#pragma pack(1)
typedef struct
{
  uint          PCB_Conference;         /* PCBoard conference number    */
  char          Area_Name[AREA_SIZE];          /* name of area                 */
/*char          Messages[MAXFLEN];       /* name of file for messages    */ */
  char          Mreserved[MAXFLEN];       /* name of file for messages    */
  char          default_aka[25];        /* Address to use for Export    */
  char          origin[70];             /* Origin line to use           */
  char          HighAscii;              /* Does this conference allow high ascii? */
  char          reserved[19];           /* Reserved                     */
} AREA_STRUCT;
#pragma pack()

/*New areastruct for new config modules */
#pragma pack(1)
typedef struct
{
  uint            ConfNum;                /* PCBoard conference number    */
  char            AreaTag[AREA_SIZE];            /* name of area                 */
  uint            AkaIndex;               /* Address to use for Export    */
  uint            OriginIndex;            /* Origin line to use           */
  char            HighAscii;              /* Does this conference allow high ascii? */
  unsigned short  LastActivityDate;        /* Julian Date for last export  */
  char            AllowPrivate;
  char            AllowFileAttach;
  char            reserved[10];
} NAREA_STRUCT;
#pragma pack()


#pragma pack(1)
typedef struct
{
    uint conf;
    uint recno;
}AINDEX;
#pragma pack()

#pragma pack(1)
typedef struct
{
    char origin[70];
    char ranges[70];
    char reserved[10];
}ORIGIN;
#pragma pack()

#pragma pack(1)
typedef struct
{
  char send[80];
} SENDVERB;
#pragma pack()

#pragma pack(1)
typedef struct
{
  char  filename[80];                    /* Name of outgoing file. Full path  */
  char  nodestr[25];                     /* Node to receive the file.         */
  sint  flag;                            /* Attribute (Kill-sent, crash,etc.) */
  sint  failedConnects;                  /* Number of failed connections      */
  bool  readOnly;                         /* Can this entry be modified by event processing */
  char  reserved[18];                    /* Reserved                          */
} QUEUE_RECORD;
#pragma pack()

#pragma pack(1)
typedef struct
{
   unsigned long slots[8];
   char reserved[sizeof(QUEUE_RECORD) - 8*sizeof(long)];
}QUEUE_HEADER;
#pragma pack()

typedef struct
{
  char addr[2000];
  char pw[25];
  char line[20];
  char codes[50];
  char prodcode[50];
  char prodname[50];
  char prodver[10];
  char prodserial[50];
  char name[60];
  char city[30];
  char sysop[30];
  char phone[50];
  char baud[10];
  char flags[50];
} Emsi;

typedef struct
{
  char  BBS_Name[60];
  char  City[30];
  char  Sysop[30];
  char  Phone[50];
  char  Baud[10];
  char  Flags[50];
} EMSI_DATA;

typedef struct
{
  char str[25];
} REMOTE_AKA;

#pragma pack(1)
typedef struct
{
  char str[60];
  char reserved[20];
} FREQ_PATH;
#pragma pack()

#pragma pack(1)
typedef struct
{
  char Path[MAXFLEN];
  char Password[10];
  char    reserved[10];
} NFREQ_PATH;
#pragma pack()

#pragma pack(1)
typedef struct
{
  uint            stime;
  uint            dtime;
  unsigned long   sbytes;
  unsigned long   dbytes;
  char            listed;
  uint            baud;
} FREQ_INFO;
#pragma pack()

#pragma pack(1)
typedef struct
{
  char    magic_name[20];
  char    real_name[50];
  char    reserved[10];
} FREQ_MAGIC;
#pragma pack()

/* New freq_magic for new config modules */
#pragma pack(1)
typedef struct
{
  char    MagicName[20];
  char    RealName[MAXFLEN];
  char    Password[10];
  char    reserved[10];
} NFREQ_MAGIC;
#pragma pack()

#pragma pack(1)
typedef struct
{
  char   send[80];
  bool   hit;
} FSEND;
#pragma pack()

#pragma pack(1)
typedef struct
{
  char            nodestr[25];
  unsigned long   bytes;
  unsigned long   time;
} FREQ_LIMIT;
#pragma pack()


#pragma pack(1)
typedef struct
{
    uint zone;
    uint net;
    uint node;
    uint point;
}FUSERS;
#pragma pack()


typedef struct _nodenode
{
    sint                 node;
    struct _nodenode  * next;
}nodenode;

typedef struct _netnode
{
    sint                 net;
    struct _nodenode   * nodelist;
    struct _netnode    * next;
}netnode;

typedef struct _nodeInf
{
    uint node;
    uint net;
}nodeInf;

typedef struct
{
  char    name[30];
  long    num;
}FIDOUSER;

typedef struct
{
    char    fName[13];
    uint    fTime;
    uint    fDate;
}FILEDATETIME;

typedef struct
{
    char     dow[3];
    sint     fileNum;
}ECHOFILEINF;

typedef struct
{
  unsigned long  FidoMsgNum;
  unsigned long  PCBMsgNum;
  bool           Hit;
} PACKMSGNFO;

#endif
