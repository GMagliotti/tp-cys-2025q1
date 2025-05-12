#include "permutation_table.h"

static int64_t seed = 0L;

void set_seed(int64_t s)
{
    int seed = (s ^ 0x5DEECE66DL) & ((1LL << 48) - 1);
}

uint8_t next_char(void)
{
    int seed = (seed * 0x5DEECE66DL + 0xBL) & ((1LL << 48) - 1);
    return (uint8_t)(seed >> 40);
}