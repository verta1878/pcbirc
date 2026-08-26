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


#ifndef PASSTHRU_HPP


#define PASSTHRU_HPP
/*******************************************************************************

								PASSTHRU.HPP
					Copyright (C) 1995 Clark Development

********************************************************************************
								 Author
								Stan Paulsen

This module contains declarations for classes and functions related to the
passthrough module for PCBOARD FIDO.

********************************************************************************/

#include <dir.h>
#include "dosclass.hpp"
#include <string.h>
#include "dosfunc.h"


#ifndef PCBSETUP
#include "pcboard.h"
#endif
#include "structs.h"

// Prototypes

typedef struct
{
	unsigned char 	R :1; 				// R registration flag
	unsigned char 	S :1; 				// S registration flag
	unsigned char 	A :1; 				// Active flag. This is so I know ehn this record should be packed out
	unsigned char  reserved : 5; //Reserved
}REGREC;		// This structure should be one byte in length


class cPTINFO
{
	private:
				char				 datafile[MAXFLEN],userfile[MAXFLEN];
				cDOSFILE		 dfile;
				FUSERS		*  users;
				REGREC		*  urecs;
        uint         numusers;
				bool				 err,needtopack;

		void		 LIBENTRY 		 create 					 (void);
		bool		 LIBENTRY 		 getUserIndex 		 (FUSERS & user,unsigned int & usernum);
		void		 LIBENTRY 		 reportErr				 (void);
		void		 LIBENTRY 		 readUrecs				 (void);
		void		 LIBENTRY 		 writeUrecs 			 (void);
		void		 LIBENTRY 		 readUsers				 (void);
		void		 LIBENTRY 		 writeUsers 			 (void);
		void		 LIBENTRY 		 ptPack 					 (void);


	protected:

	public:
                        cPTINFO            (void);
											 ~cPTINFO 					 (void);




    bool       LIBENTRY   userInArea         (const char * area, FUSERS & user);
    bool       LIBENTRY   userInArea         (const char * area, unsigned int usernum);
    void       LIBENTRY   putUserInArea      (unsigned int usernum,const char * area);
    void       LIBENTRY   putUserInArea      (FUSERS & user,const char * area);
    void       LIBENTRY   addArea            (const char * area);
    char *     LIBENTRY   getUser            (unsigned int usernum);
    bool       LIBENTRY   modifyRegRec       (REGREC & newrec);
    char *     LIBENTRY   seekToAreaNum      (unsigned int areanum);
    bool       LIBENTRY   seekToUser         (unsigned int usernum);
    bool       LIBENTRY   seekToArea         (const char * areaname);
    void       LIBENTRY   removeUserFromArea (FUSERS & user, const char * area);
	unsigned int LIBENTRY 	headerSize				 (void);
	unsigned int LIBENTRY 	areaRecSize 			 (void);
	unsigned int LIBENTRY 	numAreas					 (void);
	unsigned int LIBENTRY 	countUsersInArea	 (const char * area);
	bool										Err 							 (void) {return err;}
  void         LIBENTRY   removeFile         (void);
  void         LIBENTRY   delOldUsers        (void);
  void         LIBENTRY   updateUsers        (void);
  uint         LIBENTRY   getUserCount       (void){return numusers;}
  void         LIBENTRY   removeUser         (FUSERS & user);
  void         LIBENTRY   addUser            (FUSERS & user);

#ifdef PCBSETUP

	void	 LIBENTRY 				removeArea				 (const char * area);

#endif


};


class cPASSTHROUGH
{
	private:
		char msgfile[MAXFLEN];
		cDOSFILE msg;
    bool     err;


	bool	LIBENTRY		getNextMsg			(FIDO_MESSAGE_HDR * hdr,char * msgbuf,unsigned int buflen);
	bool	LIBENTRY		getAreaName 		(const char * msg,char * areaname);

	protected:

	public:
                           cPASSTHROUGH   (void);
                          ~cPASSTHROUGH   (void){if(msg.isOpen()) msg.close();}

  bool       LIBENTRY     addMsg              (const FIDO_MESSAGE_HDR * hdr, const char * msg);
unsigned int LIBENTRY     getNextMsgForUser   (FUSERS & user,DOSFILE & file,cPTINFO & info,bool & sent);
    void     LIBENTRY     removeMsgFile       (void){unlink(msgfile);}
    bool     LIBENTRY     needToPass          (void) {
                                                       if(fileexist(msgfile) == 255) return FALSE;
                                                       return TRUE;
                                                     }
  void       LIBENTRY     removeFile          (void);
};

class cFUSER
{
public:

		FUSERS u;

					 cFUSER(unsigned int z=0,unsigned int nt=0,unsigned int no=0,unsigned int p=0) {u.zone=z;u.net=nt;u.node=no;u.point=p;}

		char * addrStr(void){if(u.point == 0) sprintf(address,"%u:%u/%u",u.zone,u.net,u.node);
												 else sprintf(address,"%u:%u/%u.%u",u.zone,u.net,u.node,u.point);
												 return address;}
private:
		char	 address[25];

};

#endif
