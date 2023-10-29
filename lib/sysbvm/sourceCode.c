#include "sysbvm/sourceCode.h"
#include "sysbvm/string.h"
#include "sysbvm/array.h"
#include "sysbvm/orderedCollection.h"
#include "internal/context.h"

SYSBVM_API sysbvm_tuple_t sysbvm_sourceCode_create(sysbvm_context_t *context, sysbvm_tuple_t text, sysbvm_tuple_t directory, sysbvm_tuple_t name, sysbvm_tuple_t language)
{
    sysbvm_sourceCode_t *result = (sysbvm_sourceCode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.sourceCodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_sourceCode_t));
    result->text = text;
    result->directory = directory;
    result->name = name;
    result->language = language;
    result->isInFile = directory ? SYSBVM_TRUE_TUPLE : SYSBVM_FALSE_TUPLE;
    result->textSize = sysbvm_tuple_size_encode(context, sysbvm_tuple_getSizeInBytes(result->text));
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_sourceCode_createWithCStrings(sysbvm_context_t *context, const char *text, const char *directory, const char *name, const char *language)
{
    return sysbvm_sourceCode_create(context, sysbvm_string_createWithCString(context, text), sysbvm_string_createWithCString(context, directory), sysbvm_string_createWithCString(context, name), sysbvm_symbol_internWithCString(context, language));
}

SYSBVM_API sysbvm_tuple_t sysbvm_sourceCode_inferLanguageFromSourceName(sysbvm_context_t *context, sysbvm_tuple_t sourceName)
{
    if(sysbvm_string_endsWithCString(sourceName, ".sysmel"))
        return sysbvm_symbol_internWithCString(context, "sysmel");
    return sysbvm_symbol_internWithCString(context, "tlisp");
}

static sysbvm_tuple_t sysbvm_sourceCode_ensureLineStartIndexTableIsBuilt(sysbvm_context_t *context, sysbvm_tuple_t sourceCode)
{
    sysbvm_sourceCode_t *sourceCodeObject = (sysbvm_sourceCode_t*)sourceCode;
    if(sourceCodeObject->lineStartIndexTable)
        return sourceCodeObject->lineStartIndexTable;

    sysbvm_tuple_t orderedCollection = sysbvm_orderedCollection_create(context);
    size_t sourceCodeTextSize = sysbvm_tuple_getSizeInBytes(sourceCodeObject->text);
    uint8_t *sourceCodeData = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(sourceCodeObject->text)->bytes;

    sysbvm_orderedCollection_add(context, orderedCollection, sysbvm_tuple_uint32_encode(context, 0));
    for(size_t i = 0; i < sourceCodeTextSize; ++i)
    {
        if(sourceCodeData[i] == '\n')
            sysbvm_orderedCollection_add(context, orderedCollection, sysbvm_tuple_uint32_encode(context, i + 1));
    }

    sourceCodeObject->lineStartIndexTable = sysbvm_orderedCollection_asArray(context, orderedCollection);
    return sourceCodeObject->lineStartIndexTable;
}

SYSBVM_API void sysbvm_sourceCode_computeLineAndColumnForIndex(sysbvm_context_t *context, sysbvm_tuple_t sourceCode, sysbvm_tuple_t index, uint32_t *outLine, uint32_t *outColumn)
{
    // Make sure the line start index table is built.
    sysbvm_tuple_t indexTable = sysbvm_sourceCode_ensureLineStartIndexTableIsBuilt(context, sourceCode);

    // Perform a binary search.
    size_t indexTableSize = sysbvm_array_getSize(indexTable);
    size_t indexValue = sysbvm_tuple_uint32_decode(index);

    size_t left = 0;
    size_t right = indexTableSize;
    size_t bestSoFar = 0;
    size_t bestIndexSoFar = 0;
    while(left < right)
    {
        size_t middle = left + (right - left) / 2;
        size_t middleIndex = sysbvm_tuple_uint32_decode(sysbvm_array_at(indexTable, middle));
        if(middleIndex <= indexValue)
        {
            bestSoFar = middle;
            bestIndexSoFar = middleIndex;
            left = middle + 1;
        }
        else if(middleIndex > indexValue)
        {
            right = middle;
        }
    }

    size_t line = bestSoFar + 1;
    size_t column = indexValue - bestIndexSoFar + 1;

    // Emit the result.
    if(outLine)
        *outLine = line;
    if(outColumn)
        *outColumn = column;
}

SYSBVM_API sysbvm_tuple_t sysbvm_sourceCode_getText(sysbvm_tuple_t sourceCode)
{
    return ((sysbvm_sourceCode_t*)sourceCode)->text;
}

SYSBVM_API sysbvm_tuple_t sysbvm_sourceCode_getDirectory(sysbvm_tuple_t sourceCode)
{
    return ((sysbvm_sourceCode_t*)sourceCode)->directory;
}

SYSBVM_API sysbvm_tuple_t sysbvm_sourceCode_getName(sysbvm_tuple_t sourceCode)
{
    return ((sysbvm_sourceCode_t*)sourceCode)->name;
}

SYSBVM_API sysbvm_tuple_t sysbvm_sourceCode_getLanguage(sysbvm_tuple_t sourceCode)
{
    return ((sysbvm_sourceCode_t*)sourceCode)->language;
}
