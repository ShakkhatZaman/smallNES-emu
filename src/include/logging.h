#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>

// #define CREATE_LOGS

extern FILE *log_file;

#define GET_LOG_FILE(filename) log_file = fopen(filename, "w")

#define CLOSE_LOG_FILE() fclose(log_file)

#define LOG_MESSAGE(format, ...) fprintf(log_file, format, __VA_ARGS__)

#endif // !LOGGING_H
