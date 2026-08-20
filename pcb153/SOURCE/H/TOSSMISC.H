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


/****************************************************************************

			 Miscellanious suppprt functions for the tosser
						  Author: Stan Paulsen
					Copyright Clark Development 1995


*****************************************************************************/

#ifdef FIDO
#include "pcboard.h"
#include "structs.h"
#include "dosclass.hpp"

#ifndef PCBSETUP
void LIBENTRY  writeFidolog(const char * entry, int type);
#ifdef __cplusplus
extern "C" {
#endif
bool LIBENTRY  toss_messages(void);
bool LIBENTRY  copy_pcb_messages(void);
void LIBENTRY  de_init_users(void);
#ifdef __cplusplus
}
#endif
#endif

void LIBENTRY  checkDateFormat(char * date);
void LIBENTRY  checkTimeFormat(char * time);
void LIBENTRY  getSysopNam(char * nameBuf);
void LIBENTRY  TK_CopyFile(char _FAR_ *source,char _FAR_ *destination);
void LIBENTRY  get_fido_rec(long recno);
void LIBENTRY  translate_date(char *fido_date,char *pcb_date,char *time,int fidolen);
bool LIBENTRY  match_address(char *oldaddr,char *newaddr);
void LIBENTRY  print_banner(void);
bool LIBENTRY  isEmptyMessage(const char * message);
void LIBENTRY  createAttach(const unsigned int confNum,const char * fileName, bool move);
void LIBENTRY  generateImportList(char * fileSpec,FILEDATETIME * list,int & numFiles);
int 		 importListCmp(const void * a, const void *b);
char * LIBENTRY xlateFidoText(char * s,char HighAsciiSetting);
char * LIBENTRY makeUniqueFZipName ( char * name );
bool LIBENTRY  re_route(char * address);
void LIBENTRY  translate_date_time(char *inputstr,char *destdate,char *desttime);
void LIBENTRY  init_users(void);
long LIBENTRY  find_fido_rec(char * addr,bool cont,bool reset,long *new_offset);
bool LIBENTRY  toss_messages(void);
bool LIBENTRY  copy_pcb_messages(void);
void LIBENTRY  moveFileToDir(char * file, char * dir,char * description);
void LIBENTRY  updateFidoUserRecord(void);
void LIBENTRY  read_str(char *buffer,int limit,DOSFILE & file);
int  LIBENTRY  determineFidoPktType(FIDO_PACKET_HEADER_TYPE2 & type2Hdr);
bool LIBENTRY  readType1PktHdr(FIDO_PACKET_HEADER & type1Hdr,DOSFILE & pkt);
bool LIBENTRY  checkPacketPassword(char * fidoUserAddr, char * pktPassword);
void LIBENTRY  freeAndNull(char ** block);
bool LIBENTRY  readFidoMessageHeader(FIDO_MESSAGE_HDR & msghdr,DOSFILE & pkt_file);
bool LIBENTRY  myAKA(unsigned zone, unsigned net, unsigned node, unsigned point);
bool LIBENTRY  isExcluded(char * addr,char * list);
char * LIBENTRY getPath(char * pathname);
char *		 genMSGID(char * addr, unsigned int conf);
void LIBENTRY  genResponseMsg(FUSERS & user,int type, const char * text);
int  LIBENTRY  matchAKA(unsigned int zone, unsigned int net, NADDRESS * addr, int numaddr);
void LIBENTRY  forwardRequest(const char * area,bool add);
void LIBENTRY  cleanUpAfix(FUSERS & user,cDOSFILE & file);
void LIBENTRY  prepAfix(cDOSFILE & file);
void LIBENTRY  writeForwardsToFile(cDOSFILE & file);
void LIBENTRY  doRescan(const char * command);
void LIBENTRY  addFidoConf(const char * areatag);
int  LIBENTRY  makeMyAkaList(char * buf,int buflen,int whichlist);
void LIBENTRY  findMyPathAddr(char * to, NADDRESS & myrec);
char * LIBENTRY  eatSpaces(char * ptr);
char * LIBENTRY  eatNonSpaces(char * ptr);
int  LIBENTRY  fgetbyte(unsigned int waittime, bool & kbd);
void LIBENTRY  clearLine(int x, int y);
void LIBENTRY  getPointAddr(const char * msg,uint & frmpt,uint & topnt);
bool LIBENTRY  getPrimAkaForNet(FUSERS & akauser, FUSERS & destuser);
void LIBENTRY  writeFailsToPkt(cDOSFILE & pkt);
void LIBENTRY  writeFailsToFile(const char * msg);
void LIBENTRY  prepBodyForPCB(char * body);
void LIBENTRY  addrToNum(const char * addr, uint & zone, uint & net, uint & node,uint & point);
#endif
