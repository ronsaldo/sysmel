#include "sysbvm/integer.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/string.h"
#include "sysbvm/type.h"
#include "internal/context.h"
#include <string.h>
#include <stdio.h>

typedef struct sysbvm_decoded_integer_s
{
    bool isNegative;
    size_t wordCount;
    uint32_t *words;
    uint32_t inlineWordBuffer[2];
} sysbvm_decoded_integer_t;

static void sysbvm_integer_decodeLargeOrImmediate(sysbvm_context_t *context, sysbvm_decoded_integer_t *decodedInteger, sysbvm_tuple_t integer)
{
    if(sysbvm_tuple_isImmediate(integer))
    {
        // Decode the small integer.
        sysbvm_stuple_t decodedSmall = sysbvm_tuple_integer_decodeSmall(integer);
        uint64_t decodedSmallPositive = decodedSmall >= 0 ? (uint64_t)decodedSmall : (uint64_t)(-decodedSmall);

        decodedInteger->isNegative = decodedSmall < 0;
        decodedInteger->inlineWordBuffer[0] = (uint32_t)decodedSmallPositive;
        decodedInteger->inlineWordBuffer[1] = (uint32_t)(decodedSmallPositive >> 32);
        decodedInteger->words = decodedInteger->inlineWordBuffer;
        decodedInteger->wordCount = decodedInteger->inlineWordBuffer[1] == 0 ? 1 : 2;
    }
    else
    {
        // Decode the large integer.
        sysbvm_integer_t *integerObject = (sysbvm_integer_t*)integer;
        decodedInteger->isNegative = sysbvm_tuple_getType(context, integer) == context->roots.largeNegativeIntegerType;
        decodedInteger->words = integerObject->words;
        decodedInteger->wordCount = sysbvm_tuple_getSizeInBytes(integer) / 4;

        // Normalize the word count.
        while(decodedInteger->wordCount > 1 && decodedInteger->words[decodedInteger->wordCount - 1] == 0)
            --decodedInteger->wordCount;
    }
}

static int sysbvm_integer_compareMagnitudes(size_t leftWordCount, uint32_t *leftWords, size_t rightWordCount, uint32_t *rightWords)
{
    size_t wordsToCompare = leftWordCount > rightWordCount ? leftWordCount : rightWordCount;
    for(size_t i = 0; i < wordsToCompare; ++i)
    {
        size_t wordIndex = wordsToCompare - i - 1;
        uint32_t leftWord = wordIndex < leftWordCount ? leftWords[wordIndex] : 0;
        uint32_t rightWord = wordIndex < rightWordCount ? rightWords[wordIndex] : 0;
        if(leftWord < rightWord)
            return -1;
        else if(leftWord > rightWord)
            return 1;
    }

    return 0;
}

static uint32_t sysbvm_integer_sumInto(size_t leftWordCount, uint32_t *leftWords, size_t rightWordCount, uint32_t *rightWords, size_t resultWordCount, uint32_t *resultWords)
{
    uint32_t carry = 0;
    
    size_t wordsToSum = leftWordCount > rightWordCount ? leftWordCount : rightWordCount;
    for(size_t i = 0; i < wordsToSum; ++i)
    {
        uint32_t leftWord = i < leftWordCount ? leftWords[i] : 0;
        uint32_t rightWord = i < rightWordCount ? rightWords[i] : 0;

        uint64_t sum = (uint64_t)leftWord + (uint64_t)rightWord + (uint64_t)carry;
        uint32_t sumWord = (uint32_t)sum;
        if(i < resultWordCount)
            resultWords[i] = sumWord;

        carry = (uint32_t)(sum >> 32);
    }

    for(size_t i = wordsToSum; i < resultWordCount; ++i)
    {
        if(i < resultWordCount)
            resultWords[i] = carry;
        carry = 0;
    }

    return carry;
}

static void sysbvm_integer_multiplyByWordAddingInto(size_t leftWordCount, uint32_t *leftWords, uint32_t word, size_t resultWordCount, uint32_t *resultWords)
{
    uint32_t carry = 0;
    size_t wordCount = leftWordCount > resultWordCount ? leftWordCount : resultWordCount;
    for(size_t i = 0; i < wordCount; ++i)
    {
        uint32_t leftWord = i < leftWordCount ? leftWords[i] : 0;
        uint32_t resultWord = i < resultWordCount ? resultWords[i] : 0;

        uint64_t multiplication = (uint64_t)leftWord*(uint64_t)word + (uint64_t)resultWord + (uint64_t)carry;
        resultWords[i] = (uint32_t)multiplication;
        carry = (uint32_t)(multiplication >> 32);
    }
}

