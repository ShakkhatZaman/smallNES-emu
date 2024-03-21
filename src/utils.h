#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>

#define ERROR(format, ...) {                        \
    fprintf(stderr, "***\033[0;31mERROR\033[0m: "); \
    fprintf(stderr, format, __VA_ARGS__);           \
    fprintf(stderr, "\n");                          \
}

#define ERROR_RETURN(format, ...) {                 \
    ERROR(format, __VA_ARGS__)                      \
    return -1;                                      \
}

#define ERROR_EXIT(format, ...) {                   \
    ERROR(format, __VA_ARGS__)                      \
    exit(EXIT_FAILURE);                             \
}

#endif // !UTILS_H
