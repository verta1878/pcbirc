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


#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <screen.h>
#include "scrnio.h"
#include "scrnio.ext"

#ifdef DEBUG
#include <memcheck.h>
#endif

#define MENU_TOP        8
#define MENU_LEFT      23
#define MENU_LEN       35
#define BIG_MENU_LEFT   7
#define BIG_MENU_LEN   22

typedef struct {
  int      Len;
  int      Left;
  int      Adjust;
  int      Total;
  int      Select;
  char     Ch;
  char     Ext;
  struct MenuType *ptr;
} menuvaluetype;


menuvaluetype static MenuValues;

unsigned  GroupOffset;
unsigned  MenuSelection;
unsigned  NumMenuItems   = 0;
char     *MainHead1;
char     *MainHead2;

int (LIBENTRY *promptusernum)(int Ch) = NULL;
int (LIBENTRY *promptuserstr)(int Ch) = NULL;

/********************************************************************
*
*  Function:  showselect()
*
*  Called by menusel() and adjustsel() to display menu selection on screen
*/

void _NEAR_ LIBENTRY showselect(char Color) {
  int Xpos,Ypos;

  Xpos = ((MenuValues.Select % MenuValues.Adjust) * MenuValues.Len) + MenuValues.Left;
  Ypos =  (MenuValues.Select / MenuValues.Adjust) + MENU_TOP;
  setatt(Xpos,Ypos,Xpos+MenuValues.Len-1,Ypos,Color);
  gotoxy(Xpos,Ypos);
}


/********************************************************************
*
*  Function:  adjustsel()
*
*  Called by menusel() to adjust the current highlighted selection on screen
*  according to the key that was pressed.
*
*  Returns -1 if the Select value went out of range on a BIGMENU otherwise 0
*/

int _NEAR_ LIBENTRY adjustsel(int HelpNum) {
  int NewSelect;
  int Old;

  Old = MenuValues.Select + MenuValues.ptr->First;

  NewSelect = MenuValues.Select;
  if (MenuValues.Ch == 13 || MenuValues.Ch == 27 || (! MenuValues.Ext))
    return(0);

  switch(MenuValues.Ch) {
    case 59: if (HelpFile != 0) {
               if (MenuValues.Adjust == 3)  /* is it a BIG menu? */
                 showhelp(HelpNum);
               else
                 showhelp(HelpNum + NewSelect);
             }
             break;
    case 71: NewSelect = 0; break;
    case 72: NewSelect -= MenuValues.Adjust; break;
    case 73: NewSelect -= MenuValues.Total;  break;
    case 75: if (NewSelect >= 0)  NewSelect--; else NewSelect = MenuValues.Total; break;
    case 77: if (NewSelect < MenuValues.Total)  NewSelect++; else NewSelect = 0; break;
    case 79: NewSelect = MenuValues.Total-1; break;
    case 80: NewSelect += MenuValues.Adjust; break;
    case 81: NewSelect += MenuValues.Total;  break;
    case 82: break;
    default: beep();
  }

  if (MenuValues.Ch != 59)
    showselect(MenuAvail[Old] ? Colors[MENUSELECT] : Colors[MENUUNAVAIL]);

  if (NewSelect < 0) {
    if (MenuValues.Adjust != 1)
      return(-1);
    NewSelect = MenuValues.Total - 1;
  }
  if (NewSelect > MenuValues.Total - 1) {
    if (MenuValues.Adjust != 1)
      return(-1);
    NewSelect = 0;
  }
  MenuValues.Select = NewSelect;
  return(0);
}


/********************************************************************
*
*  Function:  initmenu()
*
*  Initializes each menu storing the Help Number and Text Heading
*/

void LIBENTRY initmenu(int MenuNum,bool ForwBack,char Text[],char L[],char R[]) {
  struct MenuType *p;

  p = &Menu[MenuNum];

  p->ForwBack = ForwBack;
  p->First = p->Last = 0;
  strcpy(p->Head    , Text);
  strcpy(p->LeftStr , L);
  strcpy(p->RightStr, R);
}

/********************************************************************
*
*  Function:  addmenu()
*
*  Adds Text for each menu item and keeps track of pointers to the first
*  and last items within MenuList[] that belong to each menu.
*/

void LIBENTRY addmenu(int MenuNum, char MenuItem[], void LIBENTRY (*f)(void)) {
 struct MenuType *p;

  p = &Menu[MenuNum];

  if (p->First == 0 && MenuNum != 0)
    p->First = (char) NumMenuItems;
  p->Last = (char) NumMenuItems;
  strcpy(MenuList[NumMenuItems],MenuItem);
  MenuAvail[NumMenuItems] = TRUE;
  MenuFunc[NumMenuItems] = f;
  NumMenuItems++;
}


