#include "internal/context.h"
#include "sysbvm/type.h"
#include "sysbvm/array.h"
#include "sysbvm/orderedCollection.h"
#include "sysbvm/environment.h"
#include "sysbvm/gc.h"
#include "sysbvm/gdb.h"
#include "sysbvm/stackFrame.h"
#include "sysbvm/string.h"
#include "sysbvm/set.h"
#include "sysbvm/system.h"
#include "sysbvm/function.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
extern void __deregister_frame(const void*);
#endif

static bool sysbvm_context_default_jitEnabled = true;

extern void sysbvm_array_registerPrimitives(void);
extern void sysbvm_orderedCollection_registerPrimitives(void);
extern void sysbvm_astInterpreter_registerPrimitives(void);
extern void sysbvm_boolean_registerPrimitives(void);
extern void sysbvm_bytecode_registerPrimitives(void);
extern void sysbvm_functionBytecodeDirectCompiler_registerPrimitives();
extern void sysbvm_byteStream_registerPrimitives(void);
extern void sysbvm_dictionary_registerPrimitives(void);
extern void sysbvm_errors_registerPrimitives(void);
extern void sysbvm_environment_registerPrimitives(void);
extern void sysbvm_exceptions_registerPrimitives(void);
extern void sysbvm_filesystem_registerPrimitives(void);
extern void sysbvm_float_registerPrimitives(void);
extern void sysbvm_function_registerPrimitives(void);
extern void sysbvm_integer_registerPrimitives(void);
extern void sysbvm_io_registerPrimitives(void);
extern void sysbvm_primitiveInteger_registerPrimitives(void);
extern void sysbvm_programEntity_registerPrimitives(void);
extern void sysbvm_set_registerPrimitives(void);
extern void sysbvm_sourcePosition_registerPrimitives(void);
extern void sysbvm_string_registerPrimitives(void);
extern void sysbvm_stringStream_registerPrimitives(void);
extern void sysbvm_time_registerPrimitives(void);
extern void sysbvm_tuple_registerPrimitives(void);
extern void sysbvm_type_registerPrimitives(void);

extern void sysbvm_array_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_orderedCollection_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_astInterpreter_setupASTInterpreter(sysbvm_context_t *context);
extern void sysbvm_boolean_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_bytecode_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_functionBytecodeDirectCompiler_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_byteStream_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_dictionary_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_errors_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_environment_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_exceptions_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_filesystem_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_float_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_function_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_integer_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_io_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_primitiveInteger_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_programEntity_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_set_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_sourcePosition_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_string_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_stringStream_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_time_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_tuple_setupPrimitives(sysbvm_context_t *context);
extern void sysbvm_type_setupPrimitives(sysbvm_context_t *context);

void sysbvm_context_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_identityEquals, "==");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_identityNotEquals, "~~");
    sysbvm_primitiveTable_registerFunction(sysbvm_tuple_primitive_identityHash, "identityHash");
    sysbvm_primitiveTable_registerFunction(sysbvm_string_primitive_equals, "String::=");
    sysbvm_primitiveTable_registerFunction(sysbvm_string_primitive_hash, "String::hash");

    sysbvm_array_registerPrimitives();
    sysbvm_orderedCollection_registerPrimitives();
    sysbvm_astInterpreter_registerPrimitives();
    sysbvm_boolean_registerPrimitives();
    sysbvm_bytecode_registerPrimitives();
    sysbvm_functionBytecodeDirectCompiler_registerPrimitives();
    sysbvm_byteStream_registerPrimitives();
    sysbvm_dictionary_registerPrimitives();
    sysbvm_errors_registerPrimitives();
    sysbvm_environment_registerPrimitives();
    sysbvm_exceptions_registerPrimitives();
    sysbvm_filesystem_registerPrimitives();
    sysbvm_float_registerPrimitives();
    sysbvm_function_registerPrimitives();
    sysbvm_integer_registerPrimitives();
    sysbvm_io_registerPrimitives();
    sysbvm_primitiveInteger_registerPrimitives();
    sysbvm_programEntity_registerPrimitives();
    sysbvm_set_registerPrimitives();
    sysbvm_sourcePosition_registerPrimitives();
    sysbvm_string_registerPrimitives();
    sysbvm_stringStream_registerPrimitives();
    sysbvm_time_registerPrimitives();
    sysbvm_tuple_registerPrimitives();
    sysbvm_type_registerPrimitives();
}

SYSBVM_API sysbvm_tuple_t sysbvm_context_createIntrinsicClass(sysbvm_context_t *context, const char *name, sysbvm_tuple_t supertype, ...)
{
    sysbvm_tuple_t nameSymbol = sysbvm_symbol_internWithCString(context, name);
    sysbvm_tuple_t type = sysbvm_type_createAnonymousClassAndMetaclass(context, supertype);
    sysbvm_namespace_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, nameSymbol, type);
    sysbvm_orderedCollection_add(context, context->roots.intrinsicTypes, type);

    // First pass: count the arguments.
    size_t slotNameCount = 0;
    va_list valist;
    va_start(valist, supertype);
    while(va_arg(valist, const char *))
    {
        va_arg(valist, sysbvm_typeSlotFlags_t);
        va_arg(valist, int);
        ++slotNameCount;
    }
    va_end(valist);

    // Second pass: make the argument list.
    size_t supertypeTotalSlotCount = 0;
    sysbvm_tuple_t actualSupertype = sysbvm_type_getSupertype(type);
    if(sysbvm_tuple_isNonNullPointer(actualSupertype))
        supertypeTotalSlotCount = sysbvm_type_getTotalSlotCount(actualSupertype);

    sysbvm_tuple_t slots = sysbvm_array_create(context, slotNameCount);
    va_start(valist, supertype);
    for(size_t i = 0; i < slotNameCount; ++i)
    {
        sysbvm_tuple_t slotName = sysbvm_symbol_internWithCString(context, va_arg(valist, const char *));
        sysbvm_tuple_t flags = sysbvm_tuple_bitflags_encode(va_arg(valist, int));
        sysbvm_tuple_t slotType = va_arg(valist, sysbvm_tuple_t);
        if(!slotType)
            slotType = context->roots.anyValueType;
        sysbvm_array_atPut(slots, i, sysbvm_typeSlot_create(context, type, slotName, flags, slotType, i, supertypeTotalSlotCount + i));
    }

    va_end(valist);
    sysbvm_type_setSlots(type, slots);
    sysbvm_type_setTotalSlotCount(context, type, supertypeTotalSlotCount + slotNameCount);
    sysbvm_type_buildSlotDictionary(context, type);

    return type;
}

SYSBVM_API sysbvm_tuple_t sysbvm_context_createIntrinsicPrimitiveValueType(sysbvm_context_t *context, const char *name, sysbvm_tuple_t supertype, size_t instanceSize, size_t instanceAlignment)
{
    sysbvm_tuple_t nameSymbol = sysbvm_symbol_internWithCString(context, name);
    sysbvm_tuple_t type = sysbvm_type_createAnonymousPrimitiveValueTypeAndValueMetatype(context, supertype);
    sysbvm_namespace_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, nameSymbol, type);
    sysbvm_orderedCollection_add(context, context->roots.intrinsicTypes, type);

    sysbvm_type_setSlots(type, sysbvm_array_create(context, 0));
    sysbvm_type_setTotalSlotCount(context, type, 0);
    sysbvm_type_setInstanceSizeAndAlignment(context, type, instanceSize, instanceAlignment);
    sysbvm_type_buildSlotDictionary(context, type);
    return type;
}

SYSBVM_API sysbvm_tuple_t sysbvm_context_createIntrinsicAbstractPrimitiveValueType(sysbvm_context_t *context, const char *name, sysbvm_tuple_t supertype, size_t instanceSize, size_t instanceAlignment)
{
    sysbvm_tuple_t nameSymbol = sysbvm_symbol_internWithCString(context, name);
    sysbvm_tuple_t type = sysbvm_type_createAnonymousAbstractPrimitiveValueTypeAndValueMetatype(context, supertype);
    sysbvm_namespace_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, nameSymbol, type);
    sysbvm_orderedCollection_add(context, context->roots.intrinsicTypes, type);

    sysbvm_type_setSlots(type, sysbvm_array_create(context, 0));
    sysbvm_type_setTotalSlotCount(context, type, 0);
    sysbvm_type_setInstanceSizeAndAlignment(context, type, instanceSize, instanceAlignment);
    sysbvm_type_buildSlotDictionary(context, type);
    return type;
}

SYSBVM_API sysbvm_tuple_t sysbvm_context_createIntrinsicType(sysbvm_context_t *context, const char *name, sysbvm_tuple_t supertype, ...)
{
    sysbvm_tuple_t nameSymbol = sysbvm_symbol_internWithCString(context, name);
    sysbvm_tuple_t type = sysbvm_type_createWithName(context, nameSymbol);
    if(supertype)
        sysbvm_type_setSupertype(type, supertype);
    sysbvm_environment_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, nameSymbol, type);
    sysbvm_orderedCollection_add(context, context->roots.intrinsicTypes, type);

    // First pass: count the arguments.
    size_t slotNameCount = 0;
    va_list valist;
    va_start(valist, supertype);
    while(va_arg(valist, const char *))
    {
        va_arg(valist, sysbvm_typeSlotFlags_t);
        va_arg(valist, int);
        ++slotNameCount;
    }
    va_end(valist);

    // Second pass: make the argument list.
    size_t supertypeTotalSlotCount = 0;
    sysbvm_tuple_t actualSupertype = sysbvm_type_getSupertype(type);
    if(sysbvm_tuple_isNonNullPointer(actualSupertype))
        supertypeTotalSlotCount = sysbvm_type_getTotalSlotCount(actualSupertype);

    sysbvm_tuple_t slots = sysbvm_array_create(context, slotNameCount);
    va_start(valist, supertype);
    for(size_t i = 0; i < slotNameCount; ++i)
    {
        sysbvm_tuple_t slotName = sysbvm_symbol_internWithCString(context, va_arg(valist, const char *));
        sysbvm_tuple_t flags = sysbvm_tuple_bitflags_encode(va_arg(valist, int));
        sysbvm_tuple_t slotType = va_arg(valist, sysbvm_tuple_t);
        if(!slotType)
            slotType = context->roots.anyValueType;
        sysbvm_array_atPut(slots, i, sysbvm_typeSlot_create(context, type, slotName, flags, slotType, i, supertypeTotalSlotCount + i));
    }

    va_end(valist);
    sysbvm_type_setSlots(type, slots);
    sysbvm_type_setTotalSlotCount(context, type, supertypeTotalSlotCount + slotNameCount);
    sysbvm_type_buildSlotDictionary(context, type);

    return type;
}

