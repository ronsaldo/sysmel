#include "sysbvm/float.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/string.h"
#include "sysbvm/type.h"
#include "internal/context.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127)
#endif

SYSBVM_API float sysbvm_tuple_float32_decode(sysbvm_tuple_t tuple)
{
    if(sysbvm_tuple_isNonNullPointer(tuple))
    {
        sysbvm_object_tuple_t *floatObject = (sysbvm_object_tuple_t*)tuple;
        return *((sysbvm_float32_t*)floatObject->bytes);
    }
    else
    {
        uint32_t floatBits = (uint32_t)(tuple >> SYSBVM_TUPLE_TAG_BIT_COUNT);
        float result = 0;
        memcpy(&result, &floatBits, 4);
        return result;
    }
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_float32_encode(sysbvm_context_t *context, float value)
{
    if(sizeof(float) < sizeof(sysbvm_tuple_t))
    {
        uint32_t floatBits = 0;
        memcpy(&floatBits, &value, 4);
        return ((sysbvm_tuple_t)floatBits << SYSBVM_TUPLE_TAG_BIT_COUNT) | SYSBVM_TUPLE_TAG_FLOAT32;
    }
    else
    {
        sysbvm_object_tuple_t *result = sysbvm_context_allocateByteTuple(context, context->roots.float32Type, sizeof(sysbvm_float32_t));
        *((sysbvm_float32_t*)result->bytes) = value;
        return (sysbvm_tuple_t)result;
    }
}

SYSBVM_API sysbvm_tuple_t sysbvm_float32_parseCString(sysbvm_context_t *context, const char *cstring)
{
    return sysbvm_tuple_float32_encode(context, (float)atof(cstring));
}

SYSBVM_API sysbvm_tuple_t sysbvm_float32_parseString(sysbvm_context_t *context, size_t stringSize, const char *string)
{
    char *buffer = (char*)malloc(stringSize + 1);
    memcpy(buffer, string, stringSize);
    buffer[stringSize] = 0;

    sysbvm_tuple_t result = sysbvm_float32_parseCString(context, buffer);
    free(buffer);
    return result;
}

static sysbvm_tuple_t sysbvm_float32_primitive_parseString(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);
    if(!sysbvm_tuple_isNonNullPointer(arguments[1])) return sysbvm_tuple_integer_encodeSmall(0);
    return sysbvm_float32_parseString(context, sysbvm_tuple_getSizeInBytes(arguments[1]), (const char *)SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(arguments[1])->bytes);
}

