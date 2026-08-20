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


/****************************************************************************

Given:   X   = number of nodes PLUS ONE (1 extra for the system)
         256 = number of channels

Header:  [CHANNEL FOCUS] [CHANNEL NUMBERS] [WRITE PTRS] [HANDLES] [TOPICS] [r]
            256 bytes          X              2*X         16*X     31*256

         [CHANNEL FOCUS]   each channel has a byte that can be locked for focus
         [CHANNEL NUMBERS] indicates what channel each node is talking on
         [WRITE PTRS]      each node has its own (2-byte int) write pointer
         [HANDLES]         each node has a handle for the user
         [TOPICS]          each channel can have a topic of discussion
         [r]               is sufficient reserved space to round-up
                           (19*X + 32*256) bytes to the nearest 4K for the
                           complete header

          1 -  214 nodes require a header size of 12K
        215 -  430 nodes require a header size of 16K
        431 -  645 nodes require a header size of 20K
        646 -  861 nodes require a header size of 24K
        862 - 1076 nodes require a header size of 28K
       1077 - 1293 nodes require a header size of 32K
       1294 - 1509 nodes require a header size of 36K
       1510 - 1724 nodes require a header size of 40K
       1725 - 1940 nodes require a header size of 44K
       1941 - 2155 nodes require a header size of 48K
       2156 - 2371 nodes require a header size of 52K
       2372 - 2586 nodes require a header size of 56K
       2587 - 2802 nodes require a header size of 60K
       2803 - 3018 nodes require a header size of 64K


Body:    [System] [Node1] [Node2] [...]
            512     512     512    512

         Total file size is therefore:

              19*X + 32*256 + [r] + 512*X

Total:   /2    = 12K + 512*3    =  13824
         /10   = 12K + 512*11   =  17920
         /100  = 12K + 512*101  =  64000
         /250  = 16K + 512*251  = 144896
         /350  = 16K + 512*351  = 196096
         /1000 = 28K + 512*1001 = 541184

*****************************************************************************/

#include "project.h"
#pragma hdrstop

#include <io.h>
// #include <process.h>
#include "account.h"

#ifdef __OS2__
  #include <semafore.hpp>
  #include <threads.h>
#endif

#if PCB_MAXNODES < 214
  #define HEADERSIZE (12*1024)
#elif PCB_MAXNODES < 430
  #define HEADERSIZE (16*1024)
#elif PCB_MAXNODES < 645
  #define HEADERSIZE (20*1024)
#elif PCB_MAXNODES < 861
  #define HEADERSIZE (24*1024)
#elif PCB_MAXNODES < 1076
  #define HEADERSIZE (28*1024)
#elif PCB_MAXNODES < 1293
  #define HEADERSIZE (32*1024L)
#elif PCB_MAXNODES < 1509
  #define HEADERSIZE (36*1024L)
#elif PCB_MAXNODES < 1724
  #define HEADERSIZE (40*1024L)
#elif PCB_MAXNODES < 1940
  #define HEADERSIZE (44*1024L)
#elif PCB_MAXNODES < 2155
  #define HEADERSIZE (48*1024L)
#elif PCB_MAXNODES < 2371
  #define HEADERSIZE (52*1024L)
#elif PCB_MAXNODES < 2586
  #define HEADERSIZE (56*1024L)
#elif PCB_MAXNODES < 2802
  #define HEADERSIZE (60*1024L)
#elif PCB_MAXNODES < 3018
  #define HEADERSIZE (64*1024L)
else
  error - stop the processing
#endif

#define NUMCHANNELS     256
                                          /* node number start with 1, so    */
#define SYSNODE         0                 /* there is 1 more node, sysnode=0 */
#define BUFSIZE         1024

#define FOCUSOFFSET     0
#define FOCUSSIZE       1
#define FOCUSBUFSIZE    (NUMCHANNELS*FOCUSSIZE)

#define CHANNELOFFSET   (FOCUSOFFSET+FOCUSBUFSIZE)
#define CHANNELSIZE     1
#define CHANNELBUFSIZE  (NUMNODES*CHANNELSIZE)

#define PTROFFSET       (CHANNELOFFSET+CHANNELBUFSIZE)
#define PTRSIZE         (sizeof(short))
#define PTRBUFSIZE      (NUMNODES*PTRSIZE)

#define HANDLEOFFSET    (PTROFFSET+PTRBUFSIZE)
#define HANDLESIZE      16
#define HANDLEBUFSIZE   (NUMNODES*HANDLESIZE)

#define TOPICOFFSET     (HANDLEOFFSET+HANDLEBUFSIZE)
#define TOPICSIZE       31
#define TOPICBUFSIZE    (NUMCHANNELS*TOPICSIZE)
#define CHANNELTOPIC    (Topics + OffsetIntoTopic)

#define REALHEADERSIZE  (CHANNELBUFSIZE+PTRBUFSIZE+HANDLEBUFSIZE+TOPICBUFSIZE)
#define CHATFILESIZE    ((long) (HEADERSIZE + (((long) NUMNODES) * ((long) BUFSIZE))))

#define ACCESSTIMER     9
#define KEYTIMER        10
#define LONGDELAY       SIXSECONDS
#define SHORTDELAY      (LONGDELAY - (ONESECOND+HALFSECOND))

typedef enum {PRIVATE, PUBLIC} topicstatus;

#define FORCEREFRESH 0
#define ALLNODES     0

#define NUMCHATOPTIONS 18

wordtype static Options[NUMCHATOPTIONS] = {
 {"BROADCAST"}, {"BYE"},  {"CALL"},    {"CHANNEL"}, {"ECHO"},  {"IGNORE"},
 {"HANDLE"},    {"MENU"}, {"MONITOR"}, {"NOECHO"},  {"QUIT"},  {"PRIVATE"},
 {"PUBLIC"},    {"SEND"}, {"SHOW"},    {"SILENT"},  {"TOPIC"}, {"WHO"}
};

enum {O_BCAST,O_BYE,O_CALL,O_CHANNEL,O_ECHO,O_IGNORE,O_HANDLE,O_MENU,O_MONITOR,O_NOECHO,O_QUIT,O_PRIVATE,O_PUBLIC,O_SEND,O_SHOW,O_SILENT,O_TOPIC,O_WHO};


#pragma pack(1)
typedef struct {
  char  TargetChannel;       //  channel on which to post message (0=refresh)
  uint  OriginNode;          //  node posting the message
  uint  DestinationNode;     //  node to view the message (0=all nodes)
} qhdrtype;
#pragma pack()

static char     *HeaderBuffer;
static char     *Channels;
static short    *WritePtrsOnDisk;
static short    *ReadPtrsInMemory;
static char     *ChatInBuf;
static char     *ChatOutBuf;
static char     *LineBuf;
static char     *LineBufPtr;
static char     *Handles;
static char     *MyHandle;
static char     *Topics;
static uint     NodeNum;
static short    WritePtrInMemory;
static int      CharsInBuffer;
static int      OffsetIntoTopic;
static char     IgnoreNodes[(NUMNODES+7)/8];
static char     MonitorChannels[(NUMCHANNELS+7)/8];
static char     MonitoredChannel;
static bool     Monitor;
static DOSFILE  File;
static int      CapFile;
static char     LastChannel;
static unsigned LastNode;
static bool     FocusOnMe;
static bool     UnFocusedInput;
static bool     Silent;
static bool     Echo;
static bool     PossibleCommand;
static int      SaveX;
static unsigned PrivateSendNode;
static char     BlankLines;
static long     StartTime;


char Channel;
char CalledIntoChannel;

static int  _NEAR_ LIBENTRY getlatest(void);
static void _NEAR_ LIBENTRY update(void);
static int  _NEAR_ LIBENTRY process(char Key);
static void _NEAR_ LIBENTRY chatcleanup(bool Recycling);
static void _NEAR_ LIBENTRY addkeytooutbuf(char Key);
static int  _NEAR_ LIBENTRY getcommand(int Command);



