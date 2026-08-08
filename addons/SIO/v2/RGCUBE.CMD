/*
    ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
    บ                    Config Update/Batch Editing                        บ
    บ                                                                       บ
    บ  Batch update of CONFIG.SYS-like files. CUBE modifies a Target ASCII  บ
    บ  file, given a set of commands in a Procedure file.                   บ
    บ                                                                       บ
    บ  04/06/93: V2.6 - Add 'procedure read from QUEUE' (from Steve Farrell)บ
    บ  03/06/93: V2.5 - Add IF/IFNOT to xLINE cmds (generalize N. Marks req)บ
    บ                   Correct ADDSTRING BEFORE option  (Neil Marks)       บ
    บ  07/04/93: V2.4 - Corrected ADDBOTTOM/ADDTOP in ADDSTRING (Per Hertz) บ
    บ                   Address cmd + CHECK Option + new exit rtne          บ
    บ  21/01/93: V2.3 - Added user defined string delimiter in CUBE cmds    บ
    บ  21/12/92: V2.2 - Added conditionnal command processing (WHEN)        บ
    บ  26/11/92: V2.1 - RS() for DL, DS (desinstallation case)              บ
    บ                   New LINEID command (strip leading chars)            บ
    บ                   ADDTOP,ADDBOTTOM for AS (W. Pachl requirement)      บ
    บ                   Fix Whereis (Walter Pachl).                         บ
    บ                   Exit with SaveFile return code (Walter Pachl)       บ
    บ  18/11/92: V2.0 - Changes with environment variable substitution (RS) บ
    บ                       (AS, RS, AL & AL now all have same RS() option) บ
    บ                   Logging of all changes made to Target File          บ
    บ                   Adapt/Include some of Walter Pachl's enhancements:  บ
    บ                       Single CUBE command on command line             บ
    บ                       Add'l string substitution at command line level บ
    บ                       PAUSE option (debugging purposes)               บ
    บ  05/11/92: V1.5 - AL with pre substitution                            บ
    บ  03/11/92: V1.4 - AS with substitution ; fix RS recursion.            บ
    บ  02/11/92: V1.3 - Bug fixes & cmds abbrev, thanks to Walter Pachl.    บ
    บ                   Target Backup & lineid no more limited to col 1.    บ
    บ  30/10/92: V1.2 - Added env variable substitution + version #         บ
    บ  31/08/92: V1.1 - Bug fix                                             บ
    บ  21/07/92: V1.0 - Initial revision                                    บ
    บ  Didier LAFON - LAFON at CBEPROFS                                     บ
    ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ */

'@echo off'
version = '2.6'
pf = 0
if left(arg(1),1) = '{' then parse arg '{'PFile'}' TFile Bkup . '(' Opt
else do
  parse arg PFile TFile Bkup . '(' Opt
  pf = 1
end
parse upper source source                          /* who am I ?              */

if pf then do
  if PFile = '' then call Exit 0 'no procedure !'    /* no or missing PFile     */
  if Pfile <> 'QUEUE' then if exists(PFile)='' then call Exit 0 PFile 'not found'
end
if TFile = ''  then call Exit 0 'no target !'      /* no or missing Tfile     */
if exists(TFile)='' then call Exit 0 TFile 'not found'

pause=(wordpos('PAUSE',translate(Opt))>0)          /* Pause mode ?            */
chkmd=(wordpos('CHECK',translate(Opt))>0)          /* Check mode ?            */
ap = wordpos('MAKE',translate(Opt))                /* make  specified ?       */
if ap>0 then Make=translate(word(Opt,ap+1))        /* ...when                 */
        else Make = '*'                            /* default make = all      */
MWhen='*'                                          /* Default when = all      */
dlm = wordpos('DLM',translate(Opt))                /* New delimiter specified?*/
if dlm>0 then _d_=left(word(Opt,dlm+1),1)          /*   yes use it            */
         else _d_ = '"'                            /* else use default        */
