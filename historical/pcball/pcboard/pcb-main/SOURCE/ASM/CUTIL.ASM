;*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
;* The source code in this module is proprietary software belonging to       */
;* Clark Development Company and is part of the PCBoard source code library. */
;* You are granted the right to use this source code for the building of any */
;* of the PCBoard products you have licensed.  Any other usage is forbidden  */
;* without prior written consent from Clark Development Company, Inc.        */
;*                                                                           */
;* Be sure to read the source code license agreement before utilizing any    */
;* of the source code found herein.                                          */
;*                                                                           */
;* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
;*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


Page 66,132

include \proj\pcb\source\h\rules.asi

ifdef __s__
  .model small, pascal
elseifdef __m__
  .model medium, pascal
elseifdef __c__
  .model compact, pascal
elseifdef __l__
  .model large, pascal
endif

if LPROG
   ALIGN2  equ ALIGN 2
   ALIGN16 equ ALIGN 16
else
   ALIGN2  equ ALIGN 1
   ALIGN16 equ ALIGN 1
endif


;   Global MACRO definitions
;   ~~~~~~~~~~~~~~~~~~~~~~~~
;-------------------------------------------------------------------------
Jmps      Macro Dummy
          Jmp   Short Dummy
          EndM
;-------------------------------------------------------------------------

public   _FindTokenStart, _FindTokenEnd, _FindTokenColor, _FindTokenAttr
public   _StackCheck

IFNDEF LIB
extrn    __stklen:word
ENDIF

.data
_FindTokenStart dw      0
_FindTokenEnd   dw      0
_FindTokenColor db      0
_FindTokenAttr  db      0

_StackCheck     dw      0FFFFh

if LPROG
   cutil  segment para public 'code'
   assume    cs:cutil
else
   CSeg@
endif

;=============================================================================
;   FindToken
;=============================================================================

          ALIGN2
JmpTable  Label Word
          Dw    A                      ;A - ALIAS/AUTOMORE
          Dw    B                      ;B - BEEP/BICPS/BOARDNAME/BPS/BYTECREDIT/BYTELIMIT/BYTERATIO/BYTESLEFT
          Dw    C                      ;C - CARRIER/CITY/CLREOL/CLS/CONFNAME/CONFNUM/CREDLEFT/CREDNOW/CREDSTART/CREDUSED/CURMSGNUM
          Dw    D                      ;D - DATAPHONE/DAYBYTES/DELAY/DIRNAME/DIRNUM/DLBYTES/DLFILES
          Dw    E                      ;E - ENV=/EVENT/EXPDATE/EXPDAYS
          Dw    F                      ;F - FBYTES/FFILES/FILECREDIT/FILERATIO/FIRST/FIRSTU/FNUM/FREESPACE
          Dw    Findat                 ;G -
          Dw    H                      ;H - HIGHMSGNUM/HOMEPHONE
          Dw    I                      ;I - INAME/INCONF
          Dw    Findat                 ;J -
          Dw    K                      ;K - KBLEFT/KBLIMIT
          Dw    L                      ;L - LASTCALLERNODE/LASTCALLERSYSTEM/LASTDATEON/LASTTIMEON/LMR/LOGDATE/LOGTIME/LOWMSGNUM
          Dw    M                      ;M - MAXBYTES/MAXFILES/MINLEFT/MORE/MSGLEFT/MSGREAD
          Dw    N                      ;N - NOCHAR/NODE/NUMBLT/NUMCALLS/NUMCONF/NUMDIR/NUMTIMESON
          Dw    O                      ;O - OFFHOURS/OPTEXT
          Dw    P                      ;P - PAUSE/POFF/PON/POS/PROLTR/PRODESC/PWXDATE/PWXDAYS
          Dw    Q                      ;Q - QOFF/QON
          Dw    R                      ;R - RATIOBYTES/RATIOFILES/RBYTES/RCPS/REAL/RFILES
          Dw    S                      ;S - SBYTES/SCPS/SECURITY/SFILES/SYSDATE/SYSOPIN/SYSOPOUT/SYSTIME
          Dw    T                      ;T - TIMELIMIT/TIMELEFT/TIMEUSED/TOTALTIME
          Dw    U                      ;U - UPBYTES/UPFILES/USER
          Dw    Findat                 ;V -
          Dw    W                      ;W - WAIT/WHO
          Dw    X                      ;X - color substitution
          Dw    FTscan                 ;Y - YESCHAR