static int _NEAR_ LIBENTRY checkfilesize(void) {
  unsigned  X;
  char      Temp[1024];

  if (dosfseek(&File,0,SEEK_END) != (long) CHATFILESIZE) {
    dosrewind(&File);

    memset(Temp,0,sizeof(Temp));

    /* initialize header */
    for (X = 0; X < HEADERSIZE/sizeof(Temp); X++)
      if (dosfwrite(Temp,sizeof(Temp),&File) == -1)
        return(-1);

    /* initialize node buffers */
    for (X = 0; X < NUMNODES; X++)
      if (dosfwrite(ChatOutBuf,BUFSIZE,&File) == -1)
        return(-1);

    dosflush(&File);
    dosftrunc(&File,CHATFILESIZE); /*lint !e534 truncate if necessary */
  }

  return(0);
}


static int _NEAR_ LIBENTRY addtobuffer(char *Buffer, char C, int Count) {
  Buffer[Count++] = C;
  if (Count == BUFSIZE)
    return(0);
  return(Count);
}


static void _NEAR_ LIBENTRY writecapfile(char ChNum, char *Str) {
  char Time[6];
  char Date[9];
  char FileName[66];
  char Buf[256];

  #ifdef DEBUG
    mc_register(Buf,sizeof(Buf));
  #endif

  if (PcbData.RecordGroupChat) {
    if (ChNum != LastChannel) {
      LastChannel = ChNum;
      if (CapFile > 0) {
        dosclose(CapFile);
        CapFile = 0;
      }

      if (ChNum == 0) {
        #ifdef DEBUG
          mc_unregister(Buf);
        #endif
        return;
      }

      sprintf(FileName,"%s%d.CAP",PcbData.ChtLoc,ChNum);
      if (fileexist(FileName) == 255)
        CapFile = doscreatecheck(FileName,OPEN_RDWR|OPEN_DENYNONE,OPEN_NORMAL);
      else
        CapFile = dosopencheck(FileName,OPEN_RDWR|OPEN_DENYNONE);
    }

    if (CapFile > 0) {
      sprintf(Buf,"%s %s %5u %-25.25s: %s\r\n",datestr(Date),timestr2(Time),PcbData.NodeNum,UsersData.Name,Str);
      if (doslockcheck(CapFile,0,1) != -1) {
        doslseek(CapFile,0,SEEK_END);
        writecheck(CapFile,Buf,strlen(Buf));   //lint !e534
        if (PcbData.Slaves)
          doscommit(CapFile);
        unlock(CapFile,0,1);
      }
    }
  }

  #ifdef DEBUG
    mc_unregister(Buf);
  #endif
}


// try to lock the range, but without using doslockcheck().  we want to
// repeat the attempt like doslockcheck() does, but we don't want the
// error messages that doslockcheck() generates.

static int _NEAR_ LIBENTRY trytolock(int Handle, long Offset, int Len) {
  int X;

  for (X = 0; X < 4; X++) {
    if (lock(Handle,Offset,Len) == 0)
      return(0);
    giveup();
  }
  return(-1);
}


static void _NEAR_ LIBENTRY postmessageinqueue(char Target, uint DestNode, char *Str) {
  short    Ptr;
  unsigned Len;
  char     *p;
  long     FocusSemaphoreOffset;
  long     NodeBufferOffset;
  long     NodePtrOffset;
  qhdrtype Header;
  char     Buf[BUFSIZE];

  #ifdef DEBUG
    mc_register(Buf,sizeof(Buf));
  #endif

  writecapfile(Target,Str);

  Header.TargetChannel   = Target;
  Header.OriginNode      = NodeNum;
  Header.DestinationNode = DestNode;

  NodePtrOffset        = (SYSNODE * PTRSIZE) + PTROFFSET;
  FocusSemaphoreOffset = (SYSNODE * FOCUSSIZE) + FOCUSOFFSET;
  NodeBufferOffset     = ((long) SYSNODE * BUFSIZE) + HEADERSIZE;

  doslseek(File.handle,NodePtrOffset,SEEK_SET);
  if (trytolock(File.handle,FocusSemaphoreOffset,1) == 0) {

    // get the starting pointer (points into the buffer) to determine where
    // the new information should be added
    readcheck(File.handle,&Ptr,PTRSIZE);    //lint !e534

    // read the original buffer into memory, we will the modify the buffer
    // and then write it back out
    doslseek(File.handle,NodeBufferOffset,SEEK_SET);
    readcheck(File.handle,Buf,BUFSIZE);     //lint !e534

    // add the Target Channel, Origin Node and Destination Node information
    // into the buffer just 1 character at a time
    for (Len = 0, p = (char *) &Header; Len < sizeof(Header); Len++, p++)
      Ptr = addtobuffer(Buf,*p,Ptr);

    // now add the Str into the buffer
    for (Len = strlen(Str)+1; Len > 0; Str++, Len--)
      Ptr = addtobuffer(Buf,*Str,Ptr);

    // now write the updated buffer back out to disk
    doslseek(File.handle,NodeBufferOffset,SEEK_SET);
    writecheck(File.handle,Buf,BUFSIZE);    //lint !e534

    // and update the Ptr so that the next node to read the information will
    // know what was updated
    doslseek(File.handle,NodePtrOffset,SEEK_SET);
    writecheck(File.handle,&Ptr,PTRSIZE);   //lint !e534

    unlock(File.handle,FocusSemaphoreOffset,1);
    WritePtrsOnDisk[SYSNODE] = Ptr;
  }

  #ifdef DEBUG
    mc_unregister(Buf);
  #endif
}


static int _NEAR_ LIBENTRY getfocus(void) {
  if (FocusOnMe)
    return(0);

  if (lock(File.handle,Channel+FOCUSOFFSET,1) == 0) {
    FocusOnMe = TRUE;
    return(0);
  }

  return(-1);
}


static void _NEAR_ LIBENTRY releasefocus(void) {
  if (FocusOnMe) {
    update();
    unlock(File.handle,Channel+FOCUSOFFSET,1);
    FocusOnMe = FALSE;
  }
}




static void _NEAR_ LIBENTRY setchannel(void) {
  doslseek(File.handle,(NodeNum * CHANNELSIZE) + CHANNELOFFSET,SEEK_SET);
  writecheck(File.handle,&Channel,CHANNELSIZE);  //lint !e534
}


static void _NEAR_ LIBENTRY sethandle(void) {
  doslseek(File.handle,((unsigned long) NodeNum * HANDLESIZE) + HANDLEOFFSET,SEEK_SET);
  writecheck(File.handle,MyHandle,HANDLESIZE);   //lint !e534
}


static void _NEAR_ LIBENTRY settopic(void) {
  doslseek(File.handle,OffsetIntoTopic + TOPICOFFSET,SEEK_SET);
  writecheck(File.handle,CHANNELTOPIC,TOPICSIZE);     //lint !e534
}


static void _NEAR_ LIBENTRY addchartolinebuf(char C) {
  *LineBufPtr = C;   // put a character into the Line Buffer
  LineBufPtr++;      // get ready for the next character
  *LineBufPtr = 0;   // NUL-terminate the buffer for now
}


static void _NEAR_ LIBENTRY resetlinebuffer(void) {
  LineBufPtr = LineBuf;
  *LineBufPtr = 0;
}


static bool _NEAR_ LIBENTRY linebufisempty(void) {
  return((bool) (LineBufPtr == LineBuf));
}


