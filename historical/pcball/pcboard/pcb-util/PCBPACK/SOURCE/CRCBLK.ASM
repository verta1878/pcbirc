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


;******************************************************************************
;* FILENAME: crcblk.asm                                                       *
;*                                                                            *
;* This module contains two functions for calculating CRC values              *
;*                                                                            *
;* unsigned short updblkcrc16(char far *block,int block_length,int crc)       *
;*   - Calculates a 16-bit CRC using the regular CRC-16 polynomial, not the   *
;*     CCITT polynomial                                                       *
;*                                                                            *
;* unsigned long updblkcrc32(char far *block,int len,unsigned long crc)       *
;*   - Calculates a 32-bit CRC using the CRC-32 polynomial.  The information  *
;*     about the 32-bit CRC calculations follows this header.                 *
;*                                                                            *
;* Author:  Matt Smith                                                        *
;* Copyright 1993 - Clark Development Company, Inc.                           *
;******************************************************************************
;
;
; The tables used to calculate the 16-bit CRC value were originally written
; by this guy:
;/***************************************************************************
; * updcrc macro derived from article Copyright (C) 1986 Stephen Satchell.
; *  NOTE: First srgument must be in range 0 to 255.
; *        Second argument is referenced twice.
; *
; * Programmers may incorporate any or all code into their programs,
; * giving proper credit within the source. Publication of the
; * source routines is permitted so long as proper credit is given
; * to Stephen Satchell, Satchell Evaluations and Chuck Forsberg,
; * Omen Technology.
; ***************************************************************************/
;
;
; The tables used to calculate the 32-bit CRC values were originally written
; by this guy:
;/***************************************************************************
; * Copyright (C) 1986 Gary S. Brown.  You may use this program, or
; * code or tables extracted from it, as desired without restriction.
; ***************************************************************************/
;/***************************************************************************
; * First, the polynomial itself and its table of feedback terms.  The
; * polynomial is
; * X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0
; * Note that we take it "backwards" and put the highest-order term in
; * the lowest-order bit.  The X^32 term is "implied"; the LSB is the
; * X^31 term, etc.  The X^0 term (usually shown as "+1") results in
; * the MSB being 1.
; *
; * Note that the usual hardware shift register implementation, which
; * is what we're using (we're merely optimizing it by doing eight-bit
; * chunks at a time) shifts bits into the lowest-order term.  In our
; * implementation, that means shifting towards the right.  Why do we
; * do it this way?  Because the calculated CRC must be transmitted in
; * order from highest-order term to lowest-order term.  UARTs transmit
; * characters in order from LSB to MSB.  By storing the CRC this way,
; * we hand it to the UART in the order low-byte to high-byte; the UART
; * sends each low-bit to hight-bit; and the result is transmission bit
; * by bit from highest- to lowest-order term without requiring any bit
; * shuffling on our part.  Reception works similarly.
; *
; * The feedback terms table consists of 256, 32-bit entries.  Notes:
; *
; *     The table can be generated at runtime if desired; code to do so
; *     is shown later.  It might not be obvious, but the feedback
; *     terms simply represent the results of eight shift/xor opera-
; *     tions for all combinations of data and CRC register values.
; *
; *     The values must be right-shifted by eight bits by the "updcrc"
; *     logic; the shift must be unsigned (bring in zeroes).  On some
; *     hardware you could probably optimize the shift in assembler by
; *     using byte-swap instructions.
; ***************************************************************************/
;/*** CRC polynomial 0xEDB88320 *********************************************/

ifdef __s__
  .model small,c
elseifdef __m__
  .model medium,c
elseifdef __c__
  .model compact,c
elseifdef __l__
  .model large,c
else
  .model large,c
endif


.data

