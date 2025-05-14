#include "sss_algos.h"

/**
 * @brief Applies an in-place XOR operation between the image pixel data and a pseudo-random table.
 *
 * This function performs a bitwise XOR between each byte of the input BMP image's pixel data
 * and a randomly generated table of the same size. The result is stored back into the original
 * image's pixel buffer, effectively scrambling the image in-place for use in secret sharing.
 *
 * @param image Pointer to a BMPImageT structure containing the image to be processed.
 *              The image is modified in-place.
 * @param store If not NULL, this address will store the address where the random table was created.
 *              If NULL, the random table will be discarded after use.
 *
 * @return A pointer to the same BMPImageT structure, with the pixels XORed with the random table.
 *
 * @note The random table is generated with 4-byte alignment and must match the padded size of the image data.
 *       If a store pointer is provided, the caller is responsible for freeing the memory allocated for the random table.
 *
 * @warning This function will exit the program with an error message if memory allocation fails.
 */
BMPImageT *sss_distribute_initial_xor_inplace(BMPImageT *image, uint8_t **store)
{
    uint8_t *randtable = ptable_get_rng_table_4bytealigned(image->width, image->height);
    if (randtable == NULL || image == NULL)
    {
        fprintf(stderr, "Out of memory: Failed to distribute image\n");
        exit(EXIT_FAILURE);
    }

    uint32_t width_bytes = image->width * image->bpp / 8;
    uint32_t padded_width_bytes = (width_bytes % 4 == 0) ? width_bytes : (width_bytes + (4 - (width_bytes % 4)));
    unsigned int padding = (4 - (width_bytes % 4)) % 4;
    uint8_t *secret = image->pixels;
    for (uint32_t j = 0; j < image->height; j++)
    {
        for (uint32_t i = 0; i < padded_width_bytes; i++)
        {
            unsigned int offset = (j * (width_bytes + padding) + i);
            secret[offset] ^= randtable[offset];
        }
    }

    if (store != NULL)
    {
        *store = randtable;
    }
    else
    {
        free(randtable);
    }

    return image;
}

/**
 * @brief Allocates and initializes shadow image buffers for secret image sharing.
 *
 * @param image Pointer to the source BMPImageT structure to copy attributes from (e.g., width, height, bpp, palette).
 * @param k The threshold number of shares required to reconstruct the image (unused here but typically part of the context).
 * @param n The total number of shadow images to generate.
 *
 * @return A dynamically allocated array of BMPImageT* pointers. Each shadow image contains its own pixel buffer.
 *
 * @note The caller is responsible for freeing both the array of pointers and the memory associated with each individual BMPImageT,
 *       including their pixel buffers.
 *
 * @warning This function exits the program with an error message if any memory allocation fails.
 */
BMPImageT **sss_distribute_generate_shadows_buffers(BMPImageT *image, uint32_t k, uint32_t n)
{
    BMPImageT **shadows = calloc(n, sizeof(BMPImageT *)); // calloc in case of partial failure
    if (shadows == NULL)
    {
        fprintf(stderr, "Out of memory: Failed to allocate shadows holder\n");
        goto error_oom;
    }

    for (uint32_t i = 0; i < n; i++)
    {
        shadows[i] = calloc(1, sizeof(BMPImageT));
        if (shadows[i] == NULL)
        {
            fprintf(stderr, "Out of memory: Failed to allocate shadow image\n");
            goto error_oom;
        }
    }

    return shadows;

error_oom:
    for (uint32_t i = 0; i < n; i++)
    {
        if (shadows[i])
        {
            free(shadows[i]);
        }
    }
    free(shadows);

    exit(EXIT_FAILURE);
}

BMPImageT **sss_distribute_8(BMPImageT *image, uint32_t k, uint32_t n)
{
    sss_distribute_initial_xor_inplace(image, NULL);
    BMPImageT **shadows = sss_distribute_generate_shadows_buffer(image, k, n);
}

BMPImageT **sss_distribute_generic(BMPImageT *image, uint32_t k, uint32_t n)
{
    fprintf(stderr, "sss_distribute_generic not implemented\n");
    return NULL;
}

BMPImageT *sss_recover_8(BMPImageT **shadows, uint32_t k)
{
    fprintf(stderr, "sss_recover_8 not implemented\n");
    return NULL;
}

BMPImageT *sss_recover_generic(BMPImageT **shadows, uint32_t k)
{
    fprintf(stderr, "sss_recover_generic not implemented\n");
    return NULL;
}