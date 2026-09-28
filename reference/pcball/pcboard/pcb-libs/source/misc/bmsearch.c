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


#include <misc.h>

#ifdef __OS2__
  #include <ctype.h>
  #include <string.h>
#else
  #include <model.h>
  #pragma inline
#endif

#ifdef DEBUG
  #include <memcheck.h>
#endif

void LIBENTRY maketable(char *Table, char *Srch, char SrchLen) {
  #ifdef __OS2__
    /* pre-fill the entire table with SrchLen */
    memset(Table,SrchLen,256);
    /* then each letter in the search string is set in the table as the */
    /* offset from the end (i.e. in the word FAMILY the letter F would  */
    /* be stored as a 5 in the table */
    for (SrchLen--; SrchLen; SrchLen--, Srch++)
      Table[*Srch] = SrchLen;

  #else   /* ifdef __OS2__ */

  #ifdef LDATA
  asm    push ds
  asm    lds si,Srch              /* ds:si points to srch string   */
  #else
  asm    mov si,Srch              /* ds:si points to srch string   */
  #endif
  asm    mov bx,si                /* save si in bx                 */
  asm    mov al, SrchLen          /* al = length(Srch)             */
  asm    mov ah,al                /* copy it to ah                 */
  asm    mov cx,128               /* count = 128                   */
  #ifdef LDATA
  asm    les di,Table             /* es:di points to table         */
  #else
  asm    mov dx,ds
  asm    mov es,dx
  asm    mov di,Table             /* es:di points to table         */
  #endif
  asm    mov dx,di                /* save di in dx                 */
  asm    cld                      /* go forward                    */
  asm    rep stosw                /* fill Table with SrchLen       */
  asm    mov si,bx                /* restore si                    */
  asm    mov di,dx                /* restore di                    */
  asm    cbw                      /* ah = 0                        */
  asm    cmp al,1                 /* is al <= 1?                   */
  asm    jle done                 /* yes, a we're done             */
  asm    dec ax                   /* al = length mstring - 1       */
  asm    mov cl,ah                /* cx = 0                        */
  asm    mov bh,ah                /* bh = 0                        */
next:                             /*                               */
  asm    mov bl,[si]              /* bl = Srch[cx]                 */
  asm    mov dx,ax                /* dx = length matchstr - 1      */
  asm    sub dx,cx                /* dx = dx - matchstr[position]  */
  asm    mov es:[bx+di],dl        /* put value in table            */
  asm    inc si                   /* ds:si points to next pos      */
  asm    inc cx                   /* increment counter             */
  asm    cmp cx,ax                /* are we done                   */
  asm    jne next                 /* if not, do it again           */
done:;                            /*                               */
  #ifdef LDATA
  asm    pop ds                   /* restore turbo dseg            */
  #endif

  #endif  /* ifdef __OS2__ */
}


/********************************************************************
*
*  Function:  bmsearch()
*
*  Desc    :  A case-sensitive search algorythm based on the Boyer-Moore table
*             driven search routines.
*
*             *Buf      points to the text to be searched (NULLs are ignored)
*             BufLen    defines the length of *Buf
*             Table     points to a 256-byte table created by maketable()
*             *Srch     points to the search criteria (NULLs are ignored)
*             SrchLen   defines the length of the search criteria
*
*  WARNING :  For speed purposes, this routine has been written to ASSUME that
*             the *Buf and *Srch criteria are in the SAME DATA SEGMENT.
*
*             This means that if the search criteria was allocated on the stack
*             then the buffer being searched must also be allocated there.
*
*  Returns :  0 to indicate no match was found
*             otherwise the OFFSET+1 of the first character of the match found
*/

