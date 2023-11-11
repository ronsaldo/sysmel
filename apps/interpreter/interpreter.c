#include <stdio.h>
#include <string.h>
#include <sysbvm/orderedCollection.h>
#include <sysbvm/context.h>
#include <sysbvm/environment.h>
#include <sysbvm/filesystem.h>
#include <sysbvm/sourceCode.h>
#include <sysbvm/scanner.h>
#include <sysbvm/parser.h>
#include <sysbvm/interpreter.h>
#include <sysbvm/stackFrame.h>
#include <sysbvm/string.h>

static sysbvm_context_t *context;

const char *destinationImageFilename;

static void printHelp()
{
    printf("sysbvm-lispi <inputfile> <inputfile> -- args\n");
}

static void printVersion()
{
    printf("sysbvm-lispi version 0.1\n");
}

int doMain(int startArgumentIndex, int argc, const char *argv[])
{
    bool scanOnly = false;
    bool parseOnly = false;
    bool printStats = false;
    struct {
        sysbvm_tuple_t filesToProcess;
        sysbvm_tuple_t remainingArgs;
        sysbvm_tuple_t inputFileName;
        sysbvm_tuple_t inlineScriptToRun;
        sysbvm_tuple_t inlineScriptsToRun;
        sysbvm_tuple_t sourceCode;
        sysbvm_tuple_t scanResult;
        sysbvm_tuple_t parseResult;
        sysbvm_tuple_t parseScanResults;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    // Parse the command line.
    gcFrame.filesToProcess = sysbvm_orderedCollection_create(context);
    gcFrame.remainingArgs = sysbvm_orderedCollection_create(context);
    gcFrame.inlineScriptsToRun = sysbvm_orderedCollection_create(context);
    bool isParsingRemainingArgs = false;
    for(int i = startArgumentIndex; i < argc; ++i)
    {
        const char *arg = argv[i];
        if(isParsingRemainingArgs)
        {
            sysbvm_orderedCollection_add(context, gcFrame.remainingArgs, sysbvm_string_createWithCString(context, arg));
            continue;
        }
        
        if(*arg == '-')
        {
            if(!strcmp(arg, "-help"))
            {
                printHelp();
                return 0;
            }
            else if(!strcmp(arg, "-version"))
            {
                printVersion();
                return 0;
            }
            else if(!strcmp(arg, "-save-image"))
            {
                arg = argv[++i];
                destinationImageFilename = arg;
            }
            else if(!strcmp(arg, "-scan-only"))
            {
                scanOnly = true;
            }
            else if(!strcmp(arg, "-parse-only"))
            {
                parseOnly = true;
            }
            else if(!strcmp(arg, "-print-stats"))
            {
                printStats = true;
            }
            else if(!strcmp(arg, "-e"))
            {
                arg = argv[++i];
                gcFrame.inlineScriptToRun = sysbvm_string_createWithCString(context, arg);
                sysbvm_orderedCollection_add(context, gcFrame.inlineScriptsToRun, gcFrame.inlineScriptToRun);
            }
            else if(!strcmp(arg, "--"))
            {
                isParsingRemainingArgs = true;
            }
            else if(!strcmp(argv[i], "-m32") ||
                !strcmp(argv[i], "-m64") ||
                !strcmp(argv[i], "-nojit") ||
                !strcmp(argv[i], "-nogc")
            )
            {
                // These options are parsed before the context creation.
            }
        }
        else
        {
            gcFrame.inputFileName = sysbvm_string_createWithCString(context, arg);
            sysbvm_orderedCollection_add(context, gcFrame.filesToProcess, gcFrame.inputFileName);
        }
    }

    if(scanOnly || parseOnly)
    {
        gcFrame.parseScanResults = sysbvm_orderedCollection_create(context);
        {
            size_t inputFileSize = sysbvm_orderedCollection_getSize(gcFrame.filesToProcess);
            for(size_t i = 0; i < inputFileSize; ++i)
            {
                gcFrame.inputFileName = sysbvm_orderedCollection_at(gcFrame.filesToProcess, i);
                gcFrame.inputFileName = sysbvm_filesystem_absolute(context, gcFrame.inputFileName);
                gcFrame.sourceCode = sysbvm_interpreter_loadSourceCodeWithSolvedPath(context, gcFrame.inputFileName);
                gcFrame.scanResult = sysbvm_scanner_scan(context, gcFrame.sourceCode);
                if(scanOnly)
                    sysbvm_orderedCollection_add(context, gcFrame.parseScanResults, gcFrame.scanResult);

                if(parseOnly)
                {
                    gcFrame.parseResult = sysbvm_parser_parseTokens(context, gcFrame.sourceCode, gcFrame.scanResult);
                    sysbvm_orderedCollection_add(context, gcFrame.parseScanResults, gcFrame.parseResult);
                }
            }

        }
    }
    else
    {
        {
            size_t inputFileSize = sysbvm_orderedCollection_getSize(gcFrame.filesToProcess);
            for(size_t i = 0; i < inputFileSize; ++i)
            {
                gcFrame.inputFileName = sysbvm_orderedCollection_at(gcFrame.filesToProcess, i);
                gcFrame.inputFileName = sysbvm_filesystem_absolute(context, gcFrame.inputFileName);
                sysbvm_interpreter_loadSourceNamedWithSolvedPath(context, gcFrame.inputFileName);
            }

            sysbvm_analysisQueue_waitPendingAnalysis(context, sysbvm_analysisQueue_getDefault(context));
        }

        {
            size_t inlineScriptToRunCount = sysbvm_orderedCollection_getSize(gcFrame.inlineScriptsToRun);
            for(size_t i = 0; i < inlineScriptToRunCount; ++i)
            {
                gcFrame.inlineScriptsToRun = sysbvm_orderedCollection_at(gcFrame.inlineScriptsToRun, i);
                sysbvm_interpreter_evaluateScript(context, gcFrame.inlineScriptsToRun, sysbvm_string_createWithCString(context, "commandLine"), sysbvm_symbol_internWithCString(context, "sysmel"));
            }

            sysbvm_analysisQueue_waitPendingAnalysis(context, sysbvm_analysisQueue_getDefault(context));
        }
    }

    if(printStats)
        sysbvm_context_printMemoryUsageStats(context);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return 0;
}

int mainWithContext(int startArgumentIndex, int argc, const char *argv[])
{
    sysbvm_stackFrameLandingPadRecord_t topLevelFrame = {
        .type = SYSBVM_STACK_FRAME_RECORD_TYPE_LANDING_PAD,
        .keepStackTrace = true
    };

    sysbvm_stackFrame_enterContext(context, (sysbvm_stackFrameRecord_t*)&topLevelFrame);
    int exitCode = 0;
    if(!_setjmp(topLevelFrame.jmpbuffer))
    {
        doMain(startArgumentIndex, argc, argv);
    }
    else
    {
        sysbvm_tuple_t errorString = sysbvm_tuple_asString(context, topLevelFrame.exception);
        fprintf(stderr, "Unhandled exception: " SYSBVM_STRING_PRINTF_FORMAT "\n", SYSBVM_STRING_PRINTF_ARG(errorString));
        sysbvm_stackFrame_printStackTrace(context, topLevelFrame.stackTrace);
        exitCode = 1;
    }

    sysbvm_stackFrame_leaveContext();
    return exitCode;
}

int main(int argc, const char *argv[])
{
    sysbvm_contextCreationOptions_t contextOptions = {0};

    // Allow creating the context by loading it from an image.
    int startArgumentIndex = 1;
    if(argc >= 3 && !strcmp(argv[1], "-load-image"))
    {
        context = sysbvm_context_loadImageFromFileNamed(argv[2]);
        startArgumentIndex = 3;
    }
    else
    {
        for(int i = 1; i < argc; ++i)
        {
            if(!strcmp(argv[i], "-m32"))
                contextOptions.targetWordSize = 4;
            else if(!strcmp(argv[i], "-m64"))
                contextOptions.targetWordSize = 8;
            else if(!strcmp(argv[i], "-nojit"))
                contextOptions.nojit = true;
            else if(!strcmp(argv[i], "-nogc"))
                contextOptions.gcType = SYSBVM_GC_TYPE_DISABLED;
            else if(!strcmp(argv[i], "-moving-gc"))
                contextOptions.gcType = SYSBVM_GC_TYPE_MOVING;
            else if(!strcmp(argv[i], "-non-moving-gc"))
                contextOptions.gcType = SYSBVM_GC_TYPE_NON_MOVING;
        }

        context = sysbvm_context_createWithOptions(&contextOptions);
    }

    if(!context)
    {
        fprintf(stderr, "Failed to create sysbvm context.\n");
        return 1;
    }

    int exitCode = mainWithContext(startArgumentIndex, argc, argv);

    // Allow saving the context as an image.
    if(destinationImageFilename)
        sysbvm_context_saveImageToFileNamed(context, destinationImageFilename);
    sysbvm_context_destroy(context);
    
    return exitCode;
}
