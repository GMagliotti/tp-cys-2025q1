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

/**
 * @brief Get the address of a pixel in the image, considering BMP format spec.
 * @param image The image to get the pixel from.
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @return A pointer to the pixel data.
 * @warning For performance reasons, this function does not check if the coordinates are valid.
 *         It is the caller's responsibility to ensure that the coordinates are within the image bounds.
 */
static inline void *bmp_get_pixel(const BMPImageT *image, int32_t x, int32_t y);

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
 * @return 0 on success, -1 on failure.
 */
int bmp_save(const char *filename, const BmpImage *image);

/**
 * @brief Frees the memory allocated for a BMP image.
 * @param image A pointer to the BmpImage structure to free.
 */
void bmp_unload(BmpImage *image);

#endif