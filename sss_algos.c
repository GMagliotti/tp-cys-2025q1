#include "sss_algos.h"
#include <assert.h>

#define PRIME_MODULUS 257
#define MAX_K 10
#define MIN_K 2
#define MIN_N 2

bool hide_shadow_lsb_from_buffer(const uint8_t *shadow_data, size_t shadow_len, BMPImageT *cover, uint16_t seed)
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
    memset(coeffs, 0, sizeof(coeffs));
    int sections = total_pixels / k;
    // int remaining = total_pixels % k;

    // Allocate shadow_data once
    for (int i = 0; i < n; ++i)
    {
        shadow_data[i] = malloc(sections * sizeof(uint8_t));
        if (!shadow_data[i])
        {
            fprintf(stderr, "Out of memory allocating shadow_data[%d]\n", i);
            return false;
        }
    }

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
                    // TODO see if working
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

        // For each section:
        for (int i = 0; i < n; ++i)
        {
            uint16_t fx = poly_eval(coeffs, k, i + 1);
            assert(fx <= 255);

            int shadow_idx = section;
            int x = shadow_idx % shadows[i]->width;
            int y = shadow_idx / shadows[i]->width;
            uint8_t *px = (uint8_t *)bmp_get_pixel_address(shadows[i], x, y);
            *px = (uint8_t)fx;

            shadow_data[i][section] = (uint8_t)fx;
        }
    }

    return true;
}