static void sysbvm_integer_multiplyInto(size_t leftWordCount, uint32_t *leftWords, size_t rightWordCount, uint32_t *rightWords, size_t resultWordCount, uint32_t *resultWords)
{
    for(size_t i = 0; i < resultWordCount; ++i)
        resultWords[i] = 0;

    for(size_t i = 0; i < rightWordCount && i < resultWordCount; ++i)
        sysbvm_integer_multiplyByWordAddingInto(leftWordCount, leftWords, rightWords[i], resultWordCount - i, resultWords + i);
}

static int32_t sysbvm_integer_subtractFromInto(size_t leftWordCount, uint32_t *leftWords, size_t rightWordCount, uint32_t *rightWords, size_t resultWordCount, uint32_t *resultWords)
{
    int32_t borrow = 0;
    
    size_t wordsToSum = leftWordCount > rightWordCount ? leftWordCount : rightWordCount;
    for(size_t i = 0; i < wordsToSum; ++i)
    {
        uint32_t leftWord = i < leftWordCount ? leftWords[i] : 0;
        uint32_t rightWord = i < rightWordCount ? rightWords[i] : 0;

        int64_t subtraction = (int64_t)leftWord - (int64_t)rightWord + (int64_t)borrow;
        uint32_t subtractionWord = (uint32_t)subtraction;
        if(i < resultWordCount)
            resultWords[i] = subtractionWord;

        borrow = (int32_t)(subtraction >> 32);
    }

    for(size_t i = wordsToSum; i < resultWordCount; ++i)
    {
        if(i < resultWordCount)
            resultWords[i] = 0;
    }

    return borrow;
}

