/* PCBGLOB.C - runtime globals for PPLC link */
#include "types.hpp"
#include "pcboard.h"

/* Simple globals */
int CtrlBreak = 0;
int ErrorLevel = 0;
unsigned ExtConfLen = 0;
int ForceUpdate = 0;
int LocalOn = 0;
unsigned MsgPtrLen = 0;
char PcbDir[80] = {0};
int UseAnsi = 0;
int UsersFile = 0;
char UsersData[4096] = {0};

/* Status and PcbData — proper types */
statustype Status;
pcbdattype PcbData;