static void _NEAR_ LIBENTRY showstr(uint Node, char *Str) {
  bool RedisplayLine;
  int  UnFocusX;
  char Buf[26];

  startdisplay(FORCENONSTOP);
  RedisplayLine = (bool) (UnFocusedInput && strchr(Str,'\r') != NULL);

  if (Node == NodeNum && ! (UnFocusedInput || PossibleCommand))
    getfocus();                     //lint !e534

  if (Node != LastNode) {
    if (FocusOnMe || Node != NodeNum) {
      if (awherex() != 0)
        newline();
      printcolor(0x07);
      if (Monitor && ! FocusOnMe) {
        sprintf(Buf,"(%d) %s - %d:",Node,Handles + (Node * HANDLESIZE),MonitoredChannel);
      } else
        sprintf(Buf,"(%d) %s:",Node,Handles + (Node * HANDLESIZE));
      println(Buf);
      LastNode = Node;
    }
  }

  if (Node == NodeNum) {
    if (FocusOnMe || PossibleCommand) {
      printcolor(0x0E);
      print(Str);
    } else {
      if (! UnFocusedInput)          // is this the first time unfocused?
        newline();                   //   yes, move down a line
      printcolor(0x06);              // change to the unfocused color
      print(Str);                    // write the information to screen
      UnFocusedInput = TRUE;         // remember that we have data on screen
    }
  } else {
    printcolor(0x02);
    if (UnFocusedInput) {
      UnFocusX = awherex();
      print("[s");                   // save cursor position
      if (RedisplayLine)
        backupcleareol(awherex());    // move cursor to column 1, clear eol
      print("[A");                   // move cursor up one line
      if (SaveX < UnFocusX)
        backup(UnFocusX - SaveX);     // move cursor BACK to SaveX column
      else if (SaveX > UnFocusX)
        forward(SaveX - UnFocusX);    // move cursor FORWARD to SaveX column
    } else if (! PossibleCommand)
      resetlinebuffer();

    print(Str);
    SaveX = awherex();
    if (UnFocusedInput && ! RedisplayLine)
      print("[u");
  }
}


static void _NEAR_ LIBENTRY showchar(uint Node, char C) {
  char Buf[2];

  Buf[0] = C;
  Buf[1] = 0;
  showstr(Node,Buf);
}


static void _NEAR_ LIBENTRY showunfocusedinput(void) {
  char *p;

  showstr(NodeNum,LineBuf);

  if (! UnFocusedInput) {
    for (p = LineBuf; p < LineBufPtr; p++)
      addkeytooutbuf(*p);

    resetlinebuffer();
    settimer(KEYTIMER,LONGDELAY);
  }
}


static void _NEAR_ LIBENTRY writechatbuf(void) {
  long BufOffset;
  long PtrOffset;

  BufOffset = ((unsigned long) NodeNum * BUFSIZE) + HEADERSIZE;
  PtrOffset = ((unsigned long) NodeNum * PTRSIZE) + PTROFFSET;

  if (trytolock(File.handle,PtrOffset,PTRSIZE) != -1) {
//  if (doslockcheck(File.handle,BufOffset,BUFSIZE) == 0) {
      doslseek(File.handle,BufOffset,SEEK_SET);
      writecheck(File.handle,ChatOutBuf,BUFSIZE);            //lint !e534

      doslseek(File.handle,PtrOffset,SEEK_SET);
      writecheck(File.handle,&WritePtrInMemory,PTRSIZE);     //lint !e534

      WritePtrsOnDisk[NodeNum] = WritePtrInMemory;
      CharsInBuffer = 0;
//    unlock(File.handle,BufOffset,BUFSIZE);
//  }
    unlock(File.handle,PtrOffset,PTRSIZE);
  }

  if (PcbData.Slaves)
    doscommit(File.handle);
}


static void _NEAR_ LIBENTRY update(void) {
  if (CharsInBuffer != 0 || UnFocusedInput) {
    if (! FocusOnMe) {
      if (getfocus() == -1)
        return;
      if (UnFocusedInput) {
        backupcleareol(awherex());
        UnFocusedInput = FALSE;   // we're going to release the unfocusedinput
        showunfocusedinput();     // by displaying it right now
      }
    }

    #ifdef __OS2__
      tickdelay(CharsInBuffer * TWOSECONDS / 40);  // scaled to 2 seconds for every 4 characters
    #else
      tickdelay(CharsInBuffer/40);
    #endif
    writechatbuf();
  }
}


static int _NEAR_ LIBENTRY getbuffer(int Node, char *Buf) {
  int  Read;
  int  Write;
  int  Len;

  doslseek(File.handle,(((long) Node * BUFSIZE)) + HEADERSIZE,SEEK_SET);
  readcheck(File.handle,ChatInBuf,BUFSIZE);  //lint !e534

  Read  = ReadPtrsInMemory[Node];
  Write = WritePtrsOnDisk[Node];

  if ((Len = Write - Read) > 0) {
    memcpy(Buf,&ChatInBuf[Read],Len);
  } else {
    Len = BUFSIZE - Read;
    memcpy(Buf,&ChatInBuf[Read],Len);
    memcpy(&Buf[Len],ChatInBuf,Write);
    Len += Write;
  }

  Buf[Len] = 0;

  ReadPtrsInMemory[Node] = Write;
  return(Len);
}


static int _NEAR_ LIBENTRY updatescreen(int Node) {
  int  BytesLeft;
  int  TotalBytes;
  char *p;
  char *q;
  char Buf[BUFSIZE+1];

  BlankLines = 0;
  TotalBytes = BytesLeft = getbuffer(Node,Buf);

  p = Buf;
  if (Buf[0] == '\n') {
    p++;
    BytesLeft--;
  }

  while ((q = strnchr(p,'\r',BytesLeft)) != NULL) {
    *q = 0;
    q++;

    if (*q == '\n')
      q++;

    showstr(Node,p);
    showstr(Node,"\r\n");

    if (UnFocusedInput) {
      newline();
      showunfocusedinput();
    }

    BytesLeft -= (int) (q - p);
    if (BytesLeft <= 0)
      return(TotalBytes);

    p = q;
  }

  p[BytesLeft] = 0;
  showstr(Node,p);
  return(TotalBytes);
}


static void _NEAR_ LIBENTRY refreshmemory(void) {
  char Temp[REALHEADERSIZE];
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  // read in:   Channels
  //            WritePtrsOnDisk
  //            Handles
  //            Topics
  //
  // if the single read request is successful, then copy the information
  // into the live buffer, if any part of the read failed, then ignore the
  // whole thing

  doslseek(File.handle,CHANNELOFFSET,SEEK_SET);
  if (dosread(File.handle,Temp,REALHEADERSIZE POS2ERROR) == REALHEADERSIZE)
    memcpy(HeaderBuffer,Temp,REALHEADERSIZE);
}


static void _NEAR_ LIBENTRY checkmessagequeue(void) {
  int       BufLen;
  int       StrLen;
  qhdrtype  *Header;
  char      *p;
  char      Temp[80];
  char      Buf[BUFSIZE+1];

  BufLen = getbuffer(SYSNODE,Buf);
  p = Buf;

  while (BufLen > 0) {
    Header = (qhdrtype *) p;
    p += sizeof(qhdrtype);

    // process the message only if it did not come from us
    //    if it is to ALL nodes
    //       and it is a REFRESH command
    //          then refresh memory
    //       else not a refresh, is it on our channel or one we are monitoring?
    //          go display it
    //
    //    else is it addressed specifically to us?
    //       yes, it's ours...  are we ignoring this node?
    //          no, not ignoring, go ahead and display it

    if (Header->OriginNode != NodeNum) {
      if (Header->DestinationNode == ALLNODES) {
        if (Header->TargetChannel == FORCEREFRESH)
          refreshmemory();
        else if (Header->TargetChannel == Channel || (Monitor && isset(MonitorChannels,Header->TargetChannel))) {
          printcolor(PCB_WHITE);
          sprintf(Temp,"(%u) ",Header->OriginNode);
          print(Temp);
          println(p);
        }
      } else if (Header->DestinationNode == NodeNum) {
        if (! isset(IgnoreNodes,Header->OriginNode)) {
          printcolor(PCB_WHITE);
          sprintf(Temp,"(%u) %s - ",Header->OriginNode,Handles + (Header->OriginNode * HANDLESIZE));
          print(Temp);
          ascii(Status.DisplayText,Header->OriginNode);
          displaypcbtext(TXT_RCVDPRIVATEMESSAGE,NEWLINE);
          printcolor(0x02);
          println(p);
          PrivateSendNode = Header->OriginNode;
        }
      }
    }

    StrLen = strlen(p) + 1;         // add 1 for the NULL terminator
    p += StrLen;                    // skip over string just processed
    BufLen -= StrLen;               // what's left to process?
    BufLen -= sizeof(qhdrtype);     // don't forget we removed the header too
  }
}