DataTable Label Word
          Dw    Asrch1                 ;A - ALIAS/AUTOMORE
          Dw    Bsrch1                 ;B - BEEP/BICPS/BOARDNAME/BPS/BYTECREDIT/BYTELIMIT/BYTERATIO/BYTESLEFT
          Dw    Csrch1                 ;C - CARRIER/CITY/CLREOL/CLS/CONFNAME/CONFNUM/CREDLEFT/CREDNOW/CREDSTART/CREDUSED/CURMSGNUM
          Dw    Dsrch1                 ;D - DATAPHONE/DAYBYTES/DELAY/DIRNAME/DIRNUM/DLBYTES/DLFILES
          Dw    Esrch1                 ;E - ENV=/EVENT/EXPDATE/EXPDAYS
          Dw    Fsrch1                 ;F - FBYTES/FFILES/FILECREDIT/FILERATIO/FIRST/FIRSTU/FNUM/FREESPACE
          Dw    Nosrch                 ;G -
          Dw    Hsrch1                 ;H - HIGHMSGNUM/HOMEPHONE
          Dw    Isrch1                 ;I - INAME/INCONF
          Dw    Nosrch                 ;J -
          Dw    Ksrch1                 ;K - KBLEFT/KBLIMIT
          Dw    Lsrch1                 ;L - LASTCALLERNODE/LASTCALLERSYSTEM/LASTDATEON/LASTTIMEON/LMR/LOGDATE/LOGTIME/LOWMSGNUM
          Dw    Msrch1                 ;M - MAXBYTES/MAXFILES/MINLEFT/MORE/MSGLEFT/MSGREAD
          Dw    Nsrch1                 ;N - NOCHAR/NODE/NUMBLT/NUMCALLS/NUMCONF/NUMDIR/NUMTIMESON
          Dw    Osrch1                 ;O - OFFHOURS/OPTEXT
          Dw    Psrch1                 ;P - PAUSE/POFF/PON/POS/PROLTR/PRODESC/PWXDATE/PWXDAYS
          Dw    Qsrch1                 ;Q - QOFF/QON
          Dw    Rsrch1                 ;R - RATIOBYTES/RATIOFILES/RBYTES/RCPS/REAL/RFILES
          Dw    Ssrch1                 ;S - SBYTES/SCPS/SECURITY/SFILES/SYSDATE/SYSOPIN/SYSOPOUT/SYSTIME
          Dw    Tsrch1                 ;T - TIMELIMIT/TIMELEFT/TIMEUSED/TOTALTIME
          Dw    Usrch1                 ;U - UPBYTES/UPFILES/USER
          Dw    Nosrch                 ;V -
          Dw    Wsrch1                 ;W - WAIT/WHO
          Dw    Xsrch1                 ;X - handled separately
          Dw    Ysrch                  ;Y - YESCHAR

Asrch1    Db    4, 1,'LIAS'
Asrch2    Db    7, 2,'UTOMORE'
Bsrch1    Db    3, 3,'EEP'
Bsrch2    Db    4, 4,'ICPS'
Bsrch3    Db    8, 5,'OARDNAME'
Bsrch4    Db    2, 6,'PS'
Bsrch5    Db    9, 7,'YTECREDIT'
Bsrch6    Db    8, 8,'YTELIMIT'
Bsrch7    Db    8, 9,'YTERATIO'
Bsrch8    Db    8,10,'YTESLEFT'
Csrch1    Db    6,11,'ARRIER'
Csrch2    Db    3,12,'ITY'
Csrch3    Db    5,13,'LREOL'
Csrch4    Db    2,14,'LS'
Csrch5    Db    7,15,'ONFNAME'
Csrch6    Db    6,16,'ONFNUM'
Csrch7    Db    7,17,'REDLEFT'
Csrch8    Db    6,18,'REDNOW'
Csrch9    Db    8,19,'REDSTART'
Csrch10   Db    7,20,'REDUSED'
Csrch11   Db    8,21,'URMSGNUM'
Dsrch1    Db    8,22,'ATAPHONE'
Dsrch2    Db    7,23,'AYBYTES'
Dsrch3    Db    4,24,'ELAY'
Dsrch4    Db    6,25,'IRNAME'
Dsrch5    Db    5,26,'IRNUM'
Dsrch6    Db    6,27,'LBYTES'
Dsrch7    Db    6,28,'LFILES'
Esrch1    Db    3,29,'NV='
Esrch2    Db    4,30,'VENT'
Esrch3    Db    6,31,'XPDATE'
Esrch4    Db    6,32,'XPDAYS'
Fsrch1    Db    5,33,'BYTES'
Fsrch2    Db    5,34,'FILES'
Fsrch3    Db    9,35,'ILECREDIT'
Fsrch4    Db    8,36,'ILERATIO'
Fsrch5    Db    5,37,'IRSTU'
Fsrch6    Db    4,38,'IRST'
Fsrch7    Db    3,39,'NUM'
Fsrch8    Db    8,40,'REESPACE'
Hsrch1    Db    8,41,'OMEPHONE'
Hsrch2    Db    9,42,'IGHMSGNUM'
Isrch1    Db    4,43,'NAME'
Isrch2    Db    5,44,'NCONF'
Ksrch1    Db    5,45,'BLEFT'
Ksrch2    Db    6,46,'BLIMIT'
Lsrch1    Db   13,47,'ASTCALLERNODE'
Lsrch2    Db   15,48,'ASTCALLERSYSTEM'
Lsrch3    Db    9,49,'ASTDATEON'
Lsrch4    Db    9,50,'ASTTIMEON'
Lsrch5    Db    2,51,'MR'
Lsrch6    Db    6,52,'OGDATE'
Lsrch7    Db    6,53,'OGTIME'
Lsrch8    Db    8,54,'OWMSGNUM'
Msrch1    Db    7,55,'AXBYTES'
Msrch2    Db    7,56,'AXFILES'
Msrch3    Db    6,57,'INLEFT'
Msrch4    Db    3,58,'ORE'
Msrch5    Db    6,59,'SGLEFT'
Msrch6    Db    6,60,'SGREAD'
Nsrch1    Db    5,61,'OCHAR'
Nsrch2    Db    3,62,'ODE'
Nsrch3    Db    5,63,'UMBLT'
Nsrch4    Db    7,64,'UMCALLS'
Nsrch5    Db    6,65,'UMCONF'
Nsrch6    Db    5,66,'UMDIR'
Nsrch7    Db    9,67,'UMTIMESON'
Osrch1    Db    7,68,'FFHOURS'
Osrch2    Db    5,69,'PTEXT'
Psrch1    Db    4,70,'AUSE'
Psrch2    Db    3,71,'OFF'
Psrch3    Db    2,72,'ON'
Psrch4    Db    2,73,'OS'
Psrch5    Db    5,74,'ROLTR'
Psrch6    Db    6,75,'RODESC'
Psrch7    Db    6,76,'WXDATE'
Psrch8    Db    6,77,'WXDAYS'
Qsrch1    Db    3,78,'OFF'
Qsrch2    Db    2,79,'ON'
Rsrch1    Db    9,80,'ATIOBYTES'
Rsrch2    Db    9,81,'ATIOFILES'
Rsrch3    Db    5,82,'BYTES'
Rsrch4    Db    3,83,'CPS'
Rsrch5    Db    3,84,'EAL'
Rsrch6    Db    5,85,'FILES'
Ssrch1    Db    5,86,'BYTES'
Ssrch2    Db    3,87,'CPS'
Ssrch3    Db    7,88,'ECURITY'
Ssrch4    Db    5,89,'FILES'
Ssrch5    Db    6,90,'YSDATE'
Ssrch6    Db    6,91,'YSOPIN'
Ssrch7    Db    7,92,'YSOPOUT'
Ssrch8    Db    6,93,'YSTIME'
Tsrch1    Db    8,94,'IMELIMIT'
Tsrch2    Db    7,95,'IMELEFT'
Tsrch3    Db    7,96,'IMEUSED'
Tsrch4    Db    8,97,'OTALTIME'
Usrch1    Db    6,98,'PBYTES'
Usrch2    Db    6,99,'PFILES'
Usrch3    Db    3,100,'SER'
Wsrch1    Db    3,101,'AIT'
Wsrch2    Db    2,102,'HO'
Xsrch1    Db    3,103,'OFF'
Xsrch2    Db    2,104,'ON'
Ysrch     Db    6,105,'ESCHAR'
Nosrch    Db    0  ;................  number 106 defined below


          Public FINDTOKEN
          ALIGN16
