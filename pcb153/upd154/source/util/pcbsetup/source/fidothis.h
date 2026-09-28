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


#ifdef FIDO

#ifndef FIDOTHIS_DOT_H
#define FIDOTHIS_DOT_H

#define     CONFIG_FILE             PcbData.FidoConfig
#define     FIDO_INDEX_FILE         PcbData.FidoIndex

extern char * unpad_str(char *str,int len);
void pascal Configure_This_Address(void);
bool save_address_list(void);
bool read_config(void);
void LIBENTRY addrToNum(const char * addr, uint & zone,
																	uint & net, uint & node,
																	uint & point);

#endif
#endif
