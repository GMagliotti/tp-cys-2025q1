#include "permutation_table.h"

static int64_t seed = 0L;

void ptable_set_seed(int64_t s)
{
    seed = (s ^ 0x5DEECE66DL) & ((1LL << 48) - 1);
}

uint8_t ptable_next_char(void)
{
    seed = (seed * 0x5DEECE66DL + 0xBL) & ((1LL << 48) - 1);
    return (uint8_t)(seed >> 40);
}

uint8_t *ptable_get_rng_table(size_t size) {
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