crc16tab        dw      00000h, 01021h, 02042h, 03063h, 04084h, 050A5h, 060C6h
                dw      070E7h, 08108h, 09129h, 0A14Ah, 0B16Bh, 0C18Ch, 0D1ADh
                dw      0E1CEh, 0F1EFh, 01231h, 00210h, 03273h, 02252h, 052B5h
                dw      04294h, 072F7h, 062D6h, 09339h, 08318h, 0B37Bh, 0A35Ah
                dw      0D3BDh, 0C39Ch, 0F3FFh, 0E3DEh, 02462h, 03443h, 00420h
                dw      01401h, 064E6h, 074C7h, 044A4h, 05485h, 0A56Ah, 0B54Bh
                dw      08528h, 09509h, 0E5EEh, 0F5CFh, 0C5ACh, 0D58Dh, 03653h
                dw      02672h, 01611h, 00630h, 076D7h, 066F6h, 05695h, 046B4h
                dw      0B75Bh, 0A77Ah, 09719h, 08738h, 0F7DFh, 0E7FEh, 0D79Dh
                dw      0C7BCh, 048C4h, 058E5h, 06886h, 078A7h, 00840h, 01861h
                dw      02802h, 03823h, 0C9CCh, 0D9EDh, 0E98Eh, 0F9AFh, 08948h
                dw      09969h, 0A90Ah, 0B92Bh, 05AF5h, 04AD4h, 07AB7h, 06A96h
                dw      01A71h, 00A50h, 03A33h, 02A12h, 0DBFDh, 0CBDCh, 0FBBFh
                dw      0EB9Eh, 09B79h, 08B58h, 0BB3Bh, 0AB1Ah, 06CA6h, 07C87h
                dw      04CE4h, 05CC5h, 02C22h, 03C03h, 00C60h, 01C41h, 0EDAEh
                dw      0FD8Fh, 0CDECh, 0DDCDh, 0AD2Ah, 0BD0Bh, 08D68h, 09D49h
                dw      07E97h, 06EB6h, 05ED5h, 04EF4h, 03E13h, 02E32h, 01E51h
                dw      00E70h, 0FF9Fh, 0EFBEh, 0DFDDh, 0CFFCh, 0BF1Bh, 0AF3Ah
                dw      09F59h, 08F78h, 09188h, 081A9h, 0B1CAh, 0A1EBh, 0D10Ch
                dw      0C12Dh, 0F14Eh, 0E16Fh, 01080h, 000A1h, 030C2h, 020E3h
                dw      05004h, 04025h, 07046h, 06067h, 083B9h, 09398h, 0A3FBh
                dw      0B3DAh, 0C33Dh, 0D31Ch, 0E37Fh, 0F35Eh, 002B1h, 01290h
                dw      022F3h, 032D2h, 04235h, 05214h, 06277h, 07256h, 0B5EAh
                dw      0A5CBh, 095A8h, 08589h, 0F56Eh, 0E54Fh, 0D52Ch, 0C50Dh
                dw      034E2h, 024C3h, 014A0h, 00481h, 07466h, 06447h, 05424h
                dw      04405h, 0A7DBh, 0B7FAh, 08799h, 097B8h, 0E75Fh, 0F77Eh
                dw      0C71Dh, 0D73Ch, 026D3h, 036F2h, 00691h, 016B0h, 06657h
                dw      07676h, 04615h, 05634h, 0D94Ch, 0C96Dh, 0F90Eh, 0E92Fh
                dw      099C8h, 089E9h, 0B98Ah, 0A9ABh, 05844h, 04865h, 07806h
                dw      06827h, 018C0h, 008E1h, 03882h, 028A3h, 0CB7Dh, 0DB5Ch
                dw      0EB3Fh, 0FB1Eh, 08BF9h, 09BD8h, 0ABBBh, 0BB9Ah, 04A75h
                dw      05A54h, 06A37h, 07A16h, 00AF1h, 01AD0h, 02AB3h, 03A92h
                dw      0FD2Eh, 0ED0Fh, 0DD6Ch, 0CD4Dh, 0BDAAh, 0AD8Bh, 09DE8h
                dw      08DC9h, 07C26h, 06C07h, 05C64h, 04C45h, 03CA2h, 02C83h
                dw      01CE0h, 00CC1h, 0EF1Fh, 0FF3Eh, 0CF5Dh, 0DF7Ch, 0AF9Bh
                dw      0BFBAh, 08FD9h, 09FF8h, 06E17h, 07E36h, 04E55h, 05E74h
                dw      02E93h, 03EB2h, 00ED1h, 01EF0h

