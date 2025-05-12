#ifndef _PERMATRAGO_H
#define _PERMATRAGO_H

#include <stdint.h>

/**
 * @brief Sets the seed for the pseudo-random character generator.
 *
 * Initializes the internal state of the random character generator
 * using the specified seed value. This allows for reproducible sequences
 * of characters if the same seed is used.
 *
 * @param s The seed value to initialize the generator with.
 */
void set_seed(int64_t s);

/**
 * @brief Generates the next pseudo-random character.
 *
 * Returns the next character from the pseudo-random sequence.
 * The output is influenced by the previously set seed value.
 *
 * @return A pseudo-random 8-bit character.
 */
uint8_t next_char(void);

#endif