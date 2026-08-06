page 60, 132

;   SWAP.ASM        Version 2.0     September 6, 1990
;
;   Return code (to C caller) of this swap routine
;     0 = success
;     1 = unable to shrink DOS memory allocation
;     2 = unable to save program to EMS
;     3 = unable to execute requested program
;
;   Contains code and data needed to swap most of the current program out
;   to extended memory, expanded memory, or disk; execute another program;
;   and re-load the original program back into memory.
;
;   Copyright (C) 1990 by Marty Del Vecchio
;   Released to the public domain for free use by all
;   Product is supplied as is and author disclaims all warranties,
;   explicit or implied, about the functionality of the source code
;   or object modules supplied, and shall not be held responsible
;   for any damages caused by use of product.
;
;   Contributions not solicited.  Just appreciate the fact that somebody
;   took the time to write and comment this code.  If you have any
;   questions, please contact me at:
;
;   Marty Del Vecchio                   Channel 1 BBS
;   99 Marlboro Road                    Boston, MA
;   Southborough, MA  01772             (617) 354-8873
;   (508) 485-9718
;
;   internet:  marty@bsn.mceo.dg.com
;
;   For information about the contents of this file, see the accompanying
;   file SWAP.DOC.
;

; 'DOSSEG' gives us support for Microsoft C and Turbo C segment naming
; and ordering schemes
; DOSSEG

; Figure out which memory model we're assembling for.  Specified on
;  MASM command line with /D followed by __s__, __m__, __c__ or __l__.
;  If none specified, Small is assumed.
; Once the model is defined, MASM provides two definitions, @codesize
;  and @datasize, to determine the size of code and data pointers.  If
;  @codesize is 0 (Small and Compact), there is one code segment, and
;  code addresses are 16 bits (offset only).  If @codesize is 1 (Medium
;  and Large), there are multiple code segments, and code addresses are
;  32 bits (segment and offset).  Similarly, @datasize of 0 means one
;  data segment (16-bit pointers), and @datasize of 1 means multiple
;  data segments (32-bit pointers).

IFDEF __l__
   .MODEL Large, C
   IF1
      %out Assembling for C, Large memory model
   ENDIF
ELSE
   IFDEF __c__
      .MODEL Compact, C
      IF1
         %out Assembling for C, Compact memory model
      ENDIF
   ELSE
      IFDEF __m__
         .MODEL Medium, C
         IF1
            %out Assembling for C, Medium memory model
         ENDIF
      ELSE
         .MODEL Small, C
         IF1
            %out Assembling for C, Small memory model
         ENDIF
      ENDIF
   ENDIF
ENDIF


; Figure out which save method we are using--EMS, XMS, disk, or what
;  Specified on MASM command line with /D followed by a combination
;  of "ems", "xms", and "disk" (or "all").  For example, to create a swap()
;  that will try using XMS and EMS, you would use "masm swap.asm /Dems /Dxms"
;  If none specified, it will use all.  To change the order in which swap()
;  attempts to save the program to different places, see the function
;  save_program below.

; First, see if they want all of them...
IFDEF all
   USE_DISK  EQU 1
   USE_XMS   EQU 1
   USE_EMS   EQU 1
ELSE
   ; /Dall not specified--try each individually...
   IFDEF disk
      USE_DISK  EQU 1
   ENDIF

   IFDEF xms
      USE_XMS   EQU 1
    ENDIF

   IFDEF ems
      USE_EMS   EQU 1
   ENDIF

ENDIF

; Now see if they declared anything--if not, it will use them all
IFNDEF USE_DISK
   IFNDEF USE_EMS
      IFNDEF USE_XMS
         USE_DISK  EQU 1
         USE_XMS   EQU 1
         USE_EMS   EQU 1
      ENDIF
   ENDIF
ENDIF

; Constant definitions for easier reading
STDERR          equ     2           ; Standard DOS file handle for error output
GET_VECTOR      equ     35h         ; DOS function to get interrupt vector
EMM_INT         equ     67h         ; EMS interrupt vector
EMM_NAME_LEN    equ     8           ; Length of EMS device driver name
MAX_DOS_CMD     equ     127         ; Maximum DOS command-line length

bptr            equ     byte ptr    ; Means we're loading/storing 8 bits
wptr            equ     word ptr    ; Means we're loading/storing 16 bits
dptr            equ     dword ptr   ; Means we're loading/storing 32 bits


extrn ExtendedError:word
extrn ExtendedClass:byte
extrn ExtendedAction:byte
extrn ExtendedLocus:byte

; All code and data must be in the code segment, which is the first segment
;  in all Turbo C, Turbo C++, and Microsoft C memory models.
; If we are in the Medium or Large models, there are multiple code segments.
;  If this is the case, our default code segment name will be "SWAP_TEXT".
;  However, in order for us to be loaded as early as possible in the
;  executable file, we have to call our segment "_TEXT", which is what it
;  is called in the Small and Compact models.  This is the segment where
;  the C compiler puts all of its code.
; This could be a problem if you are trying to keep certain routines
;  resident while swap() is executing your program!!!!!!!!
IF @codesize
.CODE   @@@_TEXT
ELSE
.CODE
ENDIF

; *****************************************************************************
; Our resident data declarations--this data will be needed after the swap
;  has occurred, and thus must be above the resident line
; *****************************************************************************

; *****************************************************************************
; First, all variables that will be used by all versions assembled from
; this source file, regardless of what save options are selected
; *****************************************************************************
ret_code    dw      0           ; Return code (to C caller) of this swap routine
                                ;   0 = success
                                ;   1 = unable to shrink DOS memory allocation
                                ;   2 = unable to save program to EMS
                                ;   3 = unable to execute requested program

ExtError    dw      0
ExtClass    db      0
ExtAction   db      0
ExtLocus    db      0


; *****************************************************************************
; Variables that deal with DOS' memory allocation blocks
old_size    dw      0           ; The old size (in paragraphs) of this program
new_size    dw      0           ; The new "resident" size, doesn't include code/data swapped to disk
my_psp      dw      0           ; This program's Program Segment Prefix (PSP)
mcb_psp     dw      0           ; The PSP address in this program's memory block
start_seg   dw      0           ; Segment address of released memory
; *****************************************************************************