bool sss_distribute_share_image_k(const BMPImageT *Q, BMPImageT **shadows, int k, int n, uint16_t **shadow_data)
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
    memset(coeffs, 0, sizeof(coeffs));
    int sections = (total_pixels + k - 1) / k;
    // int remaining = total_pixels % k;

    // Allocate shadow_data once
    for (int i = 0; i < n; ++i)
    {
        shadow_data[i] = malloc(sections * sizeof(uint16_t));
        if (!shadow_data[i])
        {
            fprintf(stderr, "Out of memory allocating shadow_data[%d]\n", i);
            return false;
        }
    }

    for (int section = 0; section < sections; ++section)
    {
        // Step 3: Extract r coefficients from Q (r consecutive pixels)
        for (int i = 0; i < k; ++i)
        {
            int index = section * k + i;
            coeffs[i] = (index < total_pixels)
                            ? *(uint8_t *)bmp_get_pixel_address(Q, index % Q->width, index / Q->width)
                            : 0; // pad with 0s if overflow
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
                    // TODO see if working
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

        // For each section:
        for (int i = 0; i < n; ++i)
        {
            uint16_t fx = poly_eval(coeffs, k, i + 1);
            assert(fx <= 255);

            int shadow_idx = section;
            int x = shadow_idx % shadows[i]->width;
            int y = shadow_idx / shadows[i]->width;
            uint8_t *px = (uint8_t *)bmp_get_pixel_address(shadows[i], x, y);
            *px = (uint8_t)fx;

            shadow_data[i][section] = (uint8_t)fx;
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

BMPImageT **sss_distribute_8(BMPImageT *image, uint32_t k, uint32_t n, const char *covers_dir, const char *output_dir)
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
        char cover_path[512];
        snprintf(cover_path, sizeof(cover_path), "%s/cover1.bmp", covers_dir);

        BMPImageT *cover = bmp_load(cover_path);
        if (!cover)
        {
            fprintf(stderr, "Failed to load cover image '%s' for shadow %d\n", cover_path, i);
            exit(EXIT_FAILURE);
        }

        int size = shadows[i]->width * shadows[i]->height / k;
        bool ok = sssh_8bit_lsb_into_cover(shadow_data[i], size, cover, seed);
        if (!ok)
        {
            fprintf(stderr, "Failed to hide shadow %d in cover image\n", i);
            exit(EXIT_FAILURE);
        }

        // Guardar la imagen stego
        uint16_t x = i + 1;
        cover->reserved[0] = seed & 0xFF;
        cover->reserved[1] = (seed >> 8) & 0xFF;
        cover->reserved[2] = x & 0xFF;
        cover->reserved[3] = (x >> 8) & 0xFF;

        char output_path[512];
        snprintf(output_path, sizeof(output_path), "%s/stego%d.bmp", output_dir, i + 1);
        bmp_save(output_path, cover);
        bmp_unload(cover);
    }

    return shadows;
}

BMPImageT **sss_distribute_generic(BMPImageT *image, uint32_t k, uint32_t n, const char *covers_dir, const char *output_dir)
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
    uint16_t *shadow_data[256] = {0};
    if (sss_distribute_share_image_k(image, shadows, k, n, shadow_data) == false)
    {
        fprintf(stderr, "Failed to distribute image\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++)
    {
        BMPImageT *cover = bmp_load("cover5.bmp"); // or generate it if it's fixed size
        if (!cover)
        {
            fprintf(stderr, "Failed to load cover image for shadow %d\n", i);
            exit(EXIT_FAILURE);
        }

        int sections = (image->width * image->height + k - 1) / k;
        bool ok = sssh_lsb1_into_cover_k(shadow_data[i], sections, cover, seed, k, image->width, image->height);
        if (!ok)
        {
            fprintf(stderr, "Failed to hide shadow %d in cover image\n", i);
            exit(EXIT_FAILURE);
        }

        fprintf(stderr, "Hiding shadow %d in cover image\n", i);
        uint16_t x = i + 1;
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

// Extended Euclidean Algorithm for inverse mod 257
uint16_t modinv(int a, int p)
{
    int t = 0, newt = 1;
    int r = p, newr = a;
    while (newr != 0)
    {
        int quotient = r / newr;
        int tmp = newt;
        newt = t - quotient * newt;
        t = tmp;
        tmp = newr;
        newr = r - quotient * newr;
        r = tmp;
    }
    if (r > 1)
        return 0; // Not invertible
    if (t < 0)
        t += p;
    return (uint16_t)t;
}

void lagrange_solve_coeffs(const uint8_t *y, const uint16_t *x, int k, uint8_t *out_coeffs)
{
    uint16_t **A = malloc(k * sizeof(uint16_t *));
    for (int i = 0; i < k; ++i)
    {
        A[i] = malloc((k + 1) * sizeof(uint16_t)); // last column for y
        uint16_t xi = 1;
        for (int j = 0; j < k; ++j)
        {
            A[i][j] = xi;
            xi = (xi * x[i]) % PRIME_MODULUS;
        }
        A[i][k] = y[i]; // append y as RHS
    }

    // Gaussian Elimination mod PRIME_MODULUS
    for (int col = 0; col < k; ++col)
    {
        // Find pivot
        int pivot = -1;
        for (int row = col; row < k; ++row)
        {
            if (A[row][col] != 0)
            {
                pivot = row;
                break;
            }
        }

        if (pivot == -1)
        {
            fprintf(stderr, "Singular matrix: pivot not found\n");
            exit(EXIT_FAILURE);
        }

        // Swap rows if needed
        if (pivot != col)
        {
            uint16_t *tmp = A[col];
            A[col] = A[pivot];
            A[pivot] = tmp;
        }

        // Normalize pivot row
        uint16_t inv = modinv(A[col][col], PRIME_MODULUS);
        for (int j = col; j <= k; ++j)
            A[col][j] = (A[col][j] * inv) % PRIME_MODULUS;

        // Eliminate below
        for (int row = col + 1; row < k; ++row)
        {
            uint16_t factor = A[row][col];
            for (int j = col; j <= k; ++j)
            {
                A[row][j] = (PRIME_MODULUS + A[row][j] - factor * A[col][j] % PRIME_MODULUS) % PRIME_MODULUS;
            }
        }
    }

    // Back-substitution
    for (int i = k - 1; i >= 0; --i)
    {
        uint16_t sum = A[i][k];
        for (int j = i + 1; j < k; ++j)
        {
            sum = (PRIME_MODULUS + sum - A[i][j] * out_coeffs[j] % PRIME_MODULUS) % PRIME_MODULUS;
        }
        out_coeffs[i] = sum;
    }

    for (int i = 0; i < k; ++i)
        free(A[i]);
    free(A);
}

uint8_t lagrange_reconstruct_pixel(uint8_t *y, uint16_t *x, int k)
{
    int sum = 0;

    for (int i = 0; i < k; i++)
    {
        int numerator = 1;
        int denominator = 1;

        for (int j = 0; j < k; j++)
        {
            if (j == i)
                continue;

            int xj = x[j];
            int xi = x[i];

            numerator = (numerator * (PRIME_MODULUS - xj)) % PRIME_MODULUS; // -xj mod 257
            int diff = (xi - xj + PRIME_MODULUS) % PRIME_MODULUS;
            denominator = (denominator * diff) % PRIME_MODULUS;
        }

        int li = (numerator * modinv(denominator, PRIME_MODULUS)) % PRIME_MODULUS;
        sum = (sum + li * y[i]) % PRIME_MODULUS;
    }

    return (uint8_t)sum;
}

BMPImageT *sss_recover_8(BMPImageT **shadows, uint32_t k, const char *recovered_filename)
{
    if (k < MIN_K || k > MAX_K)
    {
        fprintf(stderr, "Invalid parameters: k must be between 2 and 10\n");
        return NULL;
    }

    uint16_t seed = shadows[0]->reserved[0] | (shadows[0]->reserved[1] << 8);
    // bool flag = true, set = false,
    bool error = false;
    // int i = 0;

    if (error)
    {
        fprintf(stderr, "Error: shadows are not valid\n");
        return NULL;
    }

    uint8_t **shadow_array = malloc(k * sizeof(uint8_t *));
    uint16_t *x_array = malloc(k * sizeof(uint16_t));
    int shadow_len = shadows[0]->width * shadows[0]->height / k;
    for (int i = 0; i < k; i++)
    {
        shadow_array[i] = malloc(shadow_len);
        extract_shadow_lsb_to_buffer(shadow_array[i], shadow_len, shadows[i]);
        x_array[i] = shadows[i]->reserved[2] | (shadows[i]->reserved[3] << 8);
    }

    // modularize
    BMPImageT *recovered_image = malloc(sizeof(BMPImageT));
    recovered_image->palette = shadows[0]->palette;
    recovered_image->width = shadows[0]->width;
    recovered_image->height = shadows[0]->height;
    recovered_image->bpp = shadows[0]->bpp;
    recovered_image->colors_used = shadows[0]->colors_used;
    recovered_image->reserved = malloc(4);
    recovered_image->reserved[0] = 0;
    recovered_image->reserved[1] = 0;
    recovered_image->reserved[2] = 0;
    recovered_image->reserved[3] = 0;
    // int image_size = recovered_image->width * recovered_image->height;

    int width_bytes = recovered_image->width * recovered_image->bpp / 8;
    int padded_row_bytes = (width_bytes % 4 == 0) ? width_bytes : (width_bytes + (4 - (width_bytes % 4)));
    int padded_image_size = padded_row_bytes * recovered_image->height;
    recovered_image->pixels = malloc(padded_image_size);

    for (int section = 0; section < shadow_len; ++section)
    {
        uint8_t y_vals[MAX_K];
        for (int j = 0; j < k; ++j)
            y_vals[j] = shadow_array[j][section];

        uint8_t recovered_coeffs[MAX_K];
        lagrange_solve_coeffs(y_vals, x_array, k, recovered_coeffs);

        for (int i = 0; i < k; ++i)
        {
            int pixel_index = section * k + i;
            uint8_t *px = bmp_get_pixel_address(recovered_image, pixel_index % recovered_image->width, pixel_index / recovered_image->width);
            *px = recovered_coeffs[i];
        }
    }

    uint8_t *rng_table = ptable_get_rng_table_4bytealigned(recovered_image->width, recovered_image->height, seed);
    if (!rng_table)
    {
        fprintf(stderr, "Failed to generate RNG table\n");
        return NULL;
    }

    uint8_t *ptr = recovered_image->pixels;
    for (int i = 0; i < padded_image_size; ++i)
    {
        ptr[i] ^= rng_table[i];
    }

    bmp_save(recovered_filename, recovered_image);
    return recovered_image;
}

BMPImageT *sss_recover_generic(BMPImageT **shadows, uint32_t k, const char *recovered_filename)
{

    if (k < MIN_K || k > MAX_K)
    {
        fprintf(stderr, "Invalid parameters: k must be between 2 and 10\n");
        return NULL;
    }

    uint16_t seed = shadows[0]->reserved[0] | (shadows[0]->reserved[1] << 8);
    // bool flag = true, set = false,
    bool error = false;
    // int i = 0;

    if (error)
    {
        fprintf(stderr, "Error: shadows are not valid\n");
        return NULL;
    }

    uint16_t **shadow_array = malloc(k * sizeof(uint16_t *));
    uint16_t *x_array = malloc(k * sizeof(uint16_t));
    LSBDecodeResultT res[k];
    LSBDecodeResultT tmp = sssh_extract_kshadow_dimensions(shadows[0]);
    int total_pixels = tmp.s_height * tmp.s_width;
    int shadow_len = (total_pixels + k - 1) / k;
    for (int i = 0; i < k; i++)
    {
        shadow_array[i] = malloc(shadow_len * sizeof(uint16_t));
        res[i] = sssh_extract_lsb1_kshadow(shadow_array[i], shadow_len, shadows[i], k);
        x_array[i] = shadows[i]->reserved[2] | (shadows[i]->reserved[3] << 8);
    }

    // modularize
    BMPImageT *recovered_image = malloc(sizeof(BMPImageT));
    recovered_image->palette = shadows[0]->palette;
    recovered_image->width = res[0].s_width;
    recovered_image->height = res[0].s_height;
    recovered_image->bpp = shadows[0]->bpp;
    recovered_image->colors_used = shadows[0]->colors_used;
    recovered_image->reserved = malloc(4);
    recovered_image->reserved[0] = 0;
    recovered_image->reserved[1] = 0;
    recovered_image->reserved[2] = 0;
    recovered_image->reserved[3] = 0;

    // int image_size = recovered_image->width * recovered_image->height;

    int width_bytes = recovered_image->width * recovered_image->bpp / 8;
    int padded_row_bytes = bmp_align(width_bytes);
    int padded_image_size = padded_row_bytes * recovered_image->height;
    recovered_image->pixels = malloc(padded_image_size);

    for (int section = 0; section < shadow_len; ++section)
    {
        uint8_t y_vals[MAX_K];
        for (int j = 0; j < k; ++j)
            y_vals[j] = shadow_array[j][section];

        uint8_t recovered_coeffs[MAX_K];
        lagrange_solve_coeffs(y_vals, x_array, k, recovered_coeffs);

        for (int i = 0; i < k; ++i)
        {
            int pixel_index = section * k + i;
            if (pixel_index >= total_pixels)
                break;

            uint8_t *px = bmp_get_pixel_address(recovered_image, pixel_index % recovered_image->width, pixel_index / recovered_image->width);
            *px = recovered_coeffs[i];
        }
    }

    uint8_t *rng_table = ptable_get_rng_table_4bytealigned(recovered_image->width, recovered_image->height, seed);
    if (!rng_table)
    {
        fprintf(stderr, "Failed to generate RNG table\n");
        return NULL;
    }

    uint8_t *ptr = recovered_image->pixels;
    for (int i = 0; i < padded_image_size; ++i)
    {
        ptr[i] ^= rng_table[i];
    }

    fprintf(stdout, "Recovered image size: %d bytes\n", padded_image_size);
    fprintf(stdout, "Recovered image width: %d pixels\n", recovered_image->width);
    fprintf(stdout, "Recovered image height: %d pixels\n", recovered_image->height);
    fprintf(stdout, "Recovered image bpp: %d bits\n", recovered_image->bpp);
    fprintf(stdout, "Recovered image colors used: %d\n", recovered_image->colors_used);
    fprintf(stdout, "Recovered image reserved: %d %d %d %d\n", recovered_image->reserved[0], recovered_image->reserved[1], recovered_image->reserved[2], recovered_image->reserved[3]);
    fprintf(stdout, "Recovered image palette: %p\n", recovered_image->palette);
    fprintf(stdout, "Recovered image pixels: %p\n", recovered_image->pixels);
    bmp_save("recovered.bmp", recovered_image);
    return recovered_image;
}