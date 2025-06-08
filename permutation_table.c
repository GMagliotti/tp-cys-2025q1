#include "permutation_table.h"

static uint16_t gl_seed_gen = 0L;
static int64_t gl_seed = 0L;

void 
rngpt_set_seed(int64_t s)
{
    uint16_t s16 = (uint16_t)(s & 0xFFFF);  // Force to 16-bit seed
    gl_seed_gen = s16;
    gl_seed = ((int64_t)s16 ^ 0x5DEECE66DL) & ((1LL << 48) - 1);
}

uint8_t
rngpt_next_char(void)
{
    gl_seed = (gl_seed * 0x5DEECE66DL + 0xBL) & ((1LL << 48) - 1);
    return (uint8_t)(gl_seed >> 40);
}

uint8_t *
rngpt_get_byte_table_noalign(size_t size)
{
    uint8_t *table = malloc(size * sizeof(uint8_t));

    if (table == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for RNG table\n");
        return NULL;
    }

    printf("Using seed: %ld (%lx)\n", gl_seed, gl_seed);
    printf("Generated with seed: %d (%x)\n", gl_seed_gen, gl_seed_gen);

    for (int i = 0; i < size * sizeof(uint8_t); i++)
    {
        table[i] = rngpt_next_char();
    }
    return table;
}

uint8_t *
rngpt_get_byte_table_4balign(BMPImageT *image)
{
    int scanline_size = bmp_align(image->width);
    int image_size = scanline_size * image->height;
    uint8_t *table = malloc(image_size * sizeof(uint8_t));

    if (table == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for RNG table\n");
        return NULL;
    }

    printf("Using seed: %ld (%lx)\n", gl_seed, gl_seed);
    printf("Generated with seed: %d (%x)\n", gl_seed_gen, gl_seed_gen);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < scanline_size; x++)
        {
            int index = y * scanline_size + x;
            if (index < image_size)
            {
                table[index] = rngpt_next_char();
            }
        }
    }
    
    return table;
}

void
rngpt_inplace_xor(uint8_t *table1, uint8_t *table2, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        table1[i] ^= table2[i];
    }
}

void
rngpt_inplace_xor_aligned(BMPImageT *image, uint8_t *table)
{
    int scanline_size = bmp_align(image->width);
    int image_size = scanline_size * image->height;
    for (int i = 0; i < image_size; i++)
    {
        int x = i % scanline_size;
        int y = i / scanline_size;
        int8_t *ptr = bmp_get_pixel_address(image, x, y);
        *ptr ^= table[i];
    }
}

// void ptable_set_seed(uint16_t s)
// {
//     seed_gen = s;
//     seed = (s ^ 0x5DEECE66DL) & ((1LL << 48) - 1);
// }

// uint16_t ptable_get_seed(void)
// {
//     // Could do s ^ 0x5DEECE66DL
//     return seed_gen;
// }

// uint8_t ptable_next_char(void)
// {
//     seed = (seed * 0x5DEECE66DL + 0xBL) & ((1LL << 48) - 1);
//     return (uint8_t)(seed >> 40);
// }

// uint8_t *ptable_get_rng_table_unaligned(size_t size)
// {
//     uint8_t *table = malloc(size * sizeof(uint8_t));

//     if (table == NULL)
//     {
//         fprintf(stderr, "Failed to allocate memory for RNG table\n");
//         return NULL;
//     }

//     printf("Using seed: %x\n", seed_gen);
//     printf("Generated with seed: %ld (%lx)\n", seed, seed);

//     for (int i = 0; i < size * sizeof(uint8_t); i++)
//     {
//         table[i] = ptable_next_char();
//     }
//     return table;
// }

// uint8_t *ptable_get_rng_table_4bytealigned(int width, int height, uint16_t seed)
// {
//     uint32_t padwidth = (width % 4 == 0) ? width : (width + (4 - (width % 4)));
//     uint32_t padsize = padwidth * height;
//     uint8_t *table = malloc(padsize * sizeof(uint8_t));
//     if (table == NULL)
//     {
//         fprintf(stderr, "Failed to allocate memory for RNG table\n");
//         return NULL;
//     }

//     ptable_set_seed(seed);

//     for (int i = 0; i < padsize; i++)
//     {
//         table[i] = ptable_next_char();
//     }
//     return table;
// }