; *****************************************************************************
; Variables used during the save/restore process
handle      dw      0           ; Expanded memory handle
filehandle  dw      0           ; Disk file handle
valid       dw      0           ; Calculated validation value
saved_valid dw      0           ; Saved validation value
saved_paras dw      0           ; Number of paragraphs (16 bytes) saved to ems/disk
paras_left  dw      0           ; temporary counter
; *****************************************************************************

; *****************************************************************************
; A temporary stack in our code segment, and associated variables
old_sp      dw      0               ; Place to save this program's stack
old_ss      dw      0               ;  information while executing new program
; XMS driver needs a large stack (at least 256 bytes free)
IFDEF USE_XMS
new_stack   db      320 dup ('?')   ; Temporary stack we can address after swap
ELSE
new_stack   db      128 dup ('?')    ; Temporary stack we can address after swap
ENDIF
new_sp      label   word            ; Point SP to "top" of stack
; *****************************************************************************

; *****************************************************************************
; Variables that deal with the execution of the new program
prog_name   db      128 dup (0)     ; Storage for name of program to execute
cmd_len     db      0               ; Storage for length of command line parameters
cmd_line    db      128 dup (0)     ; Storage for command line parameters

            public  param_blk
param_blk   label   byte            ; Program Parameter Block--pass to DOS on exec call
env_seg     dw      0               ; Environment address, 0 means copy of ours
cmd_ofs     dw      offset @code:cmd_len    ; Offset address of command line
cmd_seg     dw      seg cmd_line    ; Segment address of command line
fcb1        dd      0ffffffffh      ; Default FCB's, -1 = none
fcb2        dd      0ffffffffh
; *****************************************************************************

exec_ret    db      0               ; Return code from executed program
restore_proc dw     0               ; Address of appropriate restore routine

; *****************************************************************************
; Message to display to screen when we can't reload program
abort_msg   db      0dh, 0ah, 'SWAP: Unable to reload program.', 0dh, 0ah
abort_len   dw      $ - offset @code:abort_msg
valid_msg   db      0dh, 0ah, 'SWAP: invalid swap file.', 0dh, 0ah
valid_len   dw      $ - offset @code:valid_msg
readerr_msg db      0dh, 0ah, 'SWAP: error reading swap file.', 0dh, 0ah
readerr_len dw      $ - offset @code:readerr_msg
; *****************************************************************************

; *****************************************************************************
; Next, the variables needed only for certain versions of the routine,
;  depending on which save/restore options are chosen
; *****************************************************************************

; *****************************************************************************
; Variables needed only when swapping to XMS
IFDEF USE_XMS
XMS_proc    dd      0               ; Address of XMS entry point

XMS_struc       label   byte        ; Structure needed to move memory with XMS
XMS_size        dd      0           ; # of bytes to move (must be even)
XMS_from        dw      0           ; Handle of source, 0=conventional memory
XMS_from_addr   dd      0           ; Address of source memory
XMS_to          dw      0           ; Handle of destionation, 0=conventional memory
XMS_to_addr     dd      0           ; Address of destination memory
ENDIF
; *****************************************************************************

; *****************************************************************************
; Variables needed only when swapping to EMS
IFDEF USE_EMS
pages_used  db      0           ; # of pages of EMS used
next_page   dw      0           ; Temporary storage for EMS page number
emm_name    db      'EMMXXXX0'  ; Name of EMS device driver
ems_seg     dw      0           ; Segment address of EMS page frame
ENDIF
; *****************************************************************************

; *****************************************************************************
; Variables needed only when swapping to disk
IFDEF USE_DISK
fname       db      80 dup (0)  ; Name of the file data is saved to/read from
ENDIF
; *****************************************************************************



; *****************************************************************************
; Version-dependent code--only assemble the routine to restore the program
; from each media (XMS, EMS, disk) if it was specified on the command line
; *****************************************************************************


; *****************************************************************************
; restore_xms   Attempts to restore program from XMS extended memory
;
; Entry:        DS points to our variables
;               Program was saved to XMS extended memory (handle)
; Return:       Carry set on error, carry clear on success
; *****************************************************************************
IFDEF USE_XMS
restore_xms     proc    near

                assume  ds:@code                    ; Tell MASM that DS points to our variables

; All we need to do is copy the old program from XMS to conventional memory
; Call the XMS copy memory function to do this; so fill in request block
xms_read_size:  mov     ax, wptr saved_paras        ; AX = # of paragraphs to read
                mov     bx, 10h
                mul     bx                          ; DX:AX = AX * 10h, # of bytes to read
                mov     wptr XMS_size, ax           ; Store # of bytes to read
                mov     wptr XMS_size + 2, dx

xms_read_from:  mov     ax, wptr handle             ; Source XMS handle
                mov     wptr XMS_from, ax
                xor     bx, bx
                mov     wptr XMS_from_addr, bx      ; Offset in extended memory
                mov     wptr XMS_from_addr + 2, bx  ;  block is 0000:0000

xms_read_to:    mov     wptr XMS_to, bx             ; Read into conventional memory
                mov     wptr XMS_to_addr, bx        ; Offset of dest address
                mov     ax, wptr start_seg          ; Segment of dest address
                mov     wptr XMS_to_addr + 2, ax

do_xms_read:    mov     si, offset @code:XMS_struc  ; DS:SI -> XMS structure
                mov     ah, 0Bh
                call    dptr XMS_proc               ; Do the move

xms_dealloc:    push    ax                          ; Save that return code!

                mov     dx, wptr handle             ; First, free handle
                mov     ah, 0Ah
                call    dptr XMS_proc

                pop     ax                          ; Retrieve copy return code
                cmp     ax, 1                       ; AX = 1 means success
                jnz     restore_xms_er              ; Error

restore_xms_ok: clc                                 ; Signal no error
                jmp     short restore_xms_ret

restore_xms_er: stc                                 ; Signal error

restore_xms_ret:ret
restore_xms     endp
ENDIF
; *****************************************************************************


