#include "fileHandling.h"
#include "globalvars.h"
#include "errorHandling.h"

int fexist(const char *filename, const char *inputType)
{
    int temp;
    struct stat fileStat;

    if (( temp = stat(filename, &fileStat)) == -1)
        error_(0, "%s: [fexist] Error occurred with %s input '%s': %s", exeName, filename, strerror(errno));

    return temp;
}

int favailable(const char *filename)
{
    struct stat fileStat;

    if (stat(filename, &fileStat) != 0)
        return 1;
    else
    {
        error_(1, "%s: [favailable] File '%s' already exists.", exeName, filename);
        return 0;
    }
}

off_t fsize(const char *filename)
{
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    fprintf(stderr, "%s: [fsize] Cannot determine size of %s: %s\n",
        exeName, filename, strerror(errno));

    return -1;
}

char *faddExt(char *filename, char *extension)
{
    char *result = malloc(strlen(filename) + strlen(extension) + 1);

    strcpy(result, filename);
    strcat(result, extension);
    return result;
}

void fremove(FILE *file, const char *filename)
{

    fclose(file);
    if ((remove(filename)) != 0)
        error_(0, "%s: [fremove] %s.", exeName, strerror(errno));
}