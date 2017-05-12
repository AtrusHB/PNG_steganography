#include "test.h"
#include "globalvars.h"
#include "fileHandling.h"
#include "errorHandling.h"
#include "endianness.h"
#include "runPNG.h"

static int encode = 0;
static int decode = 0;
static char *pstr, *kstr;
static const char *cstr;

//Prints the Usage message.
static void printHelp(void)
{
    printf(
        "Usage:\n"
        "  %s help\n"
        "    Show this screen.\n\n"
        "  %s encode (-c|--carrier) <c> (-p|--payload) <p> [-k|--package] <k>\n"
        "    -c|--carrier <c>\tRequired; PNG file that will hold the specified payload.\n"
        "    -p|--payload <p>\tRequired; file that will be encoded to the specified carrier.\n"
        "    -k|--package <k>\tOptional; name of file to which to write the resulting package file.\n"
            "\t\t\t  Default value is 'package'.\n\n"
        "  %s decode (-k|--package) <k> [-p|--payload] <p>\n"
        "    -k|--package <k>\tRequired; package file containing an encoded payload.\n"
        "    -p|--payload <p>\tOptional; name of file to which to write the read payload.\n"
            "\t\t\t  Default value is 'payload'.\n",
        exeName, exeName, exeName
    );
}

//Reads in the program arguments.
static void readArgs(int argc, char *argv[])
{
    int arg;
    int index = 0;

    opterr = 0;
    //Option structure
    const struct option long_options[] =
    {
        { "carrier", required_argument, 0, 'c' },
        { "payload", required_argument, 0, 'p' },
        { "package", required_argument, 0, 'k' },
        { 0,         0,                 0,  0  }
    };
    //While there are still options in 'argv' left to process...
    while ((arg = getopt_long(argc, argv, ":c:p:k:", long_options, &index)) != -1)
    {
        //...Process option 'arg'.
        switch (arg)
        {
            case 'c':
                //If 'c' is not followed by an argument...
                if (optarg[0] == '-')
                    //...roll back 'optind' by 1 (thus ignoring 'c') and trigger a non-fatal error message.
                    optind--, error_(0, "%s: [readArgs] Option '-c' requires an argument.", exeName);
                else
                    //Else, assign the argument following 'c' to 'cstr'.
                    cstr = optarg;
                //Break out of the switch loop.
                break;
            case 'p':
                //If 'p' is not followed by an argument...
                if (optarg[0] == '-')
                    //...roll back 'optind' by 1 (thus ignoring 'p') and trigger a non-fatal error message.
                    optind--, error_(0, "%s: [readArgs] Option '-p' requires an argument.", exeName);
                else
                    //Else, assign the argument following 'p' to 'pstr'.
                    pstr = optarg;
                //Break out of the switch loop.
                break;
            case 'k':
                //If 'k' is not followed by an argument...
                if (optarg[0] == '-')
                    //...roll back 'optind' by 1 (thus ignoring 'k') and trigger a non-fatal error message.
                    optind--, error_(0, "%s: [readArgs] Option '-k' requires an argument.", exeName);
                else
                    //Else, assign the argument following 'k' to 'kstr'.
                    kstr = optarg;
                //Break out of the switch loop.
                break;
            //If option is missing an argument...
            case ':':
                //...trigger a non-fatal error message and break the switch loop.
                error_(0, "%s: [readArgs] Option '-%c' requires an argument.", exeName, optopt);
                break;
            //If option character 'arg' is not included in 'long-options[]'...
            case '?':
                //...and if 'arg' (now stored in 'optopt') is a printable character...
                if (isprint(optopt))
                    //...trigger a non-fatal error message.
                    error_(0, "%s: [readArgs] unrecognized option flag '-%c'.", exeName, optopt);
                //Break out of the switch loop.
                break;
            //If option 'arg' does not match any of the above cases...
            default:
                //...trigger a fatal error message.
                error_(1, "%s: [readArgs] Failed.", exeName);
        }
    }
    //Process program arguments.
    for (int i = optind; i < argc; i++)
    {
        //If 'argv[i]' is "encode"...
        if (strcmp(argv[i], "encode") == 0)
            //If either 'encode' or 'decode' has already been set to 1...
            if (encode != decode)
                //...trigger a non-fatal error message.
                error_(0, "%s: [readArgs] Operation mode already defined.", exeName);
            else
                //Else, set 'encode' to 1.
                encode = 1;
        //...else, if 'argv[i]' is "decode"...
        else if (strcmp(argv[i], "decode") == 0)
            //If either 'encode' or 'decode' has already been set to 1...
            if (encode != decode)
                //...trigger a non-fatal error message.
                error_(0, "%s: [readArgs] Operation mode already defined.", exeName);
            else
                //Else, set 'decode' to 1.
                decode = 1;
        //...else, if 'argv[i]' is "help"...
        else if (strcmp(argv[i], "help") == 0)
            //If there are arguments other than the program name and 'help'...
            if (argc != 2)
                //...trigger a fatal error message.
                error_(1, "%s: [readArgs] Operation mode 'help' takes no options or arguments.", exeName);
            //Else...
            else
            {
                //...print the Usage message and exit the program successfully.
                printHelp();
                exit(EXIT_SUCCESS);
            }
        //...else, if 'argv[i]' does not match any of the above three conditions...
        else
            //...ignore 'argv[i]' and trigger a non-fatal error message.
            error_(0, "%s: [readArgs] Unknown argument '%s'. Will be discarded.", exeName, argv[i]);
    }
    //If neither 'encode' nor 'decode' was set to 1...
    if (encode == decode)
        //...trigger a fatal error message.
        error_(1, "%s: [readArgs] Operation mode undefined.", exeName);
}

