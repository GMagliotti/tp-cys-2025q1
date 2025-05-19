#include "sss_algos.h"
#include <assert.h>

#define PRIME_MODULUS 257
#define MAX_K 10
#define MIN_K 2
#define MIN_N 2

bool hide_shadow_lsb_from_buffer(const uint8_t *shadow_data, size_t shadow_len, BMPImageT *cover, uint16_t seed, int shadow_index)
{
    if (!shadow_data || !cover || !cover->pixels)
        return false;

    // Each byte in the shadow buffer needs 8 bits (LSBs in the cover)
    size_t bits_needed = shadow_len * 8;

    uint32_t width_bytes = cover->width * cover->bpp / 8;
    uint32_t padded_width_bytes = (width_bytes % 4 == 0) ? width_bytes : (width_bytes + (4 - (width_bytes % 4)));
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

// Polynomial evaluation (mod 257)
static inline uint16_t poly_eval(const uint8_t *coeffs, int r, uint16_t x)
{
    uint16_t result = 0;
    uint16_t power = 1;
    for (int i = 0; i < r; ++i)
    {
        result = (result + coeffs[i] * power) % PRIME_MODULUS;
        power = (power * x) % PRIME_MODULUS;
    }
    return result;
}

bool sss_distribute_share_image(const BMPImageT *Q, BMPImageT **shadows, int k, int n, uint8_t **shadow_data)
{
    if (!Q || !Q->pixels || !shadows)
    {
        fprintf(stderr, "Invalid input: Q or shadows is NULL\n");
        return false;
    }

    if (k < MIN_K || k > MAX_K)
    {
        fprintf(stderr, "Invalid parameters: k must be between 2 and 10\n");
        return false;
    }

    if (n < MIN_N || n < k)
    {
        fprintf(stderr, "Invalid parameters: n must be greater than 1, and k must be smaller than n\n");
        return false;
    }

    uint32_t total_pixels = Q->width * Q->height;
    uint8_t coeffs[MAX_K] = {0};
    int sections = total_pixels / k;
    int remaining = total_pixels % k;

    for (int section = 0; section < sections; ++section)
    {
        // Step 3: Extract r coefficients from Q (r consecutive pixels)
        for (int i = 0; i < k; ++i)
        {
            int index = section * k + i;
            int x = index % Q->width;
            int y = index / Q->width;
            coeffs[i] = *(uint8_t *)bmp_get_pixel_address(Q, x, y);
        }

        // Step 5: Retry if any fj(x) == 256
        bool valid;
        do
        {
            valid = true;
            for (int i = 0; i < n; ++i)
            {
                uint16_t fx = poly_eval(coeffs, k, i + 1);
                if (fx == 256)
                {
                    // decrease first non-zero coeff
                    for (int j = 0; j < k; ++j)
                    {
                        if (coeffs[j] != 0)
                        {
                            coeffs[j] = (coeffs[j] - 1) % 256;
                            break;
                        }
                    }
                    valid = false;
                    break;
                }
            }
        } while (!valid);

        // Step 4/6: Assign one pixel to each shadow image
        for (int i = 0; i < n; ++i)
        {
            uint16_t fx = poly_eval(coeffs, k, i + 1);
            assert(fx <= 255); // BMP can't store 256, ensure fix worked

            int shadow_index = section; // each section becomes 1 pixel in shadow image
            int x = shadow_index % shadows[i]->width;
            int y = shadow_index / shadows[i]->width;
            shadow_data[i] = malloc(sections * sizeof(uint8_t));
            uint8_t *px = (uint8_t *)bmp_get_pixel_address(shadows[i], x, y);
            shadow_data[i][section] = (uint8_t)fx;
            *px = (uint8_t)fx;
        }
    }

    return true;
}

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
 * @param seed The seed value used to generate the random table. This can be used for reproducibility.
 *
 * @return A pointer to the same BMPImageT structure, with the pixels XORed with the random table.
 *
 * @note The random table is generated with 4-byte alignment and must match the padded size of the image data.
 *       If a store pointer is provided, the caller is responsible for freeing the memory allocated for the random table.
 *
 * @warning This function will exit the program with an error message if memory allocation fails.
 */
BMPImageT *sss_distribute_initial_xor_inplace(BMPImageT *image, uint8_t **store, uint16_t seed)
{
    uint8_t *randtable = ptable_get_rng_table_4bytealigned(image->width, image->height, seed);
    if (randtable == NULL || image == NULL)
    {
        fprintf(stderr, "Out of memory: Failed to distribute image\n");
        exit(EXIT_FAILURE);
    }
    if (store != NULL)
    {
        *store = randtable;
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

        // uncomment when palette copies are implemented
        // shadows[i]->palette = calloc((1 << image->bpp), sizeof(BMPColorT));
        size_t row_size = (image->width * image->bpp + 7) / 8;
        size_t padded_row_size = (row_size + 3) & ~3; // Align to 4 bytes
        shadows[i]->pixels = calloc(padded_row_size * image->height, 1);
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
    uint16_t seed = rand() % 65536;
    sss_distribute_initial_xor_inplace(image, NULL, seed);
    BMPImageT **shadows = sss_distribute_generate_shadows_buffers(image, k, n);
    for (int i = 0; i < n; i++)
    {
        shadows[i]->bpp = image->bpp;
        shadows[i]->width = image->width;
        shadows[i]->height = image->height;
        shadows[i]->palette = image->palette; // TODO double free will be fixed later
    }
    uint8_t *shadow_data[256] = {0};
    if (sss_distribute_share_image(image, shadows, k, n, shadow_data) == false)
    {
        fprintf(stderr, "Failed to distribute image\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++)
    {
        BMPImageT *cover = bmp_load("cover.bmp"); // or generate it if it's fixed size
        if (!cover)
        {
            fprintf(stderr, "Failed to load cover image for shadow %d\n", i);
            exit(EXIT_FAILURE);
        }

        int size = shadows[i]->width * shadows[i]->height / k;
        bool ok = hide_shadow_lsb_from_buffer(shadow_data[i], size, cover, seed, i);
        if (!ok)
        {
            fprintf(stderr, "Failed to hide shadow %d in cover image\n", i);
            exit(EXIT_FAILURE);
        }

        uint16_t x = i+1;
        cover->reserved[0] = seed & 0xFF;
        cover->reserved[1] = (seed >> 8) & 0xFF;
        cover->reserved[2] = x & 0xFF;
        cover->reserved[3] = (x >> 8) & 0xFF;
        char filename[256];
        snprintf(filename, sizeof(filename), "stego%d.bmp", i);
        bmp_save(filename, cover);
        bmp_unload(cover);
    }
    return shadows;
}

BMPImageT **sss_distribute_generic(BMPImageT *image, uint32_t k, uint32_t n)
{
    fprintf(stderr, "sss_distribute_generic not implemented\n");
    return NULL;
}

// Extended Euclidean Algorithm for inverse mod 257
uint16_t modinv(int a, int p) {
    int t = 0, newt = 1;
    int r = p, newr = a;
    while (newr != 0) {
        int quotient = r / newr;
        int tmp = newt;
        newt = t - quotient * newt;
        t = tmp;
        tmp = newr;
        newr = r - quotient * newr;
        r = tmp;
    }
    if (r > 1) return 0;  // Not invertible
    if (t < 0) t += p;
    return (uint16_t)t;
}

uint8_t lagrange_reconstruct_pixel(uint8_t *y, uint16_t *x, int k) {
    int sum = 0;

    for (int i = 0; i < k; i++) {
        int numerator = 1;
        int denominator = 1;

        for (int j = 0; j < k; j++) {
            if (j == i) continue;

            int xj = x[j];
            int xi = x[i];

            numerator = (numerator * (PRIME_MODULUS - xj)) % PRIME_MODULUS;      // -xj mod 257
            int diff = (xi - xj + PRIME_MODULUS) % PRIME_MODULUS;
            denominator = (denominator * diff) % PRIME_MODULUS;
        }

        int li = (numerator * modinv(denominator, PRIME_MODULUS)) % PRIME_MODULUS;
        sum = (sum + li * y[i]) % PRIME_MODULUS;
    }

    return (uint8_t)sum;
}


bool extract_shadow_lsb_to_buffer(uint8_t *out_shadow_data, size_t shadow_len, const BMPImageT *cover)
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

BMPImageT *sss_recover_8(BMPImageT **shadows, uint32_t k)
{
    if (k < MIN_K || k > MAX_K)
    {
        fprintf(stderr, "Invalid parameters: k must be between 2 and 10\n");
        return NULL;
    }

    uint16_t seed = 0;
    bool flag = true, set = false, error = false;
    int i = 0;
    while (flag)
    {
        if (shadows[i] != NULL)
        {
            i++;
        }

        if (!set)
        {
            seed = shadows[i]->reserved[0] | (shadows[i]->reserved[1] << 8);
            set = true;
        }
        else if (seed != (shadows[i]->reserved[0] | (shadows[i]->reserved[1] << 8) ) )
        {
            fprintf(stderr, "Error: shadows have different seeds\n");
            error = true;
            break;
        }
    }

    if (error)
    {
        fprintf(stderr, "Error: shadows are not valid\n");
        return NULL;
    }
    if (i < k)
    {
        fprintf(stderr, "Not enough shadows to recover the image\n");
        return NULL;
    }

    uint8_t **shadow_array = malloc(k * sizeof(uint8_t *));
    uint16_t *x_array = malloc(k * sizeof(uint16_t));
    for (int i = 0; shadows[i] != NULL; i++) {
        int shadow_len = shadows[0]->width * shadows[0]->height;
        shadow_array[i] = malloc(shadow_len);
        extract_shadow_lsb_to_buffer(shadow_array[i], shadow_len, shadows[i]);
        x_array[i] = shadows[i]->reserved[2] | (shadows[i]->reserved[3] << 8);
    }
}

BMPImageT *sss_recover_generic(BMPImageT **shadows, uint32_t k)
{
    fprintf(stderr, "sss_recover_generic not implemented\n");
    return NULL;
}