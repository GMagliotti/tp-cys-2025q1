#include "lsb_decoder.h"
#define METADATA_SIZE 32 // 2 bytes for width and 2 bytes for height * 8 bits per byte

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

LSBDecodeResult lsb_decoder_lsb1_extract_to_buffer_extended(uint8_t *out_shadow_data, size_t shadow_len, const BMPImageT *cover, int k)
{
    if (!out_shadow_data || !cover || !cover->pixels || k < 2 || k > 10)
        return (LSBDecodeResult){.result = false, .s_width = 0, .s_height = 0};

    size_t bits_needed = shadow_len * 8 + METADATA_SIZE;
    fprintf(stdout, "bits_needed: %zu\n", bits_needed);

    uint32_t width_bytes = cover->width * cover->bpp / 8;
    uint32_t padded_width_bytes = bmp_align(width_bytes); // Align to 4 bytes
    size_t cover_capacity = padded_width_bytes * cover->height;
    fprintf(stdout, "cover_capacity: %zu\n", cover_capacity);

    if (cover_capacity < bits_needed)
    {
        fprintf(stderr, "Cover image too small to extract shadow data\n");
        return (LSBDecodeResult){.result = false, .s_width = 0, .s_height = 0};
    }

    const uint8_t *cover_data = cover->pixels;

    uint16_t s_width = 0, s_height = 0;
    for (int i = 0; i < 16; ++i)
    {
        s_width <<= 1;
        s_width |= (cover_data[i] & 0x01);
    }
    for (int i = 16; i < 32; ++i)
    {
        s_height <<= 1;
        s_height |= (cover_data[i] & 0x01);
    }
    size_t shadow_byte_index = 0;
    uint16_t current_word = 0;
    int bit_index = 7; // Start from MSB

    for (size_t i = METADATA_SIZE; i < cover_capacity && shadow_byte_index < shadow_len; ++i)
    {
        uint8_t bit = cover_data[i] & 0x01;
        current_word |= (bit << bit_index);

        bit_index--;
        if (bit_index < 0)
        {
            out_shadow_data[shadow_byte_index] = current_word;
            current_word = 0;
            bit_index = 7;
            shadow_byte_index++;
        }
    }

    return (LSBDecodeResult){.result = true, .s_width = s_width, .s_height = s_height};
}

LSBDecodeResult lsb_decoder_lsb1_get_dimensions(const BMPImageT *cover)
{
    if (!cover || !cover->pixels)
        return (LSBDecodeResult){.result = false, .s_width = 0, .s_height = 0};

    uint16_t s_width = 0, s_height = 0;
    const uint8_t *cover_data = cover->pixels;

    for (int i = 0; i < 16; ++i)
    {
        s_width <<= 1;
        s_width |= (cover_data[i] & 0x01);
    }
    for (int i = 16; i < 32; ++i)
    {
        s_height <<= 1;
        s_height |= (cover_data[i] & 0x01);
    }

    return (LSBDecodeResult){.result = true, .s_width = s_width, .s_height = s_height};
}

#undef METADATA_SIZE