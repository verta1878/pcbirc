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


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
// Structures for MODEMS.DAT used both by MSETUP.EXE and PCBMODEM.EXE
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
typedef struct {
   unsigned  Date,              // Current revision date of MODEMS.DAT
             NumOfModems;       // Total number of modems
   long      NextMdmOffset;     // Offset of next modem added
   char      Version [ 8 ];     // Version number of PCBoard
} headertype;                   // Size = 16 bytes

typedef struct {
   char      Manufs  [ 15 ],    // Manufacturer's Name
             Name    [ 25 ];    // Modem name
   unsigned  StdCfg,            // Standard configuration number
             Number;            // Modem number
   long      Offset;            // Offset in MODEMS.DAT
} manufdatatype;                // Size = 48 bytes

typedef struct {
   char      V42,               // Uses v.42
             Fax,               // Has fax capabilities
             CallID,            // Has callerID
             Seconds,           // Seconds to wait for carrier
             Factory,           // Default factory configuration
             Eprom,             // Has an eprom
             Lock,              // Lock in the port speed
             Hspd    [ 15 ],    // High speed lock
             Lspd    [ 15 ],    // Low speed lock
             Send1   [ 40 ],    // Nram string #1
             Send2   [ 40 ],    // Nram string #2
             Send3   [ 40 ],    // Nram string #3
             Cmnt1   [ 50 ],    // Comment #1
             Cmnt2   [ 50 ],    // Comment #2
             Cmnt3   [ 50 ],    // Comment #3
             Init1   [ 40 ],    // Initialize string #1
             Init2   [ 40 ],    // Initialize string #2
             Offhook [ 40 ],    // Offhook string
             Who     [ 15 ];    // Employee who last updated record
   unsigned  Date;              // Julian date of last update
   long      Baud;              // Max Speed of modem
} modemdatatype;                // Size = 448 bytes

extern  headertype      Header;
extern  modemdatatype   Modemdata;
extern  manufdatatype   * Manufdata;
