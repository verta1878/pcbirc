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


#ifndef FIDOCFG_DOT_H
#define FIDOCFG_DOT_H

#define         MAX_ARCHIVERS           4

#define     FIELD_COLUMN        22
#define     FIDO_AREA           7
#define     FIDO_EXPORT_NODE    9
#define     FIDO_EXPORT_MAX     13

#define     CONFIG_FILE             PcbData.FidoConfig
#define     FIDO_INDEX_FILE         PcbData.FidoIndex

#define     NumEKeys 12
#define     BACKUP_FILE         "PCBFIDO.BAK"       /* backup file          */

/*****************************************************************************/

int pascal              editfido(int Before);
bool pascal             create_config_file(void);
void pascal             init_indexes(void);
void                    get_fido_record(void);
void pascal             create_index_file(void);
void pascal             Configure_Fido(void);
void pascal             edit_top(int);
void write_fido_conference(void);
extern char * unpad_str(char *str,int len);
#endif FIDOCFG_DOT_H

