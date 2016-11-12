#include <stdlib.h>
#include <setjmp.h>
#include <png.h>

void pngEncode(const char *carrierPath, const char *payloadPath, char *outputPath);
void pngDecode(const char *packagePath, char *outputPath);