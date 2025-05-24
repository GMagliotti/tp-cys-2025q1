#ifndef __SSS_HELPERS_H__
#define __SSS_HELPERS_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "bmp.h"
/**
 * Hide an array of data inside a cover image using LSB steganography.
 * @param shadow_data The array of data to hide.
 * @param shadow_len The amount of bytes expected to be hidden.
 * @param cover The cover image where the data will be hidden.
 * @param seed The seed used to generate the pseudo-random table.
 * 
 * @return true if the data was hidden successfully, false otherwise.
 */
bool sssh_8bit_lsb_into_cover(const uint8_t *shadow_data, size_t shadow_len, BMPImageT *cover, uint16_t seed);
#endif