SYSBVM_API double sysbvm_tuple_float64_decode(sysbvm_tuple_t tuple)
{
    if(sysbvm_tuple_isNonNullPointer(tuple))
    {
        sysbvm_object_tuple_t *floatObject = (sysbvm_object_tuple_t*)tuple;
        return *((sysbvm_float64_t*)floatObject->bytes);
    }
    else
    {
        // TODO: Use the Sista encoding (See: https://clementbera.wordpress.com/2018/11/09/64-bits-immediate-floats/)
        return 0.0;
    }
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_float64_encode(sysbvm_context_t *context, double value)
{
    // TODO: Use the Sista encoding (See: https://clementbera.wordpress.com/2018/11/09/64-bits-immediate-floats/)
    sysbvm_object_tuple_t *result = sysbvm_context_allocateByteTuple(context, context->roots.float64Type, sizeof(sysbvm_float64_t));
    *((sysbvm_float64_t*)result->bytes) = value;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_float64_parseCString(sysbvm_context_t *context, const char *cstring)
{
    return sysbvm_tuple_float64_encode(context, atof(cstring));
}

SYSBVM_API sysbvm_tuple_t sysbvm_float64_parseString(sysbvm_context_t *context, size_t stringSize, const char *string)
{
    char *buffer = (char*)malloc(stringSize + 1);
    memcpy(buffer, string, stringSize);
    buffer[stringSize] = 0;

    sysbvm_tuple_t result = sysbvm_float64_parseCString(context, buffer);
    free(buffer);
    return result;
}

static sysbvm_tuple_t sysbvm_float64_primitive_parseString(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);
    if(!sysbvm_tuple_isNonNullPointer(arguments[1])) return sysbvm_tuple_integer_encodeSmall(0);
    return sysbvm_float64_parseString(context, sysbvm_tuple_getSizeInBytes(arguments[1]), (const char *)SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(arguments[1])->bytes);
}

static sysbvm_tuple_t sysbvm_float32_primitive_printString(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    char buffer[32] = {0};
    sysbvm_float32_t value = sysbvm_tuple_float32_decode(arguments[0]);

    int stringSize = snprintf(buffer, sizeof(buffer), "%g", value);
    if(stringSize < 0)
        return sysbvm_string_createEmptyWithSize(context, 0);
    else
        return sysbvm_string_createWithString(context, stringSize, buffer);
}

static sysbvm_tuple_t sysbvm_float32_primitive_fromFloat64(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_float32_encode(context, (float)sysbvm_tuple_float64_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float32_primitive_add(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float32_t left = sysbvm_tuple_float32_decode(arguments[0]);
    sysbvm_float32_t right = sysbvm_tuple_float32_decode(arguments[1]);
    return sysbvm_tuple_float32_encode(context, left + right);
}

static sysbvm_tuple_t sysbvm_float32_primitive_subtract(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float32_t left = sysbvm_tuple_float32_decode(arguments[0]);
    sysbvm_float32_t right = sysbvm_tuple_float32_decode(arguments[1]);
    return sysbvm_tuple_float32_encode(context, left - right);
}

static sysbvm_tuple_t sysbvm_float32_primitive_negated(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_float32_t operand = sysbvm_tuple_float32_decode(arguments[0]);
    return sysbvm_tuple_float32_encode(context, -operand);
}

static sysbvm_tuple_t sysbvm_float32_primitive_sqrt(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_float32_t operand = sysbvm_tuple_float32_decode(arguments[0]);
    return sysbvm_tuple_float32_encode(context, sqrtf(operand));
}

static sysbvm_tuple_t sysbvm_float32_primitive_multiply(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float32_t left = sysbvm_tuple_float32_decode(arguments[0]);
    sysbvm_float32_t right = sysbvm_tuple_float32_decode(arguments[1]);
    return sysbvm_tuple_float32_encode(context, left * right);
}

static sysbvm_tuple_t sysbvm_float32_primitive_divide(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float32_t left = sysbvm_tuple_float32_decode(arguments[0]);
    sysbvm_float32_t right = sysbvm_tuple_float32_decode(arguments[1]);
    return sysbvm_tuple_float32_encode(context, left / right);
}

static sysbvm_tuple_t sysbvm_float32_primitive_floor(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_float32_encode(context, (float)floor(sysbvm_tuple_float32_decode(arguments[0])));
}

static sysbvm_tuple_t sysbvm_float32_primitive_ceiling(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_float32_encode(context, (float)ceil(sysbvm_tuple_float32_decode(arguments[0])));
}

static sysbvm_tuple_t sysbvm_float32_primitive_rounded(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_float32_encode(context, (float)round(sysbvm_tuple_float32_decode(arguments[0])));
}

static sysbvm_tuple_t sysbvm_float32_primitive_truncated(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_float32_encode(context, (float)trunc(sysbvm_tuple_float32_decode(arguments[0])));
}

static sysbvm_tuple_t sysbvm_float32_primitive_compare(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float32_t left = sysbvm_tuple_float32_decode(arguments[0]);
    sysbvm_float32_t right = sysbvm_tuple_float32_decode(arguments[1]);
    if(left < right)
        return sysbvm_tuple_integer_encodeSmall(-1);
    else if(left > right)
        return sysbvm_tuple_integer_encodeSmall(1);
    else
        return sysbvm_tuple_integer_encodeSmall(0);
}

static sysbvm_tuple_t sysbvm_float32_primitive_equals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float32_t left = sysbvm_tuple_float32_decode(arguments[0]);
    sysbvm_float32_t right = sysbvm_tuple_float32_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left == right);
}

static sysbvm_tuple_t sysbvm_float32_primitive_notEquals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float32_t left = sysbvm_tuple_float32_decode(arguments[0]);
    sysbvm_float32_t right = sysbvm_tuple_float32_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left != right);
}

