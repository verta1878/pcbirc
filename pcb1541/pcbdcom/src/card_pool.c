/* card_pool.c — see card_pool.h. License: GPLv3 (pcbirc crew) */
#include <string.h>
#include "card_pool.h"

void *card_pool_get(void *pool_base, unsigned int stride,
                    unsigned int max_cards, unsigned long addr)
{
    unsigned int i;
    char *base = (char *)pool_base;
    for (i = 0; i < max_cards; i++) {
        card_pool_hdr_t *hdr = (card_pool_hdr_t *)(base + i * stride);
        if (hdr->in_use && hdr->addr == addr) return hdr;   /* existing */
    }
    for (i = 0; i < max_cards; i++) {
        card_pool_hdr_t *hdr = (card_pool_hdr_t *)(base + i * stride);
        if (!hdr->in_use) {
            memset(hdr, 0, stride);
            hdr->addr = addr;
            hdr->in_use = 1;
            return hdr;
        }
    }
    return 0;   /* pool full */
}
