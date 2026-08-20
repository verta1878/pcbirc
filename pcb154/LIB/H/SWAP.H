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


#ifdef __cplusplus
extern "C" {
#endif

int swap (char *program_name,
          char *command_line,
          char *exec_return,
          char *swap_fname,
          char *envp);

int swapenv (char *program_name,
             char *command_line,
             char *exec_return,
             char *swap_fname);


int ems_installed (void);
int xms_installed (void);

#ifdef __cplusplus
}
#endif

/* The return code from swap() will be one of the following.  Codes other    */
/* than SWAP_OK (0) indicate that an error occurred, and thus the program    */
/* has NOT been swapped, and the new program has NOT been executed.          */

#define SWAP_OK         (0)         /* Swapped OK and returned               */
#define SWAP_NO_SHRINK  (1)         /* Could not shrink DOS memory size      */
#define SWAP_NO_SAVE    (2)         /* Could not save program to XMS/EMS/disk*/
#define SWAP_NO_EXEC    (3)         /* Could not execute new program         */


/* If swap() returns 3, SWAP_NO_EXEC, the byte/char pointed to by the        */
/* parameter exec_return will be one of the following standard DOS error     */
/* codes, as specified in the DOS technical reference manuals.               */

#define BAD_FUNC        (0x01)   /* Bad DOS function number--unlikely          */
#define FILE_NOT_FOUND  (0x02)   /* File not found--couldn't find program_name */
#define ACCESS_DENIED   (0x05)   /* Access denied--couldn't open program_name  */
#define NO_MEMORY       (0x08)   /* Insufficient memory to run program_name    */
#define BAD_ENVIRON     (0x0A)   /* Invalid environment segment--unlikely      */
#define BAD_FORMAT      (0x0B)   /* Format invalid--unlikely                   */

