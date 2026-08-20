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


#define TRUE   1
#define FALSE 0
#include <iostream.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void scanHiAscii(char * str,int strip);

void main(void)
{
    char * s = "\\Õhello »»»… bill\\";
    cout << s << endl;
    scanHiAscii(s,TRUE);
    cout << s << endl;
    scanHiAscii(s,FALSE);
    cout << s << endl;
}




//***********************
// scanHiAscii will either strip and encode high ascii
// or it will decode and expand high asdcii depending on the value
// of the strip parameter.
// if strip is TRUE it will strip, of FALSE it will expand
//***********************

void scanHiAscii(char * str, int strip)
{
char * ptr1;
char * buf;
char   tmpBuf[10];


    buf =  new char[strlen(str)*3];
    if(!buf)
    {
      writelog("Could not allocate high ascii buffer."SPACERIGHT);
      return;
    }
    buf[0] = '\x0';
    tmpBuf[0] = '\x0';
    ptr1 = str;

    switch(strip)
    {

      // Stip and encode high ascii
      case TRUE:
        // If the character is not high ascii simply copy into temporary buffer
        if(*ptr1 <= '~' && *ptr1 != '\\')
        {
          sprintf(tmpBuf,"%c",*ptr1);
          strcpy(buf,tmpBuf);
        }

        // Encode the high ascii and append to buffer
        else
          {
            sprintf(tmpBuf,"\\%x",*ptr1);
            strcat(buf,tmpBuf);
          }
        ptr1++;

        // Now that the first character has been taken care of, loop though the rest
        // of the string doing the same thing.
        while(*ptr1 != '\x0')
        {
            // If it's high, encode and append
            if(!isascii(*ptr1) || *ptr1 == '\\')
              {
                sprintf(tmpBuf,"\\%02x",(unsigned char)*ptr1);
                strcat(buf,tmpBuf);
              }
            // else just append.
            else
            {
              sprintf(tmpBuf,"%c",*ptr1);
              strcat(buf,tmpBuf);
            }

            // Increment pointer.
            ++ptr1;

        }

        break;
      // expand encoded high ascii
      case FALSE:
           while(*ptr1 != '\x0')
           {
             if(*ptr1 == '\\')
             {
                ++ptr1;
                if(*ptr1 == '\\')
                {
                  tmpBuf[0] = '\\';
                  tmpBuf[1] = '\x0';
                  strcat(buf,tmpBuf);
                  continue;
                }

                // Save char after hex value
                tmpBuf[0] = ptr1[2];

                // NUL terminate hex value and temp string
                tmpBuf[2] = ptr1[2] = '\0';

                // Convert hex value to char
                tmpBuf[1] = (char) strtol(ptr1,NULL,16);

                // Restore original char
                ptr1[2] = tmpBuf[0];

                // Concatinate it
                strcat(buf,tmpBuf+1);
                ++ptr1;
             }
             else
             {
                sprintf(tmpBuf,"%c",*ptr1);
                strcat(buf,tmpBuf);
             }
             ++ptr1;
           }
           break;
      default:
           break;
    }
    strcpy(str,buf);
    delete buf;
}

