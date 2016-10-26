#include <stdlib.h>
#include <setjmp.h>
#include <png.h>

/*typedef struct IHDR ihdr
{
    int width;
    int height;
    
    png_byte *trans_alpha;
    int num_trans;
    png_color_16 *trans_color;
};*/

void pngEncode(const char *carrierPath, const char *payloadPath, char *outputPath);
void pngDecode(const char *packagePath, char *outputPath);