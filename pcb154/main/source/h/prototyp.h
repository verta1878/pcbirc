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


#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include "structs.h"
/* FIDOFUNC.C - C++ Functions */



//#ifdef __cplusplus
//extern "C" {
//#endif

/* Stolen prototypes from CONIO.H */

int         _Cdecl getch( void );
int         _Cdecl getche( void );
int         _Cdecl kbhit( void );

/* RECWAZOO.C */

bool LIBENTRY Send_Hello(void);
bool LIBENTRY get_block(HELLO_T & block,int blocksize);
bool LIBENTRY get_product_name(int code,char *name);
bool LIBENTRY Recv_Hello(void);
bool LIBENTRY Receiver_WaZoo_Init(void);
unsigned short LIBENTRY get_crc(void);


#ifndef TOSSCLASS
/* PCBTOSS.C */

void      LIBENTRY  send_file_notice(char *node, char *filename);
void      LIBENTRY  init_users(void);
bool      LIBENTRY  toss_messages(void);
bool      LIBENTRY  copy_pcb_messages(void);
bool      LIBENTRY  toss_packet(char *filename);

#endif

/* HEX.C */

unsigned long LIBENTRY HexDec (char _FAR_ *string);
int           LIBENTRY iHexDec (char _FAR_ *string);

/* XMITEMSI.C */

void send_outbound_mail(char *addr,int howCalled);
void PrintCapabilities(int theword);
bool connected_string(char *checkstring);
void logEMSI(Emsi & info);

/* XLAT.C */

bool LIBENTRY translate_phone(char *phone);

/* RECEMSI.C */

bool attempt_emsi_handshake(void);

/* CRC-16F.C */

unsigned short fido_updcrc(unsigned short, unsigned char *, unsigned int);

/* FIDOFUNC.C */

void LIBENTRY TransferMessages(int *sendcount,char *nodestr,int status,int protocol);
void LIBENTRY create_freq(void);
void LIBENTRY poll_node(void);
void LIBENTRY default_address(char *nodestr,int len);
void LIBENTRY fido_nodestr_to_int(char *addr, uint & zone, uint & net, uint & node, uint & point);
void LIBENTRY Disconnect(void);
void LIBENTRY make_emsi_dat(Emsi *emsi,char *password);
void LIBENTRY display_fido_info(Emsi &emsi);
void LIBENTRY receive_inbound_xmodem(void);
bool LIBENTRY getINTLinfo(const char * msgBuf, char * dAddr, int dAddrSz, char * sAddr,int sAddrSz);
bool LIBENTRY send_emsi_handshake(Emsi &emsi,int direction);
bool LIBENTRY receive_emsi_handshake(char *remote_nodestr, int len, Emsi &emsi);
int  LIBENTRY read_line(DOSFILE *filebuffer,char *buffer,int max_len);
long LIBENTRY get_event_info(char *zonenetnodestr);
bool LIBENTRY OKtoImportExport(void);

/* FCONFIG.C */

bool LIBENTRY read_fido_config(int flags);
bool LIBENTRY write_fido_config(void);
bool LIBENTRY make_fido_config(void);
void LIBENTRY free_fido_memory(void);

/* FIDOMENU.C */

void  do_fido_menu(void);

/* FIDOMISC.CPP */

bool get_node(unsigned int zone, unsigned int net, unsigned int node, NODE_REC *noderec);


/* FIDOQ.CPP */

bool LIBENTRY outboundRec(void);
void LIBENTRY scanQueue(void);

//#ifdef __cplusplus
//}
//#endif

#endif
