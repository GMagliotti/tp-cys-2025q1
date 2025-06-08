#include "lsb_decoder.h"

bool lsb_decoder_lsb1_extract_to_buffer(uint8_t *out_shadow_data, size_t shadow_len, const BMPImageT *cover)
{
    if (!out_shadow_data || !cover || !cover->pixels)
        return false;

    size_t bits_needed = shadow_len * 8;

    uint32_t width_bytes = cover->width * cover->bpp / 8;
    uint32_t padded_width_bytes = (width_bytes % 4 == 0) ? width_bytes : (width_bytes + (4 - (width_bytes % 4)));
    size_t cover_capacity = padded_width_bytes * cover->height;

    if (cover_capacity < bits_needed)
    {
        fprintf(stderr, "Cover image too small to extract shadow data\n");
        return false;
    }

    const uint8_t *cover_data = cover->pixels;

    size_t shadow_byte_index = 0;
    uint8_t current_byte = 0;
    int bit_index = 7; // Start from MSB

    for (size_t i = 0; i < cover_capacity && shadow_byte_index < shadow_len; ++i)
    {
        uint8_t bit = cover_data[i] & 0x01;
        current_byte |= (bit << bit_index);

        bit_index--;
        if (bit_index < 0)
        {
            out_shadow_data[shadow_byte_index] = current_byte;
            current_byte = 0;
            bit_index = 7;
            shadow_byte_index++;
        }
    }

    return true;
}