//Checks if the required options are set for the mode selected.
static void hasReqOpts (void)
{
    //Begin with the assumption that all required options have been set.
    int haveAllOpts = 1;
    //If the selected mode is "encode"...
    if (encode)
    {
        //...and if 'cstr' has not been set...
        if (!cstr)
        {
            //...trigger a non-fatal error message and indicate that not all required options are set.
            error_(0, "%s: [hasReqOpts] -c/--carrier is required for mode 'encode'.", exeName);
            haveAllOpts = 0;
        }
        //...and if 'pstr' has not been set...
        if (!pstr)
        {
            //...trigger a non-fatal error message and indicate that not all required options are set.
            error_(0, "%s: [hasReqOpts] -p/--payload is required for mode 'encode'.", exeName);
            haveAllOpts = 0;
        }
        //If 'kstr' has not been set...
        if (!kstr)
            //...use the default char array "package".
            kstr = "package";
    }
    //...else, if the selected mode is "decode"...
    else if (decode)
    {
        //...and if 'kstr' has not been set...
        if (!kstr)
        {
            //...trigger a non-fatal error message and indicate that not all required options are set.
            error_(0, "%s: [hasReqOpts] -k/--package is required for mode 'decode'.", exeName);
            haveAllOpts = 0;
        }
        //...and if 'pstr' has not been set...
        if (!pstr)
            //...use the default char array "payload".
            pstr = "payload";
    }
    //...else, if the selected mode is somehow neither "encode" nor "decode"...
    else
        //...trigger a fatal error message.
        error_(1, "%s: [hasReqOpts] Operation mode undefined.", exeName);
    //If 'haveAllOpts' is not still 1...
    if (!haveAllOpts)
        //...trigger a fatal error message.
        error_(1, "%s: [hasReqOpts] One or more required arguments missing.", exeName);
}

//Checks if all files required for the specified operation mode exist.
static void checkFiles(void)
{
    int requFilesExist;
    //If the selected mode is "encode"...
    if (encode)
    {
        /*'fexist' returns a 0 if a file exists with a name
        identical to the first parameter's stored value.*/
        int carrierResult = fexist(cstr, "carrier");
        int payloadResult = fexist(pstr, "payload");
        /*If both the specified carrier and payload files exist,
        'reqFilesExist' will be set to 1.*/
        requFilesExist = !carrierResult && !payloadResult;
    }
    //...else, if the selected mode is "decode"...
    else if (decode)
    {
        /*'fexist' returns a 0 if a file exists with a name
        identical to the first parameter's stored value.*/
        int packageResult = fexist(kstr, "package");

        /*If both the specified carrier and payload files exist,
        'reqFilesExist' will be set to 1.*/
        requFilesExist = (packageResult == 0);
    }
    //...else, if the selected mode is somehow neither "encode" nor "decode"...
    else
        //...trigger a fatal error message.
        error_(1, "%s: [checkFiles] Operation mode undefined.", exeName);

    //If any of the required files do not exist...
    if (!requFilesExist)
        //...trigger a fatal error message.
        error_(1, "Failure");
}

//Executes the specified operaion mode.
static void runType()
{
    //If the selected mode is "encode"...
    if (encode)
        //...run the encode function
        return pngEncode(cstr, (const char *)pstr, kstr);
    //...else, if the selected mode is "decode"...
    else if (decode)
        //run the decode function
        return pngDecode((const char *)kstr, pstr);
    //...else, if the selected mode is somehow neither "encode" nor "decode"...
    else
        //...trigger a fatal error message.
        error_(1, "%s: [runType] Operation mode undefined.", exeName);
}

int main(int argc, char *argv[])
{
    exeName = argv[0];
    loggingEnabled = 1;

    readArgs(argc, argv);
    //Print the values for carrier, payload, and package.
    printf("c = %s\np = %s\nk = %s\n", cstr, pstr, kstr);
    hasReqOpts();
    checkFiles();
    runType();
    /*Reaching this point means the program has run properly.
    Print success message and exit the program successfully.*/
    printf("success\n");
    exit(EXIT_SUCCESS);

    return 0;
}