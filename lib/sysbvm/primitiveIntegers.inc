#if !IS_CHARACTER
static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_printString)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

#if IS_SIGNED
    return sysbvm_primitiveInteger_signed_printString(context, primitiveInteger_decode(arguments[0]));
#else
    return sysbvm_primitiveInteger_unsigned_printString(context, primitiveInteger_decode(arguments[0]));
#endif
}
#endif

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_fromInteger)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

#if IS_SIGNED
    integer_t value = (integer_t)sysbvm_tuple_integer_decodeInt64(context, arguments[0]);
#else 
    integer_t value = (integer_t)sysbvm_tuple_integer_decodeUInt64(context, arguments[0]);
#endif
    return primitiveInteger_encode(value);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asInteger)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    integer_t value = (integer_t)primitiveInteger_decode(arguments[0]);
#if IS_SIGNED
    return sysbvm_tuple_integer_encodeInt64(context, value);
#else
    return sysbvm_tuple_integer_encodeUInt64(context, value);
#endif
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_add)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left + right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_subtract)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left - right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_negated)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    integer_t operand = primitiveInteger_decode(arguments[0]);
    return primitiveInteger_encode(-operand);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_bitInvert)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    integer_t operand = primitiveInteger_decode(arguments[0]);
    return primitiveInteger_encode(~operand);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_highBit)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    integer_t operand = primitiveInteger_decode(arguments[0]);
    return primitiveInteger_encode((integer_t)primitiveInteger_highBit(operand));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_lowBit)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    integer_t operand = primitiveInteger_decode(arguments[0]);
    return primitiveInteger_encode((integer_t)primitiveInteger_lowBit(operand));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_multiply)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left * right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_divide)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    if(right == 0)
        sysbvm_error("Division by zero");
    return primitiveInteger_encode(left / right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_remainder)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    if(right == 0)
        sysbvm_error("Division by zero");
    return primitiveInteger_encode(left % right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_bitAnd)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left & right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_bitOr)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left | right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_bitXor)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left ^ right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_shiftLeft)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left << right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_shiftRight)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return primitiveInteger_encode(left >> right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_equals)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left == right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_notEquals)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left != right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_lessThan)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left < right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_lessEquals)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left <= right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_greaterThan)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left > right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_greaterEquals)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    integer_t left = primitiveInteger_decode(arguments[0]);
    integer_t right = primitiveInteger_decode(arguments[1]);
    return sysbvm_tuple_boolean_encode(left >= right);
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asBoolean)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_boolean_encode((uint8_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asBoolean16)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_boolean16_encode(context, (uint8_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asBoolean32)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_boolean32_encode(context, (uint8_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asBoolean64)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_boolean64_encode(context, (uint8_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt8)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint8_encode((uint8_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asInt8)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int8_encode((int8_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asChar8)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_char8_encode((sysbvm_char8_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt16)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint16_encode((uint16_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asInt16)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int16_encode((int16_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asChar16)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_char16_encode((sysbvm_char16_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt32)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint32_encode(context, (uint32_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asInt32)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int32_encode(context, (int32_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asChar32)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_char32_encode(context, (sysbvm_char32_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt64)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_uint64_encode(context, (uint64_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asInt64)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_int64_encode(context, (int64_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asFloat32)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_float32_encode(context, (sysbvm_float32_t)primitiveInteger_decode(arguments[0]));
}

static sysbvm_tuple_t PRIMITIVE_INTEGER_FUNCTION(primitive_asFloat64)(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_tuple_float64_encode(context, (sysbvm_float64_t)primitiveInteger_decode(arguments[0]));
}

static void PRIMITIVE_INTEGER_FUNCTION(registerPrimitives)(void)
{
#if !IS_CHARACTER
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_printString), INTEGER_TYPE_NAME "::printString");
#endif
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_fromInteger), "Integer::as" INTEGER_TYPE_NAME);
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asInteger), INTEGER_TYPE_NAME "::asInteger");

    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_negated), INTEGER_TYPE_NAME "::negated");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_bitInvert), INTEGER_TYPE_NAME "::bitInvert");

    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_highBit), INTEGER_TYPE_NAME "::highBit");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_lowBit), INTEGER_TYPE_NAME "::lowBit");

    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_add), INTEGER_TYPE_NAME "::+");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_subtract), INTEGER_TYPE_NAME "::-");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_multiply), INTEGER_TYPE_NAME "::*");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_divide), INTEGER_TYPE_NAME "::/");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_remainder), INTEGER_TYPE_NAME "::%");

    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_bitAnd), INTEGER_TYPE_NAME "::&");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_bitOr), INTEGER_TYPE_NAME "::|");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_bitXor), INTEGER_TYPE_NAME "::^");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_shiftLeft), INTEGER_TYPE_NAME "::<<");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_shiftRight), INTEGER_TYPE_NAME "::>>");

    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_equals), INTEGER_TYPE_NAME "::=");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_notEquals), INTEGER_TYPE_NAME "::~=");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_lessThan), INTEGER_TYPE_NAME "::<");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_lessEquals), INTEGER_TYPE_NAME "::<=");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_greaterThan), INTEGER_TYPE_NAME "::>");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_greaterEquals), INTEGER_TYPE_NAME "::>=");

    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asBoolean),   INTEGER_TYPE_NAME "::asBoolean");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asBoolean16), INTEGER_TYPE_NAME "::asBoolean16");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asBoolean32), INTEGER_TYPE_NAME "::asBoolean32");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asBoolean64), INTEGER_TYPE_NAME "::asBoolean64");

    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt8), INTEGER_TYPE_NAME "::asUInt8");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asInt8), INTEGER_TYPE_NAME "::asInt8");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asChar8), INTEGER_TYPE_NAME "::asChar8");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt16), INTEGER_TYPE_NAME "::asUInt16");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asInt16), INTEGER_TYPE_NAME "::asInt16");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asChar16), INTEGER_TYPE_NAME "::asChar16");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt32), INTEGER_TYPE_NAME "::asUInt32");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asInt32), INTEGER_TYPE_NAME "::asInt32");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asChar32), INTEGER_TYPE_NAME "::asChar32");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt64), INTEGER_TYPE_NAME "::asUInt64");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asInt64), INTEGER_TYPE_NAME "::asInt64");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asFloat32), INTEGER_TYPE_NAME "::asFloat32");
    sysbvm_primitiveTable_registerFunction(PRIMITIVE_INTEGER_FUNCTION(primitive_asFloat64), INTEGER_TYPE_NAME "::asFloat64");
}

