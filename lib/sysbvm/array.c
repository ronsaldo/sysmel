#include "sysbvm/array.h"
#include "sysbvm/arraySlice.h"
#include "sysbvm/function.h"
#include "sysbvm/errors.h"
#include "sysbvm/type.h"
#include "internal/context.h"
#include <string.h>

SYSBVM_API sysbvm_tuple_t sysbvm_array_create(sysbvm_context_t *context, sysbvm_tuple_t slotCount)
{
    if(slotCount == 0)
    {
        sysbvm_type_tuple_t *arrayTypeObject = (sysbvm_type_tuple_t*)context->roots.arrayType;
        if(arrayTypeObject)
        {
            if(!arrayTypeObject->emptyTrivialSingleton)
                arrayTypeObject->emptyTrivialSingleton = (sysbvm_tuple_t)sysbvm_context_allocatePointerTuple(context, context->roots.arrayType, 0);
            return arrayTypeObject->emptyTrivialSingleton;
        }
    }

    return (sysbvm_tuple_t)sysbvm_context_allocatePointerTuple(context, context->roots.arrayType, slotCount);
}

SYSBVM_API sysbvm_tuple_t sysbvm_weakArray_create(sysbvm_context_t *context, sysbvm_tuple_t slotCount)
{
    if(slotCount == 0)
    {
        sysbvm_type_tuple_t *weakArrayTypeObject = (sysbvm_type_tuple_t*)context->roots.weakArrayType;
        if(weakArrayTypeObject)
        {
            if(!weakArrayTypeObject->emptyTrivialSingleton)
            {
                weakArrayTypeObject->emptyTrivialSingleton = (sysbvm_tuple_t)sysbvm_context_allocatePointerTuple(context, context->roots.weakArrayType, 0);
                sysbvm_tuple_markWeakObject(weakArrayTypeObject->emptyTrivialSingleton);
            }
            return weakArrayTypeObject->emptyTrivialSingleton;
        }
    }

    sysbvm_tuple_t result = (sysbvm_tuple_t)sysbvm_context_allocatePointerTuple(context, context->roots.weakArrayType, slotCount);
    sysbvm_tuple_markWeakObject(result);
    return result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_wordArray_create(sysbvm_context_t *context, sysbvm_tuple_t slotCount)
{
    if(slotCount == 0)
    {
        sysbvm_type_tuple_t *wordArrayTypeObject = (sysbvm_type_tuple_t*)context->roots.wordArrayType;
        if(wordArrayTypeObject)
        {
            if(!wordArrayTypeObject->emptyTrivialSingleton)
                wordArrayTypeObject->emptyTrivialSingleton = (sysbvm_tuple_t)sysbvm_context_allocateByteTuple(context, context->roots.wordArrayType, 0);
            return wordArrayTypeObject->emptyTrivialSingleton;
        }
    }

    return (sysbvm_tuple_t)sysbvm_context_allocateByteTuple(context, context->roots.wordArrayType, slotCount*4);
}

SYSBVM_API sysbvm_tuple_t sysbvm_byteArray_create(sysbvm_context_t *context, sysbvm_tuple_t size)
{
    if(size == 0)
    {
        sysbvm_type_tuple_t *byteArrayTypeObject = (sysbvm_type_tuple_t*)context->roots.byteArrayType;
        if(byteArrayTypeObject)
        {
            if(!byteArrayTypeObject->emptyTrivialSingleton)
                byteArrayTypeObject->emptyTrivialSingleton = (sysbvm_tuple_t)sysbvm_context_allocateByteTuple(context, context->roots.byteArrayType, 0);
            return byteArrayTypeObject->emptyTrivialSingleton;
        }
    }

    return (sysbvm_tuple_t)sysbvm_context_allocateByteTuple(context, context->roots.byteArrayType, size);
}

SYSBVM_API sysbvm_tuple_t sysbvm_array_asArraySlice(sysbvm_context_t *context, sysbvm_tuple_t array)
{
    if(!sysbvm_tuple_isNonNullPointer(array)) return SYSBVM_NULL_TUPLE;

    return sysbvm_arraySlice_createWithOffsetAndSize(context, array, 0, sysbvm_tuple_getSizeInSlots(array));
}

SYSBVM_API sysbvm_tuple_t sysbvm_array_getFirstElements(sysbvm_context_t *context, sysbvm_tuple_t array, size_t size)
{
    if(!sysbvm_tuple_isNonNullPointer(array)) return SYSBVM_NULL_TUPLE;

    size_t resultSize = sysbvm_tuple_getSizeInSlots(array);
    if(size < resultSize)
        resultSize = size;

    sysbvm_array_t *source = (sysbvm_array_t*)array;
    sysbvm_array_t *result = (sysbvm_array_t*)sysbvm_array_create(context, resultSize);
    for(size_t i = 0; i < resultSize; ++i)
        result->elements[i] = source->elements[i];

    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_arrayOrByteArray_at(sysbvm_tuple_t array, size_t index)
{
    if(!sysbvm_tuple_isNonNullPointer(array)) return SYSBVM_NULL_TUPLE;

    if(sysbvm_tuple_isBytes(array))
    {
        size_t size = sysbvm_tuple_getSizeInBytes(array);
        if(index >= size) sysbvm_error_indexOutOfBounds();
        return sysbvm_tuple_uint8_encode(SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(array)->bytes[index]);
    }
    else
    {
        size_t size = sysbvm_tuple_getSizeInSlots(array);
        if(index >= size) sysbvm_error_indexOutOfBounds();
        return SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(array)->pointers[index];
    }
}

SYSBVM_API void sysbvm_arrayOrByteArray_atPut(sysbvm_tuple_t array, size_t index, sysbvm_tuple_t value)
{
    if(!sysbvm_tuple_isNonNullPointer(array))
        return;

    if(sysbvm_tuple_isBytes(array))
    {
        size_t size = sysbvm_tuple_getSizeInBytes(array);
        if(index >= size)
            sysbvm_error_indexOutOfBounds();
        SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(array)->bytes[index] = sysbvm_tuple_uint8_decode(value);
    }
    else
    {
        size_t size = sysbvm_tuple_getSizeInSlots(array);
        if(index >= size)
            sysbvm_error_indexOutOfBounds();
        SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(array)->pointers[index] = value;
    }
}

SYSBVM_API sysbvm_tuple_t sysbvm_array_fromOffset(sysbvm_context_t *context, sysbvm_tuple_t array, size_t fromOffset)
{
    if(!sysbvm_tuple_isNonNullPointer(array)) return SYSBVM_NULL_TUPLE;
    
    size_t size = sysbvm_tuple_getSizeInSlots(array);
    if(fromOffset >= size)
        return sysbvm_array_create(context, 0);

    size_t resultSize = size - fromOffset;
    sysbvm_tuple_t result = sysbvm_array_create(context, resultSize);
    sysbvm_tuple_t *sourceElements = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(array)->pointers;
    sysbvm_tuple_t *resultElements = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(result)->pointers;
    for(size_t i = 0; i < resultSize; ++i)
        resultElements[i] = sourceElements[fromOffset + i];

    return result;
}

static sysbvm_tuple_t sysbvm_array_primitive_equals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);
    
    sysbvm_array_t **left = (sysbvm_array_t **)&arguments[0];
    sysbvm_array_t **right = (sysbvm_array_t **)&arguments[1];
    if(*left == *right)
        return SYSBVM_TRUE_TUPLE;
    if(sysbvm_tuple_getType(context, (sysbvm_tuple_t)*left) != sysbvm_tuple_getType(context, (sysbvm_tuple_t)*right))
        return SYSBVM_FALSE_TUPLE;

    size_t leftSize = sysbvm_array_getSize((sysbvm_tuple_t)*left);
    size_t rightSize = sysbvm_array_getSize((sysbvm_tuple_t)*right);
    if(leftSize != rightSize) return SYSBVM_FALSE_TUPLE;

    for(size_t i = 0; i < leftSize; ++i)
    {
        if(!sysbvm_tuple_equals(context, (*left)->elements[i], (*right)->elements[i]))
            return SYSBVM_FALSE_TUPLE;
    }

    return SYSBVM_TRUE_TUPLE;
}

