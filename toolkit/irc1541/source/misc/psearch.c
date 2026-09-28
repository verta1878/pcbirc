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


/* NOTE:   define TEST to see a test program  */
/*         define TEST2 to watch it in action */

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <malloc.h>
#else
  #include <alloc.h>
#endif

#include <stdio.h>
#include <string.h>
#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

enum { TOK_NONE, TOK_WORD, TOK_LPAREN, TOK_RPAREN, TOK_AND, TOK_OR, TOK_NOT, TOK_END };
char static *tokens = "..()&|!";

#ifdef TEST2
char static *msg[8] = {"none","word","lparen","rparen","and","or","not","end"};
#endif

static psearchtype *Search;
static int NumSearches = 0;
static int LIBENTRY (*pcb_bsearch)(char *Buf, int BufLen, char *Table, char *Srch, char SrchLen);
static int NumTokens;

psearchtype * LIBENTRY getparsersearcharray(void) {
  return(Search);
}

int LIBENTRY getparsersearchcount(void) {
  return(NumSearches);
}

void LIBENTRY setparsersearch(int Num, psearchtype *Array) {
  Search = Array;
  NumSearches = Num;
}

void LIBENTRY resetparsersearch(void) {
  NumSearches = 0;
  Search = NULL;
}


/********************************************************************
*
*  Function:  gettoken()
*
*  Desc    :  The first time gettoken() is called *Str points to the string
*             to be scanned - thereafter it should be passed a NULL pointer.
*
*             It then scans the string for each word/phrase as well as any
*             punctuation such as (, ), & or | (note: &=and, |=or).
*
*  Returns :  If it finds punctuation then it simply returns with the keyword
*             for the punctuation found.
*
*             If it finds a word/phrase then it copies it into *Token.
*/

static int _NEAR_ LIBENTRY gettoken(char *Str, char *Token) {
  char static *p;
  char *q;
  bool Started;
  bool InQuotes;

  if (Str != NULL)
    p = Str;
  else
    p++;

  Started  = FALSE;
  InQuotes = FALSE;

  while (1) {
    if (InQuotes) {
      switch (*p) {
        case  0 : return(TOK_END);
        case '"': InQuotes = FALSE;
                  *Token = 0;
                  return(TOK_WORD);
        default : *Token = *p;
                  Token++;
                  break;
      }
    } else {
      switch (*p) {
        case  0 : return(TOK_END);
/*      case '(': return(TOK_LPAREN); */                   /* it used to just return TOK_LPAREN */
        case '(': for (q = p+1; *q != ')'; ) {             /* the following code, however, now  */
                    if (*q == '&' || *q == '|' || *q == 0) /* checks to see if the text is of   */
                      return(TOK_LPAREN);                  /* the form "(xxxx)" with no & or |  */
                    q++;                                   /* and if so it just copies it into  */
                  }                                        /* the search text, up to and        */
                  for (; p < q; ) {                        /* including the right paren         */
                    *Token = *p;
                    p++;
                    Token++;
                  }
                  goto lookahead;
        case ')': return(TOK_RPAREN);
        case '!': return(TOK_NOT);
        case '&': return(TOK_AND);
        case '|': return(TOK_OR);
        case '"': InQuotes = TRUE;
        case ' ': if (! Started)                  /* skip leading spaces */
                    break;                        /* otherwise fall thru */
lookahead:
        default : Started = TRUE;
                  *Token = *p;
                  Token++;
                  switch (*(p+1)) {               /* look ahead */
                    case  0 :
                    case '(':
                    case ')':
                    case '!':
                    case '&':
                    case '|': *Token = 0;
                              for (Token--; *Token == ' '; Token--)  /* strip trailing spaces */
                                *Token = 0;
                              return(TOK_WORD);
                  }
                  break;
      }
    }
    p++;
  }
//return(TOK_END);  /* keep the compiler happy */
}


/********************************************************************
*
*  Function:  tokenscan()
*
*  Desc    :  Scans the input string to tokenize the words/phrases together
*             with the punctuation.   When it is done each word/phrase will
*             consist of a single byte (range 1 to 30) while the punctuation
*             continues unmodified as single byte characters.
*
*             For each word/phrase that is found tokenscan() allocates memory
*             for a Boyer/Moore search table and initializes it.
*
*             If it doesn't find any words/phrases then it frees the memory
*             that has already been allocated.
*
*  Notes   :  call tokenscan() with *Input equal to the text that the user
*             typed in - which then creates *CodedInput which will later be
*             used by parsersearch().
*
*  Returns :  The number of keywords that it found or -1 if a punctuation
*             error is found.
*/

