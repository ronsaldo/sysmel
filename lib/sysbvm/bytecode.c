#include "sysbvm/bytecode.h"
#include "sysbvm/bytecodeJit.h"
#include "sysbvm/array.h"
#include "sysbvm/assert.h"
#include "sysbvm/association.h"
#include "sysbvm/dictionary.h"
#include "sysbvm/context.h"
#include "sysbvm/function.h"
#include "sysbvm/environment.h"
#include "sysbvm/message.h"
#include "sysbvm/pic.h"
#include "sysbvm/gc.h"
#include "sysbvm/orderedOffsetTable.h"
#include "sysbvm/type.h"
#include "sysbvm/stackFrame.h"
#include "internal/context.h"
#ifdef _WIN32
#define alloca _alloca
#else
#include <alloca.h>
#endif
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>


static bool sysbvm_bytecodeInterpreter_tablesAreFilled;
uint8_t sysbvm_implicitVariableBytecodeOperandCountTable[16];

void sysbvm_bytecodeInterpreter_ensureTablesAreFilled(void)
{
    if(sysbvm_bytecodeInterpreter_tablesAreFilled)
        return;

    sysbvm_implicitVariableBytecodeOperandCountTable[SYSBVM_OPCODE_CALL >> 4] = 2;
    sysbvm_implicitVariableBytecodeOperandCountTable[SYSBVM_OPCODE_UNCHECKED_CALL >> 4] = 2;
    sysbvm_implicitVariableBytecodeOperandCountTable[SYSBVM_OPCODE_SEND >> 4] = 3;
    sysbvm_implicitVariableBytecodeOperandCountTable[SYSBVM_OPCODE_SEND_WITH_LOOKUP >> 4] = 4;

    sysbvm_implicitVariableBytecodeOperandCountTable[SYSBVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS >> 4] = 1;
    sysbvm_implicitVariableBytecodeOperandCountTable[SYSBVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS >> 4] = 1;
    sysbvm_implicitVariableBytecodeOperandCountTable[SYSBVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES >> 4] = 2;
    sysbvm_implicitVariableBytecodeOperandCountTable[SYSBVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS >> 4] = 1;
    sysbvm_implicitVariableBytecodeOperandCountTable[SYSBVM_OPCODE_MAKE_TUPLE_WITH_ELEMENTS >> 4] = 1;

    sysbvm_implicitVariableBytecodeOperandCountTable[SYSBVM_OPCODE_CASE_JUMP >> 4] = 2;

    sysbvm_bytecodeInterpreter_tablesAreFilled = true;
}

SYSBVM_API uint8_t sysbvm_bytecodeInterpreter_destinationOperandCountForOpcode(uint8_t opcode)
{
    switch(opcode)
    {
    case SYSBVM_OPCODE_ALLOCA:
    case SYSBVM_OPCODE_MOVE:
    case SYSBVM_OPCODE_LOAD:
    case SYSBVM_OPCODE_LOAD_SYMBOL_VALUE_BINDING:
    case SYSBVM_OPCODE_ALLOCA_WITH_VALUE:
    case SYSBVM_OPCODE_COERCE_VALUE:
    case SYSBVM_OPCODE_DOWNCAST_VALUE:
    case SYSBVM_OPCODE_UNCHECKED_DOWNCAST_VALUE:
    case SYSBVM_OPCODE_MAKE_ASSOCIATION:
    case SYSBVM_OPCODE_SLOT_AT:
    case SYSBVM_OPCODE_SLOT_REFERENCE_AT:
    case SYSBVM_OPCODE_REF_SLOT_AT:
    case SYSBVM_OPCODE_REF_SLOT_REFERENCE_AT:
    case SYSBVM_OPCODE_CALL:
    case SYSBVM_OPCODE_UNCHECKED_CALL:
    case SYSBVM_OPCODE_SEND:
    case SYSBVM_OPCODE_SEND_WITH_LOOKUP:
    case SYSBVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS:
    case SYSBVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS:
    case SYSBVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES:
    case SYSBVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS:
    case SYSBVM_OPCODE_MAKE_TUPLE_WITH_ELEMENTS:
        return 1;
    default:
        return 0;
    }
}

