#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

int fexist(const char *filename, const char *inputType);
int favailable(const char *filename);
off_t fsize(const char *filename);
char *faddExt(char *filename, char *extension);
void fremove(FILE *file, const char *filename);