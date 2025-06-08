#ifndef _LSB_DECODER_H_
#define _LSB_DECODER_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "bmp.h"

typedef struct
{
    bool result;
    uint16_t s_width;
    uint16_t s_height;
} LSBDecodeResult;

/**
 * @brief Decodes shadow data from a BMP image using LSB 1-bit method.
 * @param out_shadow_data Pointer to the output buffer where shadow data will be stored.
 * @param shadow_len Length of the array of shadow data to be extracted.
 * @param cover Pointer to the BMPImageT structure containing the cover image.
 * @note This method expects the cover image to only shadow data in its least significant bits.
 *       The secret image's width and height are considered to be same as the cover image's width and height.
 * @warning This function fails to work if the number of shares is not 8 (eight).
 */
bool lsb_decoder_lsb1_extract_to_buffer(uint8_t *out_shadow_data,
                                        size_t shadow_len,
                                        const BMPImageT *cover);

/**
 * @brief Decodes shadow data from a BMP image using LSB 1-bit method.
 * @param out_shadow_data Pointer to the output buffer where shadow data will be stored.
 * @param shadow_len Length of the array of shadow data to be extracted.
 * @param cover Pointer to the BMPImageT structure containing the cover image.
 * @param k The threshold number of shares required to reconstruct the image.
 */
LSBDecodeResult lsb_decoder_lsb1_extract_to_buffer_extended(uint8_t *out_shadow_data,
                                                            size_t shadow_len,
                                                            const BMPImageT *cover,
                                                            int k);

/**
 * @brief Extracts dimensions (width and height) of the secret image from a BMP
 * cover image using LSB 1-bit method.
 * @param cover Pointer to the BMPImageT structure containing the cover image.
 * @return LSBDecodeResult containing the result status and dimensions of the secret image.
 * @note The dimensions are extracted from the first 32 bits of the cover image's pixel data.
 *       The first 16 bits represent the width and the next 16 bits represent the height.
 *       If the cover image is invalid or too small, the result will be false and dimensions will be 0.
 */
LSBDecodeResult lsb_decoder_lsb1_get_dimensions(const BMPImageT *cover);

#endif