FindToken Proc  String:ptr
          Local FoundLoc:word, FoundCx:word
          Uses  Si, Di

          Push  Ds
if LDATA
          Les   Di,String         ;ES:DI points to string
          Mov   Ax,Es
          Mov   Ds,Ax             ;Also DS:DI points to the string
else
          Mov   Di,Ds
          Mov   Es,Di
          Mov   Di,String         ;ES:DI and DS:DI point to string
endif

          Cld
          Xor   Cx,Cx             ;Cx will be our count of bytes in string

FindAt:   Mov   Bx,'@'            ;Bh = 0, Bl = '@'
FindAt2:  Mov   Al,[Di]           ;Get character from string
          Inc   Cx
          Or    Al,Al             ;If it's NULL we're done
          Jz    NotFound
          Inc   Di
          Cmp   Al,Bl             ;If it's not an '@' then continue the scan
          Jne   FindAt2

          Mov   Bl,[Di]           ;put next character into BL
          Cmp   Bl,'A'
          Jb    FindAt            ;Restart and fix Bx at the same time
          Cmp   Bl,'Y'
          Ja    FindAt            ;Restart and fix Bx at the same time

          Sub   Bl,'A'            ;set offset to 0 for the letter 'B'
          Mov   FoundLoc,Di       ;save where we found the '@' character
          Mov   FoundCx,Cx        ;save where we were in the CX countdown
          Shl   Bx,1              ;multiply by 2 to make an offset into table
          Inc   Di                ;point to next character
;         Dec   Cx                ;decrement it in case we go back for more!

          Mov   Si, word ptr cs:DataTable[Bx]
          Jmp   word ptr cs:JmpTable[Bx]

NotFound: Xor   Ax,Ax
          Pop   Ds
          Ret

Restart:  Mov   Di,FoundLoc
          Mov   Cx,FoundCx
          Jmps  FindAt

Padding:  Inc   FoundCx                ;remove the extra characters ":###?@"
          Inc   Di
          Mov   Al,[Di]
          Cmp   Al,'0'
          Jb    Restart
          Cmp   Al,'@'
          Je    FTfound3
          Mov   Ch,'R'
          Cmp   Al,Ch
          Je    Padding
          Mov   Ch,'C'
          Cmp   Al,Ch
          Je    Padding
          Mov   Ch,'T'
          Cmp   Al,Ch
          Je    Padding
          Cmp   Al,'9'
          Ja    Restart
          Mov   Ch,Cl      ; multiply by ten
          Shl   Cl,1       ;    "
          Shl   Cl,1       ;    "
          Shl   Cl,1       ;    "
          Add   Cl,Ch      ;    "
          Add   Cl,Ch      ;    "
          Add   Cl,Al      ; add new one's digit
          Sub   Cl,'0'     ; but subtract the ascii out of it
          Xor   Ch,Ch
          Jmps  Padding

FTscan:   Xor   Ch,Ch
          Mov   Cl,cs:[Si]        ;get Len in Cx
          Inc   Si
          Mov   Bx,Cx             ;save length for use below
          Mov   Dl,cs:[Si]        ;get keynum in DL
          Inc   Si                ;(SI now points to Str in table)

FTscan0:  Mov   Al,[Di]           ;get a character
          Inc   Di
          Cmp   Al,cs:[Si]        ;compare to table string
          Jne   Restart
          Inc   Si
          Dec   Cx
          Jnz   FTscan0

FTfound:  Cmp   byte ptr [Di],':'
          Je    Padding
FTfound2: Cmp   byte ptr [Di],'@'
          Jne   Restart
FTfound3: Mov   Di,FoundLoc
          Dec   Di
          Xor   Ah,Ah
          Mov   Al,Dl             ;Keynum is now in Ax
          Mov   [Di],Ah           ;null terminate the string
          Mov   Dx,FoundCx
          Add   Dx,Bx
          Inc   Dx
          Inc   Dx                ;On return DX = Offset to END of macro
          Mov   Bx,Di             ;          BX = ptr to START of macro
          Sub   Di,word ptr String;          CL = may be a Color Code
          Pop   Ds                ;          CH = may be field attribute

          mov   _FindTokenStart,Di          ;offset to START of macro
          mov   _FindTokenEnd,Dx            ;offset to END of macro
          mov   _FindTokenColor,Cl          ;may be a Color code
          mov   _FindTokenAttr,Ch           ;may be a field attribute
          Ret