int LIBENTRY bmsearch(char *Buf, int BufLen, char *Table, char *Srch, char SrchLen) {
  #ifdef __OS2__
    char  LastCharInSrch;
    char  C;
    char *p;
    char *q;
    char *EndBuf;
    char *SrchEnd;

    if (SrchLen == 0 || SrchLen > BufLen)
      return(0);

    if (SrchLen == 1) {
      if ((p = (char *) memchr(Buf,*Srch,BufLen)) == NULL)
        return(0);
      return((int) (p-Buf) + 1);    /* 0=not found, so return offset+1 */
    }

    SrchLen--;
    SrchEnd = Srch + SrchLen;
    LastCharInSrch = *SrchEnd;      /* last character in the search string  */
    SrchEnd--;                      /* point to next-to-last character      */
    SrchLen--;                      /* set SrchLen to point to next-to-last */
    EndBuf = Buf + BufLen;          /* record a ptr to the end of Buf       */

    p = Buf + SrchLen;              /* jump forward SrchLen characters      */
    do {
      C = *p;
      if (C == LastCharInSrch) {    /* does the last character match?       */
        q = p;                      /* yes, scan backwards for the rest     */
        do {
          C = *(--q);
          if (C != *SrchEnd) {         /* does the next character match?    */
            SrchEnd = Srch + SrchLen;  /* no, restore SrchEnd for next time */
            goto next;                 /*     then go back to main loop     */
          }
        } while (--SrchEnd >= Srch);   /* yes, is there more to compare?    */
        /* no more to compare, we must have found a match! */
        /* returning 0 means not found, so add 1 more for offset+1 */
        return((int) (q - Buf) + 1);
      }
      /* get next end-of-string offset into the buffer */
      next:
      p += Table[C];
    } while (p < EndBuf);   /* more to search? */
    /* no more to search, no match found, return 0 to indicate not found */
    return(0);

  #else  /* ifdef __OS2__ */

  asm    xor  ax, ax
  asm    mov  dx, ax
  #ifdef LDATA
  asm    push ds
  asm    lds  si, Srch          /* ds:si points to srch       */
  #else
  asm    mov  si, Srch          /* ds:si points to srch       */
  #endif
  asm    mov  dl, SrchLen       /* dl holds length of srchlen */
  asm    cmp  dl, 1             /* is it < 1 char long?       */
  asm    jg   boyer             /* it's > 1 char              */
  asm    jl   notfound          /* it's an emty string        */

                                /* it's only a 1 char string so just scan for it. */
  asm    mov  al, [si]          /* al contains the char to search for             */
  asm    mov  cx, BufLen        /* cx contains length of buffer                   */
  #ifdef LDATA
  asm    les  di, Buf           /* es:di points to buf                            */
  #else
  asm    mov  bx, ds
  asm    mov  es, bx
  asm    mov  di, Buf           /* es:di points to buf                            */
  #endif
  asm    mov  bx, di            /* bx saves offset of buf                         */
  asm    cld                    /* go forward                                     */
  asm    repne scasb            /* scan buf for char in al                        */
  asm    jne   notfound         /* char wasn't found                              */
  asm    xchg  ax,di            /* ax holds offset of buf where char was found    */
  asm    sub   ax,bx            /* subtract offset of buf                         */
  asm    jmp  exit              /* we're done                                     */

boyer:
  asm    dec  dl                /* dl = length of srch - 1                       */
  asm    add  si, dx            /* ds:si points to srch[length(srch)-1]          */
  #ifdef LDATA
  asm    les  di, Buf           /* es:di points to buf                           */
  #else
  asm    mov  bx, ds
  asm    mov  es, bx
  asm    mov  di, Buf           /* es:di points to buf                           */
  #endif
  asm    mov  cx, di            /* cx = di                                       */
  asm    add  cx, BufLen        /* cx now contains offset of last char in buf+1  */
  asm    dec  cx                /* now it contains offset of last char in buf    */
  asm    add  di,dx             /* es:di points to offset of first pos to search */
  asm    inc  dl                /* dl = length of srch (was decremented above)   */
  asm    mov  dh, [si]          /* dh holds srch[len]                            */
  #ifdef LDATA
  asm    lds  bx, Table         /* ds:bx points to table                         */
  #else
  asm    mov  bx, Table         /* ds:bx points to table                         */
  #endif
  asm    std                    /* go backwards                                  */
  asm    jmp  short comp        /* skip table and start comparing                */

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
/*   At this point the register usage can be summarized as follows:         */
/*   AH    = 0   used to reinitialize registers at times                    */
/*   AL    = character from Buf[]                                           */
/*   CX    = ofset to last character in Buf[]                               */
/*   DH    = last character in Srch[]                                       */
/*   DL    = length of Srch[]                                               */
/*   DS:BX = points to Table[]     NOTE: it is assumed that Table and Srch  */
/*   DS:SI = points to Srch[]      reside in the SAME segment!!!            */
/*   ES:DI = points to Buf[]                                                */
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

nexttable:
  asm    xlat
  asm    add  di, ax            /* add value to buf */
comp:
  asm    cmp  cx, di            /* compare new offset with offset of last char in buffer */
  asm    jb   notfound          /* if less or equal, compare again else it's not found   */
  asm    mov  al, es:[di]       /* put buf char in al                                    */
  asm    cmp  dh, al            /* compare srch[len] with buf[]                          */
  asm    jne  nexttable         /* if not same, go back and try again                    */
  asm    push cx                /* else the last chars are equal so compare more.        */
  asm    mov  cl, dl            /* cl = length srch                                      */
  asm    mov  ch, ah            /* ch = 0                                                */
  asm    repe cmpsb             /* compare backwards while cx>0 and es:di=ds:si          */
  asm    je   found             /* if all the remaining chars matched we're done         */
  asm    mov  al, dl            /* else restore everything and try again                 */
  asm    sub  al, cl
  asm    add  di, ax
/*asm    inc  di*/   /* if we increment then we might pass the LAST CHAR!!! */
  asm    add  si, ax
  asm    mov  al, dh            /* put buf char back in al */
  asm    pop  cx
  asm    jmp  short nexttable
found:                          /* di points to pos   */
  asm    pop  ax                /* get junk off stack */
  #ifdef LDATA
  asm    les  ax, Buf           /* ax points to buf   */
  #else
  asm    mov  ax, Buf           /* ax points to buf   */
  #endif
  asm    xchg ax, di
  asm    sub  ax, di
  asm    add  ax, 2
  asm    jmp  short exit
notfound:
  asm    xor  ax, ax            /* result = 0 */
exit:
  asm    cld
  #ifdef LDATA
  asm    pop  ds
  #endif
  return(_AX);

  #endif  /* ifdef __OS2__ */
}


