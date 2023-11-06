#include "sysbvm/boolean.h"
#include "sysbvm/errors.h"
#include "sysbvm/function.h"
#include "sysbvm/string.h"
#include "sysbvm/type.h"
#include "internal/context.h"

SYSBVM_API sysbvm_boolean16_t sysbvm_tuple_boolean16_decode(sysbvm_tuple_t tuple)
{
    sysbvm_object_tuple_t *booleanObject = (sysbvm_object_tuple_t*)tuple;
    return *((sysbvm_boolean16_t*)booleanObject->bytes);
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_boolean16_encode(sysbvm_context_t *context, sysbvm_boolean16_t value)
{
    sysbvm_object_tuple_t *result = sysbvm_context_allocateByteTuple(context, context->roots.boolean16Type, sizeof(sysbvm_boolean16_t));
    *((sysbvm_boolean16_t*)result->bytes) = value;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_boolean32_t sysbvm_tuple_boolean32_decode(sysbvm_tuple_t tuple)
{
    sysbvm_object_tuple_t *booleanObject = (sysbvm_object_tuple_t*)tuple;
    return *((sysbvm_boolean32_t*)booleanObject->bytes);
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_boolean32_encode(sysbvm_context_t *context, sysbvm_boolean32_t value)
{
    sysbvm_object_tuple_t *result = sysbvm_context_allocateByteTuple(context, context->roots.boolean32Type, sizeof(sysbvm_boolean32_t));
    *((sysbvm_boolean32_t*)result->bytes) = value;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_boolean64_t sysbvm_tuple_boolean64_decode(sysbvm_tuple_t tuple)
{
    sysbvm_object_tuple_t *booleanObject = (sysbvm_object_tuple_t*)tuple;
    return *((sysbvm_boolean64_t*)booleanObject->bytes);
}

SYSBVM_API sysbvm_tuple_t sysbvm_tuple_boolean64_encode(sysbvm_context_t *context, sysbvm_boolean64_t value)
{
    sysbvm_object_tuple_t *result = sysbvm_context_allocateByteTuple(context, context->roots.boolean64Type, sizeof(sysbvm_boolean64_t));
    *((sysbvm_boolean64_t*)result->bytes) = value;
    return (sysbvm_tuple_t)result;
}

static sysbvm_tuple_t sysbvm_boolean_primitive_not(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_boolean_encode(!sysbvm_tuple_boolean_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_boolean_primitive_xor(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    return sysbvm_tuple_boolean_encode(sysbvm_tuple_boolean_decode(arguments[0]) ^ sysbvm_tuple_boolean_decode(arguments[1]));
}

static sysbvm_tuple_t sysbvm_boolean_primitive_asUInt8(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint8_encode(sysbvm_tuple_boolean_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_boolean_primitive_asInt8(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int8_encode(sysbvm_tuple_boolean_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_boolean16_primitive_asUInt16(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint16_encode(sysbvm_tuple_boolean16_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_boolean16_primitive_asInt16(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int16_encode(sysbvm_tuple_boolean16_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_boolean32_primitive_asUInt32(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint32_encode(context, sysbvm_tuple_boolean32_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_boolean32_primitive_asInt32(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int32_encode(context, sysbvm_tuple_boolean32_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_boolean64_primitive_asUInt64(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint64_encode(context, sysbvm_tuple_boolean64_decode(arguments[0]));
}

static sysbvm_tuple_t sysbvm_boolean64_primitive_asInt64(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int64_encode(context, sysbvm_tuple_boolean64_decode(arguments[0]));
}

void sysbvm_boolean_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_boolean_primitive_not, "Boolean::not");
    sysbvm_primitiveTable_registerFunction(sysbvm_boolean_primitive_xor, "Boolean::|");

    sysbvm_primitiveTable_registerFunction(sysbvm_boolean_primitive_asUInt8,    "Boolean::asUInt8");
    sysbvm_primitiveTable_registerFunction(sysbvm_boolean_primitive_asInt8,     "Boolean::asInt8");
    sysbvm_primitiveTable_registerFunction(sysbvm_boolean16_primitive_asUInt16, "Boolean::asUInt16");
    sysbvm_primitiveTable_registerFunction(sysbvm_boolean16_primitive_asInt16,  "Boolean::asInt16");
    sysbvm_primitiveTable_registerFunction(sysbvm_boolean32_primitive_asUInt32, "Boolean::asUInt32");
    sysbvm_primitiveTable_registerFunction(sysbvm_boolean32_primitive_asInt32,  "Boolean::asInt32");
    sysbvm_primitiveTable_registerFunction(sysbvm_boolean64_primitive_asUInt64, "Boolean::asUInt64");
    sysbvm_primitiveTable_registerFunction(sysbvm_boolean64_primitive_asInt64,  "Boolean::asInt64");
}

void sysbvm_boolean_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.booleanType, "not", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_boolean_primitive_not);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.booleanType, "|", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_boolean_primitive_xor);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.booleanType, "asUInt8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_boolean_primitive_asUInt8);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.booleanType, "asInt8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_boolean_primitive_asInt8);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.boolean16Type, "asUInt16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_boolean16_primitive_asUInt16);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.boolean16Type, "asInt16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_boolean16_primitive_asInt16);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.boolean32Type, "asUInt32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_boolean32_primitive_asUInt32);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.boolean32Type, "asInt32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_boolean32_primitive_asInt32);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.boolean64Type, "asUInt64", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_boolean64_primitive_asUInt64);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.boolean64Type, "asInt64", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, sysbvm_boolean64_primitive_asInt64);
}
