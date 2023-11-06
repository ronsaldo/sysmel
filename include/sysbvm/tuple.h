#ifndef SYSBVM_TUPLE_H
#define SYSBVM_TUPLE_H

#pragma once

#ifdef MSC_VER
#endif

#include "common.h"
#include "hash.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4127)
#endif

typedef struct sysbvm_context_s sysbvm_context_t;

typedef uint16_t sysbvm_boolean16_t;
typedef uint32_t sysbvm_boolean32_t;
typedef uint64_t sysbvm_boolean64_t;

typedef uint8_t sysbvm_char8_t;
typedef uint16_t sysbvm_char16_t;
typedef uint32_t sysbvm_char32_t;

typedef float sysbvm_float32_t;
typedef double sysbvm_float64_t;

typedef size_t sysbvm_bitflags_t;
typedef intptr_t sysbvm_systemHandle_t;

typedef size_t sysbvm_size_t;
typedef uintptr_t sysbvm_uintptr_t;
typedef intptr_t sysbvm_intptr_t;

/**
 * A tuple object pointer.
 */
typedef uintptr_t sysbvm_tuple_t;

/**
 * A tuple object signed pointer.
 */
typedef intptr_t sysbvm_stuple_t;

#define SYSBVM_TUPLE_GC_COLOR_SHIFT 0
#define SYSBVM_TUPLE_GC_COLOR_BITS 2
#define SYSBVM_TUPLE_GC_COLOR_MASK (((1 << SYSBVM_TUPLE_GC_COLOR_BITS) - 1) << SYSBVM_TUPLE_GC_COLOR_SHIFT)

#define SYSBVM_TUPLE_FLAGS_SHIFT SYSBVM_TUPLE_GC_COLOR_BITS
#define SYSBVM_TUPLE_FLAGS_BITS 6
#define SYSBVM_TUPLE_FLAGS_DIRTY (1 << SYSBVM_TUPLE_FLAGS_SHIFT)
#define SYSBVM_TUPLE_FLAGS_YOUNG (1 << (SYSBVM_TUPLE_FLAGS_SHIFT + 1))
#define SYSBVM_TUPLE_FLAGS_RESERVED (1 << (SYSBVM_TUPLE_FLAGS_SHIFT + 2))
#define SYSBVM_TUPLE_FLAGS_IMMUTABLE (1 << (SYSBVM_TUPLE_FLAGS_SHIFT + 3))
#define SYSBVM_TUPLE_FLAGS_NEEDS_FINALIZATION (1 << (SYSBVM_TUPLE_FLAGS_SHIFT + 4))
#define SYSBVM_TUPLE_FLAGS_DUMMY_VALUE (1 << (SYSBVM_TUPLE_FLAGS_SHIFT + 5))

#define SYSBVM_TUPLE_OBJECT_KIND_SHIFT (SYSBVM_TUPLE_FLAGS_SHIFT + SYSBVM_TUPLE_FLAGS_BITS)
#define SYSBVM_TUPLE_OBJECT_KIND_BITS 2
#define SYSBVM_TUPLE_OBJECT_KIND_MASK (((1 << SYSBVM_TUPLE_OBJECT_KIND_BITS) - 1) << SYSBVM_TUPLE_OBJECT_KIND_SHIFT)

#define SYSBVM_TUPLE_IDENTITY_HASH_SHIFT 10
#define SYSBVM_TUPLE_IDENTITY_HASH_BITS 22
#define SYSBVM_TUPLE_IDENTITY_HASH_MASK (((1 << SYSBVM_TUPLE_IDENTITY_HASH_BITS) - 1) << SYSBVM_TUPLE_IDENTITY_HASH_SHIFT)

#define SYSBVM_TUPLE_TAG_BIT_COUNT 4
#define SYSBVM_TUPLE_TAG_BIT_MASK 15

#define SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(oop) ((sysbvm_object_tuple_t*)oop)

#define SYSBVM_TUPLE_BIT_COUNT ((sizeof(sysbvm_tuple_t)*8))
#define SYSBVM_IMMEDIATE_BIT_COUNT (SYSBVM_TUPLE_BIT_COUNT - SYSBVM_TUPLE_TAG_BIT_COUNT)
#define SYSBVM_TUPLE_IMMEDIATE_VALUE_BIT_MASK (((sysbvm_tuple_t)1 << SYSBVM_IMMEDIATE_BIT_COUNT) - 1)

#define SYSBVM_IMMEDIATE_INT_MIN ( -((sysbvm_stuple_t)1 << (sysbvm_stuple_t)(SYSBVM_IMMEDIATE_BIT_COUNT - 1)) )
#define SYSBVM_IMMEDIATE_INT_MAX ( ((sysbvm_stuple_t)1 << (sysbvm_stuple_t)(SYSBVM_IMMEDIATE_BIT_COUNT - 1)) - 1 )

typedef enum sysbvm_object_kind_e
{
    SYSBVM_TUPLE_OBJECT_KIND_POINTERS = 0,
    SYSBVM_TUPLE_OBJECT_KIND_WEAK_POINTERS = 1,
    SYSBVM_TUPLE_OBJECT_KIND_BYTES = 2,
    SYSBVM_TUPLE_OBJECT_KIND_COMPLEX = 3,
    SYSBVM_TUPLE_OBJECT_KIND_IMMEDIATE
} sysbvm_object_kind_t;

/**
 * The different immediate pointer tags.
 */ 
