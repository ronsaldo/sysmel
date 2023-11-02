#ifndef SYSBVM_TYPE_H
#define SYSBVM_TYPE_H

#pragma once

#include "programEntity.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_valueBox_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t value;
} sysbvm_valueBox_t;

typedef struct sysbvm_variableValueBox_s
{
    sysbvm_valueBox_t super;
    sysbvm_tuple_t binding;
} sysbvm_variableValueBox_t;

typedef struct sysbvm_type_pendingDefinitionFragment_s
{
    sysbvm_tuple_header_t super;
    sysbvm_tuple_t node;
    sysbvm_tuple_t environment;
} sysbvm_type_pendingDefinitionFragment_t;

typedef struct sysbvm_type_pendingDefinitionFragments_s
{
    sysbvm_tuple_header_t super;
    sysbvm_tuple_t supertype;
    sysbvm_tuple_t valueType;
    sysbvm_tuple_t values;
    sysbvm_tuple_t definitionsAndExtensions;
} sysbvm_type_pendingDefinitionFragments_t;

typedef struct sysbvm_type_tuple_s
{
    sysbvm_programEntity_t super;
    sysbvm_tuple_t supertype;
    sysbvm_tuple_t slots;
    sysbvm_tuple_t slotsWithBasicInitialization;
    sysbvm_tuple_t allSlots;
    sysbvm_tuple_t basicInitializeMethod;
    sysbvm_tuple_t totalSlotCount;
    sysbvm_tuple_t instanceSize;
    sysbvm_tuple_t instanceAlignment;
    sysbvm_tuple_t flags;
    sysbvm_tuple_t emptyTrivialSingleton;

    sysbvm_tuple_t slotDictionary;

    sysbvm_tuple_t macroMethodDictionary;
    sysbvm_tuple_t methodDictionary;
    sysbvm_tuple_t fallbackMethodDictionary;

    sysbvm_tuple_t constructors;
    sysbvm_tuple_t conversions;

    sysbvm_tuple_t virtualMethodSelectorList;
    sysbvm_tuple_t virtualTableSelectorDictionary;
    sysbvm_tuple_t virtualTable;
    sysbvm_tuple_t gcLayout;
    sysbvm_tuple_t variableDataGCLayout;

    sysbvm_tuple_t pendingSlots;
    sysbvm_tuple_t pendingDefinitionFragments;
    sysbvm_tuple_t subtypes;
    sysbvm_tuple_t children;
} sysbvm_type_tuple_t;