OnErr = 'STOP'                                     /* Default OnError setting */
LStrip= ''                                         /* No lineid strip         */
CaseM = 'I'                                        /* String compare default  */
call get_cmdrs(Opt)
NumberOfChanges = 0

say arg
say 'CUBE' version 'applying' PFile 'to' TFile 'on' date() time()
if Bkup <> "" then do
   address cmd 'copy' Tfile Bkup '1>nul 2>nul'
   if rc = 0 then say Tfile 'backup is:' Bkup
end

Proc. = ''
if pf then do
   If PFile = 'QUEUE' then Do
     i = 1
     Do Queued()
       Parse Pull procline
       proc.i = proc.i || upkw(procline)
       if right(Proc.i,1) = ','                       /*   continuation char ?   */
         then proc.i=left(proc.i,length(proc.i)-1)' ' /*     yes: blank it out   */
         else  i = i + 1                              /*     no: new Proc line   */
     end
     Proc.0 = i-1                                     /* Proc.0 = # of lines     */
     if Proc.0 <= 0 then call Exit 0 PFile 'empty'
  end
  Else do
     i = 1 ;                                          /* current Proc line: null */
     do while lines(PFile)                            /* for all PFile's lines   */
       Proc.i = Proc.i || upkw(linein(PFile))         /*   concat to Proc line   */
       if right(Proc.i,1) = ','                       /*   continuation char ?   */
         then proc.i=left(proc.i,length(proc.i)-1)' ' /*     yes: blank it out   */
         else  i = i + 1                              /*     no: new Proc line   */
     end
     Proc.0 = i-1                                     /* Proc.0 = # of lines     */
     call close PFile
     if Proc.0 <= 0 then call Exit 0 PFile 'empty'
  End
end
else do
  Proc.0 = 1
  Proc.1 = upkw(Pfile)
end

i = 0
do while lines(TFile)                              /* for all TFile's lines   */
  i = i + 1                                        /*   get line in           */
  Target.i = linein(TFile)                         /*   Target. stem          */
end
Target.0 = i                                       /* Target.0 = # of lines   */
call close Tfile

/*  ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
    บThe real thing: go thru procedure file, interpret/execute its commands บ
    บsequentially.                                                          บ
    ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ */

p = 0                                              /* Proc lines index        */
do while p <= Proc.0                               /* for all PFile's lines   */
  p = p + 1                                        /*   index next line       */
  if Proc.p = '' then iterate                      /*   ignore null lines     */
  parse var Proc.p Verb Parms                      /*   Isolate command verb  */
  say ''
  say '>>>' Proc.p
  Verb = translate(Verb)
  Select                                           /*   Process verb          */
    When left(Verb,1) = '*'   then iterate
    When left(Verb,2) = '--'  then iterate
               /* commands that always get executed  */
    When Verb = 'WHEN'        then call APPLYWHEN
    When Verb = 'ONERROR'     then call ONERROR
    When Verb = 'CASE'        then call CASE
    When Verb = 'LINEID'      then call SLINEID
    When wordpos(Make,MWhen)=0 then iterate
               /* commands executed when WHEN/MAKE match */
    When Verb = 'REPLINE'     | verb = 'RL'  then call REPLINE
    When Verb = 'ADDLINE'     | verb = 'AL'  then call ADDLINE
    When Verb = 'ADDSTRING'   | verb = 'AS'  then call ADDSTRING
    When Verb = 'DELSTRING'   | verb = 'DS'  then call DELSTRING
    When Verb = 'REPSTRING'   | verb = 'RS'  then call REPSTRING
    When Verb = 'COMMENTLINE' | verb = 'CL'  then call COMMENTL
    When Verb = 'DELLINE'     | verb = 'DL'  then call DELLINE
    Otherwise rc=OnErrorDo(p,"Don't know what to do")
  end
  if pause then Pull .
end
if chkmd = 0 then call exit 1 source 'ended.'      /* It's OVER !! and OK !!  */
if chkmd = 1 then call exit 2 source 'ended.'      /*                         */

/* ีอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออธ
   ณ  Error report and action (based on Onerr setting, from ONERROR cmd)   ฦ
   ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ  */
OnErrorDo:
  parse arg line,msg
  say PFile', line' line':' msg
  if OnErr = 'STOP' then call Exit 0 source 'stopped.'
                    else return 0

/* ีอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออธ
   ณ  Searches All or First or Last lines in Target starting with string   ฦ
   ณ  Returns the line number(s) found.                                    ฦ
   ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ  */
Whereis: procedure expose Target. CaseM LStrip
 parse arg string,direction,mode
 if wordpos(direction,'F A')>0 then do; de=1; a=Target.0; par=1; end
                               else do; de=Target.0; a=1; par=-1; end
 stringlength=length(string); ret = ''
 do i = de to a by par
   If CaseM = 'S' then do; T = Target.i ; S = string ; end
      else do; T = translate(Target.i) ; S = translate(string) ; end
   If length(LStrip) = 1 then T = strip(T,'L',Lstrip)
   if mode=1 then do
     if left(T,stringlength)=S then do
       ret = ret i
       if direction \= 'A' then leave
     end
   end
   else do
     if pos(S,T) > 0 then do
       ret = ret i
       if direction \= 'A' then leave
     end
   end
 end
 return ret

/* ีอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออธ
   ณ  Searches All or First or Last lines in Target starting with string   ฦ
   ณ  Returns the line number(s) found.                                    ฦ
   ณ                                                                       ฦ
   ณ  Hack of Whereis by Ray Gwinn so we will not comment out lines that   ฦ
   ณ  are ALREADY comment lines.                                           ฦ
   ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ  */
WhereisHack: procedure expose Target. CaseM LStrip
 parse arg string,direction,mode,CommentString
 if wordpos(direction,'F A')>0 then do; de=1; a=Target.0; par=1; end
                               else do; de=Target.0; a=1; par=-1; end
 stringlength=length(string); CommentStringlength=length(CommentString); ret = ''
 do i = de to a by par
   If CaseM = 'S' then do; T = Target.i ; S = string ; C = CommentString ; end
      else do; T = translate(Target.i) ; S = translate(string) ; C = translate(CommentString) ; end
   if left(T,CommentStringlength) \= C then do
     If length(LStrip) = 1 then T = strip(T,'L',Lstrip)
     if mode=1 then do
       if left(T,stringlength)=S then do
         ret = ret i
         if direction \= 'A' then leave
       end
     end
     else do
       if pos(S,T) > 0 then do
         ret = ret i
         if direction \= 'A' then leave
       end
     end
   end
 end
 return ret

/* ีอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออธ
   ณ  Update Target file from Target. stem. Remove '       ' lines ฦ
   ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ  */
SaveFile:
 address cmd 'erase' TFile
 src = rc
 if src = 0 then do
   do i = 1 to Target.0
     if Target.i = '       ' then iterate
     rc=lineout(TFile,Target.i)
   end
   call close Tfile
 end
 return src

/* ีอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออธ
   ณ  Insert a line in Target file (stem) after line number i.             ฦ
   ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ  */
Insert: procedure  expose Target. NumberOfChanges
 parse arg i string
 if i = Target.0 then k = Target.0 + 1
 else do
   do j = Target.0 to i+1 by -1
     k = j + 1
     Target.k = Target.j
   end
   k = i + 1
 end
 Target.k = string
 Target.0 = Target.0 + 1
 say 'Inserted line' k ': "'Target.k'"'
 NumberOfChanges = NumberOfChanges + 1
 return

/* ีอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออธ
   ณ  returns a procedure command line  with all strings uppercased, exceptฦ
   ณ  doubled-quoted strings.                                              ฦ
   ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ  */
