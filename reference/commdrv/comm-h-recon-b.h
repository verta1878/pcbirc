/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* comm.h - COMM-DRV interface for PCBoard's COMMDRV backend.                */
/*                                                                           */
/* NOT WCSC's header.  This is a RECONSTRUCTION, written 2026-09-22 by       */
/* reading the only consumer we have: pcb153\SOURCE\MODEM\MODEMDRV.C.        */
/* pcbirc crew, GPLv3.                                                       */
/*                                                                           */
/* WHY IT EXISTS                                                             */
/* -------------                                                             */
/* PCBOARD.MAK expects WCSC's COMM-DRV developer toolkit at                  */
/*      $(LIBSDIR)\COMMDRV\H\comm.h        (include path, line 64)           */
/*      $(LIBSDIR)\commdrv\lib\commdrbl.lib                                  */
/*      $(LIBSDIR)\commdrv\lib\libsbl.lib  (link list, lines 841-842)        */
/* None of the three is in this repo and none is obtainable: WCSC's only     */
/* free offering is a Win32 library with a different API (OpenComPort,       */
/* GetByte, PutPacket - no ser_rs232_* at all), and COMM-DRV/DOS is still    */
/* a paid product.  We do have the v15.0b RUNTIME in                         */
/* pcb1541\install\dist\target\COMMDRV\ (COMMDRV.EXE, COMMTSR.EXE, nine      */
/* .DRV files) but not the toolkit.                                          */
/*                                                                           */
/* WHAT IS AND IS NOT RECOVERED                                              */
/* ---------------------------                                               */
/* Recovered exactly, from MODEMDRV.C: which functions exist, how many       */
/* arguments each takes and in what order, which struct fields are read and  */
/* written, and which constants are compared against.  That set is closed    */
/* and finite - MODEMDRV.C is the whole of PCBoard's use of COMM-DRV.        */
/*                                                                           */
/* NOT recovered, and not recoverable from source: WCSC's byte layout.       */
/* Field widths and offsets below are OUR choice.  A program built against   */
/* this header talks to a driver built against this header - it will NOT     */
/* interoperate with a genuine WCSC COMM-DRV TSR.  If interop with the real  */
/* product is ever wanted, the layout has to come out of the binaries in     */
/* pcb1541\install\dist\target\COMMDRV\, not out of this file.               */
/*                                                                           */
/* Field semantics ARE constrained by MODEMDRV.C and must not be changed:    */
/*   opcb->msr_reg    a raw 8250/16550 Modem Status Register byte.           */
/*                    bit 0x04 = RI, 0x10 = CTS, 0x80 = DCD.                 */
/*   opcb->inbuf_count / outbuf_count                                        */
/*                    bytes currently queued, not free space.                */
/*   outbuf_len       total transmit buffer size; PCBoard sets its own       */
/*                    OutBufSize from it and then subtracts 128 for slack.   */
/*   error            driver error detail, printed when setup fails.         */
/*   cardtype         board type; PCBoard saves and restores it across       */
/*                    re-open, and compares it against CARD_DIGCXI.          */
/*                                                                           */
/* WHERE THE PIECES LIVE                                                     */
/* ---------------------                                                     */
/*   this header      pcbcbase\COMMDRV\H\COMM.H                              */
/*                    One copy serves every consumer.  pcb153\153\PCBOARD.MAK */
/*                    line 64 and pcb154\MAIN\153\PCBOARD.MAK line 43 both   */
/*                    add $(LIBSDIR)\COMMDRV\H to the include path, and       */
/*                    neither makefile defines LIBSDIR -- it comes from the   */
/*                    environment, where BLDDOS.BAT sets LIBSDIR=\PCBCBASE.   */
/*                    ZMODEM.MAK uses the same path.                          */
/*                                                                           */
/*   library source   toolkit\pwa154\pcbdcom\src\ser_rs232_shim.c plus the   */
/*                    backends beside it.  That is the copy to keep; it is    */
/*                    being folded into pcb1541\pcbdcom\, which is the        */
/*                    canonical tree.  See pcb1541\pcbdcom\README.md.         */
/*                                                                           */
/*   built library    pcbcbase\commdrv\lib\COMMDRBL.LIB (and LIBSBL.LIB),     */
/*                    which is what both PCBOARD.MAKs link at lines 820-821.  */
/*                                                                           */
/* DELTA / OPENWATCOM                                                         */
/* ------------------                                                         */
/* The Delta 15.4 leg does not build the COMM-DRV backend today: its Watcom   */
/* makefiles (pcb154\MAIN\WATCOM\PCBOARD.MK, pcb154\MAIN\153\PCBWAT2.MK,    */
/* three lines each) name neither COMMDRV nor MODEMDRV nor this header, even  */
/* though pcb154\MAIN\SOURCE\MODEM\MODEMDRV.C exists.  When that leg is      */
/* fleshed out, three things follow:                                          */
/*                                                                           */
/*   1. This header needs compiler guards.  'far' and LIBENTRY below are      */
/*      Borland/MSC spellings; OpenWatcom wants __far.  Guard them the way    */
/*      pcbdcom's own compat.h already guards the interrupt keywords.         */
/*   2. The LIBRARY cannot be shared, only the source.  COMMDRBL.LIB is a     */
/*      Borland large-model OMF library; Watcom has different name mangling   */
/*      and calling conventions.  Each branch builds its own from the same    */
/*      ser_rs232_shim.c.                                                     */
/*   3. The name check has to be redone per compiler.  PCBoard compiles with  */
/*      -P, so TLINK matches MANGLED names: MODEMDRV.OBJ built with BC 3.1    */
/*      asks for @SER_RS232_GETPACKET$QIINUC.  Watcom will emit its own       */
/*      spelling.  The source fix is the same either way -- the port and      */
/*      count parameters must be int, not unsigned int -- but the            */
/*      verification is per branch, against that branch's MODEMDRV.OBJ.       */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifndef H_COMM
#define H_COMM