static int _NEAR_ LIBENTRY getlatest(void) {
  int  X;
  int  Y;
  int  Total;
  char Temp[CHANNELBUFSIZE+PTRBUFSIZE];
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  // read the channels and write pointers in a single read
  // if the read is not successful, then return without further processing
  // otherwise, copy what was read into the live HeaderBuffer
  doslseek(File.handle,CHANNELOFFSET,SEEK_SET);
  if (dosread(File.handle,Temp,CHANNELBUFSIZE+PTRBUFSIZE POS2ERROR) != CHANNELBUFSIZE+PTRBUFSIZE)   // changed from readcheck() to dosread() on 10/1/94 to avoid "error reading file" due to lock violation
    return(0);

  memcpy(HeaderBuffer,Temp,CHANNELBUFSIZE+PTRBUFSIZE);

  for (X = 0, Total = 0; X < NUMNODES; X++) {
    if (WritePtrsOnDisk[X] == ReadPtrsInMemory[X] || X == NodeNum)
      continue;

    if (X == SYSNODE) {
      if (awherex() == 0) {
        doslseek(File.handle,HANDLEOFFSET,SEEK_SET);
        if (readcheck(File.handle,Handles,HANDLEBUFSIZE) == HANDLEBUFSIZE) {
          if (Silent)
            ReadPtrsInMemory[SYSNODE] = WritePtrsOnDisk[X];
          else
            checkmessagequeue();
        }
      }
      continue;
    }

    if (isset(IgnoreNodes,X)) {
      ReadPtrsInMemory[X] = WritePtrsOnDisk[X];  /* skip over it */
      continue;
    }

    Y = Channels[X];

/*
    are we monitoring?
      is it a channel we are monitoring?
        is the channel NOT private?
          Yes, we want to process this one

          is it the same node we last saw? or are we at least in column 1?
            Yes, display channel
            No, avoid updating pointers, we *still* want to see this

      no, is it our channel?
        Yes, display it
        No, update pointers so we don't ever see it again
*/

    if (Monitor && isset(MonitorChannels,Y) && *(Topics + (Y * TOPICSIZE)) != 1) {
      if ((X == LastNode || awherex() == 0)) {
        MonitoredChannel = (char) Y;
        Total += updatescreen(X);
      }
    } else if (Channel == Y)
      Total += updatescreen(X);
    else
      ReadPtrsInMemory[X] = WritePtrsOnDisk[X];
  }

  return(Total);
}


static void _NEAR_ LIBENTRY carriagereturn(void) {
  if (UnFocusedInput) {
    bell();
    return;
  }

  getfocus();                 //lint !e534
  if (! FocusOnMe)
    UnFocusedInput = TRUE;

  if (linebufisempty()) {
    if (BlankLines < 3)
      BlankLines++;
    else {
      bell();
      return;
    }
  } else
    BlankLines = 0;

  writecapfile(Channel,LineBuf);

  process(13);                 //lint !e534    move cursor to left
  process(10);                 //lint !e534    then down a line
  resetlinebuffer();
}


static void _NEAR_ LIBENTRY backspace(void) {
  if (linebufisempty())
    return;

  process(8);                  //lint !e534
  process(' ');                //lint !e534
  process(8);                  //lint !e534
  LineBufPtr--;

  if (UnFocusedInput && LineBufPtr == LineBuf) {
    UnFocusedInput = FALSE;
    print("[A");            // move cursor up one line
    forward(SaveX);          // move cursor FORWARD to SaveX column
  }
}


static void _NEAR_ LIBENTRY wrap(void) {
  char *p;
  char Save[BUFSIZE];

  if ((p = strrchr(LineBuf,' ')) == NULL) {
    carriagereturn();
    return;
  }

  strcpy(Save,p+1);
  for (p = Save; *p != 0; p++)
    backspace();

  *LineBufPtr = 0;      // terminate the string
  carriagereturn();     // this resets LineBufPtr back to the beginning

  for (p = Save; *p != 0; p++) {
    process(*p);               //lint !e534
    addchartolinebuf(*p);
  }

  update();
}


static void _NEAR_ LIBENTRY addkeytooutbuf(char Key) {
  CharsInBuffer++;
  ChatOutBuf[WritePtrInMemory] = Key;

  WritePtrInMemory++;
  if (WritePtrInMemory >= BUFSIZE)
    WritePtrInMemory = 0;
}


static int _NEAR_ LIBENTRY process(char Key) {
  int  Num;
  char Str[10];

  if (Key == '/' && awherex() == 0)
    PossibleCommand = TRUE;

  if (Echo) {
    if (awherex() == 78) {
      if (UnFocusedInput) {
        if (Key != 8) {
          bell();
          return(0);
        }
      } else {
        switch(Key) {
          case ' ': carriagereturn();
                    return(TRUE);
          case  8 :
          case 10 :
          case 13 : break;
          default : wrap();
                    break;
        }
      }
    }

    showchar(NodeNum,Key);
  }

  if (Key == 13)
    PossibleCommand = FALSE;
  else if (PossibleCommand && awherex() >= 3) {
    if (awherex() == 3) {
      Str[0] = LineBuf[1];
      Str[1] = Key;
      Str[2] = 0;
      strupr(Str);
      if ((Num = option(Options,Str,NUMCHATOPTIONS)) > 0) {
        PossibleCommand = FALSE;
        resetlinebuffer();
        freshline();
        return(getcommand(Num));
      }
    } else
      PossibleCommand = FALSE;
  }

  if (! (UnFocusedInput || PossibleCommand)) {
    addkeytooutbuf(Key);
    settimer(KEYTIMER,LONGDELAY);
    if (CharsInBuffer > 50)                     // changed from 150 down to 50 on 9-3-94
      update();
  }

  return(TRUE);
}


static int _NEAR_ LIBENTRY gethandle(int NumTokens) {
  char *p;

  if (PcbData.AllowHandles) {
    while (1) {
      if (NumTokens > 0) {
        MyHandle[0] = 0;
        while (NumTokens) {
          p = getnexttoken();
          NumTokens--;
          addtext(MyHandle,p,HANDLESIZE);
        }
      } else {
        inputfield(MyHandle,TXT_HANDLEFORCHAT,HANDLESIZE-1,NEWLINE|LFBEFORE|HIGHASCII|FIELDLEN,NOHELP,mask_alphanum);
      }

      if (*MyHandle == 0)
        return(-1);

      removetokens(MyHandle);
      if (*MyHandle == 0 || (Status.User == USER && readtcanfile(MyHandle)))
        displaypcbtext(TXT_PICKANOTHERHANDLE,NEWLINE|LFBEFORE);
      else
        break;
    }
  }

  sethandle();
  return(0);
}


static void _NEAR_ LIBENTRY underline(char *Str, int X) {
  for (; X != 0; X--, Str++)
    *Str = '=';
  *Str = 0;
}


static void _NEAR_ LIBENTRY resettopics(void) {
  int  ChNum;
  int  Node;
  char *ChTopic;
  char NewTopic[TOPICSIZE];

  refreshmemory();

  memset(NewTopic,0,TOPICSIZE);

  for (ChNum = 1, ChTopic = Topics + TOPICSIZE; ChNum <= 255; ChNum++, ChTopic += TOPICSIZE) {
    /* scan for channels whose topic is other than the default */
    if (*ChTopic != 0) {
      /* scan for nodes participating in the channel */
      for (Node = 1; Node < NUMNODES; Node++) {
        if (Channels[Node] == ChNum)
          break;
      }
      /* if NO NODES were found participating in this channel, then */
      /* reset the topic back to an open discussion                 */
      if (Node == NUMNODES) {
        doslseek(File.handle,((long) ChNum * TOPICSIZE) + TOPICOFFSET,SEEK_SET);
        writecheck(File.handle,NewTopic,TOPICSIZE);   //lint !e534
      }
    }
  }
}