typedef enum sysbvm_tuple_tag_e
{
    SYSBVM_TUPLE_TAG_POINTER = 0,
    SYSBVM_TUPLE_TAG_NIL = 0,

    SYSBVM_TUPLE_TAG_INTEGER = 1,

    SYSBVM_TUPLE_TAG_CHAR8 = 2,
    SYSBVM_TUPLE_TAG_UINT8 = 3,
    SYSBVM_TUPLE_TAG_INT8 = 4,

    SYSBVM_TUPLE_TAG_CHAR16 = 5,
    SYSBVM_TUPLE_TAG_UINT16 = 6,
    SYSBVM_TUPLE_TAG_INT16 = 7,

    SYSBVM_TUPLE_TAG_CHAR32 = 8,
    SYSBVM_TUPLE_TAG_UINT32 = 9,
    SYSBVM_TUPLE_TAG_INT32 = 10,

    SYSBVM_TUPLE_TAG_UINT64 = 11,
    SYSBVM_TUPLE_TAG_INT64 = 12,

    SYSBVM_TUPLE_TAG_FLOAT32 = 13,
    SYSBVM_TUPLE_TAG_FLOAT64 = 14,

    SYSBVM_TUPLE_TAG_TRIVIAL = 15,

    SYSBVM_TUPLE_TAG_COUNT = 16
} sysbvm_tuple_tag_t;

/**
 * The indices for the different immediate trivial values.
 */ 
typedef enum sysbvm_tuple_immediate_trivial_index_e
{
    SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_FALSE = 0,
    SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_TRUE,
    SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_VOID,
    SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_HASHTABLE_EMPTY_ELEMENT,
    SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_TOMBSTONE,
    SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_PENDING_MEMOIZATION_VALUE,

    SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_COUNT,
} sysbvm_tuple_immediate_trivial_index_e;

#define SYSBVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(immediateTrivialIndex) ((immediateTrivialIndex << SYSBVM_TUPLE_TAG_BIT_COUNT) | SYSBVM_TUPLE_TAG_TRIVIAL)
/**
 * Tuple header that describes its content.
 */
typedef struct sysbvm_tuple_header_s
{
    union
    {
        sysbvm_tuple_t typePointer;

        struct
        {
            uint32_t typePointer32;
            uint32_t typePointer32Padding;
        };
        uint64_t typePointer64;
    };

    uint32_t identityHashAndFlags;
    uint32_t objectSize;
} sysbvm_tuple_header_t;

/**
 * A tuple can contain pointers and/or bytes.
 */
typedef struct sysbvm_object_tuple_s
{
    sysbvm_tuple_header_t header;

    union
    {
        sysbvm_tuple_t pointers[1];
        uint8_t bytes[1];
    };
} sysbvm_object_tuple_t;

#define SYSBVM_NULL_TUPLE ((sysbvm_tuple_t)0)
#define SYSBVM_FALSE_TUPLE SYSBVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_FALSE)
#define SYSBVM_TRUE_TUPLE SYSBVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_TRUE)
#define SYSBVM_VOID_TUPLE SYSBVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_VOID)
#define SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE SYSBVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_HASHTABLE_EMPTY_ELEMENT)
#define SYSBVM_TOMBSTONE_TUPLE SYSBVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_TOMBSTONE)
#define SYSBVM_PENDING_MEMOIZATION_VALUE SYSBVM_TUPLE_MAKE_IMMEDIATE_TRIVIAL_WITH_INDEX(SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_PENDING_MEMOIZATION_VALUE)

#define SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(structureType) ((sizeof(structureType) - sizeof(sysbvm_tuple_header_t)) / sizeof(sysbvm_tuple_t))
#define SYSBVM_BYTE_SIZE_FOR_STRUCTURE_TYPE(structureType) (sizeof(structureType) - sizeof(sysbvm_tuple_header_t))
#define SYSBVM_SLOT_INDEX_FOR_STRUCTURE_MEMBER(structureType, memberName) ((offsetof(structureType, memberName) - sizeof(sysbvm_tuple_header_t)) / sizeof(sysbvm_tuple_t))

#define SYSBVM_STRING_PRINTF_FORMAT "%.*s"
#define SYSBVM_STRING_PRINTF_ARG(arg) (int)sysbvm_tuple_getSizeInBytes(arg), SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(arg)->bytes

/**
 * Is this a pointer tuple?
 */
SYSBVM_INLINE bool sysbvm_tuple_isPointer(sysbvm_tuple_t tuple)
{
    return (tuple & SYSBVM_TUPLE_TAG_BIT_MASK) == SYSBVM_TUPLE_TAG_POINTER;
}

/**
 * Is this non-null pointer tuple?
 */
SYSBVM_INLINE bool sysbvm_tuple_isNonNullPointer(sysbvm_tuple_t tuple)
{
    return (tuple != 0) & ((tuple & SYSBVM_TUPLE_TAG_BIT_MASK) == SYSBVM_TUPLE_TAG_POINTER);
}

/**
 * Is this an immediate tuple?
 */
SYSBVM_INLINE bool sysbvm_tuple_isImmediate(sysbvm_tuple_t tuple)
{
    return (tuple & SYSBVM_TUPLE_TAG_BIT_MASK) != SYSBVM_TUPLE_TAG_POINTER;
}

/**
 * Gets the size in bytes of the specified tuple.
 */
SYSBVM_INLINE size_t sysbvm_tuple_getSizeInBytes(sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isNonNullPointer(tuple) ? SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.objectSize : 0;
}

/**
 * Gets the size in bytes of the specified tuple.
 */
SYSBVM_INLINE size_t sysbvm_tuple_getSizeInSlots(sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isNonNullPointer(tuple) ? SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.objectSize / sizeof(sysbvm_tuple_t) : 0;
}