typedef enum sysbvm_typeFlags_e
{
    SYSBVM_TYPE_FLAGS_NONE = 0,
    SYSBVM_TYPE_FLAGS_NULLABLE = 1<<0,
    SYSBVM_TYPE_FLAGS_BYTES = 1<<1,
    SYSBVM_TYPE_FLAGS_IMMEDIATE = 1<<2,
    SYSBVM_TYPE_FLAGS_WEAK = 1<<3,

    SYSBVM_TYPE_FLAGS_FINAL = 1<<4,
    SYSBVM_TYPE_FLAGS_ABSTRACT = 1<<5,
    SYSBVM_TYPE_FLAGS_DYNAMIC = 1<<6,

    SYSBVM_TYPE_FLAGS_VALUE = 1<<7,
    SYSBVM_TYPE_FLAGS_FAT_POINTER_VALUE = 1<<8,
    SYSBVM_TYPE_FLAGS_POINTER_VALUE = 1<<9,
    SYSBVM_TYPE_FLAGS_REFERENCE_VALUE = 1<<10,
    SYSBVM_TYPE_FLAGS_TEMPORARY_REFERENCE_VALUE = 1<<11,

    SYSBVM_TYPE_FLAGS_FUNCTION = 1<<12,
    SYSBVM_TYPE_FLAGS_EMPTY_TRIVIAL_SINGLETON = 1<<13,

    SYSBVM_TYPE_FLAGS_POINTER_LIKE_VALUE = SYSBVM_TYPE_FLAGS_TEMPORARY_REFERENCE_VALUE | SYSBVM_TYPE_FLAGS_POINTER_VALUE | SYSBVM_TYPE_FLAGS_REFERENCE_VALUE,

    SYSBVM_TYPE_FLAGS_CLASS_DEFAULT_FLAGS = SYSBVM_TYPE_FLAGS_NULLABLE,
    SYSBVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS = SYSBVM_TYPE_FLAGS_NULLABLE,

    SYSBVM_TYPE_FLAGS_POINTER_TYPE_FLAGS = SYSBVM_TYPE_FLAGS_VALUE | SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_POINTER_VALUE | SYSBVM_TYPE_FLAGS_FINAL,
    SYSBVM_TYPE_FLAGS_REFERENCE_TYPE_FLAGS = SYSBVM_TYPE_FLAGS_VALUE | SYSBVM_TYPE_FLAGS_FINAL | SYSBVM_TYPE_FLAGS_REFERENCE_VALUE,
    SYSBVM_TYPE_FLAGS_TEMPORARY_REFERENCE_TYPE_FLAGS = SYSBVM_TYPE_FLAGS_VALUE | SYSBVM_TYPE_FLAGS_FINAL | SYSBVM_TYPE_FLAGS_TEMPORARY_REFERENCE_VALUE,

    SYSBVM_TYPE_FLAGS_VALUE_TYPE_DEFAULT_FLAGS = SYSBVM_TYPE_FLAGS_VALUE,
    SYSBVM_TYPE_FLAGS_PRIMITIVE_VALUE_TYPE_DEFAULT_FLAGS = SYSBVM_TYPE_FLAGS_VALUE | SYSBVM_TYPE_FLAGS_IMMEDIATE | SYSBVM_TYPE_FLAGS_FINAL,
    SYSBVM_TYPE_FLAGS_PRIMITIVE_VALUE_TYPE_METATYPE_FLAGS = SYSBVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS | SYSBVM_TYPE_FLAGS_FINAL,

    SYSBVM_TYPE_FLAGS_ABSTRACT_PRIMITIVE_VALUE_TYPE_DEFAULT_FLAGS = SYSBVM_TYPE_FLAGS_VALUE | SYSBVM_TYPE_FLAGS_IMMEDIATE,
    SYSBVM_TYPE_FLAGS_ABSTRACT_PRIMITIVE_VALUE_TYPE_METATYPE_FLAGS = SYSBVM_TYPE_FLAGS_METATYPE_REQUIRED_FLAGS,
} sysbvm_typeFlags_t;

typedef struct sysbvm_class_s
{
    sysbvm_type_tuple_t super;
} sysbvm_class_t;

typedef struct sysbvm_valueType_s
{
    sysbvm_type_tuple_t super;
} sysbvm_valueType_t;

typedef struct sysbvm_primitiveValueType_s
{
    sysbvm_valueType_t super;
} sysbvm_primitiveValueType_t;

typedef struct sysbvm_structureType_s
{
    sysbvm_valueType_t super;
} sysbvm_structureType_t;

typedef struct sysbvm_pointerLikeType_s
{
    sysbvm_valueType_t super;
    sysbvm_tuple_t baseType;
    sysbvm_tuple_t addressSpace;
    sysbvm_tuple_t loadValueFunction;
    sysbvm_tuple_t storeValueFunction;
} sysbvm_pointerLikeType_t;

typedef struct sysbvm_pointerType_s
{
    sysbvm_pointerLikeType_t super;
} sysbvm_pointerType_t;

typedef struct sysbvm_referenceType_s
{
    sysbvm_pointerLikeType_t super;
} sysbvm_referenceType_t;

typedef struct sysbvm_temporaryReferenceType_s
{
    sysbvm_pointerLikeType_t super;
} sysbvm_temporaryReferenceType_t;

typedef struct sysbvm_metatype_s
{
    sysbvm_type_tuple_t super;
    sysbvm_tuple_t thisType;
} sysbvm_metatype_t;

typedef struct sysbvm_metaclass_s
{
    sysbvm_metatype_t super;
} sysbvm_metaclass_t;

typedef struct sysbvm_valueMetatype_s
{
    sysbvm_metatype_t super;
} sysbvm_valueMetatype_t;