static void _NEAR_ LIBENTRY showchannels(void) {
  bool FirstTime;
  bool NeverDisplayed;
  int  ChNum;
  int  Node;
  char Topic[TOPICSIZE];
  char Str[80];

  refreshmemory();
  NeverDisplayed = TRUE;

  for (ChNum = 1; ChNum <= 255; ChNum++) {
    FirstTime = TRUE;
    for (Node = 1; Node < NUMNODES; Node++) {
      if (Channels[Node] == ChNum) {
        if (FirstTime) {
          strcpy(Topic,Topics + (ChNum * TOPICSIZE));

          ascii(Status.DisplayText,ChNum);
          displaypcbtext(TXT_CHANNELTEXT,LFBEFORE);
          if (Topic[0] == 0)
            displaypcbtext(TXT_OPENTOPIC,DEFAULTS);
          else if (Topic[0] == 1)
            displaypcbtext(TXT_PRIVATETOPIC,DEFAULTS);
          else
            print(Topic);

          underline(Str,awherex());
          newline();
          println(Str);
          FirstTime = FALSE;
        }
        printcolor(PCB_CYAN);
        sprintf(Str,"(%d) %s",Node,Handles + (Node * HANDLESIZE));
        print(Str);
        if (Node == NodeNum) {
          printcolor(PCB_WHITE);
          print(" *");
        }
        newline();
        NeverDisplayed = FALSE;
        if (Display.AbortPrintout)
          break;
      }
    }
  }

  newline();

  if (NeverDisplayed)
    displaypcbtext(TXT_NOCHANNELSINUSE,NEWLINE);
}


static void _NEAR_ LIBENTRY showjoin(void) {
  pcbtexttype Buf;

  ascii(Status.DisplayText,Channel);
  displaypcbtext(TXT_JOININGCHANNEL,NEWLINE|LFBEFORE|LFAFTER);

  if (getfocus() == -1)
    displaypcbtext(TXT_CHANNELBUSY,NEWLINE);

  maxstrcpy(Status.DisplayText,MyHandle,sizeof(Status.DisplayText));
  getpcbtext(TXT_ENTEREDCHANNEL,&Buf);
  postmessageinqueue(Channel,ALLNODES,Buf.Str);
  releasefocus();
  LastNode = 0xFFFF;  /* force showstr() to re-announce who is typing */
}


static void _NEAR_ LIBENTRY showexit(char OldChannel) {
  pcbtexttype Buf;

  if (OldChannel != 0) {
    if (Channel == OldChannel && ! linebufisempty()) {
      carriagereturn();
      update();
      releasefocus();
      tickdelay(QUARTERSECOND);
    }
    maxstrcpy(Status.DisplayText,MyHandle,sizeof(Status.DisplayText));
    getpcbtext(TXT_LEFTCHANNEL,&Buf);
    postmessageinqueue(OldChannel,ALLNODES,Buf.Str);
  }
}


static void _NEAR_ LIBENTRY showhandlechange(char *OldHandle) {
  pcbtexttype Buf;
  char        Str[160];

  if (strcmp(OldHandle,MyHandle) != 0) {
    maxstrcpy(Status.DisplayText,OldHandle,sizeof(Status.DisplayText));
    getpcbtext(TXT_NEWHANDLE,&Buf);
    buildstr(Str,Buf.Str,MyHandle,NULL);
    postmessageinqueue(Channel,ALLNODES,Str);
  }
}


static void _NEAR_ LIBENTRY showtopicchange(char *Topic) {
  pcbtexttype Buf;
  char        Str[160];

  maxstrcpy(Status.DisplayText,MyHandle,sizeof(Status.DisplayText));
  getpcbtext(TXT_NEWTOPIC,&Buf);
  buildstr(Str,Buf.Str,Topic,NULL);
  postmessageinqueue(Channel,ALLNODES,Str);
}


static int _NEAR_ LIBENTRY getchannel(int NumTokens) {
  int      New;
  int      Old;
  int      NewOffsetIntoTopic;
  char     *p;
  char     Str[4];
  nodetype Buf;

  releasefocus();
  resettopics();

  if (CalledIntoChannel != 0)
    Old = CalledIntoChannel;
  else if (Channel == 0)
    Old = 1;
  else
    Old = Channel;

  while (1) {
    if (NumTokens == 0) {
      ascii(Str,Old);
      inputfield(Str,TXT_NEWCHANNEL,3,UPCASE|NEWLINE|LFBEFORE|FIELDLEN,NOHELP,mask_channel);
      p = Str;
    } else {
      p = getnexttoken();
      NumTokens = 0;
    }

    switch (*p) {
      case 'L': showchannels();
                continue;
      case 'Q': return(-1);
      default : New = atoi(p);
    }

    if (New > 0 && New < NUMCHANNELS) {
      if (New == Channel)
        return(0);

      doslseek(File.handle,TOPICOFFSET,SEEK_SET);
      readcheck(File.handle,Topics,TOPICBUFSIZE);   //lint !e534

      NewOffsetIntoTopic = New * TOPICSIZE;
      if (*(Topics + NewOffsetIntoTopic) == 1) {
        readusernetrecord(NodeNum,&Buf);
        if (New != CalledIntoChannel) {
          displaypcbtext(TXT_CHANNELISPRIVATE,NEWLINE|LFBEFORE|BELL);
          continue;
        }
      }
      break;  // passed all tests, get out now
    } else {
      displaypcbtext(TXT_INVALIDENTRY,NEWLINE|LFBEFORE);
    }
  }

  CalledIntoChannel = 0;
  Channel = (char) New;
  OffsetIntoTopic = NewOffsetIntoTopic;  //lint !e644  NewOffsetIntoTopic *is* being initialized
  setchannel();
  return(0);
}


static void _NEAR_ LIBENTRY getmonitor(int NumTokens) {
  int  X;

  if (NumTokens == 0) {
    if (Monitor) {
      memset(MonitorChannels,0,sizeof(MonitorChannels));
      displaypcbtext(TXT_MONITOROFF,NEWLINE);
      Monitor = FALSE;
    } else {
      memset(MonitorChannels,255,sizeof(MonitorChannels));
      displaypcbtext(TXT_MONITORON,NEWLINE);
      Monitor = TRUE;
    }
  } else {
    memset(MonitorChannels,0,sizeof(MonitorChannels));
    for (; NumTokens != 0; NumTokens--)
      if ((X = atoi(getnexttoken())) > 0 && X < NUMCHANNELS)
        setbit(MonitorChannels,X);

    displaypcbtext(TXT_MONITORON,NEWLINE);
    Monitor = TRUE;
  }
}


static void _NEAR_ LIBENTRY setprivpub(topicstatus Stat) {
  int         Made;
  int         Value;
  int         Now;
  pcbtexttype Buf;

  if (Stat == PRIVATE) {
    if (Channel == 1) {
      displaypcbtext(TXT_CANTBEPRIVATE,NEWLINE|LFAFTER);
      return;
    }
    Made = TXT_MADETOPICPRIVATE;
    Value = 1;
    Now = TXT_TOPICNOWPRIVATE;
  } else {
    Made = TXT_MADETOPICPUBLIC;
    Value = 0;
    Now = TXT_TOPICNOWPUBLIC;
  }

  maxstrcpy(Status.DisplayText,MyHandle,sizeof(Status.DisplayText));
  getpcbtext(Made,&Buf);
  postmessageinqueue(Channel,ALLNODES,Buf.Str);

  memset(CHANNELTOPIC,0,TOPICSIZE);
  *(CHANNELTOPIC) = (char) Value;
  settopic();

  postmessageinqueue(FORCEREFRESH,ALLNODES,"");  // force others to refresh

  displaypcbtext(Now,NEWLINE);

  if (Monitor)
    getmonitor(0);    // this will turn monitor off automatically
}


