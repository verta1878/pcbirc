/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* pcbiso.h — PCBoard file area structures for PCBISO                       */
/*                                                                           */
/* PCBoard Conference Extension — works on 15.4+                            */
/* Uses addconftype.Reserved[64] — zero-initialized by PCBoard.             */
/* PCBoard ignores these bytes. PCBISO reads them.                          */
/* No upgrade needed.                                                        */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifndef PCBISO_H
#define PCBISO_H

#define PCBISO_VERSION  "1.0.0"
#define MAX_CONF        65535
#define MAX_PATH_LEN    260
#define MAX_DESC        55
#define MAX_FILENAME    13
#define MAX_MOUNT       26       /* A-Z drive letters */

/*-----------------------------------------------------------------------*/
/* PCBoard Conference Extension — reinterprets Reserved[64]               */
/*                                                                         */
/* addconftype.Reserved[64] is zero-initialized by PCBoard 15.4.          */
/* PCBoard never reads these bytes. PCBISO uses them as per-filebase      */
/* bit flags. One bit per filebase, 8 flags per filebase.                  */
/*                                                                         */
/* FilebaseFlags[N/8] bit (N%8) = flags for filebase N                    */
/* Supports up to 512 filebases per conference.                           */
/*-----------------------------------------------------------------------*/

typedef struct {
    unsigned char FilebaseFlags[64];
    /* Per-filebase bit flags:
     *   Bit 0: ISO-backed (source is mounted ISO image)
     *   Bit 1: (reserved)
     *   Bit 2: (reserved)
     *   Bit 3: (reserved)
     *   Bit 4: (reserved)
     *   Bit 5: (reserved)
     *   Bit 6: (reserved)
     *   Bit 7: (reserved)
     */
} pcb_conf_ext;

#define FBFLAG_ISO  0x01   /* filebase is ISO-backed */

/* Helper macros */
#define FB_GET_FLAG(ext, fbnum, flag) \
    (((ext)->FilebaseFlags[(fbnum) / 8] >> ((fbnum) % 8)) & (flag))

#define FB_SET_FLAG(ext, fbnum, flag) \
    ((ext)->FilebaseFlags[(fbnum) / 8] |= ((flag) << ((fbnum) % 8)))

#define FB_CLR_FLAG(ext, fbnum, flag) \
    ((ext)->FilebaseFlags[(fbnum) / 8] &= ~((flag) << ((fbnum) % 8)))

#define FB_IS_ISO(ext, fbnum) \
    FB_GET_FLAG(ext, fbnum, FBFLAG_ISO)

/*-----------------------------------------------------------------------*/
/* PCBoard CNAMES structures (packed, matches on-disk format)              */
/*-----------------------------------------------------------------------*/

#pragma pack(1)

typedef struct {
    char Name[14];
    char PublicConf;
    char AutoRejoin;
    char ViewMembers;
    char PrivUplds;
    char PrivMsgs;
    char EchoMail;
    short ReqSecLevel;
    short AddSec;
    short AddTime;
    char MsgBlocks;
    char MsgFile[32];
    char UserMenu[32];
    char SysopMenu[32];
    char NewsFile[32];
    char PubUpldSort;
    char UpldDir[29];
    char PubUpldLoc[26];
    char PrvUpldSort;
    char PrivDir[29];
    char PrvUpldLoc[26];
    char DrsMenu[29];
    char DrsFile[33];
    char BltMenu[29];
    char BltNameLoc[33];
    char ScrMenu[29];
    char ScrNameLoc[33];
    char DirMenu[29];
    char DirNameLoc[33];     /* path to DIR.LST */
    char PthNameLoc[33];     /* path to DLPATH.LST */
} oldconftype;               /* 548 bytes */

typedef struct {
    char ForceEcho;
    char ReadOnly;
    char NoPrivateMsgs;
    char RetReceiptLevel;
    char RecordOrigin;
    char PromptForRouting;
    char AllowAliases;
    char ShowIntroOnRA;
    char ReqLevelToEnter;
    char Password[13];
    char Intro[32];
    char AttachLoc[32];
    char RegFlags[4];
    char AttachLevel;
    char CarbonLimit;
    char CmdLst[32];
    char OldIndex;
    char LongToNames;
    char CarbonLevel;
    char ConfType;
    long ExportPtr;
    float ChargeTime;
    float ChargeMsgRead;
    float ChargeMsgWrite;
    char FilebaseFlags[64];   /* per-filebase bit flags (pcb_conf_ext) */
    char Name2[48];
} addconftype;               /* 260 bytes */

#pragma pack()

/* DirListType — from DIR.LST (per file directory) */
#pragma pack(1)
typedef struct {
    char DirPath[31];        /* path to DIR listing text file */
    char DskPath[31];        /* physical path where files live */
    char DirDesc[36];        /* description shown to users */
    char SortType;           /* sort order */
} DirListType;               /* 99 bytes */
#pragma pack()

/* Mount table entry */
typedef struct {
    char drive;              /* drive letter A-Z, 0=unused */
    char isopath[MAX_PATH_LEN];
} mount_entry;

/* PCBISO.DAT — persisted mount table */
#define PCBISO_DAT_SIG  "PCBISO10"
typedef struct {
    char sig[8];             /* "PCBISO10" */
    int  count;
    mount_entry mounts[MAX_MOUNT];
} pcbiso_dat;

#endif /* PCBISO_H */
