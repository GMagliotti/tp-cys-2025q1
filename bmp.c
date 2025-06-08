#include "bmp.h"
#define CHECK_HEADER_RESERVED(a, b, c, d) (a == 0 && b == 0 && c == 0 && d == 0)

#pragma pack(push, 1)
typedef struct
{
    uint8_t signature[2];
    uint32_t file_size;
    uint8_t reserved[4];
    uint32_t bof; // Beginning of bitmap data - offset
} BitmapFileHeader;

typedef struct
{
    uint32_t dib_header_size; // The typical Win 3.x header size is 40 bytes
    int32_t width;
    int32_t height;
    uint16_t planes;           // Must be 1
    uint16_t bpp;              // Bits per pixel
    uint32_t compression;      // Compression type - (0 = none, 1 = RLE8, 2 = RLE4)
    uint32_t image_size;       // Size of the image data, may be 0 if uncompressed
    int32_t h_resolution;      // Horizontal resolution in pixels per meter
    int32_t v_resolution;      // Vertical resolution in pixels per meter
    uint32_t colors_used;      // Number of colors in the color palette (0 = default)
    uint32_t important_colors; // Number of important colors (0 = all)
} BitmapInfoHeader;

typedef struct
{
    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;
    BMPColorT *palette; // Color palette (if present)
    void *pixels;       // Pointer to pixel data
} Win3xBmpImageData;
#pragma pack(pop)

bool is_readable_bmp(FILE *file)
{
    BitmapFileHeader fheader;
    BitmapInfoHeader iheader;

    // Attempt to read file header
    fread(&fheader, sizeof(BitmapFileHeader), 1, file);
    if (ferror(file))
    {
        perror("Error reading file header");
        rewind(file);
        return false;
    }

    // Check BMP signature
    if (fheader.signature[0] != 'B' || fheader.signature[1] != 'M')
    {
        fprintf(stderr, "Invalid BMP signature\n");
        rewind(file);
        return false;
    }

    // Attempt to read info header
    fread(&iheader, sizeof(BitmapInfoHeader), 1, file);
    if (ferror(file))
    {
        perror("Error reading info header");
        rewind(file);
        return false;
    }

    // Check header size (Win3.x format)
    if (iheader.dib_header_size != sizeof(BitmapInfoHeader))
    {
        fprintf(stderr, "Unsupported DIB header size: %u\n", iheader.compression);
        rewind(file);
        return false;
    }

    // Only support 8-bit images
    if (iheader.bpp != 8)
    {
        fprintf(stderr, "Unsupported bits per pixel: %u\n", iheader.bpp);
        rewind(file);
        return false;
    }

    // Only support uncompressed images
    if (iheader.compression != 0)
    {
        fprintf(stderr, "Unsupported compression type: %u\n", iheader.compression);
        rewind(file);
        return false;
    }

    // Only support bottom-up images
    if (iheader.height < 0)
    {
        fprintf(stderr, "Unsupported image orientation: Top-down\n");
        rewind(file);
        return false;
    }

    if (iheader.width <= 0 || abs(iheader.height) <= 0)
    {
        fprintf(stderr, "Invalid image dimensions: %d x %d\n", iheader.width, abs(iheader.height));
        rewind(file);
        return false;
    }

    // All checks passed
    rewind(file);
    return true;
}

uint32_t calculate_file_size(const BmpImage *image)
{
    if (image == NULL)
    {
        return 0;
    }

    uint32_t bytes_per_pixel = image->bpp / 8;
    uint32_t bytes_per_scanline = bmp_align(image->width * bytes_per_pixel); // Align to 4-byte boundary
    uint32_t image_size = bytes_per_scanline * abs(image->height);
    uint32_t palette_size = image->colors_used * sizeof(BMPColorT);

    return sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + palette_size + image_size;
}

BMPImageT *bmp_copy(BMPImageT *image)
{
    if (image == NULL || image->bpp != 8 || image->palette == NULL || image->pixels == NULL)
    {
        fprintf(stderr, "bmp_copy: Invalid image or unsupported format\n");
        return NULL;
    }

    BmpImage *copy = malloc(sizeof(BmpImage));
    if (copy == NULL)
    {
        fprintf(stderr, "bmp_copy: Error allocating memory for new image\n");
        return NULL;
    }

    copy->width = image->width;
    copy->height = image->height;
    copy->bpp = image->bpp;

    uint32_t palette_size = (image->bpp <= 8) ? ((1 << image->bpp) * sizeof(BMPColorT)) : 0;
    copy->palette = malloc(palette_size);
    if (copy->palette == NULL)
    {
        fprintf(stderr, "bmp_copy: Error allocating memory for palette copy\n");
        free(copy);
        return NULL;
    }
    memcpy(copy->palette, image->palette, palette_size);

    uint32_t bytes_per_pixel = image->bpp / 8;
    // Each scanline must be aligned to a 4-byte boundary as per BMP format
    uint32_t bytes_per_scanline = (image->width * bytes_per_pixel + 3) & ~3; // Align to 4-byte boundary
    uint32_t image_size = bytes_per_scanline * abs(image->height);

    copy->pixels = malloc(image_size);
    if (copy->pixels == NULL)
    {
        fprintf(stderr, "bmp_copy: Error allocating memory for data copy\n");
        free(copy->palette);
        free(copy);
        return NULL;
    }
    memcpy(copy->pixels, image->pixels, image_size);

    return copy;
}

