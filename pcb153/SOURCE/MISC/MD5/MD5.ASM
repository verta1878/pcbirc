PAGE 58,132

TITLE MD5 Message-Digest Algorithm

	name	MD5

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
;       M    M  DDDDD   55555
;       MM  MM  D    D  5
;       M MM M  D    D  5555
;       M    M  D    D      5
;       M    M  DDDDD   5555
;
;
;	Assembled MASM 6.0.
;
;	This program is based on information obtained from RFC 1321 and is
;	decicated to the public domain.  Credit given to the author would
;	be nice.
;
;	Fully reentrant, all local variables are created on the stack. 
;
;	Written by Ray Gwinn, January 1995.
;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ

	option	OldStructs	;use old structure types
	option	NoScoped	;don't scope labels in proc

Function	TypeDef Proto Far Pascal
Consts	Struc
s	db	?	;The s value for the calculation
k	db	?	;The k value for the calculation
tsubi	dd	?	;The value of T[i] for the calculation
Consts	ends

Variables	struc
Isa		dd	?
Isb		dd	?
Isc		dd	?
Isd		dd	?
XValsPtr	dd	?
RoundConsts	dd	?	;ptr to the Consts structure for active round
OrigSize	dd	?	;original size of the file/string
Variables	ends

.386

code	segment	public use16 'code'
; Round 1	 s	k	T[i]
r1_0	Consts	<7,	0,	0d76aa478h>
	Consts	<12,	1,	0e8c7b756h>
	Consts	<17,	2,	0242070dbh>
	Consts	<22,	3,	0c1bdceeeh>
	Consts	<7,	4,	0f57c0fafh>
	Consts	<12,	5,	04787c62ah>
	Consts	<17,	6,	0a8304613h>
	Consts	<22,	7,	0fd469501h>
	Consts	<7,	8,	0698098d8h>
	Consts	<12,	9,	08b44f7afh>
	Consts	<17,	10,	0ffff5bb1h>
	Consts	<22,	11,	0895cd7beh>
	Consts	<7,	12,	06b901122h>
	Consts	<12,	13,	0fd987193h>
	Consts	<17,	14,	0a679438eh>
	Consts	<22,	15,	049b40821h>

; Round 2	 s	k	T[i]
	Consts	<5,	1,	0f61e2562h>
	Consts	<9,	6,	0c040b340h>
	Consts	<14,	11,	0265e5a51h>
	Consts	<20,	0,	0e9b6c7aah>
	Consts	<5,	5,	0d62f105dh>
	Consts	<9,	10,	02441453h>
	Consts	<14,	15,	0d8a1e681h>
	Consts	<20,	4,	0e7d3fbc8h>
	Consts	<5,	9,	021e1cde6h>
	Consts	<9,	14,	0c33707d6h>
	Consts	<14,	3,	0f4d50d87h>
	Consts	<20,	8,	0455a14edh>
	Consts	<5,	13,	0a9e3e905h>
	Consts	<9,	2,	0fcefa3f8h>
	Consts	<14,	7,	0676f02d9h>
	Consts	<20,	12,	08d2a4c8ah>

; Round 3	 s	k	T[i]
	Consts	<4,	5,	0fffa3942h>
	Consts	<11,	8,	08771f681h>
	Consts	<16,	11,	06d9d6122h>
	Consts	<23,	14,	0fde5380ch>
	Consts	<4,	1,	0a4beea44h>
	Consts	<11,	4,	04bdecfa9h>
	Consts	<16,	7,	0f6bb4b60h>
	Consts	<23,	10,	0bebfbc70h>
	Consts	<4,	13,	0289b7ec6h>
	Consts	<11,	0,	0eaa127fah>
	Consts	<16,	3,	0d4ef3085h>
	Consts	<23,	6,	04881d05h>
	Consts	<4,	9,	0d9d4d039h>
	Consts	<11,	12,	0e6db99e5h>
	Consts	<16,	15,	01fa27cf8h>
	Consts	<23,	2,	0c4ac5665h>

