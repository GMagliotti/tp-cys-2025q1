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
};
#pragma pack(pop)

typedef struct BmpImageT BmpImage;

/**
 * @brief Loads a BMP image file to memory.
 * @param filename The name of the BMP file to open.
 * @return A pointer to a BmpImage structure containing the image data, or NULL on failure.
 */
BmpImage* bmp_load(const char *filename);

/**
 * @brief Frees the memory allocated for a BMP image.
 * @param image A pointer to the BmpImage structure to free.
 */
void bmp_unload(BmpImage* image);

#endif