; *****************************************************************************
; restore_ems   Attempts to restore program from EMS expanded memory
;
; Entry:        DS points to our code segment
;               Program was saved to EMS expanded memory (handle)
; Return:       Carry set on error, carry clear on success
; *****************************************************************************
IFDEF USE_EMS
restore_ems     proc    near

                push    ds                      ; Save original DS
                cld                             ; All memory copies increase SI and DI

                assume  ds:@code                ; Tell MASM that DS points to our variables

                mov     bx, wptr saved_paras    ; Get # of paragraphs saved
                mov     wptr paras_left, bx     ; Used to count paras restored
                mov     ax, wptr ems_seg
                mov     ds, ax                  ; DS:0 = EMS source

                ; DS no longer points to our segment!  All references to
                ;  our variables must have cs: override until the end of
                ;  this routine.

                assume  ds:nothing              ; Tell MASM that DS doesn't point to our variables

                mov     ax, wptr cs:start_seg
                mov     es, ax                  ; ES:0 = program destination
                mov     wptr cs:next_page, 0    ; Start with page 0
                mov     dx, wptr cs:handle      ; Retrieve EMS handle

                mov     ah, 47h                 ; Save page map first
                int     67h

                or      ah, ah                  ; check EMS return code (ah = 0, OK)
                jz      ems_read_16K            ; ah was 0, OK

ems_read_fail:  mov     ah, 48h                 ; Some kind of EMS failure--
                int     67h                     ;  first restore page map
                mov     ah, 45h                 ; Deallocate my handle
                int     67h                     ;  (passed in dx)

jmp     short restore_ems_er    ; Signal error

ems_read_16k:   mov     ah, 44h                 ; Map page
                mov     bx, wptr cs:next_page   ; Next logical page to map
                mov     al, 0
                int     67h                     ; Map the next page
                or      ah, ah
                jnz     ems_read_fail           ; ah != 0 means failure

                cmp     wptr cs:paras_left, 400h    ; Less than 16K left?
                jb      last_ems_read               ; Yes, do last EMS read
                sub     wptr cs:paras_left, 400h    ; 16K less to read
                xor     si, si                      ; Set SI and DI to 0
                xor     di, di
                mov     cx, 2000h               ; Write 8K words = 16K bytes
                                                ; DS:SI = expanded memory page (source)
                                                ; ES:DI = location to restore to (destination)
                rep     movsw                   ; Move the data from EMS

                mov     ax, es                  ; Get old restore pointer
                add     ax, 400h                ; Move restore pointer up 16K
                mov     es, ax                  ; Set new restore pointer
                inc     wptr cs:next_page       ; Look at next EMS page
                jmp     short ems_read_16k      ; Read next 16K from EMS

last_ems_read:  mov     ax, wptr cs:paras_left  ; Paragraphs left to restore
                mov     cl, 4                   ; Convert to bytes
                shl     ax, cl
                mov     cx, ax                  ; CX = remaining bytes to restore
                xor     si, si
                xor     di, di
                rep     movsb                   ; Restore the remaining bytes

restore_ems_ok: mov     ah, 48h                 ; Restore EMS page map
                int     67h

ems_dealloc:    mov     ah, 45h                 ; Deallocate pages
                int     67h

                clc                             ; Signal no error
                jmp     short restore_ems_ret

restore_ems_er: stc                             ; Signal error

restore_ems_ret:pop     ds
                ret
restore_ems     endp
ENDIF
; *****************************************************************************


validate        proc    near
                push    si

                xor     si,si
                shr     cx,1
validate_loop:  lodsw
                add     [valid],ax
                loop    validate_loop

                pop     si
                ret
validate        endp


; *****************************************************************************
; restore_disk  Attempts to restore program from DOS disk file
;
; Entry:        DS points to our code segment
;               Program was saved to DOS disk file (fname)
;
; Return:       Does not return unless successful.  If an error occurs it
;               calls err_exit and exits to directly from there.
; *****************************************************************************
IFDEF USE_DISK
restore_disk    proc    near

                assume  ds:@code                ; Tell MASM that DS points to our variables

;------------------------------------------------------------------------------
;   No need to OPEN the file, the save_disk routine left it open for us
;   open_file:      mov     dx, offset @code:fname  ; DS:DX -> file name
;                   mov     ax, 3D42h               ; DOS function 3Dh, open file
;                                                   ;  al = open for read only, deny none
;                   int     21h                     ; Call DOS
;                   jnc     open_ok                 ; Carry clear = all OK
;                   jmp     restore_disk_er         ; Carry set, error
;   open_ok:        mov     wptr filehandle, ax     ; File handle returned from DOS
;------------------------------------------------------------------------------
;   move to top of file
                mov     ax, 4200h
                mov     bx, wptr filehandle
                xor     dx, dx
                xor     cx, cx
                int     21h

                mov     [valid],0

;   restore handle into ax
                mov     ax, wptr filehandle
;------------------------------------------------------------------------------

                push    ds

                mov     bx, ax                  ; Must specify in BX later
                mov     ax, wptr start_seg      ; First segment to restore to
                mov     ds, ax
                mov     ax, wptr saved_paras    ; # of paragraphs in file to read
                mov     wptr paras_left, ax     ; Keep count in this variable

disk_read_32k:  cmp     ax, 0800h               ; Less than 32K left?
                jb      last_disk_read          ; Yes, do last read
                sub     wptr cs:paras_left, 0800h   ; 32K left to read
                mov     ah, 3Fh                 ; DOS function 3Fh, read file
                mov     cx, 8000h               ; Read 32K bytes
                xor     dx, dx                  ; DS:DX -> buffer to read to
                int     21h                     ; Call DOS
                jc      disk_read_err           ; Carry set = error

disk_read_ok:   mov     cx,8000h                ; we read 32K bytes
                call    validate                ; validate entire block

                mov     ax, ds                  ; Address next read location
                add     ax, 0800h               ; It's 800h paragraphs ahead
                mov     ds, ax                  ; DS -> new restore location
                mov     ax, paras_left          ; Expecting this above
                jmp     short disk_read_32k     ; Read next 32K

disk_read_err:  pop     ds
                mov     ah, 3eh                 ; Error, close file first
                int     21h                     ; Call DOS
                mov     dx, offset @code:readerr_msg
                mov     cx, wptr readerr_len
                call    near ptr err_exit       ; exit to DOS

last_disk_read: mov     cx, 4                   ; Convert paragraphs to bytes
                shl     ax, cl
                mov     cx, ax                  ; # of bytes left in cx
                mov     ah, 3fh                 ; Read last bytes
                xor     dx, dx                  ; DS:DX -> buffer to restore to
                push    cx                      ; save CX for validate routine
                int     21h                     ; Call DOS
                pop     cx                      ; restore CX for validate
                jc      disk_read_err           ; Error reading!  Close file first

                call    validate                ; validate CX bytes read in

