/* ============================================================================
 * patch_template.c — sketch for the COMMDRV.EXE fix in red_decompress.c
 *
 * Fill in the blocks marked FROM_GHIDRA once you have the decompile.
 * ==========================================================================*/

/* ---- Add to the top of red_decompress.c near other constants ---------- */

/* WCSC chunk size: I/O layer (kick_char/f_ram/flushram) flushes the compressed
 * output buffer every N bytes and does <something> to preserve state. Confirmed
 * from divergence analysis to be 4096 bytes of compressed stream. Definition
 * of "does something" is what the Ghidra session pins down. */
#define WCSC_CHUNK_BYTES  4096

/* ---- Add helper function ---------------------------------------------- */

/* Called from BitIO whenever bio.pos crosses a WCSC_CHUNK_BYTES boundary.
 * FROM_GHIDRA: fill in based on what flushram()/f_ram() do. Likely one of:
 *   (a) Discard remaining bits, byte-align to next fresh byte
 *   (b) Skip N padding bytes at the boundary
 *   (c) Reset bit_buf/bit_cnt but keep byte position
 *   (d) Read a marker byte/word that signals the next state
 */
static void wcsc_chunk_boundary(BitIO *bio) {
    /* FROM_GHIDRA: implement one of the above patterns */
    /* Example if (a):
     *   bio->bit_cnt = 0;
     *   bio->bit_buf = 0;
     *   // pos already at byte boundary — nothing more to do
     */
    /* Example if (b):
     *   bio->pos += N;
     *   // where N is the padding count from flushram's disassembly
     */
    (void)bio;
}

/* ---- Modify fillbuf() to check chunk boundary ------------------------- */

/* In BitIO_fillbuf, after each getc(), add:
 *
 *   if (bio->pos > 0 && (bio->pos % WCSC_CHUNK_BYTES) == 0) {
 *       wcsc_chunk_boundary(bio);
 *   }
 *
 * Placement matters — do it AFTER the byte is read into bit_buf,
 * BEFORE further bit consumption. See existing fillbuf() for context.
 */
