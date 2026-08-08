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


#ifndef ___FIDOCFG_HPP___

#define ___FIDOCFG_HPP___

#ifdef FIDO

#include    <dir.h>

#include    <dosclass.hpp>

#include    <defines.h>
#include    <structs.h>

class cFIDOCFG
{

    public:

        char * srcName;
        char   destName [ 128 + 1 ];

        cDOS src;
        cDOS dest;

        unsigned int version;
        unsigned int numAreas;

         cFIDOCFG ( void );

        ~cFIDOCFG ( void );

        void pascal addNewArea ( AREA_STRUCT & newArea );
        void pascal addNewArea ( int conf, char * name, char * msgs );

    protected:

    private:

};

#endif

#endif

