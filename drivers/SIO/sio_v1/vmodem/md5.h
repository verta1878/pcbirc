/* ====================================================================
 * md5.h — MD5 Message Digest (RFC 1321)
 * ====================================================================
 * Derived from the RSA Data Security, Inc. MD5 Message-Digest Algorithm
 * reference implementation, which was placed in the public domain.
 * ==================================================================== */

#ifndef MD5_H
#define MD5_H

typedef unsigned long  UINT4;
typedef unsigned char *POINTER;

typedef struct {
    UINT4 state[4];         /* ABCD state                              */
    UINT4 count[2];         /* Number of bits, mod 2^64                */
    unsigned char buffer[64]; /* Input buffer                          */
} MD5_CTX;

void MD5Init(MD5_CTX *context);
void MD5Update(MD5_CTX *context, unsigned char *input, unsigned int inputLen);
void MD5Final(unsigned char digest[16], MD5_CTX *context);

#endif /* MD5_H */
