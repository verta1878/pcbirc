/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* install-1011.c -- PCBoard Installer (install v1.11.0)                     */
/*                                                                            */
/* Byte-exact reconstruction of Clark's INSTALL.EXE (338,548 B, NE Family    */
/* API, linker bytes 5.10). The v1.10 arc produced a portable-C functional   */
/* reimplementation (see install-1010.c) that runs INSTALL.DAT end-to-end   */
/* with 94.5% byte-perfect output. The v1.11+ arc drives the compiled       */
/* binary itself toward byte-exact match with Clark's INSTALL.EXE.          */
/*                                                                            */
/* Toolchain (v1.10.6 parity report + v1.11.0 sandbox verification):        */
/*   Compiler: Borland C++ 3.1                                              */
/*   Linker:   TLINK 5.1 (writes NE linker bytes 05 0A = decimal 5.10       */
/*             - matches Clark's INSTALL.EXE exactly)                       */
/*   Libs:     BC 3.1 standard libs + API.LIB from OS/2 SDK 1.03            */
/*             (provides DOSCALLS/KBDCALLS/VIOCALLS Family API imports)     */
/*                                                                            */
/* Phase roadmap (see docs/pcboard-internals/INSTALL-EXE-PARITY.md):        */
/*   v1.11.0  Toolchain shakedown - THIS FILE. Empty main(), verifies       */
/*            build produces NE binary with linker bytes 5.10.               */
/*   v1.11.1  Directive dispatch table (329 directives from parity report)  */
/*   v1.11.2  Port the 60 semantic handlers from install-1010.c             */
/*   v1.11.3  System-query family (~50 directives)                          */
/*   v1.11.4  Extended string ops + file I/O                                */
/*   v1.11.5  Process integration                                           */
/*   v1.11.6  Advanced flow + UI                                            */
/*   v1.11.7  Constraint enforcement                                        */
/*   v1.11.8  Memory layout + code order matching                           */
/*   v1.11.9  Compile-diff loop (cmp -l target: 0)                          */
/*   v1.11.10 Understanding-complete + byte-exact milestone                 */
/*                                                                            */
/* Build (under DOSBox-X):                                                   */
/*   Run pcb1541/install/build/BLDINS.BAT                                   */
/*                                                                            */
/* Manual build:                                                             */
/*   BCC.EXE -c -ml -IC:\BC31\INCLUDE install-1011.c                       */
/*   TLINK.EXE /Twe C:\BC31\LIB\C0L.OBJ install-1011.OBJ,                  */
/*                    INSTALL.EXE, INSTALL.MAP,                            */
/*                    C:\BC31\LIB\CL.LIB C:\OS2SDK\LIB\API.LIB             */
/*                                                                            */
/* Output verification:                                                      */
/*   file INSTALL.EXE            # should say NE for OS/2 1.x                */
/*   python3 -c "import struct; d=open('INSTALL.EXE','rb').read();          */
/*                n=struct.unpack_from('<I',d,0x3C)[0];                     */
/*                print(f'linker: {d[n+2]}.{d[n+3]:02d}')"                  */
/*   # expected: linker: 5.10                                               */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

int main(void)
{
    /* v1.11.0: empty. Toolchain shakedown only.                             */
    /* v1.11.1 will land the directive dispatch table + minimal main loop.  */
    return 0;
}