close_read:     pop     ds
                mov     ah, 3eh                 ; Close file
                int     21h                     ; Call DOS

restore_disk_ok:mov     ax,[valid]
                cmp     ax,[saved_valid]        ; is the validation okay
                je      restore_disk_ret        ;   yes, return successfully
                                                ;   otherwise, display error
                                                ;   and exit out

                mov     dx, offset @code:valid_msg
                mov     cx, wptr valid_len
                call    near ptr err_exit       ; exit to DOS

restore_disk_ret:
                mov     dx, offset @code:fname  ; DS:DX -> file name
                mov     ah, 41h                 ; DOS function 41h, delete file
                int     21h                     ; Call DOS

                clc
                ret
restore_disk    endp
ENDIF
; *****************************************************************************



; *****************************************************************************
; execute_program   Execute the program specified
;
; Entry:            param_blk has been initialized
;                   DS points to our data
; Return:           puts return code in cs:exec_ret
; *****************************************************************************
execute_program proc    near                    ; Called only from inside our segment

                push    ds                      ; These are destroyed by the
                push    es                      ;  DOS EXEC call

                assume  ds:@code                ; Tell MASM that DS points to our variables

exec_program:   mov     ax, ds                  ; Our path name is in CS (point DS to our segment)
                mov     es, ax                  ; Our parameter block is in CS (point ES to our segment)
                mov     ax, 4b00h               ; Load and execute program
                mov     bx, offset @code:param_blk
                mov     dx, offset @code:prog_name
                int     21h                     ; Sets carry flag if error
                                                ; All registers destroyed
                                                ;  except CS:IP!

                assume  ds:nothing              ; Tell MASM that DS doesn't point to our variables

                mov     bptr cs:exec_ret, al    ; Store EXEC code
                jc      exec_err                ; Ooops

get_return:     mov     ah, 4dh                 ; DOS function to get ret code
                int     21h                     ; All registers destroyed
                mov     bptr cs:exec_ret, al    ; Store EXEC code
                jmp     short exec_exit

exec_err:       mov     wptr cs:ret_code, 3     ; Signal error on executing

                mov     ah,59h
                xor     bx,bx
                int     21h
                mov     cs:ExtError,ax     ;  store extended error code
                mov     cs:ExtClass,bh     ;  store extended class code
                mov     cs:ExtAction,bl    ;  store extended action code
                mov     cs:ExtLocus,ch     ;  store extended locus code
                stc                        ;  make sure carry flag is still set

exec_exit:      pop     es
                pop     ds

                ret

execute_program endp


; *****************************************************************************
; err_exit          Prints error message and terminates program
;
; Entry:            DX points to string to display
;                   CX equals the length of the string
;
; Returns:          Doesn't return--calls DOS terminate function.
;                   Naturally, we can't use the C runtime routines,
;                   since they are swapped out.
; *****************************************************************************
err_exit        proc    near                    ; Called only from inside our segment

                mov     ax, cs
                mov     ds, ax                  ; Point DS to our data

                assume  ds:@code                ; Tell MASM that DS points to our data

                mov     ah, 40h                 ; DOS function to write to file
                mov     bx, STDERR              ; Write to standard error handle
                int     21h

                mov     ax, 4CFFh           ; Exit, return code 255 decimal
                int     21h                 ; Exit to DOS, no return

err_exit        endp


; *****************************************************************************
;   swap        The routine that does it all
;
;   Callable by a C program, takes these parameters (regardless
;     of which swap options chosen at assembly time, because
;     C calling conventions let is ignore parameters to the
;     right if we want to):
;
;   swap_both:
;       prog        Full path name of program to execute
;       cmdline     Command-line parameters for program to execute
;       return      Pointer to byte for return code of exec'd program
;       save_file   Full path name of file in which to save program image (if disk is to be used)
;
;   Depending on the memory model used, the pointers to the
;   parameters each occupy 2 bytes or 4 bytes on the stack.
;   If there is only one data segment (Small and Medium), each
;   value is a 2-byte near pointer, with DS assumed as the segment
;   register.  If there are multiple data segments (Compact and
;   Large), each value is a 4-byte far pointer, with segment and
;   offset values each pushed on the stack.
;
;   The function is declared with 4 parameters, regardless of whether
;   disk swapping is being included.  This is because the file name
;   parameter is the last on the parameter list, which C lets us
;   ignore if we want.
;
;   The swap() routine does not check the program name or command
;   line to verify that a legal command has been requested--that's
;   the caller's responsibility!
;
; *****************************************************************************

                public  swap, valid, saved_valid

swap            proc    prog:PTR, cmdline:PTR, return:PTR, save_file:PTR, envp:PTR

                push    si                      ; Save registers needed
                push    di                      ;  by the caller
                push    es
                push    ds

point_segs:     mov     ax, cs                  ; Point ES to our segment
                mov     es, ax                  ;  for copying of parameters




; *****************************************************************************
;
; Take the envp variable and, if it is not NULL, turn it into a segment address
;

IF @datasize
                mov     ax, word ptr [envp]
                mov     dx, word ptr [envp+2]
ELSE
                mov     ax, [envp]
                mov     dx, ds
ENDIF
                or      ax, ax
                jz      set_envseg
                mov     cl, 4
                shr     ax, cl
                add     ax, dx
set_envseg:     mov     cs:[env_seg], ax


; *****************************************************************************
get_name:       ; Copy program name to our variable, all versions

; If multiple data segments, load DS:SI from stack.  Else, just load SI
IF @datasize
                push    ds                      ; Save segment register
                lds     si, dptr prog           ; Load 32-bit far pointer
ELSE
                mov     si, wptr prog           ; Load 16-bit near pointer
ENDIF                                           ; DS:SI -> program name from caller

                mov     di, offset @code:prog_name  ; ES:DI -> our storage area

name_loop:      lodsb                           ; Fetch next byte
                stosb                           ; Save next byte
                or      al, al                  ; Was it 0 (end of string)?
                jnz     name_loop               ; No, get next one

