#ifndef _LSB_DECODER_H_
#define _LSB_DECODER_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "bmp.h"

/**
 * @brief Decodes shadow data from a BMP image using LSB 1-bit method.
 * @param out_shadow_data Pointer to the output buffer where shadow data will be stored.
 * @param shadow_len Length of the array of shadow data to be extracted.
 * @param cover Pointer to the BMPImageT structure containing the cover image.
 * @note This method expects the cover image to only shadow data in its least significant bits.
 *       The secret image's width and height are considered to be same as the cover image's width and height.
 */
bool lsb_decoder_lsb1_extract_to_buffer(uint8_t *out_shadow_data,
                           size_t shadow_len,
                           const BMPImageT *cover);

#endif