/* lh5_encode.c — Native LHA -lh5- encoder for redx
 *
 * install v1.8.1: SCAFFOLD ONLY. Real implementation lands in
 * v1.8.1.1 through v1.8.1.4. See DESIGN.md and ACCEPTANCE.md.
 *
 * Until then, calling lh5_encode() returns 0 (failure) and
 * red_pack.c falls back to its existing shell-out path via
 * lha_compress() in red_pack.c.
 */

#include "lh5_encode.h"
#include <string.h>

const char *lh5_encode_version(void)
{
    return "lh5_encode 0.1-scaffold (install v1.8.1)";
}

size_t lh5_encode(const uint8_t *src, size_t src_len,
                  uint8_t *dst, size_t dst_cap,
                  uint16_t *crc16)
{
    /* TODO(v1.8.1.1): LZSS pass — 8192-byte sliding window + hash
     * chain match search. See ../../lha/src/slide.c for reference
     * implementation. */
    /* TODO(v1.8.1.2): Huffman pass — static Huffman on top of LZSS
     * output. See ../../lha/src/huf.c + maketree.c + maketbl.c. */
    /* TODO(v1.8.1.2): bit-level output stream. See bitio.c. */

    (void)src; (void)src_len; (void)dst; (void)dst_cap; (void)crc16;
    return 0;  /* signal: fall back to shell-out */
}

uint8_t *lha_wrap_level0(const uint8_t *lh5_stream, size_t lh5_len,
                         uint32_t unc_size, uint16_t crc16,
                         const char *filename, size_t *out_len)
{
    /* TODO(v1.8.1.3): level-0 LHA header emission:
     *   [0]     header size (u8)
     *   [1]     header checksum (u8, sum of bytes 2..header_size)
     *   [2..6]  method ID: "-lh5-"
     *   [7..10] compressed size (LE u32)
     *   [11..14] uncompressed size (LE u32)
     *   [15..18] timestamp (MS-DOS format, LE u32)
     *   [19]    file attribute (u8, 0x20 for normal file)
     *   [20]    level identifier (u8, 0x00 for level-0)
     *   [21]    filename length (u8)
     *   [22..]  filename
     *   after filename: CRC-16 (LE u16)
     *   then:   compressed payload (lh5_stream, lh5_len bytes)
     */
    (void)lh5_stream; (void)lh5_len; (void)unc_size;
    (void)crc16; (void)filename; (void)out_len;
    return NULL;  /* not implemented */
}