FTenv:    Xor   Ch,Ch
          Mov   Cl,cs:[Si]        ;get Len in Cx
          Inc   Si
          Mov   Bx,Cx             ;save length for use below
          Mov   Dl,cs:[Si]        ;get keynum in DL
          Inc   Si                ;(SI now points to Str in table)

FTenv0:   Mov   Al,[Di]           ;get a character
          Inc   Di
          Cmp   Al,cs:[Si]        ;compare to table string
          Jne   FTenvbad
          Inc   Si
          Dec   Cx
          Jnz   FTenv0

FTenvloop:Mov   Al,[Di]
          Cmp   Al,':'
          Je    FTenv1
          Cmp   Al,'@'
          Je    FTenv2
          Inc   Bx                ;incrementing the "size" of the macro
          Inc   Di
          Or    Al,Al
          Jnz   FTenvloop
          Jmp   Restart
FTenv1:   Xor   Cx,Cx
          Jmp   Padding
FTenv2:   Xor   Cx,Cx
          Jmp   FTfound3
FTenvbad: Jmp   Restart


A:        Cmp   byte ptr [Di],'L'
          Je    FTscan1
          Mov   Si,offset Asrch2
          Jmps  FTscan1
B:        Cmp   byte ptr [Di],'E'
          Je    FTscan1
          Mov   Si,offset Bsrch2
          Cmp   byte ptr [Di],'I'
          Je    FTscan1
          Mov   Si,offset Bsrch3
          Cmp   byte ptr [Di],'O'
          Je    FTscan1
          Mov   Si,offset Bsrch4
          Cmp   byte ptr [Di],'P'
          Je    FTscan1
          Mov   Si,offset Bsrch5
          Cmp   byte ptr [Di+3],'C'
          Je    FTscan1
          Mov   Si,offset Bsrch6
          Cmp   byte ptr [Di+3],'L'
          Je    FTscan1
          Mov   Si,offset Bsrch7
          Cmp   byte ptr [Di+3],'R'
          Je    FTscan1
          Mov   Si,offset Bsrch8
          Jmp   FTscan

FTscan1:  Jmp   FTscan

C:        Cmp   byte ptr [Di],'A'
          Je    FTscan1
          Mov   Si,offset Csrch2
          Cmp   byte ptr [Di],'I'
          Je    FTscan1
          Mov   Si,offset Csrch3
          Cmp   byte ptr [Di+2],'E'
          Je    FTscan1
          Mov   Si,offset Csrch4
          Cmp   byte ptr [Di+1],'S'
          Je    FTscan1
          Mov   Si,offset Csrch5
          Cmp   byte ptr [Di+4],'A'
          Je    FTscan1
          Mov   Si,offset Csrch6
          Cmp   byte ptr [Di+4],'U'
          Je    FTscan1
          Mov   Si,offset Csrch7
          Cmp   byte ptr [Di+3],'L'
          Je    FTscan1
          Mov   Si,offset Csrch8
          Cmp   byte ptr [Di+3],'N'
          Je    FTscan1
          Mov   Si,offset Csrch9
          Cmp   byte ptr [Di+4],'T'
          Je    FTscan1
          Mov   Si,offset Csrch10
          Cmp   byte ptr [Di+3],'U'
          Je    FTscan1
          Mov   Si,offset Csrch11
          Jmp   FTscan
D:        Cmp   byte ptr [Di+1],'T'
          Je    FTscan1
          Mov   Si,offset Dsrch2
          Cmp   byte ptr [Di+1],'Y'
          Je    FTscan1
          Mov   Si,offset Dsrch3
          Cmp   byte ptr [Di],'E'
          Je    FTscan2
          Mov   Si,offset Dsrch4
          Cmp   byte ptr [Di+3],'A'
          Je    FTscan2
          Mov   Si,offset Dsrch5
          Cmp   byte ptr [Di+3],'U'
          Je    FTscan2
          Mov   Si,offset Dsrch6
          Cmp   byte ptr [Di+1],'B'
          Je    FTscan2
          Mov   Si,offset Dsrch7
          Jmp   FTscan
E:        Cmp   byte ptr [Di+2],'='
          Jne   E1
          Jmp   FTenv
E1:       Mov   Si,offset Esrch2
          Cmp   byte ptr [Di],'V'
          Je    FTscan2
          Mov   Si,offset Esrch3
          Cmp   byte ptr [Di+4],'T'
          Je    FTscan2
          Mov   Si,offset Esrch4
          Jmp   FTscan

FTscan2:  Jmp   FTscan

F:        Cmp   byte ptr [Di],'B'
          Je    FTscan2
          Mov   Si,offset Fsrch2
          Cmp   byte ptr [Di],'F'
          Je    FTscan2
          Mov   Si,offset Fsrch3
          Cmp   byte ptr [Di+3],'C'
          Je    FTscan2
          Mov   Si,offset Fsrch4
          Cmp   byte ptr [Di+3],'R'
          Je    FTscan2
          Mov   Si,offset Fsrch5
          Cmp   byte ptr [Di+4],'U'
          Je    FTscan2
          Mov   Si,offset Fsrch6
          Cmp   byte ptr [Di+1],'R'
          Je    FTscan2
          Mov   Si,offset Fsrch7
          Cmp   byte ptr [Di],'N'
          Je    FTscan2
          Mov   Si,offset Fsrch8
          Jmp   FTscan
H:        Cmp   byte ptr [Di],'O'
          Je    FTscan2
          Mov   Si,offset Hsrch2
          Jmp   FTscan