/********************************************************************
*
*  Function:  bmisearch()
*
*  Desc    :  A case-INsensitive search algorythm based on the Boyer-Moore
*             table driven search routines.
*
*             *Buf      points to the text to be searched (NULLs are ignored)
*             BufLen    defines the length of *Buf
*             Table     points to a 256-byte table created by maketable()
*             *Srch     points to the search criteria (NULLs are ignored)
*             SrchLen   defines the length of the search criteria
*
*  WARNING :  For speed purposes, this routine has been written to ASSUME that
*             the *Buf and *Srch criteria are in the SAME DATA SEGMENT.
*
*             This means that if the search criteria was allocated on the stack
*             then the buffer being searched must also be allocated there.
*
*  NOTE    :  The search criteria must be converted to UPPERCASE before
*             calling maketable() or bmisearch().
*
*  Returns :  0 to indicate no match was found
*             otherwise the OFFSET+1 of the first character of the match found
*/

int LIBENTRY bmisearch(char *Buf, int BufLen, char *Table, char *Srch, char SrchLen) {
  #ifdef __OS2__
    char  LastCharInSrch;
    char  C;
    char *p;
    char *q;
    char *EndBuf;
    char *SrchEnd;

    if (SrchLen == 0 || SrchLen > BufLen)
      return(0);

    EndBuf = Buf + BufLen;          /* record a ptr to the end of Buf       */

    if (SrchLen == 1) {
      LastCharInSrch = *Srch;
      for (p = Buf; p < EndBuf; p++) {
        if (toupper(*p) == LastCharInSrch)
          return((int) (p-Buf) + 1);    /* 0=not found, so return offset+1 */
      }
      return(0);  /* not found */
    }

    SrchLen--;
    SrchEnd = Srch + SrchLen;
    LastCharInSrch = *SrchEnd;      /* last character in the search string  */
    SrchEnd--;                      /* point to next-to-last character      */
    SrchLen--;                      /* set SrchLen to point to next-to-last */

    p = Buf + SrchLen;              /* jump forward SrchLen characters      */
    do {
      C = (char) toupper(*p);
      if (C == LastCharInSrch) {    /* does the last character match?       */
        q = p;                      /* yes, scan backwards for the rest     */
        do {
          C = (char) toupper(*(--q));
          if (C != *SrchEnd) {         /* does the next character match?    */
            SrchEnd = Srch + SrchLen;  /* no, restore SrchEnd for next time */
            goto next;                 /*     then go back to main loop     */
          }
        } while (--SrchEnd >= Srch);   /* yes, is there more to compare?    */
        /* no more to compare, we must have found a match! */
        /* returning 0 means not found, so add 1 more for offset+1 */
        return((int) (q - Buf) + 1);
      }
      /* get next end-of-string offset into the buffer */
      next:
      p += Table[C];
    } while (p < EndBuf);   /* more to search? */
    /* no more to search, no match found, return 0 to indicate not found */
    return(0);

  #else   /* ifdef __OS2__ */

  asm    xor  ax, ax
  asm    mov  dx, ax
  #ifdef LDATA
  asm    push ds
  asm    lds  si, Srch          /* ds:si points to srch       */
  #else
  asm    mov  si, Srch          /* ds:si points to srch       */
  #endif
  asm    mov  dl, SrchLen       /* dl holds length of srchlen */
  asm    cmp  dl, 0             /* is it empty?               */
  asm    je   notfound          /* it's an emty string exit   */

  asm    dec  dl                /* dl = length of srch - 1                       */
  asm    add  si, dx            /* ds:si points to srch[length(srch)-1]          */
  #ifdef LDATA
  asm    les  di, Buf           /* es:di points to buf                           */
  #else
  asm    mov  bx, ds
  asm    mov  es, bx
  asm    mov  di, Buf           /* es:di points to buf                           */
  #endif
  asm    mov  cx, di            /* cx = di                                       */
  asm    add  cx, BufLen        /* cx now contains offset of last char in buf+1  */
  asm    dec  cx                /* now it contains offset of last char in buf    */
  asm    add  di,dx             /* es:di points to offset of first pos to search */
  asm    mov  dh, [si]          /* dh holds srch[len]                            */
  #ifdef LDATA
  asm    lds  bx, Table         /* ds:bx points to table                         */
  #else
  asm    mov  bx, Table         /* ds:bx points to table                         */
  #endif
  asm    jmp  short comp        /* skip table and start comparing                */

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
/*   At this point the register usage can be summarized as follows:         */
/*   AH    = 0   used to reinitialize registers at times                    */
/*   AL    = character from Buf[]                                           */
/*   CX    = ofset to last character in Buf[]                               */
/*   DH    = last character in Srch[]                                       */
/*   DL    = length of Srch[]                                               */
/*   DS:BX = points to Table[]     NOTE: it is assumed that Table and Srch  */
/*   DS:SI = points to Srch[]      reside in the SAME segment!!!            */
/*   ES:DI = points to Buf[]                                                */
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

nexttable:
  asm    xlat
  asm    add  di, ax            /* add value to buf */
comp:
  asm    cmp  cx, di            /* compare new offset with offset of last char in buffer */
  asm    jb   notfound          /* if less or equal, compare again else it's not found   */
  asm    mov  al, es:[di]       /* put buf char in al                                    */
  asm    cmp  al, 'a'
  asm    jb   upped1
  asm    cmp  al, 'z'
  asm    ja   upped1
  asm    and  al, 0DFh
upped1:
  asm    cmp  dh, al            /* compare srch[len] with buf[]                          */
  asm    jne  nexttable         /* if not same, go back and try again                    */

  asm    push si
  asm    push di
  asm    push cx
  asm    mov  cl, dl            /* cl = length srch                                      */
  asm    mov  ch, ah            /* ch = 0                                                */
  asm    jcxz done
comploop:
  asm    dec  si
  asm    dec  di
  asm    mov  al, es:[di]
  asm    cmp  al, 'a'
  asm    jb   upped2
  asm    cmp  al, 'z'
  asm    ja   upped2
  asm    and  al, 0DFh
upped2:
  asm    cmp  al,[si]
  asm    loope comploop
done:
  asm    mov  al, dh
  asm    pop  cx
  asm    pop  di
  asm    pop  si
  asm    jne  nexttable

found:                          /* di points to pos   */
  #ifdef LDATA
  asm    les  ax, Buf           /* ax points to buf   */
  #else
  asm    mov  ax, Buf           /* ax points to buf   */
  #endif
  asm    xchg ax, di
  asm    xor  dh, dh
  asm    sub  ax, dx
  asm    sub  ax, di
  asm    inc  ax
  asm    jmp  short exit
notfound:
  asm    xor  ax, ax            /* result = 0 */
exit:
  #ifdef LDATA
  asm    pop  ds
  #endif
  return(_AX);

  #endif  /* ifdef __OS2__ */
}


#ifdef TEST
#include <string.h>
#include <stdlib.h>

char Buf[16384];

void main(void) {
  char Table[256];
  char Srch[256];
  char SrchLen;
  int  BufLen;
  int  X;
  int  Y;
  int  Found;

//strcpy(Srch,"This is a test of the emergency broadcast system.");
  strcpy(Srch,"test");
  SrchLen = (char) strlen(Srch);

  for (X = 0; X < sizeof(Buf); X++)
    Buf[X] = (char) random(256);

  memcpy(Buf+12000,Srch,SrchLen);
  BufLen = sizeof(Buf);

  maketable(Table,Srch,SrchLen);

//for (Y = 0; Y < 10; Y++) {
    for (X = 0; X < 5000; X++)
      Found = bmsearch(Buf,BufLen,Table,Srch,SrchLen);
//}

  printf("%d\n",Found);

/*
  strupr(Srch);
  maketable(Table,Srch,SrchLen);

  printf("%d\n", bmisearch(Buf,BufLen,Table,Srch,SrchLen));
*/
}
#endif  /* ifdef __OS2__ */