static sysbvm_tuple_t sysbvm_float32_primitive_lessThan(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float32_t left = sysbvm_tuple_float32_decode(arguments[0]);
    sysbvm_float32_t right = sysbvm_tuple_float32_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left < right);
}

static sysbvm_tuple_t sysbvm_float32_primitive_lessEquals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float32_t left = sysbvm_tuple_float32_decode(arguments[0]);
    sysbvm_float32_t right = sysbvm_tuple_float32_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left <= right);
}

static sysbvm_tuple_t sysbvm_float32_primitive_greaterThan(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float32_t left = sysbvm_tuple_float32_decode(arguments[0]);
    sysbvm_float32_t right = sysbvm_tuple_float32_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left > right);
}

static sysbvm_tuple_t sysbvm_float32_primitive_greaterEquals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float32_t left = sysbvm_tuple_float32_decode(arguments[0]);
    sysbvm_float32_t right = sysbvm_tuple_float32_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left >= right);
}

static sysbvm_tuple_t sysbvm_float32_primitive_asUInt8(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint8_encode((uint8_t)sysbvm_tuple_float32_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asInt8(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int8_encode((int8_t)sysbvm_tuple_float32_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asChar8(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_char8_encode((sysbvm_char8_t)sysbvm_tuple_float32_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asUInt16(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint16_encode((uint16_t)sysbvm_tuple_float32_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asInt16(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int16_encode((int16_t)sysbvm_tuple_float32_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asChar16(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_char16_encode((sysbvm_char16_t)sysbvm_tuple_float32_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asUInt32(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint32_encode(context, (uint32_t)sysbvm_tuple_float32_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asInt32(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int32_encode(context, (int32_t)sysbvm_tuple_float32_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asChar32(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_char32_encode(context, (sysbvm_char32_t)sysbvm_tuple_float32_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asUInt64(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint64_encode(context, (uint64_t)sysbvm_tuple_float32_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asInt64(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int64_encode(context, (int64_t)sysbvm_tuple_float32_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asRoundedUInt8(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint8_encode((uint8_t)(sysbvm_tuple_float32_decode(arguments[0]) + 0.5f));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asRoundedInt8(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int8_encode((int8_t)(sysbvm_tuple_float32_decode(arguments[0]) + 0.5f));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asRoundedChar8(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_char8_encode((sysbvm_char8_t)(sysbvm_tuple_float32_decode(arguments[0]) + 0.5f));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asRoundedUInt16(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint16_encode((uint16_t)(sysbvm_tuple_float32_decode(arguments[0]) + 0.5f));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asRoundedInt16(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int16_encode((int16_t)(sysbvm_tuple_float32_decode(arguments[0]) + 0.5f));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asRoundedChar16(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_char16_encode((sysbvm_char16_t)(sysbvm_tuple_float32_decode(arguments[0]) + 0.5f));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asRoundedUInt32(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint32_encode(context, (uint32_t)(sysbvm_tuple_float32_decode(arguments[0]) + 0.5f));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asRoundedInt32(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int32_encode(context, (int32_t)(sysbvm_tuple_float32_decode(arguments[0]) + 0.5f));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asRoundedChar32(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_char32_encode(context, (sysbvm_char32_t)(sysbvm_tuple_float32_decode(arguments[0]) + 0.5f));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asRoundedUInt64(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint64_encode(context, (uint64_t)(sysbvm_tuple_float32_decode(arguments[0]) + 0.5f));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asRoundedInt64(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int64_encode(context, (int64_t)(sysbvm_tuple_float32_decode(arguments[0]) + 0.5f));
}

static sysbvm_tuple_t sysbvm_float32_primitive_asIEEEFloat32Decoded(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    uint32_t uint32Value = sysbvm_tuple_uint32_decode(arguments[0]);
    float floatValue = 0;
    memcpy(&floatValue, &uint32Value, 4);

    return sysbvm_tuple_float32_encode(context, floatValue);
}

static sysbvm_tuple_t sysbvm_float32_primitive_asIEEEFloat32Encoding(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    float floatValue = sysbvm_tuple_float32_decode(arguments[0]);
    uint32_t uint32Value = 0;
    memcpy(&uint32Value, &floatValue, 4);

    return sysbvm_tuple_uint32_encode(context, uint32Value);
}

static sysbvm_tuple_t sysbvm_float64_primitive_printString(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    char buffer[32] = {0};
    sysbvm_float64_t value = sysbvm_tuple_float64_decode(arguments[0]);

    int stringSize = snprintf(buffer, sizeof(buffer), "%g", value);
    if(stringSize < 0)
        return sysbvm_string_createEmptyWithSize(context, 0);
    else
        return sysbvm_string_createWithString(context, stringSize, buffer);
}

static sysbvm_tuple_t sysbvm_float64_primitive_fromFloat32(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_float64_encode(context, sysbvm_tuple_float32_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float64_primitive_add(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float64_t left = sysbvm_tuple_float64_decode(arguments[0]);
    sysbvm_float64_t right = sysbvm_tuple_float64_decode(arguments[1]);
    return sysbvm_tuple_float64_encode(context, left + right);
}

static sysbvm_tuple_t sysbvm_float64_primitive_subtract(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float64_t left = sysbvm_tuple_float64_decode(arguments[0]);
    sysbvm_float64_t right = sysbvm_tuple_float64_decode(arguments[1]);
    return sysbvm_tuple_float64_encode(context, left - right);
}

static sysbvm_tuple_t sysbvm_float64_primitive_negated(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_float64_t operand = sysbvm_tuple_float64_decode(arguments[0]);
    return sysbvm_tuple_float64_encode(context, -operand);
}

static sysbvm_tuple_t sysbvm_float64_primitive_sqrt(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_float64_t operand = sysbvm_tuple_float64_decode(arguments[0]);
    return sysbvm_tuple_float64_encode(context, sqrt(operand));
}

static sysbvm_tuple_t sysbvm_float64_primitive_multiply(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float64_t left = sysbvm_tuple_float64_decode(arguments[0]);
    sysbvm_float64_t right = sysbvm_tuple_float64_decode(arguments[1]);
    return sysbvm_tuple_float64_encode(context, left * right);
}

static sysbvm_tuple_t sysbvm_float64_primitive_divide(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float64_t left = sysbvm_tuple_float64_decode(arguments[0]);
    sysbvm_float64_t right = sysbvm_tuple_float64_decode(arguments[1]);
    return sysbvm_tuple_float64_encode(context, left / right);
}

static sysbvm_tuple_t sysbvm_float64_primitive_floor(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_float64_encode(context, floor(sysbvm_tuple_float32_decode(arguments[0])));
}

static sysbvm_tuple_t sysbvm_float64_primitive_ceiling(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_float64_encode(context, ceil(sysbvm_tuple_float32_decode(arguments[0])));
}

static sysbvm_tuple_t sysbvm_float64_primitive_rounded(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_float64_encode(context, round(sysbvm_tuple_float32_decode(arguments[0])));
}

static sysbvm_tuple_t sysbvm_float64_primitive_truncated(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_float64_encode(context, trunc(sysbvm_tuple_float32_decode(arguments[0])));
}

static sysbvm_tuple_t sysbvm_float64_primitive_compare(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float64_t left = sysbvm_tuple_float64_decode(arguments[0]);
    sysbvm_float64_t right = sysbvm_tuple_float64_decode(arguments[1]);
    if(left < right)
        return sysbvm_tuple_integer_encodeSmall(-1);
    else if(left > right)
        return sysbvm_tuple_integer_encodeSmall(1);
    else
        return sysbvm_tuple_integer_encodeSmall(0);
}

static sysbvm_tuple_t sysbvm_float64_primitive_equals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float64_t left = sysbvm_tuple_float64_decode(arguments[0]);
    sysbvm_float64_t right = sysbvm_tuple_float64_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left == right);
}

static sysbvm_tuple_t sysbvm_float64_primitive_notEquals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float64_t left = sysbvm_tuple_float64_decode(arguments[0]);
    sysbvm_float64_t right = sysbvm_tuple_float64_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left != right);
}

static sysbvm_tuple_t sysbvm_float64_primitive_lessThan(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float64_t left = sysbvm_tuple_float64_decode(arguments[0]);
    sysbvm_float64_t right = sysbvm_tuple_float64_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left < right);
}

static sysbvm_tuple_t sysbvm_float64_primitive_lessEquals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float64_t left = sysbvm_tuple_float64_decode(arguments[0]);
    sysbvm_float64_t right = sysbvm_tuple_float64_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left <= right);
}

static sysbvm_tuple_t sysbvm_float64_primitive_greaterThan(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float64_t left = sysbvm_tuple_float64_decode(arguments[0]);
    sysbvm_float64_t right = sysbvm_tuple_float64_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left > right);
}

static sysbvm_tuple_t sysbvm_float64_primitive_greaterEquals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_float64_t left = sysbvm_tuple_float64_decode(arguments[0]);
    sysbvm_float64_t right = sysbvm_tuple_float64_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left >= right);
}

static sysbvm_tuple_t sysbvm_float64_primitive_asUInt8(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint8_encode((uint8_t)sysbvm_tuple_float64_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asInt8(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int8_encode((int8_t)sysbvm_tuple_float64_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asChar8(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_char8_encode((sysbvm_char8_t)sysbvm_tuple_float64_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asUInt16(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint16_encode((uint16_t)sysbvm_tuple_float64_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asInt16(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int16_encode((int16_t)sysbvm_tuple_float64_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asChar16(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_char16_encode((sysbvm_char16_t)sysbvm_tuple_float64_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asUInt32(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint32_encode(context, (uint32_t)sysbvm_tuple_float64_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asInt32(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int32_encode(context, (int32_t)sysbvm_tuple_float64_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asChar32(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_char32_encode(context, (sysbvm_char32_t)sysbvm_tuple_float64_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asUInt64(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint64_encode(context, (uint64_t)sysbvm_tuple_float64_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asInt64(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int64_encode(context, (int64_t)sysbvm_tuple_float64_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asRoundedUInt8(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint8_encode((uint8_t)(sysbvm_tuple_float64_decode(arguments[0]) + 0.5));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asRoundedInt8(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int8_encode((int8_t)(sysbvm_tuple_float64_decode(arguments[0]) + 0.5));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asRoundedChar8(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_char8_encode((sysbvm_char8_t)(sysbvm_tuple_float64_decode(arguments[0]) + 0.5));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asRoundedUInt16(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint16_encode((uint16_t)(sysbvm_tuple_float64_decode(arguments[0]) + 0.5));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asRoundedInt16(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int16_encode((int16_t)(sysbvm_tuple_float64_decode(arguments[0]) + 0.5));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asRoundedChar16(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_char16_encode((sysbvm_char16_t)(sysbvm_tuple_float64_decode(arguments[0]) + 0.5));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asRoundedUInt32(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint32_encode(context, (uint32_t)(sysbvm_tuple_float64_decode(arguments[0]) + 0.5));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asRoundedInt32(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int32_encode(context, (int32_t)(sysbvm_tuple_float64_decode(arguments[0]) + 0.5));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asRoundedChar32(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_char32_encode(context, (sysbvm_char32_t)(sysbvm_tuple_float64_decode(arguments[0]) + 0.5));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asRoundedUInt64(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint64_encode(context, (uint64_t)(sysbvm_tuple_float64_decode(arguments[0]) + 0.5));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asRoundedInt64(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int64_encode(context, (int64_t)(sysbvm_tuple_float64_decode(arguments[0]) + 0.5));
}

static sysbvm_tuple_t sysbvm_float64_primitive_asIEEEFloat64Encoding(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    double floatValue = sysbvm_tuple_float64_decode(arguments[0]);
    uint64_t uint64Value = 0;
    memcpy(&uint64Value, &floatValue, 8);

    return sysbvm_tuple_uint64_encode(context, uint64Value);
}

static sysbvm_tuple_t sysbvm_float64_primitive_asIEEEFloat64Decoded(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    uint64_t uint64Value = sysbvm_tuple_uint64_decode(arguments[0]);
    double floatValue = 0;
    memcpy(&floatValue, &uint64Value, 8);

    return sysbvm_tuple_float64_encode(context, floatValue);
}

void sysbvm_float_registerPrimitives(void)
{
    // Float32
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_parseString, "Float32::parseString");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_printString, "Float32::printString");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_fromFloat64, "Float32::fromFloat64");

    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_add, "Float32::+");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_subtract, "Float32::-");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_negated, "Float32::negated");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_sqrt, "Float32::sqrt");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_multiply, "Float32::*");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_divide, "Float32::/");

    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_ceiling, "Float32::ceiling");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_floor, "Float32::floor");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_rounded, "Float32::rounded");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_truncated, "Float32::truncated");

    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_compare, "Float32::<=>");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_equals, "Float32::=");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_notEquals, "Float32::~=");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_lessThan, "Float32::<");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_lessEquals, "Float32::<=");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_greaterThan, "Float32::>");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_greaterEquals, "Float32::>=");

    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asUInt8,  "Float32::asUInt8");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asInt8,   "Float32::asInt8");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asChar8,  "Float32::asChar8");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asUInt16, "Float32::asUInt16");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asInt16,  "Float32::asInt16");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asChar16, "Float32::asChar16");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asUInt32, "Float32::asUInt32");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asInt32,  "Float32::asInt32");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asChar32, "Float32::asChar32");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asUInt64, "Float32::asUInt64");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asInt64,  "Float32::asInt64");

    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asRoundedUInt8,  "Float32::asRoundedUInt8");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asRoundedInt8,   "Float32::asRoundedInt8");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asRoundedChar8,  "Float32::asRoundedChar8");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asRoundedUInt16, "Float32::asRoundedUInt16");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asRoundedInt16,  "Float32::asRoundedInt16");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asRoundedChar16, "Float32::asRoundedChar16");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asRoundedUInt32, "Float32::asRoundedUInt32");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asRoundedInt32,  "Float32::asRoundedInt32");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asRoundedChar32, "Float32::asRoundedChar32");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asRoundedUInt64, "Float32::asRoundedUInt64");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asRoundedInt64,  "Float32::asRoundedInt64");

    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asIEEEFloat32Encoding, "Float32::asIEEEFloat32Encoding");
    sysbvm_primitiveTable_registerFunction(sysbvm_float32_primitive_asIEEEFloat32Decoded, "UInt32::asIEEEFloat32Decoded");

    // Float64
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_parseString, "Float64::parseString");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_printString, "Float64::printString");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_fromFloat32, "Float64::fromFloat32");

    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_add, "Float64::+");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_subtract, "Float64::-");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_negated, "Float64::negated");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_sqrt, "Float64::sqrt");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_multiply, "Float64::*");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_divide, "Float64::/");

    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_ceiling, "Float64::ceiling");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_floor, "Float64::floor");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_rounded, "Float64::rounded");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_truncated, "Float64::truncated");

    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_compare, "Float64::<=>");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_equals, "Float64::=");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_notEquals, "Float64::~=");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_lessThan, "Float64::<");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_lessEquals, "Float64::<=");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_greaterThan, "Float64::>");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_greaterEquals, "Float64::>=");

    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asUInt8,  "Float64::asUInt8");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asInt8,   "Float64::asInt8");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asChar8,  "Float64::asChar8");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asUInt16, "Float64::asUInt16");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asInt16,  "Float64::asInt16");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asChar16, "Float64::asChar16");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asUInt32, "Float64::asUInt32");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asInt32,  "Float64::asInt32");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asChar32, "Float64::asChar32");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asUInt64, "Float64::asUInt64");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asInt64,  "Float64::asInt64");

    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asRoundedUInt8,  "Float64::asRoundedUInt8");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asRoundedInt8,   "Float64::asRoundedInt8");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asRoundedChar8,  "Float64::asRoundedChar8");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asRoundedUInt16, "Float64::asRoundedUInt16");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asRoundedInt16,  "Float64::asRoundedInt16");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asRoundedChar16, "Float64::asRoundedChar16");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asRoundedUInt32, "Float64::asRoundedUInt32");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asRoundedInt32,  "Float64::asRoundedInt32");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asRoundedChar32, "Float64::asRoundedChar32");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asRoundedUInt64, "Float64::asRoundedUInt64");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asRoundedInt64,  "Float64::asRoundedInt64");

    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asIEEEFloat64Encoding, "Float64::asIEEEFloat64Encoding");
    sysbvm_primitiveTable_registerFunction(sysbvm_float64_primitive_asIEEEFloat64Decoded, "UInt64::asIEEEFloat64Decoded");
}