typedef struct sysbvm_typeSlot_s
{
    sysbvm_programEntity_t super;
    sysbvm_tuple_t flags;
    sysbvm_tuple_t type;
    sysbvm_tuple_t referenceType;
    sysbvm_tuple_t localIndex;
    sysbvm_tuple_t index;
    sysbvm_tuple_t offset;
    sysbvm_tuple_t initialValueBlock;
    sysbvm_tuple_t sourcePosition;
} sysbvm_typeSlot_t;

typedef struct sysbvm_functionType_s
{
    sysbvm_type_tuple_t super;
    sysbvm_tuple_t functionFlags;
} sysbvm_functionType_t;

typedef struct sysbvm_simpleFunctionType_s
{
    sysbvm_functionType_t super;
    sysbvm_tuple_t argumentTypes;
    sysbvm_tuple_t resultType;
} sysbvm_simpleFunctionType_t;

typedef struct sysbvm_dependentFunctionType_s
{
    sysbvm_functionType_t super;
    sysbvm_tuple_t sourcePosition;

    sysbvm_tuple_t argumentNodes;
    sysbvm_tuple_t resultTypeNode;

    sysbvm_tuple_t environment;
    sysbvm_tuple_t captureBindings;
    sysbvm_tuple_t argumentBindings;
    sysbvm_tuple_t localBindings;
} sysbvm_dependentFunctionType_t;

typedef struct sysbvm_sequenceTupleType_s
{
    sysbvm_type_tuple_t super;
    sysbvm_tuple_t elementTypes;
} sysbvm_sequenceTupleType_t;

typedef enum sysbvm_typeSlotFlags_e
{
    SYSBVM_TYPE_SLOT_FLAG_NONE = 0,

    SYSBVM_TYPE_SLOT_FLAG_VISIBILITY_BITS = 2,
    SYSBVM_TYPE_SLOT_FLAG_VISIBILITY_SHIFT = 0,
    SYSBVM_TYPE_SLOT_FLAG_VISIBILITY_MASK = 3<<SYSBVM_TYPE_SLOT_FLAG_VISIBILITY_SHIFT,

    SYSBVM_TYPE_SLOT_FLAG_PRIVATE = 0<<SYSBVM_TYPE_SLOT_FLAG_VISIBILITY_SHIFT,
    SYSBVM_TYPE_SLOT_FLAG_PROTECTED = 1<<SYSBVM_TYPE_SLOT_FLAG_VISIBILITY_SHIFT,
    SYSBVM_TYPE_SLOT_FLAG_INTERNAL = 2<<SYSBVM_TYPE_SLOT_FLAG_VISIBILITY_SHIFT,
    SYSBVM_TYPE_SLOT_FLAG_PUBLIC = 3<<SYSBVM_TYPE_SLOT_FLAG_VISIBILITY_SHIFT,

    SYSBVM_TYPE_SLOT_FLAG_READONLY = 1<<2,
    SYSBVM_TYPE_SLOT_FLAG_NO_RTTI_EXCLUDED = 1<<3,
    SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED = 1<<4,
    SYSBVM_TYPE_SLOT_FLAG_NO_SOURCE_DEFINITION_EXCLUDED = 1<<5,
    SYSBVM_TYPE_SLOT_FLAG_BYTECODE = 1<<6,
    SYSBVM_TYPE_SLOT_FLAG_DEBUG_INFORMATION = 1<<7,
    SYSBVM_TYPE_SLOT_FLAG_JIT_SPECIFIC = 1<<8,
    SYSBVM_TYPE_SLOT_FLAG_TARGET_GENERATED = 1<<9,
    SYSBVM_TYPE_SLOT_FLAG_CACHE = 1<<10,

} sysbvm_typeSlotFlags_t;

typedef struct sysbvm_pic_s sysbvm_pic_t;

/**
 * Creates a type slot
 */
SYSBVM_API sysbvm_tuple_t sysbvm_typeSlot_create(sysbvm_context_t *context, sysbvm_tuple_t owner, sysbvm_tuple_t name, sysbvm_tuple_t flags, sysbvm_tuple_t type, size_t localIndex, size_t index);

/**
 * Gets the type from type slot.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_typeSlot_getType(sysbvm_tuple_t typeSlot)
{
    if(!sysbvm_tuple_isNonNullPointer(typeSlot)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_typeSlot_t*)typeSlot)->type;
}

/**
 * Gets the index from type slot.
 */
