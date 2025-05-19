#include "permutation_table.h"

static uint16_t seed_gen = 0L;
static int64_t seed = 0L;

void ptable_set_seed(uint16_t s)
{
    seed_gen = s;
    seed = (s ^ 0x5DEECE66DL) & ((1LL << 48) - 1);
}

uint16_t ptable_get_seed(void)
{
    // Could do s ^ 0x5DEECE66DL
    return seed_gen;
}

uint8_t ptable_next_char(void)
{
    seed = (seed * 0x5DEECE66DL + 0xBL) & ((1LL << 48) - 1);
    return (uint8_t)(seed >> 40);
}

uint8_t *ptable_get_rng_table_unaligned(size_t size) {
    int64_t seed = 0L; 
    uint8_t *table = malloc(size * sizeof(uint8_t));

    if (table == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for RNG table\n");
        return NULL;
    }

    ptable_set_seed(seed);

    for (int i = 0; i < size * sizeof(uint8_t); i++)
    {
        table[i] = ptable_next_char();
    }
    return table;
}

uint8_t *ptable_get_rng_table_4bytealigned(int width, int height, uint16_t seed) {
    uint32_t padwidth = (width % 4 == 0) ? width : (width + (4 - (width % 4)));
    uint32_t padsize = padwidth * height;
    uint8_t *table = malloc(padsize * sizeof(uint8_t));
    if (table == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for RNG table\n");
        return NULL;
    }

    ptable_set_seed(seed);

    for (int i = 0; i < padsize; i++)
    {
        table[i] = ptable_next_char();
    }
    return table;
}