/**
 * Retrieves an immediate tuple type corresponding to a specific tag.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_getImmediateTypeWithTag(sysbvm_context_t *context, size_t immediateTypeTag);

/**
 * Retrieves an immediate trivial tuple corresponding to a specific index.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_getImmediateTrivialTypeWithIndex(sysbvm_context_t *context, size_t immediateTrivialIndex);

/**
 * Sets the type of an object tuple
 */
SYSBVM_INLINE void sysbvm_tuple_setType(sysbvm_object_tuple_t *objectTuple, sysbvm_tuple_t newType)
{
    objectTuple->header.typePointer = newType;
}

/**
 * Gets the type tuple of the specified tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_pointerTuple_getType(sysbvm_tuple_t tuple)
{
    return SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.typePointer;
}

/**
 * Gets the type tuple of the specified tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_getType(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    if(sysbvm_tuple_isNonNullPointer(tuple))
    {
        return SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.typePointer;
    }
    else
    {
        size_t typeTag = tuple & SYSBVM_TUPLE_TAG_BIT_MASK;
        if(typeTag != SYSBVM_TUPLE_TAG_TRIVIAL)
            return sysbvm_tuple_getImmediateTypeWithTag(context, typeTag);
        else
            return sysbvm_tuple_getImmediateTrivialTypeWithIndex(context, tuple >> SYSBVM_TUPLE_TAG_BIT_COUNT);
    }
}

/**
 * Gets the tuple object kind.
 */
SYSBVM_INLINE sysbvm_object_kind_t sysbvm_tuple_getObjectKind(sysbvm_tuple_t tuple)
{
    if(sysbvm_tuple_isNonNullPointer(tuple))
        return (SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.identityHashAndFlags & SYSBVM_TUPLE_OBJECT_KIND_MASK) >> SYSBVM_TUPLE_OBJECT_KIND_SHIFT;
    else
        return SYSBVM_TUPLE_OBJECT_KIND_IMMEDIATE;
}

/**
 * Sets the tuple object kind.
 */
SYSBVM_INLINE void sysbvm_tuple_setObjectKind(sysbvm_tuple_t tuple, sysbvm_object_kind_t newKind)
{
    if(sysbvm_tuple_isNonNullPointer(tuple))
    {
        sysbvm_tuple_header_t *header = &SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header;
        header->identityHashAndFlags = (header->identityHashAndFlags & ~SYSBVM_TUPLE_OBJECT_KIND_MASK) | (newKind << SYSBVM_TUPLE_OBJECT_KIND_SHIFT);
    }
}

/**
 * Is this a bytes tuple?
 */
SYSBVM_INLINE bool sysbvm_tuple_isBytes(sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_getObjectKind(tuple) == SYSBVM_TUPLE_OBJECT_KIND_BYTES;
}

/**
 * Is this a weak object tuple?
 */
SYSBVM_INLINE bool sysbvm_tuple_isWeakObject(sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_getObjectKind(tuple) == SYSBVM_TUPLE_OBJECT_KIND_WEAK_POINTERS;
}

/**
 * Marks a weak object.
 */
SYSBVM_INLINE void sysbvm_tuple_markWeakObject(sysbvm_tuple_t tuple)
{
    if(sysbvm_tuple_isNonNullPointer(tuple))
        sysbvm_tuple_setObjectKind(tuple, SYSBVM_TUPLE_OBJECT_KIND_WEAK_POINTERS);
}

/**
 * Is this an immutable tuple?
 */
SYSBVM_INLINE bool sysbvm_tuple_isImmutable(sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isImmediate(tuple) || !tuple ||
        ((SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.identityHashAndFlags & SYSBVM_TUPLE_FLAGS_IMMUTABLE) != 0);
}

/**
 * Marks an immutable value.
 */
SYSBVM_INLINE void sysbvm_tuple_markImmutable(sysbvm_tuple_t tuple)
{
    if(sysbvm_tuple_isNonNullPointer(tuple))
        SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.identityHashAndFlags |= SYSBVM_TUPLE_FLAGS_IMMUTABLE;
}

/**
 * Is this an object that needs finalization?
 */
SYSBVM_INLINE bool sysbvm_tuple_isObjectThatNeedsFinalization(sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isNonNullPointer(tuple) && (SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.identityHashAndFlags & SYSBVM_TUPLE_FLAGS_NEEDS_FINALIZATION) != 0;
}

/**
 * Marks an object finalization.
 */
SYSBVM_INLINE void sysbvm_tuple_markObjectThatNeedsFinalization(sysbvm_tuple_t tuple)
{
    if(sysbvm_tuple_isNonNullPointer(tuple))
        SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.identityHashAndFlags |= SYSBVM_TUPLE_FLAGS_NEEDS_FINALIZATION;
}


/**
 * Is this a dummy value object?
 */
SYSBVM_INLINE bool sysbvm_tuple_isDummyValue(sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isNonNullPointer(tuple) && (SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.identityHashAndFlags & SYSBVM_TUPLE_FLAGS_DUMMY_VALUE) != 0;
}

/**
 * Marks a dummy value object.
 */
SYSBVM_INLINE void sysbvm_tuple_markDummyValue(sysbvm_tuple_t tuple)
{
    if(sysbvm_tuple_isNonNullPointer(tuple))
        SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.identityHashAndFlags |= SYSBVM_TUPLE_FLAGS_DUMMY_VALUE;
}

/**
 * Gets the GC color?
 */