IF @datasize
                pop     ds                      ; Pop DS if it was pushed above
ENDIF
; *****************************************************************************

; *****************************************************************************
get_cmd:        ; Copy command line to our variable, all versions

; If multiple data segments, load DS:SI from stack.  Else, just load SI
IF @datasize
                push    ds                      ; Save segment register
                lds     si, dptr cmdline        ; Load 32-bit far pointer
ELSE
                mov     si, wptr cmdline        ; Load 16-bit near pointer
ENDIF                                           ; DS:SI -> command line from caller

                mov     di, offset @code:cmd_line   ; ES:DI -> our storage area
                xor     cl, cl                  ; Keep track of length in cl

cmd_loop:       lodsb                           ; Fetch next byte from DS:SI
                or      al, al                  ; Was it 0 (end of string)?
                jz      cmd_end                 ; Yes, we're done
                stosb                           ; No, store byte
                inc     cl                      ; Increment length
                cmp     cl, MAX_DOS_CMD         ; Are we at maximum cmd length?
                jnz     cmd_loop                ; Nope, keep going

cmd_end:        mov     bptr es:[di], 0dh       ; Put CR at end of cmd line
                mov     bptr cs:cmd_len, cl     ; Store command-line length

IF @datasize
                pop     ds                      ; Pop DS if it was pushed above
ENDIF
; *****************************************************************************

; *****************************************************************************
; Get the file name from the command line, if this version needs it

;;;;  The code below USED TO BE necessary because the swap code used to
;;;;  close the file during the shell to DOS and have to RE-open it afterwards
;;;;  in order to get itself running again.  Since I (dwt) have later modified
;;;;  the code to keep the file handle open, this is no longer necessary, we
;;;;  can just use the file handle to read the file back in.
;
;     IFDEF USE_DISK
;     get_file:
;
;     ; If multiple data segments, load DS:SI, else just load SI
;     IF @datasize
;                     push    ds                      ; Save segment register
;                     lds     si, dptr save_file      ; Load 32-bit pointer
;     ELSE
;                     mov     si, save_file           ; Load 16-bit pointer
;     ENDIF                                           ; DS:SI -> swap file name from caller
;
;                     mov     di, offset @code:fname  ; ES:DI -> our storage area
;
;
;     resolve:        mov     ah, 60h                 ; DOS INTERNAL function to resolve file name to full path name
;                     int     21h                     ; Stores complete path at ES:DI--we need it after EXEC in case
;                                                     ;  current drive or directory have changed
;                                                     ; Ignore file name error here--it
;                                                     ;  will be caught in save_disk if need be
;
;     IF @datasize
;                     pop     ds                      ; Pop DS if it was pushed above
;     ENDIF
;
;     ENDIF           ; IFDEF disk

;;;;; This is my replacement.  We'll copy the filename to @code:fname

IFDEF USE_DISK
IF @datasize
                push    ds                      ; Save segment register
                lds     si, dptr save_file      ; Load 32-bit pointer
ELSE
                mov     si, save_file           ; Load 16-bit pointer
ENDIF                                           ; DS:SI -> swap file name from caller

                mov     di, offset @code:fname  ; ES:DI -> our storage area
                cld

storenameloop:  lodsb
                stosb
                or      al,al
                jnz     storenameloop

IF @datasize
                pop     ds                      ; Pop DS if it was pushed above
ENDIF
ENDIF ;IFDEF USE_DISK



; *****************************************************************************
; We have the parameters--let's go
; *****************************************************************************

                mov     wptr cs:ret_code, 0     ; Initialize swap's return code
                mov     cs:exec_ret, 0          ; Initialize exec's return code

save_stack:     mov     ax, ss
                mov     wptr cs:old_ss, ax      ; Save current SS
                mov     ax, sp
                mov     wptr cs:old_sp, ax      ; Save current SP

our_stack:      mov     ax, cs                  ; Our stack is in our CS
                cli                             ; Disable interrupts
                mov     ss, ax
                mov     sp, offset @code:new_sp ; Set new stack
                sti                             ; Re-enable interrupts

save_regs:      push    es                      ; Save needed registers
                push    ds                      ; This is the caller's DS!
                push    bp

                mov     ax, cs
                mov     ds, ax                  ; Point DS to our data

                assume  ds:@code                ; Tell MASM that DS points to our variables

save_info:      mov     ah, 51h                 ; DOS function 51h, get PSP
                int     21h                     ; Call DOS
                mov     ax, bx                  ; ax = PSP
                mov     wptr my_psp, ax         ; Save in cs: addressable location
                dec     ax                      ; PSP-1 = MCB for this mem block
                mov     es, ax
                mov     ax, es:[0001h]          ; Get PSP address--should be same!
                cmp     ax, wptr my_psp         ; All kosher?
                jz      psp_ok                  ; Yes

psp_error:      mov     wptr ret_code, 1        ; No, pass return code
                jmp     short exit_swap         ; Exit

psp_ok:         call    near ptr calc_size      ; Calc size to keep, save

try_save:       call    near ptr save_program   ; Write program to disk
                jnc     shrink_mem              ; Carry flag set on error

no_save:        mov     wptr ret_code, 2        ; Error--set return code
                jmp     short exit_swap         ; Exit routine on error

shrink_mem:     mov     ah, 4ah                 ; DOS 4Ah--modify memory allocation
                mov     es, wptr my_psp         ; Point to PSP again
                mov     bx, wptr new_size       ; new_size was figured in calc_size
                int     21h                     ; Call DOS to shrink size

; *****************************************************************************
; Any routine called or data referred to after this point MUST be located
;  in this source file BEFORE the variable new_mcb below!
; *****************************************************************************

                jnc     exec_prog               ; No carry = success

no_shrink:      mov     wptr ret_code, 1        ; Carry = couldn't shrink block
                jmp     short exit_swap         ; Should delete file here!

exec_prog:      call    near ptr execute_program    ; Execute the specified program
                jnc     re_size                     ; No carry, OK

exec_er:        mov     wptr ret_code, 3        ; Signal error

re_size:        mov     es, wptr my_psp         ; Get our PSP address
                mov     bx, wptr old_size       ; Increase back to old size
                mov     ah, 4ah                 ; DOS function 4Ah = resize
                int     21h
                jnc     restore_prog            ; Carry clear = all OK