/********************************************************************
*
*  Function:  makemenuscreen()
*
*  Called by menusel() to create the menu selection screen.
*/


void _NEAR_ LIBENTRY makemenuscreen(void) {
  char LineStr[69];
  char Temp[50];
  int  X;
  int  Xpos;
  int  Ypos;
  char *p;
  char *r;

  p = MenuList[MenuValues.ptr->First];
  r = &MenuAvail[MenuValues.ptr->First];

  memset(LineStr,'Ä',68);
  LineStr[68] = 0;
  clscolor(Colors[OUTBOX]);
  generalscreen(MainHead1,MainHead2);
  if ((Colors[MENUBOX] & 0xF0) == (Colors[OUTBOX] & 0xF0))
    box(5,4,74,23,Colors[MENUBOX],VDOUBLE);
  else
    boxcls(5,4,74,23,Colors[MENUBOX],VDOUBLE);
  fastcenter(5,MenuValues.ptr->Head,Colors[MENUTITLE]);
  if (MenuValues.ptr->LeftStr[0])
    fastprint(6,5,MenuValues.ptr->LeftStr,Colors[MENUSELECT]);
  if (MenuValues.ptr->RightStr[0])
    fastprint(74-strlen(MenuValues.ptr->RightStr),5,MenuValues.ptr->RightStr,Colors[MENUSELECT]);

  switch (MenuValues.Adjust) {
    case 1: Temp[0] = 'A'; Temp[1] = Temp[2] = ' '; break;
    case 3: Xpos = BIG_MENU_LEFT; break;
  }


  MenuValues.Total = MenuValues.ptr->Last - MenuValues.ptr->First + 1;
  for (Ypos = MENU_TOP, X = 0; X < MenuValues.Total; X++, p+=36, r++) {
    switch (MenuValues.Adjust) {
      case 1: strcpy(&Temp[3],p);
              fastprint(MENU_LEFT-2,Ypos,Temp,(*r ? Colors[MENUSELECT] : Colors[MENUUNAVAIL]));
              Ypos++;
              Temp[0]++;
              break;
      case 3: fastprint(Xpos,Ypos,p,(*r ? Colors[MENUSELECT] : Colors[MENUUNAVAIL]));
              Xpos += 23;
              if (Xpos > 69) {
                Xpos = BIG_MENU_LEFT;
                Ypos++;
              }
              break;
    }
  }

  fastprint(6,6,LineStr,Colors[MENUBOX]);
  fastprintmove(6,23,"   Use arrow keys to move bar, press ENTER to select, ESC to exit   ",Colors[MENUDESC]);
}


/********************************************************************
*
*  Function:  menusel()
*
*  Allows the user to choose an item from the menu by pressing arrow keys or
*  alphabet keys.  Returns to the caller the number of the item selected
*  starting with base 1.
*/