SYSBVM_INLINE size_t sysbvm_typeSlot_getIndex(sysbvm_tuple_t typeSlot)
{
    if(!sysbvm_tuple_isNonNullPointer(typeSlot)) return 0;
    return sysbvm_tuple_size_decode(((sysbvm_typeSlot_t*)typeSlot)->index);
}

/**
 * Gets a valid reference type corresponding to the specified type slot.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_typeSlot_getValidReferenceType(sysbvm_context_t *context, sysbvm_tuple_t typeSlot);

/**
 * Gets the generic address space singleton.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_genericAddressSpace_uniqueInstance(sysbvm_context_t *context);

/**
 * Gets the member address space singleton.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_memberAddressSpace_uniqueInstance(sysbvm_context_t *context);

/**
 * Creates an anonymous type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymous(sysbvm_context_t *context);

/**
 * Creates an anonymous metatype.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousMetatype(sysbvm_context_t *context);

/**
 * Creates an anonymous type with a corresponding metatype.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousAndMetatype(sysbvm_context_t *context);

/**
 * Creates an anonymous class type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousClass(sysbvm_context_t *context, sysbvm_tuple_t supertype, sysbvm_tuple_t metaclass);

/**
 * Creates an anonymous metaclass type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousMetaclass(sysbvm_context_t *context, sysbvm_tuple_t supertype);

/**
 * Creates an anonymous class with respective metaclass.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousClassAndMetaclass(sysbvm_context_t *context, sysbvm_tuple_t supertype);

/**
 * Creates an anonymous metaclass type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousValueMetatype(sysbvm_context_t *context, sysbvm_tuple_t supertype, size_t minimumSlotCount);

/**
 * Creates an anonymous primitive type with respective meta value type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousPrimitiveValueTypeAndValueMetatype(sysbvm_context_t *context, sysbvm_tuple_t supertype);

/**
 * Creates an anonymous primitive type with respective meta value type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createAnonymousAbstractPrimitiveValueTypeAndValueMetatype(sysbvm_context_t *context, sysbvm_tuple_t supertype);

/**
 * Creates a type with the specified name.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createWithName(sysbvm_context_t *context, sysbvm_tuple_t name);

/**
 * Creates a dependent function type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createDependentFunctionType(sysbvm_context_t *context, sysbvm_tuple_t argumentNodes, sysbvm_bitflags_t flags, sysbvm_tuple_t resultTypeNode,
    sysbvm_tuple_t environment, sysbvm_tuple_t captureBindings, sysbvm_tuple_t argumentBindings, sysbvm_tuple_t localBindings);

/**
 * Creates a simple function type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createSimpleFunctionType(sysbvm_context_t *context, sysbvm_tuple_t argumentTypes, sysbvm_bitflags_t flags, sysbvm_tuple_t resultType);

/**
 * Creates a simple function type with no arguments.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createSimpleFunctionTypeWithArguments0(sysbvm_context_t *context, sysbvm_tuple_t resultType);

/**
 * Creates a simple function type with one arguments.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createSimpleFunctionTypeWithArguments1(sysbvm_context_t *context, sysbvm_tuple_t argument, sysbvm_tuple_t resultType);

/**
 * Creates a simple function type with two arguments.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createSimpleFunctionTypeWithArguments2(sysbvm_context_t *context, sysbvm_tuple_t argument0, sysbvm_tuple_t argument1, sysbvm_tuple_t resultType);

/**
 * Creates a sequence tuple type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createSequenceTupleType(sysbvm_context_t *context, sysbvm_tuple_t elementTypes);

/**
 * Creates a sequence tuple.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_sequenceTuple_create(sysbvm_context_t *context, sysbvm_tuple_t sequenceTupleType);

/**
 * Creates a sequence tuple for the specified function definition.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_sequenceTuple_createForFunctionDefinition(sysbvm_context_t *context, sysbvm_tuple_t functionDefinition);

/**
 * Creates a pointer type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createPointerType(sysbvm_context_t *context, sysbvm_tuple_t baseType, sysbvm_tuple_t addressSpace);

/**
 * Creates a reference type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createReferenceType(sysbvm_context_t *context, sysbvm_tuple_t baseType, sysbvm_tuple_t addressSpace);

/**
 * Creates a temporary reference type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createTemporaryReferenceType(sysbvm_context_t *context, sysbvm_tuple_t baseType, sysbvm_tuple_t addressSpace);

/**
 * Creates a generic reference type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createGenericReferenceType(sysbvm_context_t *context, sysbvm_tuple_t baseType);

/**
 * Creates a member reference type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_createMemberReferenceType(sysbvm_context_t *context, sysbvm_tuple_t baseType);

/**
 * Creates a value box with the given value.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_valueBox_with(sysbvm_context_t *context, sysbvm_tuple_t boxedValue);

/**
 * Creates a binding value box with the given value.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_variableValueBox_with(sysbvm_context_t *context, sysbvm_tuple_t boxedValue);

/**
 * Creates a pointer like with the specified storage
 */
