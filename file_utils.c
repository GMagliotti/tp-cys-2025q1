#include "file_utils.h"

// todo use this instead of direct fread
int32_t fileutils_fread(void *ptr, size_t size, size_t count, FILE *stream) {
    size_t total_read = 0;
    while (total_read < count) {
        size_t bytes_read = fread((char *)ptr + total_read * size, size, count - total_read, stream);
        if (bytes_read == 0) {
            if (ferror(stream)) {
                perror("Error reading file");
                return -1;
            }
            break; // EOF reached
        }
        total_read += bytes_read;
    }
    return total_read;
}