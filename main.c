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

    sss_distribute(image, 8, 10);
    return 0;
}
