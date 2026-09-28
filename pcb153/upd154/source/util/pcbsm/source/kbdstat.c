/* KbdStatus global — provides the far pointer to keyboard status byte. */
/* Defined in Clark's original INITSCRN.C but commented out in 15.3 source. */
/* hexadecimal, v0.032. */

typedef unsigned int uint;

uint *KbdStatus = (uint far *)0x00400017L;  /* BIOS keyboard status @ 0040:0017 */