static void _NEAR_ LIBENTRY gettopic(int NumTokens, char *OldTopic) {
  char        *p;
  char        NewTopic[TOPICSIZE];
  pcbtexttype Buf;

  if (NumTokens > 0) {
    NewTopic[0] = 0;
    while (NumTokens) {
      p = getnexttoken();
      NumTokens--;
      addtext(NewTopic,p,HANDLESIZE);
    }
  } else {
    strcpy(NewTopic,OldTopic);
    if (NewTopic[0] == 0) {
      getpcbtext(TXT_OPENTOPIC,&Buf);
      maxstrcpy(NewTopic,Buf.Str,TOPICSIZE);
    } else if (NewTopic[0] == 1) {
      getpcbtext(TXT_PRIVATETOPIC,&Buf);
      maxstrcpy(NewTopic,Buf.Str,TOPICSIZE);
    }

    ascii(Status.DisplayText,Channel);
    inputfield(NewTopic,TXT_TOPICFORCHAT,TOPICSIZE-1,NEWLINE|LFBEFORE|HIGHASCII|FIELDLEN,NOHELP,mask_alphanum);
  }

  if (*NewTopic != 0 && strcmp(NewTopic,OldTopic) != 0) {
    if (*(CHANNELTOPIC) == 1)       // was it originally private?
      setprivpub(PUBLIC);           //   yes, change it to public first

    strcpy(CHANNELTOPIC,NewTopic);
    settopic();
    showtopicchange(CHANNELTOPIC);
  }
}


static void _NEAR_ LIBENTRY showignore(void) {
  bool FirstTime;
  int  X;
  char Str[10];

  for (X = 1, FirstTime = TRUE; X < NUMNODES; X++) {
    if (isset(IgnoreNodes,X)) {
      if (FirstTime) {
        displaypcbtext(TXT_IGNORINGNODES,LFBEFORE);
        FirstTime = FALSE;
      } else {
        print(", ");
      }
      ascii(Str,X);
      print(Str);
    }
  }

  if (! FirstTime)
    newline();
}


static void _NEAR_ LIBENTRY getignore(int NumTokens) {
  static char mask_ignore[] = {5, 0,'0','9', 'C', 'W'};
  bool Cancelled;
  int  X;
  char *p;
  char Str[80];

top:
  if (NumTokens == 0) {
    showignore();
    inputfield(Str,TXT_GETIGNORELIST,sizeof(Str)-1,NEWLINE|LFBEFORE|STACKED|UPCASE,NOHELP,mask_ignore);
    NumTokens = tokenize(Str);
    if (NumTokens == 0)
      return;
  }

  for (Cancelled = FALSE; NumTokens != 0; NumTokens--) {
    p = getnexttoken();
    if (alldigits(p) && (X = atoi(p)) > 0 && X < NUMNODES) {
      setbit(IgnoreNodes,X);
      Cancelled = FALSE;
    } else if (*(p+1) == 0) {
      switch (*p) {
        case 'C': memset(IgnoreNodes,0,sizeof(IgnoreNodes));
                  Cancelled = TRUE;
                  break;
        case 'W': displayusernet(0);
                  NumTokens = 0;
                  goto top;
      }
    }
  }

  if (Cancelled)
    displaypcbtext(TXT_IGNORECANCELLED,NEWLINE|LFBEFORE);
}


static void _NEAR_ LIBENTRY sendmessage(int NumTokens) {
  static char mask_getnode[] = {5, 0,'0','9', 'C', 'S'};
  unsigned    Num;
  char        *p;
  char        Str[80];

top:
  if (NumTokens != 0) {
    p = getnexttoken();
    NumTokens--;
  } else {
    if (PrivateSendNode != 0)
      ascii(Str,PrivateSendNode);
    else
      Str[0] = 0;

    inputfield(Str,TXT_SENDTONODE,5,NEWLINE|UPCASE|FIELDLEN,NOHELP,mask_getnode);
    p = Str;
  }

  if (*p == 'C')
    return;

  if (*p == 'S') {
    showchannels();
    NumTokens = 0;
    goto top;
  }

  if ((Num = atoi(p)) < 1 || Num > PCB_MAXNODES)
    return;

  if (*(Handles + (Num * HANDLESIZE)) == 0) {
    displaypcbtext(TXT_NODENOTINCHAT,NEWLINE|LFBEFORE);
    NumTokens = 0;
    goto top;
  }

  if (NumTokens != 0) {
    for (Str[0] = 0; NumTokens > 0; NumTokens--) {
      p = getnexttoken();
      addtext(Str,p,sizeof(Str));
    }
  } else {
    displaypcbtext(TXT_TEXTTOSEND,NEWLINE|LFBEFORE);

    Str[0] = 0;
    inputfieldstr(Str,"",Display.DefaultColor,75,NEWLINE|HIGHASCII|FIELDLEN|GUIDE,NOHELP,mask_alphanum);
    if (Str[0] == 0)
      return;
  }

  PrivateSendNode = Num;
  postmessageinqueue(Channel,(uint) Num,Str);
}


#ifdef COMM
static void _NEAR_ LIBENTRY lostcarrierinchat(void) {
  char        Str[80];
  char        Temp[80];
  pcbtexttype Buf;

  getsystext(TXT_CARRIERLOST,&Buf);
  xlatetext(Temp,Buf.Str);
  sprintf(Str,"%s: %s",MyHandle,Temp);
  postmessageinqueue(Channel,ALLNODES,Str);
  showexit(Channel);
  chatcleanup(FALSE);
  loguseroff(ALOGOFF);
}
#endif