SYSBVM_INLINE uint32_t sysbvm_tuple_getGCColor(sysbvm_tuple_t tuple)
{
    return (uint32_t)(SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->header.identityHashAndFlags & SYSBVM_TUPLE_GC_COLOR_MASK) >> SYSBVM_TUPLE_GC_COLOR_SHIFT;
}

/**
 * Gets the GC color?
 */
SYSBVM_INLINE void sysbvm_tuple_setGCColor(sysbvm_tuple_t tuple, uint32_t newColor)
{
    sysbvm_object_tuple_t *objectTuple = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple);
    objectTuple->header.identityHashAndFlags = (objectTuple->header.identityHashAndFlags & ~SYSBVM_TUPLE_GC_COLOR_MASK) | (newColor << SYSBVM_TUPLE_GC_COLOR_SHIFT);
}

/**
 * Decodes a small integer as an immediate tuple.
 */
SYSBVM_INLINE sysbvm_stuple_t sysbvm_tuple_integer_decodeSmall(sysbvm_tuple_t tuple)
{
    return ((sysbvm_stuple_t)tuple) >> SYSBVM_TUPLE_TAG_BIT_COUNT;
}

/**
 * Encodes a small integer as an immediate tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_integer_encodeSmall(sysbvm_stuple_t value)
{
    return (((sysbvm_stuple_t)value) << SYSBVM_TUPLE_TAG_BIT_COUNT) | SYSBVM_TUPLE_TAG_INTEGER;
}

/**
 * Decodes an integer as an int32
 */
SYSBVM_API int32_t sysbvm_tuple_integer_decodeInt32(sysbvm_context_t *context, sysbvm_tuple_t value);

/**
 * Decodes an integer as an uint32
 */
SYSBVM_API uint32_t sysbvm_tuple_integer_decodeUInt32(sysbvm_context_t *context, sysbvm_tuple_t value);

/**
 * Decodes an integer as an int64
 */
SYSBVM_API int64_t sysbvm_tuple_integer_decodeInt64(sysbvm_context_t *context, sysbvm_tuple_t value);

/**
 * Decodes an integer as an uint64
 */
SYSBVM_API uint64_t sysbvm_tuple_integer_decodeUInt64(sysbvm_context_t *context, sysbvm_tuple_t value);

/**
 * Encodes an int32 as an integer.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_integer_encodeBigInt32(sysbvm_context_t *context, int32_t value);

/**
 * Encodes an uint32 as an integer.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_integer_encodeBigUInt32(sysbvm_context_t *context, uint32_t value);

/**
 * Encodes an int64 as an integer.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_integer_encodeBigInt64(sysbvm_context_t *context, int64_t value);

/**
 * Encodes an uint64 as an integer.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_integer_encodeBigUInt64(sysbvm_context_t *context, uint64_t value);

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

/**
 * Encodes an integer as a tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_integer_encodeUInt32(sysbvm_context_t *context, uint32_t value)
{
    int32_t svalue = (int32_t)value;
    if(sizeof(uint32_t) < sizeof(sysbvm_stuple_t) || (SYSBVM_IMMEDIATE_INT_MIN <= svalue && svalue <= SYSBVM_IMMEDIATE_INT_MAX))
        return sysbvm_tuple_integer_encodeSmall((sysbvm_stuple_t)value);
    else
        return sysbvm_tuple_integer_encodeBigUInt32(context, value);
}

/**
 * Encodes an integer as a tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_integer_encodeInt32(sysbvm_context_t *context, int32_t value)
{
    if(sizeof(int32_t) < sizeof(sysbvm_stuple_t) || (SYSBVM_IMMEDIATE_INT_MIN <= value && value <= SYSBVM_IMMEDIATE_INT_MAX))
        return sysbvm_tuple_integer_encodeSmall((sysbvm_stuple_t)value);
    else
        return sysbvm_tuple_integer_encodeBigInt32(context, value);
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

/**
 * Encodes an integer as a tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_integer_encodeInt64(sysbvm_context_t *context, int64_t value)
{
    if(sizeof(int64_t) < sizeof(sysbvm_stuple_t) || (SYSBVM_IMMEDIATE_INT_MIN <= value && value <= SYSBVM_IMMEDIATE_INT_MAX))
        return sysbvm_tuple_integer_encodeSmall((sysbvm_stuple_t)value);
    else
        return sysbvm_tuple_integer_encodeBigInt64(context, value);
}

/**
 * Encodes an integer as a tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_integer_encodeUInt64(sysbvm_context_t *context, uint64_t value)
{
    int64_t svalue = (int64_t)value;
    if(sizeof(uint64_t) < sizeof(sysbvm_stuple_t) || (SYSBVM_IMMEDIATE_INT_MIN <= svalue && svalue <= SYSBVM_IMMEDIATE_INT_MAX))
        return sysbvm_tuple_integer_encodeSmall((sysbvm_stuple_t)value);
    else
        return sysbvm_tuple_integer_encodeBigUInt64(context, value);
}

/**
 * Encodes an integer as a tuple.
 */
SYSBVM_INLINE sysbvm_size_t sysbvm_tuple_integer_decodeSize(sysbvm_context_t *context, size_t value)
{
    if(sizeof(uint32_t) == sizeof(size_t))
        return sysbvm_tuple_integer_decodeUInt32(context, value);
    else
        return (sysbvm_size_t)sysbvm_tuple_integer_decodeUInt64(context, value);
}

