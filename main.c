#include "sss.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <dirent.h>

int main(int argc, char const *argv[]) {
    int distribute = 0;
    int recover = 0;
    char *secret_file = NULL;
    char *dir = ".";
    int k = -1;
    int n = -1;

    static struct option long_options[] = {
        {"d",       no_argument,       0, 'd'},
        {"r",       no_argument,       0, 'r'},
        {"secret",  required_argument, 0, 's'},
        {"k",       required_argument, 0, 'k'},
        {"n",       required_argument, 0, 'n'},
        {"dir",     required_argument, 0, 'D'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, (char * const *)argv, "drs:k:n:D:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'd':
                distribute = 1;
                break;
            case 'r':
                recover = 1;
                break;
            case 's':
                secret_file = optarg;
                break;
            case 'k':
                k = atoi(optarg);
                break;
            case 'n':
                n = atoi(optarg);
                break;
            case 'D':
                dir = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s --d|--r --secret file --k num [--n num] [--dir directory]\n", argv[0]);
                return 1;
        }
    }

    // Validation of mandatory parameters
    if ((distribute + recover) != 1 || !secret_file || k <= 0) {
        fprintf(stderr, "Error: Missing or incorrect mandatory parameters.\n");
        fprintf(stderr, "Use: %s -d|-r -secret archivo -k num [-n num] [-dir directory]\n", argv[0]);
        return 1;
    }

    if (distribute) {
        // Distribute
        BmpImage *image = bmp_load(secret_file);
        if (!image) {
            fprintf(stderr, "Could not load secret image: %s", secret_file);
            return 1;
        }

        // If n was not specified, search for all images in the directory
        if (n == -1) {
            DIR *dp = opendir(dir);
            if (!dp) {
                perror("Could not open the directory");
                return 1;
            }
            n = 0;
            struct dirent *entry;
            while ((entry = readdir(dp))) {
                if (strstr(entry->d_name, ".bmp")) {
                    n++;
                }
            }
            closedir(dp);
        }

        sss_distribute(image, k, n, dir, "./stego_images");
    } else if (recover) {
        // Recover
        BMPImageT *shadows[n];
        char filename[512];

        for (int i = 0; i < n; i++) {
            snprintf(filename, sizeof(filename), "%s/stego%d.bmp", dir, i + 1);
            shadows[i] = bmp_load(filename);
            if (!shadows[i]) {
                fprintf(stderr, "Could not load: %s\n", filename);
                return 1;
            }
        }

        BMPImageT *recovered = sss_recover(shadows, k, secret_file);
        if (recovered) {
            bmp_save(secret_file, recovered);
        } else {
            fprintf(stderr, "Failure to recover the secret\n");
        }
    }

    return 0;
}
