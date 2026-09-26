/* ============================================================================
 * card_pool.h — generic per-backend card pool
 *
 * Each smart-card backend has a small static pool of card records (max 4
 * cards per system). This helper finds an existing card by address key,
 * or allocates the next free slot. Card structs MUST start with:
 *   unsigned long addr;   // key
 *   unsigned char in_use; // 0 = free slot
 * License: GPLv3 (pcbirc crew)
 * ==========================================================================*/
#ifndef CARD_POOL_H
#define CARD_POOL_H

typedef struct {
    unsigned long addr;
    unsigned char in_use;
} card_pool_hdr_t;

/* Find or allocate a card by address key. Returns NULL if pool full. */
void *card_pool_get(void *pool_base, unsigned int stride,
                    unsigned int max_cards, unsigned long addr);

#endif
