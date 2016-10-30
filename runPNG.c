#include "runPNG.h"
#include "fileHandling.h"
#include "errorHandling.h"
#include "globalvars.h"
#include "encoding.h"

#define PNG_SIG_LENGTH 8                        //length of the PNG magin number, in bytes
#define BYTE_SIZE 8                             //size of a byte, in bits
#define MARKER 1635021427ul                     //integer that will indicate a file has a hidden payload
#define MARKER_LENGTH 32                        //length of MARKER, in bits
#define FILESIZE_LENGTH 32                      //length of the filesize value, in bits
#define MARKER_PLUS_FILESIZE 64                 //combined length of MARKER_LENGTH and FILESIZE_LENGTH


typedef struct pngReader
{
    png_infop info_ptr;                         //pointer to a png_info structure
    png_structp read_ptr;                       //pointer to a read png_struct structure
    png_bytep *row_pointers;                    //array of pointers to the pixel data for each row

    int width,                                  //width of the image in pixels
        height,                                 //height of the image in pixels
        bit_depth,                              //bit depth of the image
        color_type,                             //the PNG file's color type, represented as an integer
        channels;                               //number of color channels in the PNG file
} pngReader;


pngReader readPNG(const char *inputPath)
{
    FILE *inputFile;                            //pointer to the file at location inputPath
    unsigned char sigBuffer[BYTE_SIZE];         //char array to read inputFile's first 8 bytes into
    pngReader reader;                           //pngReader container that will hold inputFile's information



    //if inputFile cannot be opened, exit the program
    if ( !(inputFile = fopen(inputPath, "rb")) )
        error_(1, "%s: [readPNG] Cannot open '%s'.", exeName, inputPath);

    //if inputFile's first 8 bytes are not identical to the PNG magic number, close inputFile and exit the program
    fread(sigBuffer, 1, PNG_SIG_LENGTH, inputFile);
    if (png_sig_cmp(sigBuffer, 0, PNG_SIG_LENGTH))
    {
        fclose(inputFile);
        error_(1, "%s: [readPNG] '%s' is not a PNG file.", exeName, inputPath);
    }

    //create a read png_struct structure; if unsucessful, close inputFile and exit the program
    if ( !(reader.read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) )
    {
        fclose(inputFile);
        error_(1, "%s: [readPNG] 'png_create_read_struct' failed.", exeName);
    }

    //create an info png_struct structure; if unsucessful, destroy the read png_struct structure, close inputFile and exit the program
    if ( !(reader.info_ptr = png_create_info_struct(reader.read_ptr)) )
    {
        png_destroy_read_struct(&reader.read_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(inputFile);
        error_(1, "%s: [readPNG] 'png_create_info_struct' (info_ptr) failed.", exeName);
    }

    //if "png_init_io" fails, jump back here to destroy the read png_struct structure, close inputFile and exit the program
    if (setjmp(png_jmpbuf(reader.read_ptr)))
    {
        png_destroy_read_struct(&reader.read_ptr, &reader.info_ptr, (png_infopp)NULL);
        fclose(inputFile);
        error_(1, "%s: [readPNG] Error during 'init_io'.", exeName);
    }
    //initialize input/output for inputFile
    png_init_io(reader.read_ptr, inputFile);

    //set the first eight bytes of inputFile as already read
    png_set_sig_bytes(reader.read_ptr, PNG_SIG_LENGTH);

    //if "png_read_png" fails, jump back here to destroy the read png_struct structure, close inputFile and exit the program
    if (setjmp(png_jmpbuf(reader.read_ptr)))
    {
        png_destroy_read_struct(&reader.read_ptr, &reader.info_ptr, (png_infopp)NULL);
        fclose(inputFile);
        error_(1, "%s: [readPNG] Error during 'read_png'.", exeName);
    }
    //read inputFile
    png_read_png(reader.read_ptr, reader.info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    //retrieve image data from info_ptr
    reader.row_pointers = png_get_rows(reader.read_ptr, reader.info_ptr);

    //retrieve inputFile's width, height, bit depth, color type, and number of color channels
    reader.width = png_get_image_width(reader.read_ptr, reader.info_ptr);
    reader.height = png_get_image_height(reader.read_ptr, reader.info_ptr);
    reader.bit_depth = png_get_bit_depth(reader.read_ptr, reader.info_ptr);
    reader.color_type = png_get_color_type(reader.read_ptr, reader.info_ptr);
    reader.channels = png_get_channels(reader.read_ptr, reader.info_ptr);

    //if inputFile's bit depth does not equal one byte, destroy the read png_struct structure, close inputFile, and exit the program
    if (reader.bit_depth != BYTE_SIZE)
    {
        png_destroy_read_struct(&reader.read_ptr, &reader.info_ptr, (png_infopp)NULL);
        fclose(inputFile);
        error_(1, "%s: [readPNG] Bit depth does not equal 1 byte (8 bits).", exeName);
    }

    printf("bitdepth: %d\ncolortype: %d\n", reader.bit_depth, reader.color_type);

    //close inputFile and return reader
    fclose(inputFile);
    return reader;
}

static void writePNG(pngReader *inputPNG, char *outputPath)
{
    FILE *outputFile;                           //file pointer to file at location outputPath
    png_structp write_ptr;                      //write png_struct structure



    //if a file already exists at outputPath, or a file cannot be created at outputPath, destroy the read png_struct structure and exit the program
    if (favailable(outputPath))
    {
        if ( !(outputFile = fopen(outputPath, "wb")) )
        {
            png_destroy_read_struct(&inputPNG->read_ptr, &inputPNG->info_ptr, (png_infopp)NULL);
            error_(1, "%s: [writePNG] Could not create '%s' file.", exeName, outputPath);
        }
    }
    else
    {
        png_destroy_read_struct(&inputPNG->read_ptr, &inputPNG->info_ptr, (png_infopp)NULL);
        error_(1, "%s: [writePNG] file '%s' already exists.", exeName, outputPath);
    }

    //create a write png_struct; if unsuccessful, destroy the read png_struct structure, delete outputFile and exit the program
    if ( !(write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) )
    {
        png_destroy_read_struct(&inputPNG->read_ptr, &inputPNG->info_ptr, (png_infopp)NULL);
        fremove(outputFile, outputPath);
        error_(1, "%s: [writePNG] 'png_create_write_struct' failed.", exeName);
    }

    //if "png_init_io" fails, jump back here to destroy the read and write png_struct structures, delete outputFile and exit the program
    if (setjmp(png_jmpbuf(write_ptr)))
    {
        png_destroy_write_struct(&write_ptr, &inputPNG->info_ptr);
        png_destroy_read_struct(&inputPNG->read_ptr, &inputPNG->info_ptr, (png_infopp)NULL);
        fremove(outputFile, outputPath);
        error_(1, "%s: [writePNG] Error during 'init_io'.", exeName);
    }
    //initialize input/output for outputFile
    png_init_io(write_ptr, outputFile);

    //put the image data from row_pointers into the png_info structure
    png_set_rows(write_ptr, inputPNG->info_ptr, inputPNG->row_pointers);

    //if "png_write_png" fails, jump back here to destroy the read and write png_struct structures, delete outputFile and exit the program
    if (setjmp(png_jmpbuf(write_ptr)))
    {
        png_destroy_write_struct(&write_ptr, &inputPNG->info_ptr);
        png_destroy_read_struct(&inputPNG->read_ptr, &inputPNG->info_ptr, (png_infopp)NULL);
        fremove(outputFile, outputPath);
        error_(1, "%s: [readPNG] Error during 'write_png'.", exeName);
    }
    //write the PNG file to outputFile
    png_write_png(write_ptr, inputPNG->info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    //close outputFile and destroy the write png_struct structute
    fclose(outputFile);
    png_destroy_write_struct(&write_ptr, &inputPNG->info_ptr);

    return;
}

void pngEncode(const char *carrierPath, const char *payloadPath, char *outputPath)
{
    pngReader carrier;                          //pngReader container that will hold the information of the file at location carrierPath
    FILE *payload;                              //pointer to the file at location payloadPath
    FILE *herp, *derp;
    unsigned long payloadsize = 0;              //size of file "payload" in bytes
    unsigned char bytebuffer = 0;               //buffer to hold the payload byte currently being written



    //initialize carrier
    carrier = readPNG(carrierPath);

    //if the initialization of payload fails, exit the program
    if ( !(payload = fopen(payloadPath, "rb")) )
        error_(1, "%s: [pngEncode] Could not read in payload.", exeName);
    else
        payloadsize = fsize(payloadPath);

    //for the payload to be successfully encoded, the number of pixels in the carrier image times its number of color channels must be EQUAL TO OR GREATER THAN the size of the payload in bits plus 64 additional bits. If not, exit the program.
    if ((carrier.width * carrier.height * carrier.channels) < (payloadsize * 8 + MARKER_PLUS_FILESIZE))
        error_(1, "%s: [pngEncode] Payload will not fit in carrier.", exeName);

    if (loggingEnabled)
    {
        herp = fopen("herpcarrier.log", "w");
        derp = fopen("derpcarrier.log", "w");
    }

    //iterate through each row of the carrier image pixel data
    for (int y = 0; y < carrier.height; y++)
    {
        //reset the column position to 0 for each new row
        int x = 0;

        //if we are on the first row of pixels, encode our marker number and "payloadsize" before doing anything else
        if (y == 0)
        {
            for (x = x; x < MARKER_PLUS_FILESIZE; x++)
            {
                if (loggingEnabled)
                    fwrite(carrier.row_pointers[y]+x, 1, 1, herp);

                if (x < MARKER_LENGTH)
                    writebit(MARKER, (unsigned char *)(carrier.row_pointers[y]+x), x);
                else
                    writebit(payloadsize, (unsigned char *)(carrier.row_pointers[y]+x), x - MARKER_LENGTH);

                if (loggingEnabled)
                    fwrite(carrier.row_pointers[y]+x, 1, 1, derp);
            }
        }

        //each row of bytes has a length equivalent to the width of the carrier image times its number of color channels
        for (x = x; x < carrier.width * carrier.channels; x++)
        {
            //if no payload bytes have been written on this row, or if the payload byte in "bytebuffer" has been fully writter, attempt to read in another byte from the payload
            if (x % BYTE_SIZE == 0)
                //if there are no more payload bytes left to read in, or if no more payload bytes can be read in, stop iterating through the carrier image pixel data
                if (!fread(&bytebuffer, 1, 1, payload))
                    goto LOOP_END;

            if (loggingEnabled)
                fwrite(carrier.row_pointers[y]+x, 1, 1, herp);

            //write the appropriate bit of bytebuffer to the least significant position of the current carrier byte
            writebit((unsigned long)bytebuffer, (unsigned char *)(carrier.row_pointers[y]+x), x % BYTE_SIZE);

            if (loggingEnabled)
                fwrite(carrier.row_pointers[y]+x, 1, 1, derp);
        }
    }

    LOOP_END:
    //add the .png extension to the desired output name
    outputPath = faddExt(outputPath, ".png");
    //create a new file at location outputPath and write to it our generated package image
    writePNG(&carrier, outputPath);

    if (loggingEnabled)
    {
        fclose(herp);
        fclose(derp);
    }
    //close the payload file
    fclose(payload);
    //destroy read png_struct structure
    png_destroy_read_struct(&carrier.read_ptr, &carrier.info_ptr, (png_infopp)NULL);

    return;
}

void pngDecode(const char *packagePath, char *outputPath)
{
    pngReader package;
    FILE *outputFile;
    FILE *herpderp;
    unsigned long markervalue = 0,
                  payloadsize = 0;
    unsigned char bytebuffer = 0;


    //read in information from the package PNG file
    package = readPNG(packagePath);

    //exit the program if a file already exists at loacation outputPath, or if a file cannot be created at outputPath
    if (favailable(outputPath))
    {
        if ( !(outputFile = fopen(outputPath, "wb")) )
            error_(1, "%s: [pngDecode] Could not create '%s' file.", exeName, outputPath);
    }
    else
        error_(1, "%s: [pngDecode] file '%s' already exists.", exeName, outputPath);

    if (loggingEnabled)
        herpderp = fopen("herpderpcarrier.log", "w");

    //itherate through row_pointers
    for (int y = 0; y < package.height; y++)
    {
        //reset column position to 0 for each row
        int x = 0;

        //do these things before writing anything to outputFile
        if (y == 0)
        {
            for (x = x; x < MARKER_PLUS_FILESIZE; x++)
            {
                //if the completed markervalue is not equal to MARKER, then a package file embedded by this program was not found; destroy the read png_struct structure, close log file if logging is enabled, and exit the program.
                if (x == MARKER_LENGTH)
                    if (markervalue != MARKER)
                    {
                        png_destroy_read_struct(&package.read_ptr, &package.info_ptr, (png_infopp)NULL);
                        if (loggingEnabled)
                            fclose(herpderp);
                        error_(1, "%s: [pngDecode] file '%s' does not contain a file embedded by this program.", exeName, packagePath);
                    }

                if (loggingEnabled)
                    fwrite(package.row_pointers[y]+x, 1, 1, herpderp);

                //if not all bits of markervalue have been assinged, assign this bit to the appropriate bit position in markervalue; otherwise, assign this bit to the appropriate bit position in payloadsize
                if (x < MARKER_LENGTH)
                    markervalue |= ( ( *(package.row_pointers[y]+x) & 1) << x);
                else
                    payloadsize |= ( ( *(package.row_pointers[y]+x) & 1) << (x - MARKER_LENGTH));
            }
        }

        //iterate through row_pointers[y]
        for (x = x; x < package.width * package.channels; x++)
        {
            if (loggingEnabled)
                fwrite(package.row_pointers[y]+x, 1, 1, herpderp);

            //if all 8 bits in bytebuffer have been set, write the byte to outputFile and clear bytebuffer
            if ((x > MARKER_PLUS_FILESIZE || y > 0) && x % BYTE_SIZE == 0)
                fwrite(&bytebuffer, 1, 1, outputFile), bytebuffer = 0;

            //if all bits of the payload have been written, exit the loop
            if ((package.width * y * package.channels + x) == payloadsize * BYTE_SIZE + MARKER_PLUS_FILESIZE)
                goto LOOP_END;

            //use the LSB of the current package byte to set the appropriate bit position in bytebuffer 
            bytebuffer |= ((*(package.row_pointers[y]+x) & 1) << x % BYTE_SIZE);
        }
    }

    LOOP_END:
    if (loggingEnabled)
        fclose(herpderp);
    fclose(outputFile);

    return;
}