static int _NEAR_ LIBENTRY getcommand(int Command) {
  bool RedisplayMenu;
  int  NumTokens;
  int  Num;
  char OldChannel;
  char *p;
  char OldHandle[HANDLESIZE];
  char OldTopic[TOPICSIZE];
  char Str[61];

  if (UnFocusedInput) {
    bell();
    return(0);
  }

  releasefocus();

  if (Command != 0) {
    Num = Command;
    NumTokens = 1;
    goto docmd;
  }

  freshline();
  RedisplayMenu = FALSE;

  while (1) {
    if (RedisplayMenu || ! UsersData.ExpertMode) {
      Display.NumLinesPrinted = 0;
      displayfile(PcbData.ChatMenu,GRAPHICS|LANGUAGE|SECURITY|RUNMENU|RUNPPL);
      RedisplayMenu = FALSE;
    }

    Str[0] = 0;
    inputfield(Str,(UsersData.ExpertMode ? TXT_CHATPROMPTEXPERT : TXT_CHATPROMPTNOVICE),sizeof(Str)-1,UPCASE|STACKED,HLP_CMENU,mask_command);

    #ifdef COMM
      if (Asy.LostCarrier) {
        lostcarrierinchat();
        return(-1);
      }
    #endif

    startdisplay(FORCENONSTOP);  // in case the INPUT reset non-stop mode
    NumTokens = tokenize(Str);
    backupcleareol(awherex());

    if (NumTokens == 0)
      return(0);

    p = getnexttoken();
    if (alldigits(p) && (Num = atoi(p)) > 0) {
      switch(Num) {
        case  7: dispatch(p,PcbData.SysopSec[SEC_7],NumTokens,usermaint);
                 return(0);
        case 11: strcpy(Str,"X");
                 tokenize(Str);   //lint !e534
                 dispatch("11",PcbData.SysopSec[SEC_11],2,displayusernet);
                 return(0);
        case 12: dispatch(p,PcbData.SysopSec[SEC_12],NumTokens,logoffcommand);
                 return(0);
        case 13: dispatch(p,PcbData.SysopSec[SEC_13],NumTokens,nodeviewcallerslog);
                 return(0);
        case 14: dispatch(p,PcbData.SysopSec[SEC_14],NumTokens,dropdoscommand);
                 return(0);
        case 15:
                 // need to fix the security level here!
                 dispatch(p,PcbData.SysopSec[SEC_12],NumTokens,recyclecommand);
                 break;
      }
    } else if (p[1] == 0) {
      switch(p[0]) {
        case 'G': showexit(Channel);
                  chatcleanup(FALSE);
                  byecommand();
                  return(-1);
        case 'Q': showexit(Channel);
                  return(-1);
        case 'X': if (seclevelokay(p,PcbData.UserLevels[SEC_X])) {
                    if (UsersData.ExpertMode) {
                      UsersData.ExpertMode = FALSE;
                      displaypcbtext(TXT_EXPERTOFF,NEWLINE|LFBEFORE|LOGIT);
                    } else {
                      UsersData.ExpertMode = TRUE;
                      displaypcbtext(TXT_EXPERTON,NEWLINE|LFBEFORE|LOGIT);
                    }
                  }
                  break;
      }
    } else {
      Num = option(Options,Str,NUMCHATOPTIONS);

docmd:
      switch(Num) {
        case O_BCAST  : dispatch("BRDCST",PcbData.SysopSec[SEC_BROADCAST],NumTokens,broadcast);
                        return(0);
        case O_BYE    : showexit(Channel);
                        chatcleanup(FALSE);
                        loguseroff(NLOGOFF);
                        return(-1);
        case O_CALL   : callcommand(NumTokens-1);
                        return(0);
        case O_CHANNEL: OldChannel = Channel;
                        if (getchannel(NumTokens-1) == -1)
                          return(-1);
                        if (Channel != OldChannel) {
                          showexit(OldChannel);
                          showjoin();
                        }
                        return(0);
        case O_ECHO   : Echo = TRUE;
                        displaypcbtext(TXT_ECHOENABLED,NEWLINE);
                        return(0);
        case O_IGNORE : getignore(NumTokens-1);
                        return(0);
        case O_HANDLE : strcpy(OldHandle,MyHandle);
                        if (gethandle(NumTokens-1) == -1)
                          return(-1);
                        showhandlechange(OldHandle);
                        return(0);
        case O_MENU   : RedisplayMenu = TRUE;
                        break;
        case O_MONITOR: getmonitor(NumTokens-1);
                        return(0);
        case O_NOECHO : Echo = FALSE;
                        displaypcbtext(TXT_ECHODISABLED,NEWLINE);
                        return(0);
        case O_QUIT   : showexit(Channel);
                        return(-1);
        case O_PRIVATE: setprivpub(PRIVATE);
                        return(0);
        case O_PUBLIC : setprivpub(PUBLIC);
                        return(0);
        case O_SEND   : sendmessage(NumTokens-1);
                        return(0);
        case O_SHOW   : showchannels();
                        return(0);
        case O_SILENT : displaypcbtext((Silent ? TXT_SILENTOFF : TXT_SILENTON),NEWLINE);
                        Silent = (bool) (! Silent);
                        return(0);
        case O_TOPIC  : strcpy(OldTopic,CHANNELTOPIC);
                        gettopic(0,OldTopic);
                        return(0);
        case O_WHO    : dispatch("WHO",PcbData.UserLevels[SEC_WHO],NumTokens,displayusernet);
                        startdisplay(FORCENONSTOP);  // in case WHO display reset non-stop mode
                        newline();
                        return(0);
      }
    }
  }
}


static int _NEAR_ LIBENTRY openchat(void) {
  Channels = NULL;
  ChatOutBuf = NULL;
  ChatInBuf = NULL;
  LineBuf = NULL;
  WritePtrsOnDisk = NULL;
  ReadPtrsInMemory = NULL;
  Handles = NULL;
  Topics = NULL;
  memset(&File,0,sizeof(File));

  if ((HeaderBuffer = (char *) bmalloc(REALHEADERSIZE)) == NULL ||
      (ChatOutBuf = (char *) bmalloc(BUFSIZE)) == NULL ||
      (ChatInBuf = (char *) bmalloc(BUFSIZE)) == NULL ||
      (LineBuf = (char *) bmalloc(BUFSIZE)) == NULL ||
      (ReadPtrsInMemory = (short *) bmalloc(PTRBUFSIZE)) == NULL)
    return(1);

  NodeNum = PcbData.NodeNum;

  if (dosfopen(PcbData.ChatFile,OPEN_RDWR|OPEN_DENYNONE,&File) == -1)
    return(2);

  Channels        = (char *)  (HeaderBuffer);
  WritePtrsOnDisk = (short *) (HeaderBuffer + CHANNELBUFSIZE);
  Handles         = (char *)  (HeaderBuffer + CHANNELBUFSIZE + PTRBUFSIZE);
  Topics          = (char *)  (HeaderBuffer + CHANNELBUFSIZE + PTRBUFSIZE + HANDLEBUFSIZE);

  dossetbuf(&File,8192);
  memset(HeaderBuffer,0,REALHEADERSIZE);
  memset(ReadPtrsInMemory,0,PTRBUFSIZE);
  memset(ChatOutBuf,0,BUFSIZE);
  LastChannel = 0;
  CapFile     = 0;
  BlankLines  = 0;

  if (checkfilesize() == -1)
    return(3);

  writechatbuf();

  MyHandle = Handles + (NodeNum * HANDLESIZE);
  return(0);
}


static void _NEAR_ LIBENTRY debugoutput(char *Str) {
  if (DebugLevel >= 5)
    writedebugrecord(Str);
}


static void _NEAR_ LIBENTRY closechat(void) {
  if (ChatOutBuf != NULL) {
    debugoutput("Close Chat - ChatOutBuf");
    memset(ChatOutBuf,0,BUFSIZE);
    if (File.handle > 0)
      writechatbuf();
  }

  Channel = LastChannel = (char) 0;
  OffsetIntoTopic = 0;

  if (CapFile > 0) {
    debugoutput("Close Chat - CapFile");
    dosclose(CapFile);
    CapFile = 0;
  }

  if (File.handle > 0) {
    releasefocus();
    debugoutput("Close Chat - setchannel()");
    setchannel();
    debugoutput("Close Chat - resettopics()");
    resettopics();
    dosfclose(&File);
  }

  debugoutput("Close Chat - Ptrs");

  if (ReadPtrsInMemory != NULL) {
    bfree(ReadPtrsInMemory);
    ReadPtrsInMemory = NULL;
  }
  if (LineBuf != NULL) {
    bfree(LineBuf);
    LineBuf = NULL;
  }
  if (ChatInBuf != NULL) {
    bfree(ChatInBuf);
    ChatInBuf = NULL;
  }
  if (ChatOutBuf != NULL) {
    bfree(ChatOutBuf);
    ChatOutBuf = NULL;
  }
  if (HeaderBuffer != NULL) {
    bfree(HeaderBuffer);
    HeaderBuffer = NULL;
  }

  debugoutput("Close Chat - Done");
}


#ifdef __OS2__
static bool KillThread;

#pragma argsused
static void THREADFUNC focusthread(void * Ignore) {
  CSemaphore Timer;

  Timer.createunique("GCHAT");

  while (! KillThread) {
    Timer.waitforevent(ONESECOND);
    if (FocusOnMe) {
      if ((awherex() == 0 && gettimer(KEYTIMER) < SHORTDELAY) || timerexpired(KEYTIMER))
        releasefocus();
    }
  }
}

static void LIBENTRY startfocusthread(void) {
  KillThread = FALSE;
  startthread(focusthread,8*1024,NULL);
}

static void LIBENTRY stopfocusthread(void) {
  KillThread = TRUE;
}
#endif  // ifdef __OS2__


