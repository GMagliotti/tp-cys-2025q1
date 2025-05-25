#ifndef _SSS_ALGOS_H
#define _SSS_ALGOS_H
#include "bmp.h"
#include "sss_helpers.h"
#include "permutation_table.h"

typedef BMPImageT **(*DistributeFnT)(BMPImageT *image, uint32_t k, uint32_t n, const char *covers_dir, const char *output_dir);
typedef BMPImageT *(*RecoverFnT)(BMPImageT **shadows, uint32_t k, const char * recovered_filename);

BMPImageT **sss_distribute_8(BMPImageT *image, uint32_t k, uint32_t n, const char *covers_dir, const char *output_dir);
BMPImageT **sss_distribute_generic(BMPImageT *image, uint32_t k, uint32_t n, const char *covers_dir, const char *output_dir);

BMPImageT *sss_recover_8(BMPImageT **shadows, uint32_t k, const char * recovered_filename);
BMPImageT *sss_recover_generic(BMPImageT **shadows, uint32_t k, const char * recovered_filename);

#endif