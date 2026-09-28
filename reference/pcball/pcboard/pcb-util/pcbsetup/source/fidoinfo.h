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

#ifndef FIDOINFO_DOT_H
#define FIDOINFO_DOT_H

#define         CONFIG_FILE             PcbData.FidoConfig


#define         IN_PACKETS_LINE         10
#define         OUT_PACKETS_LINE        11
#define         OUT_MSGS_LINE           12
#define         WORK_DIR_LINE           13

#define         ZIP_LINE                5
#define         ZIP_SW_LINE             6
#define         UNZIP_LINE              7
#define         UNZIP_SW_LINE           8

#define         ARJ_LINE                9
#define         ARJ_SW_LINE             10
#define         UNARJ_LINE              11
#define         UNARJ_SW_LINE           12

#define         ARC_LINE                13
#define         ARC_SW_LINE             14
#define         UNARC_LINE              15
#define         UNARC_SW_LINE           16

#define         LZH_LINE                17
#define         LZH_SW_LINE             18
#define         UNLZH_LINE              19
#define         UNLZH_SW_LINE           20

void pascal             edit_fido_dirs(void);


#endif
#endif