void LIBENTRY menusel(int MenuNum, int HelpNum, menusizetype MenuSize, int StartNum, bool AllowJump) {
  menuvaluetype Backup;
  int  OldSelect;
  int  NewSelect;
  int  Offset;

  Backup = MenuValues;

  MenuValues.ptr = &Menu[MenuNum];
  Offset = Menu[MenuNum].First;

  switch (MenuSize) {
    case SMALL: MenuValues.Len    = MENU_LEN;
                MenuValues.Left   = MENU_LEFT;
                MenuValues.Adjust = 1;
                break;
    case BIG  : MenuValues.Len    = BIG_MENU_LEN;
                MenuValues.Left   = BIG_MENU_LEFT;
                MenuValues.Adjust = 3;
                break;
  }

  MenuValues.Select = StartNum;
  if (KeyFlags != PGUP && KeyFlags != PGDN)
    makemenuscreen();

  do {
    switch (KeyFlags) {
      case PGUP : MenuValues.Select = MenuValues.ptr->Last-MenuValues.ptr->First;
      case PGDN : MenuValues.Ch = 13;
                  KeyFlags = 0;
                  break;
      default   : setcursor(CUR_BLANK);
                  showselect(MenuAvail[MenuValues.Select+Offset] ? Colors[MENUBAR] : Colors[MENUUNBAR]);
                  KeyFlags = 0;
                  MenuValues.Ch = inkey(&MenuValues.Ext,ShowClock);
                  if (! MenuValues.Ext)
                    MenuValues.Ch = (char) toupper(MenuValues.Ch);

                  if (! MenuValues.Ext) {
                    if (AllowJump) {
                      switch (MenuSize) {
                        case BIG  : if (MenuValues.Ch >= 48 && MenuValues.Ch <= 57) {
                                      if (promptusernum != NULL && (NewSelect = promptusernum(MenuValues.Ch)) != 0) {
                                        showselect(MenuAvail[MenuValues.Select+Offset] ? Colors[MENUSELECT] : Colors[MENUUNAVAIL]);
                                        MenuValues.Select = NewSelect-1;
                                        /* showselect(MenuAvail[MenuValues.Select+Offset] ? Colors[MENUBAR] : Colors[MENUUNBAR]); */
                                        /* MenuValues.Ch = 13; */
                                        KeyFlags = KeyFlags = RET;
                                        MenuSelection = MenuValues.Select;
                                        goto exit;
                                      }
                                    } else if (MenuValues.Ch != 13) {
                                      if (promptuserstr != NULL && (NewSelect = promptuserstr(MenuValues.Ch)) != 0) {
                                        showselect(MenuAvail[MenuValues.Select+Offset] ? Colors[MENUSELECT] : Colors[MENUUNAVAIL]);
                                        MenuValues.Select = NewSelect-1;
                                        /* showselect(MenuAvail[MenuValues.Select+Offset] ? Colors[MENUBAR] : Colors[MENUUNBAR]); */
                                        /* MenuValues.Ch = 13; */
                                        KeyFlags = KeyFlags = RET;
                                        MenuSelection = MenuValues.Select;
                                        goto exit;
                                      }
                                    }
                                    break;
                        case SMALL: if (MenuValues.Ch >= 65 && MenuValues.Ch <= MenuValues.Total+64) {
                                      showselect(MenuAvail[MenuValues.Select+Offset] ? Colors[MENUSELECT] : Colors[MENUUNAVAIL]);
                                      MenuValues.Select = MenuValues.Ch - 65;
                                      showselect(MenuAvail[MenuValues.Select+Offset] ? Colors[MENUBAR] : Colors[MENUUNBAR]);
                                      MenuValues.Ch = 13;
                                    }
                                    break;
                      }
                    }
                  } else
                    if (adjustsel(HelpNum) == -1) {
                      switch (MenuValues.Ch) {
                        case 72: KeyFlags = UP;   break;
                        case 73: KeyFlags = PGUP; break;
                        case 80: KeyFlags = DN;   break;
                        case 81: KeyFlags = PGDN; break;
                      }
                      MenuSelection = MenuValues.Select;
                      goto exit;
                    }
                  break;
    }

    if (MenuValues.Ch == 13) {
      if (MenuAvail[MenuValues.Select+Offset]) {
        if (MenuValues.ptr->ForwBack) {
          while (KeyFlags != ESC) {
            MenuSelection = MenuValues.Select;
            MenuFunc[MenuValues.ptr->First+MenuValues.Select]();
            OldSelect = MenuValues.Select;
            switch (KeyFlags) {
              case PGUP:  MenuValues.Select--;
                          if (MenuValues.Select < 0) {
                            /* MenuValues.Select = -1; */
                            goto exit;
                          }
                          break;
              case PGDN:  MenuValues.Select++;
                          if (MenuValues.Select > MenuValues.ptr->Last-MenuValues.ptr->First)
                            goto exit;
                          break;
            }
            if (! MenuAvail[MenuValues.ptr->First+MenuValues.Select]) {
              MenuValues.Select = OldSelect;
              KeyFlags = ESC;
            }
          }
        } else {
          MenuSelection = MenuValues.Select;
          MenuFunc[MenuValues.ptr->First+MenuValues.Select]();
        }
      } else beep();
      makemenuscreen();
    }
  } while (MenuValues.Ch != 27);

 if (MenuValues.Ch == 27)
   KeyFlags = ESC;

exit:
  setcursor(CUR_NORMAL);
  showkeystatus();
  MenuValues = Backup;
}


