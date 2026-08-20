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
*
*                  Pcboard's FIDO import/export class Header
*                         Author: Stan Paulsen
*                   Copyright Clark Development 1995
*
*    Member data and function prototypes  related to importing and exporting
*                            FIDO messages
*
*
*
*****************************************************************************/
#ifndef PCBTOSS_HPP
#define PCBTOSS_HPP
#ifdef FIDO

// Borland include files

// Pcboard includes files
#include "pcboard.h"
#include "messages.h"
#include "dosclass.hpp"

// Fido includes files
#include "structs.h"
#include "defines.h"
#include "data.hpp"

class cTOSSER
{
public:
                  cTOSSER(void);
                 ~cTOSSER(void);
void      LIBENTRY  send_file_notice(char *node, char *filename);
bool      LIBENTRY  exportMessages(void);
bool      LIBENTRY  toss_packet(char *filename);
bool      LIBENTRY  importMessages(void);
void      LIBENTRY  send_netmail(void);

protected:

private:

// Member Functions

 bool   LIBENTRY  unarchive(char *filename,int type);
 char*  LIBENTRY  make_outbound_list(uint &len, int conf);
 void   LIBENTRY  save_rest(void);
 void   LIBENTRY  add_netmail(void);
 void   LIBENTRY  handle_areafix(char *buffer,char *from);
 void   LIBENTRY  process_areafix_command(char *command, bool *changed,cDOSFILE & result_file);
 void   LIBENTRY  send_areafix_reply(char *to,char *from,DOSFILE *result_file,msgbasetype *MsgBase);
 void   LIBENTRY  send_areafix_list(DOSFILE *result_file,bool send_active,bool send_unactive);
 void   LIBENTRY  areafixAddRemoveAll(bool *changed,bool add);
 void   LIBENTRY  toss_message(char *buffer,msgbasetype *MsgBase);
 void   LIBENTRY  archive_pkt(void);
 void   LIBENTRY  copy_messages(uint confnum,char *list);
 void   LIBENTRY  look_for_files(void);
 void   LIBENTRY  fill_in_node_struct(char *nodestr);
 void   LIBENTRY  check_netmail_str(char *to,int to_len);
 bool   LIBENTRY  get_area_record(char *areaname);
 bool   LIBENTRY  get_area_number(int);
 bool   LIBENTRY  open_pkt_file(void);
 bool   LIBENTRY  write_pkt_header(char *nodestr);
 bool   LIBENTRY  transfer_body(char *list);
 bool   LIBENTRY  get_noderec(void);
 bool   LIBENTRY  transfer_msg_header(void);
 bool   LIBENTRY  write_fido_message(void);
 bool   LIBENTRY  add_fido_control_lines(char *list);
 bool   LIBENTRY  add_seen_by(char *list);
 bool   LIBENTRY  add_path(char * haspath);
 bool   LIBENTRY  transfer_netmail(char *to_buf,char *nodestr);
 int    LIBENTRY  archiver_type(char *filename);
 int    LIBENTRY  handle_routing(char * filename);
 char * LIBENTRY  get_area(char *buffer,char *areabuf);
 char * LIBENTRY  read_fido_body(char * msg,int buflen, bool & end);
 bool   LIBENTRY  exportPassthru(const char * userName);
 int    LIBENTRY  getNewAddr(int oldaka,const char * command);
 void   LIBENTRY  fillThisArea(NAREA_STRUCT & area,AREA_STRUCT & thisarea);
 bool   LIBENTRY  allowPrvMsgs(const char * areaname);



// Member Data
    DOSFILE                      pkt_file;
    FIDO_MESSAGE_HDR             fido_msg_hdr;
    AREA_STRUCT                  this_area;
    NODE_T                       this_node;
    FIDO_PACKET_HEADER_TYPE2     fido_header_type2;
    FIDO_PACKET_HEADER           fido_header;

    int                          conf,matched_aka;
    int                          dest_zone,dest_net,dest_node,orig_point,dest_point,orig_net,orig_node,orig_zone;
    int                          pkt_type;
    uint                         numimp;
    long                         totattimp;
    long                         grandtotalimported;
    long                         msg_count,pkt_count;
    char                         zeros[2];
    char                         pktname[80];
    char                         pkt_name[15];
    char                         messagebuff[100];
    char                         *path[100];
    char                         *seenby[100];
    char                         tear_line[100];
    char                         origin_line[100];
    bool                         netmail;
    bool                         add_tear;
    bool                         add_origin;
    bool                         add_msgid;
    bool                         dupe;
    bool                         banner_disp;
    bool                         sendlist;
    bool                         sendhelp;
    bool                         sendquery;
    bool                         sendunlink;
    bool                         direct;
    bool                         hold;
    bool                         crash;
    bool                         cerr;
    bool                         dontecho;
    pcbconftype                  confrec;
    char                         weekdays[7][3];
    char                         sendingTo[50];


};
#endif
#endif