/* ---- return codes ------------------------------------------------------ */
/* MODEMDRV.C compares against NONE and BUSY only.  PARAM and NOPORT are     */
/* pcbdcom's, kept here so both sides share one set.                         */

#define RS232ERR_NONE       0
#define RS232ERR_BUSY       1
#define RS232ERR_PARAM      2
#define RS232ERR_NOPORT     3

/* ---- word length (port_param.lngth) ------------------------------------ */

#define LENGTH_7            7
#define LENGTH_8            8

/* ---- parity (port_param.parity) ---------------------------------------- */

#define PARITY_NONE         0
#define PARITY_ODD          1
#define PARITY_EVEN         2

/* ---- flow control (port_param.protocol) -------------------------------- */
/* PCBoard sets PROT_RTSRTS unless PcbData.DisableCTS. */

#define PROT_NONE           0
#define PROT_RTSRTS         1
#define PROT_XONXOFF        2

/* ---- baud (port_param.baud) -------------------------------------------- */
/* These are 8250 DIVISOR LATCH values, not an index.  COMMDRV_setport()     */
/* takes a BaudDivisor argument and assigns it straight to pcb.baud, and     */
/* COMMDRV_bauddivisor() maps PCBoard's speed-in-tens to one of these --     */
/* case 11520 returns BAUD115200.  115200 / rate = divisor.                  */

#define BAUD300           384
#define BAUD1200           96
#define BAUD2400           48
#define BAUD4800           24
#define BAUD9600           12
#define BAUD19200           6
#define BAUD38400           3
#define BAUD57600           2
#define BAUD115200          1

/* ---- card types (port_param.cardtype, opcb.cardtype) ------------------- */
/* MODEMDRV.C names only CARD_DIGCXI: the Digiboard COM/Xi does not refresh  */
/* the buffer counters often enough, so PCBoard forces an extra              */
/* ser_rs232_getpacket() call when it sees that value.  The rest are         */
/* pcbdcom's eight supported families.                                       */

#define CARD_8250           0
#define CARD_BOCA           1
#define CARD_CYCLOM         2
#define CARD_DIGPCXE        3
#define CARD_DIGCXI         4
#define CARD_ROCKET         5
#define CARD_EASYIO         6
#define CARD_ARNET          7

/* ---- opcb.flag bits ---------------------------------------------------- */
/* XMTOFF_STATE appears only in commented-out lines of COMMDRV_commstop()    */
/* and COMMDRV_commpause().  Kept so those lines compile if reinstated.      */

#define XMTOFF_STATE     0x0001

/* ---- calling convention ------------------------------------------------ */
/* PCBoard declares every library entry point LIBENTRY, which TYPES.HPP      */
/* expands to 'pascal' on Borland and MSC and to nothing on Watcom.  It is   */
/* not decoration: it is why MODEMDRV.OBJ asks for UPPERCASE symbols         */
/* (@SER_RS232_GETPACKET$QIINUC).  An implementation that omits it produces  */
/* different names and will not link.  Defined here so a file that includes  */
/* only this header still gets it right.                                     */

#ifndef LIBENTRY
  #ifdef __WATCOMC__
    #define LIBENTRY
  #else
    #define LIBENTRY pascal
  #endif
#endif