resize_err:     mov     dx, offset @code:abort_msg
                mov     cx, wptr abort_len
                call    near ptr err_exit       ; Can't return, exit to DOS

restore_prog:   call    wptr restore_proc       ; Restore program from disk
                jc      resize_err              ; Carry set if error

read_ok:
exit_swap:      pop     bp                      ; Restore saved registers
                pop     ds                      ; This is the caller's DS!
                pop     es

                assume  ds:nothing              ; Tell MASM DS doesn't point to our variables

prev_stack:     mov     ax, wptr cs:old_ss      ; Restore original stack
                cli
                mov     ss, ax
                mov     sp, wptr cs:old_sp
                sti

; Giving user exec's return code.  It could be a 16- or 32-bit pointer
IF @datasize
                push    ds
                lds     si, dptr return         ; Load 32-bit pointer
ELSE
                mov     si, wptr return         ; Load 16-bit pointer
ENDIF                                           ; DS:SI -> return code variable

                mov     al, bptr cs:exec_ret    ; Store exec's return code
                mov     bptr [si], al           ;  at address specified by caller

IF @datasize
                pop     ds                      ; Pop DS if pushed above
ENDIF

                pop     ds
                pop     es
                pop     di
                pop     si


                ; in case of Swap Code = 3, restore registers, then save
                ; them out to the variables used by EXTENDED.C in the DOS_x.LIB

                mov     ax,cs:ExtError     ;  restore extended error code
                mov     bh,cs:ExtClass     ;  restore extended class code
                mov     bl,cs:ExtAction    ;  restore extended action code
                mov     ch,cs:ExtLocus     ;  restore extended locus code

                mov     ds:ExtendedError,ax   ;  store extended error code
                mov     ds:ExtendedClass,bh   ;  store extended class code
                mov     ds:ExtendedAction,bl  ;  store extended action code
                mov     ds:ExtendedLocus,ch   ;  store extended locus code

                mov     ax, wptr cs:ret_code    ; Give return code
                ret
swap            endp
; *****************************************************************************


; *****************************************************************************
; *****************************************************************************
ALIGN 10h       ; Aligns next code item on paragraph boundary
para_align      proc    near
new_mcb         db      32 dup (0)          ; DOS will put MCB of released memory here
para_align      endp
; *****************************************************************************
; *****************************************************************************

; *****************************************************************************
; Everything after here is only needed BEFORE we change our allocation size.
;  Everything below this line will be (temporarily) swapped out of memory,
;  and thus cannot be used once we shrink our memory allocation.
; *****************************************************************************

; *****************************************************************************
; calc_size     Calculates the size of the program to keep and
;               how much of it to swap out.
;
; Entry:        DS points to our variables
;               ES points to DOS Memory Control Block for our program
; Return:       old_size, start_seg, new_size initialized
; *****************************************************************************
calc_size       proc    near                    ; Called only from inside our segment

                assume  ds:@code                ; Tell MASM that DS points to our variables

                mov     ax, es:[0003h]          ; Get # paragraphs allocated
                                                ;  in this memory block
                mov     wptr old_size, ax       ; Save old size of program
                mov     bx, cs                  ; BX = segment of our code
                mov     ax, offset @code:new_mcb; Last address to keep
                mov     cl, 4                   ; new_mcb is para aligned
                shr     ax, cl                  ; AX = ofs new_mcb / 16
                inc     ax
                add     bx, ax
                mov     wptr start_seg, bx      ; Segment of released memory
                sub     bx, wptr my_psp         ; BX = size to keep in paragraphs
                mov     wptr new_size, bx       ; Save new, smaller size
                ret

calc_size       endp
; *****************************************************************************

; *****************************************************************************
; xms_installed     Checks to see if XMS driver (himem.sys) is loaded
;
; Entry:            No assumptions--can be called by user
; Return:           1 if XMS driver is load, 0 if not
; *****************************************************************************
IFDEF USE_XMS
                public  xms_installed
xms_installed   proc                            ; Called by user also!

                push    ds                  ; Save all "important" registers
                push    si
                push    es
                push    di

                mov     ax, 4300h           ; Multiplex code for XMS driver, load check function
                int     2Fh                 ; Call multiplex interrupt
                cmp     al, 80h             ; al = 80h means XMS driver IS loaded
                jnz     no_xms              ; Nope, not there

yes_xms:        mov     ax, 4310h               ; Get address of entry point
                int     2Fh                     ; Returns address in ES:BX
                mov     wptr cs:XMS_proc, bx
                mov     wptr cs:XMS_proc + 2, es
                mov     ax, 1                   ; Return 1, XMS installed
                jmp     short xms_ret

no_xms:         xor     ax, ax              ; Return 0, XMS not installed

xms_ret:        pop     di
                pop     es
                pop     si
                pop     ds
                ret

xms_installed   endp
ENDIF
; *****************************************************************************

; *****************************************************************************
; ems_installed     Checks to see if EMS driver is loaded
;
; Entry:            No assumptions--can be called by user
; Return:           1 if EMS driver is load, 0 if not
; *****************************************************************************
IFDEF USE_EMS
                public  ems_installed
ems_installed   proc                            ; Called by user also!

                push    ds                      ; Save "important" registers
                push    si
                push    es
                push    di


get_emm_vector: mov     ah, GET_VECTOR          ; Get EMM interrupt vector
                mov     al, 67h                 ; EMM accessed through Int 67h
                int     21h                     ; Call DOS to get vector
                mov     di, 0ah                 ; vector + di = name
                mov     ax, cs
                mov     ds, ax                  ; DS:SI -> EMM device driver name
                mov     si, offset @code:emm_name   ; Compare with EMM device name
                mov     cx, EMM_NAME_LEN
                cld
                repe    cmpsb                   ; Compare bytes
                jnz     ems_no                  ; Same?  If not, EMS installed

ems_yes:        mov     ax, 1                   ; EMS installed, return 1
                jmp     short ems_ret

ems_no:         xor     ax, ax                  ; EMS not installed, return 0

ems_ret:        pop     di
                pop     es
                pop     si
                pop     ds
                ret

ems_installed   endp
ENDIF
; *****************************************************************************