I:        Cmp   byte ptr [Di+1],'A'
          Je    FTscan2
          Mov   Si,offset Isrch2
          Jmp   FTscan
L:        Cmp   byte ptr [Di+9],'N'
          Je    FTscan2
          Mov   Si,offset Lsrch2
          Cmp   byte ptr [Di+9],'S'
          Je    FTscan2
          Mov   Si,offset Lsrch3
          Cmp   byte ptr [Di+3],'D'
          Je    FTscan2
          Mov   Si,offset Lsrch4
          Cmp   byte ptr [Di],'A'
          Je    FTscan2
          Mov   Si,offset Lsrch5
          Cmp   byte ptr [Di],'M'
          Je    FTscan3
          Mov   Si,offset Lsrch6
          Cmp   byte ptr [Di+2],'D'
          Je    FTscan3
          Mov   Si,offset Lsrch7
          Cmp   byte ptr [Di+2],'T'
          Je    FTscan3
          Mov   Si,offset Lsrch8
          Jmp   FTscan
K:        Cmp   byte ptr [Di+2],'E'
          Je    FTscan3
          Mov   Si,offset Ksrch2
          Jmp   FTscan
M:        Cmp   byte ptr [Di+2],'B'
          Je    FTscan3
          Mov   Si,offset Msrch2
          Cmp   byte ptr [Di+2],'F'
          Je    FTscan3
          Mov   Si,offset Msrch3
          Cmp   byte ptr [Di],'I'
          Je    FTscan3
          Mov   Si,offset Msrch4
          Cmp   byte ptr [Di],'O'
          Je    FTscan3
          Mov   Si,offset Msrch5
          Cmp   byte ptr [Di+2],'L'
          Je    FTscan3
          Mov   Si,offset Msrch6
          Jmp   FTscan

FTscan3:  Jmp   FTscan

N:        Cmp   byte ptr [Di+1],'C'
          Je    FTscan3
          Mov   Si,offset Nsrch2
          Cmp   byte ptr [Di+1],'D'
          Je    FTscan3
          Mov   Si,offset Nsrch3
          Cmp   byte ptr [Di+2],'B'
          Je    FTscan3
          Mov   Si,offset Nsrch4
          Cmp   byte ptr [Di+3],'A'
          Je    FTscan3
          Mov   Si,offset Nsrch5
          Cmp   byte ptr [Di+3],'O'
          Je    FTscan3
          Mov   Si,offset Nsrch6
          Cmp   byte ptr [Di+2],'D'
          Je    FTscan3
          Mov   Si,offset Nsrch7
          Jmp   FTscan
O:        Cmp   byte ptr [Di],'F'
          Je    FTscan3
          Mov   Si,offset Osrch2
          Jmp   FTscan
P:        Cmp   byte ptr [Di],'A'
          Je    FTscan3
          Mov   Si,offset Psrch2
          Cmp   byte ptr [Di+1],'F'
          Je    FTscan3
          Mov   Si,offset Psrch3
          Cmp   byte ptr [Di+1],'N'
          Je    FTscan3
          Mov   Si,offset Psrch4
          Cmp   byte ptr [Di+1],'S'
          Je    FTscan3
          Mov   Si,offset Psrch5
          Cmp   byte ptr [Di+2],'L'
          Je    FTscan3
          Mov   Si,offset Psrch6
          Cmp   byte ptr [Di+3],'E'
          Je    FTscan4
          Mov   Si,offset Psrch7
          Cmp   byte ptr [Di+4],'T'
          Je    FTscan4
          Mov   Si,offset Psrch8
          Jmp   FTscan
Q:        Cmp   byte ptr [Di+1],'F'
          Je    FTscan4
          Mov   Si,offset Qsrch2
          Jmp   FTscan
R:        Cmp   byte ptr [Di+4],'B'
          Je    FTscan4
          Mov   Si,offset Rsrch2
          Cmp   byte ptr [Di+4],'F'
          Je    FTscan4
          Mov   Si,offset Rsrch3
          Cmp   byte ptr [Di],'B'
          Je    FTscan4
          Mov   Si,offset Rsrch4
          Cmp   byte ptr [Di],'C'
          Je    FTscan4
          Mov   Si,offset Rsrch5
          Cmp   byte ptr [Di],'E'
          Je    FTscan4
          Mov   Si,offset Rsrch6
          Jmp   FTscan

FTscan4:  Jmp   FTscan

S:        Cmp   byte ptr [Di],'B'
          Je    FTscan4
          Mov   Si,offset Ssrch2
          Cmp   byte ptr [Di],'C'
          Je    FTscan4
          Mov   Si,offset Ssrch3
          Cmp   byte ptr [Di],'E'
          Je    FTscan4
          Mov   Si,offset Ssrch4
          Cmp   byte ptr [Di],'F'
          Je    FTscan4
          Mov   Si,offset Ssrch5
          Cmp   byte ptr [Di+2],'D'
          Je    FTscan4
          Mov   Si,offset Ssrch6
          Cmp   byte ptr [Di+4],'I'
          Je    FTscan4
          Mov   Si,offset Ssrch7
          Cmp   byte ptr [Di+4],'O'
          Je    FTscan4
          Mov   Si,offset Ssrch8
          Jmp   FTscan
T:        Cmp   byte ptr [Di+4],'I'
          Je    FTscan4
          Mov   Si,offset Tsrch2
          Cmp   byte ptr [Di+4],'E'
          Je    FTscan4
          Mov   Si,offset Tsrch3
          Cmp   byte ptr [Di+3],'U'
          Je    FTscan4
          Mov   Si,offset Tsrch4
          Jmp   FTscan
