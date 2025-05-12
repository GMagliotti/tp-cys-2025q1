#include "sss.h"
#include "sss_algos.h"

static DistributeFnT get_distribute_function(uint32_t k)
{
    if (k == 8)
    {
        return sss_distribute_8;
    }
    return sss_distribute_generic;
}

static RecoverFnT get_recover_function(uint32_t k)
{
    if (k == 8)
    {
        return sss_recover_8;
    }
    return sss_recover_generic;
}

BMPImageT **sss_distribute(BMPImageT *image, uint32_t k, uint32_t n)
{
    if (k < 2 || k > 10)
    {
        fprintf(stderr, "Invalid parameters: k must be between 2 and 10\n");
        return NULL;
    }

    if (n < 2 || n < k)
    {
        fprintf(stderr, "Invalid parameters: n must be greater than 1, and k must be smaller than n");
    }

    BMPImageT **shadows = get_distribute_function(k)(image, k, n);
    return shadows;
}

BMPImageT *sss_recover(BMPImageT **shadows, uint32_t k)
{
    BMPImageT *image = get_recover_function(k)(shadows, k);
    return image;
}