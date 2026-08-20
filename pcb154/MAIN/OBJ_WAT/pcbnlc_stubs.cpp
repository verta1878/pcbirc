/* pcbnlc_stubs.cpp — stubs for PCBNLC linking */
#include <string.h>
#include "d4all.h"

/* Include dbase.hpp for cDBF class */
#include "dbase.hpp"

/* Include PCBoard FidoNet structures */
#include "structs.h"
#include "prototyp.h"

/* cDBF implementation stubs — placeholder for CodeBase operations */
static CODE4 _code4;
static int _cdbf_err = 0;

cDBF::cDBF(void) { memset(&_code4, 0, sizeof(_code4)); d4init(&_code4); }
cDBF::~cDBF(void) { d4init_undo(&_code4); }

void LIBENTRY cDBF::dbfCreate(char *name, int exclusive, char **finfo) {
    (void)name; (void)exclusive; (void)finfo;
    _cdbf_err = 0;
}
void LIBENTRY cDBF::dbfOpen(char *name, int exclusive) {
    (void)name; (void)exclusive;
    _cdbf_err = 0;
}
void LIBENTRY cDBF::dbfClose(void) { }
void LIBENTRY cDBF::ndxCreate(char *name, char *expr) {
    (void)name; (void)expr;
}
int LIBENTRY cDBF::error(void) { return _cdbf_err; }

/* CodeBase C functions */
extern "C" {
    int d4init(CODE4 *c4) { memset(c4, 0, sizeof(CODE4)); return 0; }
    int d4init_undo(CODE4 *c4) { (void)c4; return 0; }
    void mem4reset(void) { }
}

/* FidoNet stubs */
DIRECTORIES directory_info;
NODELIST *nodelist_list = 0;
unsigned num_lists = 0;

bool read_fido_config(int flags) { (void)flags; return 0; }
void free_fido_memory(void) { }
bool get_node(unsigned zone, unsigned net, unsigned node, NODE_REC *rec) {
    (void)zone; (void)net; (void)node; (void)rec;
    return 0;
}

/* cCODEBASE static members */
CODE4 cCODEBASE::c4;
int cCODEBASE::cnt = 0;
