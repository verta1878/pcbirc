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


#ifndef SEENBY_HPP
#define SEENBY_HPP
#ifdef FIDO

class cSEENBY
{
public:
                cSEENBY       (char *seenByList[], char *addr,char * sendee);
                cSEENBY       (void);
               ~cSEENBY       (void) {delete list;}
        void    parseList     (char *seenByList[]);
        void    createNewList (char *oldList[]);
        bool    isDupe        (char *addr);
        bool    isDupe        (void);
        bool    addAddr       (char *addr);
        bool    addList       (char *list);
        bool    find          (nodeInf &node);
        void    sort          (void);
static  int     compare       (const void *, const void *);
        void    print         (void);
        void    makeNode      (const char * str, nodeInf & node);

private:

int       seenbylen;
int       numElm;
bool      dupe;
nodeInf  *list;
nodeInf   newAddr;
};

#endif
#endif