upkw: procedure expose vn. vv. _d_
  parse arg sentence
  sentence = strip(sentence)
  phrase = ""
  do forever
    if sentence = '' then leave
    if left(word(sentence,1),1) = _d_ then do
       parse var sentence (_d_) y (_d_) sentence
       phrase = phrase _d_ || y || _d_
    end
    else do
       parse var sentence y sentence
       phrase = phrase translate(y)
    end
  end
  return phrase

/* ีอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออธ
   ณ  apply command line-specified substitutions within a string           ฦ
   ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ  */
Cmdrs: procedure expose vn. vv.
 parse arg y
 do i = 1 to vn.0
   out = ''
   do forever
     if pos(vn.i,y) > 0 then do
        parse var y x (vn.i) y
        out = out || x || vv.i
     end
     else leave
   end
 y = out || y
 end
 return y

/* ีอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออธ
   ณ  apply env. variables substitutions to STRING if req. in OPTION.      ฦ
   ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ  */
Envrs: procedure
  Parse arg String,Option
  out = ''
  parse var Option x 'RS('c')' .
  if length(c) = 1 then do
     do forever
        parse var String x (c) name (c) String
        if name = "" then leave
        out = out || x || value(name,,'OS2ENVIRONMENT')
     end
     String = out || x
  end
  return String

/* ีอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออธ
   ณ All that must be done to quit and more: say msg, save Target file if  ฦ
   ณ necessary (type=1).                                                   ฦ
   ิอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออพ  */