void sysbvm_float_setupPrimitives(sysbvm_context_t *context)
{
    // Float32
    sysbvm_type_setPrintStringFunction(context, context->roots.float32Type, sysbvm_function_createPrimitive(context, 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_printString));

    sysbvm_context_setIntrinsicPrimitiveMethod(context, sysbvm_tuple_getType(context, context->roots.float32Type), "parseString:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_parseString);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "f32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_fromFloat64);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "+", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_add);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "-", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_subtract);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "negated", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_negated);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "sqrt", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_sqrt);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "*", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_multiply);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "/", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_divide);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "floor", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_floor);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "ceiling", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_ceiling);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "rounded", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_rounded);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "truncated", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_truncated);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "<=>", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_compare);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_equals);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "~=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_notEquals);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "<", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_lessThan);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "<=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_lessEquals);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, ">", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_greaterThan);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, ">=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_greaterEquals);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asUInt8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asUInt8);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asInt8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asInt8);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asChar8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asChar8);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asUInt16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asUInt16);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asInt16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asInt16);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asChar16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asChar16);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asUInt32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asUInt32);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asInt32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asInt32);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asChar32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asChar32);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asUInt64", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asUInt64);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asInt64", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asInt64);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asRoundedUInt8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asRoundedUInt8);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asRoundedInt8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asRoundedInt8);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asRoundedChar8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asRoundedChar8);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asRoundedUInt16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asRoundedUInt16);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asRoundedInt16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asRoundedInt16);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asRoundedChar16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asRoundedChar16);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asRoundedUInt32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asRoundedUInt32);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asRoundedInt32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asRoundedInt32);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asRoundedChar32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asRoundedChar32);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asRoundedUInt64", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asRoundedUInt64);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asRoundedInt64", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asRoundedInt64);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asIEEEFloat32Encoding", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asIEEEFloat32Encoding);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.uint32Type, "asIEEEFloat32Decoded", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asIEEEFloat32Decoded);

    // Float64
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float32Type, "asFloat64", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_fromFloat32);

    sysbvm_type_setPrintStringFunction(context, context->roots.float64Type, sysbvm_function_createPrimitive(context, 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_printString));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, sysbvm_tuple_getType(context, context->roots.float64Type), "parseString:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_parseString);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "+", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_add);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "-", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_subtract);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "negated", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_negated);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "sqrt", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_sqrt);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "*", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_multiply);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "/", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_divide);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "floor", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_floor);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "ceiling", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_ceiling);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "rounded", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_rounded);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "truncated", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_truncated);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "<=>", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_compare);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_equals);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "~=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_notEquals);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "<", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_lessThan);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "<=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_lessEquals);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, ">", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_greaterThan);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, ">=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_greaterEquals);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asUInt8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asUInt8);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asInt8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asInt8);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asChar8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asChar8);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asUInt16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asUInt16);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asInt16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asInt16);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asChar16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asChar16);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asUInt32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asUInt32);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asInt32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asInt32);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asChar32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asChar32);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asUInt64", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asUInt64);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asInt64", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asInt64);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asRoundedUInt8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float32_primitive_asRoundedUInt8);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asRoundedInt8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asRoundedInt8);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asRoundedChar8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asRoundedChar8);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asRoundedUInt16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asRoundedUInt16);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asRoundedInt16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asRoundedInt16);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asRoundedChar16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asRoundedChar16);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asRoundedUInt32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asRoundedUInt32);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asRoundedInt32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asRoundedInt32);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asRoundedChar32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asRoundedChar32);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asRoundedUInt64", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asRoundedUInt64);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asRoundedInt64", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asRoundedInt64);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.float64Type, "asIEEEFloat64Encoding", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asIEEEFloat64Encoding);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.uint64Type, "asIEEEFloat64Decoded", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_float64_primitive_asIEEEFloat64Decoded);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
