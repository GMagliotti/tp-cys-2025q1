#include "bmp.h"

#pragma pack(push, 1)
typedef struct {
    uint8_t signature[2]; 
    uint32_t file_size;
    uint8_t reserved[4];
    uint32_t bof; // Beginning of bitmap data - offset
} BitmapFileHeader;

typedef struct {
    uint32_t dib_header_size; // The typical Win 3.x header size is 40 bytes
    int32_t width;
    int32_t height;
    uint16_t planes; // Must be 1
    uint16_t bpp; // Bits per pixel
    uint32_t compression; // Compression type - (0 = none, 1 = RLE8, 2 = RLE4)
    uint32_t image_size; // Size of the image data, may be 0 if uncompressed
    int32_t h_resolution; // Horizontal resolution in pixels per meter
    int32_t v_resolution; // Vertical resolution in pixels per meter
    uint32_t colors_used; // Number of colors in the color palette (0 = default)
    uint32_t important_colors; // Number of important colors (0 = all)
} BitmapInfoHeader;

typedef struct {
    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;
    BMPColorT* palette; // Color palette (if present)
    void* pixels; // Pointer to pixel data
} Win3xBmpImageData;
#pragma pack(pop)

bool is_readable_bmp(FILE* file) {
    BitmapFileHeader fheader;
    BitmapInfoHeader iheader;

    // Attempt to read file header
    fread(&fheader, sizeof(BitmapFileHeader), 1, file);
    if (ferror(file)) {
        perror("Error reading file header");
        rewind(file);
        return false;
    }

    // Check BMP signature
    if (fheader.signature[0] != 'B' || fheader.signature[1] != 'M') {
        fprintf(stderr, "Invalid BMP signature\n");
        rewind(file);
        return false;
    }

    // Attempt to read info header
    fread(&iheader, sizeof(BitmapInfoHeader), 1, file);
    if (ferror(file)) {
        perror("Error reading info header");
        rewind(file);
        return false;
    }

    // Check header size (Win3.x format)
    if (iheader.dib_header_size != sizeof(BitmapInfoHeader)) {
        fprintf(stderr, "Unsupported DIB header size: %u\n", iheader.compression);
        rewind(file);
        return false;
    }

    // Only support 8-bit images
    if (iheader.bpp != 8) {
        fprintf(stderr, "Unsupported bits per pixel: %u\n", iheader.bpp);
        rewind(file);
        return false;
    }

    // Only support uncompressed images
    if (iheader.compression != 0) {
        fprintf(stderr, "Unsupported compression type: %u\n", iheader.compression);
        rewind(file);
        return false;
    }

    // Only support bottom-up images
    if (iheader.height < 0) {
        fprintf(stderr, "Unsupported image orientation: Top-down\n");
        rewind(file);
        return false;
    }

    if (iheader.width <= 0 || abs(iheader.height) <= 0) {
        fprintf(stderr, "Invalid image dimensions: %d x %d\n", iheader.width, abs(iheader.height));
        rewind(file);
        return false;
    }

    // All checks passed
    rewind(file);
    return true;
}

BmpImage* bmp_load(const char* filename) {
    FILE* file = fopen(filename, "rb");

    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    BmpImage* image = malloc(sizeof(BmpImage));

    if (image == NULL) {
        perror("Error allocating memory");
        goto cleanup_file;
    }

    if (!is_readable_bmp(file)) {
        fprintf(stderr, "Invalid BMP file\n");
        goto cleanup_image;
    }

    BitmapFileHeader fheader;
    BitmapInfoHeader iheader;
    fread(&fheader, sizeof(BitmapFileHeader), 1, file);
    if (ferror(file)) {
        perror("Error reading file header");
        goto cleanup_image;
    }

    fread(&iheader, sizeof(BitmapInfoHeader), 1, file);
    if (ferror(file)) {
        perror("Error reading info header");
        goto cleanup_image;
    }

    uint32_t palette_entries = iheader.colors_used ? iheader.colors_used : (1 << iheader.bpp);
    uint32_t palette_offset = sizeof(BitmapFileHeader) + iheader.dib_header_size;
    uint32_t bytes_per_scanline = (iheader.bpp * iheader.width + 31) / 32 * 4;
    uint32_t image_size = bytes_per_scanline * abs(iheader.height);

    image->palette = calloc(palette_entries, sizeof(*image->palette));
    if (image->palette == NULL) {
        perror("Error allocating memory for palette data");
        goto cleanup_image;
    }

    image->pixels = malloc(image_size);
    if (image->pixels == NULL) {
        perror("Error allocating memory for pixel data");
        goto cleanup_palette;
    }
    image->bpp = iheader.bpp;
    image->width = iheader.width;
    image->height = abs(iheader.height);
    
    fseek(file, fheader.bof, SEEK_SET);
    fread(image->pixels, 1, image_size, file);
    if (ferror(file)) {
        perror("Error reading pixel data");
        goto cleanup_all;
    }

    if (palette_entries == 0) {
        fprintf(stderr, "No palette found in BMP file\n");
        goto cleanup_all;
    }

    fseek(file, palette_offset, SEEK_SET);
    fread(image->palette, sizeof(BMPColorT), palette_entries, file);
    if (ferror(file)) {
        perror("Error reading palette data");
        goto cleanup_all;
    }

    fclose(file);
    return image;

    cleanup_all:
        free(image->pixels);
        image->pixels = NULL;
    cleanup_palette:
        free(image->palette);
        image->palette = NULL;
    cleanup_image:
        free(image);
        image = NULL;
    cleanup_file:
        fclose(file);
        return NULL;
}

void bmp_unload(BmpImage* image) {
    if (image) {
        free(image->pixels);
        image->pixels = NULL;
        free(image->palette);
        image->palette = NULL;
        free(image);
    }
}

