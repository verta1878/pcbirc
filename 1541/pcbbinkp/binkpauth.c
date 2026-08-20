/* ====================================================================
 * binkpauth.c — CRAM-MD5 Authentication for BinkP
 * ====================================================================
 * Implements FSP-1024 CRAM-MD5 extension for BinkP sessions.
 * Uses RFC 1321 MD5 (public domain reference implementation).
 *
 * Copyright (C) 2026 pcbrevival contributors
 * License: GPLv3
 * ==================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "binkp.h"
#include "md5.h"

/* ====================================================================
 * HMAC-MD5 (RFC 2104)
 * ====================================================================
 * CRAM-MD5 uses HMAC-MD5 with the password as key and the
 * challenge string as the message.
 * ==================================================================== */

static void hmac_md5(const unsigned char *key, int keylen,
                     const unsigned char *msg, int msglen,
                     unsigned char digest[16])
{
    MD5_CTX ctx;
    unsigned char ipad[64], opad[64];
    unsigned char key_hash[16];
    int i;

    /* If key > 64 bytes, hash it first */
    if (keylen > 64) {
        MD5Init(&ctx);
        MD5Update(&ctx, (unsigned char *)key, keylen);
        MD5Final(key_hash, &ctx);
        key = key_hash;
        keylen = 16;
    }

    /* Prepare ipad and opad */
    memset(ipad, 0x36, 64);
    memset(opad, 0x5C, 64);
    for (i = 0; i < keylen; i++) {
        ipad[i] ^= key[i];
        opad[i] ^= key[i];
    }

    /* Inner: MD5(ipad + msg) */
    MD5Init(&ctx);
    MD5Update(&ctx, ipad, 64);
    MD5Update(&ctx, (unsigned char *)msg, msglen);
    MD5Final(digest, &ctx);

    /* Outer: MD5(opad + inner_digest) */
    MD5Init(&ctx);
    MD5Update(&ctx, opad, 64);
    MD5Update(&ctx, digest, 16);
    MD5Final(digest, &ctx);
}

/* ====================================================================
 * Generate CRAM-MD5 challenge string
 * ====================================================================
 * Format: hex string of random bytes. BinkP uses a simple hex
 * challenge derived from random data + timestamp.
 * ==================================================================== */

void binkp_make_challenge(char *buf, int bufsize)
{
    unsigned long seed;
    unsigned char raw[16];
    int i;

    /* Generate pseudo-random challenge from time + counter */
    seed = (unsigned long)time(NULL);
    seed ^= (unsigned long)clock();

    for (i = 0; i < 16; i++) {
        seed = seed * 1103515245UL + 12345UL;
        raw[i] = (unsigned char)((seed >> 16) & 0xFF);
    }

    /* Convert to hex string */
    for (i = 0; i < 16 && (i * 2 + 2) < bufsize; i++)
        sprintf(buf + i * 2, "%02x", raw[i]);

    buf[(i * 2 < bufsize) ? i * 2 : bufsize - 1] = '\0';
}

/* ====================================================================
 * Build CRAM-MD5 digest response
 * ====================================================================
 * Takes password and challenge, returns hex digest string.
 * Used by calling (originating) side.
 * Returns 0 on success, -1 on error.
 * ==================================================================== */

int binkp_build_digest(const char *password, const char *challenge,
                       char *out, int outsize)
{
    unsigned char chal_bin[64];
    int chal_len;
    unsigned char digest[16];
    int i;
    char *p;

    if (!password || !challenge || !out)
        return -1;

    /* Decode hex challenge to binary */
    chal_len = (int)strlen(challenge) / 2;
    if (chal_len > (int)sizeof(chal_bin))
        chal_len = sizeof(chal_bin);

    for (i = 0; i < chal_len; i++) {
        unsigned int byte;
        if (sscanf(challenge + i * 2, "%02x", &byte) != 1)
            return -1;
        chal_bin[i] = (unsigned char)byte;
    }

    /* Compute HMAC-MD5 */
    hmac_md5((const unsigned char *)password, (int)strlen(password),
             chal_bin, chal_len, digest);

    /* Convert digest to hex string */
    if (outsize < 33)
        return -1;

    p = out;
    for (i = 0; i < 16; i++) {
        sprintf(p, "%02x", digest[i]);
        p += 2;
    }
    *p = '\0';

    return 0;
}

/* ====================================================================
 * Verify CRAM-MD5 digest
 * ====================================================================
 * Takes password, challenge, and received digest. Returns 1 if match.
 * Used by answering side.
 * ==================================================================== */

int binkp_verify_digest(const char *password, const char *challenge,
                        const char *digest)
{
    char expected[64];

    if (binkp_build_digest(password, challenge, expected, sizeof(expected)) < 0)
        return 0;

    /* Case-insensitive compare of hex strings */
    return (stricmp(expected, digest) == 0);
}
