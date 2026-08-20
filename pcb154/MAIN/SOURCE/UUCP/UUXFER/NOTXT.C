/* NOTXT.C - minimal stub for pcbtext functions */
#include <pcbtools.h>
void LIBENTRY closepcbtext(void) {}
int LIBENTRY readpcbtextfile(char *ext, int from) { (void)ext; (void)from; return 0; }
bool LIBENTRY getpcbtext(int num, pcbtexttype *buf) { (void)num; (void)buf; return FALSE; }
void LIBENTRY displaypcbtext(int num, DISPLAYTYPE disp) { (void)num; (void)disp; }
bool LIBENTRY pcbtextspaces(int num) { (void)num; return FALSE; }
void LIBENTRY smalltext(void) {}