/* ---- the per-port control blocks --------------------------------------- */
/*                                                                          */
/* port_param is the caller's block, filled in by ser_rs232_getport() and    */
/* handed back to ser_rs232_setup().  opcb and auxpcb point INTO the         */
/* driver's own memory -- PCBoard reads them live, every time, and never     */
/* copies them.  Shapes taken from pcbdcom's inc/pcbdcom.h, which is the     */
/* working implementation; this header is now the single declaration and     */
/* pcbdcom.h includes it.                                                    */

typedef struct {
    unsigned char  msr_reg;        /* modem status register shadow       */
    unsigned char  flag;           /* XMTOFF_STATE etc.                  */
    unsigned char  cardtype;       /* CARD_* constant                    */
    unsigned char  reserved1;
    unsigned int   inbuf_count;    /* bytes available in RX buffer       */
    unsigned int   outbuf_count;   /* bytes pending in TX buffer         */
    unsigned char  padding[24];
} opcb_type;

typedef struct {
    int aux_frmint;                /* framing error count                */
    int aux_ovrint;                /* overrun error count                */
    int aux_parint;                /* parity error count                 */
} auxpcb_type;

struct port_param {
    unsigned int   baud;           /* divisor latch value, BAUD* above   */
    unsigned char  parity;         /* PARITY_*                           */
    unsigned char  data_bits;      /* 5..8                               */
    unsigned char  stop_bits;      /* 1 or 2                             */
    unsigned char  flow;           /* 0=none 1=RTS/CTS 2=XON/XOFF        */
    unsigned int   buf_size;       /* RX buffer size                     */
    unsigned char  lngth;          /* LENGTH_7 or LENGTH_8               */
    unsigned char  cardtype;       /* CARD_*                             */
    unsigned char  protocol;       /* PROT_*                             */
    int            error;          /* detail for a failed setup          */
    unsigned int   outbuf_len;     /* TX buffer size                     */
    unsigned int   inbuf_len;      /* RX buffer size                     */
    unsigned char  block[4];       /* PCBoard clears block[1] before setup */
    opcb_type     *opcb;           /* live driver state                  */
    auxpcb_type   *auxpcb;         /* live error counters                */
    unsigned char  misc[16];       /* reserved                           */
};

/* ---- the API ----------------------------------------------------------- */
/*                                                                          */
/* Thirteen entry points.  That is not a selection -- it is every COMM-DRV  */
/* function MODEMDRV.C calls, and MODEMDRV.C is the whole of PCBoard's use  */
/* of COMM-DRV.  Ports are 0-BASED: PCBoard passes Asy.ComPortNumber - 1,   */
/* so COM1 arrives as 0.  All return RS232ERR_NONE on success.              */
/*                                                                          */
/* The int parameters are int, not unsigned int, because that is what       */
/* MODEMDRV.OBJ's mangled externals require -- QII is (int,int).            */
/*                                                                          */
/* ser_rs232_getpacket(port, count, buf) is called three ways:              */
/*   count 0,     buf != NULL   refresh opcb; the buffer is SCRATCH, not    */
/*                              an output.  PCBoard reads opcb->msr_reg     */
/*                              on the next line.                           */
/*   count 32767, buf == NULL   refresh the counters and LEAVE THE DATA     */
/*                              ALONE.  COMMDRV_inbytes() reads             */
/*                              opcb->inbuf_count immediately after.        */
/*   count n,     buf != NULL   read n bytes.                               */
/*                                                                          */
/* ser_rs232_flush(port, which): 0 = input, 1 = output, 2 = both, from      */
/* COMMDRV_clearinbuf / clearoutbuf / commstop respectively.                */
/*                                                                          */
/* ser_rs232_putpacket(port, 0, NULL) must KICK THE TRANSMITTER -- that is  */
/* the only thing COMMDRV_turnonxmit() calls.                               */

int LIBENTRY ser_rs232_init       (void);
int LIBENTRY ser_rs232_getport    (int Port, struct port_param *Pcb);
int LIBENTRY ser_rs232_setup      (int Port, struct port_param *Pcb);
int LIBENTRY ser_rs232_getpacket  (int Port, int Count, unsigned char *Buf);
int LIBENTRY ser_rs232_putpacket  (int Port, int Count, unsigned char *Buf);
int LIBENTRY ser_rs232_viewpacket (int Port, int Count, unsigned char *Buf);
int LIBENTRY ser_rs232_getbyte    (int Port, unsigned char *Byte);
int LIBENTRY ser_rs232_putbyte    (int Port, unsigned char *Byte);
int LIBENTRY ser_rs232_flush      (int Port, int Which);
int LIBENTRY ser_rs232_dtr_on     (int Port);
int LIBENTRY ser_rs232_dtr_off    (int Port);
int LIBENTRY ser_rs232_rts_on     (int Port);
int LIBENTRY ser_rs232_rts_off    (int Port);

#endif  /* H_COMM */
