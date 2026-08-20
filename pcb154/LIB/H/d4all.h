/* d4all.h — Minimal CodeBase shim for PCBoard PCBNLC
 * Provides opaque type declarations so dbase.hpp compiles.
 * Actual dBASE file I/O implemented in codebase_stubs.c
 * 
 * Reference: CodeBase-for-DBF (LGPL v3.0) by Sequiter Software
 * pcbrevival Phase 0, August 2026
 */

#ifndef D4ALL_INC
#define D4ALL_INC

#include <stdlib.h>
#include <string.h>

/* Opaque structs — internal layout not needed by PCBoard code */
typedef struct CODE4_s {
    int  errorCode;
    int  accessMode;
    int  readOnly;
    int  safety;
    int  autoOpen;
    int  exclusive;
    char reserved[256];
} CODE4;

typedef struct DATA4_s {
    char alias[64];
    int  recCount;
    int  recNo;
    int  bof_flag;
    int  eof_flag;
    char reserved[256];
} DATA4;

typedef struct FIELD4_s {
    char name[11];
    char type;
    int  len;
    int  dec;
    char reserved[32];
} FIELD4;

typedef struct {
    char  *name;
    short type;
    short len;
    short dec;
} FIELD4INFO;

typedef struct {
    char *name;
    char *expression;
    char *filter;
    char *descending;
    short unique;
} TAG4INFO;

/* Function prototypes */
#ifdef __cplusplus
extern "C" {
#endif

int   d4init(CODE4 *c4);
int   d4init_undo(CODE4 *c4);
void  mem4reset(void);
void  e4hook(CODE4 *code_base, int err_code, char *desc1, char *desc2, char *desc3);

/* DATA4 operations used by cDBF class */
DATA4 *d4open(CODE4 *c4, char *name);
DATA4 *d4create(CODE4 *c4, char *name, FIELD4INFO *fields, TAG4INFO *tags);
int    d4close(DATA4 *d4);
int    d4pack(DATA4 *d4);
int    d4lock(DATA4 *d4, long recNo);
int    d4unlock(DATA4 *d4);
int    d4top(DATA4 *d4);
int    d4bottom(DATA4 *d4);
int    d4skip(DATA4 *d4, long numSkip);
int    d4go(DATA4 *d4, long recNo);
long   d4reccount(DATA4 *d4);
long   d4recno(DATA4 *d4);
int    d4bof(DATA4 *d4);
int    d4eof(DATA4 *d4);
int    d4blank(DATA4 *d4);
int    d4delete(DATA4 *d4);
int    d4recall(DATA4 *d4);
int    d4deleted(DATA4 *d4);
int    d4changed(DATA4 *d4, int flag);
int    d4append_start(DATA4 *d4, int lockFlag);
int    d4append(DATA4 *d4);
char  *d4alias(DATA4 *d4);
int    d4flush_all(DATA4 *d4);
int    d4refresh(DATA4 *d4);
int    d4seek(DATA4 *d4, char *key);

FIELD4 *d4field(DATA4 *d4, char *name);
int     d4numFields(DATA4 *d4);
char   *d4fieldName(DATA4 *d4, int n);
char    d4fieldType(DATA4 *d4, char *name);
int     d4fieldLen(DATA4 *d4, char *name);
int     d4fieldDec(DATA4 *d4, char *name);

/* Field I/O */
void  f4assign(FIELD4 *f4, char *val);
char *f4str(FIELD4 *f4);
void  f4assignInt(FIELD4 *f4, int val);
void  f4assignLong(FIELD4 *f4, long val);
void  f4assignDouble(FIELD4 *f4, double val);
void  f4assignChar(FIELD4 *f4, char val);

/* Index/Tag */
int   i4create(DATA4 *d4, char *name, TAG4INFO *tags);
int   t4select(DATA4 *d4, char *tagName);

/* Error */
int   code4errorCode(CODE4 *c4);

#ifdef __cplusplus
}
#endif

#endif /* D4ALL_INC */