SYSBVM_API uint8_t sysbvm_bytecodeInterpreter_offsetOperandCountForOpcode(uint8_t opcode)
{
    switch(opcode)
    {
    case SYSBVM_OPCODE_JUMP:
    case SYSBVM_OPCODE_JUMP_IF_TRUE:
    case SYSBVM_OPCODE_JUMP_IF_FALSE:
    case SYSBVM_OPCODE_CASE_JUMP:
        return 1;
    default: return 0;
    }
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_functionApplyNoCopyArguments(sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments, sysbvm_bitflags_t applicationFlags)
{
    if((applicationFlags & (SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK | SYSBVM_FUNCTION_APPLICATION_FLAGS_VARIADIC_EXPANDED)) == 0 &&
        sysbvm_function_isVariadic(context, function))
    {
        size_t expectedArgumentCount = sysbvm_function_getArgumentCount(context, function);
        SYSBVM_ASSERT(expectedArgumentCount > 0);

        // Move the variadic arguments into a variadic vector.
        {
            size_t directArgumentCount = expectedArgumentCount - 1;
            if(argumentCount < directArgumentCount)
                sysbvm_error("Missing required arguments.");

            size_t variadicArgumentCount = argumentCount - directArgumentCount;
            sysbvm_tuple_t variadicVector = sysbvm_array_create(context, variadicArgumentCount);
            sysbvm_tuple_t *variadicVectorElements = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(variadicVector)->pointers;
            for(size_t i = 0; i < variadicArgumentCount; ++i)
                variadicVectorElements[i] = arguments[directArgumentCount + i];
            arguments[directArgumentCount] = variadicVector;
        }

        return sysbvm_function_apply(context, function, expectedArgumentCount, arguments, applicationFlags);

    }
    return sysbvm_function_apply(context, function, argumentCount, arguments, applicationFlags);
}

static sysbvm_tuple_t sysbvm_bytecodeInterpreter_functionApply(sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments, sysbvm_bitflags_t applicationFlags)
{
    sysbvm_tuple_t argumentsBuffer[16];
    memcpy(argumentsBuffer, arguments, argumentCount * sizeof(sysbvm_tuple_t));

    return sysbvm_bytecodeInterpreter_functionApplyNoCopyArguments(context, function, argumentCount, argumentsBuffer, applicationFlags);
}

static sysbvm_tuple_t sysbvm_bytecodeInterpreter_interpretSend(sysbvm_context_t *context, sysbvm_tuple_t receiverType, sysbvm_tuple_t selector, size_t argumentCount, sysbvm_tuple_t *receiverAndArguments)
{
    sysbvm_tuple_t method = sysbvm_type_lookupSelector(context, receiverType, selector);
    if(method)
        return sysbvm_bytecodeInterpreter_functionApply(context, method, argumentCount + 1, receiverAndArguments, 0);

    // Attempt to send doesNotUnderstand:
    if(selector != context->roots.doesNotUnderstandSelector)
        method = sysbvm_type_lookupSelector(context, receiverType, selector);
    if(!method)
        sysbvm_error("Message not understood");

    // Make the message.
    sysbvm_tuple_t arguments = sysbvm_array_create(context, argumentCount);
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_array_atPut(arguments, i, receiverAndArguments[1 + i]);

    sysbvm_tuple_t message = sysbvm_message_create(context, selector, arguments, receiverType, SYSBVM_NULL_TUPLE);
    return sysbvm_function_apply2(context, method, receiverAndArguments[0], message);
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_interpretSendWithReceiverTypeNoCopyArguments(sysbvm_context_t *context, sysbvm_pic_t *pic, sysbvm_tuple_t receiverType, sysbvm_tuple_t selector, size_t argumentCount, sysbvm_tuple_t *receiverAndArguments, sysbvm_bitflags_t applicationFlags)
{
    SYSBVM_ASSERT(pic);
    sysbvm_tuple_t method = SYSBVM_NULL_TUPLE;
    if(!sysbvm_pic_lookupTypeAndSelector(pic, selector, receiverType, &method))
    {
        method = sysbvm_type_lookupSelector(context, receiverType, selector);
        sysbvm_pic_addSelectorTypeAndMethod(pic, selector, receiverType, method);
    }

    if(method)
        return sysbvm_bytecodeInterpreter_functionApplyNoCopyArguments(context, method, argumentCount + 1, receiverAndArguments, applicationFlags);

    // Attempt to send doesNotUnderstand:
    if(selector != context->roots.doesNotUnderstandSelector)
        method = sysbvm_type_lookupSelector(context, receiverType, selector);
    if(!method)
        sysbvm_error("Message not understood");

    // Make the message.
    sysbvm_tuple_t arguments = sysbvm_array_create(context, argumentCount);
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_array_atPut(arguments, i, receiverAndArguments[1 + i]);

    sysbvm_tuple_t message = sysbvm_message_create(context, selector, arguments, receiverType, SYSBVM_NULL_TUPLE);
    return sysbvm_function_apply2(context, method, receiverAndArguments[0], message);
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_interpretSendNoCopyArguments(sysbvm_context_t *context, sysbvm_pic_t *pic, sysbvm_tuple_t selector, size_t argumentCount, sysbvm_tuple_t *receiverAndArguments, sysbvm_bitflags_t applicationFlags)
{
    return sysbvm_bytecodeInterpreter_interpretSendWithReceiverTypeNoCopyArguments(context, pic, sysbvm_tuple_getType(context, receiverAndArguments[0]), selector, argumentCount, receiverAndArguments, applicationFlags);
}

SYSBVM_API void sysbvm_bytecodeInterpreter_interpretWithActivationRecord(sysbvm_context_t *context, sysbvm_stackFrameBytecodeFunctionActivationRecord_t *activationRecord)
{
    sysbvm_bytecodeInterpreter_ensureTablesAreFilled();
    int16_t decodedOperands[SYSBVM_BYTECODE_FUNCTION_OPERAND_REGISTER_FILE_SIZE] = {0};
    sysbvm_tuple_t *operandRegisterFile = activationRecord->operandRegisterFile;
    sysbvm_tuple_t *localVector = activationRecord->inlineLocalVector;

    size_t instructionsSize;
    size_t pc;
    size_t countExtension = 0;
    for(;;)
    {
        pc = activationRecord->pc;
        instructionsSize = sysbvm_tuple_getSizeInBytes(activationRecord->instructions);
        if(pc >= instructionsSize)
            break;

        uint8_t *instructions = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(activationRecord->instructions)->bytes;
        uint8_t opcode = instructions[pc++];
        if(opcode == SYSBVM_OPCODE_COUNT_EXTENSION)
        {
            uint8_t lowByte = instructions[pc++];
            uint8_t highByte = instructions[pc++];
            countExtension = (countExtension << 16) | (highByte << 8) | lowByte;
            continue;
        }

        uint8_t standardOpcode = opcode;
        size_t operandCount = 0;
        size_t caseCount = 0;
        if(opcode >= SYSBVM_OPCODE_FIRST_VARIABLE)
        {
            operandCount = (countExtension << 4) + (opcode & 0x0F);
            standardOpcode = opcode & 0xF0;
            if(standardOpcode == SYSBVM_OPCODE_CASE_JUMP)
            {
                caseCount = operandCount;
                operandCount *= 2;
            }

            operandCount += sysbvm_implicitVariableBytecodeOperandCountTable[opcode >> 4];
        }
        else
        {
            operandCount = opcode >> 4;
        }
        countExtension = 0;

        // Decode the operands.
        SYSBVM_ASSERT(operandCount <= SYSBVM_BYTECODE_FUNCTION_OPERAND_REGISTER_FILE_SIZE);
        SYSBVM_ASSERT(pc + operandCount*2 <= instructionsSize);
        for(uint8_t i = 0; i < operandCount; ++i)
        {
            uint16_t lowByte = instructions[pc++];
            uint16_t highByte = instructions[pc++];
            decodedOperands[i] = lowByte | (highByte << 8);
        }

        // Validate the destination operands.
        uint8_t destinationOperandCount = sysbvm_bytecodeInterpreter_destinationOperandCountForOpcode(standardOpcode);
        size_t offsetOperandCount = caseCount + sysbvm_bytecodeInterpreter_offsetOperandCountForOpcode(standardOpcode);

        for(uint8_t i = 0; i < destinationOperandCount; ++i)
        {
            if((decodedOperands[i] & SYSBVM_OPERAND_VECTOR_BITMASK) != SYSBVM_OPERAND_VECTOR_LOCAL)
                sysbvm_error("Bytecode destination operands must be in the local vector.");
            
            decodedOperands[i] >>= SYSBVM_OPERAND_VECTOR_BITS;
            if(decodedOperands[i] >= 0 && (size_t)decodedOperands[i] >= activationRecord->inlineLocalVectorSize)
                sysbvm_error("Bytecode destination operand is beyond the local vector bounds.");
        }

        // Fetch the source operands.
        for(uint8_t i = destinationOperandCount; i < operandCount - offsetOperandCount; ++i)
        {
            SYSBVM_ASSERT(i < SYSBVM_BYTECODE_FUNCTION_OPERAND_REGISTER_FILE_SIZE);

            int16_t decodedOperand = decodedOperands[i];
            int16_t vectorIndex = decodedOperand >> SYSBVM_OPERAND_VECTOR_BITS;
            uint8_t vectorType = decodedOperand & SYSBVM_OPERAND_VECTOR_BITMASK;
            switch(vectorType)
            {
            case SYSBVM_OPERAND_VECTOR_ARGUMENTS:
                if((size_t)vectorIndex >= activationRecord->argumentCount)
                    sysbvm_error("Bytecode operand is beyond the argument vector bounds.");
                operandRegisterFile[i] = activationRecord->arguments[vectorIndex];
                break;
            case SYSBVM_OPERAND_VECTOR_CAPTURES:
                if((size_t)vectorIndex >= sysbvm_tuple_getSizeInSlots(activationRecord->captureVector))
                    sysbvm_error("Bytecode operand is beyond the capture vector bounds.");
                operandRegisterFile[i] = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(activationRecord->captureVector)->pointers[vectorIndex];
                break;
            case SYSBVM_OPERAND_VECTOR_LITERAL:
                if((size_t)vectorIndex >= sysbvm_tuple_getSizeInSlots(activationRecord->literalVector))
                    sysbvm_error("Bytecode operand is beyond the literal vector bounds.");
                operandRegisterFile[i] = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(activationRecord->literalVector)->pointers[vectorIndex];
                break;
            case SYSBVM_OPERAND_VECTOR_LOCAL:
                if((size_t)vectorIndex >= activationRecord->inlineLocalVectorSize)
                    sysbvm_error("Bytecode operand is beyond the local vector bounds.");
                operandRegisterFile[i] = activationRecord->inlineLocalVector[vectorIndex];
                break;
            default:
                abort();
            }
        }

        // Execute the opcodes.
        bool isBackwardBranch = false;
        switch(standardOpcode)
        {
        // Zero operands
        case SYSBVM_OPCODE_NOP:
            // Nothing is required here.
            break;
        case SYSBVM_OPCODE_BREAKPOINT:
            // Nothing is required here.
            break;
        case SYSBVM_OPCODE_UNREACHABLE:
            sysbvm_error("Unreachable bytecode executed");
            break;

        // One operands
        case SYSBVM_OPCODE_RETURN:
            activationRecord->result = operandRegisterFile[0];
            return;
        case SYSBVM_OPCODE_JUMP:
            pc += decodedOperands[0];
            isBackwardBranch = decodedOperands[0] < 0;
            break;

        // Two operands.
        case SYSBVM_OPCODE_ALLOCA:
            operandRegisterFile[0] = sysbvm_pointerLikeType_withEmptyBox(context, operandRegisterFile[1]);
            break;
        case SYSBVM_OPCODE_LOAD:
            operandRegisterFile[0] = sysbvm_pointerLikeType_load(context, operandRegisterFile[1]);
            break;
        case SYSBVM_OPCODE_LOAD_SYMBOL_VALUE_BINDING:
            operandRegisterFile[0] = sysbvm_symbolValueBinding_getValue(operandRegisterFile[1]);
            break;
        case SYSBVM_OPCODE_STORE:
            sysbvm_pointerLikeType_store(context, operandRegisterFile[0], operandRegisterFile[1]);
            break;
        case SYSBVM_OPCODE_MOVE:
            operandRegisterFile[0] = operandRegisterFile[1];
            break;
        case SYSBVM_OPCODE_JUMP_IF_TRUE:
            if(sysbvm_tuple_boolean_decode(operandRegisterFile[0]))
            {
                pc += decodedOperands[1];
                isBackwardBranch = decodedOperands[1] < 0;
            }
            break;
        case SYSBVM_OPCODE_JUMP_IF_FALSE:
            if(!sysbvm_tuple_boolean_decode(operandRegisterFile[0]))
            {
                pc += decodedOperands[1];
                isBackwardBranch = decodedOperands[1 < 0];
            }
            break;
        case SYSBVM_OPCODE_SET_DEBUG_VALUE:
            // Nop.
            break;
        case SYSBVM_OPCODE_SLOT_AT:
            {
                size_t slotIndex = sysbvm_typeSlot_getIndex(operandRegisterFile[2]);
                operandRegisterFile[0] = sysbvm_tuple_slotAt(context, operandRegisterFile[1], slotIndex);
            }
            break;
        case SYSBVM_OPCODE_SLOT_REFERENCE_AT:
            {
                sysbvm_tuple_t slotReferenceType = sysbvm_typeSlot_getValidReferenceType(context, operandRegisterFile[2]);
                operandRegisterFile[0] = sysbvm_referenceType_withTupleAndTypeSlot(context, slotReferenceType, operandRegisterFile[1], operandRegisterFile[2]);
            }
            break;
        case SYSBVM_OPCODE_SLOT_AT_PUT:
            {
                size_t slotIndex = sysbvm_typeSlot_getIndex(operandRegisterFile[1]);
                sysbvm_tuple_slotAtPut(context, operandRegisterFile[0], slotIndex, operandRegisterFile[2]);
            }
            break;
        case SYSBVM_OPCODE_REF_SLOT_AT:
            {
                size_t slotIndex = sysbvm_typeSlot_getIndex(operandRegisterFile[2]);
                operandRegisterFile[0] = sysbvm_tuple_slotAt(context, sysbvm_pointerLikeType_load(context, operandRegisterFile[1]), slotIndex);
            }
            break;
        case SYSBVM_OPCODE_REF_SLOT_REFERENCE_AT:
            {
                sysbvm_tuple_t slotReferenceType = sysbvm_typeSlot_getValidReferenceType(context, operandRegisterFile[2]);
                // FIXME: Increase or update the pointer location here.
                operandRegisterFile[0] = sysbvm_referenceType_incrementWithTypeSlot(context, slotReferenceType, operandRegisterFile[1], operandRegisterFile[2]);
            }
            break;
        case SYSBVM_OPCODE_REF_SLOT_AT_PUT:
            {
                size_t slotIndex = sysbvm_typeSlot_getIndex(operandRegisterFile[1]);
                sysbvm_tuple_slotAtPut(context, sysbvm_pointerLikeType_load(context, operandRegisterFile[0]), slotIndex, operandRegisterFile[2]);
            }
            break;

        // Three operands.
        case SYSBVM_OPCODE_ALLOCA_WITH_VALUE:
            operandRegisterFile[0] = sysbvm_pointerLikeType_withBoxForValue(context, operandRegisterFile[1], operandRegisterFile[2]);
            break;
        case SYSBVM_OPCODE_COERCE_VALUE:
            operandRegisterFile[0] = sysbvm_type_coerceValue(context, operandRegisterFile[1], operandRegisterFile[2]);
            break;
        case SYSBVM_OPCODE_DOWNCAST_VALUE:
            sysbvm_tuple_typecheckValue(context, operandRegisterFile[1], operandRegisterFile[2]);
            operandRegisterFile[0] = operandRegisterFile[2];
            break;
        case SYSBVM_OPCODE_UNCHECKED_DOWNCAST_VALUE:
            operandRegisterFile[0] = operandRegisterFile[2];
            break;            
        case SYSBVM_OPCODE_MAKE_ASSOCIATION:
            operandRegisterFile[0] = sysbvm_association_create(context, operandRegisterFile[1], operandRegisterFile[2]);
            break;

        // Variable operand.
        case SYSBVM_OPCODE_CALL:
            operandRegisterFile[0] = sysbvm_bytecodeInterpreter_functionApply(context, operandRegisterFile[1], opcode & 0xF, operandRegisterFile + 2, 0);
            break;
        case SYSBVM_OPCODE_UNCHECKED_CALL:
            operandRegisterFile[0] = sysbvm_bytecodeInterpreter_functionApply(context, operandRegisterFile[1], opcode & 0xF, operandRegisterFile + 2, SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK);
            break;
        case SYSBVM_OPCODE_SEND:
            operandRegisterFile[0] = sysbvm_bytecodeInterpreter_interpretSend(context, sysbvm_tuple_getType(context, operandRegisterFile[2]), operandRegisterFile[1], opcode & 0xF, operandRegisterFile + 2);
            break;
        case SYSBVM_OPCODE_SEND_WITH_LOOKUP:
            operandRegisterFile[0] = sysbvm_bytecodeInterpreter_interpretSend(context, operandRegisterFile[1], operandRegisterFile[2], opcode & 0xF, operandRegisterFile + 3);
            break;

        case SYSBVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS:
            {
                size_t arraySize = opcode & 0xF;
                operandRegisterFile[0] = sysbvm_array_create(context, arraySize);
                sysbvm_tuple_t *arraySlots = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(operandRegisterFile[0])->pointers;
                for(size_t i = 0; i < arraySize; ++i)
                    arraySlots[i] = operandRegisterFile[1 + i];
            }
            break;
        case SYSBVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS:
            {
                size_t arraySize = opcode & 0xF;
                operandRegisterFile[0] = sysbvm_byteArray_create(context, arraySize);
                uint8_t *bytes = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(operandRegisterFile[0])->bytes;
                for(size_t i = 0; i < arraySize; ++i)
                    bytes[i] = sysbvm_tuple_uint8_decode(operandRegisterFile[1 + i]);
            }
            break;
        case SYSBVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES:
            {
                size_t captureVectorSize = opcode & 0xF;
                sysbvm_functionDefinition_t *functionDefinition = (sysbvm_functionDefinition_t*)operandRegisterFile[1];
                operandRegisterFile[0] = sysbvm_sequenceTuple_create(context, functionDefinition->captureVectorType);
                sysbvm_tuple_t *captureVectorSlots = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(operandRegisterFile[0])->pointers;
                for(size_t i = 0; i < captureVectorSize; ++i)
                    captureVectorSlots[i] = operandRegisterFile[2 + i];
                operandRegisterFile[0] = sysbvm_function_createClosureWithCaptureVector(context, operandRegisterFile[1], operandRegisterFile[0]);
            }
            break;
        case SYSBVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS:
            {
                size_t dictionarySize = opcode & 0xF;
                operandRegisterFile[0] = sysbvm_dictionary_createWithCapacity(context, dictionarySize);
                for(size_t i = 0; i < dictionarySize; ++i)
                    sysbvm_dictionary_add(context, operandRegisterFile[0], operandRegisterFile[1 + i]);
            }
            break;
            
        case SYSBVM_OPCODE_CASE_JUMP:
            {
                bool hasFoundCase = false;
                for(size_t i = 0; i < caseCount; ++i)
                {
                    if(operandRegisterFile[0] == operandRegisterFile[i + 1] || sysbvm_tuple_equals(context, operandRegisterFile[0], operandRegisterFile[i + 1]))
                    {
                        // Found the case.
                        hasFoundCase = true;
                        pc += decodedOperands[1 + caseCount + i];
                        break;
                    }
                }

                if(!hasFoundCase)
                    pc += decodedOperands[operandCount - 1];
            }
            break;
        default:
            sysbvm_error("Unsupported bytecode instruction.");
            break;
        }

        // Write back the destinations and the new PC.
        for(uint8_t i = 0; i < destinationOperandCount; ++i)
        {
            if(decodedOperands[i] >= 0)
            {
                if((size_t)decodedOperands[i] >= activationRecord->inlineLocalVectorSize)
                    sysbvm_error("Bytecode destination operand is beyond the local vector bounds.");
                localVector[decodedOperands[i]] = operandRegisterFile[i];
            }
        }
        activationRecord->pc = pc;

        // Safepoint in backward branches.
        if(isBackwardBranch)
            sysbvm_gc_safepoint(context);
    }

    SYSBVM_ASSERT(activationRecord->pc < sysbvm_tuple_getSizeInBytes(activationRecord->instructions));
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_getSourcePositionForPC(sysbvm_context_t *context, sysbvm_functionBytecode_t *functionBytecode, size_t pc)
{
    return sysbvm_orderedOffsetTable_findValueWithOffset(context, functionBytecode->debugSourcePositions, (uint32_t)pc);
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_getSourcePositionForActivationRecord(sysbvm_context_t *context, sysbvm_stackFrameBytecodeFunctionActivationRecord_t *activationRecord)
{
    sysbvm_functionBytecode_t **functionBytecodeObject = (sysbvm_functionBytecode_t **)&activationRecord->functionBytecode;
    sysbvm_tuple_t actualSourcePosition = sysbvm_bytecodeInterpreter_getSourcePositionForPC(context, *functionBytecodeObject, activationRecord->pc);
    if(actualSourcePosition)
        return actualSourcePosition;

    sysbvm_functionDefinition_t **functionDefinitionObject = (sysbvm_functionDefinition_t**)&activationRecord->functionDefinition;
    if((*functionDefinitionObject)->sourceDefinition)
        return ((sysbvm_functionSourceDefinition_t*)(*functionDefinitionObject)->sourceDefinition)->sourcePosition;

    return SYSBVM_NULL_TUPLE;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_getSourcePositionForJitActivationRecord(sysbvm_context_t *context, sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t *activationRecord)
{
    sysbvm_function_t *functionObject = (sysbvm_function_t*)activationRecord->function;
    sysbvm_functionDefinition_t *functionDefinitionObject = (sysbvm_functionDefinition_t*)functionObject->definition;
    sysbvm_functionBytecode_t *functionBytecodeObject = (sysbvm_functionBytecode_t *)functionDefinitionObject->bytecode;
    sysbvm_tuple_t actualSourcePosition = sysbvm_bytecodeInterpreter_getSourcePositionForPC(context, functionBytecodeObject, activationRecord->pc);
    if(actualSourcePosition)
        return actualSourcePosition;

    if(functionDefinitionObject->sourceDefinition)
        return ((sysbvm_functionSourceDefinition_t*)functionDefinitionObject->sourceDefinition)->sourcePosition;

    return SYSBVM_NULL_TUPLE;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_activateAndApply(sysbvm_context_t *context, sysbvm_tuple_t function_, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    sysbvm_stackFrameBytecodeFunctionActivationRecord_t activationRecord = {
        .type = SYSBVM_STACK_FRAME_RECORD_TYPE_BYTECODE_FUNCTION_ACTIVATION,
    };
    sysbvm_stackFrame_pushRecord((sysbvm_stackFrameRecord_t*)&activationRecord);  

    activationRecord.function = function_;
    sysbvm_function_t **functionObject = (sysbvm_function_t**)&activationRecord.function;
    
    activationRecord.functionDefinition = (*functionObject)->definition;
    sysbvm_functionDefinition_t **functionDefinitionObject = (sysbvm_functionDefinition_t**)&activationRecord.functionDefinition;

    activationRecord.functionBytecode = (*functionDefinitionObject)->bytecode;
    sysbvm_functionBytecode_t **functionBytecodeObject = (sysbvm_functionBytecode_t **)&activationRecord.functionBytecode;

    activationRecord.argumentCount = argumentCount;
    activationRecord.arguments = arguments;

    activationRecord.captureVector = (*functionObject)->captureVector;
    activationRecord.literalVector = (*functionBytecodeObject)->literalVector;
    activationRecord.instructions = (*functionBytecodeObject)->instructions;
    SYSBVM_ASSERT(sysbvm_tuple_isBytes(activationRecord.instructions));

    // Set the initial PC.
    activationRecord.pc = 0;

    // Allocate the inline local vector.
    size_t requiredLocalVectorSize = sysbvm_tuple_size_decode((*functionBytecodeObject)->localVectorSize);
    sysbvm_tuple_t *inlineLocalVector = (sysbvm_tuple_t *)alloca(requiredLocalVectorSize * sizeof(sysbvm_tuple_t));
    memset(inlineLocalVector, 0, requiredLocalVectorSize * sizeof(sysbvm_tuple_t));

    activationRecord.inlineLocalVectorSize = requiredLocalVectorSize;
    activationRecord.inlineLocalVector = inlineLocalVector;

    // Interpret.
    if(!_setjmp(activationRecord.jmpbuffer))
        sysbvm_bytecodeInterpreter_interpretWithActivationRecord(context, &activationRecord);

    sysbvm_stackFrame_popRecord((sysbvm_stackFrameRecord_t*)&activationRecord);
    return activationRecord.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_apply(sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments)
{
#ifdef SYSBVM_JIT_SUPPORTED
    if(context->jitEnabled)
    {
        sysbvm_function_t *functionObject = (sysbvm_function_t*)function;
        sysbvm_functionDefinition_t *functionDefinitionObject = (sysbvm_functionDefinition_t*)functionObject->definition;
        sysbvm_functionBytecode_t *functionBytecodeObject = (sysbvm_functionBytecode_t *)functionDefinitionObject->bytecode;
        if(!functionBytecodeObject->jittedCode || functionBytecodeObject->jittedCodeSessionToken != context->roots.sessionToken)
            sysbvm_bytecodeJit_jit(context, functionBytecodeObject);

        if(functionBytecodeObject->jittedCode && functionBytecodeObject->jittedCodeSessionToken == context->roots.sessionToken)
        {
            sysbvm_bytecodeJit_entryPoint entryPoint = (sysbvm_bytecodeJit_entryPoint)sysbvm_tuple_systemHandle_decode(functionBytecodeObject->jittedCode);
            return entryPoint(context, function, argumentCount, arguments);
        }
    }
#endif

    return sysbvm_bytecodeInterpreter_activateAndApply(context, function, argumentCount, arguments);
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeInterpreter_applyJitTrampolineDestination(sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    return sysbvm_bytecodeInterpreter_apply(context, function, argumentCount, arguments);
}

void sysbvm_bytecode_registerPrimitives(void)
{
}

void sysbvm_bytecode_setupPrimitives(sysbvm_context_t *context)
{
    // Export the function opcodes.

    // Zero operands.
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Nop", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_NOP));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Breakpoint", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_BREAKPOINT));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Unreachable", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_UNREACHABLE));

    // One operands.
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Return", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_RETURN));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Jump", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_JUMP));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::CountExtension", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_COUNT_EXTENSION));

    // Two operands.
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Alloca", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_ALLOCA));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Move", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_MOVE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Load", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_LOAD));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::LoadSymbolValueBinding", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_LOAD_SYMBOL_VALUE_BINDING));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Store", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_STORE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::JumpIfTrue", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_JUMP_IF_TRUE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::JumpIfFalse", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_JUMP_IF_FALSE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::SetDebugValue", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_SET_DEBUG_VALUE));

    // Three operands.
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::AllocaWithValue", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_ALLOCA_WITH_VALUE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::CoerceValue", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_COERCE_VALUE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::DownCastValue", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_DOWNCAST_VALUE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::UncheckedDownCastValue", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_UNCHECKED_DOWNCAST_VALUE));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeAssociation", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_MAKE_ASSOCIATION));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::SlotAt", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_SLOT_AT));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::SlotReferenceAt", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_SLOT_REFERENCE_AT));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::SlotAtPut", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_SLOT_AT_PUT));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::RefSlotAt", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_REF_SLOT_AT));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::RefSlotReferenceAt", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_REF_SLOT_REFERENCE_AT));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::RefSlotAtPut", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_REF_SLOT_AT_PUT));

    // Variable operand count.
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Call", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_CALL));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::UncheckedCall", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_UNCHECKED_CALL));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::Send", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_SEND));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::SendWithLookup", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_SEND_WITH_LOOKUP));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeArrayWithElements", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeByteArrayWithElements", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeClosureWithCaptures", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeDictionaryWithElements", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::MakeTupleWithElements", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_MAKE_TUPLE_WITH_ELEMENTS));

    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::CaseJump", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_CASE_JUMP));

    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::Opcode::FirstVariable", sysbvm_tuple_uint8_encode(SYSBVM_OPCODE_FIRST_VARIABLE));

    // Export the operand vector names.
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::Arguments", sysbvm_tuple_int16_encode(SYSBVM_OPERAND_VECTOR_ARGUMENTS));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::Captures", sysbvm_tuple_int16_encode(SYSBVM_OPERAND_VECTOR_CAPTURES));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::Literal", sysbvm_tuple_int16_encode(SYSBVM_OPERAND_VECTOR_LITERAL));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::Local", sysbvm_tuple_int16_encode(SYSBVM_OPERAND_VECTOR_LOCAL));

    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::Bits", sysbvm_tuple_int16_encode(SYSBVM_OPERAND_VECTOR_BITS));
    sysbvm_context_setIntrinsicSymbolBindingNamedWithValue(context, "FunctionBytecode::OperandVectorName::BitMask", sysbvm_tuple_int16_encode(SYSBVM_OPERAND_VECTOR_BITMASK));
}