U:        Cmp   byte ptr [Di+1],'B'
          Je    FTscan4
          Mov   Si,offset Usrch2
          Cmp   byte ptr [Di+1],'F'
          Je    FTscan4
          Mov   Si,offset Usrch3
          Jmp   FTscan

Restart2: Jmp   Restart
FTscan5:  Jmp   FTscan

W:        Cmp   byte ptr [Di],'A'
          Je    FTscan5
          Mov   Si,offset Wsrch2
          Jmp   FTscan
X:        Mov   Ah,[Di]            ;First check for XOFF/XON macros
          Cmp   Ah,'O'
          Jne   X0
          Cmp   byte ptr [Di+1],'F'
          Je    FTscan5
          Mov   Si,offset Xsrch2
          Jmp   FTscan
X0:       Sub   Ah,'0'                 ;Checking for valid colors here...
          Jc    Restart2               ;if less than '0' jump out now
          Cmp   Ah,9                   ;is it less than or equal to 9?
          Jbe   X1                     ;  yes, it's good - go do it now
          Sub   Ah,7                   ;perhaps it's A thru F?
          Cmp   Ah,15                  ;is it greater than 15?
          Ja    Restart2               ;  yes, it's not ours
          Cmp   Ah,10                  ;is it less than 10?
          Jb    Restart2               ;  it wasn't A thru F, get out now
X1:       Mov   Al,[Di+1]
          Sub   Al,'0'
          Jc    Restart2               ;if less than '0' jump out now
          Cmp   Al,9                   ;is it less than or equal to 9?
          Jbe   X2                     ;  yes, it's good - go do it now
          Sub   Al,7                   ;perhaps it's A thru F?
          Cmp   Al,15                  ;is it greater than 15?
          Ja    Restart2               ;  yes, it's not ours
          Cmp   Al,10                  ;is it less than 10?
          Jb    Restart2               ;  it wasn't A thru F, get out now
X2:       Mov   Cl,4
          Shl   Ah,Cl                  ;Move the background to the top 4 bits
          Mov   Cl,Al                  ;Save the foreground color in Cl
          Add   Cl,Ah                  ;Add the background color to Cl
          Mov   Dl,106                 ;Our return code is 106 for COLORS
          Mov   Bx,1                   ;Bx = 1 (the length to skip)
          Jmp   FTfound3  ;jump past FTfound to skip checking for the 2nd '@'
FindToken EndP


;=============================================================================
;   Xmodem CRC Calculations
;=============================================================================

IFNDEF LIB
crctable dw 00000h, 01021h, 02042h, 03063h, 04084h, 050A5h, 060C6h, 070E7h
         dw 08108h, 09129h, 0A14Ah, 0B16Bh, 0C18Ch, 0D1ADh, 0E1CEh, 0F1EFh
         dw 01231h, 00210h, 03273h, 02252h, 052B5h, 04294h, 072F7h, 062D6h
         dw 09339h, 08318h, 0B37Bh, 0A35Ah, 0D3BDh, 0C39Ch, 0F3FFh, 0E3DEh
         dw 02462h, 03443h, 00420h, 01401h, 064E6h, 074C7h, 044A4h, 05485h
         dw 0A56Ah, 0B54Bh, 08528h, 09509h, 0E5EEh, 0F5CFh, 0C5ACh, 0D58Dh
         dw 03653h, 02672h, 01611h, 00630h, 076D7h, 066F6h, 05695h, 046B4h
         dw 0B75Bh, 0A77Ah, 09719h, 08738h, 0F7DFh, 0E7FEh, 0D79Dh, 0C7BCh
         dw 048C4h, 058E5h, 06886h, 078A7h, 00840h, 01861h, 02802h, 03823h
         dw 0C9CCh, 0D9EDh, 0E98Eh, 0F9AFh, 08948h, 09969h, 0A90Ah, 0B92Bh
         dw 05AF5h, 04AD4h, 07AB7h, 06A96h, 01A71h, 00A50h, 03A33h, 02A12h
         dw 0DBFDh, 0CBDCh, 0FBBFh, 0EB9Eh, 09B79h, 08B58h, 0BB3Bh, 0AB1Ah
         dw 06CA6h, 07C87h, 04CE4h, 05CC5h, 02C22h, 03C03h, 00C60h, 01C41h
         dw 0EDAEh, 0FD8Fh, 0CDECh, 0DDCDh, 0AD2Ah, 0BD0Bh, 08D68h, 09D49h
         dw 07E97h, 06EB6h, 05ED5h, 04EF4h, 03E13h, 02E32h, 01E51h, 00E70h
         dw 0FF9Fh, 0EFBEh, 0DFDDh, 0CFFCh, 0BF1Bh, 0AF3Ah, 09F59h, 08F78h
         dw 09188h, 081A9h, 0B1CAh, 0A1EBh, 0D10Ch, 0C12Dh, 0F14Eh, 0E16Fh
         dw 01080h, 000A1h, 030C2h, 020E3h, 05004h, 04025h, 07046h, 06067h
         dw 083B9h, 09398h, 0A3FBh, 0B3DAh, 0C33Dh, 0D31Ch, 0E37Fh, 0F35Eh
         dw 002B1h, 01290h, 022F3h, 032D2h, 04235h, 05214h, 06277h, 07256h
         dw 0B5EAh, 0A5CBh, 095A8h, 08589h, 0F56Eh, 0E54Fh, 0D52Ch, 0C50Dh
         dw 034E2h, 024C3h, 014A0h, 00481h, 07466h, 06447h, 05424h, 04405h
         dw 0A7DBh, 0B7FAh, 08799h, 097B8h, 0E75Fh, 0F77Eh, 0C71Dh, 0D73Ch
         dw 026D3h, 036F2h, 00691h, 016B0h, 06657h, 07676h, 04615h, 05634h
         dw 0D94Ch, 0C96Dh, 0F90Eh, 0E92Fh, 099C8h, 089E9h, 0B98Ah, 0A9ABh
         dw 05844h, 04865h, 07806h, 06827h, 018C0h, 008E1h, 03882h, 028A3h
         dw 0CB7Dh, 0DB5Ch, 0EB3Fh, 0FB1Eh, 08BF9h, 09BD8h, 0ABBBh, 0BB9Ah
         dw 04A75h, 05A54h, 06A37h, 07A16h, 00AF1h, 01AD0h, 02AB3h, 03A92h
         dw 0FD2Eh, 0ED0Fh, 0DD6Ch, 0CD4Dh, 0BDAAh, 0AD8Bh, 09DE8h, 08DC9h
         dw 07C26h, 06C07h, 05C64h, 04C45h, 03CA2h, 02C83h, 01CE0h, 00CC1h
         dw 0EF1Fh, 0FF3Eh, 0CF5Dh, 0DF7Ch, 0AF9Bh, 0BFBAh, 08FD9h, 09FF8h
         dw 06E17h, 07E36h, 04E55h, 05E74h, 02E93h, 03EB2h, 00ED1h, 01EF0h

          Public XMODEM
          ALIGN16
