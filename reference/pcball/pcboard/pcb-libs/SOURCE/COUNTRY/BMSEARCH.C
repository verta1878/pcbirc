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


#ifdef __OS2__
  #include <ctype.h>
  #include <string.h>
#else
  #include <model.h>
  #pragma inline
#endif

#include <country.h>
#ifdef DEBUG
#include <memcheck.h>
#endif


/********************************************************************
*
*  Function:  maketable()
*
*  Desc    :  Creates a 256-byte Table which is used by the Boyer-Moore
*             search routines to speed search times.
*
*             *Table   must point to a 256-byte buffer (NULLs are ignored)
*             *Srch    may contain NULL characters (or not)
*             SrchLen  determines the length of the search text
*/

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
  asm    xor ah,ah                /* ah = 0                        */
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
  asm    pop ds
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
          q--;
          if (*q != *SrchEnd) {        /* does the next character match?    */
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
*  NOTE    :  This version of bmisearch() has been modified to be compatible
*             with the COUNTRY-INDEPENDENT routines.  To accomplish this, it
*             uses an UpperCase Table to convert the text being searched to
*             uppercase during the comparison process.  Prior to using this
*             routine the UpperCase Table needs to have been initialized!
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
        if (UpperCase[*p] == LastCharInSrch)
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
      C = UpperCase[*p];
      if (C == LastCharInSrch) {    /* does the last character match?       */
        q = p;                      /* yes, scan backwards for the rest     */
        do {
          q--;
          if (UpperCase[*q] != *SrchEnd) { /* does the next character match?    */
            SrchEnd = Srch + SrchLen;      /* no, restore SrchEnd for next time */
            goto next;                     /*     then go back to main loop     */
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


  int  SaveBx;
  #ifdef LDATA
  #ifndef CPU386
  int  TableDs;
  int  UpCaseDs;
  #endif
  #endif

  asm    xor  ax, ax
  asm    mov  dx, ax
  #ifdef LDATA
  asm    push ds
  asm    lds  si, Srch      /* ds:si points to srch       */
  #ifdef CPU386
  asm    push bp
  #endif
  #else
  asm    mov  si, Srch      /* ds:si points to srch       */
  #endif
  asm    mov  dl, SrchLen   /* dl holds length of srchlen */
  asm    cmp  dl, 0         /* is it empty?               */
  asm    je   notfound      /* it's an emty string exit   */

  asm    dec  dl            /* dl = length of srch - 1               */
  asm    add  si, dx        /* ds:si points to srch[length(srch)-1]  */
  #ifdef LDATA
  asm    les  di, Buf       /* es:di points to buf                   */
  #else
  asm    mov  bx, ds
  asm    mov  es, bx
  asm    mov  di, Buf       /* es:di points to buf                           */
  #endif
  asm    mov  cx, di        /* cx = di                                       */
  asm    add  cx, BufLen    /* cx now contains offset of last char in buf+1  */
  asm    dec  cx            /* now it contains offset of last char in buf    */
  asm    add  di,dx         /* es:di points to offset of first pos to search */
  asm    mov  dh, [si]      /* dh holds  srch[len]                           */
  #ifdef LDATA
  #ifdef CPU386
  asm    lfs  bx, Table     /* ds:bx points to table                         */
  asm    mov  bp,bx         /* save bx pointer to table                      */
  asm    mov  bx, seg UpperCase
  asm    mov  ds, bx
  asm    mov  bx, offset UpperCase /* ds:bx points to UpperCase              */
  #else
  asm    lds  bx, Table     /* ds:bx points to table                         */
  asm    mov  [SaveBx],bx   /* save bx pointer to table                      */
  asm    mov  [TableDs],ds  /* save ds pointer to table                      */
  asm    mov  bx, seg UpperCase
  asm    mov  ds, bx
  asm    mov  [UpCaseDs],bx /* save ds pointer to UpperCase                  */
  asm    mov  bx, offset UpperCase /* ds:bx points to UpperCase              */
  #endif
  #else
  asm    mov  bx, Table     /* ds:bx points to table                         */
  asm    mov  [SaveBx],bx   /* save bx pointer to table                      */
  asm    mov  bx, offset UpperCase /* ds:bx points to UpperCase              */
  #endif

  asm    jmp  short comp    /* skip table and start comparing                */

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
/*   At this point the register usage can be summarized as follows:         */
/*   AH    = 0   used to reinitialize registers at times                    */
/*   AL    = character from Buf[]                                           */
/*   CX    = ofset to last character in Buf[]                               */
/*   DH    = last character in Srch[]                                       */
/*   DL    = length of Srch[]                                               */
/*   DS:BX = points to UpperCase[]  NOTE: it is assumed that Table and Srch */
/*   DS:SI = points to Srch[]       reside in the SAME segment!!!           */
/*   ES:DI = points to Buf[]                                                */
/*   SaveBx= points to Table[]   (BX & Save are XCHG'd to access it)        */
/*   SaveDs= points to Table[]   Switched in to access Table or Srch!       */
/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/

nexttable:
  #ifdef LDATA
  #ifndef CPU386
  asm    xchg bx, SaveBx   /* switch in pointer to Table[]          */
  asm    mov  ds, TableDs  /* switch in DS pointer to Table[]       */
  asm    xlat              /* get boyer-moyer jump table distance   */
  asm    mov  ds, UpCaseDs /* switch back to DS ptr to UpperCase[]  */
  asm    xchg bx, SaveBx   /* switch back to pointer to UpperCase[] */
  #else
  asm    xchg bx, bp       /* switch in pointer to Table[]          */
  asm    db 64h
  asm    xlat              /* get boyer-moyer jump table distance   */
  asm    xchg bx, bp       /* switch back to pointer to UpperCase[] */
  #endif
  #else
  asm    xchg bx, SaveBx   /* switch in pointer to Table[]          */
  asm    xlat              /* get boyer-moyer jump table distance   */
  asm    xchg bx, SaveBx   /* switch back to pointer to UpperCase[] */
  #endif
  asm    add  di, ax       /* add jump value to buf */

comp:
  asm    cmp  cx, di       /* compare new offset with offset of last char in buffer */
  asm    jb   notfound     /* if less or equal, compare again else it's not found   */
  asm    mov  al, es:[di]  /* put buf char in al                                    */
  asm    xlat              /* covert AL to uppercase               */

  asm    cmp  dh, al       /* compare srch[len] with buf[]         */
  asm    jne  nexttable    /* if not same, go back and try again   */

  asm    push si
  asm    push di
  asm    push cx
  asm    mov  cl, dl            /* cl = length srch                      */
  asm    mov  ch, ah            /* ch = 0                                */
  asm    jcxz done

comploop:
  asm    dec  si
  asm    dec  di
  asm    mov  al, es:[di]       /* get next character      */
  asm    xlat                   /* convert to uppercase    */

  #ifdef LDATA
  #ifndef CPU386
  asm    mov  ds, TableDs  /* switch in DS pointer to Srch[]  */
  asm    cmp  al,[si]
  asm    mov  ds, UpCaseDs /* switch back to DS ptr to UpperCase[]  */
  #else
  asm    cmp  al,fs:[si]
  #endif
  #else
  asm    cmp  al,[si]
  #endif
  asm    loope comploop
done:
  asm    mov  al, dh
  asm    pop  cx
  asm    pop  di
  asm    pop  si
  asm    jne  nexttable

found:                          /* di points to pos   */
  #ifdef LDATA
  #ifdef CPU386
  asm    pop  bp
  #endif
  asm    les  ax, Buf           /* ax points to buf   */
  #else
  asm    mov  ax, Buf           /* ax points to buf   */
  #endif
  asm    xchg ax, di
  asm    xor  dh, dh
  asm    sub  ax, di
  asm    inc  ax
  asm    jmp  short exit

notfound:
  asm    xor  ax, ax            /* result = 0 */
  #ifdef LDATA
  #ifdef CPU386
  asm    pop  bp
  #endif
  #endif

exit:
  #ifdef LDATA
  asm    pop  ds
  #endif
  return(_AX);

  #endif  /* ifdef __OS2__ */
}


#ifdef TEST
#include <stdio.h>
#include <string.h>

void main(void) {
  char Table[256];
  char Key[40];
  char Text[160];
  int  KeyLen;
  int  TextLen;
  int  Found;
  unsigned X;
  unsigned Y;

  getcountryspecs(0,0);

  strcpy(Text,"This is a test to verify that the Boyer-Moore SEARCH functions "
              "are operating correctly.");

  strcpy(Key,"the boyer-moore search");
  strupr(Key);

  KeyLen = strlen(Key);
  TextLen = strlen(Text);
  maketable(Table,Key,KeyLen);

  for (Y = 1; Y < 5; Y++)
    for (X = 0; X < 64000U; X++)
      Found = bmisearch(Text,TextLen,Table,Key,KeyLen);

  if (Found != 0)
    puts(&Text[Found-strlen(Key)]);
}
#endif
