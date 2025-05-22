#include "sss.h"

int main(int argc, char const *argv[])
{
    BmpImage* image = bmp_load("./input.bmp");
    if (image == NULL) {
        fprintf(stderr, "Failed to load BMP image\n");
        return 1;
    }
    printf("Width: %d\n", image->width);
    printf("Height: %d\n", image->height);
    printf("BPP: %d\n", image->bpp);
    printf("Successfully load the whole thing!\n");

    bmp_save("copy.bmp", image);
    sss_distribute(image, 8, 10);
    
    BMPImageT *shadows[11] = {0};
    for (int i = 0; i < 10; i++)
    {
        char filename[256];
        snprintf(filename, sizeof(filename), "stego%d.bmp", i);
        shadows[i] = bmp_load(filename);
        printf("seed %d: %d\n", i, shadows[i]->reserved[0] | (shadows[i]->reserved[1] << 8));
    }
    sss_recover(shadows, 8);
    return 0;
}