crc32tab        dd      000000000h, 077073096h, 0EE0E612Ch, 0990951BAh
                dd      0076DC419h, 0706AF48Fh, 0E963A535h, 09E6495A3h
                dd      00EDB8832h, 079DCB8A4h, 0E0D5E91Eh, 097D2D988h
                dd      009B64C2Bh, 07EB17CBDh, 0E7B82D07h, 090BF1D91h
                dd      01DB71064h, 06AB020F2h, 0F3B97148h, 084BE41DEh
                dd      01ADAD47Dh, 06DDDE4EBh, 0F4D4B551h, 083D385C7h
                dd      0136C9856h, 0646BA8C0h, 0FD62F97Ah, 08A65C9ECh
                dd      014015C4Fh, 063066CD9h, 0FA0F3D63h, 08D080DF5h
                dd      03B6E20C8h, 04C69105Eh, 0D56041E4h, 0A2677172h
                dd      03C03E4D1h, 04B04D447h, 0D20D85FDh, 0A50AB56Bh
                dd      035B5A8FAh, 042B2986Ch, 0DBBBC9D6h, 0ACBCF940h
                dd      032D86CE3h, 045DF5C75h, 0DCD60DCFh, 0ABD13D59h
                dd      026D930ACh, 051DE003Ah, 0C8D75180h, 0BFD06116h
                dd      021B4F4B5h, 056B3C423h, 0CFBA9599h, 0B8BDA50Fh
                dd      02802B89Eh, 05F058808h, 0C60CD9B2h, 0B10BE924h
                dd      02F6F7C87h, 058684C11h, 0C1611DABh, 0B6662D3Dh
                dd      076DC4190h, 001DB7106h, 098D220BCh, 0EFD5102Ah
                dd      071B18589h, 006B6B51Fh, 09FBFE4A5h, 0E8B8D433h
                dd      07807C9A2h, 00F00F934h, 09609A88Eh, 0E10E9818h
                dd      07F6A0DBBh, 0086D3D2Dh, 091646C97h, 0E6635C01h
                dd      06B6B51F4h, 01C6C6162h, 0856530D8h, 0F262004Eh
                dd      06C0695EDh, 01B01A57Bh, 08208F4C1h, 0F50FC457h
                dd      065B0D9C6h, 012B7E950h, 08BBEB8EAh, 0FCB9887Ch
                dd      062DD1DDFh, 015DA2D49h, 08CD37CF3h, 0FBD44C65h
                dd      04DB26158h, 03AB551CEh, 0A3BC0074h, 0D4BB30E2h
                dd      04ADFA541h, 03DD895D7h, 0A4D1C46Dh, 0D3D6F4FBh
                dd      04369E96Ah, 0346ED9FCh, 0AD678846h, 0DA60B8D0h
                dd      044042D73h, 033031DE5h, 0AA0A4C5Fh, 0DD0D7CC9h
                dd      05005713Ch, 0270241AAh, 0BE0B1010h, 0C90C2086h
                dd      05768B525h, 0206F85B3h, 0B966D409h, 0CE61E49Fh
                dd      05EDEF90Eh, 029D9C998h, 0B0D09822h, 0C7D7A8B4h
                dd      059B33D17h, 02EB40D81h, 0B7BD5C3Bh, 0C0BA6CADh
                dd      0EDB88320h, 09ABFB3B6h, 003B6E20Ch, 074B1D29Ah
                dd      0EAD54739h, 09DD277AFh, 004DB2615h, 073DC1683h
                dd      0E3630B12h, 094643B84h, 00D6D6A3Eh, 07A6A5AA8h
                dd      0E40ECF0Bh, 09309FF9Dh, 00A00AE27h, 07D079EB1h
                dd      0F00F9344h, 08708A3D2h, 01E01F268h, 06906C2FEh
                dd      0F762575Dh, 0806567CBh, 0196C3671h, 06E6B06E7h
                dd      0FED41B76h, 089D32BE0h, 010DA7A5Ah, 067DD4ACCh
                dd      0F9B9DF6Fh, 08EBEEFF9h, 017B7BE43h, 060B08ED5h
                dd      0D6D6A3E8h, 0A1D1937Eh, 038D8C2C4h, 04FDFF252h
                dd      0D1BB67F1h, 0A6BC5767h, 03FB506DDh, 048B2364Bh
                dd      0D80D2BDAh, 0AF0A1B4Ch, 036034AF6h, 041047A60h
                dd      0DF60EFC3h, 0A867DF55h, 0316E8EEFh, 04669BE79h
                dd      0CB61B38Ch, 0BC66831Ah, 0256FD2A0h, 05268E236h
                dd      0CC0C7795h, 0BB0B4703h, 0220216B9h, 05505262Fh
                dd      0C5BA3BBEh, 0B2BD0B28h, 02BB45A92h, 05CB36A04h
                dd      0C2D7FFA7h, 0B5D0CF31h, 02CD99E8Bh, 05BDEAE1Dh
                dd      09B64C2B0h, 0EC63F226h, 0756AA39Ch, 0026D930Ah
                dd      09C0906A9h, 0EB0E363Fh, 072076785h, 005005713h
                dd      095BF4A82h, 0E2B87A14h, 07BB12BAEh, 00CB61B38h
                dd      092D28E9Bh, 0E5D5BE0Dh, 07CDCEFB7h, 00BDBDF21h
                dd      086D3D2D4h, 0F1D4E242h, 068DDB3F8h, 01FDA836Eh
                dd      081BE16CDh, 0F6B9265Bh, 06FB077E1h, 018B74777h
                dd      088085AE6h, 0FF0F6A70h, 066063BCAh, 011010B5Ch
                dd      08F659EFFh, 0F862AE69h, 0616BFFD3h, 0166CCF45h
                dd      0A00AE278h, 0D70DD2EEh, 04E048354h, 03903B3C2h
                dd      0A7672661h, 0D06016F7h, 04969474Dh, 03E6E77DBh
                dd      0AED16A4Ah, 0D9D65ADCh, 040DF0B66h, 037D83BF0h
                dd      0A9BCAE53h, 0DEBB9EC5h, 047B2CF7Fh, 030B5FFE9h
                dd      0BDBDF21Ch, 0CABAC28Ah, 053B39330h, 024B4A3A6h
                dd      0BAD03605h, 0CDD70693h, 054DE5729h, 023D967BFh
                dd      0B3667A2Eh, 0C4614AB8h, 05D681B02h, 02A6F2B94h
                dd      0B40BBE37h, 0C30C8EA1h, 05A05DF1Bh, 02D02EF8Dh

