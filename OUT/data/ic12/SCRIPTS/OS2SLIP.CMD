/* ------------------------------------------------------------------------ */
/*                OS/2 2.0 SLIP Driver for IBM TCP/IP v1.2.1                */
/*                               Version 1.0                                */
/* ------------------------------------------------------------------------ */

parse arg interface , dialcmd first last password

/* ------------------------------------------------------------------------ */
/*                   Initialization and Main Script Code                    */
/* ------------------------------------------------------------------------ */

/* Set some definitions for easier COM strings */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

langnum = '6'
cr='0d'x
crlf='0d0a'x

say ''
say 'PCBoard - SLIP Example Connection Script ',
    '(interface' interface')'

/* Prompt for missing information */
if dialcmd = '' then do
   call charout , 'Dial Command: '
   parse pull dialcmd
end
if first = '' | first = '*' then do
   call charout , 'First Name: '
   parse pull first
end
else do
   say 'First Name:' first
end
if last = '' | last = '*' then do
   call charout , 'Last Name: '
   parse pull last
end
else do
   say 'Last Name:' last
end
if password = '' | password = '*' then do
   call charout , 'Password: '
   password = readpass()
end

/* Flush any stuff left over from previous COM activity */
call flush_receive

/* Reset the modem here */
/* You may need to customize this for your modem make and model */
call lineout , 'Reset modem...'
call send 'AT' || cr
call waitfor 'OK', 5;   call flush_receive 'echo'
if RC = 1 then do
  call lineout , 'Modem not resetting...Trying again'
  call send '+++'
  call waitfor 'OK', 5
  call send 'ATHZ' || cr
  call waitfor 'OK', 3
end

/* Dial the remote server */
call charout, 'Now Dialing...'

/* Wait for connection */
call send dialcmd || cr
call waitfor 'CONNECT'  ; call waitfor crlf

/* Handle login.  We wait for standard strings, and then flush anything */
/* else to take care of trailing spaces, etc..                          */
/* call send cr */

call waitfor 'anguage', 5 ; call flush_receive 'echo'
call send langnum || cr
call waitfor 'graphics', 5; call flush_receive 'echo'
call send 'n;q;ns' || cr
call waitfor 'name?', 5;  call flush_receive 'echo'
call send first || cr
call waitfor 'name?', 5;  call flush_receive 'echo'
call send last || cr
call waitfor 'Password', 5;  call flush_receive 'echo'
call send password || cr
call waitfor 'IP address'
parse var waitfor_buffer . ') address is:' a '.' b '.' c '.' d '0d0a'x .
annex_address = a||'.'||b||'.'||c||'.'||d
call waitfor crlf
parse var waitfor_buffer  a '.' b '.' c '.' d '0d0a'x .
os2_address = a||'.'||b||'.'||c||'.'||d

call flush_receive 'echo'

'ifconfig sl0' os2_address 'netmask 255.255.255.0'
'route -f add default' annex_address '1'

say crlf || 'SLIP Connection Established'
say 'Configuring local address =' os2_address ', Annex =' annex_address

'ifconfig sl0' os2_address annex_address 'netmask 255.255.255.0'
'route add default' annex_address '1'

/* All done */
exit 0


/* ------------------------------------------------------------------------ */
/*                            send ( sendstring)                            */
/*..........................................................................*/
/*                                                                          */
/* Routine to send a character string off to the modem.                     */
/*                                                                          */
/* ------------------------------------------------------------------------ */

send:

   parse arg sendstring
   call slip_com_output interface , sendstring

   return

/* ------------------------------------------------------------------------ */
/*                          waitfor ( waitstring )                          */
/*..........................................................................*/
/*                                                                          */
/* Waits for the supplied string to show up in the COM input.  All input    */
/* from the time this function is called until the string shows up in the   */
/* input is accumulated in the "waitfor_buffer" variable.                   */
/*                                                                          */
/* ------------------------------------------------------------------------ */

waitfor:

   parse arg waitstring , timeout

   waitfor_buffer = '' ; done = 0 ; curpos = 1

   if (remain_buffer = 'REMAIN_BUFFER') then do
      remain_buffer = ''
   end

   do while done = 0
      if (remain_buffer \= '') then do
         line = remain_buffer
         remain_buffer = ''
      end
      else do
         line = slip_com_input(interface)
      end
      waitfor_buffer = waitfor_buffer || line
      index = pos(waitstring,waitfor_buffer)
      if (index > 0) then do
         remain_buffer = substr(waitfor_buffer,index+length(waitstring))
         waitfor_buffer = delstr(waitfor_buffer,index+length(waitstring))
         done = 1
      end
      call charout , substr(waitfor_buffer,curpos)
      curpos = length(waitfor_buffer)+1
    end

  return


/* ------------------------------------------------------------------------ */
/*                               readpass ()                                */
/*..........................................................................*/
/*                                                                          */
/* Routine used to read a password from the user without echoing the        */
/* password to the screen.                                                  */
/*                                                                          */
/* ------------------------------------------------------------------------ */

