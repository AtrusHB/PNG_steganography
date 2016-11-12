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

static void printHelp(void)
{
    printf(
        "usage:\n"
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

static void readArgs(int argc, char *argv[])
{
    int arg;
    int index = 0;

    opterr = 0;

    const struct option long_options[] =
    {
        { "carrier", required_argument, 0, 'c' },
        { "payload", required_argument, 0, 'p' },
        { "package", required_argument, 0, 'k' },
        { 0,         0,                 0,  0  }
    };

    while ((arg = getopt_long(argc, argv, ":c:p:k:", long_options, &index)) != -1)
    {
        switch (arg)
        {
            case 'c':
                if (optarg[0] == '-')
                    optind--, error_(0, "%s: [readArgs] Option '-c' requires an argument.", exeName);
                else
                    cstr = optarg;
                break;
            case 'p':
                if (optarg[0] == '-')
                    optind--, error_(0, "%s: [readArgs] Option '-p' requires an argument.", exeName);
                else
                    pstr = optarg;
                break;
            case 'k':
                if (optarg[0] == '-')
                    optind--, error_(0, "%s: [readArgs] Option '-k' requires an argument.", exeName);
                else
                    kstr = optarg;
                break;
            case ':':
                error_(0, "%s: [readArgs] Option '-%c' requires an argument.", exeName, optopt);
                break;
            case '?':
                if (isprint(optopt))
                    error_(0, "%s: [readArgs] unrecognized option flag '-%c'.", exeName, optopt);
                break;
                //exit(EXIT_FAILURE);
            default:
                error_(1, "%s: [readArgs] Failed.", exeName);
        }
    }

    for (int i = optind; i < argc; i++){
        if (strcmp(argv[i], "encode") == 0)
            if (encode != decode)
                error_(0, "%s: [readArgs] Operation mode already defined.", exeName);
            else
                encode = 1;
        else if (strcmp(argv[i], "decode") == 0)
            if (encode != decode)
                error_(0, "%s: [readArgs] Operation mode already defined.", exeName);
            else
                decode = 1;
        else if (strcmp(argv[i], "help") == 0)
            if (argc != 2)
                error_(1, "%s: [readArgs] Operation mode 'help' takes no options or arguments.", exeName);
            else
            {
                printHelp();
                exit(EXIT_SUCCESS);
            }
        else
            error_(0, "%s: [readArgs] Unknown argument '%s'. Will be discarded.", exeName, argv[i]);
    }

    if (encode == decode)
        error_(1, "%s: [readArgs] Operation mode undefined.", exeName);
}

static void hasReqArgs (void)
{
    int haveAllArgs = 1;

    if (encode)
    {
        if (!cstr)
        {
            error_(0, "%s: [hasReqArgs] -c/--carrier is required for mode 'encode'.", exeName);
            haveAllArgs = 0;
        }
        if (!pstr)
        {
            error_(0, "%s: [hasReqArgs] -p/--payload is required for mode 'encode'.", exeName);
            haveAllArgs = 0;
        }

        if (!kstr)
            kstr = "package";
    }
    else if (decode)
    {
        if (!kstr)
        {
            error_(0, "%s: [hasReqArgs] -k/--package is required for mode 'decode'.", exeName);
            haveAllArgs = 0;
        }

        if (!pstr)
            pstr = "payload";
    }
    else
        error_(1, "%s: [hasReqArgs] Operation mode undefined.", exeName);

    if (!haveAllArgs)
        error_(1, "%s: [hasReqArgs] One or more required arguments missing.", exeName);
}

static void checkFiles(void)
{
    int finalResult;

    if (encode)
    {
        //return GetFileAttributes(cstr) != INVALID_FILE_ATTRIBUTES;
        int carrierResult = fexist(cstr, "carrier");
        int payloadResult = fexist(pstr, "payload");

        finalResult = !carrierResult && !payloadResult;
    }
    else if (decode)
    {
        int packageResult = fexist(kstr, "package");

        finalResult = (packageResult == 0);
    }
    else
        error_(1, "%s: [checkFiles] Operation mode undefined.", exeName);

    if (!finalResult)
        error_(1, "Failure");
}

static void runType()
{
    if (encode)
        return pngEncode(cstr, (const char *)pstr, kstr);
    else if (decode)
        return pngDecode((const char *)kstr, pstr);
    else
        error_(1, "%s: [runType] Operation mode undefined.", exeName);
}

int main(int argc, char *argv[])
{
    exeName = argv[0];
    loggingEnabled = 1;

    readArgs(argc, argv);
    printf("c = %s\np = %s\nk = %s\n", cstr, pstr, kstr);
    hasReqArgs();
    checkFiles();
    runType();
    printf("success\n");
    exit(EXIT_SUCCESS);

    return 0;
}