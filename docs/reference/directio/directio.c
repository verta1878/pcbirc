/****************************************************************************\
*                                                                            *
* Listing 7-1                                                                *
*                                                                            *
* DIRECTIO.C -  Uses the console API to do video I/O                         *
*               (c) Guy Eddon, 1993                                          *
*                                                                            *
*                                                                            *
* Comments:     Port of MS-DOS direct screen access to NT.                   *
*                                                                            *
\****************************************************************************/

#include <STDIO.H>
#include <STRING.H>
#include <STDLIB.H>
#include <STDARG.H>
#include <LIMITS.H>
#include <WINDOWS.H>
#include "directio.h"

#define WHITE_ON_CYAN FOREGROUND_WHITE|FOREGROUND_INTENSITY|BACKGROUND_CYAN

/* Declaration for Ctrl-C and Ctrl-Break handler */

extern BOOL WINAPI CtrlHandler(DWORD CtrlType);

HANDLE hStdOut;
CONSOLE_SCREEN_BUFFER_INFO csbi;

/* Initialize Screen Access and set Ctrl-C and Ctrl-Break handlesr */

void set_vid_mem(void)
   {
   hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
   GetConsoleScreenBufferInfo(hStdOut, &csbi);
   SetConsoleCtrlHandler(CtrlHandler, TRUE);
   }

/* Display string pointed by str at location (x,y) with attribute attr */

void mxyputs(unsigned char x, unsigned char y, char *str, unsigned  attr)
   {
   COORD BufferCoord;
   DWORD dummy;
   WORD Color[MAX_STR];
   unsigned count;

   BufferCoord.X = x;
   BufferCoord.Y = y;

   WriteConsoleOutputCharacter(hStdOut, str, strlen(str), BufferCoord,
      &dummy);

   for(count = 0; count < strlen(str); count++)
      Color[count] = attr;

   WriteConsoleOutputAttribute(hStdOut, Color, strlen(str), BufferCoord,
      &dummy);
   }

/* Display character ch num times starting at (x,y) with attribute atr */

void mxyputc(unsigned char x, unsigned char y, char ch, unsigned char num,
   unsigned char attr)
   {
   COORD BufferCoord;
   DWORD dummy;

   BufferCoord.X = x;
   BufferCoord.Y = y;

   FillConsoleOutputCharacter(hStdOut, ch, num, BufferCoord, &dummy);
   FillConsoleOutputAttribute(hStdOut, attr, num, BufferCoord, &dummy);
   }

/* Display character ch at location (x,y) with attribute attr   */

void moutchar(unsigned char x, unsigned char y, char ch, unsigned char attr)
   {
   COORD BufferCoord;
   DWORD dummy;

   BufferCoord.X = x;
   BufferCoord.Y = y;

   FillConsoleOutputCharacter(hStdOut, ch, 1, BufferCoord, &dummy);
   }

/*
   Display Box from (x1,y1) to (x2,y2), S_D flag for single
   or double border
*/
void box(unsigned char x1, unsigned char y1, unsigned char x2,
   unsigned char y2, char S_D)
   {
   COORD BufferCoord;
   DWORD dummy;
   unsigned char count;
   char chr;

   for(count = y1 + 1; count < y2; count++)
      {
      BufferCoord.X = x1;
      BufferCoord.Y = count;

      chr = S_D ? 186 : 179;

      FillConsoleOutputCharacter(hStdOut, chr, 1, BufferCoord, &dummy);
      FillConsoleOutputAttribute(hStdOut, WHITE_ON_CYAN, 1, BufferCoord, 
         &dummy);

      BufferCoord.X = x2;

      FillConsoleOutputCharacter(hStdOut, chr, 1, BufferCoord, &dummy);
      FillConsoleOutputAttribute(hStdOut, WHITE_ON_CYAN, 1, BufferCoord, 
         &dummy);
      }

   BufferCoord.X = x1 + 1;
   BufferCoord.Y = y1;

   count = x2 - x1 - 1;
   chr =   S_D ? 205 : 196;

   FillConsoleOutputCharacter(hStdOut, chr, count, BufferCoord, &dummy);
   FillConsoleOutputAttribute(hStdOut, WHITE_ON_CYAN, count+1, BufferCoord,
      &dummy);

   BufferCoord.Y = y2;

   chr = S_D ? 205 : 196;

   FillConsoleOutputCharacter(hStdOut, chr, count, BufferCoord, &dummy);
   FillConsoleOutputAttribute(hStdOut, WHITE_ON_CYAN, count+1, BufferCoord,
      &dummy);

   BufferCoord.X = x1;
   BufferCoord.Y = y1;

   chr = S_D ? 201 : 218;

   FillConsoleOutputCharacter(hStdOut, chr, 1, BufferCoord, &dummy);
   FillConsoleOutputAttribute(hStdOut, WHITE_ON_CYAN, 1, BufferCoord,
      &dummy);

   BufferCoord.Y = y2;

   chr = S_D ? 200 : 192;

   FillConsoleOutputCharacter(hStdOut, chr, 1, BufferCoord, &dummy);
   FillConsoleOutputAttribute(hStdOut, WHITE_ON_CYAN, 1, BufferCoord,
      &dummy);

   BufferCoord.X = x2;
   BufferCoord.Y = y1;

   chr = S_D ? 187 : 191;

   FillConsoleOutputCharacter(hStdOut, chr, 1, BufferCoord, &dummy);
   FillConsoleOutputAttribute(hStdOut, WHITE_ON_CYAN, 1, BufferCoord,
      &dummy);

   BufferCoord.Y = y2;

   chr = S_D ? 188 : 217;

   FillConsoleOutputCharacter(hStdOut, chr, 1, BufferCoord, &dummy);
   FillConsoleOutputAttribute(hStdOut, WHITE_ON_CYAN, 1, BufferCoord,
      &dummy);
   }

