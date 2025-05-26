#ifndef __SSS_HELPERS_H__
#define __SSS_HELPERS_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "bmp.h"

typedef struct {
    bool result;
    int16_t s_width;
    int16_t s_height;
} LSBDecodeResultT;

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

/**
 * Hide an array of packets of k bits of data inside a cover image using 1bit LSB steganography
 * @param shadow_data The array of data packets to hide.
 * @param shadow_len The amount of packets expected to be hidden.
 * @param cover The cover image where the data will be hidden.
 * @param seed The seed used to generate the pseudo-random table.
 * @param k The number of bits to use for hiding (1-10).
 * @param s_width The width of the secret image.
 * @param s_height The height of the secret image.
 * 
 * @return true if the data was hidden successfully, false otherwise.
 * 
 * @note The width and height are stored in the first 4 bytes of the cover image, with
 *       the width in the first 2 bytes and the height in the next 2 bytes.
 */
bool sssh_lsb1_into_cover_k(const uint16_t *shadow_data, size_t shadow_len, BMPImageT *cover, uint16_t seed, int k, int16_t s_width, int16_t s_height);

bool extract_shadow_lsb_to_buffer(uint8_t *out_shadow_data, size_t shadow_len, const BMPImageT *cover);

LSBDecodeResultT sssh_extract_lsb1_kshadow(uint16_t *out_shadow_data, size_t shadow_len, const BMPImageT *cover, int k);

LSBDecodeResultT sssh_extract_kshadow_dimensions(const BMPImageT *cover);
#endif