; Exact-name stubs for symbols with special naming
; GPL v3.0 — PWA 15.4 project by hexadecimal
.386
.model flat
_DATA segment dword public 'DATA'
    public __compiled_under_generic
    __compiled_under_generic dd 0
    
    public needtoscanusernet_
    needtoscanusernet_ dd 0
    
    public VMDataShutDownAtExitSet_
    VMDataShutDownAtExitSet_ dd 0
_DATA ends

_TEXT segment dword public 'CODE'
    public ___wcpp_4_data_init_longjmp_
    ___wcpp_4_data_init_longjmp_:
    ret
    
    public __wcpp_4_fs_handler_rtn__
    __wcpp_4_fs_handler_rtn__:
    ret
    
    public VMDataStartUp_
    VMDataStartUp_:
    ret
    
    public VMDataShutDown_
    VMDataShutDown_:
    ret
    
    public _getch
    _getch:
    xor eax,eax
    ret
    
    public createusernetthread_
    createusernetthread_:
    xor eax,eax
    ret
    
    public destroyusernetthread_
    destroyusernetthread_:
    xor eax,eax
    ret
    
    public allocatelist_
    allocatelist_:
    xor eax,eax
    ret
    
    public deallocatelist_
    deallocatelist_:
    xor eax,eax
    ret
    
    public foundinlist_
    foundinlist_:
    xor eax,eax
    ret
    
    public e4is_constant_
    e4is_constant_:
    xor eax,eax
    ret
    
    public e4is_tag_
    e4is_tag_:
    xor eax,eax
    ret
    
    public i4reindex_
    i4reindex_:
    xor eax,eax
    ret
    
    public t4reindex_
    t4reindex_:
    xor eax,eax
    ret
_TEXT ends
end