/*
   Reads field from (x1,y) to (x2,y), saves value in a variable pointed by
   buffer
*/
void read_field(unsigned char x1, unsigned char x2, unsigned char y,
   char *buffer)
   {
   unsigned char col = x1;
   unsigned char count;

   for(count = x1; count <= x2; count++)
      moutchar(count, y, (char)32, 0);
   for(count = x1; ; count++)
      {
      buffer[count - x1] = toupper(get_character_wait());
      if(buffer[count - x1] == '\r')
         break;
      if(buffer[count - x1] == '\b')
         {
         count--;
         if(col != x1)
            {
            moutchar(--col, y, (char)32, 0);
            count--;
            }
         }
      else
         moutchar(col++, y, buffer[count - x1], 0);
      if(col < x1)
         col = x1;
      else
         if(col > ((unsigned char)(x2 + 1)))
            {
            count--;
            col = x2 + 1;
            moutchar(col, y, (char)32, 0);
            }
      }
   buffer[count - x1] = 0;
   }

/* Clears screen from (x1,y1) to (x2,y2) */

void clear(unsigned char x1, unsigned char y1, unsigned char x2,
   unsigned char y2, unsigned char attr)
   {
   unsigned char row;

   for(row = y1; row <= y2; row++)
      mxyputc(x1, row, (char)32, (unsigned char)((x2-x1)+1), attr);
   }

/* Clears the entire screen   */

void clearscreen(WORD attr)
   {
   COORD BufferCoord;
   DWORD dummy;

   BufferCoord.X = 0;
   BufferCoord.Y = 0;

   FillConsoleOutputCharacter(hStdOut, (char)32, 80*25, BufferCoord, &dummy);
   FillConsoleOutputAttribute(hStdOut, attr, 80*25, BufferCoord, &dummy);

   if(!attr)
      SetConsoleCtrlHandler(CtrlHandler, FALSE);
   }

/* Waits for a character and returns it */

char get_character_wait(void)
   {
   HANDLE hStdIn;     /* standard input         */
   DWORD dwInputMode; /* to save the input mode */
   DWORD dwRead;
   char chBuf;        /* buffer to read into    */

   hStdIn = GetStdHandle(STD_INPUT_HANDLE);
   GetConsoleMode(hStdIn, &dwInputMode);
   SetConsoleMode(hStdIn, dwInputMode & ~ENABLE_LINE_INPUT & 
      ~ENABLE_ECHO_INPUT);
   ReadFile(hStdIn, &chBuf, sizeof(chBuf), &dwRead, NULL);
   SetConsoleMode(hStdIn, dwInputMode);
   return chBuf;
   }

/* If there is a character, returns it. Otherwise, returns */

char get_character_no_wait(void)
   {
   HANDLE hStdIn;     /* standard input         */
   INPUT_RECORD aInputBuffer;
   DWORD dummy;
   DWORD dwInputMode; /* to save the input mode */

   hStdIn = GetStdHandle(STD_INPUT_HANDLE);
   GetConsoleMode(hStdIn, &dwInputMode);
   SetConsoleMode(hStdIn, dwInputMode & ~ENABLE_LINE_INPUT &
      ~ENABLE_ECHO_INPUT);
   PeekConsoleInput(hStdIn, &aInputBuffer, 1, &dummy);
   SetConsoleMode(hStdIn, dwInputMode);
   FlushConsoleInputBuffer(hStdIn);

   if(aInputBuffer.EventType == KEY_EVENT)
      return (char)(aInputBuffer.Event.KeyEvent.wVirtualKeyCode);
   }

BOOL WINAPI CtrlHandler(DWORD CtrlType)
   {
   switch (CtrlType)
      {
      case CTRL_BREAK_EVENT:
      case CTRL_C_EVENT:
         return TRUE;
         break;
      }
   return TRUE;
   }