Exit:
 parse arg type msg
 src=0
 Select
   When type=1 then do
        src = SaveFile()
        if src <> 0 then msg = msg 'Error writing' TFile
                    else msg = msg NumberOfChanges 'changes applied'
        end
   When type=2 then do
        src = NumberOfChanges
        msg = msg NumberOfChanges 'changes applied'
        end
   Otherwise nop
 End
 say msg
 Exit src

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ ONERROR [CONTINUE] [STOP] : what to do on syntax errors ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู           */
ONERROR:
 if wordpos(translate(Parms),'CONTINUE STOP')>0 then OnErr = translate(Parms)
    else rc=OnErrorDo(p,'On Error what ?')
 return

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ WHEN     ... wordlist of when codes ...                 ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู           */
APPLYWHEN:
 MWhen = strip(translate(Parms),'B')
 if MWhen = '*' then MWhen = Make
 return

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ CASE [SENSITIVE] [IGNORE] : string compare mode         ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู           */
CASE:
 if wordpos(translate(Parms),'SENSITIVE IGNORE')>0 then CaseM = translate(left(Parms,1))
    else rc=OnErrorDo(p,'Case what ?')
 return

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ LINEID [NOSTRIP] [STRIP "x"]                            ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู           */
SLINEID:
 Select
   When word(translate(Parms),1) = 'NOSTRIP' then Lstrip = ""
   When word(translate(Parms),1) = 'STRIP' then do
      Parse var Parms 'STRIP' (_d_) ww (_d_)
      if length(ww) <> 1 then rc=OnErrorDo(p,'Strip leading what ?')
                         else Lstrip = ww
      end
   Otherwise rc=OnErrorDo(p,'Lineid what ?')
 end
 return

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ REPLINE lineid WITH replacement [( options]             ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู           */
REPLINE:
  parse var Parms (_d_) Lineid (_d_)  'WITH' (_d_) With (_d_)  '(' Opt
  if Lineid = '' then do                           /* No line identifier      */
    rc=OnErrorDo(p,'Replace what line ?')
    return
  end
  if With = '' then do                             /* No replacement string   */
    rc=OnErrorDo(p,'Replace line with ?')          /*   process error         */
    return                                         /*   ignore command        */
  end
  With = Cmdrs(With)                               /* cmd substitution        */
  With = Envrs(With,Opt)                           /* env substitution if req */
  dir = Searchdir(opt)                             /* What target lines ?     */
  mod = Lidmod(opt)                                /* floating line id ?      */
  select                                           /* What if no target lines?*/
    when wordpos('ADDTOP',Opt)>0 then after=0      /* add after line 0        */
    when wordpos('ADDBOTTOM',Opt)>0 then after=Target.0 /* add after last line     */
    when wordpos('DONTADD',Opt)>0 then after=-1    /* don't add               */
    otherwise after=-1                             /* don't add is the default*/
  end
  if ififnot() then return                         /* Process only when       */
  where = Whereis(Lineid,dir,mod)                  /* Get target lines numbers*/
  if where \= '' then do                           /* if target(s) found      */
    do until where = ''                            /*   process all targets   */
      parse var where w where                      /*     1 at a time         */
      was = Target.w                               /* save old value for log  */
      Target.w = With                              /*     target = replacmnt. */
      call logrep w,was,Target.w                   /*     log action          */
      if dir \= 'A' then leave                     /*     quit if not ALL     */
    end
  end
  else if after>-1 then call insert after With     /* if no target, try add   */
  return

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ ADDLINE     line  [( options]                             ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู         */
ADDLINE:
  parse var Parms (_d_) Line (_d_)  '(' Opt
  if Line = '' then do                             /* No line identifier      */
    rc=OnErrorDo(p,'Add what line ?')              /*    process error        */
    return                                         /*    ignore command       */
  end
  select                                           /* When to add ?           */
    when wordpos('IFNEW',Opt)>0 then always=0      /*   if not already there  */
    when wordpos('ALWAYS',Opt)>0 then always=1     /*   even if already there */
    otherwise always=0                             /*   IFNEW is the default  */
  end
  Line = Cmdrs(Line)                               /* cmd substitution        */
  Line = envrs(Line,Opt)                           /* env substitution if req */
  mod = Lidmod(opt)                                /* floating line id ?      */
  exist = Whereis(Line,'F',mod)                    /* If this line exists and */
  if exist \= '' & always = 0 then return          /* IFNEW , don't add !     */
  if ififnot() then return

  select                                           /* Where to add ?          */
    when wordpos('AFTER',Opt)>0 then do;           /* 1) After a given line   */
      parse var Opt 'AFTER' (_d_) astr (_d_)  .    /*    line identifier ?    */
      if astr = '' then after = Target.0           /*    no id = add bottom   */
      else after = Whereis(astr,'F',mod)           /*    else get # of 1st    */
      parse var after after .                      /*    line with this id.   */
      if after='' then do                          /*    no match found       */
        if wordpos('ONLY',Opt)>0 then after=-1     /*      if ONLY, don't add */
           else after=Target.0                     /*      else add bottom    */
      end
    end
    when wordpos('BEFORE',Opt)>0 then do;          /* 2) Before a given line  */
      parse var Opt 'BEFORE' (_d_) bstr (_d_)  .   /*    line identifier ?    */
      if bstr = '' then after = 0                  /*    no id = add top      */
      else after = Whereis(bstr,'F',mod)           /*    else get # of 1st    */
      parse var after after .                      /*    line with this id.   */
      if after ='' then do                         /*    no match found       */
        if wordpos('ONLY',Opt)>0 then after=-1     /*      if ONLY don't add  */
           else after=0                            /*      else add top       */
      end
      else after=max(0,after-1)                    /*    match found          */
    end
    otherwise after=Target.0                       /* 3) default = add bottom */
  end
  if after \= -1 then call insert after Line       /* add the line            */
  return

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ ADDSTRING string IN lineid [(Options]                     ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู         */
ADDSTRING:
  parse var Parms (_d_) With (_d_)  'IN' (_d_) Lineid (_d_)   '(' Opt
  if Lineid = '' then do                           /* No line identifier      */
    rc=OnErrorDo(p,'Add string where ?')           /*    process error        */
    return                                         /*    ignore command       */
  end
  if With = '' then do                             /* No string to add        */
    rc=OnErrorDo(p,'Add what string ?')            /*    process error        */
    return                                         /*    ignore command       */
  end
  With = Cmdrs(With)                               /* cmd substitution        */
  With = Envrs(With,Opt)                           /* env substitution if req */
  dir=Searchdir(opt)                               /* Which target line ?     */
  mod=Lidmod(opt)                                  /* floating line id ?      */
  where = Whereis(Lineid,dir,mod)                  /* Select target(s)        */
  if where \= '' then do                           /*   if target found       */
    do until where = ''                            /*     process target(s)   */
      parse var where w where                      /*     1 at a time         */
      if CaseM = 'S' then do
         Tar= Target.w; Wi = With; end             /*     string compare mode */
      else do
         Tar = translate(Target.w); Wi=translate(With); end  /*     string compare mode */
      if pos(Wi,Tar) > 0 & ,                       /* String already there &  */
         wordpos('ALWAYS',Opt) = 0 then leave      /* ALWAYS not specified.   */

      select                                       /* Where to add ?          */
        when wordpos('AFTER',Opt)>0 then do        /* 1) After a given string */
          astr=''                                  /*    defaulted to null    */
          parse var Opt 'AFTER' (_d_) astr (_d_)  .  /*    what is this string  */
          If CaseM = 'I' then astr=translate(astr)
          was = Target.w                           /*    save for logging     */
          if astr = '' | pos(astr,Tar)=0           /* if no string or no match*/
             then Target.w = Target.w || With      /*    add at end of target */
          else do
             parse var Tar xx (astr) rest          /* insert string after     */
             Target.w = xx || astr || With || rest /*   specified string      */
          end
          call logrep w,was,Target.w               /*   log action            */
        end
        when wordpos('BEFORE',Opt)>0 then do       /* 2) Before a given string*/
          bstr=''                                  /*    defaulted to null    */
          parse var Opt 'BEFORE' (_d_) bstr (_d_)  . /*    what is this string  */
          If CaseM = 'I' then bstr=translate(bstr)
          was = Target.w                           /*    save for logging     */
          if bstr = '' | pos(bstr,Tar)=0           /* if no string or no match*/
          then do                                  /* add at beginning        */
            If CaseM = 'I' then Lid=translate(Lineid)
            Parse var Tar (Lid) rest               /* but after identifier    */
            Target.w = Lineid || With || rest      /*                         */
          end
          else do
             parse var Tar xx (bstr) rest          /* insert string before    */
             Target.w = xx || With || bstr || rest /*   specified string      */
          end
          call logrep w,was,Target.w               /*   log action            */
        end
        otherwise nop
      end
      if dir \= 'A' then leave                     /* leave if not ALL targets*/
    end
  end
  else do                                          /* no target : add line ?  */
    if wordpos('ADDTOP',Opt)>0 then call insert 0 Lineid || With
    if wordpos('ADDBOTTOM',Opt)>0 then call insert Target.0 Lineid || With
  end
  return

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ DELSTRING string [IN lineid] [(Options]                   ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู           */
DELSTRING:
  parse var Parms (_d_) What (_d_)  'IN' (_d_) Lineid (_d_)  '(' Opt
  if Lineid = '' then parse var Parms (_d_) What (_d_)  '(' Opt
  if What = '' then do                             /* No string to del        */
    rc=OnErrorDo(p,'Delete what string ?')         /*    process error        */
    return                                         /*    ignore command       */
  end
  What = Cmdrs(What)                               /* cmd substitution        */
  What = envrs(What,Opt)                           /* env substitution if req */
  dir=Searchdir(opt)                               /* Which target line ?     */
  mod=Lidmod(opt)                                  /* floating line id ?      */
  where = Whereis(Lineid,dir,mod)                  /* Select target(s)        */
  if where \= '' then do                           /*   if target found       */
    do until where = ''                            /*     process target(s)   */
      parse var where w where                      /*     1 at a time         */
      do forever                                   /* for all occurences      */
        if CaseM = 'S' then do
           Tar = Target.w; Wh=What; end            /*     string compare mode */
        else do
           Tar = translate(Target.w); Wh=translate(What); end  /*     string compare mode */
        if pos(Wh,Tar) > 0 then do                 /*     String is there     */
           was = Target.w                          /*     save for logging    */
           parse var Tar xx (Wh) rest              /*     isolate string      */
           Target.w = xx || rest                   /*     delete string       */
           call logrep w,was,Target.w              /*     log action          */
        end
        else leave
      end
      if dir \= 'A' then leave                     /* leave if not ALL targets*/
    end
  end
  return

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ REPSTRING ostring [WITH nstring] [IN lineid] [(Options]   ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู           */
REPSTRING:
  parse var Parms (_d_) Ostr (_d_)  'WITH' (_d_) With (_d_)  'IN' (_d_) Lineid (_d_)  '(' Opt
  if Ostr = '' then do                             /* No old string specif.   */
    rc=OnErrorDo(p,'Replace what string ?')        /*    process error        */
    return                                         /*    ignore command       */
  end
  if With = '' then parse var Parms (_d_) Ostr (_d_)  'IN' (_d_) Lineid (_d_)  '(' Opt
  if Lineid = '' then parse var Parms (_d_) Ostr (_d_)  '(' Opt
  if With = '' then With = Ostr                    /* No rep string specif.   */
  With = Cmdrs(With)                               /* cmd substitution        */
  With = envrs(With,Opt)                           /* env substitution if req.*/
  dir=Searchdir(opt)                               /* Which target line ?     */
  mod=Lidmod(opt)                                  /* floating line id ?      */
  where = Whereis(Lineid,dir,mod)                  /* Select target(s)        */
  do while where \= ''                             /*   if target found       */
      parse var where w where                      /*     1 at a time         */
      was = Target.w
      newtar = ''
      do forever                                   /* for all occurences      */
        if CaseM = 'S' then do
           Tar = Target.w; Os=Ostr; end            /*     string compare mode */
        else do
           Tar = translate(Target.w); Os=translate(Ostr); end  /*     string compare mode */
        if pos(Os,Tar) > 0 then do                 /*     String is there     */
           parse var Tar xx (Os) rest              /*     isolate string      */
           newtar = newtar || xx || With           /*     replace occurrence  */
           Target.w = rest                         /*     next                */
        end
        else leave
      end
      Target.w = newtar || target.w
      call logrep w,was,Target.w                   /* log action              */
      if dir \= 'A' then leave                     /* leave if not ALL targets*/
  end
  return

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ COMMENTLINE lineid WITH type [(options ]                ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู           */
COMMENTL:
  parse var Parms (_d_) Lineid (_d_)  'WITH' (_d_) cmnt (_d_)  '(' Opt
  if Lineid = '' then do                           /* No identifier           */
    rc=OnErrorDo(p,'Comment what ?')               /*    process error        */
    return                                         /*    ignore command       */
  end
  if cmnt = '' then do                             /* No comment string       */
    rc=OnErrorDo(p,'Comment how ?')                /*    process error        */
    return                                         /*    ignore command       */
  end
  dir=Searchdir(opt)                               /* Which target lines ?    */
  mod=Lidmod(opt)                                  /* floating line id ?      */
  if ififnot() then return

/* The following line altered by RG so as not to comment a commented line */
/*where= whereis(Lineid,dir,mod)*/
  where= WhereisHack(Lineid,dir,mod,cmnt)          /* get target lines #s     */
  if where \= '' then do                           /* if match(es) found      */
    do until where = ''                            /*   process target(s)     */
      parse var where w where                      /*   1 at a time           */
      was = Target.w                               /*  save for logging       */
      Target.w = cmnt Target.w                     /*   comment target        */
      call logrep w,was,Target.w                   /*   log action            */
      if dir \= 'A' then leave                     /* leave if not ALL targets*/
    end
  end
  return

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ DELLINE lineid [(options ]                              ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู           */
DELLINE:
  parse var Parms (_d_) Lineid (_d_)   '(' Opt
  if Lineid = '' then do                           /* No identifier           */
    rc=OnErrorDo(p,'Delete what line ?')           /*    process error        */
    return                                         /*    ignore command       */
  end
  Lineid = Cmdrs(Lineid)                           /* cmd substitution        */
  Lineid = envrs(Lineid,Opt)                       /* env substitution if req */
  dir=Searchdir(opt)                               /* Which target line(s) ?  */
  mod=Lidmod(opt)                                  /* floating line id ?      */
  if ififnot() then return
  where= whereis(Lineid,dir,mod)                   /* Get target lin(s) #s    */
  if where \= '' then do                           /* if match(es) found      */
    do until where = ''                            /*    process all targets  */
      parse var where w where                      /*    one at a time        */
      say 'Deleted line' w
      say '  was:' Target.w
      NumberOfChanges = NumberOfChanges + 1
      Target.w = '       '                 /*    mark for delete      */
      if dir \= 'A' then leave                     /* leave if not ALL targets*/
    end
  end
  return

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ SEARCHDIR: Direction for line search in Target File     ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู           */
Searchdir: procedure
  select                                           /* What target lines ?     */
    when wordpos('LAST',arg(1))>0 then dir='L'     /* set reverse search      */
    when wordpos('FIRST',arg(1))>0 then dir='F'    /* set forward search      */
    otherwise dir='A'                              /* default is all lines    */
  end
  return dir

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ LIDMOD: Identify line at 1st col or anywhere in line    ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู           */
Lidmod: procedure
  if wordpos('*ID',arg(1))>0 then return(0)
                             else return(1)


/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ Stream functions (close & exists)                       ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู           */
close:
  return stream(arg(1),'C','CLOSE')

exists:
  return stream(arg(1),'C','QUERY EXISTS')

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ get substitution variables and their values in cmdline. ณ
        ณ ( RS(#aaa#=aaa #bbb#=bbb)                               ณ
        ณOutput: vn.i = number and names of subst. variables      ณ
        ณ        vv.i = values of these variables                 ณ
        ณAdapted from Walter Pachl's                              ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู           */
get_cmdrs: procedure expose vn. vv.
  parse arg Opt
  vn.0 = 0
  p = pos('RS(',translate(Opt))
  if p > 0 then do
    rs = substr(Opt,p+3)
    Parse var rs rs ')'
    Do i=1 By 1 While rs<>''
      rs=strip(rs,'L')
      Parse Var rs vn '=' vv rs
      if left(vn,1) = right(vn,1) then do
        vn.i = vn
        vv.i = vv
      end
      else call Exit 0 'Invalid substitution variables ('vn vv')'
    End
    vn.0=i-1
  end
  Return

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ Log a change in a target line                           ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู           */
logrep: procedure expose NumberOfChanges
parse arg ln,old,new
if old<>new then do
  say 'Changed line' ln
  say '  old: "'old'"'
  say '  new: "'new'"'
  NumberOfChanges = NumberOfChanges + 1
end

return

/*      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
        ณ Process IF/IFNOT logic for Lines commands               ณ
        ณ    returns 1 if IF/IFNOT condition is false !!          ณ
        ณ    returns 0 otherwise                                  ณ
        ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู           */
ififnot:
if wordpos('IFNOT',Opt)>0 Then Do                /* Process only when       */
  parse var Opt 'IFNOT' (_d_) istr (_d_) .       /* another line doesn't    */
  iexist = Whereis(istr,'F',mod)                 /* exists                  */
  if iexist \= '' then return(1)                 /*                         */
end                                              /*                         */
if wordpos('IF',Opt)>0 Then Do                   /* Process only when       */
  parse var Opt 'IF' (_d_) istr (_d_) .          /* another line exists     */
  iexist = Whereis(istr,'F',mod)                 /*                         */
  if iexist  = '' then return(1)                 /*                         */
end                                              /*                         */
return(0)