static void sysbvm_context_setIntrinsicTypeMetadata(sysbvm_context_t *context, sysbvm_tuple_t type, const char *name, sysbvm_tuple_t supertype, ...)
{
    sysbvm_tuple_t nameSymbol = sysbvm_symbol_internWithCString(context, name);
    if(supertype)
        sysbvm_type_setSupertype(type, supertype);
    sysbvm_namespace_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, nameSymbol, type);
    sysbvm_orderedCollection_add(context, context->roots.intrinsicTypes, type);

    // First pass: count the arguments.
    size_t slotNameCount = 0;
    va_list valist;
    va_start(valist, supertype);
    while(va_arg(valist, const char *))
    {
        va_arg(valist, sysbvm_typeSlotFlags_t);
        va_arg(valist, int);
        ++slotNameCount;
    }
    va_end(valist);

    // Second pass: make the argument list.
    size_t supertypeTotalSlotCount = 0;
    sysbvm_tuple_t actualSupertype = sysbvm_type_getSupertype(type);
    if(sysbvm_tuple_isNonNullPointer(actualSupertype))
        supertypeTotalSlotCount = sysbvm_type_getTotalSlotCount(actualSupertype);

    sysbvm_tuple_t slots = sysbvm_array_create(context, slotNameCount);
    va_start(valist, supertype);
    for(size_t i = 0; i < slotNameCount; ++i)
    {
        sysbvm_tuple_t slotName = sysbvm_symbol_internWithCString(context, va_arg(valist, const char *));
        sysbvm_tuple_t flags = sysbvm_tuple_bitflags_encode(va_arg(valist, int));
        sysbvm_tuple_t slotType = va_arg(valist, sysbvm_tuple_t);
        if(!slotType)
            slotType = context->roots.anyValueType;
        sysbvm_array_atPut(slots, i, sysbvm_typeSlot_create(context, type, slotName, flags, slotType, i, supertypeTotalSlotCount + i));
    }

    va_end(valist);
    sysbvm_type_setSlots(type, slots);
    sysbvm_type_setTotalSlotCount(context, type, supertypeTotalSlotCount + slotNameCount);
    sysbvm_type_buildSlotDictionary(context, type);
}

SYSBVM_API void sysbvm_context_setIntrinsicSymbolBindingValue(sysbvm_context_t *context, sysbvm_tuple_t symbol, sysbvm_tuple_t value)
{
    sysbvm_namespace_setNewSymbolBindingWithValue(context, context->roots.globalNamespace, symbol, value);

}

SYSBVM_API void sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(sysbvm_context_t *context, const char *symbolName, sysbvm_tuple_t binding)
{
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, symbolName), binding);
}

