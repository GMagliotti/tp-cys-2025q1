#include "bmp.h"

int main(int argc, char const *argv[])
{
    BmpImage* image = bmp_load("./beetmaep.bmp");
    if (image == NULL) {
        fprintf(stderr, "Failed to load BMP image\n");
        return 1;
    }
    printf("Width: %d\n", image->width);
    printf("Height: %d\n", image->height);
    printf("BPP: %d\n", image->bpp);
    printf("Successfully load the whole thing!\n");

    if (bmp_save("./meetbaep.bmp", image) == -1) {
        printf("Couldn't save file\n");
    }
    return 0;
}
