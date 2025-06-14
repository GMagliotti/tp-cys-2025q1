#ifndef _PERMATRAGO_H
#define _PERMATRAGO_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "bmp.h"

void 
rngpt_set_seed(int64_t s);

uint8_t *
rngpt_get_byte_table_noalign(size_t size);

uint8_t *
rngpt_get_byte_table_4balign(BMPImageT *image);

void
rngpt_inplace_xor(uint8_t *table1, uint8_t *table2, size_t size);

void
rngpt_inplace_xor_aligned(BMPImageT *image, uint8_t *table);


// /**
//  * @brief Sets the seed for the pseudo-random character generator.
//  *
//  * Initializes the internal state of the random character generator
//  * using the specified seed value. This allows for reproducible sequences
//  * of characters if the same seed is used.
//  *
//  * @param s The seed value to initialize the generator with.
//  */
// void ptable_set_seed(uint16_t s);

// /**
//  * @brief Generates the next pseudo-random character.
//  *
//  * Returns the next character from the pseudo-random sequence.
//  * The output is influenced by the previously set seed value.
//  *
//  * @return A pseudo-random 8-bit character.
//  */
// uint8_t ptable_next_char(void);

// /**
//  * @brief Allocates and fills a table with pseudo-random characters.
//  * 
//  * The output is influenced by the previously set seed value.
//  * 
//  * @param size The number of characters to generate and fill in the table.
//  * @param seed_ptr Pointer to a 16-bit seed value. If NULL, the default seed is used.
//  * 
//  * @return A pointer to the allocated table filled with random characters,
//  *         or NULL if memory allocation fails.
//  * 
//  * @note The caller is responsible for freeing the allocated memory for
//  *       the table when it is no longer needed.
//  */
// uint8_t *ptable_get_rng_table_unaligned(size_t size);

// uint8_t *ptable_get_rng_table_4bytealigned(int width, int height, uint16_t seed_ptr);
#endif