static sysbvm_tuple_t sysbvm_array_primitive_hash(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_array_t **array = (sysbvm_array_t **)&arguments[0];
    size_t size = sysbvm_array_getSize((sysbvm_tuple_t)*array);

    size_t result = sysbvm_tuple_identityHash(sysbvm_tuple_getType(context, (sysbvm_tuple_t)*array));
    for(size_t i = 0; i < size; ++i)
        result = sysbvm_hashConcatenate(result, sysbvm_tuple_hash(context, (*array)->elements[i]));

    return sysbvm_tuple_size_encode(context, result);
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_copyFromUntil(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t startIndex = sysbvm_tuple_size_decode(arguments[1]);
    size_t endIndex = sysbvm_tuple_size_decode(arguments[2]);
    if(endIndex >= size)
        endIndex = size;
    
    size_t copySize = startIndex <= endIndex ? endIndex - startIndex : 0;
    sysbvm_tuple_t arrayType = sysbvm_tuple_getType(context, arguments[0]);

    if(copySize == 0)
    {
        sysbvm_type_tuple_t *arrayTypeObject = (sysbvm_type_tuple_t*)arrayType;
        if(!arrayTypeObject->emptyTrivialSingleton)
            arrayTypeObject->emptyTrivialSingleton = (sysbvm_tuple_t)sysbvm_context_allocateByteTuple(context, arrayType, 0);
        return arrayTypeObject->emptyTrivialSingleton;
    }

    sysbvm_byteArray_t *copy = (sysbvm_byteArray_t *)sysbvm_context_allocateByteTuple(context, arrayType, copySize);

    memcpy(copy->elements, array->elements + startIndex, copySize);
    return (sysbvm_tuple_t)copy;
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_replaceBytesFromWith(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 5) sysbvm_error_argumentCountMismatch(5, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t startIndex = sysbvm_tuple_size_decode(arguments[1]);
    size_t replacementSize = sysbvm_tuple_size_decode(arguments[2]);
    sysbvm_byteArray_t *replacement = (sysbvm_byteArray_t*)arguments[3];
    size_t replacementOffset = sysbvm_tuple_size_decode(arguments[4]);

    if(startIndex + replacementSize > sysbvm_tuple_getSizeInBytes(arguments[0])) sysbvm_error_outOfBoundsSlotAccess();
    if(replacementSize > sysbvm_tuple_getSizeInBytes(arguments[3])) sysbvm_error_outOfBoundsSlotAccess();

    memcpy(array->elements + startIndex, replacement->elements + replacementOffset, replacementSize);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_char8At(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 1 > size) sysbvm_error_outOfBoundsSlotAccess();

    return sysbvm_tuple_char8_encode(array->elements[index]);
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_uint8At(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 1 > size) sysbvm_error_outOfBoundsSlotAccess();

    return sysbvm_tuple_uint8_encode(array->elements[index]);
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_int8At(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 1 > size) sysbvm_error_outOfBoundsSlotAccess();

    return sysbvm_tuple_int8_encode(array->elements[index]);
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_char16At(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 2 > size) sysbvm_error_outOfBoundsSlotAccess();

    return sysbvm_tuple_char16_encode(*(uint16_t*)(array->elements + index));
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_uint16At(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 2 > size) sysbvm_error_outOfBoundsSlotAccess();

    return sysbvm_tuple_uint16_encode(*(uint16_t*)(array->elements + index));
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_int16At(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 2 > size) sysbvm_error_outOfBoundsSlotAccess();

    return sysbvm_tuple_int16_encode(*(int16_t*)(array->elements + index));
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_char32At(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 4 > size) sysbvm_error_outOfBoundsSlotAccess();

    return sysbvm_tuple_char32_encode(context, *(uint32_t*)(array->elements + index));
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_uint32At(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 4 > size) sysbvm_error_outOfBoundsSlotAccess();

    return sysbvm_tuple_uint32_encode(context, *(uint32_t*)(array->elements + index));
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_int32At(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 4 > size) sysbvm_error_outOfBoundsSlotAccess();

    return sysbvm_tuple_int32_encode(context, *(int32_t*)(array->elements + index));
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_uint64At(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 8 > size) sysbvm_error_outOfBoundsSlotAccess();

    return sysbvm_tuple_uint64_encode(context, *(uint64_t*)(array->elements + index));
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_int64At(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 8 > size) sysbvm_error_outOfBoundsSlotAccess();

    return sysbvm_tuple_int64_encode(context, *(int64_t*)(array->elements + index));
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_float32At(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 4 > size) sysbvm_error_outOfBoundsSlotAccess();

    float value = 0;
    memcpy(&value, array->elements + index, 4);
    return sysbvm_tuple_float32_encode(context, value);
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_float64At(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 8 > size) sysbvm_error_outOfBoundsSlotAccess();

    double value = 0;
    memcpy(&value, array->elements + index, 8);
    return sysbvm_tuple_float64_encode(context, value);
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_char8AtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 1 > size) sysbvm_error_outOfBoundsSlotAccess();

    array->elements[index] = sysbvm_tuple_char8_decode(arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_uint8AtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 1 > size) sysbvm_error_outOfBoundsSlotAccess();

    array->elements[index] = sysbvm_tuple_uint8_decode(arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_int8AtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 1 > size) sysbvm_error_outOfBoundsSlotAccess();

    array->elements[index] = sysbvm_tuple_int8_decode(arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_char16AtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 2 > size) sysbvm_error_outOfBoundsSlotAccess();

    *(uint16_t*)(array->elements + index) = sysbvm_tuple_char16_decode(arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_uint16AtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 2 > size) sysbvm_error_outOfBoundsSlotAccess();

    *(uint16_t*)(array->elements + index) = sysbvm_tuple_uint16_decode(arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_int16AtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 2 > size) sysbvm_error_outOfBoundsSlotAccess();

    *(int16_t*)(array->elements + index) = sysbvm_tuple_int16_decode(arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_char32AtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 4 > size) sysbvm_error_outOfBoundsSlotAccess();

    *(uint32_t*)(array->elements + index) = sysbvm_tuple_char32_decode(arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_uint32AtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 4 > size) sysbvm_error_outOfBoundsSlotAccess();

    *(uint32_t*)(array->elements + index) = sysbvm_tuple_uint32_decode(arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_int32AtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 4 > size) sysbvm_error_outOfBoundsSlotAccess();

    *(int32_t*)(array->elements + index) = sysbvm_tuple_int32_decode(arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_uint64AtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 8 > size) sysbvm_error_outOfBoundsSlotAccess();

    *(uint64_t*)(array->elements + index) = sysbvm_tuple_uint64_decode(arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_int64AtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 8 > size) sysbvm_error_outOfBoundsSlotAccess();

    *(int64_t*)(array->elements + index) = sysbvm_tuple_int64_decode(arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_float32AtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 4 > size) sysbvm_error_outOfBoundsSlotAccess();

    float value = sysbvm_tuple_float32_decode(arguments[2]);
    memcpy(array->elements + index, &value, 4);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_byteArray_primitive_float64AtPut(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_byteArray_t *array = (sysbvm_byteArray_t *)arguments[0];
    size_t size = sysbvm_tuple_getSizeInBytes(arguments[0]);
    size_t index = sysbvm_tuple_size_decode(arguments[1]);
    if(index + 8 > size) sysbvm_error_outOfBoundsSlotAccess();

    double value = sysbvm_tuple_float64_decode(arguments[2]);
    memcpy(array->elements + index, &value, 8);
    return SYSBVM_VOID_TUPLE;
}

void sysbvm_array_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_array_primitive_equals, "Array::=");
    sysbvm_primitiveTable_registerFunction(sysbvm_array_primitive_hash, "Array::hash");

    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_copyFromUntil, "ByteArray::copyFrom:until:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_replaceBytesFromWith, "ByteArray::replaceBytesFrom:count:with:startingAt:");

    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_char8At, "ByteArray::char8At:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_uint8At, "ByteArray::uint8At:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_int8At , "ByteArray::int8At:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_char16At, "ByteArray::char16At:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_uint16At, "ByteArray::uint16At:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_int16At , "ByteArray::int16At:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_char32At, "ByteArray::char32At:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_uint32At, "ByteArray::uint32At:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_int32At , "ByteArray::int32At:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_uint64At, "ByteArray::uint64At:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_int64At , "ByteArray::int64At:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_float32At , "ByteArray::float32At:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_float64At , "ByteArray::float64At:");

    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_char8AtPut, "ByteArray::char8At:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_uint8AtPut, "ByteArray::uint8At:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_int8AtPut , "ByteArray::int8At:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_char16AtPut, "ByteArray::char16At:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_uint16AtPut, "ByteArray::uint16At:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_int16AtPut , "ByteArray::int16At:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_char32AtPut, "ByteArray::char32At:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_uint32AtPut, "ByteArray::uint32At:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_int32AtPut , "ByteArray::int32At:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_uint64AtPut, "ByteArray::uint64At:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_int64AtPut , "ByteArray::int64At:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_float32AtPut , "ByteArray::float32At:put:");
    sysbvm_primitiveTable_registerFunction(sysbvm_byteArray_primitive_float64AtPut , "ByteArray::float64At:put:");
}

