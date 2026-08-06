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


******************************************************************************
* Function: bool getINTLinfo(const char*,char*,int,char*,int)                *
* Purpose : Locate INTL FIDO address information in a message body, (msgBuf) *
*           and copy the destination address into the paramter dAddr as well *
*           as the source address into sAddr.                                *
* Returns : Returns TRUE on success and FALSE on failure                     *
******************************************************************************

//bool getINTLinfo(const char * msgBuf, char * dAddr, int dAddrSz, char * sAddr,
//                 int sAddrSz);
bool getINTLinfo(const char * msgBuf, char * dAddr, int dAddrSz, char * sAddr,
                 int sAddrSz)
{
char * addrPtr    = NULL;
char * addrEndPtr = NULL;
char   tmpdAddr[20];
char   tmpsAddr[20];

    // Look fro the key word INTL in the message body.
    if( (addrPtr = strstr(msgBuf,"INTL")) == NULL) return FALSE;

    // Find the beginning of the address.
    while(!isdigit(addrPtr) && addrPtr != '\x0') addrPtr++;

    // end of message body
    if(addPtr == '\x0') return FALSE;
    // Set end pointer to beginning of address
    addreEndPtr = addrPtr;

    // Now walk end pointer to the end of the address
    while(addrEndPtr != ' ' && addrEndPtr != '\x0') addrEndPtr++;

    // Copy first address into temp buffer
    if(addrEndPtr != '\x0' && strlen(addrPtr) < sizeof(tmpdAddr))
      maxstrcpy(tmpdAddr,addrPtr,addrEndPtr-addrPtr);
    else
      return FALSE;

    // Make sure it's a valid address
    if(!strchr(tmpdAddr,':') || !strchr(tmpaddr,'/')) return FALSE;

    // Move to the beginning of the next address
    addrPtr = addrEndPtr;
    while(!isdigit(addrPtr) && addrPtr != '\x0') addrPtr++,addrEndPtr++;

    // Now walk to the end of the second address
    if(addrPtr != '\x0')
      while(addrEndPtr != ' ' && addrEndPtr != '\x0') addrEndPtr++;
    else
      return FALSE;

    // Copy the address into a temp buffer
    if(addrEndPtr != '\x0' && (addrEndPtr - addrPtr) < sizeof(tmpsAddr))
      maxstrcpy(tmpsAddr,addrPtr,addrEndPtr-addrPtr)
    else
      return FALSE;

    if(!strchr(tmpsAddr,':' || !strchr(tmpsAddr,'/')
      return FALSE;
    else
    {
        maxstrcpy(dAddr,tmpdAddr,dAddrSz);
        maxstrcpy(sAddr,tmpsAddr,sAddrSz);
    }
    return TRUE;
}