BMPColorT *bmp_copy_palette(BMPImageT *image)
{
    if (image == NULL || image->palette == NULL)
    {
        fprintf(stderr, "bmp_copy_palette: Invalid image or no palette\n");
        return NULL;
    }

    uint32_t palette_size = (image->bpp <= 8) ? ((1 << image->bpp) * sizeof(BMPColorT)) : 0;
    BMPColorT *palette_copy = malloc(palette_size);
    if (palette_copy == NULL)
    {
        fprintf(stderr, "bmp_copy_palette: Error allocating memory for palette copy\n");
        return NULL;
    }
    memcpy(palette_copy, image->palette, palette_size);
    return palette_copy;
}

BmpImage *bmp_create(int32_t width, int32_t height, uint8_t bpp, BMPColorT *palette, uint32_t pcolors)
{
    if (width <= 0 || height == 0)
    {
        fprintf(stderr, "bmp_create: Invalid image dimensions: %d x %d\n", width, height);
        return NULL;
    }

    if (height < 0)
    {
        fprintf(stderr, "bmp_create: Top-down BMP images are not yet supported\n");
        return NULL;
    }

    if (bpp != 8)
    {
        fprintf(stderr, "bmp_create: Unsupported bits per pixel: %u\n", bpp);
        return NULL;
    }

    BmpImage *image = malloc(sizeof(BmpImage));
    if (image == NULL)
    {
        fprintf(stderr, "bmp_create: Error allocating memory for new image\n");
        return NULL;
    }

    image->width = width;
    image->height = height;
    image->bpp = bpp;

    uint32_t new_pcolors = (1 << bpp);
    image->palette = calloc(new_pcolors, sizeof(BMPColorT));
    if (image->palette == NULL)
    {
        fprintf(stderr, "bmp_create: Error allocating memory for palette\n");
        goto error_clean_image;
    }

    bool can_copy_palette = pcolors > 0 && pcolors <= new_pcolors;
    if (!can_copy_palette)
    {
        fprintf(stderr, "bmp_create: Palette of  %u\n colors cannot be copied over.\n", pcolors);
        fprintf(stderr, "Defaulting to zeroed palette.\n");
    }
    else if (palette != NULL)
    {
        fprintf(stdout, "bmp_create: Copying palette data\n");
        memcpy(image->palette, palette, pcolors * sizeof(BMPColorT));
    }

    // 4-byte alignment for pixel scanline, required by BMP spec
    uint32_t bytes_per_pixel = bpp / 8;
    uint32_t bytes_per_scanline = (width * bytes_per_pixel + 3) & ~3;
    uint32_t image_size = bytes_per_scanline * abs(height); // height can be negative!

    image->pixels = calloc(image_size, 1);
    if (image->pixels == NULL)
    {
        fprintf(stderr, "bmp_create: Error allocating memory for pixel data\n");
        goto error_clean_palette;
    }

    return image;

error_clean_palette:
    free(image->palette);
    image->palette = NULL;
error_clean_image:
    free(image);
    image = NULL;
    return NULL;
}