.code

;***************************************************************************
;* unsigned short updblkcrc16(char far *block,int block_length,int crc)    *
;*                                                                         *
;* Performs a 16-bit CRC using the CRC-16 polynomial.                      *
;* It works the same way as the following C-code it was written from       *
;*                                                                         *
;* unsigned short  updcrc16(int cp, unsigned short crc)                    *
;*  {                                                                      *
;*    return (crc16tab[((crc >> 8) & 0xFF)] ^ (crc << 8) ^ cp);            *
;*  }                                                                      *
;*                                                                         *
;***************************************************************************

                public  updblkcrc16
updblkcrc16     PROC    block:far ptr,blklength:word,precondition:word
                uses    bx,cx,dx,ds,es,si,di

                lds     si,block                ; ds:si = pointer to block
                mov     cx,blklength            ; length of block to calc
                mov     dx,precondition         ; dx = running CRC value
                mov     ax,SEG crc16tab
                mov     es,ax
                mov     di,OFFSET crc16tab      ; es:di = table
                cld
charloop:       lodsb
                cbw                             ; get byte
                mov     bx,dx                   ; dx= current CRC value
                mov     bh,bl                   ; bx=crc<<8
                xor     bl,bl
                xor     ax,bx                   ; ax=(crc<<8)^cp
                mov     bx,dx
                mov     bl,bh
                xor     bh,bh
                and     bx,0ffh
                shl     bx,1
                push    cx
                mov     cx,ax                   ; cx=(crc<<8)^cp
                mov     ax,es:[di+bx]           ; load table value
                xor     ax,cx
                pop     cx
                mov     dx,ax
                loop    charloop
                ret
                ENDP

;***************************************************************************
;* unsigned long updblkcrc32(char far *block,int len,unsigned long crc)    *
;* Performs a 32-bit CRC using the CRC-32 polynomial.                      *
;*                                                                         *
;* It works the same way as the following C-code it was written from       *
;* unsigned long   updcrc32(int b, unsigned long c)                        *
;*  {                                                                      *
;*     return (crc32tab[((int)c ^ b) & 0xFF] ^ ((c >> 8) & 0x00FFFFFFL));  *
;*  {                                                                      *
;*                                                                         *
;***************************************************************************

                public  updblkcrc32
updblkcrc32     PROC    block:far ptr,blklength:word,precondition:dword
                USES    bx,cx,ds,es,di,si

                lds     si,block                        ; ds:si = block
                mov     cx,blklength                    ; cx = count
                mov     ax,SEG crc32tab
                mov     es,ax
                mov     di,OFFSET crc32tab              ; es:di = table
                mov     ax,WORD PTR [precondition]
                mov     dx,WORD PTR[precondition+2]     ; dx:ax = CRC

charloop2:      mov     bx,ax
                mov     al,ah
                mov     ah,dl
                mov     dl,dh
                xor     dh,dh                   ; dx:ax = ((c>>8)&0x00FFFFFFL))
                push    dx
                push    ax                      ; push onto stack
                lodsb
                cbw
                xor     bx,ax
                and     bx,0ffh
                shl     bx,1
                shl     bx,1
                mov     dx,es:[di+bx+2]         ; dx:ax=table[((int)c^b)&0xFF]
                mov     ax,es:[di+bx]
                pop     bx
                xor     ax,bx                   ; XOR everything together
                pop     bx
                xor     dx,bx
                dec     cx
                jz      getout
                jmp     charloop2
getout:         ret
                ENDP

        END