static void _NEAR_ LIBENTRY chatcleanup(bool Recycling) {
  #ifdef __OS2__
    stopfocusthread();
  #endif

  if (InChat) {
    timestr2(Status.DisplayText);
    logsystext(TXT_NODECHATENDED,SPACERIGHT);

    if (! Recycling)
      displaypcbtext(TXT_NODECHATENDED,NEWLINE|LFBEFORE);

    #ifdef PCB152
    {
      long MinutesUsed;

      if (StartTime != -1) {
        MinutesUsed = minutesused(StartTime);
        if (Status.ActStatus != ACT_DISABLED && MinutesUsed != 0) {
          recordusage("CHAT TIME","",AccountRates.ChargeForGroupChat,MinutesUsed,&UsersData.Account.DebitGroupChat);
          checkaccountbalance();
        }
      }
    }
    #endif
  }

  closechat();
  InChat = FALSE;

  if (! Recycling) {
    usernetavailable();
    // Restore IgnoreCDLoss status
    #ifdef COMM
      Asy.IgnoreCDLoss = FALSE;
    #endif
  }
}


void LIBENTRY newchat(void) {
  bool UserWasCalledIntoChat;
  int  Key;
  int  Rate;
  int  Delay;
  int  Temp;
  #ifdef AUTO
  bool Auto = FALSE;
  #endif

  checkforansi();

  StartTime = -1;       // it stays set to -1 until node chat is really entered

  switch (openchat()) {
    case 0: break;
    case 1: displaypcbtext(TXT_INSUFMEMFORCHAT,NEWLINE|LOGIT);
            closechat();
            return;
    case 2: displaypcbtext(TXT_CANTOPENCHATFILE,NEWLINE|LOGIT);
            closechat();
            return;
    case 3: displaypcbtext(TXT_CHATFORMATERROR,NEWLINE|LOGIT);
            closechat();
            return;
  }

  dossetbuf(&File,2048);

  LastNode        = 0xFFFF;
  SaveX           = 0;
  UnFocusedInput  = FALSE;
  Silent          = FALSE;
  Monitor         = FALSE;
  Echo            = TRUE;
  PrivateSendNode = 0;
  resetlinebuffer();

  memset(MonitorChannels,0,sizeof(MonitorChannels));
  memset(IgnoreNodes,0,sizeof(IgnoreNodes));

  Delay   = HALFSECOND;
  Channel = 0;
  OffsetIntoTopic = 0;

  refreshmemory();

  memcpy(ReadPtrsInMemory,WritePtrsOnDisk,PTRBUFSIZE);
  WritePtrInMemory = WritePtrsOnDisk[NodeNum];
  CharsInBuffer = 0;

  startdisplay(FORCENONSTOP);  // also turn kbd timer off during node chat

  UserWasCalledIntoChat = CalledIntoChannel;
  if (getchannel(0) == -1)
    goto done;

  maxstrcpy(MyHandle,Status.FirstName,HANDLESIZE);
  if (gethandle(0) == -1)
    goto done;

  InChat = TRUE;
  writeusernetstatus(GROUPCHAT,NULL);

  UsersData.Stats.NumGroupChats++;
  timestr2(Status.DisplayText);
  logsystext(TXT_NODECHATENTERED,SPACERIGHT);

  StartTime = dosgetlongtime();

  if (! UseAnsi)
    displayfile(PcbData.NoAnsi,SECURITY|LANGUAGE);


  // Set Asy.IngoreCDLoss status so that the chat routines can detect the
  // carrier loss and, when it occurs, inform other users in the chat session
  // that the caller has lost carrier so that they don't think he is still
  // connected and in chat.
  #ifdef COMM
    Asy.IgnoreCDLoss = TRUE;
  #endif

  showjoin();
//showchannels();

  if (! UserWasCalledIntoChat) {
    if (! UsersData.ExpertMode)
      moreprompt(PRESSENTER);

    if (getcommand(0) == -1)
      goto done;
  }

  displaypcbtext(TXT_PRESSESCFORCOMMAND,NEWLINE|LFBEFORE);
  startdisplay(FORCENONSTOP);
  settimer(ACCESSTIMER,Delay);
  #ifdef __OS2__
    releasekbdblock(Delay);
    startfocusthread();
  #endif

  while (1) {
    #ifdef __OS2__
      Key = waitforkey();
    #else
      Key = cinkey();
    #endif
    if (Key != 0) {
      if (Key < 1000) {
        #ifdef COMM
          if (Key == -1) {
            lostcarrierinchat();
            goto done;
          }
        #endif
        switch (Key) {
          #ifdef AUTO
          case '!': Auto = TRUE; break;
          case '@': Auto = FALSE; break;
          #endif
          case   8:
          case 127: backspace();
                    break;
          case  10: break;                     // don't let LF through!
          case  13: carriagereturn();
//                  update();                  // note:  previous "focus version" did not use update - should this line be removed?
                    break;
          case  27: if (getcommand(0) == -1)
                      goto done;
                    startdisplay(FORCENONSTOP);
                    break;
          default:  if (Key < ' ' && Key > 3 && Key != 20)
                      break;
                    if ((Temp = process((char) Key)) == -1)
                      goto done;
                    if (Temp)
                      addchartolinebuf((char) Key);
                    break;
        }
      }

      #ifdef __OS2__
        updatelinesnow();
      #endif
    } else {
      // no keystrokes to process...  is the focus on us?
      // yes, are we either:  1) sitting in column 1 and have been for 1/2
      // second, or 2) idling for 10 seconds or more?  if so, release focus

      #ifdef AUTO
        if (Auto) {
          tickdelay(HALFSECOND);
          stuffbufferstr("ABCD ");
        }
      #endif

      #ifndef __OS2__
        if (FocusOnMe) {
          if ((awherex() == 0 && gettimer(KEYTIMER) < SHORTDELAY) || timerexpired(KEYTIMER))
            releasefocus();
        }
      #endif
    }


    // NOTE:  This is TEST code - it shows the tick countdown on the screen
    #ifdef TEST
    {
      char TempStr[5];
      int  TicksLeft;

      if (FocusOnMe) {
        TicksLeft = (int) gettimer(KEYTIMER);
        if (awherex() == 0)
          TicksLeft -= SHORTDELAY;
        if (TicksLeft < 0)
          TicksLeft = 0;
        sprintf(TempStr,"%2d",((TicksLeft + 9) * 10) / 182);
        fastprint(56,Status.StatusLine1,TempStr,0x74);
      } else
        fastprint(56,Status.StatusLine1,"  ",0x70);
    }
    #endif

    if (Control.WarnMinute != 0)
      warntime();

    if (timerexpired(ACCESSTIMER)) {
      update();
      Delay = 3 * TENTHSECOND;

      // we only scan for new information when it's not our focus
      if (! FocusOnMe) {              // in non-focus version, was if (linebufisempty())
        Rate = getlatest();

        /* set the scanning rate according to how fast data is coming in */
        if (Rate > 100)
          Delay = -1;
        else if (Rate > 50)
          Delay = TENTHSECOND;
        else if (Rate > 10)
          Delay = QUARTERSECOND;
      }

      settimer(ACCESSTIMER,Delay);
      #ifdef __OS2__
        releasekbdblock(Delay);
      #endif
    }

    #ifdef __OS2__
      if ((! FocusOnMe) && needtoscanusernet())
        scanusernet();  //lint !e534
    #else
      if ((! FocusOnMe) && timerexpired(7)) {  // in non-buffered version was: if (timerexpired(7) && linebufisempty()) {
        settimer(7,Control.NetTimer);
        scanusernet();    //lint !e534
      }
    #endif
  }

done:
  chatcleanup(FALSE);
}


void LIBENTRY initchatfile(void) {
  openchat();      //lint !e534
  closechat();
}


// The exitchat() function is called in the RECYCLE.C module.  This function
// checks to see if the caller was in chat.  If so, we probably got here
// because of a keyboard timeout or session timeout.  To keep things standard,
// we'll use the same "carrier lost" message to inform other callers of the
// caller's departure from the chat channel.

void LIBENTRY exitchat(void) {
  char        Str[80];
  char        Temp[80];
  pcbtexttype Buf;

  if (InChat) {
    getsystext(TXT_CARRIERLOST,&Buf);
    xlatetext(Temp,Buf.Str);
    sprintf(Str,"%s: %s",MyHandle,Temp);
    postmessageinqueue(Channel,ALLNODES,Str);
    showexit(Channel);
    chatcleanup(TRUE);
  }
}
