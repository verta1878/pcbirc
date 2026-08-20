;
;This is an information file that the sio2k set of drivers uses to
;identify PCI serial cards.  This file, and its structure may change, so do
;not depent on it staying the same.  You may add additional cards (lines)
;to the file if you feel capable.
;
;The first item in a line must be a quoted (" ") string.  You can use
;either single quote marks (' ') or double quote marks (" ").  The quoted
;string is a descripition of the board.  The description need NOT be exact
;as it is NOT used in the board identification process.  The quoted string
;is only used for log file entries etc.
;
;The second group of items are in the form HexNumber=NexNumber, called
;conditions.  The HexNumber on the left of the equal (=) sign is an index
;into the PCI boards configuration data, often called an offset or
;registers.  The HexNumber to the right of the equal sign is the value that
;must  exist in the PCI board's configuration data at the given offset.
;There can be up to 5 condition pairs, and I believe at least two are
;needed to correctly identify a PCI card.
;
;A note about hexidecimal numbers (HexNumber).  If the HexNumber begins
;with an alphabetic letter (A through F), then add a leading 0 (zero) to the
;number.  For example, the HexNumber a1 (or A1) should be entered as 0a1 or
;0A1.
;
;Another note, the equal (=) sign and at (@) sign can be used interchangeably.
;The use of the at (@) sign may make the file a little more readable.
;
;Following the (up to 5) condition pairs, is the UART@HexNumber items.
;One UART@HexNumber is ALWAYS REQUIRED.  This item specifies the offset in
;the PCI board's configuration data that contains the I/O address of a uart
;on the board.  If your PCI card only has one uart, no futher data is needed
;on that line.  You will find the I/O port addresses between offsets 0x10
;and 0x28.  The I/O port addresses always end with a 1.
;
;Any number of UART@ (pronounced "uart at") items can appear on the line
;of data that identifies a PCI card.  The UART@ item has two possible
;forms.  The first form is explained in the previous paragraph.  That is,
;the HexNumber to the right of the "at sign" specifies the offset of the
;I/O address for the a uart.  The SECOND form of the UART@ item is
;UART@+HexNumber.   In the second form the plus sign (+) preceeds the
;HexNumber.  The plus sign identifies the HexNumber as being an increment
;and NOT an offset.  When an increment is used, the I/O address of the uart
;is relative to the I/O address of the preceeding uart.  For example,
;if one specifies UART@+8, then 8 would be added to the I/O address of the
;preceeding uart to obtain the I/O address of uart.
;
;Finally, you can use single or double plus signs to specify a number of
;additional uarts following a previous UART@+8 item.  The immediately
;preceeding UART@ item MUST specify an increment, ie UART@+8, which provides
;the incremental value used to generate the I/O address of the uart.
;
;For example + + (two pluses separated by a space) means the board contains
;two additional uarts.  Four pluses separated by spaces (+ + + +) would
;mean the board has 4 additional uarts.  The double plus (++), being two
;plus signs without a space between them, means the drivers are to probe
;for the number of uarts.  The double plus is useful for boards that have
;identical PCI configuration information, but may have a different number
;of uarts.
;
;There is a file named PCI.EXE in the sio2k distribution zip. PCI.EXE is
;a DOS program.  To use PCI.EXE open a DOS session and execute the program
;(type PCI at a DOS command prompt), then hit F5. This will create a file
;named PCI_REGS.DAT in the current directory.  You can examine the values
;in the PCI registers (offsets) to generate your own PCI config lines.
;
;If all else fails, zip the PCI_REGS.DAT file and send it to me at
;sio2k@gwinn.com.  I will attempt to generate the addition to PCI.INC to
;support your card.  Be SURE to send the file as an ATTACHMENT.  DO NOT send
;the file as text included in your message because either your mail program,
;or my mail program will mangle the data into an unreadable form.
;
; In addition to the PCI_REGS.DAT file, I need to know:
;      1 - Who manufactures the card, and what they call the card.
;      2 - How many serial ports are on the board.
;      3 - How many parallel ports are on the board (if any).
;      4 - The I/O ports if you know them, possibly from Windows.
;      5 - Anything else that you may think is useful.

