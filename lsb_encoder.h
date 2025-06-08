#ifndef _LSB_ENCODER_H_
#define _LSB_ENCODER_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "bmp.h"

/**
 * @brief Encodes shadow data into a BMP image using LSB 1-bit method.
 * @param shadow_data Pointer to the input buffer containing shadow data to be hidden.
 * @param shadow_len Length of the shadow data buffer.
 * @param cover Pointer to the BMPImageT structure containing the cover image.
 * @param seed A seed value for randomization (not used in this function).
 * @return true if the shadow data was successfully encoded into the cover image, false otherwise.
 * @note This function modifies the cover image's pixel data in-place.
 * @warning This function fails to work if the number of shares is not 8 (eight).
 */
bool lsb_encoder_lsb1_into_cover(const uint8_t *shadow_data,
                                 size_t shadow_len,
                                 BMPImageT *cover,
                                 uint16_t seed);

/**
 * @brief Encodes shadow data into a BMP image using LSB 1-bit method.
 * @param shadow_data Pointer to the input buffer containing shadow data to be hidden.
 * @param shadow_len Length of the shadow data buffer.
 * @param cover Pointer to the BMPImageT structure containing the cover image.
 * @param seed A seed value for randomization (not used in this function).
 * @param k The threshold number of shares required to reconstruct the image.
 * @param s_width The width of the secret image to be encoded.
 * @param s_height The height of the secret image to be encoded.
 * @return true if the shadow data was successfully encoded into the cover image, false otherwise.
 * @note This function modifies the cover image's pixel data in-place.
 */
bool lsb_encoder_lsb1_into_cover_extended(const uint8_t *shadow_data,
                                          size_t shadow_len,
                                          BMPImageT *cover,
                                          uint16_t seed,
                                          int k,
                                          int16_t s_width,
                                          int16_t s_height);

#endif