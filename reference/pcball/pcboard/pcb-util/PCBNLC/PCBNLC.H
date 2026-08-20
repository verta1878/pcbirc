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


#ifndef PCBNLC_DOT_H
#define PCBNLC_DOT_H


/* Various defines */
//#define START_COL       9
#define STATUS_LINE     4
#define MESSAGES        5
#define COMP_LINE       6
#define LAST_COL        79

/****************************************************************************/
/* Function prototypes                                                      */

//#ifdef __cplusplus
//extern "C"
//{
//#endif

int  pascal read_line(DOSFILE *filebuffer,char *buffer,int max_len);
int  pascal process_line(char *buffer);
bool pascal process_node_list(void);
void pascal print_help(void);
void pascal print_banner(void);
void pascal abort_compile(bool flag);
int  pascal read_list_line(DOSFILE *filebuffer,char *buffer,int max_len);
int  pascal process_command(char *buffer,DOSFILE *nodelist_file,DOSFILE *nodediff_file,DOSFILE *output_file);
int  pascal checkdatfile(char Ver[]);
bool pascal translate_phone(char *phone);
bool pascal process_nodediff(void);
char * pascal findstartofname(char *Path);

bool get_node(unsigned int zone, unsigned int net, unsigned int node, NODE_REC * noderec);

//#ifdef __cplusplus
//}
//#endif

#endif
