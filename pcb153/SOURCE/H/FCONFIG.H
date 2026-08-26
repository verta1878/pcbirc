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

#define     AREA_RECS     1
#define     ADDR_RECS     2
#define     NODE_RECS     4
#define     XLAT_RECS     8
#define     LIST_RECS     16
#define     FREQ_RECS     32

/*#ifdef __cplusplus */
/*extern "C" { */
/*#endif */

extern bool pascal read_fido_config(int flags);
extern bool pascal write_fido_config(void);
extern bool pascal make_fido_config(void);
extern void pascal free_fido_memory(void);

/*#ifdef __cplusplus */
/*} */
/*#endif */

#endif