int LIBENTRY tokenscan(char *Input, char *CodedInput, bool CaseSensitive) {
  int  Token;
  int  NumWords;
  int  LastToken;
  int  Depth;
  char *p;
  psearchtype *Temp;
  char Key[80];

  Depth     = 0;
  NumTokens = 0;
  NumWords  = 0;
  LastToken = TOK_NONE;
  p         = CodedInput;
  Search    = (psearchtype *) malloc(sizeof(psearchtype));

  if (Search == NULL)
    return(0);

  pcb_bsearch = (CaseSensitive ? bmsearch : bmisearch);

  while ((Token = gettoken(Input,Key)) != TOK_END) {
    Input = NULL;
    switch (Token) {
      case TOK_WORD  : if (LastToken == TOK_RPAREN) {
                         #ifdef TEST2
                           printf(" punctuation error");
                         #endif
                         NumWords = -1;
                         goto done;
                       }
                       NumWords++;
                       if ((Temp = (psearchtype *)realloc(Search,NumWords * sizeof(psearchtype))) == NULL) {
                         free(Search);
                         return(0);
                       }
                       Search = Temp;
                       strupr(Key);
                       Search[NumWords-1].KeyLen = (char) strlen(Key);
                       memcpy(Search[NumWords-1].Key,Key,Search[NumWords-1].KeyLen+1);
                       maketable(Search[NumWords-1].Table,Key,Search[NumWords-1].KeyLen);
                       *p++ = (char) NumWords;
                       break;
      case TOK_RPAREN: if (LastToken == TOK_AND || LastToken == TOK_OR || LastToken == TOK_NOT) {
                         #ifdef TEST2
                           printf(" punctuation error");
                         #endif
                         NumWords = -1;
                         goto done;
                       }
                       Depth--;
                       *p = tokens[TOK_RPAREN];
                       p++;
                       NumTokens++;
                       break;

      case TOK_LPAREN: if (LastToken == TOK_WORD) {
                         #ifdef TEST2
                           printf(" punctuation error");
                         #endif
                         NumWords = -1;
                         goto done;
                       }
                       Depth++;   /* add one to the depth of parentheses */
                       *p = tokens[TOK_LPAREN];
                       p++;
                       NumTokens++;
                       break;

      default        : *p = tokens[Token];
                       p++;
                       NumTokens++;
                       break;
    }
    LastToken = Token;
  }

done:
  *p = 0;

  if (Depth != 0) {
    NumWords = -1;
    #ifdef TEST2
      printf(" unbalanced parentheses");
    #endif
  }

  if (NumWords < 1) {
    free(Search);
    NumSearches = 0;
  } else
    NumSearches = NumWords;

  #ifdef TEST2
    printf("\nNum Words = %d\nCommand = [%s]\n",NumWords,CodedInput);
  #endif
  return(NumWords);
}


/********************************************************************
*
*  Function:  parsersearch()
*
*  Desc    :  This function parses set of tokenized text commands which
*             instruct it on how to search (there may be multiple words or
*             phrases to be searched on and they may be combined with
*             punctuation such as &(for AND) and | (for OR) operations as
*             well as left and right parenthesis to prioritize the criteria.
*
*             This function will go recursive if any parenthesis are found!
*
*  NOTE    :  1) To start this function you should first call tokenscan()
*             2) Then call parsersearch() with RetVal = FALSE and LastToken = 0
*             3) When done call stopsearch()
*
*  Returns :  TRUE if *CodedInput was found in *Text, otherwise FALSE
*/