SYSBVM_API sysbvm_tuple_t sysbvm_pointerLikeType_withStorage(sysbvm_context_t *context, sysbvm_tuple_t referenceType, sysbvm_tuple_t storage);

/**
 * Creates a pointer like value that boxes a value.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_pointerLikeType_withBoxForValue(sysbvm_context_t *context, sysbvm_tuple_t referenceType, sysbvm_tuple_t boxedValue);

/**
 * Creates a pointer like value that boxes a value which is empty.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_pointerLikeType_withEmptyBox(sysbvm_context_t *context, sysbvm_tuple_t referenceType);

/**
 * Creates a reference with the specified storage
 */
SYSBVM_API sysbvm_tuple_t sysbvm_referenceType_withStorage(sysbvm_context_t *context, sysbvm_tuple_t referenceType, sysbvm_tuple_t storage);

/**
 * Creates a reference that boxes a value.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_referenceType_withBoxForValue(sysbvm_context_t *context, sysbvm_tuple_t referenceType, sysbvm_tuple_t boxedValue);

/**
 * Creates a reference onto a specific type slot in an object
 */
SYSBVM_API sysbvm_tuple_t sysbvm_referenceType_withTupleAndTypeSlot(sysbvm_context_t *context, sysbvm_tuple_t referenceType, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot);

/**
 * Creates a reference onto a specific type slot in an object by incrementing an existing reference.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_referenceType_incrementWithTypeSlot(sysbvm_context_t *context, sysbvm_tuple_t referenceType, sysbvm_tuple_t parentReference, sysbvm_tuple_t typeSlot);

/**
 * Canonicalizes a function type
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_canonicalizeFunctionType(sysbvm_context_t *context, sysbvm_tuple_t functionType);

/**
 * Performs the lookup of the given macro selector.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_lookupMacroSelector(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t selector);

/**
 * Performs the lookup of the given slot.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_lookupSlot(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t selector);

/**
 * Performs the lookup of the given selector.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_lookupSelector(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t selector);

/**
 * Performs the lookup of the given selector accelerated with the given polymorphic inline cache.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_lookupSelectorWithPIC(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t selector, sysbvm_pic_t *inlineCache);

/**
 * Performs the lookup of the given macro fallback selector.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_lookupFallbackSelector(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t selector);

/**
 * Sets the specified method with the given selector in the method dictionary.
 */
SYSBVM_API void sysbvm_type_setMethodWithSelector(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t selector, sysbvm_tuple_t method);

/**
 * Build the slot dictionary.
 */
SYSBVM_API void sysbvm_type_buildSlotDictionary(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Is this type a subtype of?
 */
SYSBVM_INLINE bool sysbvm_type_isDirectSubtypeOf(sysbvm_tuple_t type, sysbvm_tuple_t supertype)
{
    if(!sysbvm_tuple_isNonNullPointer(supertype)) return false;
    if(!sysbvm_tuple_isNonNullPointer(type)) return false;
    if(sysbvm_tuple_isDummyValue(type)) return false;

    while(type)
    {
        if(type == supertype)
            return true;

        type = ((sysbvm_type_tuple_t *)type)->supertype;
    }

    return false;
}

/**
 * Gets the name of a type
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_type_getName(sysbvm_tuple_t type)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return SYSBVM_NULL_TUPLE;
    if(sysbvm_tuple_isDummyValue(type)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_type_tuple_t*)type)->super.name;
}

/**
 * Sets the name of a type
 */
SYSBVM_INLINE void sysbvm_type_setName(sysbvm_tuple_t type, sysbvm_tuple_t name)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return;
    if(sysbvm_tuple_isDummyValue(type)) return;
    ((sysbvm_type_tuple_t*)type)->super.name = name;
}