Xmodem    Proc  String:ptr, Len:word
          Uses  Si

if LDATA
          Push  Ds
          Lds   Si,String         ;DS:SI points to string
else
          Mov   Si,String         ;DS:SI points to string
endif

          Cld
          Xor   Ax,Ax             ;Ax will hold the working CRC value
          Xor   Dx,Dx             ;Dl will hold the working checksum value
          Mov   Cx,Len

loop1:    Xor   Bx,Bx
          Mov   Bl,Ah
          Mov   Ah,Al
          Lodsb
          Add   Dl,Al
          Shl   Bx,1
          Xor   Ax,cs:crctable[Bx]
          Loop  loop1

;         OLD WAY
;         ---------------------------------------------------------------
;                                 ;for compatibility with the old Xmodem:
;         Mov   Bx,Ax             ;return CRC value in BX
;         Mov   Ah,Dl             ;return Checksum value in Ah
;
;         NEW WAY
;         ---------------------------------------------------------------
;         do nothing!  just return the CRC in AX and the checksum in DL

if LDATA
          Pop   Ds
endif
          Ret
Xmodem    EndP


;=============================================================================
;   Routine to patch COMTrap so that it is usable with PCBoard
;=============================================================================

;             public comfix
;   ComFix    Proc  Far
;             Push  Bp                ;Save everything we need to...
;             Mov   Bp,Sp
;             Push  Ds
;             Push  Ax
;             Push  Bx
;             Push  Cx
;             Push  Dx
;             Push  Es
;             Push  Si
;             Push  Di
;
;             Sub   Ax,Ax             ;Establish our Extra Segment
;             Mov   Es,Ax
;             Mov   Ax,Es:[4*14h+2]   ;Get the COM interrupt segment
;             Mov   Es,Ax             ;Use this as extra segment
;
;
;             Jmps  CFskip
;   Good      db    'Interrupt Disable Successful!',0AH,0DH,0AH,'$' ;check string from ComTrap
;
;
;   ;
;   ;  Make sure that we're resident....
;   ;
;   CFskip:   Push Ds
;             Push Cs
;             Pop  DS
;             Mov  Si,Offset Good  ;Address of message in this segment (DS:SI points to the message)
;             Mov  Di,103h         ;The message in mem SHOULD be at this offset!
;             Mov  Cx,20h          ;Compare 32 bytes
;             Repe Cmpsb           ;Compare the strings
;             Pop  Ds
;             Jnz  Com_Exit        ;If not equal we're not resident
;
;   ;Yup - we're resident so lets go patch the code.....
;   ;
;             Push  Es               ;Set up data segment
;             Pop   Ds
;             Mov   Di,12Ch
;             Mov   Byte Ptr [Di],55h
;             Mov   Di,0130h
;             Mov   Byte Ptr [Di],10h
;             Mov   Di,0137h
;             Mov   Byte Ptr [Di],80h
;             Mov   Di,0138h
;             Mov   Byte Ptr [Di],0FCh
;             Mov   Di,139h
;             Mov   Byte Ptr [Di],01
;             Mov   Di,142h
;             Mov   Byte Ptr [Di],80h
;             Mov   Di,143h
;             Mov   Byte Ptr [Di],0FCh
;             Mov   Di,144h
;             Mov   Byte Ptr [Di],02
;   Com_Exit:                          ;Restore the stack and exit
;             Pop   Di
;             Pop   Si
;             Pop   Es
;             Pop   Dx
;             Pop   Cx
;             Pop   Bx
;             Pop   Ax
;             Pop   Ds
;             Pop   Bp
;             Ret
;   ComFix    EndP


;=============================================================================
;   Routine to create a "FFFFFFFFEEE" out of a filename.  No dot separator,
;   but padded with spaces to fill in the fields.
;=============================================================================

          public MAKEIDXNAME
          ALIGN16
MakeIdxName Proc  FileNameStr:ptr, IdxStr:ptr
          Uses  Si, Di

if LDATA
          Push  Ds
          Lds   Si,FileNameStr    ;Now DS:SI points to FileNameStr
          Les   Di,IdxStr         ;Now ES:DI points to IdxStr
else
          Mov   Si,FileNameStr    ;Now DS:SI points to FileNameStr
          Mov   Ax,Ds
          Mov   Es,Ax
          Mov   Di,IdxStr         ;Now ES:DI points to IdxStr