bool LIBENTRY parsersearch(char *Text, int TextLen, char *CodedInput, bool RetVal, int LastToken) {
  int        Token;
  bool       Found;
  bool       InParen;
  bool       Not;
  char       Key[80];
  char       *p;
  psearchtype *Srch;

  Not = FALSE;

  if (NumSearches == 1 && NumTokens == 0) {
    return((bool) (pcb_bsearch(Text,TextLen,Search[0].Table,Search[0].Key,Search[0].KeyLen) != 0));
  } else {
    p       = CodedInput;
    InParen = (bool) (LastToken == TOK_LPAREN);
    while ((Token = gettoken(p,Key)) != TOK_END) {
      p = NULL;
      #ifdef TEST2
        printf("\n[%s]",msg[Token]);
      #endif
      switch (Token) {
        case TOK_NOT   : Not = TRUE;
                         break;
        case TOK_WORD  : Srch = &Search[Key[0]-1];
                         Found = (bool) (pcb_bsearch(Text,TextLen,Srch->Table,Srch->Key,Srch->KeyLen) != 0);
                         #ifdef TEST2
                           printf(" '%s' = %d",Srch->Key,Found);
                         #endif
                         if (Not)
                           Found = (bool) (! Found);
                         Not = FALSE;
                         switch (LastToken) {
                           case TOK_AND   : RetVal &= Found;  break;
                           case TOK_OR    : RetVal |= Found;  break;
                           default        : RetVal =  Found;  break;
                         }
                         break;
        case TOK_OR    : Not = FALSE;
                         if (RetVal)  /* short circuit the comparison */
                           goto exit;
                         break;
        case TOK_AND   : Not = FALSE;
                         if (! RetVal) /* short circuit the comparison */
                           goto exit;
                         break;
        case TOK_LPAREN: RetVal = parsersearch(Text,TextLen,NULL,RetVal,Token);
                         if (Not)
                           RetVal = (bool) (! RetVal);
                         Not = FALSE;
                         Token = TOK_RPAREN;
                         break;
        case TOK_RPAREN: return(RetVal);
      }
      LastToken = Token;
    }

  exit:
    if (InParen)     /* if we're INSIDE parens then find the closing paren */
      do {
        Token = gettoken(p,Key);
      } while (Token != TOK_RPAREN && Token != TOK_END);

    return(RetVal);
  }
}


/********************************************************************
*
*  Function:  stopsearch()
*
*  Desc    :  This function free's the memory that was allocated by the
*             function tokenscan() prior to using parsersearch().
*/

void LIBENTRY stopsearch(void) {
  if (NumSearches != 0 && Search != NULL)
    free(Search);
  NumSearches = 0;
  Search = NULL;
}


bool LIBENTRY insearch(void) {
  return((bool) (Search != NULL));
}



/********************************************************************
*
*  Function:  searchfirst()
*
*  Desc    :  searches for the first occurance (closest to the beginning of
*             the string) of one of the search criteria
*
*  Returns :  the offset into the string of the match or -1 if not found
*/

int LIBENTRY searchfirst(char *Str, int *FoundLen) {
  int          X;
  int          First;
  int          Found;
  int          SaveLen;
  int          Len;
  psearchtype *Srch;

  if (NumSearches == 0 || (Len = strlen(Str)) == 0)
    return(-1);

  /* scan thru each of the search criteria, remembering only the offset */
  /* (stored in First) of the one closed to the beginning of the string */

  for (X = 0, First = 0, Srch = Search; X < NumSearches; X++, Srch++) {
    if ((Found = pcb_bsearch(Str,Len,Srch->Table,Srch->Key,Srch->KeyLen)) != 0) {
      First   = Found;
      SaveLen = Srch->KeyLen;

      /* By adjusting the Len *downward* we prevent subsequent matches */
      /* that are nearer the end, plus we reduce the search time!      */
      /* If we've reached the beginning, then break out now.           */
      if ((Len = First) == 0)
        break;
    }
  }

  if (First == 0)
    return(-1);

  *FoundLen = SaveLen;
  return(First-1);
}



#ifdef TEST
void main(void) {
  char Str[256];
  char Input[80];
  char FileName[80];
  FILE *file;

  printf("enter filename to search: ");
  gets(FileName);

  if ((file = fopen(FileName,"rt")) == NULL)
    return;

  printf("enter search criteria: ");
  gets(Input);

  printf("\nsearching...\n");

  if (tokenscan(Input,Input,FALSE) > 0) {
    while (fgets(Str,sizeof(Str),file) != NULL) {
      if (parsersearch(Str,strlen(Str),Input,FALSE,TOK_NONE))
        printf("%s",Str);
    }
    stopsearch();
  }
  fclose(file);
}
#endif
