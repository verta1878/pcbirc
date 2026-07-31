/* SETUPGLB.C - Global variable stubs for PCBSETUP link */
int OldCnames = 0;
int PerformValidation = 1;
int WriteConf = 0;
char pcbfile[128] = {0};
int UpdateKbdStatus = 0;
int mask_numbers = 0;
char CompressBat[128] = {0};

/* shelltodos stub - not needed in PCBSETUP */
void shelltodos(void) {}

/* ComSpec - moved to far to avoid DGROUP overflow */
char far ComSpec[128] = {0};
char QwkBat[128] = {0};