; Round 4	 s	k	T[i]
	Consts	<6,	0,	0f4292244h>
	Consts	<10,	7,	0432aff97h>
	Consts	<15,	14,	0ab9423a7h>
	Consts	<21,	5,	0fc93a039h>
	Consts	<6,	12,	0655b59c3h>
	Consts	<10,	3,	08f0ccc92h>
	Consts	<15,	10,	0ffeff47dh>
	Consts	<21,	1,	085845dd1h>
	Consts	<6,	8,	06fa87e4fh>
	Consts	<10,	15,	0fe2ce6e0h>
	Consts	<15,	6,	0a3014314h>
	Consts	<21,	13,	04e0811a1h>
	Consts	<6,	4,	0f7537e82h>
	Consts	<10,	11,	0bd3af235h>
	Consts	<15,	2,	02ad7d2bbh>
	Consts	<21,	9,	0eb86d391h>

@curseg ends

code	segment	public use16 'code'
	assume	cs:@curseg, ds:nothing, es:nothing, ss:nothing
	assume	fs:nothing, gs:nothing

F	Proc Far Pascal	uses edx
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;	MD5's F function
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;
	mov	eax,fs:[bx].Isb		;get x
	and	eax,fs:[bx].Isc		;x and y

	mov	edx,fs:[bx].Isb		;get x
	not	edx			;not(x)
	and	edx,fs:[bx].Isd		;not(x) and z
	or	eax,edx			;(x and y) or (not(x) and z)
	ret

F	endp

G	Proc Far Pascal	uses edx
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;	MD5's G function
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;
	mov	eax,fs:[bx].Isb		;get x
	and	eax,fs:[bx].Isd		;x and z

	mov	edx,fs:[bx].Isd		;get z
	not	edx			;not(z)
	and	edx,fs:[bx].Isc		;not(z) and y
	or	eax,edx			;(x and z) or (not(z) and y)
	ret

G	endp

H	Proc Far Pascal
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;	MD5's H function
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;
	mov	eax,fs:[bx].Isb
	xor	eax,fs:[bx].Isc
	xor	eax,fs:[bx].Isd
	ret

H	endp

I	Proc Far Pascal
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;	MD5's I function
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;
	mov	eax,fs:[bx].Isd
	not	eax
	or	eax,fs:[bx].Isb
	xor	eax,fs:[bx].Isc
	ret

I	endp

Calc	Proc Far Pascal	uses ds es,
	FunctionPtr:Far Ptr Function,
	ConstPtr:dword
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;	a = b + ((a + Function(b,c,d) + X[k] + T[i]) <<< s)
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;
	pushad

	Invoke	FunctionPtr		;Function F, G, H, or I(b,c,d)

	add	eax,fs:[bx].Isa		;a + f(b,c,d)
	lds	di,ConstPtr
	add	eax,[di].tsubi		;a + f(b,c,d) + T[i]
	mov	cl,[di].s		;get s to cl
	mov	ch,[di].k		;get k to ch
	les	di,fs:[bx].XValsPtr	;address of message words
	movzx	edi,di			;expand edi to dword
	push	bx
	movzx	ebx,ch			;get k to ebx
	add	eax,es:[ebx*4+edi]	;a + f(b,c,d) + X[k] + T[i]
	pop	bx
	rol	eax,cl			;a + f(b,c,d) + X[k] + T[i] <<< s
	add	eax,fs:[bx].Isb		;b + (a + f(b,c,d) + X[k] + T[i] <<< s)

	mov	fs:[bx].Isa,eax		;store result

	popad
	ret

Calc	endp

Round	Proc Far Pascal,
	FunctionPtr:Far Ptr Function
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;	Do calculation loop
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;
	pushad

	mov	cx,16
cal30:	Invoke	Calc,
		FunctionPtr,		;function routine address
		fs:[bx].RoundConsts	;address of round constants

	add	fs:[bx].RoundConsts,size Consts

	mov	edx,fs:[bx].Isd		;rotate a, b, c, d
	mov	eax,fs:[bx].Isc
	mov	fs:[bx].Isd,eax		;d=c
	mov	eax,fs:[bx].Isb
	mov	fs:[bx].Isc,eax		;c=b
	mov	eax,fs:[bx].Isa
	mov	fs:[bx].Isb,eax		;b=a
	mov	fs:[bx].Isa,edx		;a=d
	loop	cal30

	popad
	ret

Round	endp