/**
 * Gets the supertype
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_type_getSupertype(sysbvm_tuple_t type)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return SYSBVM_NULL_TUPLE;
    if(sysbvm_tuple_isDummyValue(type)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_type_tuple_t*)type)->supertype;
}

/**
 * Sets the supertype
 */
SYSBVM_INLINE void sysbvm_type_setSupertype(sysbvm_tuple_t type, sysbvm_tuple_t supertype)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return;
    if(sysbvm_tuple_isDummyValue(type)) return;
    
    ((sysbvm_type_tuple_t*)type)->supertype = supertype;
}

/**
 * Gets the slot names
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_type_getSlots(sysbvm_tuple_t type)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return SYSBVM_NULL_TUPLE;
    if(sysbvm_tuple_isDummyValue(type)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_type_tuple_t*)type)->slots;
}

/**
 * Sets the slot names
 */
SYSBVM_INLINE void sysbvm_type_setSlots(sysbvm_tuple_t type, sysbvm_tuple_t slots)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return;
    ((sysbvm_type_tuple_t*)type)->slots = slots;
}

/**
 * Gets the total slot count.
 */
SYSBVM_API size_t sysbvm_type_getTotalSlotCount(sysbvm_tuple_t type);

/**
 * Sets the total slot count
 */
SYSBVM_API void sysbvm_type_setTotalSlotCount(sysbvm_context_t *context, sysbvm_tuple_t type, size_t totalSlotCount);

/**
 * Sets the instance size and alignment.
 */
SYSBVM_API void sysbvm_type_setInstanceSizeAndAlignment(sysbvm_context_t *context, sysbvm_tuple_t type, size_t instanceSize, size_t instanceAlignment);

/**
 * Gets the type flags.
 */
SYSBVM_INLINE sysbvm_bitflags_t sysbvm_type_getFlags(sysbvm_tuple_t type)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return 0;
    if(sysbvm_tuple_isDummyValue(type)) return 0;
    return sysbvm_tuple_bitflags_decode(((sysbvm_type_tuple_t*)type)->flags);
}

/**
 * Sets the type flags.
 */
SYSBVM_API void sysbvm_type_setFlags(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_bitflags_t flags);

/**
 * Sets the type and meta type flags.
 */
SYSBVM_API void sysbvm_typeAndMetatype_setFlags(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_bitflags_t flags, sysbvm_bitflags_t metatypeFlags);

/**
 * Is this a nullable type?
 */
SYSBVM_INLINE bool sysbvm_type_isNullable(sysbvm_tuple_t type)
{
    return !type || (sysbvm_type_getFlags(type) & SYSBVM_TYPE_FLAGS_NULLABLE) != 0;
}

/**
 * Is this a dynamic type?
 */
SYSBVM_INLINE bool sysbvm_type_isDynamic(sysbvm_tuple_t type)
{
    return !type || sysbvm_tuple_isDummyValue(type) || (sysbvm_type_getFlags(type) & SYSBVM_TYPE_FLAGS_DYNAMIC) != 0;
}

/**
 * Is this type a function type?
 */
SYSBVM_INLINE bool sysbvm_type_isFunctionType(sysbvm_tuple_t type)
{
    return (sysbvm_type_getFlags(type) & SYSBVM_TYPE_FLAGS_FUNCTION) != 0;
}

/**
 * Is this tuple a function?
 */
SYSBVM_INLINE bool sysbvm_tuple_isFunction(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    (void)context;
    return sysbvm_tuple_isNonNullPointer(tuple) && sysbvm_type_isFunctionType(sysbvm_pointerTuple_getType(tuple));
}

/**
 * Is this a pointer like value?
 */
SYSBVM_INLINE bool sysbvm_type_isPointerLikeType(sysbvm_tuple_t type)
{
    return (sysbvm_type_getFlags(type) & SYSBVM_TYPE_FLAGS_POINTER_LIKE_VALUE) != 0;
}

