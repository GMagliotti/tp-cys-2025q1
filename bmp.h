// #ifndef _BMP_IMAGE_H_
// #define _BMP_IMAGE_H_

// #include <stdint.h>

// /**
//  * @brief Opens a BMP image file and loads its data into a BmpImageT structure.
//  * @param filename The name of the BMP file to open.
//  * @return A pointer to a BmpImageT structure containing the image data, or NULL on failure.
//  */
// BmpImageT * bmp_open(const char *filename);

// uint32_t bmp_get_width(BmpImageT image);
// uint32_t bmp_get_height(BmpImageT image);
// uint16_t bmp_get_bpp(BmpImageT image);
// uint32_t bmp_get_size(BmpImageT image);

// /**
//  * 
//  */
// void bmp_close(BmpImageT *image);

// #endif

#ifndef _BMP_H
#define _BMP_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} BMPColorT;

typedef struct BmpImageT {
    int32_t width;
    int32_t height;
    uint16_t bpp;
    BMPColorT* palette;
    void* pixels;
    uint8_t *reserved; // 4
} BMPImageT;
#pragma pack(pop)

typedef struct BmpImageT BmpImage;

/**
 * @brief Creates a deep copy of an 8-bit BMP image.
 *
 * @param image Pointer to the source BmpImage to copy. Must be non-NULL, 8 bpp, and have valid
 *              palette and pixel buffers.
 *
 * @return A pointer to the newly allocated BmpImage copy, or NULL if input is invalid or
 *         memory allocation fails. This BmpImage meets the BMP format specifications.
 *
 * @note This function only supports 8-bit paletted BMP images. Other formats are rejected.
 *       The caller is responsible for freeing the returned BmpImage and its associated buffers.
 *
 * @warning On allocation failure, NULL is returned and an error message is printed to stderr.
 */
BMPImageT *bmp_copy(BMPImageT *image);

/** Create an empty bmp image with all necessary memory allocations. 
 * 
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param bpp The color depth of the image in bits per pixel
 * @param palette The color palette to use for the image. 
 *          If NULL, the pallette will be intialized with zeros.
 *          If bpp is greater than 8, this parameter is ignored.
 * @param pcolors The expected number of colors in the pallette.
 *         If bpp is greater than 8, this parameter is ignored.
 * @return A pointer to the newly created BmpImage structure, or NULL on failure.
 * @note The caller is responsible for freeing the returned BmpImage and its associated buffers.
 *       It is recommended to use bmp_unload() for this purpose.
*/
BMPImageT *bmp_create(uint32_t width, uint32_t height, uint8_t bpp, BMPColorT *palette, uint32_t pcolors);

/**
 * @brief Get the address of a pixel in the image, considering BMP format spec.
 * @param image The image to get the pixel from.
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @return A pointer to the pixel data.
 * @warning For performance reasons, this function does not check if the coordinates are valid.
 *         It is the caller's responsibility to ensure that the coordinates are within the image bounds.
 */
static inline void *bmp_get_pixel_address(const BMPImageT *image, int32_t x, int32_t y)
{
    int padw_b = ((image->width)*(image->bpp)/8 + 3) & ~3;
    return ((uint8_t *)image->pixels) + (y * padw_b) + x * (image->bpp / 8);
}

/**
 * @brief Loads a BMP image file to memory.
 * @param filename The name of the BMP file to open.
 * @return A pointer to a BmpImage structure containing the image data, or NULL on failure.
 */
BmpImage *bmp_load(const char *filename);

/**
 * @brief Save a BMP image to a file.
 * @param filename The name of the file to save the BMP image to.
 * @param image A pointer to the BmpImage structure containing the image data. Must be constant.
 * @param secret A pointer to a 4 byte buffer to -- the reserved bytes.
 *               If null, the reserved bytes are set to to the standard.
 *               This is non standard, and might cause issues with some BMP readers.
 * @return 0 on success, -1 on failure.
 */
int bmp_save(const char *filename, const BmpImage *image);

/**
 * @brief Frees the memory allocated for a BMP image.
 * @param image A pointer to the BmpImage structure to free.
 */
void bmp_unload(BmpImage *image);

#endif