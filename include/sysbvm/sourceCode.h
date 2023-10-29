#ifndef SYSBVM_SOURCE_CODE_H
#define SYSBVM_SOURCE_CODE_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_sourceCode_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t text;
    sysbvm_tuple_t directory;
    sysbvm_tuple_t name;
    sysbvm_tuple_t language;
    sysbvm_tuple_t isInFile;
    sysbvm_tuple_t textSize;
    sysbvm_tuple_t lineStartIndexTable;
} sysbvm_sourceCode_t;

/**
 * Creates a source code with the given text and name.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_sourceCode_create(sysbvm_context_t *context, sysbvm_tuple_t text, sysbvm_tuple_t directory, sysbvm_tuple_t name, sysbvm_tuple_t language);

/**
 * Creates a source code with the given text and name, provided as C strings.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_sourceCode_createWithCStrings(sysbvm_context_t *context, const char *text, const char *directory, const char *name, const char *language);

/**
 * Infers a language from a specific source name.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_sourceCode_inferLanguageFromSourceName(sysbvm_context_t *context, sysbvm_tuple_t sourceName);

/**
 * Gets the corresponding line and column for the specified index.
 */
SYSBVM_API void sysbvm_sourceCode_computeLineAndColumnForIndex(sysbvm_context_t *context, sysbvm_tuple_t sourceCode, sysbvm_tuple_t index, uint32_t *outLine, uint32_t *outColumn);

/**
 * Gets the source code text.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_sourceCode_getText(sysbvm_tuple_t sourceCode);

/**
 * Gets the source code directory.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_sourceCode_getDirectory(sysbvm_tuple_t sourceCode);

/**
 * Gets the source code name.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_sourceCode_getName(sysbvm_tuple_t sourceCode);

/**
 * Gets the source code text.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_sourceCode_getLanguage(sysbvm_tuple_t sourceCode);

#endif //SYSBVM_SOURCE_CODE_H