; *****************************************************************************
; save_program      Try to save in XMS/EMS/disk.
;
; Entry:            DS points to our variables
;
; Returns:          Success:  carry flag clear
;                   Failure:  carry flag set
; *****************************************************************************
save_program    proc    near            ; Called only from inside our segment

                push    si              ; Save registers
                push    di
                push    ds
                push    es

; Now figure out which routines to call, based on command-line definitions
; To change the order in which swap() attempts to swap, change the order
;  of these three conditional blocks.
IF1
   %out swap() will attempt to save the program in the following order:
ENDIF


; *****************************************************************************
IFDEF USE_XMS
IF1
   %out -- XMS extended memory
ENDIF
                call    save_xms        ; Try saving to XMS extended memory
                jnc     save_ok         ; Carry clear == success, all done
ENDIF
; *****************************************************************************


; *****************************************************************************
IFDEF USE_EMS
IF1
   %out -- EMS expanded memory
ENDIF
                call    save_ems        ; Try saving to EMS expanded memory
                jnc     save_ok       ; Carry clear == success, all done
ENDIF
; *****************************************************************************


; *****************************************************************************
IFDEF USE_DISK
IF1
   %out -- DOS disk file
ENDIF
                call    save_disk       ; Try saving to DOS disk file
                jnc     save_ok         ; Carry clear == success, all done
ENDIF
; *****************************************************************************

save_er:        stc                     ; Couldn't save anywhere, return error
                jmp     short save_ret

save_ok:        clc                     ; Saved successfully, return OK

save_ret:       pop     es              ; Restore registers
                pop     ds
                pop     di
                pop     si

                ret
save_program    endp
; *****************************************************************************


; *****************************************************************************
; Version-dependent code--only assemble the routine to save the program
; to each place if it was requested on the command line
; *****************************************************************************


; *****************************************************************************
; save_xms      Attempts to save program to XMS extended memory
;
; Entry:        DS points to our variables
;
; Return:       Carry set on error, carry clear on success
;               If successful, updates restore_proc with the address of
;               the XMS restore routine
; *****************************************************************************
IFDEF USE_XMS
save_xms        proc    near

                assume  ds:@code                ; Tell MASM DS points to our variables

                call    xms_installed           ; Check if XMS installed
                or      ax, ax                  ; Returns 0 if not installed
                jnz     xms_inst                ; AX != 0, XMS installed
                jmp     save_xms_er             ; AX == 0, XMS not installed

xms_inst:       mov     dx, wptr old_size       ; Old size in paragraphs
                sub     dx, wptr new_size       ; DX = size to save to XMS
                mov     wptr saved_paras, dx    ; Save # of paragraphs written

                mov     cl, 6                   ; Convert Paragraphs to kilobytes
                shr     dx, cl                  ; dx = dx / 64
                inc     dx                      ; dx = kilobytes needed (plus 1 for safety)

xms_alloc:      mov     ah, 09h                 ; XMS function 09, allocate extended memory block
                call    dptr XMS_proc           ; Call XMS entry point directly
                cmp     ax, 1                   ; AX = 1 on success
                jnz     save_xms_er             ; Allocation unsuccessful, error

xms_alloc_ok:   mov     wptr handle, dx         ; Save returned handle in DX

; OK, memory is allocated.  Now fill in the XMS request block for memory copy
xms_write_size: mov     ax, wptr saved_paras        ; AX = # of paragraphs to write
                mov     bx, 10h
                mul     bx                          ; DX:AX = AX * 10h
                mov     wptr XMS_size, ax           ; Store # of bytes to write
                mov     wptr XMS_size + 2, dx

xms_write_from: xor     bx, bx
                mov     wptr XMS_from, bx           ; 0 means from conventional memory
                mov     wptr XMS_from_addr, bx      ; Offset of source address
                mov     ax, wptr start_seg          ; Segment of source address
                mov     wptr XMS_from_addr + 2, ax

xms_write_to:   mov     ax, wptr handle             ; Destination XMS handle
                mov     wptr XMS_to, ax
                mov     wptr XMS_to_addr, bx        ; Offset in extended memory
                mov     wptr XMS_to_addr + 2, bx    ;  block is 0000:0000

do_write:       mov     si, offset @code:XMS_struc  ; DS:SI -> XMS request structure
                mov     ah, 0Bh                     ; Function B, copy memory
                call    dptr XMS_proc               ; Do the memory copy move
                cmp     ax, 1                       ; AX = 1 means success
                jz      save_xms_ok                 ; Success, all done!

write_error:    mov     dx, wptr handle             ; Free allocated handle
                mov     ah, 0Ah
                call    dptr XMS_proc               ; Falls through to failure code

save_xms_er:    stc
                jmp     short save_xms_ret

save_xms_ok:    mov     wptr restore_proc, offset @code:restore_xms
                clc

save_xms_ret:   ret
save_xms        endp
ENDIF
; *****************************************************************************


; *****************************************************************************
; save_ems      Attempts to save program to EMS expanded memory
;
; Entry:        DS points to our variables
;
; Return:       Carry set on error, carry clear on success
;               If successful, updates restore_proc with the address of
;               the EMS restore routine
; *****************************************************************************
IFDEF USE_EMS
save_ems        proc    near

                assume  ds:@code                ; Tell MASM DS points to our variables

                push    ds                      ; Save DS--lost later
                cld                             ; All memory copies increment SI and DI

                call    ems_installed           ; Check if EMS installed
                or      ax, ax                  ; AX = 0 if not installed
                jnz     ems_inst                ; AX != 0, ems installed
                jmp     save_ems_er             ; AX = 0, no EMS, error!

ems_inst:       mov     ah, 41h                 ; Get page frame segment
                int     67h
                or      ah, ah                  ; ah = 0 on success
                jz      save_ems1               ; Zero, all OK
                jmp     save_ems_er             ; Non-zero, fatal error

save_ems1:      mov     wptr ems_seg, bx        ; Store segment address

                mov     bx, wptr old_size       ; Segment of released memory
                sub     bx, wptr new_size       ; BX = # paragraphs to save
                mov     wptr saved_paras, bx    ; Save
                mov     wptr paras_left, bx     ; Used to count paras written

                mov     cl, 10                  ; Convert Paragraphs to 16K pages
                shr     bx, cl
                inc     bx                      ; BX = pages needed
                mov     bptr pages_used, bl     ; Save for later use

                mov     ah, 43h                 ; EMM function 43h, allocate
                int     67h
                or      ah, ah                  ; OK return code?
                jz      ems_alloc_ok            ; Yes, skip ahead
                jmp     save_ems_er             ; No, not enough EMS