endif

          Cld
          Mov   Dx,2E20h          ;Dh = '.',  Dl = ' '
          Mov   Bx,Di
          Add   Bx,8              ;ES:BX points to the start of the extension
          Mov   Cx,Di
          Add   Cx,11

Min1:     Lodsb
          Cmp   Al,Dh
          Je    FoundDot
          Cmp   Al,Dl
          Jle   FoundEnd
          Stosb
          Cmp   Di,Bx
          Jle   Min1
          Jmps  FoundEnd

FoundDot: Xchg  Bx,Cx
          Sub   Cx,Di
          Mov   Al,Dl
          Rep   Stosb
          Mov   Cx,Bx

Min2:     Lodsb
          Cmp   Al,Dl
          Jle   FoundEnd
          Stosb
          Cmp   Di,Cx
          Jl    Min2
          Jmps  MinDone

FoundEnd: Sub   Cx,Di
          Mov   Al,Dl
          Rep   Stosb

MinDone:
if LDATA
          Pop   Ds
endif
          Ret
MakeIdxName EndP



;=============================================================================
;   PROCEDURES: Criteria, WildMatch, WildCard
;=============================================================================
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;   MakeWild formats WildCard (Called by CRITERIA, MATCH)
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

; memset(Criteria,' ',11);  /* not NULL terminated */
; for (p = Name, q = Criteria; *p && q < &Criteria[11]; p++) {
;   switch(*p) {
;     case '*': if (q < &Criteria[8]) {
;                 for (; q < &Criteria[8]; q++)
;                   *q = '?';
;               } else if (q < &Criteria[11]) {
;                 for (; q < &Criteria[11]; q++)
;                   *q = '?';
;               }
;               break;
;     case '.': q = &Criteria[8];
;               break;
;     default : *q = *p;
;               q++;
;               break;
;   }
; }

;when called...
;  DS:SI points to FileName
;  ES:DI points to Criteria

          ALIGN16
MakeWild  Proc  Near
          Cld
          Mov   Bx,Di                  ;save DI in BX
          Mov   Al,' '                 ;Fill Criteria with spaces
          Mov   Cx,11
          Rep   Stosb
          Mov   Di,Bx                  ;Restore DI
          Mov   Dx,Bx
          Add   Bx,8                   ;Bx = &Criteria[8]
          Add   Dx,11                  ;Dx = &Criteria[11]

MWtop:    Cmp   Di,Dx
          Jge   MWexit
          Lodsb
          Cmp   Al,'*'
          Je    MWast
          Cmp   Al,'.'
          Je    MWdot
          Cmp   Al,0
          Je    MWexit
          Cmp   Al,97
          Jl    MWstore
          And   Al,223
MWstore:  Stosb
          Jmps  MWtop

MWast:    Cmp   Di,Bx
          Jge   MWext
          Mov   Cx,Bx
          Sub   Cx,Di
          Mov   Al,'?'
          Rep   Stosb
          Jmps  MWtop

MWext:    Mov   Cx,Dx
          Sub   Cx,Di
          Mov   Al,'?'
          Rep   Stosb
          Jmps  MWexit

MWdot:    Mov   Di,Bx
          Jmps  MWtop

MWexit:   Ret
MakeWild  EndP

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

          public CRITERIA
          ALIGN16
Criteria  Proc  FileNameStr:ptr, CriteriaStr:ptr
          Uses  Ds, Si, Di

if LDATA
          Push  Ds
          Lds   Si,FileNameStr    ;Now DS:SI points to FileNameStr
          Les   Di,CriteriaStr    ;Now ES:DI points to CriteriaStr
else
          Mov   Si,FileNameStr    ;Now DS:SI points to FileNameStr
          Mov   Ax,Ds
          Mov   Es,Ax
          Mov   Di,CriteriaStr    ;Now ES:DI points to CriteriaStr
endif

          Call  MakeWild

if LDATA
          Pop   Ds
endif
          Ret
Criteria  EndP

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

          public WILDMATCH
          ALIGN16
WildMatch Proc  FileNameStr:ptr, CriteriaStr:ptr
          Uses  Ds, Si, Di
          Local FName:byte:11

if LDATA
          Push  Ds
          Lds   Si,FileNameStr    ;Now DS:SI points to FileNameStr
else
          Mov   Si,FileNameStr    ;Now DS:SI points to FileNameStr
endif

          Mov   Ax,Ss
          Mov   Es,Ax
          Lea   Di,FName          ;Now ES:DI points to FName
          Call  MakeWild          ;Make FileName into a wildcard format (FName)

          Lea   Di,FName          ;Now ES:DI points to FName

if LDATA
          Lds   Si,CriteriaStr    ;Now DS:SI points to the CriteriaStr
else
          Mov   Si,CriteriaStr    ;Now DS:SI points to the CriteriaStr
endif

          Mov   Al,1         ;default to TRUE which means a MATCH was found
          Mov   Cx,11        ;scan all eleven formatted positions
          Mov   Bl,'?'       ;watch out for wildcards
                             ;
top:      Rep   Cmpsb        ;
          Je    done         ;if it's equal get out of here now
          Cmp   [Si-1],Bl    ;
          Je    top          ;if it was a wildcard go compare some more
                             ;
nomatch:  Xor   Ax,Ax        ;return FALSE - no match found
done:
if LDATA
          Pop   Ds
endif
          Ret
WildMatch EndP
ENDIF

IFNDEF LIB
           public CHECKSTACK
checkstack Proc
           Cmp Sp,[_StackCheck]
           Jb  savevalue
           Cmp Sp,[__stklen]
           Ja  overflow
           Ret

savevalue: Mov [_StackCheck],Sp
           Ret

overflow:  Mov [_StackCheck],0
           Ret
checkstack EndP
ENDIF


if LPROG
  cutil  ends
else
  CSegEnd@
endif

          End