/**
 * Is this a pointer type?
 */
SYSBVM_INLINE bool sysbvm_type_isPointerType(sysbvm_tuple_t type)
{
    return (sysbvm_type_getFlags(type) & SYSBVM_TYPE_FLAGS_POINTER_VALUE) != 0;
}

/**
 * Is this a reference type?
 */
SYSBVM_INLINE bool sysbvm_type_isReferenceType(sysbvm_tuple_t type)
{
    return (sysbvm_type_getFlags(type) & SYSBVM_TYPE_FLAGS_REFERENCE_VALUE) != 0;
}

/**
 * Gets the slot dictionary.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_type_getSlotDictionary(sysbvm_tuple_t type)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return SYSBVM_NULL_TUPLE;
    if(sysbvm_tuple_isDummyValue(type)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_type_tuple_t*)type)->slotDictionary;
}

/**
 * Gets the macro method dictionary.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_type_getMacroMethodDictionary(sysbvm_tuple_t type)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return SYSBVM_NULL_TUPLE;
    if(sysbvm_tuple_isDummyValue(type)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_type_tuple_t*)type)->macroMethodDictionary;
}

/**
 * Gets the method dictionary.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_type_getMethodDictionary(sysbvm_tuple_t type)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return SYSBVM_NULL_TUPLE;
    if(sysbvm_tuple_isDummyValue(type)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_type_tuple_t*)type)->methodDictionary;
}

/**
 * Gets the macro fallback method dictionary.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_type_getFallbackMethodDictionary(sysbvm_tuple_t type)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return SYSBVM_NULL_TUPLE;
    if(sysbvm_tuple_isDummyValue(type)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_type_tuple_t*)type)->fallbackMethodDictionary;
}

/**
 * Sets the method dictionary.
 */
SYSBVM_INLINE void sysbvm_type_setMethodDictionary(sysbvm_tuple_t type, sysbvm_tuple_t methodDictionary)
{
    if(!sysbvm_tuple_isNonNullPointer(type)) return;
    if(sysbvm_tuple_isDummyValue(type)) return;
    ((sysbvm_type_tuple_t*)type)->methodDictionary = methodDictionary;
}

/**
 * Gets the equals function of a type
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getEqualsFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the equals function of a type
 */
SYSBVM_API void sysbvm_type_setEqualsFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t equalsFunction);

/**
 * Gets the hash function of a type
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getHashFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the hash function of a type
 */
SYSBVM_API void sysbvm_type_setHashFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t hashFunction);

/**
 * Gets the asString function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getAsStringFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the asString function of a type
 */
SYSBVM_API void sysbvm_type_setAsStringFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t asStringFunction);

/**
 * Gets the printString function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getPrintStringFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the printString function of a type
 */
SYSBVM_API void sysbvm_type_setPrintStringFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t printStringFunction);

/**
 * Gets the astNodeAnalysisFunction function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getAstNodeAnalysisFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the astNodeAnalysisFunction function of a type
 */
SYSBVM_API void sysbvm_type_setAstNodeAnalysisFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t astNodeAnalysisFunction);

/**
 * Gets the astNodeEvaluationFunction function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getAstNodeEvaluationFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the astNodeEvaluationFunction function of a type
 */
SYSBVM_API void sysbvm_type_setAstNodeEvaluationFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t astNodeEvaluationFunction);

/**
 * Gets the astNodeAnalysisAndEvaluationFunction function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getAstNodeAnalysisAndEvaluationFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the astNodeAnalysisAndEvaluationFunction function of a type
 */
SYSBVM_API void sysbvm_type_setAstNodeAnalysisAndEvaluationFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t astNodeAnalysisAndEvaluationFunction);

/**
 * Gets the astNodeValidationThenAnalysisAndEvaluationFunction function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getAstNodeValidationWithAnalysisAndEvaluationFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Gets the analyzeAndEvaluateMessageSendNode with environment function function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the analyzeAndEvaluateMessageSendNode with environment function function of a type.
 */
SYSBVM_API void sysbvm_type_setAnalyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t function);

/**
 * Gets the analyzeMessageSendNode with environment function function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeMessageSendNodeWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the analyzeMessageSendNode with environment function function of a type.
 */