void sysbvm_array_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.arrayType, "=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_array_primitive_equals);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.arrayType, "hash", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_array_primitive_hash);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.byteArrayType, "copyFrom:until:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_copyFromUntil);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.byteArrayType, "replaceBytesFrom:count:with:startingAt:", 5, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_replaceBytesFromWith);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.stringType, "replaceBytesFrom:count:with:startingAt:", 5, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_replaceBytesFromWith);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__char8At__:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_char8At);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__uint8At__:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_uint8At);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__int8At__:" , 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_int8At);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__char16At__:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_char16At);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__uint16At__:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_uint16At);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__int16At__:" , 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_int16At);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__char32At__:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_char32At);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__uint32At__:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_uint32At);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__int32At__:" , 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_int32At);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__uint64At__:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_uint64At);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__int64At__:" , 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_int64At);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__float32At__:" , 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_float32At);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__float64At__:" , 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_float64At);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__char8At__:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_char8AtPut);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__uint8At__:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_uint8AtPut);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__int8At__:put:" , 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_int8AtPut);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__char16At__:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_char16AtPut);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__uint16At__:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_uint16AtPut);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__int16At__:put:" , 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_int16AtPut);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__char32At__:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_char32AtPut);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__uint32At__:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_uint32AtPut);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__int32At__:put:" , 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_int32AtPut);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__uint64At__:put:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_uint64AtPut);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__int64At__:put:" , 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_int64AtPut);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__float32At__:put:" , 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_float32AtPut);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.anyValueType, "__float64At__:put:" , 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_byteArray_primitive_float64AtPut);
}