Rounds	Proc Far Pascal
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;	Do a round of calculations
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;
	pushad

	mov	eax,fs:[bx].Isa
	mov	esi,fs:[bx].Isb
	mov	ecx,fs:[bx].Isc
	mov	edx,fs:[bx].Isd
	mov	word ptr fs:[bx].RoundConsts, offset r1_0
	mov	word ptr fs:[bx].RoundConsts+2, seg r1_0

	Invoke	Round,
		addr F

	Invoke	Round,
		addr G

	Invoke	Round,
		addr H

	Invoke	Round,
		addr I

	add	fs:[bx].Isa,eax
	add	fs:[bx].Isb,esi
	add	fs:[bx].Isc,ecx
	add	fs:[bx].Isd,edx
	popad
	ret

Rounds	endp

DoPad	Proc Far Pascal	uses ds es
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;	Do the padding speced by the MD5 spec
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;
	Local	additional[64]:byte

	push	ss			;get ss
	pop	es			;to es
	lea	di,additional		;es:di points to bufr on sk
	lds	si,fs:[bx].XValsPtr	;ds:si points to remaining bytes
	mov	word ptr fs:[bx].XValsPtr,di
	mov	word ptr fs:[bx].XValsPtr+2,es

	mov	edx,fs:[bx].OrigSize	;get bytes in the message
	and	dx,(64-1)		;excess above mod 64 bytes (512 bits)
	jz	dp20			;br if no excess

	mov	cx,dx			;bytes to move to cx
	rep movsb			;move bytes to the pad buffer
dp20:	mov	al,80h			;required 80h pad character
	stosb
	inc	dx			;bump bytes in the temp bufr
dp30:	mov	cx,64			;max bytes for the block
	sub	cx,dx			;number of bytes to zero to cx
	jcxz	dp40			;br if no zeroing needed

	xor	al,al			;al=0
	rep stosb			;zero to the end of the pad buffer
dp40:	cmp	dx,56			;test if room for original count
	jbe	dp50			;br if room exists

	Invoke	Rounds			;process sixteen 32 bit words
	lea	di,additional		;es:di points to bufr on sk
	xor	dx,dx			;nothing in the buffer
	jmp	dp30			;go start again with empty bufr

dp50:	mov	eax,fs:[bx].OrigSize	;get bytes in the message
	xor	edx,edx
	shld	edx,eax,3		;get high 3 bits to edx (does not change eax)
	shl	eax,3			;edx:eax contains string count in bits
	mov	dword ptr additional[56],eax	;low 32 bits of the count
	mov	dword ptr additional[60],edx	;high 32 bits of the count
	Invoke	Rounds			;process sixteen 32 bit words

	ret

DoPad	endp

		public	md5String
md5String	Proc Far Pascal	uses ds es fs,
		StringPtr:dword,	;points to the string
		Count:word,		;bytes in the string
		ResultPtr:dword		;points to 16 byte bufr for result
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;	MD5 Message-Digest Algorithm on a given string
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;
	Local	Var:Variables

	pushad
	pushfd

	cld
	push	ss			;get ss
	pop	fs			;to fs
	lea	bx,Var			;fs:bx points to the Variables struc
	mov	Var.Isa,067452301h	;initial value for a
	mov	Var.Isb,0efcdab89h	;initial value for b
	mov	Var.Isc,098badcfeh	;initial value for c
	mov	Var.Isd,010325476h	;initial value for d

	movzx	eax,count		;get bytes in the message
	mov	Var.OrigSize,eax	;save original size
	mov	eax,StringPtr		;address of the string
	mov	Var.XValsPtr,eax	;store address of the string

	mov	cx,count		;get bytes in the message
	shr	cx,6			;number of 16 byte groups
	jcxz	ms40			;br if string is less than 64 bytes

ms30:	Invoke	Rounds			;process sixteen 32 bit words
	add	word ptr Var.XValsPtr,64	;bump to next 64 byte group
	loop	ms30			;loop till all bytes processed

ms40:	Invoke	DoPad			;go do the padding

	lds	si,ResultPtr		;load result address
	mov	eax,Var.Isa		;get result value
	mov	[si],eax		;and store
	mov	eax,Var.Isb		;get result value
	mov	[si+4],eax		;and store
	mov	eax,Var.Isc		;get result value
	mov	[si+8],eax		;and store
	mov	eax,Var.Isd		;get result value
	mov	[si+12],eax		;and store

	popfd
	popad

	ret

md5String	endp

@curseg ends

;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ
; THE FOLLOWING CODE IS UNIQUE TO OS/2 V2 AND UP
;ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ

code32	segment public flat 'code'

	assume	cs:flat, ds:flat, es:nothing, ss:nothing
	assume	fs:nothing, gs:nothing

		PUBLIC	DoThunk32to16
DoThunk32to16	Proc Near Pascal	uses es ebx edi esi,
		RoutineAddr:dword,	;16:16 address of the routine to call
		ParamBytes:dword	;number of param bytes the routine expects
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;	The net effect is that the stack is converted to a 16 bit stack and
;	the parameters is/are pushed back onto the 16 bit stack with word
;	alignment.
;
;	DoThunk32to16 is 32 bit code
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;
	mov	eax,esp			;get esp to eax so we can push it
	push	ss			;save ss
	push	eax			;save the stack pointer

	lea	esi,RoutineAddr+4	;parameters are at RoutineAddr+4
	mov	ecx,ParamBytes		;get count of parameter bytes
	sub	esp,ecx			;make room for the parameters
	mov	edi,esp			;destination address to edi
	rep movsb			;move the parameters for the 16:16 routine

	; convert the stack to its 16:16 value
	mov	eax,esp			;esp to eax so we can change it
	ror	eax,16			;swap high word with low word
	shl	ax,3			;left 3
	or	al,7			;add bits for LDT and ring 3
	rol	eax,16			;swap back, eax is now a sel:offset
	push	eax			;new sk pointer to mem so we can load it
	lss	sp,[esp]		;stack is now 16:16 and points to same location
	movzx	esp,sp			;make sure esp is less than 64k

	jmp	FAR PTR Do16BitCall	;go call the 16:16 routine

ThunkReturn:
	movzx	esp,sp			;insure esp is valid
	lss	esp,[esp]		;restore the stack
	ret

DoThunk32to16	endp

@CURSEG	ENDS

code	segment	public use16 'code'

Do16BitCall	PROC FAR
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;	The following is a 16 bit segment that is used to call the target
;	routine.
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;

; Note that the the +12 below is affected by the number of parameters in
; the routine DoThunk32to16.  The constant (12) is count of param bytes+4.
	call    DWORD PTR [bp+12]       ; call 16:16 function
	jmp	FAR PTR FLAT:ThunkReturn
Do16BitCall     ENDP

Callmd5String	Proc Far  Pascal,
		StringPtr:dword,	;points to the string
		Count:dword,		;bytes in the string
		ResultPtr:dword		;points to 16 byte bufr for result
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;	The thunker needs to call a C routine, so this C proc is creaed to
;	interface to the PASCAL routine md5String
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;
	Invoke	md5String,
		StringPtr,
		word ptr Count,
		ResultPtr
	ret

Callmd5String	endp

@curseg ends

code32	segment public flat 'code'

	PUBLIC	FlatToSel
FlatToSel	Proc Near Pascal,
		FlatOffset:dword
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;	Convert the passed flat offset to 16:16 and return it in eax.
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;
	mov	eax,FlatOffset		;get the flat offset to eax
	test	eax,eax			;test for null offset
	jz	fts90			;no conversion for null addr

	ror	eax,16			;swap high word with low word
	shl	ax,3			;left 3
	or	al,7			;add bits for LDT and ring 3
	rol	eax,16			;swap back, eax is now a sel:offset
fts90:	ret

FlatToSel	endp

		public	md5String32
md5String32	Proc Near Pascal	uses ds es fs gs,
		StringPtr:dword,	;points to the string
		Count:dword,		;bytes in the string
		ResultPtr:dword		;points to 16 byte bufr for result
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;	MD5 Message-Digest Algorithm on a given string
;ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;
	pushad
	pushfd
	mov	esi,esp			;save esp in esi

	Invoke	FlatToSel,		;32 bit address to 16 bit address
		StringPtr
	push	eax			;to sk for thunker
	push	Count			;count parameter

	Invoke	FlatToSel,		;32 bit address to 16 bit address
		ResultPtr
	push	eax			;to sk for thunker

	mov	ax,seg Callmd5String	;16 bit segment value
	shl	eax,16			;to high bits of eax
	mov	ax,offset Callmd5String	;offset in 16 bit segment
	Invoke	DoThunk32to16,
		eax,
		12

	mov	esp,esi
	popfd
	popad
	ret

md5String32	endp

@curseg ends

	end