ems_alloc_ok:   mov     wptr cs:handle, dx      ; Returned handle in DX

                mov     ah, 47h                 ; Save EMS page map first
                int     67h
                or      ah, ah
                jnz     save_ems_fail2          ;  Free allocated handle

                mov     wptr next_page, 0       ; Next page to use is 0
                mov     ax, wptr start_seg
                mov     ds, ax                  ; DS:0 = address to write from

                assume  ds:nothing              ; Tell MASM that DS doesn't point to our variables

                mov     ax, wptr cs:ems_seg     ; ES:0 = EMS page 0
                mov     es, ax

ems_map_next:   mov     ah, 44h                 ; EMM function 44h, map page
                mov     bx, wptr cs:next_page   ; Next page to map
                mov     al, 0                   ; Always use physical page 0
                int     67h                     ; Tell EMM to map the page
                or      ah, ah
                jz      save_ems2               ; ah = 0, skip ahead

save_ems_fail:  mov     ah, 48h                 ; Restore page map first
                int     67h
save_ems_fail2: mov     ah, 45h                 ; And deallocate my handle (in dx)
                int     67h
                jmp     short save_ems_er       ; Signal error

save_ems2:      cmp     wptr cs:paras_left, 400h    ; Less than 16K left?
                jb      last_ems_write              ; Yes, do last EMS write
                sub     wptr cs:paras_left, 400h    ; 16K less to save
                xor     si, si
                xor     di, di
                mov     cx, 2000h               ; Write 8K words
                rep     movsw                   ; Move the data to EMS

                mov     ax, ds                  ; Move source pointer (DS:0)
                add     ax, 400h                ;  up by the 16K we have
                mov     ds, ax                  ;  just written
                inc     wptr cs:next_page
                jmp     ems_map_next

last_ems_write: mov     ax, wptr cs:paras_left  ; Paragraphs left to save
                mov     cl, 4                   ; Convert to bytes
                shl     ax, cl
                mov     cx, ax                  ; CX = remaining bytes to save
                xor     si, si
                xor     di, di
                rep     movsb                   ; Write the remaining bytes

save_ems_ok:    mov     dx, wptr cs:handle
                mov     ah, 48h                 ; Restore page map first
                int     67h
                mov     wptr cs:restore_proc, offset @code:restore_ems
                clc
                jmp     short save_ems_ret

save_ems_er:    stc

save_ems_ret:   pop     ds
                ret
save_ems        endp
ENDIF
; *****************************************************************************


; *****************************************************************************
; save_disk     Attempts to save program to DOS disk file
;
; Entry:        DS points to our variables
;
; Return:       Carry set on error, carry clear on success
;               If successful, updates restore_proc with the address of
;               the disk restore routine
; *****************************************************************************
IFDEF USE_DISK
save_disk       proc    near

                assume  ds:@code                ; Tell MASM DS points to our variables

creat_file:     mov     dx, offset @code:fname  ; DS:DX -> file name
                mov     ah, 3ch                 ; Create/truncate file
                mov     cx, 02h                 ; Create a hidden file
                int     21h                     ; Call DOS
                jnc     creat_ok                ; Carry not set, created okay
                jmp     save_disk_er            ; Carry set, couldn't create file

creat_ok:       mov     wptr filehandle, ax     ; Save handle returned by DOS
                mov     bx, ax
                mov     ax, wptr start_seg      ; Released segment address
                mov     ds, ax

                assume  ds:nothing              ; Tell MASM DS doesn't point to our variables
                mov     [valid],0

                mov     ax, wptr cs:old_size    ; Segment of released memory
                sub     ax, wptr cs:new_size    ; AX = # paragraphs to save
                mov     wptr cs:saved_paras, ax ; Save
                mov     wptr cs:paras_left, ax  ; Used to count paras written

disk_write_32k: cmp     ax, 0800h               ; paras_left less than 32K?
                jb      finish_disk_write       ; Yes, exit
                sub     wptr cs:paras_left, 800h; We will write 32K bytes now

                mov     ah, 40h                 ; DOS function to write to file
                mov     cx, 8000h               ; Write 32K bytes
                xor     dx, dx                  ; DS:DX is buffer to write
                int     21h                     ; Write data to file
                jc      disk_write_er           ; This write was successful

                cmp     ax, 8000h               ; compare number of bytes
                jne     disk_write_er           ; written, error if != 8000h

disk_write_ok:  mov     cx, 8000h               ; We wrote 32K bytes
                call    validate                ; Validate all 32K bytes

                mov     ax, ds                  ; Move write pointer in memory
                add     ax, 800h                ; We just wrote 1K paragraphs
                mov     ds, ax
                mov     ax, wptr cs:paras_left  ; AX checked above
                jmp     short disk_write_32k    ; Loop on next 32K

disk_write_er:  mov     ah, 3eh                 ; Close file first
                int     21h
                jmp     short save_disk_er      ; Return error

finish_disk_write:
                mov     cl, 4                   ; AX = # paragraphs left to write
                shl     ax, cl                  ; Paragraphs to bytes
                mov     cx, ax
                mov     ah, 40h                 ; 40h = write to file
                xor     dx, dx                  ; DS:DX = buffer
                push    cx                      ; Save CX for validate routine
                int     21h                     ; Call DOS
                pop     cx                      ; restore CX for validate
                jc      disk_write_er           ; Carry set, error (close file first)
                cmp     ax,cx                   ; Check bytes written
                jne     disk_write_er           ; Error if not correct number of bytes

                call    validate                ; Validate CX bytes
;---------------------------------------------------------------------------
;   leave the file OPEN and let the restore_disk routine close/delete it
;   close_file:     mov     ah, 3eh                 ; 3eh = close file
;                   int     21h
;---------------------------------------------------------------------------

save_disk_ok:   mov     wptr restore_proc, offset @code:restore_disk
                mov     ax,[valid]
                mov     [saved_valid],ax
                clc
                jmp     short save_disk_ret

save_disk_er:   stc

save_disk_ret:  ret
save_disk       endp
ENDIF
; *****************************************************************************



END