/**
 * Encodes an integer as a tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_integer_encodeSize(sysbvm_context_t *context, size_t value)
{
    if(sizeof(uint32_t) == sizeof(size_t))
        return sysbvm_tuple_integer_encodeUInt32(context, (uint32_t)value);
    else
        return sysbvm_tuple_integer_encodeUInt64(context, value);
}

/**
 * Encodes a Char8 as an immediate tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_char8_encode(sysbvm_char8_t value)
{
    return ((sysbvm_stuple_t)value << SYSBVM_TUPLE_TAG_BIT_COUNT) | SYSBVM_TUPLE_TAG_CHAR8;
}

/**
 * Decodes a Char from an immediate tuple.
 */
SYSBVM_INLINE sysbvm_char8_t sysbvm_tuple_char8_decode(sysbvm_tuple_t tuple)
{
    return (sysbvm_char8_t)((sysbvm_stuple_t)tuple >> SYSBVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Encodes an UInt8 as an immediate tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_uint8_encode(uint8_t value)
{
    return ((sysbvm_stuple_t)value << SYSBVM_TUPLE_TAG_BIT_COUNT) | SYSBVM_TUPLE_TAG_UINT8;
}

/**
 * Decodes an UInt8 from an immediate tuple.
 */
SYSBVM_INLINE uint8_t sysbvm_tuple_uint8_decode(sysbvm_tuple_t tuple)
{
    return (uint8_t) ((sysbvm_stuple_t)tuple >> SYSBVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Encodes an Int8 as an immediate tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_int8_encode(int8_t value)
{
    return ((sysbvm_stuple_t)value << SYSBVM_TUPLE_TAG_BIT_COUNT) | SYSBVM_TUPLE_TAG_INT8;
}

/**
 * Decodes an Int8 from an immediate tuple.
 */
SYSBVM_INLINE int8_t sysbvm_tuple_int8_decode(sysbvm_tuple_t tuple)
{
    return (int8_t) ((sysbvm_stuple_t)tuple >> SYSBVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Encodes a Char16 as an immediate tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_char16_encode(sysbvm_char16_t value)
{
    return (((sysbvm_stuple_t)value) << SYSBVM_TUPLE_TAG_BIT_COUNT) | SYSBVM_TUPLE_TAG_CHAR16;
}

/**
 * Decodes a Char from an immediate tuple.
 */
SYSBVM_INLINE sysbvm_char16_t sysbvm_tuple_char16_decode(sysbvm_tuple_t tuple)
{
    return (sysbvm_char16_t)((sysbvm_stuple_t)tuple >> SYSBVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Encodes an UInt16 as an immediate tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_uint16_encode(uint16_t value)
{
    return ((sysbvm_stuple_t)value << SYSBVM_TUPLE_TAG_BIT_COUNT) | SYSBVM_TUPLE_TAG_UINT16;
}

/**
 * Decodes an UInt16 from an immediate tuple.
 */
SYSBVM_INLINE uint16_t sysbvm_tuple_uint16_decode(sysbvm_tuple_t tuple)
{
    return (uint16_t) ((sysbvm_stuple_t)tuple >> SYSBVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Encodes an Int16 as an immediate tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_int16_encode(int16_t value)
{
    return ((sysbvm_stuple_t)value << SYSBVM_TUPLE_TAG_BIT_COUNT) | SYSBVM_TUPLE_TAG_INT16;
}

/**
 * Decodes an Int16 from an immediate tuple.
 */
SYSBVM_INLINE int16_t sysbvm_tuple_int16_decode(sysbvm_tuple_t tuple)
{
    return (int16_t) ((sysbvm_stuple_t)tuple >> SYSBVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Encodes a Char32 as an immediate tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_char32_encodeSmall(sysbvm_char32_t value)
{
    return (((sysbvm_stuple_t)value) << SYSBVM_TUPLE_TAG_BIT_COUNT) | SYSBVM_TUPLE_TAG_CHAR32;
}

/**
 * Encodes a Char32 as a tuple.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_char32_encodeBig(sysbvm_context_t *context, sysbvm_char32_t value);

/**
 * Decodes a Char from an immediate tuple.
 */
SYSBVM_INLINE sysbvm_char32_t sysbvm_tuple_char32_decodeSmall(sysbvm_tuple_t tuple)
{
    return (sysbvm_char32_t)((sysbvm_stuple_t)tuple >> SYSBVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Decodes a Char32 from a tuple.
 */
SYSBVM_INLINE sysbvm_char32_t sysbvm_tuple_char32_decode(sysbvm_tuple_t tuple)
{
    if(sysbvm_tuple_isImmediate(tuple))
        return sysbvm_tuple_char32_decodeSmall(tuple);
    return *((sysbvm_char32_t*)SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes);
}

/**
 * Encodes an UInt32 as an immediate tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_uint32_encodeSmall(uint32_t value)
{
    return ((sysbvm_stuple_t)value << SYSBVM_TUPLE_TAG_BIT_COUNT) | SYSBVM_TUPLE_TAG_UINT32;
}

/*
 * Encodes an UInt32 as a bytes tuple.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_uint32_encodeBig(sysbvm_context_t *context, uint32_t value);

/**
 * Decodes an UInt32 from an immediate tuple.
 */
SYSBVM_INLINE uint32_t sysbvm_tuple_uint32_decodeSmall(sysbvm_tuple_t tuple)
{
    return (uint32_t) ((sysbvm_stuple_t)tuple >> SYSBVM_TUPLE_TAG_BIT_COUNT);
}

/*
 * Decodes an UInt32 from a tuple.
 */
SYSBVM_INLINE uint32_t sysbvm_tuple_uint32_decode(sysbvm_tuple_t tuple)
{
    if(sysbvm_tuple_isImmediate(tuple) || !tuple)
        return sysbvm_tuple_uint32_decodeSmall(tuple);
    return *((uint32_t*)SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes);
}

/**
 * Encodes an Int32 as an immediate tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_int32_encodeSmall(int32_t value)
{
    return ((sysbvm_stuple_t)value << SYSBVM_TUPLE_TAG_BIT_COUNT) | SYSBVM_TUPLE_TAG_INT32;
}

/**
 * Encodes an Int32 as a bytes tuple.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_int32_encodeBig(sysbvm_context_t *context, int32_t value);

/**
 * Decodes an Int32 from an immediate tuple.
 */
SYSBVM_INLINE int32_t sysbvm_tuple_int32_decodeSmall(sysbvm_tuple_t tuple)
{
    return (int32_t) ((sysbvm_stuple_t)tuple >> SYSBVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Decodes an Int32 from a tuple.
 */
SYSBVM_INLINE int32_t sysbvm_tuple_int32_decode(sysbvm_tuple_t tuple)
{
    if(sysbvm_tuple_isImmediate(tuple))
        return sysbvm_tuple_int32_decodeSmall(tuple);
    return *((int32_t*)SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

/**
 * Encodes a Char32 as a tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_char32_encode(sysbvm_context_t *context, sysbvm_char32_t value)
{
    int32_t svalue = (int32_t)value;
    if(sizeof(sysbvm_char32_t) < sizeof(sysbvm_tuple_t) || (SYSBVM_IMMEDIATE_INT_MIN <= svalue && svalue <= SYSBVM_IMMEDIATE_INT_MAX))
        return sysbvm_tuple_char32_encodeSmall(value);
    else
        return sysbvm_tuple_char32_encodeBig(context, value);
}

/**
 * Encodes an UInt32 as a tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_uint32_encode(sysbvm_context_t *context, uint32_t value)
{
    int32_t svalue = (int32_t)value;
    if(sizeof(uint32_t) < sizeof(sysbvm_tuple_t) || (SYSBVM_IMMEDIATE_INT_MIN <= svalue && svalue <= SYSBVM_IMMEDIATE_INT_MAX))
        return sysbvm_tuple_uint32_encodeSmall(value);
    else
        return sysbvm_tuple_uint32_encodeBig(context, value);
}

/**
 * Encodes an Int32 as a tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_int32_encode(sysbvm_context_t *context, int32_t value)
{
    if(sizeof(int32_t) < sizeof(sysbvm_tuple_t) || (SYSBVM_IMMEDIATE_INT_MIN <= value && value <= SYSBVM_IMMEDIATE_INT_MAX))
        return sysbvm_tuple_int32_encodeSmall(value);
    else
        return sysbvm_tuple_int32_encodeBig(context, value);
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

/**
 * Encodes an UInt64 as an immediate tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_uint64_encodeSmall(uint64_t value)
{
    return ((sysbvm_stuple_t)value << SYSBVM_TUPLE_TAG_BIT_COUNT) | SYSBVM_TUPLE_TAG_UINT64;
}

/*
 * Encodes an UInt64 as a bytes tuple.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_uint64_encodeBig(sysbvm_context_t *context, uint64_t value);

/**
 * Decodes an UInt64 from an immediate tuple.
 */
SYSBVM_INLINE uint64_t sysbvm_tuple_uint64_decodeSmall(sysbvm_tuple_t tuple)
{
    return (uint64_t) ((sysbvm_stuple_t)tuple >> SYSBVM_TUPLE_TAG_BIT_COUNT);
}

/*
 * Decodes an UInt64 from a tuple.
 */
SYSBVM_INLINE uint64_t sysbvm_tuple_uint64_decode(sysbvm_tuple_t tuple)
{
    if(sysbvm_tuple_isImmediate(tuple) || !tuple)
        return sysbvm_tuple_uint64_decodeSmall(tuple);
    return *((uint64_t*)SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes);
}

/**
 * Encodes an Int64 as an immediate tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_int64_encodeSmall(int64_t value)
{
    return ((sysbvm_stuple_t)value << SYSBVM_TUPLE_TAG_BIT_COUNT) | SYSBVM_TUPLE_TAG_INT64;
}

/*
 * Encodes an Int64 as a bytes tuple.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_int64_encodeBig(sysbvm_context_t *context, int64_t value);

/**
 * Decodes an Int64 from an immediate tuple.
 */
SYSBVM_INLINE int64_t sysbvm_tuple_int64_decodeSmall(sysbvm_tuple_t tuple)
{
    return (int64_t) ((sysbvm_stuple_t)tuple >> SYSBVM_TUPLE_TAG_BIT_COUNT);
}

/**
 * Decodes an Int64 from a tuple.
 */
SYSBVM_INLINE int64_t sysbvm_tuple_int64_decode(sysbvm_tuple_t tuple)
{
    if(sysbvm_tuple_isImmediate(tuple))
        return sysbvm_tuple_int64_decodeSmall(tuple);
    return *((int64_t*)SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(tuple)->bytes);
}

/**
 * Encodes an UInt64 as a tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_uint64_encode(sysbvm_context_t *context, uint64_t value)
{
    int64_t svalue = (int64_t)value;
    if(SYSBVM_IMMEDIATE_INT_MIN <= svalue && svalue <= SYSBVM_IMMEDIATE_INT_MAX)
        return sysbvm_tuple_uint64_encodeSmall(value);
    else
        return sysbvm_tuple_uint64_encodeBig(context, value);
}

/**
 * Encodes an Int64 as a tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_int64_encode(sysbvm_context_t *context, int64_t value)
{
    if((SYSBVM_IMMEDIATE_INT_MIN <= value && value <= SYSBVM_IMMEDIATE_INT_MAX))
        return sysbvm_tuple_int64_encodeSmall(value);
    else
        return sysbvm_tuple_int64_encodeBig(context, value);
}

/**
 * Decodes a bitflags from tuple.
 */
SYSBVM_INLINE sysbvm_bitflags_t sysbvm_tuple_bitflags_decode(sysbvm_tuple_t tuple)
{
    if(sizeof(sysbvm_bitflags_t) == sizeof(uint32_t))
        return (sysbvm_bitflags_t)sysbvm_tuple_uint32_decode(tuple);
    else
        return (sysbvm_bitflags_t)sysbvm_tuple_uint64_decode(tuple);
}

/**
 * Decodes a bitflags from tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_bitflags_encode(sysbvm_bitflags_t flags)
{
    if(sizeof(sysbvm_bitflags_t) == sizeof(uint32_t))
        return sysbvm_tuple_uint32_encodeSmall((uint32_t)flags);
    else
        return sysbvm_tuple_uint64_encodeSmall(flags);
}

/**
 * Encodes a size as a tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_size_encode(sysbvm_context_t *context, sysbvm_size_t value)
{
    if(sizeof(sysbvm_size_t) == sizeof(uint32_t))
        return sysbvm_tuple_uint32_encode(context, (uint32_t)value);
    else
        return sysbvm_tuple_uint64_encode(context, (uint64_t)value);
}

/**
 * Decodes a size from tuple.
 */
SYSBVM_INLINE sysbvm_size_t sysbvm_tuple_size_decode(sysbvm_tuple_t tuple)
{
    if(sizeof(sysbvm_size_t) == sizeof(uint32_t))
        return sysbvm_tuple_uint32_decode(tuple);
    else
        return (sysbvm_size_t)sysbvm_tuple_uint64_decode(tuple);
}

/**
 * Encodes an uintptr_t as a tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_uintptr_encode(sysbvm_context_t *context, sysbvm_uintptr_t value)
{
    if(sizeof(sysbvm_uintptr_t) == sizeof(uint32_t))
        return sysbvm_tuple_uint32_encode(context, (uint32_t)value);
    else
        return sysbvm_tuple_uint64_encode(context, (uint64_t)value);
}

/**
 * Decodes a intptr_t from tuple.
 */
SYSBVM_INLINE sysbvm_uintptr_t sysbvm_tuple_uintptr_decode(sysbvm_tuple_t tuple)
{
    if(sizeof(sysbvm_uintptr_t) == sizeof(uint32_t))
        return sysbvm_tuple_uint32_decode(tuple);
    else
        return (sysbvm_uintptr_t)sysbvm_tuple_uint64_decode(tuple);
}

/**
 * Encodes an intptr_t as a tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_intptr_encode(sysbvm_context_t *context, sysbvm_intptr_t value)
{
    if(sizeof(sysbvm_intptr_t) == sizeof(int32_t))
        return sysbvm_tuple_int32_encode(context, (int32_t)value);
    else
        return sysbvm_tuple_int64_encode(context, (int64_t)value);
}

/**
 * Decodes a intptr_t from tuple.
 */
SYSBVM_INLINE sysbvm_intptr_t sysbvm_tuple_intptr_decode(sysbvm_tuple_t tuple)
{
    if(sizeof(sysbvm_intptr_t) == sizeof(int32_t))
        return sysbvm_tuple_int32_decode(tuple);
    else
        return (sysbvm_intptr_t)sysbvm_tuple_int64_decode(tuple);
}

/**
 * Encodes a system handle as a tuple.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_systemHandle_encode(sysbvm_context_t *context, sysbvm_systemHandle_t value)
{
    if(sizeof(sysbvm_systemHandle_t) == sizeof(int32_t))
        return sysbvm_tuple_int32_encode(context, (int32_t)value);
    else
        return sysbvm_tuple_int64_encode(context, (int64_t)value);
}

/**
 * Decodes a system handle from tuple.
 */
SYSBVM_INLINE sysbvm_intptr_t sysbvm_tuple_systemHandle_decode(sysbvm_tuple_t tuple)
{
    if(sizeof(sysbvm_systemHandle_t) == sizeof(int32_t))
        return sysbvm_tuple_int32_decode(tuple);
    else
        return (sysbvm_intptr_t)sysbvm_tuple_int64_decode(tuple);
}

/**
 * Decodes a float32 from a tuple.
 */
SYSBVM_API sysbvm_float32_t sysbvm_tuple_float32_decode(sysbvm_tuple_t tuple);

/**
 * Encodes a float32 as a tuple.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_float32_encode(sysbvm_context_t *context, sysbvm_float32_t value);

/**
 * Decodes a float64 from a tuple.
 */
SYSBVM_API sysbvm_float64_t sysbvm_tuple_float64_decode(sysbvm_tuple_t tuple);

/**
 * Encodes a float64 as a tuple.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_float64_encode(sysbvm_context_t *context, sysbvm_float64_t value);

/**
 * Encodes a boolean value.
 */ 
SYSBVM_INLINE sysbvm_tuple_t sysbvm_tuple_boolean_encode(bool value)
{
    return value ? SYSBVM_TRUE_TUPLE : SYSBVM_FALSE_TUPLE;
}

/**
 * Decodes a boolean16 from a tuple.
 */
SYSBVM_API sysbvm_boolean16_t sysbvm_tuple_boolean16_decode(sysbvm_tuple_t tuple);

/**
 * Encodes a boolean16 as a tuple.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_boolean16_encode(sysbvm_context_t *context, sysbvm_boolean16_t value);

/**
 * Decodes a boolean32 from a tuple.
 */
SYSBVM_API sysbvm_boolean32_t sysbvm_tuple_boolean32_decode(sysbvm_tuple_t tuple);

/**
 * Encodes a boolean32 as a tuple.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_boolean32_encode(sysbvm_context_t *context, sysbvm_boolean32_t value);

/**
 * Decodes a boolean64 from a tuple.
 */
SYSBVM_API sysbvm_boolean64_t sysbvm_tuple_boolean64_decode(sysbvm_tuple_t tuple);

/**
 * Encodes a boolean64 as a tuple.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_boolean64_encode(sysbvm_context_t *context, sysbvm_boolean64_t value);

/**
 * Encodes a boolean value.
 */ 
SYSBVM_INLINE bool sysbvm_tuple_boolean_decode(sysbvm_tuple_t value)
{
    return value == SYSBVM_TRUE_TUPLE;
}

/**
 * Computes or retrieves the identity hash 
 */
SYSBVM_INLINE size_t sysbvm_tuple_identityHash(sysbvm_tuple_t tuple)
{
    if(sysbvm_tuple_isNonNullPointer(tuple))
    {
        sysbvm_object_tuple_t *object = (sysbvm_object_tuple_t*)tuple;
        return (object->header.identityHashAndFlags & SYSBVM_TUPLE_IDENTITY_HASH_MASK) >> SYSBVM_TUPLE_IDENTITY_HASH_SHIFT;
    }
    else
    {
        return sysbvm_hashMultiply(tuple);
    }
}

/**
 * Sets the identity hash.
 */
SYSBVM_INLINE void sysbvm_tuple_setIdentityHash(sysbvm_object_tuple_t *objectTuple, size_t newIdentityHash)
{
    objectTuple->header.identityHashAndFlags = (objectTuple->header.identityHashAndFlags & ~SYSBVM_TUPLE_IDENTITY_HASH_MASK) | ((newIdentityHash << SYSBVM_TUPLE_IDENTITY_HASH_SHIFT) & SYSBVM_TUPLE_IDENTITY_HASH_MASK);
}

/**
 * Compares two tuples for identity equality.
 */
SYSBVM_INLINE bool sysbvm_tuple_identityEquals(sysbvm_tuple_t a, sysbvm_tuple_t b)
{
    return a == b;
}

/**
 * Compares two tuples for identity inequality.
 */
SYSBVM_INLINE bool sysbvm_tuple_identityNotEquals(sysbvm_tuple_t a, sysbvm_tuple_t b)
{
    return a != b;
}

/**
 * Decodes any size valuable tuple.
 */
SYSBVM_INLINE size_t sysbvm_tuple_anySize_decode(sysbvm_tuple_t tuple)
{
    if(sysbvm_tuple_isImmediate(tuple))
        return (size_t)(tuple>>SYSBVM_TUPLE_TAG_BIT_COUNT);
    return 0;
}

/**
 * Computes the hash of a tuple
 */
SYSBVM_API size_t sysbvm_tuple_hash(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Compares two tuples by equality.
 */
SYSBVM_API bool sysbvm_tuple_equals(sysbvm_context_t *context, sysbvm_tuple_t a, sysbvm_tuple_t b);

/**
 * The primitive identity hash function.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_primitive_identityHash(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments);

/**
 * The primitive identity equals function.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_primitive_identityEquals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments);;

/**
 * The primitive identity not-equals function.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_primitive_identityNotEquals(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments);;

/**
 * Makes a C string with the specified bytes tuple.
 */
SYSBVM_API char *sysbvm_tuple_bytesToCString(sysbvm_tuple_t tuple);

/**
 * Frees a C string that was allocated for converting a tuple into a C string.
 */
SYSBVM_API void sysbvm_tuple_bytesToCStringFree(char *cstring);

/**
 * Gets the value of a slot in a tuple
 */
SYSBVM_API sysbvm_tuple_t sysbvm_tuple_slotAt(sysbvm_context_t *context, sysbvm_tuple_t tuple, size_t slotIndex);

/**
 * Gets the value of a slot in a byte tuple
 */
SYSBVM_API uint8_t sysbvm_tuple_byteSlotAt(sysbvm_context_t *context, sysbvm_tuple_t tuple, size_t slotIndex);

/**
 * Sets the value in a slot of a tuple
 */
SYSBVM_API void sysbvm_tuple_slotAtPut(sysbvm_context_t *context, sysbvm_tuple_t tuple, size_t slotIndex, sysbvm_tuple_t value);

/**
 * Sets the value in a slot of a byte tuple
 */
SYSBVM_API void sysbvm_tuple_byteSlotAtPut(sysbvm_context_t *context, sysbvm_tuple_t tuple, size_t slotIndex, uint8_t value);

/**
 * Is this tuple a kind of the specified type?
 */
SYSBVM_API bool sysbvm_tuple_isKindOf(sysbvm_context_t *context, sysbvm_tuple_t tuple, sysbvm_tuple_t type);

/**
 * Is this tuple a kind of the specified type?
 */
SYSBVM_API bool sysbvm_tuple_isTypeSatisfiedWithValue(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t value);

/**
 * Typechecks the given value
 */
SYSBVM_API void sysbvm_tuple_typecheckValue(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t value);

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif //SYSBVM_TUPLE_H
