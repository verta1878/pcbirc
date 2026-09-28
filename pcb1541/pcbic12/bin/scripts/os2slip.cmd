/* ------------------------------------------------------------------------ */
/*                os/2 2.0 slip driver for ibm tcp/ip v1.2.1                */
/*                               version 1.0                                */
/* ------------------------------------------------------------------------ */

parse arg interface , dialcmd first last password

/* ------------------------------------------------------------------------ */
/*                   initialization and main script code                    */
/* ------------------------------------------------------------------------ */

/* set some definitions for easier com strings */
call rxfuncadd 'sysloadfuncs', 'rexxutil', 'sysloadfuncs'
call sysloadfuncs

langnum = '6'
cr='0d'x
crlf='0d0a'x

say ''
say 'pcboard - slip example connection script ',
    '(interface' interface')'

/* prompt for missing information */
if dialcmd = '' then do
   call charout , 'dial command: '
   parse pull dialcmd
end
if first = '' | first = '*' then do
   call charout , 'first name: '
   parse pull first
end
else do
   say 'first name:' first
end
if last = '' | last = '*' then do
   call charout , 'last name: '
   parse pull last
end
else do
   say 'last name:' last
end
if password = '' | password = '*' then do
   call charout , 'password: '
   password = readpass()
end

/* flush any stuff left over from previous com activity */
call flush_receive

/* reset the modem here */
/* you may need to customize this for your modem make and model */
call lineout , 'reset modem...'
call send 'at' || cr
call waitfor 'ok', 5;   call flush_receive 'echo'
if rc = 1 then do
  call lineout , 'modem not resetting...trying again'
  call send '+++'
  call waitfor 'ok', 5
  call send 'athz' || cr
  call waitfor 'ok', 3
end

/* dial the remote server */
call charout, 'now dialing...'

/* wait for connection */
call send dialcmd || cr
call waitfor 'connect'  ; call waitfor crlf

/* handle login.  we wait for standard strings, and then flush anything */
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
call waitfor 'password', 5;  call flush_receive 'echo'
call send password || cr
call waitfor 'ip address'
parse var waitfor_buffer . ') address is:' a '.' b '.' c '.' d '0d0a'x .
annex_address = a||'.'||b||'.'||c||'.'||d
call waitfor crlf
parse var waitfor_buffer  a '.' b '.' c '.' d '0d0a'x .
os2_address = a||'.'||b||'.'||c||'.'||d

call flush_receive 'echo'

'ifconfig sl0' os2_address 'netmask 255.255.255.0'
'route -f add default' annex_address '1'

say crlf || 'slip connection established'
say 'configuring local address =' os2_address ', annex =' annex_address

'ifconfig sl0' os2_address annex_address 'netmask 255.255.255.0'
'route add default' annex_address '1'

/* all done */
exit 0


/* ------------------------------------------------------------------------ */
/*                            send ( sendstring)                            */
/*..........................................................................*/
/*                                                                          */
/* routine to send a character string off to the modem.                     */
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
/* waits for the supplied string to show up in the com input.  all input    */
/* from the time this function is called until the string shows up in the   */
/* input is accumulated in the "waitfor_buffer" variable.                   */
/*                                                                          */
/* ------------------------------------------------------------------------ */

waitfor:

   parse arg waitstring , timeout

   waitfor_buffer = '' ; done = 0 ; curpos = 1

   if (remain_buffer = 'remain_buffer') then do
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
/* routine used to read a password from the user without echoing the        */
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
/* routine to flush any pending characters to be read from the com port.    */
/* reads everything it can until nothing new shows up for 100ms, at which   */
/* point it returns.                                                        */
/*                                                                          */
/* the optional echo argument, if 1, says to echo flushed information.      */
/*                                                                          */
/* ------------------------------------------------------------------------ */

flush_receive:

   parse arg echo

   /* if echoing the flush - take care of waitfor remaining buffer */
   if (echo \= '') & (length(remain_buffer) > 0) then do
      call charout , remain_buffer
      remain_buffer = ''
   end

   /* eat anything left in the modem or com buffers */
   /* stop when nothing new appears for 100ms.      */

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
/* waits for the supplied strings to show up in the com input.  all input   */
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

   if (remain_buffer = 'remain_buffer') then do
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
/* waits for the supplied strings to show up in the com input.  all input   */
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

   if (remain_buffer = 'remain_buffer') then do
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