BmpImage *bmp_load(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        perror("Error opening file");
        return NULL;
    }

    BmpImage *image = malloc(sizeof(BmpImage));

    if (image == NULL)
    {
        perror("Error allocating memory");
        goto cleanup_file;
    }

    if (!is_readable_bmp(file))
    {
        fprintf(stderr, "Invalid BMP file\n");
        goto cleanup_image;
    }

    BitmapFileHeader fheader;
    BitmapInfoHeader iheader;
    fread(&fheader, sizeof(BitmapFileHeader), 1, file);
    if (ferror(file))
    {
        perror("Error reading file header");
        goto cleanup_image;
    }

    fread(&iheader, sizeof(BitmapInfoHeader), 1, file);
    if (ferror(file))
    {
        perror("Error reading info header");
        goto cleanup_image;
    }

    uint32_t palette_entries = iheader.colors_used ? iheader.colors_used : (1 << iheader.bpp);
    image->colors_used = palette_entries;
    fprintf(stdout, "bmp_load: Colors used: %u\n", image->colors_used);
    uint32_t palette_offset = sizeof(BitmapFileHeader) + iheader.dib_header_size;
    uint32_t bytes_per_scanline = (iheader.bpp * iheader.width + 31) / 32 * 4;
    uint32_t image_size = bytes_per_scanline * abs(iheader.height);
    image->reserved = malloc(4);
    if (image->reserved == NULL)
    {
        goto cleanup_reserved;
    }
    image->palette = calloc(palette_entries, sizeof(*image->palette));
    if (image->palette == NULL)
    {
        perror("Error allocating memory for palette data");
        goto cleanup_image;
    }

    image->pixels = malloc(image_size);
    if (image->pixels == NULL)
    {
        perror("Error allocating memory for pixel data");
        goto cleanup_palette;
    }
    image->bpp = iheader.bpp;
    image->width = iheader.width;
    image->height = abs(iheader.height);

    fseek(file, 6, SEEK_SET);
    fread(image->reserved, 1, 4, file);
    if (ferror(file))
    {
        perror("Error reading reserved bytes in header");
        goto cleanup_all;
    }
    fseek(file, fheader.bof, SEEK_SET);
    fread(image->pixels, 1, image_size, file);
    if (ferror(file))
    {
        perror("Error reading pixel data");
        goto cleanup_all;
    }

    if (palette_entries == 0)
    {
        fprintf(stderr, "No palette found in BMP file\n");
        goto cleanup_all;
    }

    fseek(file, palette_offset, SEEK_SET);
    fread(image->palette, sizeof(BMPColorT), palette_entries, file);
    if (ferror(file))
    {
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
cleanup_reserved:
    free(image->reserved);
    image->reserved = NULL;
cleanup_image:
    free(image);
    image = NULL;
cleanup_file:
    fclose(file);
    return NULL;
}

int bmp_save(const char *filename, const BmpImage *image)
{
    fprintf(stdout, "bmp_save: Colors used %d\n", image->colors_used);
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        perror("Error opening file for writing");
        return -1;
    }

    const uint8_t *res = image->reserved;
    if (!CHECK_HEADER_RESERVED(res[0], res[1], res[2], res[3]))
        printf("Saving non-standard image with modified reserved bytes.\n");

    BitmapFileHeader fheader = {0};
    BitmapInfoHeader iheader = {0};

    fheader.signature[0] = 'B';
    fheader.signature[1] = 'M';
    fheader.file_size = calculate_file_size(image);
    fheader.bof = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + image->colors_used * sizeof(BMPColorT);
    fheader.reserved[0] = image->reserved[0];
    fheader.reserved[1] = image->reserved[1];
    fheader.reserved[2] = image->reserved[2];
    fheader.reserved[3] = image->reserved[3];
    iheader.dib_header_size = sizeof(BitmapInfoHeader);
    iheader.width = image->width;
    iheader.height = image->height;
    iheader.planes = 1;
    iheader.bpp = image->bpp;
    iheader.compression = 0;
    iheader.image_size = 0;                   // Set to 0 for uncompressed
    iheader.h_resolution = 0;                 // Set to 0 for default resolution
    iheader.v_resolution = 0;                 // Set to 0 for default resolution
    iheader.colors_used = image->colors_used; // Set to 0 when no palette is used
    iheader.important_colors = 0;             // Set to 0 for all colors
    fwrite(&fheader, sizeof(BitmapFileHeader), 1, file);
    if (ferror(file))
    {
        perror("Error writing file header");
        fclose(file);
        return -1;
    }

    fwrite(&iheader, sizeof(BitmapInfoHeader), 1, file);
    if (ferror(file))
    {
        perror("Error writing info header");
        fclose(file);
        return -1;
    }

    if (image->bpp <= 8)
    {
        // the palette is init with calloc
        fwrite(image->palette, sizeof(BMPColorT), image->colors_used, file);
        if (ferror(file))
        {
            perror("Error writing palette data");
            fclose(file);
            return -1;
        }
    }

    uint32_t pixel_data_size = calculate_file_size(image) - fheader.bof;
    fwrite(image->pixels, 1, pixel_data_size, file);
    if (ferror(file))
    {
        perror("Error writing pixel data");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

void bmp_unload(BmpImage *image)
{
    if (image)
    {
        free(image->reserved);
        image->reserved = NULL;
        free(image->pixels);
        image->pixels = NULL;
        free(image->palette);
        image->palette = NULL;
        free(image);
    }
}

#undef CHECK_HEADER_RESERVED