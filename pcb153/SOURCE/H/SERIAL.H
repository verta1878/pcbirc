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


typedef enum { SERIAL, SERIALENCRYPTED, SERIALCRC, SERIALNODES, SERIALOS1, SERIALOS2 } dispersetype;

#pragma pack(1)
typedef struct {
  char           Num[6];
  unsigned short Date;
  short          NumNodes;
  short          Version;
  char           CustName[26];
  char           OSver;        /* D=DOS, O=OS2, B=BOTH */
} serialtype;
#pragma pack()

void LIBENTRY setcollect(dispersetype Disperse);
unsigned char LIBENTRY collect(void);
void LIBENTRY openserialfile(void);
void LIBENTRY readserialfile(void);
void LIBENTRY decryptall(void);
void LIBENTRY decryptwithoutcrc(void);
void LIBENTRY checkcrc(void);