SYSBVM_API sysbvm_tuple_t sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(sysbvm_context_t *context, const char *symbolString, size_t argumentCount, sysbvm_bitflags_t flags, void *userdata, sysbvm_functionEntryPoint_t entryPoint)
{
    struct {
        sysbvm_tuple_t symbol;
        sysbvm_tuple_t primitiveFunction;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.symbol = sysbvm_symbol_internWithCString(context, symbolString);
    gcFrame.primitiveFunction = sysbvm_function_createPrimitive(context, argumentCount, flags, userdata, entryPoint);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, gcFrame.symbol, gcFrame.primitiveFunction);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.primitiveFunction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(sysbvm_context_t *context, const char *symbolString, sysbvm_tuple_t ownerClass, const char *selectorString, size_t argumentCount, sysbvm_bitflags_t flags, void *userdata, sysbvm_functionEntryPoint_t entryPoint)
{
    struct {
        sysbvm_tuple_t symbol;
        sysbvm_tuple_t selector;
        sysbvm_tuple_t primitiveFunction;
        sysbvm_tuple_t ownerClass;
    } gcFrame = {
        .ownerClass = ownerClass
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.primitiveFunction = sysbvm_function_createPrimitive(context, argumentCount, flags, userdata, entryPoint);
    if(selectorString)
    {
        gcFrame.selector = sysbvm_symbol_internWithCString(context, selectorString);
        sysbvm_type_setMethodWithSelector(context, gcFrame.ownerClass, gcFrame.selector, gcFrame.primitiveFunction);
    }

    if(symbolString)
    {
        gcFrame.symbol = sysbvm_symbol_internWithCString(context, symbolString);
        sysbvm_context_setIntrinsicSymbolBindingValue(context, gcFrame.symbol, gcFrame.primitiveFunction);
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.primitiveFunction;
}

SYSBVM_API sysbvm_tuple_t sysbvm_context_setIntrinsicPrimitiveMethod(sysbvm_context_t *context, sysbvm_tuple_t ownerClass, const char *selectorString, size_t argumentCount, sysbvm_bitflags_t flags, void *userdata, sysbvm_functionEntryPoint_t entryPoint)
{
    struct {
        sysbvm_tuple_t symbol;
        sysbvm_tuple_t selector;
        sysbvm_tuple_t primitiveFunction;
        sysbvm_tuple_t ownerClass;
    } gcFrame = {
        .ownerClass = ownerClass
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.primitiveFunction = sysbvm_function_createPrimitive(context, argumentCount, flags, userdata, entryPoint);
    gcFrame.selector = sysbvm_symbol_internWithCString(context, selectorString);
    sysbvm_type_setMethodWithSelector(context, gcFrame.ownerClass, gcFrame.selector, gcFrame.primitiveFunction);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.primitiveFunction;
}

static void sysbvm_context_createBasicTypes(sysbvm_context_t *context)
{
    // Make a circular base type.
    context->roots.untypedType = sysbvm_type_createAnonymousAndMetatype(context);
    context->roots.anyValueType = sysbvm_type_createAnonymousAndMetatype(context);
    context->roots.typeType = sysbvm_type_createAnonymous(context);
    sysbvm_tuple_setType((sysbvm_object_tuple_t*)context->roots.typeType, context->roots.typeType);
    sysbvm_type_setSupertype(context->roots.anyValueType, context->roots.untypedType);

    sysbvm_type_setFlags(context, context->roots.anyValueType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_DYNAMIC);
    sysbvm_type_setFlags(context, context->roots.untypedType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_DYNAMIC);
    sysbvm_type_setFlags(context, context->roots.typeType, SYSBVM_TYPE_FLAGS_NULLABLE);

    context->roots.objectType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.anyValueType);
    context->roots.lookupKeyType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.programEntityType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.lookupKeyType);
    context->roots.programEntityWithChildrenType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.programEntityType);
    sysbvm_type_setSupertype(context->roots.typeType, context->roots.programEntityWithChildrenType);
    sysbvm_type_setSupertype(sysbvm_tuple_getType(context, context->roots.lookupKeyType), sysbvm_tuple_getType(context, context->roots.objectType));
    sysbvm_type_setSupertype(sysbvm_tuple_getType(context, context->roots.programEntityType), sysbvm_tuple_getType(context, context->roots.lookupKeyType));
    sysbvm_type_setSupertype(sysbvm_tuple_getType(context, context->roots.programEntityWithChildrenType), sysbvm_tuple_getType(context, context->roots.programEntityType));

    context->roots.metatypeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.untypedType), context->roots.metatypeType);
    sysbvm_type_setSupertype(sysbvm_tuple_getType(context, context->roots.untypedType), context->roots.typeType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.anyValueType), context->roots.metatypeType);
    sysbvm_type_setSupertype(sysbvm_tuple_getType(context, context->roots.anyValueType), context->roots.typeType);

    context->roots.classType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);
    context->roots.metaclassType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.metatypeType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.objectType), context->roots.metaclassType);
    sysbvm_type_setSupertype(sysbvm_tuple_getType(context, context->roots.objectType), context->roots.classType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.lookupKeyType), context->roots.metaclassType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.programEntityType), context->roots.metaclassType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.programEntityWithChildrenType), context->roots.metaclassType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.metatypeType), context->roots.metaclassType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.classType), context->roots.metaclassType);

    sysbvm_tuple_setType((sysbvm_object_tuple_t*)sysbvm_tuple_getType(context, context->roots.metaclassType), context->roots.metaclassType);

    // Package type
    context->roots.packageType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.programEntityWithChildrenType);

    // Create the type slot class.
    context->roots.typeSlotType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.programEntityType);

    // Create the function class.
    context->roots.functionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.programEntityType);
    sysbvm_type_setFlags(context, context->roots.functionType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_FUNCTION);
    context->roots.functionDefinitionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.programEntityType);
    context->roots.functionSourceDefinitionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.functionSourceAnalyzedDefinitionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.functionBytecodeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.functionNativeCodeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.symbolValueBindingNativeCodeDefinitionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.orderedOffsetTableType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.orderedOffsetTableBuilderType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);

    context->roots.nativeCodeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.nativeCodeBindingLocationType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.nativeCodeProgramEntityType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.nativeCodeSectionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.nativeCodeSymbolType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.nativeCodeSymbolTableType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.nativeCodeStackMapType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);

    // Create the function type classes.
    context->roots.functionTypeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);
    context->roots.dependentFunctionTypeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.functionTypeType);
    context->roots.simpleFunctionTypeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.functionTypeType);

    context->roots.symbolBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.programEntityType);
    context->roots.symbolAnalysisBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolBindingType);
    context->roots.symbolArgumentBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolAnalysisBindingType);
    context->roots.symbolCaptureBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolAnalysisBindingType);
    context->roots.symbolLocalBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolAnalysisBindingType);
    context->roots.symbolMacroValueBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolBindingType);
    context->roots.symbolTupleSlotBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolAnalysisBindingType);
    context->roots.symbolValueBindingType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolBindingType);
    context->roots.environmentType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.programEntityWithChildrenType);
    context->roots.namespaceType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.environmentType);
    context->roots.astNodeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);

    // Collection base hierarchy
    context->roots.collectionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.hashedCollectionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.collectionType);
    context->roots.sequenceableCollectionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.collectionType);
    context->roots.arrayedCollectionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.sequenceableCollectionType);

    // Create the basic hash functions.
    context->roots.symbolType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.stringSymbolType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.symbolType);
    sysbvm_type_setFlags(context, context->roots.stringSymbolType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_BYTES);

    context->roots.dictionaryType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.hashedCollectionType);
    context->roots.weakKeyDictionaryType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.dictionaryType);
    context->roots.weakValueDictionaryType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.dictionaryType);
    context->roots.identityDictionaryType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.dictionaryType);
    context->roots.methodDictionaryType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.hashedCollectionType);

    context->roots.setType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.hashedCollectionType);
    context->roots.identitySetType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.setType);
    context->roots.weakSetType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.setType);
    context->roots.weakIdentitySetType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.weakSetType);

    context->roots.arrayType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayedCollectionType);
    context->roots.byteArrayType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayedCollectionType);
    context->roots.integerArrayType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayedCollectionType);
    context->roots.wordArrayType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayedCollectionType);
    context->roots.orderedCollectionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.sequenceableCollectionType);
    context->roots.weakArrayType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayType);
    context->roots.weakOrderedCollectionType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.orderedCollectionType);

    context->roots.pendingDefinitionFragmentsType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.pendingDefinitionFragmentType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);

    context->roots.gcLayoutBuilderType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.gcLayoutType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayedCollectionType);
    context->roots.nativeCodeCallFrameInformation = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.byteArrayType);
    context->roots.nativeCodeLocationListType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.byteArrayType);
    context->roots.nativeCodeRelocationTableType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.byteArrayType);
    context->roots.virtualTableLayoutType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.objectType);
    context->roots.virtualTableType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.arrayedCollectionType);

    context->roots.internedSymbolSet = sysbvm_identitySet_create(context);
    context->roots.sessionToken = sysbvm_tuple_systemHandle_encode(context, 1);

    context->roots.identityEqualsFunction = sysbvm_function_createPrimitive(context, 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_tuple_primitive_identityEquals);
    context->roots.identityNotEqualsFunction = sysbvm_function_createPrimitive(context, 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_tuple_primitive_identityNotEquals);
    context->roots.identityHashFunction = sysbvm_function_createPrimitive(context, 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL | SYSBVM_FUNCTION_FLAGS_ALWAYS_INLINE | SYSBVM_FUNCTION_FLAGS_WITHOUT_SOURCE_DEBUGGING, NULL, sysbvm_tuple_primitive_identityHash);
    context->roots.stringEqualsFunction = sysbvm_function_createPrimitive(context, 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_string_primitive_equals);
    context->roots.stringHashFunction = sysbvm_function_createPrimitive(context, 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_string_primitive_hash);

    // Tuple type classes
    context->roots.anySequenceTupleType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.sequenceableCollectionType);
    context->roots.sequenceTupleType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);

    // Create the intrinsic built-in environment
    context->roots.globalNamespace = sysbvm_namespace_create(context, SYSBVM_NULL_TUPLE);
    context->roots.intrinsicTypes = sysbvm_orderedCollection_create(context);

    context->roots.equalsSelector = sysbvm_symbol_internWithCString(context, "=");
    context->roots.hashSelector = sysbvm_symbol_internWithCString(context, "hash");
    context->roots.asStringSelector = sysbvm_symbol_internWithCString(context, "asString");
    context->roots.printStringSelector = sysbvm_symbol_internWithCString(context, "printString");
    context->roots.doesNotUnderstandSelector = sysbvm_symbol_internWithCString(context, "doesNotUnderstand:");

    context->roots.loadFromAtOffsetWithTypeSelector = sysbvm_symbol_internWithCString(context, "loadFrom:atOffset:withType:");
    context->roots.storeInAtOffsetWithTypeSelector = sysbvm_symbol_internWithCString(context, "store:in:atOffset:withType:");
    context->roots.assignmentSelector = sysbvm_symbol_internWithCString(context, ":=");
    context->roots.underscoreSelector = sysbvm_symbol_internWithCString(context, "_");
    context->roots.plusSelector = sysbvm_symbol_internWithCString(context, "+");
    context->roots.subscriptSelector = sysbvm_symbol_internWithCString(context, "[]:");
    context->roots.addressSelector = sysbvm_symbol_internWithCString(context, "address");
    context->roots.loadSelector = sysbvm_symbol_internWithCString(context, "load");
    context->roots.storeSelector = sysbvm_symbol_internWithCString(context, "store:");
    context->roots.refLoadSelector = sysbvm_symbol_internWithCString(context, "__refLoad__");
    context->roots.refStoreSelector = sysbvm_symbol_internWithCString(context, "__refStore__:");
    context->roots.tempRefAsRefSelector = sysbvm_symbol_internWithCString(context, "__tempRefAsRef__");

    context->roots.astNodeAnalysisSelector = sysbvm_symbol_internWithCString(context, "analyzeWithEnvironment:");
    context->roots.astNodeEvaluationSelector = sysbvm_symbol_internWithCString(context, "evaluateWithEnvironment:");
    context->roots.astNodeAnalysisAndEvaluationSelector = sysbvm_symbol_internWithCString(context, "analyzeAndEvaluateWithEnvironment:");
    context->roots.astNodeValidateThenAnalyzeAndEvaluateWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "validateThenAnalyzeAndEvaluateWithEnvironment:");
    context->roots.astNodeCompileIntoBytecodeSelector = sysbvm_symbol_internWithCString(context, "doCompileIntoBytecodeWith:");
    context->roots.ensureAnalysisSelector = sysbvm_symbol_internWithCString(context, "ensureAnalysis");
    
    context->roots.analyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeAndEvaluateMessageSendNode:forReceiver:withEnvironment:");
    context->roots.analyzeMessageSendNodeWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeMessageSendNode:withEnvironment:");
    context->roots.analyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeAndEvaluateMessageChainNode:forReceiver:withEnvironment:");
    context->roots.analyzeMessageChainNodeWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeMessageChainNode:withEnvironment:");
    context->roots.analyzeAndEvaluateConcreteMetaValueWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeAndEvaluateConcreteMetaValue:withEnvironment:");
    context->roots.analyzeConcreteMetaValueWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeConcreteMetaValue:withEnvironment:");
    context->roots.analyzeAndEvaluateUnexpandedApplicationNodeOfWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeAndEvaluateUnexpandedApplicationNode:of:withEnvironment:");
    context->roots.analyzeUnexpandedApplicationNodeWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeUnexpandedApplicationNode:withEnvironment:");

    context->roots.coerceASTNodeWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "coerceASTNode:withEnvironment:");
    context->roots.analyzeAndTypeCheckFunctionApplicationNodeWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeAndTypeCheckFunctionApplicationNode:withEnvironment:");
    context->roots.analyzeAndTypeCheckSolvedMessageSendNodeWithEnvironmentSelector = sysbvm_symbol_internWithCString(context, "analyzeAndTypeCheckSolvedMessageSendNode:withEnvironment:");
    context->roots.getOrCreateDependentApplicationValueForNodeSelector = sysbvm_symbol_internWithCString(context, "getOrCreateDependentApplicationValueForNode:");

    context->roots.applyWithoutArgumentsSelector = sysbvm_symbol_internWithCString(context, "()");
    context->roots.applyWithArgumentsSelector = sysbvm_symbol_internWithCString(context, "():");

    context->roots.primitiveNamedSelector = sysbvm_symbol_internWithCString(context, "primitive:");
    context->roots.keepSourceDefinitionSelector = sysbvm_symbol_internWithCString(context, "keepSourceDefinition");

    context->roots.coerceValueSelector = sysbvm_symbol_internWithCString(context, "coerceValue:");
    context->roots.defaultValueSelector = sysbvm_symbol_internWithCString(context, "defaultValue");

    context->roots.anyValueToVoidPrimitiveName = sysbvm_symbol_internWithCString(context, "Void::fromAnyValue");
    context->roots.pointerLikeLoadPrimitiveName = sysbvm_symbol_internWithCString(context, "PointerLikeType::load");
    context->roots.pointerLikeStorePrimitiveName = sysbvm_symbol_internWithCString(context, "PointerLikeType::store:");

    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "nil"), SYSBVM_NULL_TUPLE);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "false"), SYSBVM_FALSE_TUPLE);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "true"), SYSBVM_TRUE_TUPLE);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "void"), SYSBVM_VOID_TUPLE);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "__hashtableEmptyElement__"), SYSBVM_HASHTABLE_EMPTY_ELEMENT_TUPLE);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "__tombstone__"), SYSBVM_TOMBSTONE_TUPLE);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "__pendingMemoizationValue__"), SYSBVM_PENDING_MEMOIZATION_VALUE);

    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "__Global__"), context->roots.globalNamespace);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "BootstrapEnv::IntrinsicTypes"), context->roots.intrinsicTypes);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "BootstrapEnv::InternedSymbolSet"), context->roots.internedSymbolSet);

    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "Untyped::identityHash"), sysbvm_context_shallowCopy(context, context->roots.identityHashFunction));
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "Untyped::=="), sysbvm_context_shallowCopy(context, context->roots.identityEqualsFunction));
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "Untyped::~~"), sysbvm_context_shallowCopy(context, context->roots.identityNotEqualsFunction));
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "identityHash"), context->roots.identityHashFunction);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "=="), context->roots.identityEqualsFunction);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "~~"), context->roots.identityNotEqualsFunction);

    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "String::hash"), context->roots.stringHashFunction);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "String::="), context->roots.stringEqualsFunction);

    // Some basic method
    sysbvm_type_setHashFunction(context, context->roots.anyValueType, sysbvm_function_createPrimitive(context, 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_VIRTUAL, NULL, sysbvm_tuple_primitive_identityHash));
    sysbvm_type_setEqualsFunction(context, context->roots.anyValueType, sysbvm_function_createPrimitive(context, 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_VIRTUAL, NULL, sysbvm_tuple_primitive_identityEquals));
    
    sysbvm_type_setMethodWithSelector(context, context->roots.anyValueType, sysbvm_symbol_internWithCString(context, "identityHash"), context->roots.identityHashFunction);
    sysbvm_type_setMethodWithSelector(context, context->roots.anyValueType, sysbvm_symbol_internWithCString(context, "=="), context->roots.identityEqualsFunction);
    sysbvm_type_setMethodWithSelector(context, context->roots.anyValueType, sysbvm_symbol_internWithCString(context, "~~"), context->roots.identityNotEqualsFunction);

    // Create the value type classes.
    context->roots.valueType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.typeType);
    context->roots.valueMetatypeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.metatypeType);
    context->roots.primitiveValueType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.valueType);
    
    context->roots.anyPointerType = sysbvm_type_createAnonymous(context);
    context->roots.anyReferenceType = sysbvm_type_createAnonymous(context);
    sysbvm_type_setSupertype(context->roots.anyReferenceType, context->roots.untypedType);
    context->roots.anyTemporaryReferenceType = sysbvm_type_createAnonymous(context);
    sysbvm_type_setSupertype(context->roots.anyTemporaryReferenceType, context->roots.untypedType);

    context->roots.pointerLikeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.primitiveValueType);
    context->roots.pointerType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.pointerLikeType);
    context->roots.referenceLikeType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.pointerLikeType);
    context->roots.referenceType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.referenceLikeType);
    context->roots.temporaryReferenceType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.referenceLikeType);

    // Some basic types
    context->roots.voidType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Void", context->roots.anyValueType, 0, 1);

    context->roots.primitiveBooleanType = sysbvm_context_createIntrinsicAbstractPrimitiveValueType(context, "PrimitiveBoolean", context->roots.anyValueType, 0, 1);
    context->roots.primitiveNumberType = sysbvm_context_createIntrinsicAbstractPrimitiveValueType(context, "PrimitiveNumber", context->roots.anyValueType, 0, 1);
    context->roots.primitiveIntegerType = sysbvm_context_createIntrinsicAbstractPrimitiveValueType(context, "PrimitiveInteger", context->roots.primitiveNumberType, 0, 1);
    context->roots.primitiveCharacterType = sysbvm_context_createIntrinsicAbstractPrimitiveValueType(context, "PrimitiveCharacter", context->roots.primitiveIntegerType, 0, 1);
    context->roots.primitiveUnsignedIntegerType = sysbvm_context_createIntrinsicAbstractPrimitiveValueType(context, "PrimitiveUnsignedInteger", context->roots.primitiveIntegerType, 0, 1);
    context->roots.primitiveSignedIntegerType = sysbvm_context_createIntrinsicAbstractPrimitiveValueType(context, "PrimitiveSignedInteger", context->roots.primitiveIntegerType, 0, 1);
    context->roots.primitiveFloatType = sysbvm_context_createIntrinsicAbstractPrimitiveValueType(context, "PrimitiveFloat", context->roots.primitiveNumberType, 0, 1);

    context->roots.booleanType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Boolean", context->roots.primitiveBooleanType, 1, 1);
    context->roots.trueType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "True", context->roots.booleanType, 1, 1);
    context->roots.falseType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "False", context->roots.booleanType, 1, 1);

    context->roots.boolean8Type =  context->roots.booleanType;
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "Boolean8"), context->roots.boolean8Type);
    context->roots.boolean16Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Boolean16", context->roots.primitiveBooleanType, 2, 2);
    context->roots.boolean32Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Boolean32", context->roots.primitiveBooleanType, 4, 4);
    context->roots.boolean64Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Boolean64", context->roots.primitiveBooleanType, 8, 8);

    context->roots.char8Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Char8", context->roots.primitiveCharacterType, 1, 1);
    context->roots.uint8Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "UInt8", context->roots.primitiveUnsignedIntegerType, 1, 1);
    context->roots.int8Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Int8", context->roots.primitiveSignedIntegerType, 1, 1);

    context->roots.char16Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Char16", context->roots.primitiveCharacterType, 2, 2);
    context->roots.uint16Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "UInt16", context->roots.primitiveUnsignedIntegerType, 2, 2);
    context->roots.int16Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Int16", context->roots.primitiveSignedIntegerType, 2, 2);

    context->roots.char32Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Char32", context->roots.primitiveCharacterType, 4, 4);
    context->roots.uint32Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "UInt32", context->roots.primitiveUnsignedIntegerType, 4, 4);
    context->roots.int32Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Int32", context->roots.primitiveSignedIntegerType, 4, 4);

    context->roots.uint64Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "UInt64", context->roots.primitiveUnsignedIntegerType, 8, 8);
    context->roots.int64Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Int64", context->roots.primitiveSignedIntegerType, 8, 8);

    context->roots.float32Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Float32", context->roots.primitiveFloatType, 4, 4);
    context->roots.float64Type = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Float64", context->roots.primitiveFloatType, 8, 8);
    
    context->roots.bitflagsType = sizeof(sysbvm_bitflags_t) == 4 ? context->roots.uint32Type : context->roots.uint64Type;
    context->roots.systemHandleType = sizeof(sysbvm_systemHandle_t) == 4 ? context->roots.int32Type : context->roots.int64Type;

    context->roots.sizeType = context->targetWordSize == 4 ? context->roots.uint32Type : context->roots.uint64Type;
    context->roots.uintptrType = context->targetWordSize == 4 ? context->roots.uint32Type : context->roots.uint64Type;
    context->roots.intptrType = context->targetWordSize == 4 ? context->roots.int32Type : context->roots.int64Type;

    context->roots.integerType = sysbvm_context_createIntrinsicClass(context, "Integer", SYSBVM_NULL_TUPLE, NULL);
    context->roots.smallIntegerType = sysbvm_context_createIntrinsicClass(context, "SmallInteger", context->roots.integerType, NULL);
    context->roots.largeIntegerType = sysbvm_context_createIntrinsicClass(context, "LargeInteger", context->roots.integerType, NULL);
    context->roots.largePositiveIntegerType = sysbvm_context_createIntrinsicClass(context, "LargePositiveInteger", context->roots.largeIntegerType, NULL);
    context->roots.largeNegativeIntegerType = sysbvm_context_createIntrinsicClass(context, "LargeNegativeInteger", context->roots.largeIntegerType, NULL);

    context->roots.undefinedObjectType = sysbvm_context_createIntrinsicClass(context, "UndefinedObject", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.undefinedObjectType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_IMMEDIATE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);

    context->roots.pendingMemoizationValueType = sysbvm_context_createIntrinsicClass(context, "PendingMemoizationValue", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.pendingMemoizationValueType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_IMMEDIATE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);

    context->roots.tombstoneType = sysbvm_context_createIntrinsicClass(context, "ObjectTombstone", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.tombstoneType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_IMMEDIATE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);

    context->roots.stringType = sysbvm_context_createIntrinsicClass(context, "String", context->roots.arrayedCollectionType, NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.stringType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_BYTES | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);

    // Special types used during semantic analysis.
    context->roots.controlFlowEscapeType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "ControlFlowEscapeType", context->roots.voidType, 0, 1);
    context->roots.controlFlowBreakType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "ControlFlowBreakType", context->roots.controlFlowEscapeType, 0, 1);
    context->roots.controlFlowContinueType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "ControlFlowContinueType", context->roots.controlFlowEscapeType, 0, 1);
    context->roots.controlFlowReturnType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "ControlFlowReturnType", context->roots.controlFlowEscapeType, 0, 1);
    context->roots.noReturnType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "NoReturn", context->roots.controlFlowEscapeType, 0, 1);
    context->roots.unwindsType = sysbvm_context_createIntrinsicPrimitiveValueType(context, "Unwinds", context->roots.controlFlowEscapeType, 0, 1);

    context->roots.typeInferenceType = sysbvm_context_createIntrinsicClass(context, "TypeInferenceType", SYSBVM_NULL_TUPLE, NULL);
    context->roots.decayedTypeInferenceType = sysbvm_context_createIntrinsicClass(context, "DecayedTypeInferenceType", context->roots.typeInferenceType, NULL);
    context->roots.receiverTypeInferenceType = sysbvm_context_createIntrinsicClass(context, "ReceiverTypeInferenceType", context->roots.typeInferenceType, NULL);
    context->roots.directTypeInferenceType = sysbvm_context_createIntrinsicClass(context, "DirectTypeInferenceType", context->roots.typeInferenceType, NULL);

    // Set the name of the root basic type.
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.untypedType, "Untyped", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.anyValueType, "AnyValue", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.objectType, "Object", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.lookupKeyType, "LookupKey", SYSBVM_NULL_TUPLE,
        "key", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.untypedType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.programEntityType, "ProgramEntity", SYSBVM_NULL_TUPLE,
        "owner", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.programEntityType,
        "serialToken", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anyValueType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.programEntityWithChildrenType, "ProgramEntityWithChildren", SYSBVM_NULL_TUPLE,
        "children", SYSBVM_TYPE_SLOT_FLAG_PROTECTED, context->roots.orderedCollectionType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.packageType, "Package", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.typeType, "PendingDefinitionFragment", SYSBVM_NULL_TUPLE,
        "node", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "environment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.typeType, "PendingDefinitionFragments", SYSBVM_NULL_TUPLE,
        "supertype", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "valueType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "values", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "definitionsAndExtensions", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.typeType, "Type", SYSBVM_NULL_TUPLE,
        "supertype", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_NO_RTTI_EXCLUDED, context->roots.typeType,
        "slots", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.arrayType,
        "slotsWithBasicInitialization", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.arrayType,
        "allSlots", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.arrayType,
        "basicInitializeMethod", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_NO_RTTI_EXCLUDED, context->roots.functionType,
        "totalSlotCount", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anyValueType,
        "instanceSize", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "instanceAlignment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "flags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        "emptyTrivialSingleton", SYSBVM_TYPE_SLOT_FLAG_READONLY , context->roots.untypedType,

        "slotDictionary", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.methodDictionaryType,

        "macroMethodDictionary", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.methodDictionaryType,
        "methodDictionary", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.methodDictionaryType,
        "fallbackMethodDictionary", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.methodDictionaryType,

        "constructors", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.orderedCollectionType,
        "conversions", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.orderedCollectionType,

        "publicSymbolTable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.identityDictionaryType,

        "virtualMethodSelectorList", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.orderedCollectionType,
        "virtualTableLayout", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_TARGET_GENERATED | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.virtualTableLayoutType,
        "virtualTable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_TARGET_GENERATED, context->roots.virtualTableType,
        "gcLayout", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.gcLayoutType,
        "variableDataGCLayout", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.gcLayoutType,

        "pendingSlots", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, SYSBVM_NULL_TUPLE,
        "pendingDefinitionFragments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.pendingDefinitionFragmentsType,
        "subtypes", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.classType, "Class", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.metatypeType, "Metatype", SYSBVM_NULL_TUPLE,
        "thisType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_NO_RTTI_EXCLUDED, context->roots.typeType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.metaclassType, "Metaclass", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.typeSlotType, "TypeSlot", SYSBVM_NULL_TUPLE,
        "flags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        "type", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        "referenceType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        "localIndex", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "index", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "offset", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "initialValueBlock", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anyValueType,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.valueType, "ValueType", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.valueMetatypeType, "ValueMetatype", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.primitiveValueType, "PrimitiveValueType", SYSBVM_NULL_TUPLE, NULL);
    
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.anyPointerType, "AnyPointer", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.anyReferenceType, "AnyReference", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.anyTemporaryReferenceType, "AnyTemporaryReference", SYSBVM_NULL_TUPLE, NULL);

    context->roots.addressSpaceType = sysbvm_context_createIntrinsicClass(context, "AddressSpace", SYSBVM_NULL_TUPLE, NULL);
    context->roots.genericAddressSpaceType = sysbvm_context_createIntrinsicClass(context, "GenericAddressSpace", context->roots.addressSpaceType, NULL);
    sysbvm_type_setFlags(context, context->roots.genericAddressSpaceType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_EMPTY_TRIVIAL_SINGLETON);
    context->roots.memberAddressSpaceType = sysbvm_context_createIntrinsicClass(context, "MemberAddressSpace", context->roots.addressSpaceType, NULL);
    sysbvm_type_setFlags(context, context->roots.memberAddressSpaceType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_EMPTY_TRIVIAL_SINGLETON);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.pointerLikeType, "PointerLikeType", SYSBVM_NULL_TUPLE,
        "baseType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_NO_RTTI_EXCLUDED, context->roots.typeType,
        "addressSpace", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.addressSpaceType,
        "loadValueFunction", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.functionType,
        "storeValueFunction", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.functionType,
        SYSBVM_NULL_TUPLE);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.pointerType, "PointerType", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.referenceLikeType, "ReferenceLikeType", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.referenceType, "ReferenceType", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.temporaryReferenceType, "TemporaryReferenceType", SYSBVM_NULL_TUPLE, NULL);

    context->roots.analysisQueueEntryType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.valueType);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.analysisQueueEntryType, "AnalysisQueueEntry", context->roots.objectType,
        "programEntity", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.programEntityType,
        "nextEntry", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.analysisQueueEntryType,
        NULL);
    context->roots.analysisQueueType = sysbvm_context_createIntrinsicClass(context, "AnalysisQueue", context->roots.objectType,
        "firstEntry", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.analysisQueueEntryType,
        "lastEntry", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.analysisQueueEntryType,
        NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.environmentType, "Environment", SYSBVM_NULL_TUPLE,
        "parent", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "analysisQueue", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.analysisQueueType,
        "symbolTable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.identityDictionaryType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.namespaceType, "Namespace", SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.analysisAndEvaluationEnvironmentType = sysbvm_context_createIntrinsicClass(context, "AnalysisAndEvaluationEnvironment", context->roots.environmentType,
        "usedTuplesWithNamedSlots", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "analyzerToken", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.objectType,
        "expectedType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        "returnTarget", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "breakTarget", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "continueTarget", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.analysisEnvironmentType = sysbvm_context_createIntrinsicClass(context, "AnalysisEnvironment", context->roots.analysisAndEvaluationEnvironmentType,
        "hasBreakTarget", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "hasContinueTarget", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        NULL);
    context->roots.functionActivationEnvironmentType = sysbvm_context_createIntrinsicClass(context, "FunctionActivationEnvironment", context->roots.analysisAndEvaluationEnvironmentType,
        "function", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionType,
        "functionDefinition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "dependentFunctionType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionTypeType,
        "argumentVectorSize", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "captureVector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anySequenceTupleType,
        "valueVector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.functionAnalysisEnvironmentType = sysbvm_context_createIntrinsicClass(context, "FunctionAnalysisEnvironment", context->roots.analysisEnvironmentType,
        "functionDefinition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "captureBindingTable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.identityDictionaryType,
        "captureBindingList", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "argumentBindingList", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "localBindingList", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "pragmaList", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "innerFunctionList", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "primitiveName", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "keepSourceDefinition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "returnTypeExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.localAnalysisEnvironmentType = sysbvm_context_createIntrinsicClass(context, "LocalAnalysisEnvironment", context->roots.analysisEnvironmentType,
        NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolBindingType, "SymbolBinding", SYSBVM_NULL_TUPLE,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "type", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolAnalysisBindingType, "SymbolAnalysisBinding", SYSBVM_NULL_TUPLE,
        "ownerFunction", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "vectorIndex", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolArgumentBindingType, "SymbolArgumentBinding", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolCaptureBindingType, "SymbolCaptureBinding", SYSBVM_NULL_TUPLE,
        "sourceBinding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolAnalysisBindingType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolTupleSlotBindingType, "SymbolTupleSlotBinding", SYSBVM_NULL_TUPLE,
        "tupleBinding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolAnalysisBindingType,
        "typeSlot", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeSlotType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolLocalBindingType, "SymbolLocalBinding", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolMacroValueBindingType, "SymbolMacroValueBinding", SYSBVM_NULL_TUPLE,
        "expansion", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolValueBindingType, "SymbolValueBinding", SYSBVM_NULL_TUPLE,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.untypedType,
        "isMutable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "isExternal", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "isThreadLocal", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "nativeCodeDefinition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolValueBindingNativeCodeDefinitionType,
        "virtualAddress", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_TARGET_GENERATED, context->roots.uintptrType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.functionType, "Function", SYSBVM_NULL_TUPLE,
        "flags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        "argumentCount", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "captureVector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anySequenceTupleType,
        "captureEnvironment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.anyValueType,
        "definition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "primitiveTableIndex", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint32Type,
        "primitiveName", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_NO_RTTI_EXCLUDED, context->roots.symbolType,
        "memoizationTable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.weakValueDictionaryType,
        "annotations", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_NO_RTTI_EXCLUDED, context->roots.dictionaryType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.functionDefinitionType, "FunctionDefinition", SYSBVM_NULL_TUPLE,
        "flags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        "callingConventionName", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.symbolType,
        "argumentCount", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,

        "captureVectorType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sequenceTupleType,
        "type", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.functionTypeType,
        "primitiveName", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.symbolType,
        "pragmas", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.arrayType,
        "innerFunctions", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "annotations", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.dictionaryType,

        "sourceDefinition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionSourceDefinitionType,
        "sourceAnalyzedDefinition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED | SYSBVM_TYPE_SLOT_FLAG_NO_SOURCE_DEFINITION_EXCLUDED, context->roots.functionSourceAnalyzedDefinitionType,
        "bytecode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_BYTECODE, context->roots.functionBytecodeType,
        "optimizedBytecode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_BYTECODE, context->roots.functionBytecodeType,
        "nativeCodeDefinition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.functionNativeCodeType,
        "targetOptimizedBytecode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_BYTECODE, context->roots.functionBytecodeType,
        "targetNativeCodeDefinition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.functionNativeCodeType,

        "boxDescriptor", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "capturelessUncheckedEntryPoint", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uintptrType,
        "uncheckedEntryPoint", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uintptrType,
        "checkedEntryPoint", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uintptrType,
        NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.functionSourceDefinitionType, "FunctionSourceDefinition", SYSBVM_NULL_TUPLE,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "environment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "argumentNodes", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "resultTypeNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "bodyNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.functionSourceAnalyzedDefinitionType, "FunctionSourceAnalyzedDefinition", SYSBVM_NULL_TUPLE,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "environment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionAnalysisEnvironmentType,
        "captures", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "locals", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,

        "argumentNodes", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "resultTypeNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "bodyNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.functionBytecodeType, "FunctionBytecode", SYSBVM_NULL_TUPLE,
        "argumentCount", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "captureVectorSize", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "localVectorSize", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "literalVector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "instructions", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.byteArrayType,

        "definition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,

        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "captures", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "temporaryTypes", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,

        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "debugSourcePositions", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_DEBUG_INFORMATION | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.orderedOffsetTableType,
        "debugSourceEnvironments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_DEBUG_INFORMATION | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.orderedOffsetTableType,
        
        "jittedCode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_JIT_SPECIFIC, context->roots.systemHandleType,
        "jittedCodeWritePointer", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_JIT_SPECIFIC, context->roots.systemHandleType,
        "jittedCodeSessionToken", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_JIT_SPECIFIC, context->roots.systemHandleType,

        "jittedCodeTrampoline", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_JIT_SPECIFIC, context->roots.systemHandleType,
        "jittedCodeTrampolineWritePointer", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_JIT_SPECIFIC, context->roots.systemHandleType,
        "jittedCodeTrampolineSessionToken", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_JIT_SPECIFIC, context->roots.systemHandleType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.functionNativeCodeType, "FunctionNativeCodeDefinition", SYSBVM_NULL_TUPLE,
        "definition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "nativeCode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.nativeCodeType,
        "capturelessUncheckedEntryPoint", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint32Type,
        "uncheckedEntryPoint", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint32Type,
        "checkedEntryPoint", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint32Type,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolValueBindingNativeCodeDefinitionType, "SymbolValueBindingNativeCodeDefinition", SYSBVM_NULL_TUPLE,
        "binding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolValueBindingType,
        "nativeCode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.nativeCodeType,
        "symbol", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint32Type,
        NULL
    );

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.orderedOffsetTableType, "OrderedOffsetTable", SYSBVM_NULL_TUPLE,
        "keys", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.wordArrayType,
        "values", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.orderedOffsetTableBuilderType, "OrderedOffsetTableBuilder", SYSBVM_NULL_TUPLE,
        "keys", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "values", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.nativeCodeType, "NativeCode", SYSBVM_NULL_TUPLE,
        "symbolTable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.nativeCodeSymbolTableType,
        "sections", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "programEntities", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "programEntityImportedSymbols", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "functionDefinitionsEntryPoints", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "symbolValueBindingDefinitions", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "hirTextIR", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_NO_SOURCE_DEFINITION_EXCLUDED, context->roots.stringType,
        "mirTextIR", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_NO_SOURCE_DEFINITION_EXCLUDED, context->roots.stringType,
        "asmTextIR", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_NO_SOURCE_DEFINITION_EXCLUDED, context->roots.stringType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.nativeCodeProgramEntityType, "NativeCodeProgramEntity", SYSBVM_NULL_TUPLE,
        "name", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "sourceProgramEntity", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.programEntityType,
        "contentSymbol", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.nativeCodeSymbolType,
        "trampolineTarget", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.nativeCodeSymbolType,
        "stackMap", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.nativeCodeStackMapType,
        "debugSourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "debugSourceEnvironment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.nativeCodeSymbolType, "NativeCodeSymbol", SYSBVM_NULL_TUPLE,
        "name", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "section", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.nativeCodeSectionType,
        "objectValue", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.untypedType,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.intptrType,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "type", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint8Type,
        "visibility", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint8Type,
        "flags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint16Type,
        "virtualAddress", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_TARGET_GENERATED, context->roots.uintptrType,
        "writeableVirtualAddress", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_TARGET_GENERATED, context->roots.uintptrType,
        "gotVirtualAddress", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_TARGET_GENERATED, context->roots.uintptrType,
        "pltVirtualAddress", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_TARGET_GENERATED, context->roots.uintptrType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.nativeCodeSymbolTableType, "NativeCodeSymbolTable", SYSBVM_NULL_TUPLE,
        "symbols", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.nativeCodeSectionType, "NativeCodeSection", SYSBVM_NULL_TUPLE,
        "name", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "machoSectionName", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "machoSegmentName", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "symbolTable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.nativeCodeSymbolTableType,
        "callFrameInformations", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "relocations", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.nativeCodeRelocationTableType,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "alignment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "virtualAddress", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_TARGET_GENERATED, context->roots.uintptrType,
        "writeableVirtualAddress", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_TARGET_GENERATED, context->roots.uintptrType,
        "flags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint32Type,
        "data", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.byteArrayType,

        "debugSourcePositions", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_DEBUG_INFORMATION | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.orderedOffsetTableType,
        "debugSourceEnvironments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC| SYSBVM_TYPE_SLOT_FLAG_DEBUG_INFORMATION | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.orderedOffsetTableType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.nativeCodeBindingLocationType, "NativeCodeBindingLocation", SYSBVM_NULL_TUPLE,
        "binding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolBindingType,
        "isMutable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "location", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.nativeCodeLocationListType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.nativeCodeCallFrameInformation, "NativeCodeCallFrameInformation", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.nativeCodeCallFrameInformation, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_BYTES | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.nativeCodeStackMapType, "NativeCodeStackMap", SYSBVM_NULL_TUPLE,
        "frameBase", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.nativeCodeLocationListType,
        "captureBase", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.nativeCodeLocationListType,

        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "captures", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "locals", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.nativeCodeRelocationTableType, "NativeCodeRelocationTable", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.nativeCodeRelocationTableType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_BYTES | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.nativeCodeLocationListType, "NativeCodeLocationList", SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.nativeCodeLocationListType = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.byteArrayType);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.functionTypeType, "FunctionType", SYSBVM_NULL_TUPLE,
        "functionFlags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.dependentFunctionTypeType, "DependentFunctionType", SYSBVM_NULL_TUPLE,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, SYSBVM_NULL_TUPLE,
        "argumentNodes", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.arrayType,
        "resultTypeNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.astNodeType,

        "environment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.environmentType,
        "captureBindings", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.arrayType,
        "argumentBindings", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.arrayType,
        "localBindings", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.arrayType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.simpleFunctionTypeType, "SimpleFunctionType", SYSBVM_NULL_TUPLE,
        "argumentTypes", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.arrayType,
        "resultType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_MIN_RTTI_EXCLUDED, context->roots.typeType,
        NULL);

    // Tuple type classes
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.anySequenceTupleType, "AnySequenceTuple", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.sequenceTupleType, "SequenceTupleType", SYSBVM_NULL_TUPLE,
        "elementTypes", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.symbolType, "Symbol", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.stringSymbolType, "StringSymbol", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.setType, "Set", SYSBVM_NULL_TUPLE,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.identitySetType, "IdentitySet", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.weakSetType, "WeakSet", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.weakIdentitySetType, "WeakIdentitySet", SYSBVM_NULL_TUPLE,
        NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.collectionType, "Collection", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.hashedCollectionType, "HashedCollection", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.sequenceableCollectionType, "SequenceableCollection", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.arrayedCollectionType, "ArrayedCollection", SYSBVM_NULL_TUPLE, NULL);

    // 
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.arrayType, "Array", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.orderedCollectionType, "OrderedCollection", SYSBVM_NULL_TUPLE,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.weakArrayType, "WeakArray", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.weakOrderedCollectionType, "WeakOrderedCollection", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.arrayType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_FINAL | SYSBVM_TYPE_FLAGS_EMPTY_TRIVIAL_SINGLETON, SYSBVM_TYPE_FLAGS_FINAL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.orderedCollectionType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_NONE);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.weakArrayType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_FINAL | SYSBVM_TYPE_FLAGS_WEAK | SYSBVM_TYPE_FLAGS_EMPTY_TRIVIAL_SINGLETON, SYSBVM_TYPE_FLAGS_FINAL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.weakOrderedCollectionType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.virtualTableType, "VirtualTable", SYSBVM_NULL_TUPLE,
        "type", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.virtualTableLayoutType, "VirtualTableLayout", SYSBVM_NULL_TUPLE,
        "supertypeLayout", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.virtualTableLayoutType,
        "type", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "baseIndex", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "selectorToIndexTable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.methodDictionaryType,
        "newSelectors", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        NULL);

    // Create other root basic types.
    context->roots.arraySliceType = sysbvm_context_createIntrinsicClass(context, "ArraySlice", context->roots.sequenceableCollectionType,
        "elements", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "offset", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.arraySliceType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);
    context->roots.associationType = sysbvm_context_createIntrinsicClass(context, "Association", context->roots.lookupKeyType,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.untypedType,
        NULL);
    context->roots.weakValueAssociationType = sysbvm_context_createIntrinsicClass(context, "WeakValueAssociation", context->roots.lookupKeyType,
        //"value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_WEAK, SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.associationType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.byteArrayType, "ByteArray", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.byteArrayType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_BYTES | SYSBVM_TYPE_FLAGS_FINAL | SYSBVM_TYPE_FLAGS_EMPTY_TRIVIAL_SINGLETON, SYSBVM_TYPE_FLAGS_FINAL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.integerArrayType, "IntegerArray", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.integerArrayType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_BYTES | SYSBVM_TYPE_FLAGS_FINAL | SYSBVM_TYPE_FLAGS_EMPTY_TRIVIAL_SINGLETON, SYSBVM_TYPE_FLAGS_FINAL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.wordArrayType, "WordArray", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.wordArrayType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_BYTES | SYSBVM_TYPE_FLAGS_FINAL | SYSBVM_TYPE_FLAGS_EMPTY_TRIVIAL_SINGLETON, SYSBVM_TYPE_FLAGS_FINAL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.gcLayoutType, "GCLayout", SYSBVM_NULL_TUPLE,
        NULL
    );
    sysbvm_typeAndMetatype_setFlags(context, context->roots.gcLayoutType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_BYTES | SYSBVM_TYPE_FLAGS_FINAL | SYSBVM_TYPE_FLAGS_EMPTY_TRIVIAL_SINGLETON, SYSBVM_TYPE_FLAGS_FINAL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.gcLayoutBuilderType, "GCLayoutBuilder", SYSBVM_NULL_TUPLE,
        "repetitions", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint32Type,
        "strong", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "weak", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "fat", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        NULL
    );

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.dictionaryType, "Dictionary", SYSBVM_NULL_TUPLE,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.identityDictionaryType, "IdentityDictionary", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.weakKeyDictionaryType, "WeakKeyDictionary", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.weakValueDictionaryType, "WeakValueDictionary", SYSBVM_NULL_TUPLE,
        NULL);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.methodDictionaryType, "MethodDictionary", SYSBVM_NULL_TUPLE,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.generatedSymbolType = sysbvm_context_createIntrinsicClass(context, "GeneratedSymbol", context->roots.symbolType,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        NULL);
    context->roots.hashtableEmptyType = sysbvm_context_createIntrinsicClass(context, "HashtableEmpty", SYSBVM_NULL_TUPLE, NULL);
    sysbvm_typeAndMetatype_setFlags(context, context->roots.hashtableEmptyType, SYSBVM_TYPE_FLAGS_NULLABLE | SYSBVM_TYPE_FLAGS_IMMEDIATE | SYSBVM_TYPE_FLAGS_FINAL, SYSBVM_TYPE_FLAGS_FINAL);

    context->roots.macroContextType = sysbvm_context_createIntrinsicClass(context, "MacroContext", SYSBVM_NULL_TUPLE,
        "sourceNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "environment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        NULL);
    context->roots.messageType = sysbvm_context_createIntrinsicClass(context, "Message", SYSBVM_NULL_TUPLE,
        "selector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "lookupType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        NULL);
    context->roots.pragmaType = sysbvm_context_createIntrinsicClass(context, "Pragma", SYSBVM_NULL_TUPLE,
        "selector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.streamType = sysbvm_context_createIntrinsicClass(context, "Stream", SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.byteStreamType = sysbvm_context_createIntrinsicClass(context, "ByteStream", context->roots.streamType,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.byteArrayType,
        NULL);
    context->roots.stringStreamType = sysbvm_context_createIntrinsicClass(context, "StringStream", context->roots.streamType,
        "size", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "storage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringType,
        NULL);
    context->roots.valueBoxType = sysbvm_context_createIntrinsicClass(context, "ValueBox", SYSBVM_NULL_TUPLE,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.untypedType,
        NULL);
    context->roots.variableValueBoxType = sysbvm_context_createIntrinsicClass(context, "VariableValueBox", context->roots.valueBoxType,
        "binding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolBindingType,
        NULL);
    context->roots.defaultAnalysisQueueValueBox = (sysbvm_tuple_t)sysbvm_context_allocatePointerTuple(context, context->roots.valueBoxType, 1);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "__DefaultAnalysisQueueValueBox__"), context->roots.defaultAnalysisQueueValueBox);

    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "Bitflags"), context->roots.bitflagsType);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "SystemHandle"), context->roots.systemHandleType);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "Size"), context->roots.sizeType);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "UIntPointer"), context->roots.uintptrType);
    sysbvm_context_setIntrinsicSymbolBindingValue(context, sysbvm_symbol_internWithCString(context, "IntPointer"), context->roots.intptrType);

    context->roots.programmingLanguageType = sysbvm_context_createIntrinsicClass(context, "ProgrammingLanguage", SYSBVM_NULL_TUPLE,
        NULL);

    context->roots.sourceCodeType = sysbvm_context_createIntrinsicClass(context, "SourceCode", SYSBVM_NULL_TUPLE,
        "text", SYSBVM_TYPE_SLOT_FLAG_PUBLIC | SYSBVM_TYPE_SLOT_FLAG_NO_SOURCE_DEFINITION_EXCLUDED, context->roots.stringType,
        "directory", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringType,
        "name", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringType,
        "language", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringSymbolType,
        "isInFile", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "textSize", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "lineStartIndexTable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.sourcePositionType = sysbvm_context_createIntrinsicClass(context, "SourcePosition", SYSBVM_NULL_TUPLE,
        "sourceCode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourceCodeType,
        "startIndex", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint32Type,
        "endIndex", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint32Type,
        NULL);
    context->roots.tokenType = sysbvm_context_createIntrinsicClass(context, "SysmelToken", SYSBVM_NULL_TUPLE,
        "kind", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.int32Type,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anyValueType,
        NULL);

    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.astNodeType, "ASTNode", SYSBVM_NULL_TUPLE,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "analyzerToken", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "analyzedType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        NULL);
    context->roots.astArgumentNodeType = sysbvm_context_createIntrinsicClass(context, "ASTArgumentNode", context->roots.astNodeType,
        "isForAll", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "name", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "type", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "binding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.astBinaryExpressionSequenceNodeType = sysbvm_context_createIntrinsicClass(context, "ASTBinaryExpressionSequenceNode", context->roots.astNodeType,
        "elements", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astBreakNodeType = sysbvm_context_createIntrinsicClass(context, "ASTBreakNode", context->roots.astNodeType,
        NULL);
    context->roots.astCoerceValueNodeType = sysbvm_context_createIntrinsicClass(context, "ASTCoerceValueNode", context->roots.astNodeType,
        "typeExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "valueExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astCaseNodeType = sysbvm_context_createIntrinsicClass(context, "ASTCaseNode", context->roots.astNodeType,
        "keyExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "valueExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astContinueNodeType = sysbvm_context_createIntrinsicClass(context, "ASTContinueNode", context->roots.astNodeType,
        NULL);
    context->roots.astDoWhileContinueWithNodeType = sysbvm_context_createIntrinsicClass(context, "ASTDoWhileContinueWithNode", context->roots.astNodeType,
        "bodyExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "conditionExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "continueExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astDownCastNodeType = sysbvm_context_createIntrinsicClass(context, "ASTDownCastNode", context->roots.astNodeType,
        "typeExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "valueExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "isUnchecked", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        NULL);
    context->roots.astErrorNodeType = sysbvm_context_createIntrinsicClass(context, "ASTErrorNode", context->roots.astNodeType,
        "errorMessage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringType,
        NULL);
    context->roots.astFunctionApplicationNodeType = sysbvm_context_createIntrinsicClass(context, "ASTFunctionApplicationNode", context->roots.astNodeType,
        "functionExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "applicationFlags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        NULL);
    context->roots.astLambdaNodeType = sysbvm_context_createIntrinsicClass(context, "ASTLambdaNode", context->roots.astNodeType,
        "name", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "flags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        "callingConventionName", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "resultType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "body", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "hasLazyAnalysis", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anyValueType,
        "functionDefinition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionDefinitionType,
        "binding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        NULL);
    context->roots.astLexicalBlockNodeType = sysbvm_context_createIntrinsicClass(context, "ASTLexicalBlockNode", context->roots.astNodeType,
        "body", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "bodyEnvironment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        NULL);
    context->roots.astLiteralNodeType = sysbvm_context_createIntrinsicClass(context, "ASTLiteralNode", context->roots.astNodeType,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.untypedType,
        NULL);
    context->roots.astVariableDefinitionNodeType = sysbvm_context_createIntrinsicClass(context, "ASTVariableDefinitionNode", context->roots.astNodeType,
        "nameExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "typeExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "valueExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "binding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, SYSBVM_NULL_TUPLE,
        "isMacroSymbol", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "isMutable", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "isPublic", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "isExternal", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "isThreadLocal", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "analyzedValueType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        NULL);
    context->roots.astIdentifierReferenceNodeType = sysbvm_context_createIntrinsicClass(context, "ASTIdentifierReferenceNode", context->roots.astNodeType,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        "binding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolBindingType,
        NULL);
    context->roots.astIfNodeType = sysbvm_context_createIntrinsicClass(context, "ASTIfNode", context->roots.astNodeType,
        "conditionExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "trueExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "falseExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);

    context->roots.astMakeAssociationNodeType = sysbvm_context_createIntrinsicClass(context, "ASTMakeAssociationNode", context->roots.astNodeType,
        "key", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "value", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astMakeByteArrayNodeType = sysbvm_context_createIntrinsicClass(context, "ASTMakeByteArrayNode", context->roots.astNodeType,
        "elements", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astMakeDictionaryNodeType = sysbvm_context_createIntrinsicClass(context, "ASTMakeDictionaryNode", context->roots.astNodeType,
        "elements", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astMakeArrayNodeType = sysbvm_context_createIntrinsicClass(context, "ASTMakeArrayNode", context->roots.astNodeType,
        "elements", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);

    context->roots.astMessageSendNodeType = sysbvm_context_createIntrinsicClass(context, "ASTMessageSendNode", context->roots.astNodeType,
        "receiver", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "receiverLookupType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "selector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "isDynamic", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "boundMethod", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anyValueType,
        "boundMethodOwner", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeType,
        "applicationFlags", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.bitflagsType,
        NULL);
    context->roots.astMessageChainNodeType = sysbvm_context_createIntrinsicClass(context, "ASTMessageChainNode", context->roots.astNodeType,
        "receiver", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "receiverLookupType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "messages", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astMessageChainMessageNodeType = sysbvm_context_createIntrinsicClass(context, "ASTMessageChainMessageNode", context->roots.astNodeType,
        "selector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astPragmaNodeType = sysbvm_context_createIntrinsicClass(context, "ASTPragmaNode", context->roots.astNodeType,
        "selector", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astReturnNodeType = sysbvm_context_createIntrinsicClass(context, "ASTReturnNode", context->roots.astNodeType,
        "expression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astSequenceNodeType = sysbvm_context_createIntrinsicClass(context, "ASTSequenceNode", context->roots.astNodeType,
        "pragmas", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "expressions", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astSwitchNodeType = sysbvm_context_createIntrinsicClass(context, "ASTSwitchNode", context->roots.astNodeType,
        "expression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "caseExpressions", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "defaultExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astTupleSlotNamedAtNodeType = sysbvm_context_createIntrinsicClass(context, "ASTTupleSlotNamedAtNode", context->roots.astNodeType,
        "tupleExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "nameExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "boundSlot", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeSlotType,
        NULL);
    context->roots.astTupleSlotNamedAtPutNodeType = sysbvm_context_createIntrinsicClass(context, "ASTTupleSlotNamedAtPutNode", context->roots.astNodeType,
        "tupleExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "nameExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "valueExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "boundSlot", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeSlotType,
        NULL);
    context->roots.astTupleSlotNamedReferenceAtNodeType = sysbvm_context_createIntrinsicClass(context, "ASTTupleSlotNamedReferenceAtNode", context->roots.astNodeType,
        "tupleExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "nameExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "boundSlot", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.typeSlotType,
        NULL);
    context->roots.astTupleWithLookupStartingFromNodeType = sysbvm_context_createIntrinsicClass(context, "ASTTupleWithLookupStartingFromNode", context->roots.astNodeType,
        "tupleExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "typeExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astUnexpandedApplicationNodeType = sysbvm_context_createIntrinsicClass(context, "ASTUnexpandedApplicationNode", context->roots.astNodeType,
        "functionOrMacroExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astUnexpandedSExpressionNodeType = sysbvm_context_createIntrinsicClass(context, "ASTUnexpandedSExpressionNode", context->roots.astNodeType,
        "elements", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.astUseNamedSlotsOfNodeType = sysbvm_context_createIntrinsicClass(context, "ASTUseNamedSlotsOfNode", context->roots.astNodeType,
        "tupleExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "binding", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolLocalBindingType,
        NULL);
    context->roots.astWhileContinueWithNodeType = sysbvm_context_createIntrinsicClass(context, "ASTWhileContinueWithNode", context->roots.astNodeType,
        "conditionExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "bodyExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "continueExpression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);

    context->roots.astQuoteNodeType = sysbvm_context_createIntrinsicClass(context, "ASTQuoteNode", context->roots.astNodeType,
        "node", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astQuasiQuoteNodeType = sysbvm_context_createIntrinsicClass(context, "ASTQuasiQuoteNode", context->roots.astNodeType,
        "node", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);
    context->roots.astQuasiUnquoteNodeType = sysbvm_context_createIntrinsicClass(context, "ASTQuasiUnquoteNode", context->roots.astNodeType,
        "expression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "astTemplateParameterIndex", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        NULL);
    context->roots.astSpliceNodeType = sysbvm_context_createIntrinsicClass(context, "ASTSpliceNode", context->roots.astNodeType,
        "expression", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        "astTemplateParameterIndex", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        NULL);

    context->roots.functionBytecodeAssemblerAbstractOperand = sysbvm_context_createIntrinsicClass(context, "FunctionBytecodeAssemblerAbstractOperand", context->roots.objectType,
        "name", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.symbolType,
        NULL);
    context->roots.functionBytecodeAssemblerAbstractInstruction = sysbvm_type_createAnonymousClassAndMetaclass(context, context->roots.functionBytecodeAssemblerAbstractOperand);
    sysbvm_context_setIntrinsicTypeMetadata(context, context->roots.functionBytecodeAssemblerAbstractInstruction, "FunctionBytecodeAssemblerAbstractInstruction", SYSBVM_NULL_TUPLE,
        "pc", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "endPC", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,
        "previous", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionBytecodeAssemblerAbstractInstruction,
        "next", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionBytecodeAssemblerAbstractInstruction,

        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "sourceEnvironment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "sourceASTNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,
        NULL);

    context->roots.functionBytecodeAssemblerLabel = sysbvm_context_createIntrinsicClass(context, "FunctionBytecodeAssemblerLabel", context->roots.functionBytecodeAssemblerAbstractInstruction,
        NULL);
    context->roots.functionBytecodeAssemblerInstruction = sysbvm_context_createIntrinsicClass(context, "FunctionBytecodeAssemblerInstruction", context->roots.functionBytecodeAssemblerAbstractInstruction,
        "standardOpcode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.uint8Type,
        "operands", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        NULL);
    context->roots.functionBytecodeAssemblerVectorOperand = sysbvm_context_createIntrinsicClass(context, "FunctionBytecodeAssemblerVectorOperand", context->roots.functionBytecodeAssemblerAbstractOperand,
        "index", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.int16Type,
        "vectorType", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.int16Type,

        "hasAllocaDestination", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "hasNonAllocaDestination", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "hasSlotReferenceAtDestination", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "hasNonSlotReferenceAtDestination", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,

        "hasLoadStoreUsage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        "hasNonLoadStoreUsage", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        
        "allocaPointerRankIsLowered", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,

        "optimizationTupleOperand", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anyValueType,
        "optimizationTypeSlotOperand", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anyValueType,
        NULL);
    context->roots.functionBytecodeAssembler = sysbvm_context_createIntrinsicClass(context, "FunctionBytecodeAssembler", context->roots.objectType,
        "arguments", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "captures", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.arrayType,
        "literals", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "literalDictionary", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.identityDictionaryType,
        "temporaries", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "temporaryTypes", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.orderedCollectionType,
        "usedTemporaryCount", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sizeType,

        "firstInstruction", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionBytecodeAssemblerAbstractInstruction,
        "lastInstruction", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionBytecodeAssemblerAbstractInstruction,

        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        "sourceEnvironment", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.environmentType,
        "sourceASTNode", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.astNodeType,

        NULL);
    context->roots.functionBytecodeCompiler = sysbvm_context_createIntrinsicClass(context, "FunctionBytecodeCompiler", context->roots.objectType,
        NULL);
    context->roots.functionBytecodeDirectCompiler = sysbvm_context_createIntrinsicClass(context, "FunctionBytecodeDirectCompiler", context->roots.functionBytecodeCompiler,
        "assembler", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionBytecodeAssembler,
        "bindingDictionary", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.methodDictionaryType,

        "breakLabel", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionBytecodeAssemblerAbstractInstruction,
        "continueLabel", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.functionBytecodeAssemblerAbstractInstruction,

        NULL);
    context->roots.functionNativeCodeCompiler = sysbvm_context_createIntrinsicClass(context, "FunctionNativeCodeCompiler", context->roots.objectType,
        NULL);

    context->roots.exceptionType = sysbvm_context_createIntrinsicClass(context, "Exception", context->roots.objectType,
        "messageText", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.stringType,
        "innerException", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.exceptionType,
        "sourcePosition", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.sourcePositionType,
        NULL);
    context->roots.errorType = sysbvm_context_createIntrinsicClass(context, "Error", context->roots.exceptionType, NULL);
    context->roots.argumentsCountMismatchType = sysbvm_context_createIntrinsicClass(context, "ArgumentsCountMismatch", context->roots.errorType, NULL);
    context->roots.arithmeticErrorType = sysbvm_context_createIntrinsicClass(context, "ArithmeticError", context->roots.errorType, NULL);
    context->roots.compilationErrorType = sysbvm_context_createIntrinsicClass(context, "CompilationError", context->roots.errorType, NULL);
    context->roots.evaluationErrorType = sysbvm_context_createIntrinsicClass(context, "EvaluationError", context->roots.compilationErrorType, NULL);
    context->roots.parseErrorType = sysbvm_context_createIntrinsicClass(context, "ParseError", context->roots.compilationErrorType, NULL);
    context->roots.scanErrorType = sysbvm_context_createIntrinsicClass(context, "ScanError", context->roots.compilationErrorType, NULL);
    context->roots.domainErrorType = sysbvm_context_createIntrinsicClass(context, "DomainError", context->roots.arithmeticErrorType, NULL);
    context->roots.semanticAnalysisErrorType = sysbvm_context_createIntrinsicClass(context, "SemanticAnalysisError", context->roots.compilationErrorType, NULL);
    context->roots.zeroDivideType = sysbvm_context_createIntrinsicClass(context, "ZeroDivide", context->roots.zeroDivideType,
        "dividend", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.anyValueType,
        NULL);
    context->roots.assertionFailureType = sysbvm_context_createIntrinsicClass(context, "AssertionFailure", context->roots.errorType,
        NULL);
    context->roots.cannotReturnType = sysbvm_context_createIntrinsicClass(context, "CannotReturn", context->roots.errorType,
        "result", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.untypedType,
        NULL);
    context->roots.modificationForbiddenType = sysbvm_context_createIntrinsicClass(context, "ModificationForbidden", context->roots.errorType,
        "object", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.untypedType,
        NULL);
    context->roots.messageNotUnderstoodType = sysbvm_context_createIntrinsicClass(context, "MessageNotUnderstood", context->roots.errorType,
        "message", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.messageType,
        "receiver", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.untypedType,
        "reachedDefaultHandler", SYSBVM_TYPE_SLOT_FLAG_PUBLIC, context->roots.booleanType,
        NULL);

    // Fill the immediate type table.
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_NIL] = context->roots.undefinedObjectType;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_INTEGER] = context->roots.smallIntegerType;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_CHAR8] = context->roots.char8Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_UINT8] = context->roots.uint8Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_INT8] = context->roots.int8Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_CHAR16] = context->roots.char16Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_UINT16] = context->roots.uint16Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_INT16] = context->roots.int16Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_CHAR32] = context->roots.char32Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_UINT32] = context->roots.uint32Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_INT32] = context->roots.int32Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_UINT64] = context->roots.uint64Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_INT64] = context->roots.int64Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_FLOAT32] = context->roots.float32Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_FLOAT64] = context->roots.float64Type;
    context->roots.immediateTypeTable[SYSBVM_TUPLE_TAG_TRIVIAL] = context->roots.undefinedObjectType;

    // Fill the immediate trivial type table.
    context->roots.immediateTrivialTypeTable[SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_FALSE] = context->roots.falseType;
    context->roots.immediateTrivialTypeTable[SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_TRUE] = context->roots.trueType;
    context->roots.immediateTrivialTypeTable[SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_VOID] = context->roots.voidType;
    context->roots.immediateTrivialTypeTable[SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_HASHTABLE_EMPTY_ELEMENT] = context->roots.hashtableEmptyType;
    context->roots.immediateTrivialTypeTable[SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_TOMBSTONE] = context->roots.tombstoneType;
    context->roots.immediateTrivialTypeTable[SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_INDEX_PENDING_MEMOIZATION_VALUE] = context->roots.pendingMemoizationValueType;
}

SYSBVM_API sysbvm_context_t *sysbvm_context_createWithOptions(sysbvm_contextCreationOptions_t *contextOptions)
{
    sysbvm_context_t *context = (sysbvm_context_t*)calloc(1, sizeof(sysbvm_context_t));
    context->targetWordSize = contextOptions->targetWordSize ? contextOptions->targetWordSize : sizeof(void*);
    context->identityHashSeed = 1;
    context->jitEnabled = sysbvm_context_default_jitEnabled && !contextOptions->nojit;
    context->gcDisabled = contextOptions->gcType == SYSBVM_GC_TYPE_DISABLED;
    sysbvm_dynarray_initialize(&context->jittedObjectFileEntries, sizeof(sysbvm_gdb_jit_code_entry_t*), 1024);
    sysbvm_dynarray_initialize(&context->jittedRegisteredFrames, sizeof(void*), 1024);
    sysbvm_dynarray_initialize(&context->markingStack, sizeof(sysbvm_tuple_t), 1<<20);

    sysbvm_heap_initialize(&context->heap);
    context->analyzeASTWithEnvironmentPIC = (sysbvm_pic_t*)sysbvm_chunkedAllocator_allocate(&context->heap.picTableAllocator, sizeof(sysbvm_pic_t), sizeof(uintptr_t));
    context->evaluateASTWithEnvironment = (sysbvm_pic_t*)sysbvm_chunkedAllocator_allocate(&context->heap.picTableAllocator, sizeof(sysbvm_pic_t), sizeof(uintptr_t));
    context->evaluateAndAnalyzeASTWithEnvironment = (sysbvm_pic_t*)sysbvm_chunkedAllocator_allocate(&context->heap.picTableAllocator, sizeof(sysbvm_pic_t), sizeof(uintptr_t));
    sysbvm_gc_lock(context);

    sysbvm_context_createBasicTypes(context);

    {
        if(!contextOptions->buildArchitectureName)
            contextOptions->buildArchitectureName = sysbvm_system_getArchitectureName();
        if(!contextOptions->buildVendorName)
            contextOptions->buildVendorName = sysbvm_system_getVendorName();
        if(!contextOptions->buildOSName)
            contextOptions->buildOSName = sysbvm_system_getOSName();
        if(!contextOptions->buildAbiName)
            contextOptions->buildAbiName = sysbvm_system_getAbiName();
        if(!contextOptions->buildObjectFileName)
            contextOptions->buildObjectFileName = sysbvm_system_getObjectFileName();
        if(!contextOptions->buildDebugInformationFormatName)
            contextOptions->buildDebugInformationFormatName = sysbvm_system_getDebugInformationFormatName();
        if(!contextOptions->buildExceptionHandlingTableFormatName)
            contextOptions->buildExceptionHandlingTableFormatName = sysbvm_system_getExceptionHandlingTableFormatName();

        if(!contextOptions->hostArchitectureName)
            contextOptions->hostArchitectureName = contextOptions->buildArchitectureName;
        if(!contextOptions->hostVendorName)
            contextOptions->hostVendorName = contextOptions->buildVendorName;
        if(!contextOptions->hostOSName)
            contextOptions->hostOSName = contextOptions->buildOSName;
        if(!contextOptions->hostAbiName)
            contextOptions->hostAbiName = contextOptions->buildAbiName;
        if(!contextOptions->hostObjectFileName)
            contextOptions->hostObjectFileName = contextOptions->buildObjectFileName;
        if(!contextOptions->hostDebugInformationFormatName)
            contextOptions->hostDebugInformationFormatName = contextOptions->buildDebugInformationFormatName;
        if(!contextOptions->hostExceptionHandlingTableFormatName)
            contextOptions->hostExceptionHandlingTableFormatName = contextOptions->buildExceptionHandlingTableFormatName;

        if(!contextOptions->targetArchitectureName)
            contextOptions->targetArchitectureName = contextOptions->hostArchitectureName;
        if(!contextOptions->targetVendorName)
            contextOptions->targetVendorName = contextOptions->hostVendorName;
        if(!contextOptions->targetOSName)
            contextOptions->targetOSName = contextOptions->hostOSName;
        if(!contextOptions->targetAbiName)
            contextOptions->targetAbiName = contextOptions->hostAbiName;
        if(!contextOptions->targetObjectFileName)
            contextOptions->targetObjectFileName = contextOptions->hostObjectFileName;
        if(!contextOptions->targetDebugInformationFormatName)
            contextOptions->targetDebugInformationFormatName = contextOptions->hostDebugInformationFormatName;
        if(!contextOptions->targetExceptionHandlingTableFormatName)
            contextOptions->targetExceptionHandlingTableFormatName = contextOptions->hostExceptionHandlingTableFormatName;
        
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__BuildArchitecture__", sysbvm_symbol_internWithCString(context, contextOptions->buildArchitectureName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__BuildVendor__", sysbvm_symbol_internWithCString(context, contextOptions->buildVendorName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__BuildOS__", sysbvm_symbol_internWithCString(context, contextOptions->buildOSName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__BuildAbi__", sysbvm_symbol_internWithCString(context, contextOptions->buildAbiName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__BuildObjectFile__", sysbvm_symbol_internWithCString(context, contextOptions->buildObjectFileName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__BuildDebugInformationFormat__", sysbvm_symbol_internWithCString(context, contextOptions->buildDebugInformationFormatName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__BuildExceptionHandlingTableFormat__", sysbvm_symbol_internWithCString(context, contextOptions->buildExceptionHandlingTableFormatName));

        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__HostArchitecture__", sysbvm_symbol_internWithCString(context, contextOptions->hostArchitectureName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__HostVendor__", sysbvm_symbol_internWithCString(context, contextOptions->hostVendorName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__HostOS__", sysbvm_symbol_internWithCString(context, contextOptions->hostOSName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__HostAbi__", sysbvm_symbol_internWithCString(context, contextOptions->hostAbiName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__HostObjectFile__", sysbvm_symbol_internWithCString(context, contextOptions->hostObjectFileName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__HostDebugInformationFormat__", sysbvm_symbol_internWithCString(context, contextOptions->hostDebugInformationFormatName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__HostExceptionHandlingTableFormat__", sysbvm_symbol_internWithCString(context, contextOptions->hostExceptionHandlingTableFormatName));

        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__TargetArchitecture__", sysbvm_symbol_internWithCString(context, contextOptions->targetArchitectureName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__TargetVendor__", sysbvm_symbol_internWithCString(context, contextOptions->targetVendorName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__TargetOS__", sysbvm_symbol_internWithCString(context, contextOptions->targetOSName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__TargetAbi__", sysbvm_symbol_internWithCString(context, contextOptions->targetAbiName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__TargetObjectFile__", sysbvm_symbol_internWithCString(context, contextOptions->targetObjectFileName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__TargetDebugInformationFormat__", sysbvm_symbol_internWithCString(context, contextOptions->targetDebugInformationFormatName));
        sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "__TargetExceptionHandlingTableFormat__", sysbvm_symbol_internWithCString(context, contextOptions->targetExceptionHandlingTableFormatName));
    }
    
    sysbvm_array_setupPrimitives(context);
    sysbvm_orderedCollection_setupPrimitives(context);
    sysbvm_astInterpreter_setupASTInterpreter(context);
    sysbvm_boolean_setupPrimitives(context);
    sysbvm_bytecode_setupPrimitives(context);
    sysbvm_functionBytecodeDirectCompiler_setupPrimitives(context);
    sysbvm_byteStream_setupPrimitives(context);
    sysbvm_dictionary_setupPrimitives(context);
    sysbvm_errors_setupPrimitives(context);
    sysbvm_environment_setupPrimitives(context);
    sysbvm_exceptions_setupPrimitives(context);
    sysbvm_filesystem_setupPrimitives(context);
    sysbvm_float_setupPrimitives(context);
    sysbvm_function_setupPrimitives(context);
    sysbvm_integer_setupPrimitives(context);
    sysbvm_io_setupPrimitives(context);
    sysbvm_primitiveInteger_setupPrimitives(context);
    sysbvm_programEntity_setupPrimitives(context);
    sysbvm_set_setupPrimitives(context);
    sysbvm_sourcePosition_setupPrimitives(context);
    sysbvm_string_setupPrimitives(context);
    sysbvm_stringStream_setupPrimitives(context);
    sysbvm_time_setupPrimitives(context);
    sysbvm_tuple_setupPrimitives(context);
    sysbvm_type_setupPrimitives(context);
    
    sysbvm_gc_unlock(context);

    return context;
}

SYSBVM_API sysbvm_context_t *sysbvm_context_create(void)
{
    sysbvm_contextCreationOptions_t emptyOptions = {0};
    return sysbvm_context_createWithOptions(&emptyOptions);
}

SYSBVM_API void sysbvm_context_destroy(sysbvm_context_t *context)
{
    if(!context) return;

    // Unregister the jitted object codes.
    {
        sysbvm_gdb_jit_code_entry_t **registeredEntries = (sysbvm_gdb_jit_code_entry_t**)context->jittedObjectFileEntries.data;
        for(size_t i = 0; i < context->jittedObjectFileEntries.size; ++i)
        {
            sysbvm_gdb_jit_code_entry_t *entry = registeredEntries[i];
            sysbvm_gdb_unregisterObjectFile(entry);
            free(entry);
        }
        sysbvm_dynarray_destroy(&context->jittedObjectFileEntries);
    }

    // Unregister the jitted registered frames.
    {
        void **registeredFrames = (void **)context->jittedRegisteredFrames.data;
        for(size_t i = 0; i < context->jittedRegisteredFrames.size; ++i)
        {
#ifdef _WIN32
#else
            __deregister_frame(registeredFrames[i]);
#endif
        }
        sysbvm_dynarray_destroy(&context->jittedRegisteredFrames);
    }

    // Destroy the context heap.
    sysbvm_dynarray_destroy(&context->markingStack);
    sysbvm_heap_destroy(&context->heap);
    free(context);
}

SYSBVM_API sysbvm_context_t *sysbvm_context_loadImageFromFileNamed(const char *filename)
{
#if 0
#ifdef _WIN32
    FILE *inputFile = NULL;
    if(fopen_s(&inputFile, filename, "rb"))
        return NULL;
#else
    FILE *inputFile = fopen(filename, "rb");
#endif
    if(!inputFile)
        return NULL;

    char magic[4] = {0};
    if(fread(magic, 4, 1, inputFile) != 1 || memcmp(magic, "TVIM", 4))
    {
        fclose(inputFile);
        return NULL;
    }

    sysbvm_context_t *context = (sysbvm_context_t*)calloc(1, sizeof(sysbvm_context_t));
    if(fread(&context->targetWordSize, sizeof(context->targetWordSize), 1, inputFile) != 1 ||
        fread(&context->identityHashSeed, sizeof(context->identityHashSeed), 1, inputFile) != 1 ||
        fread(&context->roots, sizeof(context->roots), 1, inputFile) != 1 ||
        !sysbvm_heap_loadFromFile(&context->heap, inputFile, sizeof(context->roots) / sizeof(sysbvm_tuple_t), (sysbvm_tuple_t*)&context->roots))
    {
        fclose(inputFile);
        free(context);
        return NULL;
    }
    
    context->jitEnabled = sysbvm_context_default_jitEnabled;

    fclose(inputFile);

    context->roots.sessionToken = sysbvm_tuple_systemHandle_encode(context, sysbvm_tuple_systemHandle_decode(context->roots.sessionToken) +  1);
    return context;
#else
    (void)filename;
    return NULL;
#endif
}

SYSBVM_API void sysbvm_context_saveImageToFileNamed(sysbvm_context_t *context, const char *filename)
{
#if 0    
    sysbvm_gc_collect(context);
#ifdef _WIN32
    FILE *outputFile = NULL;
    if(fopen_s(&outputFile, filename, "wb"))
        return;
#else
    FILE *outputFile = fopen(filename, "wb");
#endif
    fwrite("TVIM", 4, 1, outputFile);
    fwrite(&context->targetWordSize, sizeof(context->targetWordSize), 1, outputFile);
    fwrite(&context->identityHashSeed, sizeof(context->identityHashSeed), 1, outputFile);
    fwrite(&context->roots, sizeof(context->roots), 1, outputFile);
    sysbvm_heap_dumpToFile(&context->heap, outputFile);
    fclose(outputFile);
#else
    (void)context;
    (void)filename;
#endif
}

sysbvm_heap_t *sysbvm_context_getHeap(sysbvm_context_t *context)
{
    if(!context) return 0;

    return &context->heap;
}

static size_t sysbvm_context_generateIdentityHash(sysbvm_context_t *context)
{
    context->identityHashSeed = sysbvm_hashMultiply(context->identityHashSeed) + 12345;
    return context->identityHashSeed & SYSBVM_HASH_BIT_MASK;
}

sysbvm_object_tuple_t *sysbvm_context_allocateByteTuple(sysbvm_context_t *context, sysbvm_tuple_t type, size_t byteSize)
{
    if(!context) return 0;

    sysbvm_object_tuple_t *result = sysbvm_heap_allocateByteTuple(&context->heap, byteSize);
    sysbvm_tuple_setIdentityHash(result, sysbvm_context_generateIdentityHash(context));
    if(result)
        sysbvm_tuple_setType(result, type);
    return result;
}

sysbvm_object_tuple_t *sysbvm_context_allocatePointerTuple(sysbvm_context_t *context, sysbvm_tuple_t type, size_t slotCount)
{
    if(!context) return 0;

    sysbvm_object_tuple_t *result = sysbvm_heap_allocatePointerTuple(&context->heap, slotCount);
    sysbvm_tuple_setIdentityHash(result, sysbvm_context_generateIdentityHash(context));
    if(result)
        sysbvm_tuple_setType(result, type);
    return result;
}

/**
 * Print memory usage statistics.
 */
SYSBVM_API void sysbvm_context_printMemoryUsageStats(sysbvm_context_t *context)
{
    if(!context)
        return;

    printf("Heap Size: %lld\n", (long long)context->heap.totalSize);
}

SYSBVM_API sysbvm_tuple_t sysbvm_context_shallowCopy(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    if(!sysbvm_tuple_isNonNullPointer(tuple))
        return tuple;

    sysbvm_object_tuple_t *result = sysbvm_heap_shallowCopyTuple(&context->heap, (sysbvm_object_tuple_t*)tuple);
    sysbvm_tuple_setIdentityHash(result, sysbvm_context_generateIdentityHash(context));
    return (sysbvm_tuple_t)result;    
}
