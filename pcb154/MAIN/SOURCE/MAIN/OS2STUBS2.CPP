/* C++ typed stubs needing PCBoard headers for mangled name matching */
/* GPL v3.0 — pcbrevival project by hexadecimal */
#include "project.h"
#pragma hdrstop
#include "pcboard.h"
#include "structs.h"
#include "scrnio.h"

/* Screen globals needing enum types */


/* Usernet with proper nodetype */
void LIBENTRY readusernetrecord(unsigned r, nodetype *b) { (void)r;(void)b; }
void LIBENTRY updateusernetrecord(unsigned r, nodetype *b) { (void)r;(void)b; }

/* updatelines with correct enum signature */
void LIBENTRY updatelines(displaytype d, int a, int b) { (void)d;(void)a;(void)b; }

/* dosfindfirst/dosfindnext with correct types */
int LIBENTRY dosfindfirst(char *p, struct find_t *b, int a, int *d, int *c) {
    (void)p;(void)b;(void)a;(void)d;(void)c; return -1;
}
int LIBENTRY dosfindnext(struct find_t *b, int *d, int c) {
    (void)b;(void)d;(void)c; return -1;
}

/* mktemp */
char * LIBENTRY mktemp(char *t) { return t; }

/* Menu */
MenuType Menu[64];

/* Screen adapter type */
vidcardtype Scrn_Adapter = VID_VGA;
