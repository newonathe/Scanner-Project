#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include "inputs.h"

static int isinputfile(const char *name)
{
    const char *suffix = "_input.txt";
    size_t n = strlen(name), m = strlen(suffix);
    return n > m && strcmp(name + n - m, suffix) == 0;
}

int runinputs(int argc, char **argv, int (*process)(const char *))
{
    int i, count = 0, result = 0;
    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            int status = process(argv[i]);
            if (status == 2)
                return 1; /* the parser stops at its first parse error */
            if (status != 0)
                result = 1;
        }
    } else {
        DIR *directory = opendir(".");
        struct dirent *entry;
        if (directory == NULL) {
            fprintf(stderr, "Cannot open current directory\n");
            return 1;
        }
        while ((entry = readdir(directory)) != NULL) {
            int status;
            if (!isinputfile(entry->d_name))
                continue;
            count++;
            status = process(entry->d_name);
            if (status == 2) {
                closedir(directory);
                return 1;
            }
            if (status != 0)
                result = 1;
        }
        closedir(directory);
        if (count == 0) {
            fprintf(stderr, "No *_input.txt files in the current directory\n");
            return 1;
        }
    }
    return result;
}
