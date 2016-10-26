#include "errorHandling.h"

void error_(int do_exit, const char * s, ...)
{
        va_list args;
        va_start(args, s);
        vfprintf(stderr, s, args);
        fprintf(stderr, "\n");
        va_end(args);
        if (do_exit)
        {
            fprintf(stderr, "Exiting...\n");
            exit(EXIT_FAILURE);
        }
}