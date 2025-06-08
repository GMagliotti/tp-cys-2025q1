#include "lsb_encoder.h"
#define METADATA_SIZE 32 // 2 bytes for width and 2 bytes for height * 8 bits per byte

bool lsb_encoder_lsb1_into_cover(const uint8_t *shadow_data, size_t shadow_len, BMPImageT *cover, uint16_t seed)
{
    if (!shadow_data || !cover || !cover->pixels)
        return false;

    // Each byte in the shadow buffer needs 8 bits (LSBs in the cover)
    size_t bits_needed = shadow_len * 8;

    uint32_t width_bytes = cover->width * cover->bpp / 8;
    uint32_t padded_width_bytes = bmp_align(width_bytes); // Align to 4 bytes
    size_t cover_capacity = padded_width_bytes * cover->height;

    if (cover_capacity < bits_needed)
    {
        fprintf(stderr, "Cover image too small to hide shadow data\n");
        return false;
    }

    uint8_t *cover_data = cover->pixels;

    size_t shadow_byte_index = 0;
    uint8_t current_byte = shadow_data[shadow_byte_index];
    int bit_index = 7; // Start from MSB

    for (size_t i = 0; i < cover_capacity && shadow_byte_index < shadow_len; ++i)
    {
        // Clear LSB of cover byte
        cover_data[i] &= 0xFE;

        // Insert current bit from shadow
        uint8_t bit = (current_byte >> bit_index) & 0x01;
        cover_data[i] |= bit;

        bit_index--;
        if (bit_index < 0)
        {
            bit_index = 7;
            shadow_byte_index++;
            if (shadow_byte_index < shadow_len)
            {
                current_byte = shadow_data[shadow_byte_index];
            }
        }
    }

    return true;
}

bool lsb_encoder_lsb1_into_cover_extended(const uint8_t *shadow_data, size_t shadow_len, BMPImageT *cover, uint16_t seed, int k, int16_t s_width, int16_t s_height)
{
    // 2 bytes for width and 2 bytes for height, 8 bits per byte

    if (!shadow_data || !cover || !cover->pixels || k < 1 || k > 10)
        return false;

    // Each byte in the shadow buffer needs 8 bits (LSBs in the cover) + 4 bytes for width and height
    size_t bits_needed = shadow_len * 8 + METADATA_SIZE;

    // padding is usable for the LSB
    uint32_t width_bytes = cover->width * cover->bpp / 8;
    uint32_t padded_width_bytes = bmp_align(width_bytes); // Align to 4 bytes
    size_t cover_capacity = padded_width_bytes * cover->height;

    if (cover_capacity < bits_needed)
    {
        fprintf(stderr, "Cover image too small to hide shadow data\n");
        return false;
    }

    uint8_t *cover_data = cover->pixels;

    size_t i = 0; // use the same index for metadata and shadow data
    for (int j = 0; j < 16; ++j, ++i)
    {
        uint8_t bit = (s_width >> (15 - j)) & 0x01;
        cover_data[i] = (cover_data[i] & 0xFE) | bit;
    }

    for (int j = 0; j < 16; ++j, ++i)
    {
        uint8_t bit = (s_height >> (15 - j)) & 0x01;
        cover_data[i] = (cover_data[i] & 0xFE) | bit;
    }

    size_t shadow_byte_index = 0;
    uint16_t current_word = shadow_data[shadow_byte_index];
    int bit_index = 7; // Start from MSB

    for (; i < cover_capacity && shadow_byte_index < shadow_len; ++i)
    {
        // Clear LSB of cover byte
        cover_data[i] &= 0xFE;

        // Insert current bit from shadow
        uint8_t bit = (current_word >> bit_index) & 0x01;
        cover_data[i] |= bit;

        bit_index--;
        if (bit_index < 0)
        {
            bit_index = 7;
            shadow_byte_index++;
            if (shadow_byte_index < shadow_len)
            {
                current_word = shadow_data[shadow_byte_index];
            }
        }
    }

    return true;
}

#undef METADATA_SIZE