"CTI Blue Heat" 00=11b0 2c=12c4 uart@14 uart@+8 ++
"GTEK JetPort" 00=10b5 2e=a001 uart@18 uart@1c
"GTEK JetPort" 00=10b5 2e=c001 uart@1c uart@+8 ++
"VScom 200S" 00=10b5 02=1103 uart@18 uart@1c
"VScom 210S" 00=10b5 02=1078 uart@18 uart@1c
"VScom 400S" 00=10b5 02=1077 uart@18 uart@+8 + +
"VScom 800S" 00=10b5 02=1076 uart@18 uart@+8 ++
"VScom 100L" 00=14D2 02=8010 uart@14
"VScom 110L" 00=14D2 02=8011 uart@14
"VScom 200L" 00=14D2 02=8020 uart@14 uart@18
"VScom 210L" 00=14D2 02=8021 uart@14 uart@18
"VScom 100H" 00=14D2 02=a001 uart@10
"VScom 200H" 00=14D2 02=a005 uart@10 uart@+8
"VScom 400H" 00=14D2 02=a003 uart@10 uart@+8 + +
"VScom 800HF1" 00=14D2 02=a004 uart@10 uart@+8 + +
"VScom 100HV2" 00=14D2 02=e010 uart@10
"VScom 200HV2" 00=14D2 02=e020 uart@10 uart@14
"VScom 400L" 00=14D2 02=8040 uart@14 uart@18 uart@20 uart@+8
"VScom 800L" 00=14D2 02=8080 uart@14 uart@18 uart@20 uart@+8 + + + +
"SIIG 2P1S PCI 550" 00=131f 02=2040 uart@10
"SIIG 2P1S PCI 650" 00=131f 02=2041 uart@10
"SIIG 2P1S PCI 850" 00=131f 02=2042 uart@10
"SIIG Cyber Serial PCI 550" 00=131f 02=2000 uart@10
"SIIG Cyber Serial PCI 650" 00=131f 02=2001 uart@10
"SIIG Cyber Serial PCI 850" 00=131f 02=2002 uart@10
"SIIG Cyber I/O PCI 550" 00=131f 02=2010 uart@10
"SIIG Cyber I/O PCI 650" 00=131f 02=2011 uart@10
"SIIG Cyber I/O PCI 850" 00=131f 02=2012 uart@10
"SIIG Cyber Serial Dual PCI 550" 00=131f 02=2030 uart@10 uart@14
"SIIG Cyber Serial Dual PCI 650" 00=131f 02=2031 uart@10 uart@14
"SIIG Cyber Serial Dual PCI 850" 00=131f 02=2032 uart@10 uart@14
"SIIG Cyber 2S1P PCI 550" 00=131f 02=2060 uart@10 uart@14
"SIIG Cyber 2S1P PCI 650" 00=131f 02=2061 uart@10 uart@14
"SIIG Cyber 2S1P PCI 850" 00=131f 02=2062 uart@10 uart@14
"SIIG Cyber 4S PCI 550" 00=131f 02=2050 uart@10 uart@14 uart@18 uart@1c
"SIIG Cyber 4S PCI 650" 00=131f 02=2051 uart@10 uart@14 uart@18 uart@1c
"SIIG Cyber 4S PCI 850" 00=131f 02=2052 uart@10 uart@14 uart@18 uart@1c

;The following line was contributed by François Jacques
"3COM/US Robotics 56K Voice Modem (3CP-2976-OEM-50)" 00=12b9 02=1008 uart@10

"Rayon P584" 00=10b5 02=9050 uart@18 uart@+8 ++
"ActionTec Internal PCI" 00=11c1 02=0480 uart@14

;The following line was contributed by Igor K. Zhitko
"Moxa C104H/PCI" 00=1393 02=1040 uart@18 uart@+8 ++

;The following line was contributed by Alex Slyotov
"Moxa C168H/PCI" 00=1393 02=1680 uart@18 uart@+8 ++

; Card sold from Hama in Germany, contributed by Gerd Thier
"SUNIX PCI 4036A" 00=1409 02=7168 uart1@10 uart2=+8 ++

;The following line was contributed by YANO Takashi
"ActionTec Internal PCI" 00=11c1 02=0480 uart@14

