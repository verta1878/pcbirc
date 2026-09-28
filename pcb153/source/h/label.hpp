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


/****************************************************************************/
/*                                                                          */
/*                                LABEL.HPP                                 */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*                 Data and functions for the cLABEL class.                 */
/*                                                                          */
/*==========================================================================*/
/*                                                                          */
/*                                Written by                                */
/*                            Scott Dale Robison                            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*           Copyright (C) 1993 - Clark Development Company, Inc.           */
/*                                                                          */
/****************************************************************************/

#ifndef	___LABEL_HPP___

#define	___LABEL_HPP___

/****************************************************************************/

/* Pragmas */

/****************************************************************************/

/* Included Files */

#ifdef __BORLANDC__
  #include  <alloc.h>      
#else
  #include  <malloc.h>
#endif

#include    <string.h>

#include    "list.hpp"

/****************************************************************************/

/* Defined Macros */

/****************************************************************************/

/* Types */

class cLABEL
{
	public:

        cLABEL(char * _label, long _ptr) :
			label((_label == NULL) ? NULL : strdup(_label)),
            ptr(unsigned(_ptr))
		{ }

		~cLABEL(void)
		{
			if (label != NULL) free(label);
		}

		int operator == (cLABEL &r) const { return stricmp(label,r.label) == 0; }
		int operator != (cLABEL &r) const { return stricmp(label,r.label) != 0; }
		int operator <  (cLABEL &r) const { return stricmp(label,r.label) <  0; }
		int operator <= (cLABEL &r) const { return stricmp(label,r.label) <= 0; }
		int operator >  (cLABEL &r) const { return stricmp(label,r.label) >  0; }
		int operator >= (cLABEL &r) const { return stricmp(label,r.label) >= 0; }

		static int LIBENTRY labNameMatch (const cLABEL & l, const void * s);

        char *   label;
        unsigned ptr;

};

	/*------------------------------------------------------------------*/

typedef cILIST<cLABEL> tLABLST;

/****************************************************************************/

/* Constants */

/****************************************************************************/

/* Variables */

/****************************************************************************/

/* Function Prototypes */

/****************************************************************************/

/* Inline Functions */

/****************************************************************************/

#endif
