/* lh5_encode.h — Native LHA -lh5- encoder for redx
 *
 * Copyright (C) 2026 hexadecimal / pcbirc project.
 * Licensed GPLv3.
 *
 * Public API for the native LH5 encoder that will replace the
 * shell-out to `lha 1.14i` in red_pack.c::lha_compress().
 *
 * IMPLEMENTATION STATUS: install v1.8.1 — API frozen, implementation
 * TODO. See DESIGN.md and ACCEPTANCE.md in this directory for the
 * roadmap. Until v1.8.1.5, red_pack.c continues to shell out.
 */

#ifndef LH5_ENCODE_H
#define LH5_ENCODE_H

#include <stddef.h>
#include <stdint.h>

/* Compress `src` (src_len bytes) to `dst` (up to dst_cap bytes) using
 * the LHA -lh5- algorithm (LZSS 8192-byte window + static Huffman).
 *
 * Returns: number of bytes written to dst, or 0 on failure.
 * Failure modes: dst_cap too small, src_len too large (> 16 MB), OOM.
 *
 * Sets *crc16 to the CRC-16-IBM (poly 0xA001) of src on success,
 * as required by the LHA level-0 header spec.
 *
 * Not thread-safe.
 */
size_t lh5_encode(const uint8_t *src, size_t src_len,
                  uint8_t *dst, size_t dst_cap,
                  uint16_t *crc16);

/* Wrap a raw LH5 stream in a level-0 LHA container (headers, CRC,
 * filename field). Level-0 is what red_pack.c currently parses from
 * lha 1.14i's output.
 *
 * Returns: malloc'd buffer of the complete level-0 archive (caller
 * frees). Sets *out_len to buffer size. Returns NULL on failure.
 */
uint8_t *lha_wrap_level0(const uint8_t *lh5_stream, size_t lh5_len,
                         uint32_t unc_size, uint16_t crc16,
                         const char *filename, size_t *out_len);

/* Version string for --version output. */
const char *lh5_encode_version(void);

#endif /* LH5_ENCODE_H */