static sysbvm_tuple_t sysbvm_integer_normalize(sysbvm_context_t *context, sysbvm_integer_t *integer)
{
    // Counte the required number of normalized words.
    size_t wordCount = sysbvm_tuple_getSizeInBytes((sysbvm_tuple_t)integer) / 4;
    size_t normalizedWordCount = wordCount;
    while(normalizedWordCount > 0 && integer->words[normalizedWordCount - 1])
        --normalizedWordCount;

    // Trivial cases.
    if(normalizedWordCount == wordCount)
    {
        return (sysbvm_tuple_t)integer;
    }
    else if(normalizedWordCount == 0)
    {
        return sysbvm_tuple_integer_encodeSmall(0);
    }
    
    // Attempt to fit in an immediate, if possible.
    sysbvm_tuple_t largeIntegerType = sysbvm_tuple_getType(context, (sysbvm_tuple_t)integer);
    bool isNegative = largeIntegerType == context->roots.largeNegativeIntegerType;
    if(normalizedWordCount <= 2)
    {
        uint64_t valueUInt64 = (uint64_t)integer->words[0] | ((uint64_t)integer->words[1] << 32);
        if(isNegative)
        {
            if(valueUInt64 <= (uint64_t)-SYSBVM_IMMEDIATE_INT_MIN)
                return (sysbvm_tuple_t)sysbvm_tuple_integer_encodeSmall(-(sysbvm_stuple_t)valueUInt64);
        }
        else
        {
            if(valueUInt64 <= SYSBVM_IMMEDIATE_INT_MAX)
                return (sysbvm_tuple_t)sysbvm_tuple_integer_encodeSmall((sysbvm_stuple_t)valueUInt64);
        }
    }

    // Make a newer smaller large integer.
    sysbvm_integer_t *normalizedResult = (sysbvm_integer_t*)sysbvm_context_allocateByteTuple(context, largeIntegerType, normalizedWordCount*4);
    memcpy(normalizedResult->words, integer->words, 4*normalizedWordCount);
    return (sysbvm_tuple_t)normalizedWordCount;
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_integer_encodeBigInt32(sysbvm_context_t *context, int32_t value)
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
    if(SYSBVM_IMMEDIATE_INT_MIN <= value && value <= SYSBVM_IMMEDIATE_INT_MAX)
        return sysbvm_tuple_integer_encodeSmall(value);

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    sysbvm_integer_t *result = (sysbvm_integer_t*)sysbvm_context_allocateByteTuple(context, value >= 0 ? context->roots.largePositiveIntegerType : context->roots.largeNegativeIntegerType, 4);
    if(value >= 0)
        result->words[0] = (uint32_t)value;
    else
        result->words[0] = (uint32_t)(-value);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_integer_encodeBigUInt32(sysbvm_context_t *context, uint32_t value)
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
    if(value <= SYSBVM_IMMEDIATE_INT_MAX)
        return sysbvm_tuple_integer_encodeSmall(value);

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    sysbvm_integer_t *result = (sysbvm_integer_t*)sysbvm_context_allocateByteTuple(context, context->roots.largePositiveIntegerType, 4);
    result->words[0] = (uint32_t)value;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_integer_encodeBigInt64(sysbvm_context_t *context, int64_t value)
{
    if(SYSBVM_IMMEDIATE_INT_MIN <= value && value <= SYSBVM_IMMEDIATE_INT_MAX)
        return sysbvm_tuple_integer_encodeSmall((sysbvm_stuple_t)value);

    uint64_t positiveValue = value >= 0 ? (uint64_t)value : (uint64_t)(-value);

    if(positiveValue <= UINT32_MAX)
    {
        sysbvm_integer_t *result = (sysbvm_integer_t*)sysbvm_context_allocateByteTuple(context, value >= 0 ? context->roots.largePositiveIntegerType : context->roots.largeNegativeIntegerType, 4);
        result->words[0] = (uint32_t)value;
        return (sysbvm_tuple_t)result;
    }
    else
    {
        sysbvm_integer_t *result = (sysbvm_integer_t*)sysbvm_context_allocateByteTuple(context, value >= 0 ? context->roots.largePositiveIntegerType : context->roots.largeNegativeIntegerType, 8);
        result->words[0] = (uint32_t)value;
        result->words[1] = (uint32_t)(value >> 32);
        return (sysbvm_tuple_t)result;
    }
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_integer_encodeBigUInt64(sysbvm_context_t *context, uint64_t value)
{
    if(value <= SYSBVM_IMMEDIATE_INT_MAX)
        return sysbvm_tuple_integer_encodeSmall((sysbvm_stuple_t)value);

    if(value <= UINT32_MAX)
    {
        sysbvm_integer_t *result = (sysbvm_integer_t*)sysbvm_context_allocateByteTuple(context, context->roots.largePositiveIntegerType, 4);
        result->words[0] = (uint32_t)value;
        return (sysbvm_tuple_t)result;
    }
    else
    {
        sysbvm_integer_t *result = (sysbvm_integer_t*)sysbvm_context_allocateByteTuple(context, context->roots.largePositiveIntegerType, 8);
        result->words[0] = (uint32_t)value;
        result->words[1] = (uint32_t)(value >> 32);
        return (sysbvm_tuple_t)result;
    }
}

int32_t sysbvm_tuple_integer_decodeInt32(sysbvm_context_t *context, sysbvm_tuple_t value)
{
    if(sysbvm_tuple_isImmediate(value))
        return (int32_t)sysbvm_tuple_integer_decodeSmall(value);

    if(sysbvm_tuple_getSizeInBytes(value) < 4)
        return 0;

    sysbvm_integer_t *integer = (sysbvm_integer_t *)value;
    int32_t decodedValue = integer->words[0];
    if(sysbvm_tuple_getType(context, value) == context->roots.largeNegativeIntegerType)
        decodedValue = -decodedValue;
    return decodedValue;
}

uint32_t sysbvm_tuple_integer_decodeUInt32(sysbvm_context_t *context, sysbvm_tuple_t value)
{
    return (uint32_t)sysbvm_tuple_integer_decodeInt32(context, value);
}

int64_t sysbvm_tuple_integer_decodeInt64(sysbvm_context_t *context, sysbvm_tuple_t value)
{
    if(sysbvm_tuple_isImmediate(value))
        return sysbvm_tuple_integer_decodeSmall(value);

    size_t byteSize = sysbvm_tuple_getSizeInBytes(value);
    if(byteSize < 4)
        return 0;

    sysbvm_integer_t *integer = (sysbvm_integer_t *)value;
    int64_t decodedValue = 0;
    if(byteSize >= 8)
        decodedValue = (uint64_t)integer->words[0] | ((uint64_t)integer->words[1] << 32);
    else
        decodedValue = (uint64_t)integer->words[0];
    
    if(sysbvm_tuple_getType(context, value) == context->roots.largeNegativeIntegerType)
        decodedValue = -decodedValue;
    return decodedValue;
}

uint64_t sysbvm_tuple_integer_decodeUInt64(sysbvm_context_t *context, sysbvm_tuple_t value)
{
    return (uint64_t)sysbvm_tuple_integer_decodeInt64(context, value);
}

/**
 * Parses an integer from a string.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_integer_parseString(sysbvm_context_t *context, size_t stringSize, const char *string)
{
    size_t index = 0;
    bool isNegative = false;

    // Parse the sign.
    if(index < stringSize)
    {
        if(string[index] == '-')
        {
            isNegative = true;
            ++index;
        }
        else if(string[index] == '+')
        {
            ++index;
        }
    }

    sysbvm_tuple_t result = sysbvm_tuple_integer_encodeSmall(0);
    sysbvm_tuple_t radix = sysbvm_tuple_integer_encodeSmall(10);
    bool canParseRadix = true;
    for(; index < stringSize; ++index)
    {
        char digit = string[index];
        if('0' <= digit && digit <= '9')
        {
            sysbvm_tuple_t digitValue = sysbvm_tuple_integer_encodeSmall(digit - '0');
            if(sysbvm_integer_greaterEquals(context, digitValue, radix))
                sysbvm_error("Digit value is beyond the radix.");

            result = sysbvm_integer_multiply(context, result, radix);
            result = sysbvm_integer_add(context, result, digitValue);
        }
        else if(!canParseRadix && 'a' <= digit && digit <= 'z')
        {
            sysbvm_tuple_t digitValue = sysbvm_tuple_integer_encodeSmall(digit - 'a' + 10);
            if(sysbvm_integer_greaterEquals(context, digitValue, radix))
                sysbvm_error("Digit value is beyond the radix.");
            result = sysbvm_integer_multiply(context, result, radix);
            result = sysbvm_integer_add(context, result, digitValue);
        }
        else if(!canParseRadix && 'A' <= digit && digit <= 'Z')
        {
            sysbvm_tuple_t digitValue = sysbvm_tuple_integer_encodeSmall(digit - 'A' + 10);
            if(sysbvm_integer_greaterEquals(context, digitValue, radix))
                sysbvm_error("Digit value is beyond the radix.");
            result = sysbvm_integer_multiply(context, result, radix);
            result = sysbvm_integer_add(context, result, digitValue);
        }
        else if('_' == digit)
        {
            // Ignore it
        }
        else if(canParseRadix && ('r' == digit || 'r' == digit))
        {
            // We just saw the radix.
            radix = result;
            result = sysbvm_tuple_integer_encodeSmall(0);
            canParseRadix = false;
        }
        else
        {
            sysbvm_error("Invalid integer literal.");
        }
    }

    if(isNegative)
        return sysbvm_integer_negated(context, result);

    return result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_integer_parseCString(sysbvm_context_t *context, const char *cstring)
{
    return sysbvm_integer_parseString(context, strlen(cstring), cstring);
}

SYSBVM_API sysbvm_tuple_t sysbvm_integer_add(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right)
{
    if(sysbvm_tuple_isImmediate(left) && sysbvm_tuple_isImmediate(right))
    {
        sysbvm_stuple_t leftValue = sysbvm_tuple_integer_decodeSmall(left);
        sysbvm_stuple_t rightValue = sysbvm_tuple_integer_decodeSmall(right);
        return sysbvm_tuple_integer_encodeInt64(context, leftValue + rightValue);
    }

    sysbvm_decoded_integer_t decodedLeftInteger = {0};
    sysbvm_decoded_integer_t decodedRightInteger = {0};
    sysbvm_integer_decodeLargeOrImmediate(context, &decodedLeftInteger, left);
    sysbvm_integer_decodeLargeOrImmediate(context, &decodedRightInteger, right);

    if(decodedLeftInteger.isNegative == decodedRightInteger.isNegative)
    {
        // Same sign, sum
        uint32_t carry = sysbvm_integer_sumInto(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words, 0, NULL);
        size_t resultWordCount = (decodedLeftInteger.wordCount > decodedRightInteger.wordCount) ? decodedLeftInteger.wordCount : decodedRightInteger.wordCount;
        if(carry != 0)
            ++resultWordCount;

        sysbvm_integer_t *result = (sysbvm_integer_t*)sysbvm_context_allocateByteTuple(context, decodedLeftInteger.isNegative ? context->roots.largeNegativeIntegerType : context->roots.largePositiveIntegerType, resultWordCount*4);
        sysbvm_integer_sumInto(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words, resultWordCount, result->words);
        return (sysbvm_tuple_t)result;
    }
    else
    {
        // Different sign, subtract.
        int magnitudeComparison = sysbvm_integer_compareMagnitudes(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words);
        if(magnitudeComparison == 0)
            return sysbvm_tuple_integer_encodeSmall(0);

        if(magnitudeComparison > 0)
        {
            size_t resultWordCount = decodedLeftInteger.wordCount;
            bool resultIsNegative = decodedLeftInteger.isNegative;
            sysbvm_integer_t *result = (sysbvm_integer_t*)sysbvm_context_allocateByteTuple(context, resultIsNegative ? context->roots.largeNegativeIntegerType : context->roots.largePositiveIntegerType, resultWordCount*4);
            sysbvm_integer_subtractFromInto(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words, resultWordCount, result->words);
            return sysbvm_integer_normalize(context, result);
        }
        else
        {
            size_t resultWordCount = decodedRightInteger.wordCount;
            bool resultIsNegative = decodedRightInteger.isNegative;
            sysbvm_integer_t *result = (sysbvm_integer_t*)sysbvm_context_allocateByteTuple(context, resultIsNegative ? context->roots.largeNegativeIntegerType : context->roots.largePositiveIntegerType, resultWordCount*4);
            sysbvm_integer_subtractFromInto(decodedRightInteger.wordCount, decodedRightInteger.words, decodedLeftInteger.wordCount, decodedLeftInteger.words, resultWordCount, result->words);
            return sysbvm_integer_normalize(context, result);
        }
    }
}

SYSBVM_API sysbvm_tuple_t sysbvm_integer_subtract(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right)
{
    if(sysbvm_tuple_isImmediate(left) && sysbvm_tuple_isImmediate(right))
    {
        sysbvm_stuple_t leftValue = sysbvm_tuple_integer_decodeSmall(left);
        sysbvm_stuple_t rightValue = sysbvm_tuple_integer_decodeSmall(right);
        return sysbvm_tuple_integer_encodeInt64(context, leftValue - rightValue);
    }

    sysbvm_decoded_integer_t decodedLeftInteger = {0};
    sysbvm_decoded_integer_t decodedRightInteger = {0};
    sysbvm_integer_decodeLargeOrImmediate(context, &decodedLeftInteger, left);
    sysbvm_integer_decodeLargeOrImmediate(context, &decodedRightInteger, right);

    if(decodedLeftInteger.isNegative == decodedRightInteger.isNegative)
    {
        // Same sign, subtract.
        int magnitudeComparison = sysbvm_integer_compareMagnitudes(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words);
        if(magnitudeComparison == 0)
            return sysbvm_tuple_integer_encodeSmall(0);

        if(magnitudeComparison > 0)
        {
            size_t resultWordCount = decodedLeftInteger.wordCount;
            bool resultIsNegative = decodedLeftInteger.isNegative;
            sysbvm_integer_t *result = (sysbvm_integer_t*)sysbvm_context_allocateByteTuple(context, resultIsNegative ? context->roots.largeNegativeIntegerType : context->roots.largePositiveIntegerType, resultWordCount*4);
            sysbvm_integer_subtractFromInto(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words, resultWordCount, result->words);
            return sysbvm_integer_normalize(context, result);
        }
        else
        {
            size_t resultWordCount = decodedRightInteger.wordCount;
            bool resultIsNegative = decodedRightInteger.isNegative;
            sysbvm_integer_t *result = (sysbvm_integer_t*)sysbvm_context_allocateByteTuple(context, resultIsNegative ? context->roots.largeNegativeIntegerType : context->roots.largePositiveIntegerType, resultWordCount*4);
            sysbvm_integer_subtractFromInto(decodedRightInteger.wordCount, decodedRightInteger.words, decodedLeftInteger.wordCount, decodedLeftInteger.words, resultWordCount, result->words);
            return sysbvm_integer_normalize(context, result);
        }
    }
    else
    {
        // Differing sign, sum
        uint32_t carry = sysbvm_integer_sumInto(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words, 0, NULL);
        size_t resultWordCount = (decodedLeftInteger.wordCount > decodedRightInteger.wordCount) ? decodedLeftInteger.wordCount : decodedRightInteger.wordCount;
        if(carry != 0)
            ++resultWordCount;

        sysbvm_integer_t *result = (sysbvm_integer_t*)sysbvm_context_allocateByteTuple(context, decodedLeftInteger.isNegative ? context->roots.largeNegativeIntegerType : context->roots.largePositiveIntegerType, resultWordCount*4);
        sysbvm_integer_sumInto(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words, resultWordCount, result->words);
        return (sysbvm_tuple_t)result;
    }
}

SYSBVM_API sysbvm_tuple_t sysbvm_integer_negated(sysbvm_context_t *context, sysbvm_tuple_t integer)
{
    if(sysbvm_tuple_isImmediate(integer))
    {
        sysbvm_stuple_t integerValue = sysbvm_tuple_integer_decodeSmall(integer);
        return sysbvm_tuple_integer_encodeInt64(context, -integerValue);
    }

    // Shallow copy and swap the integer type.
    sysbvm_tuple_t negatedInteger = sysbvm_context_shallowCopy(context, integer);
    sysbvm_tuple_t integerType = sysbvm_tuple_getType(context, negatedInteger);
    sysbvm_tuple_setType(SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(negatedInteger), integerType == context->roots.largePositiveIntegerType ? context->roots.largeNegativeIntegerType : context->roots.largePositiveIntegerType);
    return sysbvm_integer_normalize(context, (sysbvm_integer_t*)negatedInteger);
}

SYSBVM_API sysbvm_tuple_t sysbvm_integer_multiply(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right)
{
    if(sysbvm_tuple_isImmediate(left) && sysbvm_tuple_isImmediate(right))
    {
        sysbvm_stuple_t leftValue = sysbvm_tuple_integer_decodeSmall(left);
        sysbvm_stuple_t rightValue = sysbvm_tuple_integer_decodeSmall(right);

        sysbvm_stuple_t result = leftValue * rightValue;
        if(leftValue == 0 || rightValue == 0 || leftValue == result / rightValue)
            return sysbvm_tuple_integer_encodeInt64(context, leftValue * rightValue);
    }

    sysbvm_decoded_integer_t decodedLeftInteger = {0};
    sysbvm_decoded_integer_t decodedRightInteger = {0};
    sysbvm_integer_decodeLargeOrImmediate(context, &decodedLeftInteger, left);
    sysbvm_integer_decodeLargeOrImmediate(context, &decodedRightInteger, right);

    bool resultIsNegative = decodedLeftInteger.isNegative != decodedRightInteger.isNegative;
    size_t resultWordCount = decodedLeftInteger.wordCount + decodedRightInteger.wordCount + 1;
    sysbvm_integer_t *result = (sysbvm_integer_t*)sysbvm_context_allocateByteTuple(context, resultIsNegative ? context->roots.largeNegativeIntegerType : context->roots.largePositiveIntegerType, resultWordCount*4);
    sysbvm_integer_multiplyInto(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words, resultWordCount, result->words);
    return sysbvm_integer_normalize(context, result);
}

SYSBVM_API sysbvm_tuple_t sysbvm_integer_divide(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right)
{
    (void)context;
    if(sysbvm_tuple_isImmediate(left) && sysbvm_tuple_isImmediate(right))
    {
        sysbvm_stuple_t leftValue = sysbvm_tuple_integer_decodeSmall(left);
        sysbvm_stuple_t rightValue = sysbvm_tuple_integer_decodeSmall(right);
        return sysbvm_tuple_integer_encodeSmall(leftValue / rightValue);
    }

    return SYSBVM_NULL_TUPLE;
}

SYSBVM_API sysbvm_tuple_t sysbvm_integer_remainder(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right)
{
    (void)context;
    if(sysbvm_tuple_isImmediate(left) && sysbvm_tuple_isImmediate(right))
    {
        sysbvm_stuple_t leftValue = sysbvm_tuple_integer_decodeSmall(left);
        sysbvm_stuple_t rightValue = sysbvm_tuple_integer_decodeSmall(right);
        return sysbvm_tuple_integer_encodeSmall(leftValue % rightValue);
    }

    return SYSBVM_NULL_TUPLE;
}

SYSBVM_API int sysbvm_integer_compare(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right)
{
    if(sysbvm_tuple_isImmediate(left) && sysbvm_tuple_isImmediate(right))
    {
        sysbvm_stuple_t leftValue = sysbvm_tuple_integer_decodeSmall(left);
        sysbvm_stuple_t rightValue = sysbvm_tuple_integer_decodeSmall(right);
        if(leftValue < rightValue)
            return -1;
        else if(leftValue > rightValue)
            return 1;
        else
            return 0;
    }

    sysbvm_decoded_integer_t decodedLeftInteger = {0};
    sysbvm_decoded_integer_t decodedRightInteger = {0};
    sysbvm_integer_decodeLargeOrImmediate(context, &decodedLeftInteger, left);
    sysbvm_integer_decodeLargeOrImmediate(context, &decodedRightInteger, right);

    // Check the differing signs.
    if(decodedLeftInteger.isNegative != decodedRightInteger.isNegative)
    {
        if(decodedLeftInteger.isNegative)
            return -1;
        else
            return 1;
    }

    // Check the word count.
    int magnitudeComparison = sysbvm_integer_compareMagnitudes(decodedLeftInteger.wordCount, decodedLeftInteger.words, decodedRightInteger.wordCount, decodedRightInteger.words);
    if(decodedLeftInteger.isNegative)
        magnitudeComparison = -magnitudeComparison;

    return magnitudeComparison;
}

SYSBVM_API sysbvm_tuple_t sysbvm_integer_factorial(sysbvm_context_t *context, sysbvm_tuple_t integer)
{
    sysbvm_tuple_t one = sysbvm_tuple_integer_encodeSmall(1);
    if(sysbvm_integer_lessEquals(context, integer, one))
        return one;
    
    sysbvm_tuple_t n1 = sysbvm_integer_factorial(context, sysbvm_integer_subtract(context, integer, sysbvm_tuple_integer_encodeSmall(1)));
    return sysbvm_integer_multiply(context, n1, integer);
}

SYSBVM_API bool sysbvm_integer_equals(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right)
{
    return sysbvm_integer_compare(context, left, right) == 0;
}

SYSBVM_API bool sysbvm_integer_notEquals(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right)
{
    return sysbvm_integer_compare(context, left, right) != 0;
}

SYSBVM_API bool sysbvm_integer_lessThan(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right)
{
    return sysbvm_integer_compare(context, left, right) < 0;
}

SYSBVM_API bool sysbvm_integer_lessEquals(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right)
{
    return sysbvm_integer_compare(context, left, right) <= 0;
}

SYSBVM_API bool sysbvm_integer_greaterThan(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right)
{
    return sysbvm_integer_compare(context, left, right) > 0;
}

SYSBVM_API bool sysbvm_integer_greaterEquals(sysbvm_context_t *context, sysbvm_tuple_t left, sysbvm_tuple_t right)
{
    return sysbvm_integer_compare(context, left, right) >= 0;
}

SYSBVM_API sysbvm_tuple_t sysbvm_integer_printString(sysbvm_context_t *context, sysbvm_tuple_t integer)
{
    // Decode the small integer.
    if(sysbvm_tuple_isImmediate(integer))
    {
        sysbvm_stuple_t value = sysbvm_tuple_integer_decodeSmall(integer);
        char buffer[64];
        size_t bufferSize = 0;

        // Work with positive integers.
        bool isNegative = false;
        if(value < 0)
        {
            isNegative = true;
            value = -value;
        }

        // Extract each one of the digits.
        while (value != 0 || bufferSize == 0)
        {
            buffer[bufferSize++] = '0' + (value % 10);
            value /= 10;
        }

        // Add the sign.
        if(isNegative)
            buffer[bufferSize++] = '-';
        buffer[bufferSize] = 0;
        return sysbvm_string_createWithReversedString(context, bufferSize, buffer);
    }

    return sysbvm_string_createWithCString(context, "TODO: sysbvm_integer_printString for large integer.");
}

SYSBVM_API sysbvm_tuple_t sysbvm_integer_toHexString(sysbvm_context_t *context, sysbvm_tuple_t integer)
{
    sysbvm_decoded_integer_t decodedInteger = {0};
    sysbvm_integer_decodeLargeOrImmediate(context, &decodedInteger, integer);
    
    // Count the required nibbles.
    size_t nibbleCount = 1; // We require at least one nibble.
    for(size_t i = 0; i < decodedInteger.wordCount; ++i)
    {
        uint32_t word = decodedInteger.words[i];
        for(uint32_t j = 0; j < 8; ++j)
        {
            size_t nibbleIndex = i*8 + j;
            if(((word >> (j*4)) & 0xF) != 0)
                nibbleCount = nibbleIndex + 1;
        }
    }

    // Compute the character count.
    size_t characterCount = nibbleCount + (decodedInteger.isNegative ? 1 : 0);
    
    sysbvm_tuple_t resultString = sysbvm_string_createEmptyWithSize(context, characterCount);
    uint8_t *resultBytes = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(resultString)->bytes;
    if(decodedInteger.isNegative)
    {
        resultBytes[0] = '-';
        ++resultBytes;
        --characterCount;
    }

    // Emit the hex characters
    for(size_t i = 0; i < decodedInteger.wordCount; ++i)
    {
        uint32_t word = decodedInteger.words[i];
        for(uint32_t j = 0; j < 8; ++j)
        {
            size_t nibbleIndex = i*8 + j;
            if(nibbleIndex >= nibbleCount)
                break;

            uint32_t nibble = (word >> (j*4)) & 0xF;
            uint8_t nibbleCharacter = 0;
            if(nibble < 10)
                nibbleCharacter = (uint8_t) ('0' + nibble);
            else
                nibbleCharacter = (uint8_t) ('A' + nibble - 10);

            resultBytes[nibbleCount - nibbleIndex - 1] = nibbleCharacter;
        }
    }

    return resultString;
}

static sysbvm_tuple_t sysbvm_integer_primitive_parseString(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);
    if(!sysbvm_tuple_isNonNullPointer(arguments[1])) return sysbvm_tuple_integer_encodeSmall(0);
    return sysbvm_integer_parseString(context, sysbvm_tuple_getSizeInBytes(arguments[1]), (const char *)SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(arguments[1])->bytes);
}

static sysbvm_tuple_t sysbvm_integer_primitive_printString(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);
    return sysbvm_integer_printString(context, arguments[0]);
}

static sysbvm_tuple_t sysbvm_integer_primitive_add(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_integer_add(context, arguments[0], arguments[1]);
}

static sysbvm_tuple_t sysbvm_integer_primitive_subtract(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_integer_subtract(context, arguments[0], arguments[1]);
}

static sysbvm_tuple_t sysbvm_integer_primitive_negated(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_integer_negated(context, arguments[0]);
}

static sysbvm_tuple_t sysbvm_integer_primitive_multiply(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_integer_multiply(context, arguments[0], arguments[1]);
}

static sysbvm_tuple_t sysbvm_integer_primitive_divide(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_integer_divide(context, arguments[0], arguments[1]);
}

static sysbvm_tuple_t sysbvm_integer_primitive_remainder(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_integer_remainder(context, arguments[0], arguments[1]);
}

static sysbvm_tuple_t sysbvm_integer_primitive_compare(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_tuple_integer_encodeSmall(sysbvm_integer_compare(context, arguments[0], arguments[1]));
}

static sysbvm_tuple_t sysbvm_integer_primitive_equals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_tuple_boolean_encode(sysbvm_integer_equals(context, arguments[0], arguments[1]));
}

static sysbvm_tuple_t sysbvm_integer_primitive_notEquals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_tuple_boolean_encode(sysbvm_integer_notEquals(context, arguments[0], arguments[1]));
}

static sysbvm_tuple_t sysbvm_integer_primitive_lessThan(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_tuple_boolean_encode(sysbvm_integer_lessThan(context, arguments[0], arguments[1]));
}

static sysbvm_tuple_t sysbvm_integer_primitive_lessEquals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_tuple_boolean_encode(sysbvm_integer_lessEquals(context, arguments[0], arguments[1]));
}

static sysbvm_tuple_t sysbvm_integer_primitive_greaterThan(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_tuple_boolean_encode(sysbvm_integer_greaterThan(context, arguments[0], arguments[1]));
}

static sysbvm_tuple_t sysbvm_integer_primitive_greaterEquals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_tuple_boolean_encode(sysbvm_integer_greaterEquals(context, arguments[0], arguments[1]));
}

static sysbvm_tuple_t sysbvm_integer_primitive_factorial(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_integer_factorial(context, arguments[0]);
}

void sysbvm_integer_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_parseString, "Integer::parseString:");
    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_printString, "Integer::printString");

    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_add, "Integer::+");
    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_subtract, "Integer::-");
    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_negated, "Integer::negated");
    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_multiply, "Integer::*");
    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_divide, "Integer::/");
    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_remainder, "Integer::%");

    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_compare, "Integer::<=>");
    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_equals, "Integer::=");
    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_notEquals, "Integer::~=");
    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_lessThan, "Integer::<");
    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_lessEquals, "Integer::<=");
    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_greaterThan, "Integer::>");
    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_greaterEquals, "Integer::>=");

    sysbvm_primitiveTable_registerFunction(sysbvm_integer_primitive_factorial, "Integer::factorial");
}

void sysbvm_integer_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_tuple_t printString = sysbvm_function_createPrimitive(context, 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_printString);
    sysbvm_type_setPrintStringFunction(context, context->roots.integerType, printString);
    sysbvm_type_setPrintStringFunction(context, context->roots.largePositiveIntegerType, printString);
    sysbvm_type_setPrintStringFunction(context, context->roots.largeNegativeIntegerType, printString);
    
    sysbvm_context_setIntrinsicPrimitiveMethod(context, sysbvm_tuple_getType(context, context->roots.integerType), "parseString:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_parseString);
    
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, "+", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_add);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, "-", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_subtract);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, "negated", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_negated);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, "*", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_multiply);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, "//", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_divide);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, "%", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_remainder);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, "<=>", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_compare);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, "=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_equals);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, "~=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_notEquals);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, "<", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_lessThan);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, "<=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_lessEquals);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, ">", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_greaterThan);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, ">=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_greaterEquals);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, "factorial", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_integer_primitive_factorial);
}
