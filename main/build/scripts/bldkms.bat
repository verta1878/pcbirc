@echo off
rem ============================================================
rem  bldkms.bat - build pcbkms (microsoft c 7.0)
rem  pwa153 toolkit: 119 modules x 4 memory models
rem  sources: toolkit\pwa153 (95) + pcb153 (24, incl 5 asm)
rem  objects -> out\lib\pwa153\msc70\obj\<model>\  (clark convention)
rem  generated from main/build/manifests/pwa153-complete-118.json
rem
rem  host: msc 7.0 cl "3216" dos passes need a 32-bit dpmi host
rem  (386max 8.03, devtools/386max-803.7z). or run the os/2 "1616"
rem  compiler under os/2 (no dpmi). set cc below. asm uses tasm.
rem  see msc70/readme.md and main/build/sdk-build-status.md.
rem ============================================================
echo.
echo building pcbkms (microsoft c 7.0)
echo pwa153 toolkit - 119 modules x 4 models (small/medium/compact/large)
echo.
rem --- host compiler driver ---
rem  dos route (needs 386max dpmi):  set cc=c:\msc70\bin\cl.exe -Dlib
rem  os/2 route (no dpmi):           set cc=c:\msc70\os2\binp\cl.exe
set cc=c:\msc70\bin\cl.exe -Dlib
set include=c:\msc70\include
set lib=c:\msc70\lib
set tkinc=c:\toolkit\pwa153\h
echo.
echo === memory model: small ===
if not exist c:\out\lib\pwa153\msc70\obj\small md c:\out\lib\pwa153\msc70\obj\small
cd \toolkit\pwa153\source\pcb
echo   [  1/119] addbacks
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\addbacks.obj addbacks.c
cd \toolkit\pwa153\source\misc
echo   [  2/119] addchar
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\addchar.obj addchar.c
cd \toolkit\pwa153\source\screen
echo   [  3/119] ansi
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\ansi.obj ansi.c
cd \toolkit\pwa153\source\misc
echo   [  4/119] ascii
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\ascii.obj ascii.c
cd \pcb153\source\asm
echo   [  5/119] async (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__s__ async.asm,c:\out\lib\pwa153\msc70\obj\small\async.obj
cd \toolkit\pwa153\source\toolkit
echo   [  6/119] atclose
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\atclose.obj atclose.c
cd \toolkit\pwa153\source\misc
echo   [  7/119] bd_long
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\bd_long.obj bd_long.c
cd \pcb153\source\asm
echo   [  8/119] bgkey (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__s__ bgkey.asm,c:\out\lib\pwa153\msc70\obj\small\bgkey.obj
cd \toolkit\pwa153\source\misc
echo   [  9/119] bs_long
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\bs_long.obj bs_long.c
cd \toolkit\pwa153\source\dos
echo   [ 10/119] chkappen
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\chkappen.obj chkappen.c
echo   [ 11/119] chkcreat
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\chkcreat.obj chkcreat.c
echo   [ 12/119] chklock
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\chklock.obj chklock.c
echo   [ 13/119] chkopen
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\chkopen.obj chkopen.c
echo   [ 14/119] chkread
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\chkread.obj chkread.c
echo   [ 15/119] chkwrite
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\chkwrite.obj chkwrite.c
cd \toolkit\pwa153\source\screen
echo   [ 16/119] cls
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\cls.obj cls.c
cd \toolkit\pwa153\source\toolkit
echo   [ 17/119] cnames
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\cnames.obj cnames.c
cd \toolkit\pwa153\source\country
echo   [ 18/119] comma
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\comma.obj comma.c
echo   [ 19/119] country
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\country.obj country.c
cd \toolkit\pwa153\source\misc
echo   [ 20/119] ctod
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\ctod.obj ctod.c
cd \toolkit\pwa153\source\screen
echo   [ 21/119] cursor
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\cursor.obj cursor.c
cd \toolkit\pwa153\source\toolkit
echo   [ 22/119] custhelp
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\custhelp.obj custhelp.c
cd \pcb153\source\asm
echo   [ 23/119] cutil (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__s__ cutil.asm,c:\out\lib\pwa153\msc70\obj\small\cutil.obj
cd \toolkit\pwa153\source\pcb
echo   [ 24/119] datafil2
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\datafil2.obj datafil2.c
cd \toolkit\pwa153\source\country
echo   [ 25/119] date
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\date.obj date.c
cd \toolkit\pwa153\source\screen
echo   [ 26/119] datestr
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\datestr.obj datestr.c
echo   [ 27/119] delay
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\delay.obj delay.c
cd \toolkit\pwa153\source\dos
echo   [ 28/119] dosappen
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\dosappen.obj dosappen.c
echo   [ 29/119] dosclose
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\dosclose.obj dosclose.c
echo   [ 30/119] doscomit
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\doscomit.obj doscomit.c
echo   [ 31/119] doscreat
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\doscreat.obj doscreat.c
echo   [ 32/119] doserror
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\doserror.obj doserror.c
echo   [ 33/119] dosfclos
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\dosfclos.obj dosfclos.c
echo   [ 34/119] dosfgets
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\dosfgets.obj dosfgets.c
echo   [ 35/119] dosflush
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\dosflush.obj dosflush.c
echo   [ 36/119] dosfopen
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\dosfopen.obj dosfopen.c
echo   [ 37/119] dosfputs
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\dosfputs.obj dosfputs.c
echo   [ 38/119] dosfread
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\dosfread.obj dosfread.c
echo   [ 39/119] dosfseek
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\dosfseek.obj dosfseek.c
echo   [ 40/119] dosfwrit
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\dosfwrit.obj dosfwrit.c
cd \toolkit\pwa153\source\toolkit
echo   [ 41/119] dosinit
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\dosinit.obj dosinit.c
cd \toolkit\pwa153\source\dos
echo   [ 42/119] doslseek
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\doslseek.obj doslseek.c
echo   [ 43/119] dosopen
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\dosopen.obj dosopen.c
echo   [ 44/119] dosread
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\dosread.obj dosread.c
echo   [ 45/119] dosrewin
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\dosrewin.obj dosrewin.c
echo   [ 46/119] dosstbuf
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\dosstbuf.obj dosstbuf.c
echo   [ 47/119] doswrite
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\doswrite.obj doswrite.c
cd \toolkit\pwa153\source\misc
echo   [ 48/119] exist
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\exist.obj exist.c
cd \toolkit\pwa153\source\pcb
echo   [ 49/119] exitdos
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\exitdos.obj exitdos.c
cd \toolkit\pwa153\source\dos
echo   [ 50/119] extended
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\extended.obj extended.c
cd \toolkit\pwa153\source\misc
echo   [ 51/119] fmemcpy
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\fmemcpy.obj fmemcpy.c
echo   [ 52/119] fmemset
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\fmemset.obj fmemset.c
cd \toolkit\pwa153\source\screen
echo   [ 53/119] getmode
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\getmode.obj getmode.c
echo   [ 54/119] giveup
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\giveup.obj giveup.c
cd \toolkit\pwa153\source\toolkit
echo   [ 55/119] goodbye
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\goodbye.obj goodbye.c
cd \toolkit\pwa153\source\screen
echo   [ 56/119] gotoxy
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\gotoxy.obj gotoxy.c
cd \toolkit\pwa153\source\dos
echo   [ 57/119] handlers
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\handlers.obj handlers.c
cd \toolkit\pwa153\source\scrnio
echo   [ 58/119] help
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\help.obj help.c
cd \toolkit\pwa153\source\misc
echo   [ 59/119] hextoi
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\hextoi.obj hextoi.c
echo   [ 60/119] index
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\index.obj index.c
cd \toolkit\pwa153\source\toolkit
echo   [ 61/119] init
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\init.obj init.c
echo   [ 62/119] inputreq
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\inputreq.obj inputreq.c
cd \toolkit\pwa153\source\dos
echo   [ 63/119] int24hnd (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__s__ int24hnd.asm,c:\out\lib\pwa153\msc70\obj\small\int24hnd.obj
cd \toolkit\pwa153\source\misc
echo   [ 64/119] isset
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\isset.obj isset.c
echo   [ 65/119] julian
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\julian.obj julian.c
echo   [ 66/119] lastchar
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\lastchar.obj lastchar.c
echo   [ 67/119] long_bd
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\long_bd.obj long_bd.c
echo   [ 68/119] long_bs
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\long_bs.obj long_bs.c
cd \toolkit\pwa153\source\country
echo   [ 69/119] memfcmp
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\memfcmp.obj memfcmp.c
echo   [ 70/119] memicmp
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\memicmp.obj memicmp.c
cd \pcb153\source\asm
echo   [ 71/119] memmove (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__s__ memmove.asm,c:\out\lib\pwa153\msc70\obj\small\memmove.obj
cd \toolkit\pwa153\source\misc
echo   [ 72/119] mstrcpy
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\mstrcpy.obj mstrcpy.c
echo   [ 73/119] padstr
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\padstr.obj padstr.c
cd \toolkit\pwa153\source\pcb
echo   [ 74/119] parsepth
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\parsepth.obj parsepth.c
cd \toolkit\pwa153\source\toolkit
echo   [ 75/119] pcbinit
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\pcbinit.obj pcbinit.c
cd \toolkit\pwa153\source\screen
echo   [ 76/119] print
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\print.obj print.c
cd \toolkit\pwa153\source\misc
echo   [ 77/119] prnready
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\prnready.obj prnready.c
echo   [ 78/119] proper
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\proper.obj proper.c
cd \toolkit\pwa153\source\toolkit
echo   [ 79/119] recycle
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\recycle.obj recycle.c
cd \toolkit\pwa153\source\screen
echo   [ 80/119] saverest
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\saverest.obj saverest.c
cd \toolkit\pwa153\source\misc
echo   [ 81/119] setbit
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\setbit.obj setbit.c
cd \toolkit\pwa153\source\dos
echo   [ 82/119] showerr
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\showerr.obj showerr.c
cd \toolkit\pwa153\source\toolkit
echo   [ 83/119] slowmodm
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\slowmodm.obj slowmodm.c
cd \toolkit\pwa153\source\screen
echo   [ 84/119] sound
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\sound.obj sound.c
cd \toolkit\pwa153\source\pcb
echo   [ 85/119] srchpath
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\srchpath.obj srchpath.c
cd \toolkit\pwa153\source\country
echo   [ 86/119] stricmp
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\stricmp.obj stricmp.c
cd \toolkit\pwa153\source\misc
echo   [ 87/119] stripl
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\stripl.obj stripl.c
echo   [ 88/119] stripr
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\stripr.obj stripr.c
cd \toolkit\pwa153\source\country
echo   [ 89/119] strlwr
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\strlwr.obj strlwr.c
cd \toolkit\pwa153\source\dos
echo   [ 90/119] strnchr
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\strnchr.obj strnchr.c
cd \toolkit\pwa153\source\country
echo   [ 91/119] strnicmp
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\strnicmp.obj strnicmp.c
echo   [ 92/119] strupr
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\strupr.obj strupr.c
cd \toolkit\pwa153\source\misc
echo   [ 93/119] subst
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\subst.obj subst.c
echo   [ 94/119] time
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\time.obj time.c
cd \toolkit\pwa153\source\screen
echo   [ 95/119] time1
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\time1.obj time1.c
echo   [ 96/119] time2
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\time2.obj time2.c
cd \pcb153\source\asm
echo   [ 97/119] timer (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__s__ timer.asm,c:\out\lib\pwa153\msc70\obj\small\timer.obj
cd \toolkit\pwa153\source\misc
echo   [ 98/119] unsetbit
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\unsetbit.obj unsetbit.c
cd \toolkit\pwa153\source\toolkit
echo   [ 99/119] usersys
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\usersys.obj usersys.c
cd \toolkit\pwa153\source\screen
echo   [100/119] wherey
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\wherey.obj wherey.c
cd \pcb153\source\main
echo   [101/119] chat
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\chat.obj chat.c
cd \pcb153\source\modem
echo   [102/119] devioctl
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\devioctl.obj devioctl.c
cd \pcb153\source\display
echo   [103/119] display
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\display.obj display.c
echo   [104/119] files
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\files.obj files.c
cd \pcb153\source\main
echo   [105/119] inkey
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\inkey.obj inkey.c
echo   [106/119] input
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\input.obj input.c
echo   [107/119] language
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\language.obj language.c
cd \pcb153\source\node
echo   [108/119] log
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\log.obj log.c
cd \pcb153\source\support
echo   [109/119] memory
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\memory.obj memory.c
cd \pcb153\source\main
echo   [110/119] misc
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\misc.obj misc.c
cd \pcb153\source\modem
echo   [111/119] modem
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\modem.obj modem.c
cd \pcb153\source\display
echo   [112/119] pcbtext
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\pcbtext.obj pcbtext.c
echo   [113/119] screen
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\screen.obj screen.c
cd \pcb153\source\main
echo   [114/119] shell
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\shell.obj shell.c
cd \pcb153\source\display
echo   [115/119] status
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\status.obj status.c
cd \pcb153\source\main
echo   [116/119] sys
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\sys.obj sys.c
cd \pcb153\source\support
echo   [117/119] token
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\token.obj token.c
cd \pcb153\source\display
echo   [118/119] xlate
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\xlate.obj xlate.c
cd \pcb153\source\modem
echo   [119/119] modemasy
%cc% -c -AS -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\small\modemasy.obj modemasy.c
echo.
echo === memory model: medium ===
if not exist c:\out\lib\pwa153\msc70\obj\medium md c:\out\lib\pwa153\msc70\obj\medium
cd \toolkit\pwa153\source\pcb
echo   [  1/119] addbacks
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\addbacks.obj addbacks.c
cd \toolkit\pwa153\source\misc
echo   [  2/119] addchar
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\addchar.obj addchar.c
cd \toolkit\pwa153\source\screen
echo   [  3/119] ansi
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\ansi.obj ansi.c
cd \toolkit\pwa153\source\misc
echo   [  4/119] ascii
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\ascii.obj ascii.c
cd \pcb153\source\asm
echo   [  5/119] async (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__m__ async.asm,c:\out\lib\pwa153\msc70\obj\medium\async.obj
cd \toolkit\pwa153\source\toolkit
echo   [  6/119] atclose
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\atclose.obj atclose.c
cd \toolkit\pwa153\source\misc
echo   [  7/119] bd_long
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\bd_long.obj bd_long.c
cd \pcb153\source\asm
echo   [  8/119] bgkey (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__m__ bgkey.asm,c:\out\lib\pwa153\msc70\obj\medium\bgkey.obj
cd \toolkit\pwa153\source\misc
echo   [  9/119] bs_long
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\bs_long.obj bs_long.c
cd \toolkit\pwa153\source\dos
echo   [ 10/119] chkappen
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\chkappen.obj chkappen.c
echo   [ 11/119] chkcreat
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\chkcreat.obj chkcreat.c
echo   [ 12/119] chklock
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\chklock.obj chklock.c
echo   [ 13/119] chkopen
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\chkopen.obj chkopen.c
echo   [ 14/119] chkread
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\chkread.obj chkread.c
echo   [ 15/119] chkwrite
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\chkwrite.obj chkwrite.c
cd \toolkit\pwa153\source\screen
echo   [ 16/119] cls
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\cls.obj cls.c
cd \toolkit\pwa153\source\toolkit
echo   [ 17/119] cnames
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\cnames.obj cnames.c
cd \toolkit\pwa153\source\country
echo   [ 18/119] comma
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\comma.obj comma.c
echo   [ 19/119] country
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\country.obj country.c
cd \toolkit\pwa153\source\misc
echo   [ 20/119] ctod
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\ctod.obj ctod.c
cd \toolkit\pwa153\source\screen
echo   [ 21/119] cursor
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\cursor.obj cursor.c
cd \toolkit\pwa153\source\toolkit
echo   [ 22/119] custhelp
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\custhelp.obj custhelp.c
cd \pcb153\source\asm
echo   [ 23/119] cutil (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__m__ cutil.asm,c:\out\lib\pwa153\msc70\obj\medium\cutil.obj
cd \toolkit\pwa153\source\pcb
echo   [ 24/119] datafil2
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\datafil2.obj datafil2.c
cd \toolkit\pwa153\source\country
echo   [ 25/119] date
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\date.obj date.c
cd \toolkit\pwa153\source\screen
echo   [ 26/119] datestr
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\datestr.obj datestr.c
echo   [ 27/119] delay
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\delay.obj delay.c
cd \toolkit\pwa153\source\dos
echo   [ 28/119] dosappen
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\dosappen.obj dosappen.c
echo   [ 29/119] dosclose
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\dosclose.obj dosclose.c
echo   [ 30/119] doscomit
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\doscomit.obj doscomit.c
echo   [ 31/119] doscreat
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\doscreat.obj doscreat.c
echo   [ 32/119] doserror
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\doserror.obj doserror.c
echo   [ 33/119] dosfclos
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\dosfclos.obj dosfclos.c
echo   [ 34/119] dosfgets
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\dosfgets.obj dosfgets.c
echo   [ 35/119] dosflush
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\dosflush.obj dosflush.c
echo   [ 36/119] dosfopen
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\dosfopen.obj dosfopen.c
echo   [ 37/119] dosfputs
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\dosfputs.obj dosfputs.c
echo   [ 38/119] dosfread
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\dosfread.obj dosfread.c
echo   [ 39/119] dosfseek
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\dosfseek.obj dosfseek.c
echo   [ 40/119] dosfwrit
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\dosfwrit.obj dosfwrit.c
cd \toolkit\pwa153\source\toolkit
echo   [ 41/119] dosinit
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\dosinit.obj dosinit.c
cd \toolkit\pwa153\source\dos
echo   [ 42/119] doslseek
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\doslseek.obj doslseek.c
echo   [ 43/119] dosopen
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\dosopen.obj dosopen.c
echo   [ 44/119] dosread
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\dosread.obj dosread.c
echo   [ 45/119] dosrewin
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\dosrewin.obj dosrewin.c
echo   [ 46/119] dosstbuf
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\dosstbuf.obj dosstbuf.c
echo   [ 47/119] doswrite
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\doswrite.obj doswrite.c
cd \toolkit\pwa153\source\misc
echo   [ 48/119] exist
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\exist.obj exist.c
cd \toolkit\pwa153\source\pcb
echo   [ 49/119] exitdos
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\exitdos.obj exitdos.c
cd \toolkit\pwa153\source\dos
echo   [ 50/119] extended
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\extended.obj extended.c
cd \toolkit\pwa153\source\misc
echo   [ 51/119] fmemcpy
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\fmemcpy.obj fmemcpy.c
echo   [ 52/119] fmemset
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\fmemset.obj fmemset.c
cd \toolkit\pwa153\source\screen
echo   [ 53/119] getmode
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\getmode.obj getmode.c
echo   [ 54/119] giveup
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\giveup.obj giveup.c
cd \toolkit\pwa153\source\toolkit
echo   [ 55/119] goodbye
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\goodbye.obj goodbye.c
cd \toolkit\pwa153\source\screen
echo   [ 56/119] gotoxy
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\gotoxy.obj gotoxy.c
cd \toolkit\pwa153\source\dos
echo   [ 57/119] handlers
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\handlers.obj handlers.c
cd \toolkit\pwa153\source\scrnio
echo   [ 58/119] help
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\help.obj help.c
cd \toolkit\pwa153\source\misc
echo   [ 59/119] hextoi
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\hextoi.obj hextoi.c
echo   [ 60/119] index
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\index.obj index.c
cd \toolkit\pwa153\source\toolkit
echo   [ 61/119] init
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\init.obj init.c
echo   [ 62/119] inputreq
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\inputreq.obj inputreq.c
cd \toolkit\pwa153\source\dos
echo   [ 63/119] int24hnd (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__m__ int24hnd.asm,c:\out\lib\pwa153\msc70\obj\medium\int24hnd.obj
cd \toolkit\pwa153\source\misc
echo   [ 64/119] isset
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\isset.obj isset.c
echo   [ 65/119] julian
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\julian.obj julian.c
echo   [ 66/119] lastchar
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\lastchar.obj lastchar.c
echo   [ 67/119] long_bd
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\long_bd.obj long_bd.c
echo   [ 68/119] long_bs
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\long_bs.obj long_bs.c
cd \toolkit\pwa153\source\country
echo   [ 69/119] memfcmp
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\memfcmp.obj memfcmp.c
echo   [ 70/119] memicmp
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\memicmp.obj memicmp.c
cd \pcb153\source\asm
echo   [ 71/119] memmove (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__m__ memmove.asm,c:\out\lib\pwa153\msc70\obj\medium\memmove.obj
cd \toolkit\pwa153\source\misc
echo   [ 72/119] mstrcpy
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\mstrcpy.obj mstrcpy.c
echo   [ 73/119] padstr
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\padstr.obj padstr.c
cd \toolkit\pwa153\source\pcb
echo   [ 74/119] parsepth
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\parsepth.obj parsepth.c
cd \toolkit\pwa153\source\toolkit
echo   [ 75/119] pcbinit
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\pcbinit.obj pcbinit.c
cd \toolkit\pwa153\source\screen
echo   [ 76/119] print
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\print.obj print.c
cd \toolkit\pwa153\source\misc
echo   [ 77/119] prnready
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\prnready.obj prnready.c
echo   [ 78/119] proper
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\proper.obj proper.c
cd \toolkit\pwa153\source\toolkit
echo   [ 79/119] recycle
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\recycle.obj recycle.c
cd \toolkit\pwa153\source\screen
echo   [ 80/119] saverest
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\saverest.obj saverest.c
cd \toolkit\pwa153\source\misc
echo   [ 81/119] setbit
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\setbit.obj setbit.c
cd \toolkit\pwa153\source\dos
echo   [ 82/119] showerr
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\showerr.obj showerr.c
cd \toolkit\pwa153\source\toolkit
echo   [ 83/119] slowmodm
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\slowmodm.obj slowmodm.c
cd \toolkit\pwa153\source\screen
echo   [ 84/119] sound
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\sound.obj sound.c
cd \toolkit\pwa153\source\pcb
echo   [ 85/119] srchpath
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\srchpath.obj srchpath.c
cd \toolkit\pwa153\source\country
echo   [ 86/119] stricmp
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\stricmp.obj stricmp.c
cd \toolkit\pwa153\source\misc
echo   [ 87/119] stripl
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\stripl.obj stripl.c
echo   [ 88/119] stripr
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\stripr.obj stripr.c
cd \toolkit\pwa153\source\country
echo   [ 89/119] strlwr
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\strlwr.obj strlwr.c
cd \toolkit\pwa153\source\dos
echo   [ 90/119] strnchr
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\strnchr.obj strnchr.c
cd \toolkit\pwa153\source\country
echo   [ 91/119] strnicmp
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\strnicmp.obj strnicmp.c
echo   [ 92/119] strupr
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\strupr.obj strupr.c
cd \toolkit\pwa153\source\misc
echo   [ 93/119] subst
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\subst.obj subst.c
echo   [ 94/119] time
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\time.obj time.c
cd \toolkit\pwa153\source\screen
echo   [ 95/119] time1
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\time1.obj time1.c
echo   [ 96/119] time2
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\time2.obj time2.c
cd \pcb153\source\asm
echo   [ 97/119] timer (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__m__ timer.asm,c:\out\lib\pwa153\msc70\obj\medium\timer.obj
cd \toolkit\pwa153\source\misc
echo   [ 98/119] unsetbit
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\unsetbit.obj unsetbit.c
cd \toolkit\pwa153\source\toolkit
echo   [ 99/119] usersys
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\usersys.obj usersys.c
cd \toolkit\pwa153\source\screen
echo   [100/119] wherey
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\wherey.obj wherey.c
cd \pcb153\source\main
echo   [101/119] chat
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\chat.obj chat.c
cd \pcb153\source\modem
echo   [102/119] devioctl
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\devioctl.obj devioctl.c
cd \pcb153\source\display
echo   [103/119] display
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\display.obj display.c
echo   [104/119] files
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\files.obj files.c
cd \pcb153\source\main
echo   [105/119] inkey
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\inkey.obj inkey.c
echo   [106/119] input
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\input.obj input.c
echo   [107/119] language
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\language.obj language.c
cd \pcb153\source\node
echo   [108/119] log
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\log.obj log.c
cd \pcb153\source\support
echo   [109/119] memory
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\memory.obj memory.c
cd \pcb153\source\main
echo   [110/119] misc
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\misc.obj misc.c
cd \pcb153\source\modem
echo   [111/119] modem
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\modem.obj modem.c
cd \pcb153\source\display
echo   [112/119] pcbtext
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\pcbtext.obj pcbtext.c
echo   [113/119] screen
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\screen.obj screen.c
cd \pcb153\source\main
echo   [114/119] shell
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\shell.obj shell.c
cd \pcb153\source\display
echo   [115/119] status
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\status.obj status.c
cd \pcb153\source\main
echo   [116/119] sys
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\sys.obj sys.c
cd \pcb153\source\support
echo   [117/119] token
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\token.obj token.c
cd \pcb153\source\display
echo   [118/119] xlate
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\xlate.obj xlate.c
cd \pcb153\source\modem
echo   [119/119] modemasy
%cc% -c -AM -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\medium\modemasy.obj modemasy.c
echo.
echo === memory model: compact ===
if not exist c:\out\lib\pwa153\msc70\obj\compact md c:\out\lib\pwa153\msc70\obj\compact
cd \toolkit\pwa153\source\pcb
echo   [  1/119] addbacks
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\addbacks.obj addbacks.c
cd \toolkit\pwa153\source\misc
echo   [  2/119] addchar
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\addchar.obj addchar.c
cd \toolkit\pwa153\source\screen
echo   [  3/119] ansi
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\ansi.obj ansi.c
cd \toolkit\pwa153\source\misc
echo   [  4/119] ascii
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\ascii.obj ascii.c
cd \pcb153\source\asm
echo   [  5/119] async (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__c__ async.asm,c:\out\lib\pwa153\msc70\obj\compact\async.obj
cd \toolkit\pwa153\source\toolkit
echo   [  6/119] atclose
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\atclose.obj atclose.c
cd \toolkit\pwa153\source\misc
echo   [  7/119] bd_long
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\bd_long.obj bd_long.c
cd \pcb153\source\asm
echo   [  8/119] bgkey (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__c__ bgkey.asm,c:\out\lib\pwa153\msc70\obj\compact\bgkey.obj
cd \toolkit\pwa153\source\misc
echo   [  9/119] bs_long
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\bs_long.obj bs_long.c
cd \toolkit\pwa153\source\dos
echo   [ 10/119] chkappen
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\chkappen.obj chkappen.c
echo   [ 11/119] chkcreat
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\chkcreat.obj chkcreat.c
echo   [ 12/119] chklock
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\chklock.obj chklock.c
echo   [ 13/119] chkopen
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\chkopen.obj chkopen.c
echo   [ 14/119] chkread
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\chkread.obj chkread.c
echo   [ 15/119] chkwrite
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\chkwrite.obj chkwrite.c
cd \toolkit\pwa153\source\screen
echo   [ 16/119] cls
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\cls.obj cls.c
cd \toolkit\pwa153\source\toolkit
echo   [ 17/119] cnames
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\cnames.obj cnames.c
cd \toolkit\pwa153\source\country
echo   [ 18/119] comma
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\comma.obj comma.c
echo   [ 19/119] country
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\country.obj country.c
cd \toolkit\pwa153\source\misc
echo   [ 20/119] ctod
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\ctod.obj ctod.c
cd \toolkit\pwa153\source\screen
echo   [ 21/119] cursor
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\cursor.obj cursor.c
cd \toolkit\pwa153\source\toolkit
echo   [ 22/119] custhelp
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\custhelp.obj custhelp.c
cd \pcb153\source\asm
echo   [ 23/119] cutil (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__c__ cutil.asm,c:\out\lib\pwa153\msc70\obj\compact\cutil.obj
cd \toolkit\pwa153\source\pcb
echo   [ 24/119] datafil2
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\datafil2.obj datafil2.c
cd \toolkit\pwa153\source\country
echo   [ 25/119] date
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\date.obj date.c
cd \toolkit\pwa153\source\screen
echo   [ 26/119] datestr
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\datestr.obj datestr.c
echo   [ 27/119] delay
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\delay.obj delay.c
cd \toolkit\pwa153\source\dos
echo   [ 28/119] dosappen
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\dosappen.obj dosappen.c
echo   [ 29/119] dosclose
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\dosclose.obj dosclose.c
echo   [ 30/119] doscomit
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\doscomit.obj doscomit.c
echo   [ 31/119] doscreat
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\doscreat.obj doscreat.c
echo   [ 32/119] doserror
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\doserror.obj doserror.c
echo   [ 33/119] dosfclos
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\dosfclos.obj dosfclos.c
echo   [ 34/119] dosfgets
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\dosfgets.obj dosfgets.c
echo   [ 35/119] dosflush
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\dosflush.obj dosflush.c
echo   [ 36/119] dosfopen
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\dosfopen.obj dosfopen.c
echo   [ 37/119] dosfputs
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\dosfputs.obj dosfputs.c
echo   [ 38/119] dosfread
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\dosfread.obj dosfread.c
echo   [ 39/119] dosfseek
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\dosfseek.obj dosfseek.c
echo   [ 40/119] dosfwrit
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\dosfwrit.obj dosfwrit.c
cd \toolkit\pwa153\source\toolkit
echo   [ 41/119] dosinit
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\dosinit.obj dosinit.c
cd \toolkit\pwa153\source\dos
echo   [ 42/119] doslseek
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\doslseek.obj doslseek.c
echo   [ 43/119] dosopen
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\dosopen.obj dosopen.c
echo   [ 44/119] dosread
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\dosread.obj dosread.c
echo   [ 45/119] dosrewin
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\dosrewin.obj dosrewin.c
echo   [ 46/119] dosstbuf
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\dosstbuf.obj dosstbuf.c
echo   [ 47/119] doswrite
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\doswrite.obj doswrite.c
cd \toolkit\pwa153\source\misc
echo   [ 48/119] exist
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\exist.obj exist.c
cd \toolkit\pwa153\source\pcb
echo   [ 49/119] exitdos
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\exitdos.obj exitdos.c
cd \toolkit\pwa153\source\dos
echo   [ 50/119] extended
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\extended.obj extended.c
cd \toolkit\pwa153\source\misc
echo   [ 51/119] fmemcpy
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\fmemcpy.obj fmemcpy.c
echo   [ 52/119] fmemset
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\fmemset.obj fmemset.c
cd \toolkit\pwa153\source\screen
echo   [ 53/119] getmode
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\getmode.obj getmode.c
echo   [ 54/119] giveup
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\giveup.obj giveup.c
cd \toolkit\pwa153\source\toolkit
echo   [ 55/119] goodbye
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\goodbye.obj goodbye.c
cd \toolkit\pwa153\source\screen
echo   [ 56/119] gotoxy
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\gotoxy.obj gotoxy.c
cd \toolkit\pwa153\source\dos
echo   [ 57/119] handlers
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\handlers.obj handlers.c
cd \toolkit\pwa153\source\scrnio
echo   [ 58/119] help
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\help.obj help.c
cd \toolkit\pwa153\source\misc
echo   [ 59/119] hextoi
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\hextoi.obj hextoi.c
echo   [ 60/119] index
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\index.obj index.c
cd \toolkit\pwa153\source\toolkit
echo   [ 61/119] init
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\init.obj init.c
echo   [ 62/119] inputreq
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\inputreq.obj inputreq.c
cd \toolkit\pwa153\source\dos
echo   [ 63/119] int24hnd (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__c__ int24hnd.asm,c:\out\lib\pwa153\msc70\obj\compact\int24hnd.obj
cd \toolkit\pwa153\source\misc
echo   [ 64/119] isset
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\isset.obj isset.c
echo   [ 65/119] julian
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\julian.obj julian.c
echo   [ 66/119] lastchar
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\lastchar.obj lastchar.c
echo   [ 67/119] long_bd
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\long_bd.obj long_bd.c
echo   [ 68/119] long_bs
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\long_bs.obj long_bs.c
cd \toolkit\pwa153\source\country
echo   [ 69/119] memfcmp
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\memfcmp.obj memfcmp.c
echo   [ 70/119] memicmp
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\memicmp.obj memicmp.c
cd \pcb153\source\asm
echo   [ 71/119] memmove (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__c__ memmove.asm,c:\out\lib\pwa153\msc70\obj\compact\memmove.obj
cd \toolkit\pwa153\source\misc
echo   [ 72/119] mstrcpy
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\mstrcpy.obj mstrcpy.c
echo   [ 73/119] padstr
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\padstr.obj padstr.c
cd \toolkit\pwa153\source\pcb
echo   [ 74/119] parsepth
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\parsepth.obj parsepth.c
cd \toolkit\pwa153\source\toolkit
echo   [ 75/119] pcbinit
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\pcbinit.obj pcbinit.c
cd \toolkit\pwa153\source\screen
echo   [ 76/119] print
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\print.obj print.c
cd \toolkit\pwa153\source\misc
echo   [ 77/119] prnready
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\prnready.obj prnready.c
echo   [ 78/119] proper
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\proper.obj proper.c
cd \toolkit\pwa153\source\toolkit
echo   [ 79/119] recycle
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\recycle.obj recycle.c
cd \toolkit\pwa153\source\screen
echo   [ 80/119] saverest
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\saverest.obj saverest.c
cd \toolkit\pwa153\source\misc
echo   [ 81/119] setbit
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\setbit.obj setbit.c
cd \toolkit\pwa153\source\dos
echo   [ 82/119] showerr
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\showerr.obj showerr.c
cd \toolkit\pwa153\source\toolkit
echo   [ 83/119] slowmodm
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\slowmodm.obj slowmodm.c
cd \toolkit\pwa153\source\screen
echo   [ 84/119] sound
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\sound.obj sound.c
cd \toolkit\pwa153\source\pcb
echo   [ 85/119] srchpath
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\srchpath.obj srchpath.c
cd \toolkit\pwa153\source\country
echo   [ 86/119] stricmp
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\stricmp.obj stricmp.c
cd \toolkit\pwa153\source\misc
echo   [ 87/119] stripl
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\stripl.obj stripl.c
echo   [ 88/119] stripr
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\stripr.obj stripr.c
cd \toolkit\pwa153\source\country
echo   [ 89/119] strlwr
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\strlwr.obj strlwr.c
cd \toolkit\pwa153\source\dos
echo   [ 90/119] strnchr
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\strnchr.obj strnchr.c
cd \toolkit\pwa153\source\country
echo   [ 91/119] strnicmp
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\strnicmp.obj strnicmp.c
echo   [ 92/119] strupr
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\strupr.obj strupr.c
cd \toolkit\pwa153\source\misc
echo   [ 93/119] subst
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\subst.obj subst.c
echo   [ 94/119] time
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\time.obj time.c
cd \toolkit\pwa153\source\screen
echo   [ 95/119] time1
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\time1.obj time1.c
echo   [ 96/119] time2
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\time2.obj time2.c
cd \pcb153\source\asm
echo   [ 97/119] timer (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__c__ timer.asm,c:\out\lib\pwa153\msc70\obj\compact\timer.obj
cd \toolkit\pwa153\source\misc
echo   [ 98/119] unsetbit
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\unsetbit.obj unsetbit.c
cd \toolkit\pwa153\source\toolkit
echo   [ 99/119] usersys
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\usersys.obj usersys.c
cd \toolkit\pwa153\source\screen
echo   [100/119] wherey
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\wherey.obj wherey.c
cd \pcb153\source\main
echo   [101/119] chat
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\chat.obj chat.c
cd \pcb153\source\modem
echo   [102/119] devioctl
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\devioctl.obj devioctl.c
cd \pcb153\source\display
echo   [103/119] display
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\display.obj display.c
echo   [104/119] files
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\files.obj files.c
cd \pcb153\source\main
echo   [105/119] inkey
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\inkey.obj inkey.c
echo   [106/119] input
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\input.obj input.c
echo   [107/119] language
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\language.obj language.c
cd \pcb153\source\node
echo   [108/119] log
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\log.obj log.c
cd \pcb153\source\support
echo   [109/119] memory
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\memory.obj memory.c
cd \pcb153\source\main
echo   [110/119] misc
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\misc.obj misc.c
cd \pcb153\source\modem
echo   [111/119] modem
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\modem.obj modem.c
cd \pcb153\source\display
echo   [112/119] pcbtext
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\pcbtext.obj pcbtext.c
echo   [113/119] screen
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\screen.obj screen.c
cd \pcb153\source\main
echo   [114/119] shell
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\shell.obj shell.c
cd \pcb153\source\display
echo   [115/119] status
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\status.obj status.c
cd \pcb153\source\main
echo   [116/119] sys
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\sys.obj sys.c
cd \pcb153\source\support
echo   [117/119] token
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\token.obj token.c
cd \pcb153\source\display
echo   [118/119] xlate
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\xlate.obj xlate.c
cd \pcb153\source\modem
echo   [119/119] modemasy
%cc% -c -AC -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\compact\modemasy.obj modemasy.c
echo.
echo === memory model: large ===
if not exist c:\out\lib\pwa153\msc70\obj\large md c:\out\lib\pwa153\msc70\obj\large
cd \toolkit\pwa153\source\pcb
echo   [  1/119] addbacks
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\addbacks.obj addbacks.c
cd \toolkit\pwa153\source\misc
echo   [  2/119] addchar
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\addchar.obj addchar.c
cd \toolkit\pwa153\source\screen
echo   [  3/119] ansi
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\ansi.obj ansi.c
cd \toolkit\pwa153\source\misc
echo   [  4/119] ascii
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\ascii.obj ascii.c
cd \pcb153\source\asm
echo   [  5/119] async (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__l__ async.asm,c:\out\lib\pwa153\msc70\obj\large\async.obj
cd \toolkit\pwa153\source\toolkit
echo   [  6/119] atclose
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\atclose.obj atclose.c
cd \toolkit\pwa153\source\misc
echo   [  7/119] bd_long
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\bd_long.obj bd_long.c
cd \pcb153\source\asm
echo   [  8/119] bgkey (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__l__ bgkey.asm,c:\out\lib\pwa153\msc70\obj\large\bgkey.obj
cd \toolkit\pwa153\source\misc
echo   [  9/119] bs_long
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\bs_long.obj bs_long.c
cd \toolkit\pwa153\source\dos
echo   [ 10/119] chkappen
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\chkappen.obj chkappen.c
echo   [ 11/119] chkcreat
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\chkcreat.obj chkcreat.c
echo   [ 12/119] chklock
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\chklock.obj chklock.c
echo   [ 13/119] chkopen
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\chkopen.obj chkopen.c
echo   [ 14/119] chkread
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\chkread.obj chkread.c
echo   [ 15/119] chkwrite
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\chkwrite.obj chkwrite.c
cd \toolkit\pwa153\source\screen
echo   [ 16/119] cls
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\cls.obj cls.c
cd \toolkit\pwa153\source\toolkit
echo   [ 17/119] cnames
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\cnames.obj cnames.c
cd \toolkit\pwa153\source\country
echo   [ 18/119] comma
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\comma.obj comma.c
echo   [ 19/119] country
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\country.obj country.c
cd \toolkit\pwa153\source\misc
echo   [ 20/119] ctod
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\ctod.obj ctod.c
cd \toolkit\pwa153\source\screen
echo   [ 21/119] cursor
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\cursor.obj cursor.c
cd \toolkit\pwa153\source\toolkit
echo   [ 22/119] custhelp
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\custhelp.obj custhelp.c
cd \pcb153\source\asm
echo   [ 23/119] cutil (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__l__ cutil.asm,c:\out\lib\pwa153\msc70\obj\large\cutil.obj
cd \toolkit\pwa153\source\pcb
echo   [ 24/119] datafil2
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\datafil2.obj datafil2.c
cd \toolkit\pwa153\source\country
echo   [ 25/119] date
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\date.obj date.c
cd \toolkit\pwa153\source\screen
echo   [ 26/119] datestr
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\datestr.obj datestr.c
echo   [ 27/119] delay
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\delay.obj delay.c
cd \toolkit\pwa153\source\dos
echo   [ 28/119] dosappen
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\dosappen.obj dosappen.c
echo   [ 29/119] dosclose
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\dosclose.obj dosclose.c
echo   [ 30/119] doscomit
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\doscomit.obj doscomit.c
echo   [ 31/119] doscreat
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\doscreat.obj doscreat.c
echo   [ 32/119] doserror
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\doserror.obj doserror.c
echo   [ 33/119] dosfclos
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\dosfclos.obj dosfclos.c
echo   [ 34/119] dosfgets
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\dosfgets.obj dosfgets.c
echo   [ 35/119] dosflush
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\dosflush.obj dosflush.c
echo   [ 36/119] dosfopen
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\dosfopen.obj dosfopen.c
echo   [ 37/119] dosfputs
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\dosfputs.obj dosfputs.c
echo   [ 38/119] dosfread
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\dosfread.obj dosfread.c
echo   [ 39/119] dosfseek
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\dosfseek.obj dosfseek.c
echo   [ 40/119] dosfwrit
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\dosfwrit.obj dosfwrit.c
cd \toolkit\pwa153\source\toolkit
echo   [ 41/119] dosinit
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\dosinit.obj dosinit.c
cd \toolkit\pwa153\source\dos
echo   [ 42/119] doslseek
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\doslseek.obj doslseek.c
echo   [ 43/119] dosopen
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\dosopen.obj dosopen.c
echo   [ 44/119] dosread
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\dosread.obj dosread.c
echo   [ 45/119] dosrewin
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\dosrewin.obj dosrewin.c
echo   [ 46/119] dosstbuf
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\dosstbuf.obj dosstbuf.c
echo   [ 47/119] doswrite
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\doswrite.obj doswrite.c
cd \toolkit\pwa153\source\misc
echo   [ 48/119] exist
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\exist.obj exist.c
cd \toolkit\pwa153\source\pcb
echo   [ 49/119] exitdos
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\exitdos.obj exitdos.c
cd \toolkit\pwa153\source\dos
echo   [ 50/119] extended
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\extended.obj extended.c
cd \toolkit\pwa153\source\misc
echo   [ 51/119] fmemcpy
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\fmemcpy.obj fmemcpy.c
echo   [ 52/119] fmemset
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\fmemset.obj fmemset.c
cd \toolkit\pwa153\source\screen
echo   [ 53/119] getmode
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\getmode.obj getmode.c
echo   [ 54/119] giveup
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\giveup.obj giveup.c
cd \toolkit\pwa153\source\toolkit
echo   [ 55/119] goodbye
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\goodbye.obj goodbye.c
cd \toolkit\pwa153\source\screen
echo   [ 56/119] gotoxy
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\gotoxy.obj gotoxy.c
cd \toolkit\pwa153\source\dos
echo   [ 57/119] handlers
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\handlers.obj handlers.c
cd \toolkit\pwa153\source\scrnio
echo   [ 58/119] help
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\help.obj help.c
cd \toolkit\pwa153\source\misc
echo   [ 59/119] hextoi
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\hextoi.obj hextoi.c
echo   [ 60/119] index
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\index.obj index.c
cd \toolkit\pwa153\source\toolkit
echo   [ 61/119] init
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\init.obj init.c
echo   [ 62/119] inputreq
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\inputreq.obj inputreq.c
cd \toolkit\pwa153\source\dos
echo   [ 63/119] int24hnd (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__l__ int24hnd.asm,c:\out\lib\pwa153\msc70\obj\large\int24hnd.obj
cd \toolkit\pwa153\source\misc
echo   [ 64/119] isset
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\isset.obj isset.c
echo   [ 65/119] julian
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\julian.obj julian.c
echo   [ 66/119] lastchar
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\lastchar.obj lastchar.c
echo   [ 67/119] long_bd
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\long_bd.obj long_bd.c
echo   [ 68/119] long_bs
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\long_bs.obj long_bs.c
cd \toolkit\pwa153\source\country
echo   [ 69/119] memfcmp
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\memfcmp.obj memfcmp.c
echo   [ 70/119] memicmp
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\memicmp.obj memicmp.c
cd \pcb153\source\asm
echo   [ 71/119] memmove (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__l__ memmove.asm,c:\out\lib\pwa153\msc70\obj\large\memmove.obj
cd \toolkit\pwa153\source\misc
echo   [ 72/119] mstrcpy
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\mstrcpy.obj mstrcpy.c
echo   [ 73/119] padstr
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\padstr.obj padstr.c
cd \toolkit\pwa153\source\pcb
echo   [ 74/119] parsepth
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\parsepth.obj parsepth.c
cd \toolkit\pwa153\source\toolkit
echo   [ 75/119] pcbinit
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\pcbinit.obj pcbinit.c
cd \toolkit\pwa153\source\screen
echo   [ 76/119] print
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\print.obj print.c
cd \toolkit\pwa153\source\misc
echo   [ 77/119] prnready
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\prnready.obj prnready.c
echo   [ 78/119] proper
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\proper.obj proper.c
cd \toolkit\pwa153\source\toolkit
echo   [ 79/119] recycle
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\recycle.obj recycle.c
cd \toolkit\pwa153\source\screen
echo   [ 80/119] saverest
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\saverest.obj saverest.c
cd \toolkit\pwa153\source\misc
echo   [ 81/119] setbit
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\setbit.obj setbit.c
cd \toolkit\pwa153\source\dos
echo   [ 82/119] showerr
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\showerr.obj showerr.c
cd \toolkit\pwa153\source\toolkit
echo   [ 83/119] slowmodm
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\slowmodm.obj slowmodm.c
cd \toolkit\pwa153\source\screen
echo   [ 84/119] sound
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\sound.obj sound.c
cd \toolkit\pwa153\source\pcb
echo   [ 85/119] srchpath
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\srchpath.obj srchpath.c
cd \toolkit\pwa153\source\country
echo   [ 86/119] stricmp
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\stricmp.obj stricmp.c
cd \toolkit\pwa153\source\misc
echo   [ 87/119] stripl
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\stripl.obj stripl.c
echo   [ 88/119] stripr
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\stripr.obj stripr.c
cd \toolkit\pwa153\source\country
echo   [ 89/119] strlwr
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\strlwr.obj strlwr.c
cd \toolkit\pwa153\source\dos
echo   [ 90/119] strnchr
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\strnchr.obj strnchr.c
cd \toolkit\pwa153\source\country
echo   [ 91/119] strnicmp
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\strnicmp.obj strnicmp.c
echo   [ 92/119] strupr
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\strupr.obj strupr.c
cd \toolkit\pwa153\source\misc
echo   [ 93/119] subst
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\subst.obj subst.c
echo   [ 94/119] time
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\time.obj time.c
cd \toolkit\pwa153\source\screen
echo   [ 95/119] time1
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\time1.obj time1.c
echo   [ 96/119] time2
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\time2.obj time2.c
cd \pcb153\source\asm
echo   [ 97/119] timer (asm)
tasm.exe -mx -ic:\toolkit\pwa153\h -d__l__ timer.asm,c:\out\lib\pwa153\msc70\obj\large\timer.obj
cd \toolkit\pwa153\source\misc
echo   [ 98/119] unsetbit
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\unsetbit.obj unsetbit.c
cd \toolkit\pwa153\source\toolkit
echo   [ 99/119] usersys
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\usersys.obj usersys.c
cd \toolkit\pwa153\source\screen
echo   [100/119] wherey
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\wherey.obj wherey.c
cd \pcb153\source\main
echo   [101/119] chat
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\chat.obj chat.c
cd \pcb153\source\modem
echo   [102/119] devioctl
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\devioctl.obj devioctl.c
cd \pcb153\source\display
echo   [103/119] display
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\display.obj display.c
echo   [104/119] files
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\files.obj files.c
cd \pcb153\source\main
echo   [105/119] inkey
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\inkey.obj inkey.c
echo   [106/119] input
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\input.obj input.c
echo   [107/119] language
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\language.obj language.c
cd \pcb153\source\node
echo   [108/119] log
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\log.obj log.c
cd \pcb153\source\support
echo   [109/119] memory
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\memory.obj memory.c
cd \pcb153\source\main
echo   [110/119] misc
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\misc.obj misc.c
cd \pcb153\source\modem
echo   [111/119] modem
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\modem.obj modem.c
cd \pcb153\source\display
echo   [112/119] pcbtext
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\pcbtext.obj pcbtext.c
echo   [113/119] screen
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\screen.obj screen.c
cd \pcb153\source\main
echo   [114/119] shell
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\shell.obj shell.c
cd \pcb153\source\display
echo   [115/119] status
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\status.obj status.c
cd \pcb153\source\main
echo   [116/119] sys
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\sys.obj sys.c
cd \pcb153\source\support
echo   [117/119] token
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\token.obj token.c
cd \pcb153\source\display
echo   [118/119] xlate
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\xlate.obj xlate.c
cd \pcb153\source\modem
echo   [119/119] modemasy
%cc% -c -AL -I%tkinc% -Ic:\pcb153\source\h -Foc:\out\lib\pwa153\msc70\obj\large\modemasy.obj modemasy.c
echo.
echo all 119 modules compiled/assembled in 4 models.
call mklib.bat pcbkms ms msc70
echo.
