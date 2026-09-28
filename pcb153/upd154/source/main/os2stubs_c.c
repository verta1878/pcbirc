/* C-linkage stubs for PCBOARD2.EXE OS/2 link */
/* Names: wcc386 adds trailing _, so omit _ from source names */
/* GPL v3.0 — PWA 15.4 project by hexadecimal */
#include <stdlib.h>
#include <string.h>

/* Watcom C++ runtime */
int __compiled_under_generic = 0;
void ___wcpp_4_data_init_longjmp(void) {}
void __wcpp_4_fs_handler_rtn(void) {}

/* VMDATA */
void VMDataStartUp(void) {}
void VMDataShutDown(void) {}
int VMDataShutDownAtExitSet = 0;

/* Usernet */
int createusernetthread(void) { return 0; }
int destroyusernetthread(void) { return 0; }
int needtoscanusernet = 0;

/* List */
int allocatelist(void) { return 0; }
int deallocatelist(void) { return 0; }
int foundinlist(void) { return 0; }

/* CodeBase internal */
int e4is_constant(void) { return 0; }
int e4is_tag(void) { return 0; }
int i4reindex(void) { return 0; }
int t4reindex(void) { return 0; }

/* CRT */
int _getch(void) { return 0; }