static void PRIMITIVE_INTEGER_FUNCTION(setupPrimitives)(sysbvm_context_t *context)
{
#if !IS_CHARACTER
    sysbvm_type_setPrintStringFunction(context, context->roots.INTEGER_TYPE_ROOT_NAME, sysbvm_function_createPrimitive(context, 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_printString)));
#endif
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, INTEGER_TYPE_SHORT_SUFFIX_NAME, 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_fromInteger));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.integerType, "as" INTEGER_TYPE_NAME, 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_fromInteger));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asInteger", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asInteger));

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "negated", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_negated));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "bitInvert", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_bitInvert));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "highBit", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_highBit));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "lowBit", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_lowBit));

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "+", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_add));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "-", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_subtract));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "*", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_multiply));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "/", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_divide));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "//", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_divide));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "%", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_remainder));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "\\\\", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_remainder));

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "&", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_bitAnd));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "|", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_bitOr));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "^", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_bitXor));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "<<", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_shiftLeft));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, ">>", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_shiftRight));

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_equals));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "~=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_notEquals));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "<", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_lessThan));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "<=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_lessEquals));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, ">", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_greaterThan));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, ">=", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_greaterEquals));

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asBoolean",   1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asBoolean));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asBoolean16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asBoolean16));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asBoolean32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asBoolean32));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asBoolean64", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asBoolean64));

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asUInt8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt8));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asInt8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asInt8));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asChar8", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asChar8));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asUInt16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt16));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asInt16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asInt16));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asChar16", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asChar16));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asUInt32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt32));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asInt32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asInt32));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asChar32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asChar32));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asUInt64", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt64));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asInt64", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asInt64));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asFloat32", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asFloat32));
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asFloat64", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asFloat64));

    if(context->roots.sizeType == context->roots.uint32Type)
    {
        sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asUIntPointer", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt32));
        sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asIntPointer", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asInt32));
        sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asSize", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt32));
    }
    else
    {
        sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asUIntPointer", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt64));
        sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asIntPointer", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asInt64));
        sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.INTEGER_TYPE_ROOT_NAME, "asSize", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE | SYSBVM_FUNCTION_FLAGS_PURE | SYSBVM_FUNCTION_FLAGS_FINAL, NULL, PRIMITIVE_INTEGER_FUNCTION(primitive_asUInt64));
    }
}

#undef integer_t
#undef primitiveInteger_decode
#undef primitiveInteger_encode
#undef primitiveInteger_highBit
#undef primitiveInteger_lowBit
#undef IS_CHARACTER
#undef IS_SIGNED
#undef FUNCTION_PREFIX
#undef INTEGER_TYPE_NAME
#undef INTEGER_TYPE_ROOT_NAME
#undef INTEGER_TYPE_SHORT_SUFFIX_NAME