void LIBENTRY bigmenusel(int MenuNum, int HelpNum, int StartNum, void LIBENTRY (*func)(void), int LIBENTRY (*bounds)(unsigned Num, unsigned *Offset), bool AllowJump) {
  menuvaluetype Backup;
  unsigned      NewSelect;
  unsigned      Offset;
  unsigned      NumEntries;
  int           Change;

  GroupOffset = 0;
  MenuSelection = StartNum;

  if (KeyFlags == PGDN)
    StartNum--;

  Backup = MenuValues;

  MenuValues.ptr = &Menu[MenuNum];
  Offset = Menu[MenuNum].First;
  NumEntries = MenuValues.ptr->Last - MenuValues.ptr->First + 1;

  MenuValues.Len    = BIG_MENU_LEN;
  MenuValues.Left   = BIG_MENU_LEFT;
  MenuValues.Adjust = 3;
  MenuValues.Select = StartNum;

top:
  while (1) {
    switch (KeyFlags) {
      case PGUP :
      case PGDN : while (1) {
                    switch (KeyFlags) {
                      case PGUP:  MenuValues.Select--;
                                  MenuSelection = MenuValues.Select + GroupOffset;
                                  if (MenuValues.Select < 0) {
                                    if (bounds(MenuSelection,&GroupOffset) == -1) {
                                      KeyFlags = NOTHING;
                                      goto exit;
                                    }
                                    MenuValues.Select = MenuSelection - GroupOffset;
                                  }
                                  break;
                      case PGDN:  MenuValues.Select++;
                                  MenuSelection = MenuValues.Select + GroupOffset;
                                  if (MenuValues.Select > MenuValues.ptr->Last-MenuValues.ptr->First) {
                                    if (bounds(MenuSelection,&GroupOffset) == -1) {
                                      KeyFlags = NOTHING;
                                      goto exit;
                                    }
                                    MenuValues.Select = MenuSelection - GroupOffset;
                                  }
                                  break;
                    }
                    func();
                    if (KeyFlags == ESC)
                      goto top;
                  }
                  break;
      default   : setcursor(CUR_BLANK);
                  if (bounds(MenuSelection,&GroupOffset) == -1) {
                    KeyFlags = NOTHING;
                    goto exit;
                  }
                  MenuValues.Select = MenuSelection - GroupOffset;
                  makemenuscreen();
                  while (1) {
                    showselect(MenuAvail[MenuValues.Select+Offset] ? Colors[MENUBAR] : Colors[MENUUNBAR]);
                    KeyFlags = 0;
                    MenuValues.Ch = inkey(&MenuValues.Ext,ShowClock);
                    if (! MenuValues.Ext) {
                      if (MenuValues.Ch >= 48 && MenuValues.Ch <= 57) {
                        if (AllowJump) {
                          if (promptusernum != NULL && (NewSelect = promptusernum(MenuValues.Ch)) != 0) {
                            MenuSelection = NewSelect - 1;
                            if (bounds(MenuSelection,&GroupOffset) == -1) {
                              KeyFlags = NOTHING;
                              goto exit;
                            }
                            MenuValues.Select = MenuSelection - GroupOffset;
                            func();
                            if (KeyFlags == ESC || KeyFlags == PGUP || KeyFlags == PGDN)
                              goto top;
                          }
                        }
                      } else if (MenuValues.Ch == 13) {
                        MenuSelection = MenuValues.Select + GroupOffset;
                        func();
                        goto top;
                      } else if (MenuValues.Ch == 27) {
                        KeyFlags = ESC;
                        goto exit;
                      } else {
                        if (AllowJump) {
                          if (promptuserstr != NULL && (NewSelect = promptuserstr(MenuValues.Ch)) != 0) {
                            MenuSelection = NewSelect - 1;
                            if (bounds(MenuSelection,&GroupOffset) == -1) {
                              KeyFlags = NOTHING;
                              goto exit;
                            }
                            MenuValues.Select = MenuSelection - GroupOffset;
                            func();
                            if (KeyFlags == ESC || KeyFlags == PGUP || KeyFlags == PGDN)
                              goto top;
                          }
                        }
                      }
                    } else {
                      if (adjustsel(HelpNum) == -1) {
                        MenuSelection = MenuValues.Select + GroupOffset;
                        switch (MenuValues.Ch) {
                          case 72: KeyFlags = UP;   Change = -3;          break;
                          case 73: KeyFlags = PGUP; Change = -NumEntries; break;
                          case 80: KeyFlags = DN;   Change = +3;          break;
                          case 81: KeyFlags = PGDN; Change = +NumEntries; break;
                          default: Change = 0; break;
                        }
                        if (Change < 0 && MenuSelection < -Change) {
                          KeyFlags = NOTHING;
                          goto exit;
                        }
                        MenuSelection += Change;
                        if (bounds(MenuSelection,&GroupOffset) == -1) {
                          KeyFlags = NOTHING;
                          goto exit;
                        }
                        MenuValues.Select = MenuSelection - GroupOffset;
                        KeyFlags = NOTHING;
                        goto top;
                      }
                    }
                  }
                  break;
    }
  }

exit:
  setcursor(CUR_NORMAL);
  showkeystatus();
  MenuValues = Backup;
}
