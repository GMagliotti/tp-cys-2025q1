#ifndef _FILE_UTILS_H
#define _FILE_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int32_t safe_fread(void *ptr, size_t size, size_t count, FILE *stream);

#endif