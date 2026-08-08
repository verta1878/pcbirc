/* ====================================================================
 * md5.c — MD5 Message Digest Algorithm (RFC 1321)
 * ====================================================================
 * Based on the RSA Data Security, Inc. reference implementation
 * which was released into the public domain.
 * ==================================================================== */

#include <string.h>
#include "md5.h"

static unsigned char PADDING[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

#define FF(a,b,c,d,x,s,ac) { (a) += F((b),(c),(d)) + (x) + (UINT4)(ac); (a) = ROTATE_LEFT((a),(s)); (a) += (b); }
#define GG(a,b,c,d,x,s,ac) { (a) += G((b),(c),(d)) + (x) + (UINT4)(ac); (a) = ROTATE_LEFT((a),(s)); (a) += (b); }
#define HH(a,b,c,d,x,s,ac) { (a) += H((b),(c),(d)) + (x) + (UINT4)(ac); (a) = ROTATE_LEFT((a),(s)); (a) += (b); }
#define II(a,b,c,d,x,s,ac) { (a) += I((b),(c),(d)) + (x) + (UINT4)(ac); (a) = ROTATE_LEFT((a),(s)); (a) += (b); }

static void Encode(unsigned char *output, UINT4 *input, unsigned int len)
{
    unsigned int i, j;
    for (i = 0, j = 0; j < len; i++, j += 4) {
        output[j]   = (unsigned char)(input[i] & 0xff);
        output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
        output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
        output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
    }
}

static void Decode(UINT4 *output, unsigned char *input, unsigned int len)
{
    unsigned int i, j;
    for (i = 0, j = 0; j < len; i++, j += 4)
        output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
                    (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}

static void MD5Transform(UINT4 state[4], unsigned char block[64])
{
    UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

    Decode(x, block, 64);

    /* Round 1 */
    FF(a,b,c,d,x[ 0], 7, 0xd76aa478); FF(d,a,b,c,x[ 1],12, 0xe8c7b756);
    FF(c,d,a,b,x[ 2],17, 0x242070db); FF(b,c,d,a,x[ 3],22, 0xc1bdceee);
    FF(a,b,c,d,x[ 4], 7, 0xf57c0faf); FF(d,a,b,c,x[ 5],12, 0x4787c62a);
    FF(c,d,a,b,x[ 6],17, 0xa8304613); FF(b,c,d,a,x[ 7],22, 0xfd469501);
    FF(a,b,c,d,x[ 8], 7, 0x698098d8); FF(d,a,b,c,x[ 9],12, 0x8b44f7af);
    FF(c,d,a,b,x[10],17, 0xffff5bb1); FF(b,c,d,a,x[11],22, 0x895cd7be);
    FF(a,b,c,d,x[12], 7, 0x6b901122); FF(d,a,b,c,x[13],12, 0xfd987193);
    FF(c,d,a,b,x[14],17, 0xa679438e); FF(b,c,d,a,x[15],22, 0x49b40821);

    /* Round 2 */
    GG(a,b,c,d,x[ 1], 5, 0xf61e2562); GG(d,a,b,c,x[ 6], 9, 0xc040b340);
    GG(c,d,a,b,x[11],14, 0x265e5a51); GG(b,c,d,a,x[ 0],20, 0xe9b6c7aa);
    GG(a,b,c,d,x[ 5], 5, 0xd62f105d); GG(d,a,b,c,x[10], 9, 0x02441453);
    GG(c,d,a,b,x[15],14, 0xd8a1e681); GG(b,c,d,a,x[ 4],20, 0xe7d3fbc8);
    GG(a,b,c,d,x[ 9], 5, 0x21e1cde6); GG(d,a,b,c,x[14], 9, 0xc33707d6);
    GG(c,d,a,b,x[ 3],14, 0xf4d50d87); GG(b,c,d,a,x[ 8],20, 0x455a14ed);
    GG(a,b,c,d,x[13], 5, 0xa9e3e905); GG(d,a,b,c,x[ 2], 9, 0xfcefa3f8);
    GG(c,d,a,b,x[ 7],14, 0x676f02d9); GG(b,c,d,a,x[12],20, 0x8d2a4c8a);

    /* Round 3 */
    HH(a,b,c,d,x[ 5], 4, 0xfffa3942); HH(d,a,b,c,x[ 8],11, 0x8771f681);
    HH(c,d,a,b,x[11],16, 0x6d9d6122); HH(b,c,d,a,x[14],23, 0xfde5380c);
    HH(a,b,c,d,x[ 1], 4, 0xa4beea44); HH(d,a,b,c,x[ 4],11, 0x4bdecfa9);
    HH(c,d,a,b,x[ 7],16, 0xf6bb4b60); HH(b,c,d,a,x[10],23, 0xbebfbc70);
    HH(a,b,c,d,x[13], 4, 0x289b7ec6); HH(d,a,b,c,x[ 0],11, 0xeaa127fa);
    HH(c,d,a,b,x[ 3],16, 0xd4ef3085); HH(b,c,d,a,x[ 6],23, 0x04881d05);
    HH(a,b,c,d,x[ 9], 4, 0xd9d4d039); HH(d,a,b,c,x[12],11, 0xe6db99e5);
    HH(c,d,a,b,x[15],16, 0x1fa27cf8); HH(b,c,d,a,x[ 2],23, 0xc4ac5665);

    /* Round 4 */
    II(a,b,c,d,x[ 0], 6, 0xf4292244); II(d,a,b,c,x[ 7],10, 0x432aff97);
    II(c,d,a,b,x[14],15, 0xab9423a7); II(b,c,d,a,x[ 5],21, 0xfc93a039);
    II(a,b,c,d,x[12], 6, 0x655b59c3); II(d,a,b,c,x[ 3],10, 0x8f0ccc92);
    II(c,d,a,b,x[10],15, 0xffeff47d); II(b,c,d,a,x[ 1],21, 0x85845dd1);
    II(a,b,c,d,x[ 8], 6, 0x6fa87e4f); II(d,a,b,c,x[15],10, 0xfe2ce6e0);
    II(c,d,a,b,x[ 6],15, 0xa3014314); II(b,c,d,a,x[13],21, 0x4e0811a1);
    II(a,b,c,d,x[ 4], 6, 0xf7537e82); II(d,a,b,c,x[11],10, 0xbd3af235);
    II(c,d,a,b,x[ 2],15, 0x2ad7d2bb); II(b,c,d,a,x[ 9],21, 0xeb86d391);

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    memset((POINTER)x, 0, sizeof(x));
}

void MD5Init(MD5_CTX *context)
{
    context->count[0] = context->count[1] = 0;
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
}

void MD5Update(MD5_CTX *context, unsigned char *input, unsigned int inputLen)
{
    unsigned int i, index, partLen;
    index = (unsigned int)((context->count[0] >> 3) & 0x3F);
    if ((context->count[0] += ((UINT4)inputLen << 3)) < ((UINT4)inputLen << 3))
        context->count[1]++;
    context->count[1] += ((UINT4)inputLen >> 29);
    partLen = 64 - index;
    if (inputLen >= partLen) {
        memcpy((POINTER)&context->buffer[index], (POINTER)input, partLen);
        MD5Transform(context->state, context->buffer);
        for (i = partLen; i + 63 < inputLen; i += 64)
            MD5Transform(context->state, &input[i]);
        index = 0;
    } else {
        i = 0;
    }
    memcpy((POINTER)&context->buffer[index], (POINTER)&input[i], inputLen - i);
}

void MD5Final(unsigned char digest[16], MD5_CTX *context)
{
    unsigned char bits[8];
    unsigned int index, padLen;
    Encode(bits, context->count, 8);
    index = (unsigned int)((context->count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    MD5Update(context, PADDING, padLen);
    MD5Update(context, bits, 8);
    Encode(digest, context->state, 16);
    memset((POINTER)context, 0, sizeof(*context));
}