readpass:

  answer = ''
  do until key = cr
    key = slip_getch()
    if key \= cr then do
      answer = answer || key
    end
  end
  say ''
  return answer


/* ------------------------------------------------------------------------ */
/*                             flush_receive ()                             */
/*..........................................................................*/
/*                                                                          */
/* Routine to flush any pending characters to be read from the COM port.    */
/* Reads everything it can until nothing new shows up for 100ms, at which   */
/* point it returns.                                                        */
/*                                                                          */
/* The optional echo argument, if 1, says to echo flushed information.      */
/*                                                                          */
/* ------------------------------------------------------------------------ */

flush_receive:

   parse arg echo

   /* If echoing the flush - take care of waitfor remaining buffer */
   if (echo \= '') & (length(remain_buffer) > 0) then do
      call charout , remain_buffer
      remain_buffer = ''
   end

   /* Eat anything left in the modem or COM buffers */
   /* Stop when nothing new appears for 100ms.      */

   do until line = ''
     line = slip_com_input(interface,,100)
     if echo \= '' then
        call charout , line
   end

   return


/* ------------------------------------------------------------------------ */
/*    waitfor3 ( waitstring1 , waitstring2 , waitstring3)                   */
/*..........................................................................*/
/*                                                                          */
/* Waits for the supplied strings to show up in the COM input.  All input   */
/* from the time this function is called until the string shows up in the   */
/* input is accumulated in the "waitfor_buffer" variable.                   */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/* modified to accomodate three strings                                     */
/* this functions returns 1, 2, or 3 depending on which string was received */
/* first                                                                    */

waitfor3:

   parse arg waitstring1 , waitstring2 , waitstring3 , timeout

   waitfor_buffer = '' ; done = 0 ; curpos = 1

   if (remain_buffer = 'REMAIN_BUFFER') then do
      remain_buffer = ''
   end

   do while done = 0
      if (remain_buffer \= '') then do
         line = remain_buffer
         remain_buffer = ''
      end
      else do
         line = slip_com_input(interface)
      end
      waitfor_buffer = waitfor_buffer || line
      index1 = pos(waitstring1,waitfor_buffer)
      index2 = pos(waitstring2,waitfor_buffer)
      index3 = pos(waitstring3,waitfor_buffer)
      if (index1 > 0) then do
         remain_buffer = substr(waitfor_buffer,index1+length(waitstring1))
         waitfor_buffer = delstr(waitfor_buffer,index1+length(waitstring1))
         stringchosen = 1
         done = 1
      end
      else do
        if (index2 > 0) then do
                remain_buffer = substr(waitfor_buffer,index2+length(waitstring2))
                waitfor_buffer = delstr(waitfor_buffer,index2+length(waitstring2))
                stringchosen = 2
                done = 1
        end
        else do
                if (index3 > 0) then do
                remain_buffer = substr(waitfor_buffer,index3+length(waitstring3))
                waitfor_buffer = delstr(waitfor_buffer,index3+length(waitstring3))
                stringchosen = 3
                done = 1
                end
        end
      end

      call charout , substr(waitfor_buffer,curpos)
      curpos = length(waitfor_buffer)+1
    end

  return

/* ------------------------------------------------------------------------ */
/*    waitfor2 ( waitstring1 , waitstring2 )                                */
/*..........................................................................*/
/*                                                                          */
/* Waits for the supplied strings to show up in the COM input.  All input   */
/* from the time this function is called until the string shows up in the   */
/* input is accumulated in the "waitfor_buffer" variable.                   */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/* modified to accomodate a second string                                   */
/* this functions returns 1 or 2 depending on which string was received     */
/* first                                                                    */

waitfor2:

   parse arg waitstring1 , waitstring2 , timeout

   waitfor_buffer = '' ; done = 0 ; curpos = 1

   if (remain_buffer = 'REMAIN_BUFFER') then do
      remain_buffer = ''
   end

   do while done = 0
      if (remain_buffer \= '') then do
         line = remain_buffer
         remain_buffer = ''
      end
      else do
         line = slip_com_input(interface)
      end
      waitfor_buffer = waitfor_buffer || line
      index1 = pos(waitstring1,waitfor_buffer)
      index2 = pos(waitstring2,waitfor_buffer)
      if (index1 > 0) then do
         remain_buffer = substr(waitfor_buffer,index1+length(waitstring1))
         waitfor_buffer = delstr(waitfor_buffer,index1+length(waitstring1))
         stringchosen = 1
         done = 1
      end
      else do
        if (index2 > 0) then do
                remain_buffer = substr(waitfor_buffer,index2+length(waitstring2))
                waitfor_buffer = delstr(waitfor_buffer,index2+length(waitstring2))
                stringchosen = 2
                done = 1
        end
      end

      call charout , substr(waitfor_buffer,curpos)
      curpos = length(waitfor_buffer)+1
    end

  return

clarkslc@xmission.com /archive/os2 =->