SYSBVM_API void sysbvm_type_setAnalyzeMessageSendNodeWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t function);

/**
 * Gets the analyzeAndEvaluateMessageChainNode with environment function function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the analyzeAndEvaluateMessageChainNode with environment function function of a type.
 */
SYSBVM_API void sysbvm_type_setAnalyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t function);

/**
 * Gets the analyzeMessageChainNode with environment function function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeMessageChainNodeWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the analyzeMessageChainNode with environment function function of a type.
 */
SYSBVM_API void sysbvm_type_setAnalyzeMessageChainNodeWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t function);

/**
 * Gets the analyzeAndEvaluateMessageChainNode with environment function function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeAndEvaluateConcreteMetaValueWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the analyzeAndEvaluateMessageChainNode with environment function function of a type.
 */
SYSBVM_API void sysbvm_type_setAnalyzeAndEvaluateConcreteMetaValueWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t function);

/**
 * Gets the analyzeMessageChainNode with environment function function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeConcreteMetaValueWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the analyzeMessageChainNode with environment function function of a type.
 */
SYSBVM_API void sysbvm_type_setAnalyzeConcreteMetaValueWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t function);

/**
 * Gets the analyzeAndEvaluateUnexpandedApplicationNodeOfWithEnvironmentSelector with environment function function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeAndEvaluateUnexpandedApplicationNodeOfWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the analyzeAndEvaluateUnexpandedApplicationNodeOfWithEnvironmentSelector with environment function function of a type.
 */
SYSBVM_API void sysbvm_type_setAnalyzeAndEvaluateUnexpandedApplicationNodeOfWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t function);

/**
 * Gets the analyzeUnexpandedApplicationNodeWithEnvironmentSelector with environment function function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeUnexpandedApplicationNodeWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the analyzeUnexpandedApplicationNodeWithEnvironmentSelector with environment function function of a type.
 */
SYSBVM_API void sysbvm_type_setAnalyzeUnexpandedApplicationNodeWithEnvironmentFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t function);

/**
 * Gets the coerceValueFunction function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getCoerceValueFunction(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the coerceValueFunction function of a type
 */
SYSBVM_API void sysbvm_type_setCoerceValueFunction(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t coerceValueFunction);

/**
 * Gets the typeCheckFunction function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeAndTypeCheckFunctionApplicationNodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the typeCheckFunctionApplication function of a type
 */
SYSBVM_API void sysbvm_type_setAnalyzeAndTypeCheckFunctionApplicationNodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t coerceValueFunction);

/**
 * Gets the typeCheckFunction function of a type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getAnalyzeAndTypeCheckMessageSendNodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Sets the typeCheckFunctionApplication function of a type
 */
SYSBVM_API void sysbvm_type_setAnalyzeAndTypeCheckMessageSendNodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t coerceValueFunction);

/**
 * Coerces a value into the specified type. Error if the coercion is not possible.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_coerceValue(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t value);

/**
 * Down casts a value into the specified type. Error if the coercion is not possible.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_downCastValue(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t value);

/**
 * Coerces a value into the specified type. Passing references in case of no type to coerce-
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_coerceValuePassingReferences(sysbvm_context_t *context, sysbvm_tuple_t type, sysbvm_tuple_t value);

/**
 * Loads the value from a pointer like type instance.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_pointerLikeType_load(sysbvm_context_t *context, sysbvm_tuple_t pointerLikeValue);

/**
 * Store the value into a pointer like type instance.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_pointerLikeType_store(sysbvm_context_t *context, sysbvm_tuple_t pointerLikeValue, sysbvm_tuple_t valueToStore);

/**
 * Gets a decayed version of the type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_decay(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Gets a decayed version of the receiver type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_decayReceiver(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Gets the canonical pending type
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getCanonicalPendingInstanceType(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Gets the canonical dependent result type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_canonicalizeDependentResultType(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Gets the default value for the given type.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_getDefaultValue(sysbvm_context_t *context, sysbvm_tuple_t type);

/**
 * Computes the LCA between two types
 */
SYSBVM_API sysbvm_tuple_t sysbvm_type_computeLCA(sysbvm_tuple_t leftType, sysbvm_tuple_t rightType);

#endif //SYSBVM_TYPE_H