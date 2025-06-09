#ifndef _SSS_H
#define _SSS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "permutation_table.h"
#include "bmp.h"

/**
 * @brief Distributes a BMP image into multiple shadow images using a (k, n) threshold scheme.
 *
 * This function splits the input BMP image into `n` shadow images such that any `k` of them
 * can be used to reconstruct the original. The shadows are typically smaller than the original
 * and contain encoded polynomial values derived from the image pixels. The resulting images
 * are saved in the specified output directory.
 *
 * @param image Pointer to the input BMPImageT representing the secret image.
 * @param k Minimum number of shadows required to reconstruct the image (threshold).
 * @param n Total number of shadow images to generate.
 * @param covers_dir Directory of path where the cover images are saved.
 * @param output_dir Directory path where the resulting shadow images will be saved.
 *
 * @return Pointer to an array of `n` BMPImageT* shadow images. Returns NULL on failure.
 *
 * @note The caller is responsible for freeing the returned BMP images. Use the `bmp_unload` function
 *       provided in bmp.h to do so.
 */
BMPImageT **sss_distribute(
    BMPImageT *image,
    uint32_t k,
    uint32_t n,
    const char *covers_dir,
    const char *output_dir
);


/**
 * @brief Recovers the original BMP image from a set of shadow images.
 *
 * This function reconstructs the original image using any `k` shadow images
 * generated from a previous call to `sss_distribute`. If fewer than `k` valid shadows
 * are provided, the function may return corrupted or incomplete data.
 *
 * @param shadows Array of at least `k` pointers to BMPImageT shadow images.
 * @param k The minimum number of shadows required to reconstruct the image.
 * @param recovered_filename Pointer to the filename where the recovered secret will be saved.
 *
 * @return Pointer to the recovered BMPImageT image, or NULL on failure or invalid input.
 *
 * @note The caller is responsible for freeing the returned BMP images. Use the `bmp_unload` function
 *       provided in bmp.h to do so.
 */
BMPImageT *sss_recover(BMPImageT **shadows, uint32_t k, const char * recovered_filename);
#endif