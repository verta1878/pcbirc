/* NOSCROLL.C — Replaces NOSCROLL.ASM. Screen scroll control. */

#pragma aux BEGNOSCROLL "*"
#pragma aux ENDNOSCROLL "*"
#pragma aux SCROLLON "*"

static int noscroll_active = 0;

void BEGNOSCROLL(void) { noscroll_active = 1; }
void ENDNOSCROLL(void) { noscroll_active = 0; }
void SCROLLON(void)    { noscroll_active = 0; }
