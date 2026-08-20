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


#include    <vmdata.h>

//-------------------------------------------------------------------------
//
//  VMData Class and Object
//
//  If you are using C++ you need to include this module in your
//  program and read the following notes to avoid C++ memory problems.
//
//  (1) Link this OBJ in first. This causes the $$VMDATA object to be
//      the first one constructed and the last destructed in your
//      program.
//
//  (2) Borland C++ - Link this object after the C0 object module.
//
//  (3) Microsoft C++ - Do not compile this object module with Gy option as
//                      link order is not guaranteed for packaged functions
//
//  (4) Do not make the destructor routine be inline because certain 
//      compilers may change the link order
//
//  (5) Do not include any other calls to $$VMDataShutDown or
//      VMDataStartup in your program.
//
//-------------------------------------------------------------------------


//-----------------
//  Declare Class
//-----------------
class __VMDATA_CLASS
{
  public:
    __VMDATA_CLASS();
    ~__VMDATA_CLASS();
};


//------------------------------------------------------------------------
//  Constructor Routine - This is not done inline because inlining
//                        can change link order and we need this object
//                        first so that constructor before any others.
//------------------------------------------------------------------------
__VMDATA_CLASS::__VMDATA_CLASS()
{
  // Keep VMData from placing VMDataShutDown on the atexit() chain
  VMDataShutDownAtExitSet( VM_FALSE );

  // Start VMData
  VMDataStartUp( "$VMDATA$.$$$", 32, 64, VM_FALSE );
}

//------------------------------------------------------------------------
//  Destructor Routine - This is not done inline because inlining
//                       can change link order and we need this object
//                       first so that its destructor is called last
//                       in exit chain
//------------------------------------------------------------------------
__VMDATA_CLASS::~__VMDATA_CLASS()
{
  VMDataShutDown();
}


//------------------
//  Declare Object
//------------------
__VMDATA_CLASS  __VMDATA_OBJECT;

