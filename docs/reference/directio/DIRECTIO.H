/****************************************************************************\
*                                                                            *
* Listing 7-2                                                                *
*                                                                            *
* DIRECTIO.H -  Defines for DIRECTIO.C                                       *
*               (c) Guy Eddon, 1993                                          *
*                                                                            *
* Comments:     Port of MS-DOS direct screen access to NT.                   *
*                                                                            *
\****************************************************************************/

#define FOREGROUND_YELLOW (WORD)0x0006
#define FOREGROUND_CYAN   (WORD)0x0003
#define BACKGROUND_CYAN   (WORD)0x0030
#define FOREGROUND_WHITE  (WORD)0x0007
#define FOREGROUND_BLUE         0x0001
#define FOREGROUND_GREEN        0x0002
#define FOREGROUND_RED          0x0004
#define FOREGROUND_INTENSITY    0x0008
#define BACKGROUND_BLUE         0x0010
#define BACKGROUND_GREEN        0x0020
#define BACKGROUND_RED          0x0040
#define BACKGROUND_INTENSITY    0x0080

#define SINGLE    0
#define DOUBLE    1
#define MAX_STR 100

extern HANDLE hStdOut;
extern CONSOLE_SCREEN_BUFFER_INFO csbi; /* used to get current attribute */

extern void clearscreen(WORD);
extern void clear(unsigned char x1, unsigned char y1, unsigned char x2,
   unsigned char y2, unsigned char attribute);
extern void read_field(unsigned char x1, unsigned char x2, unsigned char y,
   char *buffer);
extern void mxyputs(unsigned char x, unsigned char y, char *buffer,
   unsigned attribute);
extern void set_vid_mem(void);
extern void mxyputc(unsigned char x, unsigned char y, char ch,
   unsigned char num, unsigned char attr);
extern void moutchar(unsigned char x, unsigned char y, char character,
   unsigned char attribute);
extern void box(unsigned char x1, unsigned char y1, unsigned char x2,
   unsigned char y2, char S_D);
extern char get_character_wait(void);
extern char get_character_no_wait(void);

