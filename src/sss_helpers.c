#include "../include/sss_helpers.h"
#define METADATA_SIZE 32 // 2 bytes for width and 2 bytes for height * 8 bits per byte

static int ends_with_bmp(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    return dot && strcmp(dot, ".bmp") == 0;
}

bool sssh_can_hide_bits(const BMPImageT *cover, size_t bits_needed)
{
    if (!cover || !cover->pixels)
        return false;

    uint32_t width_bytes = cover->width * cover->bpp / 8;
    uint32_t padded_width_bytes = bmp_align(width_bytes); // Align to 4 bytes
    size_t cover_capacity = padded_width_bytes * cover->height;

    return cover_capacity >= bits_needed;
}

BMPImageT **load_bmp_images(
    const char *dir_path,
    uint32_t max_images,
    BMPFilterFunc filter,
    void *context)
{
    DIR *dir = opendir(dir_path);
    if (!dir)
    {
        perror("Error opening BMP image directory");
        return NULL;
    }

    BMPImageT **images = calloc(max_images, sizeof(BMPImageT *));
    if (!images)
    {
        perror("Error allocating memory for BMP images");
        closedir(dir);
        return NULL;
    }

    struct dirent *entry;
    uint32_t count = 0;

    while ((entry = readdir(dir)) != NULL && count < max_images)
    {
        if (entry->d_type == DT_REG && ends_with_bmp(entry->d_name))
        {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

            BMPImageT *bmp = bmp_load(full_path);
            if (!bmp)
            {
                fprintf(stderr, "Failed to load BMP image '%s'\n", full_path);
                continue;
            }

            if (filter && !filter(bmp, full_path, context))
            {
                bmp_unload(bmp);
                continue;
            }

            images[count++] = bmp;
        }
    }

    closedir(dir);

    if (count < max_images)
    {
        fprintf(stderr, "Only %d suitable .bmp files found in '%s' (need %d)\n", count, dir_path, max_images);
        for (uint32_t i = 0; i < count; i++)
        {
            bmp_unload(images[i]);
        }
        free(images);
        return NULL;
    }

    return images;
}

bool can_hide_bits_filter(const BMPImageT *bmp, const char *path, void *ctx)
{
    size_t bits_needed = *((size_t *)ctx);
    if (!sssh_can_hide_bits(bmp, bits_needed))
    {
        fprintf(stderr, "Cover image '%s' too small to hide required bits\n", path);
        return false;
    }
    return true;
}

BMPImageT **load_bmp_covers(const char *covers_dir, uint32_t n, size_t bits_needed)
{
    return load_bmp_images(covers_dir, n, can_hide_bits_filter, &bits_needed);
}

void free_bmp_covers(BMPImageT **images, uint32_t k)
{
    if (!images)
        return;

    for (uint32_t i = 0; i < k; i++)
    {
        if (images[i])
        {
            bmp_unload(images[i]);
        }
    }
    free(images);
}

#undef METADATA_SIZE

// cover
// shadow_data
// acceso_bit