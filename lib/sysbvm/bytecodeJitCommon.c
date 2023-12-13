#include "sysbvm/bytecodeJit.h"
#include "sysbvm/assert.h"
#include "sysbvm/association.h"
#include "sysbvm/dwarf.h"
#include "sysbvm/elf.h"
#include "sysbvm/gc.h"
#include "sysbvm/gdb.h"
#include "sysbvm/environment.h"
#include "sysbvm/function.h"
#include "sysbvm/orderedOffsetTable.h"
#include "sysbvm/sourceCode.h"
#include "sysbvm/sourcePosition.h"
#include "sysbvm/stackFrame.h"
#include "sysbvm/type.h"
#include "internal/context.h"
#include "internal/virtualMemory.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef SYSBVM_JIT_SUPPORTED

extern uint8_t sysbvm_implicitVariableBytecodeOperandCountTable[16];
void sysbvm_bytecodeInterpreter_ensureTablesAreFilled();

SYSBVM_API void sysbvm_bytecodeJit_initialize(sysbvm_bytecodeJit_t *jit, sysbvm_context_t *context)
{
    memset(jit, 0, sizeof(sysbvm_bytecodeJit_t));
    jit->context = context;
    sysbvm_dynarray_initialize(&jit->instructions, 1, 1024);
    sysbvm_dynarray_initialize(&jit->constants, 1, 1024);
    sysbvm_dynarray_initialize(&jit->relocations, sizeof(sysbvm_bytecodeJitRelocation_t), 0);
    sysbvm_dynarray_initialize(&jit->pcRelocations, sizeof(sysbvm_bytecodeJitPCRelocation_t), 0);
    sysbvm_dynarray_initialize(&jit->sourcePositions, sizeof(sysbvm_bytecodeJitSourcePositionRecord_t), 256);

    sysbvm_dynarray_initialize(&jit->unwindInfo, 1, 64);
    sysbvm_dynarray_initialize(&jit->unwindInfoBytecode, 1, 64);

    sysbvm_dwarf_cfi_create(&jit->dwarfEhBuilder);
    sysbvm_dwarf_debugInfo_create(&jit->dwarfDebugInfoBuilder);
    sysbvm_dynarray_initialize(&jit->objectFileHeader, 1, sizeof(sysbvm_elf64_header_t));
    sysbvm_dynarray_initialize(&jit->objectFileContent, 1, 1024);
}

SYSBVM_API size_t sysbvm_bytecodeJit_addBytes(sysbvm_bytecodeJit_t *jit, size_t byteCount, uint8_t *bytes)
{
    return sysbvm_dynarray_addAll(&jit->instructions, byteCount, bytes);
}

SYSBVM_API size_t sysbvm_bytecodeJit_addByte(sysbvm_bytecodeJit_t *jit, uint8_t byte)
{
    return sysbvm_bytecodeJit_addBytes(jit, 1, &byte);
}

SYSBVM_API size_t sysbvm_bytecodeJit_addConstantsBytes(sysbvm_bytecodeJit_t *jit, size_t byteCount, uint8_t *bytes)
{
    size_t offset = jit->constants.size;
    sysbvm_dynarray_addAll(&jit->constants, byteCount, bytes);
    return offset;
}

SYSBVM_API size_t sysbvm_bytecodeJit_addUnwindInfoBytes(sysbvm_bytecodeJit_t *jit, size_t byteCount, uint8_t *bytes)
{
    size_t offset = jit->unwindInfo.size;
    sysbvm_dynarray_addAll(&jit->unwindInfo, byteCount, bytes);
    return offset;
}

SYSBVM_API size_t sysbvm_bytecodeJit_addUnwindInfoByte(sysbvm_bytecodeJit_t *jit, uint8_t byte)
{
    return sysbvm_bytecodeJit_addUnwindInfoBytes(jit, 1, &byte);
}

#ifdef _WIN32
SYSBVM_API void sysbvm_bytecodeJit_uwop(sysbvm_bytecodeJit_t *jit, uint8_t opcode, uint8_t operationInfo)
{
    uint8_t prologueOffset = (uint8_t)jit->instructions.size;
    uint8_t operation = (operationInfo << 4) | opcode;
    uint16_t code = prologueOffset | (operation << 8);
    sysbvm_dynarray_addAll(&jit->unwindInfoBytecode, 2, &code);
}

SYSBVM_API void sysbvm_bytecodeJit_uwop_pushNonVol(sysbvm_bytecodeJit_t *jit, uint8_t reg)
{
    sysbvm_bytecodeJit_uwop(jit, /*UWOP_PUSH_NONVOL */0 , reg);
}

SYSBVM_API void sysbvm_bytecodeJit_uwop_setFPReg(sysbvm_bytecodeJit_t *jit)
{
    sysbvm_bytecodeJit_uwop(jit, /* UWOP_SET_FPREG */3, 0);
}

SYSBVM_API void sysbvm_bytecodeJit_uwop_alloc(sysbvm_bytecodeJit_t *jit, size_t amount)
{
    if(amount == 0) return;

    SYSBVM_ASSERT((amount % 8) == 0);
    if(amount <= 128)
    {
        sysbvm_bytecodeJit_uwop(jit, /* UWOP_ALLOC_SMALL */2, (uint8_t)(amount/8 - 1));
    }
    else if(amount <= 512*1024 - 8)
    {
        size_t encodedAmount = amount / 8;
        SYSBVM_ASSERT(encodedAmount <= 0xFFFF);
        uint16_t encodedAmountU16 = (uint16_t)encodedAmount;
        sysbvm_dynarray_addAll(&jit->unwindInfoBytecode, 2, &encodedAmountU16);
        sysbvm_bytecodeJit_uwop(jit, /* UWOP_ALLOC_LARGE */1, 0);
    }
    else
    {
        abort();
    }
}

#endif

SYSBVM_API void sysbvm_bytecodeJit_addSourcePositionRecordWith(sysbvm_bytecodeJit_t *jit, size_t nativePC, sysbvm_tuple_t sourcePosition)
{
    if(jit->sourcePositions.size)
    {
        sysbvm_bytecodeJitSourcePositionRecord_t *existingRecords = (sysbvm_bytecodeJitSourcePositionRecord_t*)jit->sourcePositions.data;
        if(existingRecords[jit->sourcePositions.size - 1].sourcePosition == sourcePosition)
            return;
    }

    sysbvm_bytecodeJitSourcePositionRecord_t newRecord = {0};
    newRecord.sourcePosition = sourcePosition;
    newRecord.pc = nativePC;
    if(!sysbvm_sourcePosition_getStartLineAndColumn(jit->context, sourcePosition, &newRecord.line, &newRecord.column))
        return;

    sysbvm_dynarray_add(&jit->sourcePositions, &newRecord);
}

SYSBVM_API void sysbvm_bytecodeJit_addSourcePositionRecord(sysbvm_bytecodeJit_t *jit, sysbvm_functionBytecode_t *functionBytecode, uint16_t bytecodePC, size_t nativePC)
{
    if(functionBytecode->debugSourcePositions)
    {
        sysbvm_tuple_t foundDebugPosition = sysbvm_orderedOffsetTable_findValueWithOffset(jit->context, functionBytecode->debugSourcePositions, bytecodePC);
        if(foundDebugPosition)
        {
            sysbvm_bytecodeJit_addSourcePositionRecordWith(jit, nativePC, foundDebugPosition);
            return;
        }
    }

    if(functionBytecode->sourcePosition)
        sysbvm_bytecodeJit_addSourcePositionRecordWith(jit, nativePC, functionBytecode->sourcePosition);
}

SYSBVM_API int sysbvm_jit_dwarfLineInfoEmissionState_indexOfDirectory(sysbvm_jit_dwarfLineInfoEmissionState_t *state, sysbvm_tuple_t directory)
{
    for(int i = 0; i < state->directoryCount; ++i)
    {
        if(state->directories[i] == directory)
            return i + 1;
    }

    return 0;
}

void sysbvm_jit_dwarfLineInfoEmissionState_addDirectory(sysbvm_jit_dwarfLineInfoEmissionState_t *state, sysbvm_tuple_t directory)
{
    if(!sysbvm_jit_dwarfLineInfoEmissionState_indexOfDirectory(state, directory))
    {
        SYSBVM_ASSERT(state->directoryCount < SYSBVM_JIT_DWARF_LINE_INFO_EMISSION_STATE_MAX_DIRECTORIES);
        state->directories[state->directoryCount++] = directory;
    }
}

SYSBVM_API int sysbvm_jit_dwarfLineInfoEmissionState_indexOfFile(sysbvm_jit_dwarfLineInfoEmissionState_t *state, sysbvm_tuple_t file)
{
    for(int i = 0; i < state->fileCount; ++i)
    {
        if(state->files[i] == file)
            return i + 1;
    }

    return 0;
}

void sysbvm_jit_dwarfLineInfoEmissionState_addSourceCode(sysbvm_jit_dwarfLineInfoEmissionState_t *state, sysbvm_tuple_t sourceCode)
{
    if(!sysbvm_tuple_isNonNullPointer(sourceCode))
        return;

    if(!sysbvm_jit_dwarfLineInfoEmissionState_indexOfFile(state, sourceCode))
    {
        sysbvm_sourceCode_t *sourceCodeObject = (sysbvm_sourceCode_t*)sourceCode;
        sysbvm_jit_dwarfLineInfoEmissionState_addDirectory(state, sourceCodeObject->directory);

        state->files[state->fileCount++] = sourceCode;
    }
}

void sysbvm_jit_dwarfLineInfoEmissionState_addSourcePositionSourceCode(sysbvm_jit_dwarfLineInfoEmissionState_t *state, sysbvm_tuple_t sourcePosition)
{
    if(!sysbvm_tuple_isNonNullPointer(sourcePosition))
        return;

    sysbvm_sourcePosition_t *sourcePositionObject = (sysbvm_sourcePosition_t*)sourcePosition;
    sysbvm_jit_dwarfLineInfoEmissionState_addSourceCode(state, sourcePositionObject->sourceCode);
}

void sysbvm_jit_dwarfLineInfoEmissionState_addSourcePosition(sysbvm_jit_dwarfLineInfoEmissionState_t *state, sysbvm_tuple_t sourcePosition, uint32_t line)
{
    if(!sysbvm_tuple_isNonNullPointer(sourcePosition))
        return;

    sysbvm_sourcePosition_t *sourcePositionObject = (sysbvm_sourcePosition_t*)sourcePosition;
    sysbvm_jit_dwarfLineInfoEmissionState_addSourceCode(state, sourcePositionObject->sourceCode);

    int lineAdvance = state->previousLine - line;
    if(abs(lineAdvance) < 8)
    {
        if(lineAdvance < state->minLineAdvance)
            state->minLineAdvance = lineAdvance;
        if(lineAdvance > state->maxLineAdvance)
            state->maxLineAdvance = lineAdvance;
    }

    state->previousLine = line;
}

void sysbvm_jit_dwarfLineInfoEmissionState_emitSourcePosition(sysbvm_bytecodeJit_t *jit, sysbvm_jit_dwarfLineInfoEmissionState_t *state, sysbvm_tuple_t sourcePosition, size_t pc, uint32_t line, uint32_t column)
{
    if(!sysbvm_tuple_isNonNullPointer(sourcePosition))
        return;

    sysbvm_sourcePosition_t *sourcePositionObject = (sysbvm_sourcePosition_t*)sourcePosition;

    // Set the source code.
    if(sourcePositionObject->sourceCode != state->currentFile)
    {
        sysbvm_dwarf_debugInfo_line_setFile(&jit->dwarfDebugInfoBuilder, sysbvm_jit_dwarfLineInfoEmissionState_indexOfFile(state, sourcePositionObject->sourceCode));
        state->currentFile = sourcePositionObject->sourceCode;
    }

    // Set the column.
    sysbvm_dwarf_debugInfo_line_setColumn(&jit->dwarfDebugInfoBuilder, column);

    int pcAdvance = (int)(pc - state->pc);
    int lineAdvance = line - state->previousLine;
    sysbvm_dwarf_debugInfo_line_advanceLineAndPC(&jit->dwarfDebugInfoBuilder, lineAdvance, pcAdvance);
    state->pc = pc;
    state->previousLine = line;
}

SYSBVM_API bool sysbvm_jit_emitDebugLineInfo(sysbvm_bytecodeJit_t *jit)
{
    if(!jit->sourcePositions.size)
        return false;

    sysbvm_jit_dwarfLineInfoEmissionState_t *state = &jit->dwarfLineEmissionState;
    memset(state, 0, sizeof(*state));
    sysbvm_jit_dwarfLineInfoEmissionState_addSourcePositionSourceCode(state, jit->sourcePosition);

    // Preprocessing step.
    sysbvm_bytecodeJitSourcePositionRecord_t *sourcePositionRecords = (sysbvm_bytecodeJitSourcePositionRecord_t*)jit->sourcePositions.data;
    state->previousLine = 1;
    state->minLineAdvance = INT32_MAX;
    state->maxLineAdvance = INT32_MIN;
    for(size_t i = 0; i < jit->sourcePositions.size; ++i)
    {
        sysbvm_bytecodeJitSourcePositionRecord_t *record = sourcePositionRecords + i;
        sysbvm_jit_dwarfLineInfoEmissionState_addSourcePosition(state, record->sourcePosition, record->line);
    }

    if(state->minLineAdvance > state->maxLineAdvance)
        state->minLineAdvance = state->maxLineAdvance = 1;

    state->lineBase = state->minLineAdvance;
    state->lineRange = state->maxLineAdvance - state->minLineAdvance;
    if(state->lineRange < 1)
        state->lineRange = 1;

    jit->dwarfDebugInfoBuilder.lineProgramHeader.lineBase = state->lineBase;
    jit->dwarfDebugInfoBuilder.lineProgramHeader.lineRange = state->lineRange;

    sysbvm_dwarf_debugInfo_beginLineInformation(&jit->dwarfDebugInfoBuilder);

    // Directories
    for(int i = 0; i < state->directoryCount; ++i)
        sysbvm_dwarf_debugInfo_addDirectory(&jit->dwarfDebugInfoBuilder, state->directories[i]);
    sysbvm_dwarf_debugInfo_endDirectoryList(&jit->dwarfDebugInfoBuilder);

    // Files    
    for(int i = 0; i < state->fileCount; ++i)
    {
        sysbvm_sourceCode_t *sourceCode = (sysbvm_sourceCode_t*)state->files[i];
        sysbvm_dwarf_debugInfo_addFile(&jit->dwarfDebugInfoBuilder, sysbvm_jit_dwarfLineInfoEmissionState_indexOfDirectory(state, sourceCode->directory), sourceCode->name);
    }
    sysbvm_dwarf_debugInfo_endFileList(&jit->dwarfDebugInfoBuilder);

    sysbvm_dwarf_debugInfo_endLineInformationHeader(&jit->dwarfDebugInfoBuilder);

    // Emit the source positions.
    state->previousLine = 1;
    state->pc = 0;
    state->currentFile = SYSBVM_NULL_TUPLE;
    if(state->fileCount > 0)
        state->currentFile = state->files[0];
    sysbvm_dwarf_debugInfo_line_setAddress(&jit->dwarfDebugInfoBuilder, state->pc);
    for(size_t i = 0; i < jit->sourcePositions.size; ++i)
    {
        sysbvm_bytecodeJitSourcePositionRecord_t *record = sourcePositionRecords + i;
        sysbvm_jit_dwarfLineInfoEmissionState_emitSourcePosition(jit, state, record->sourcePosition, record->pc, record->line, record->column);
    }

    sysbvm_dwarf_debugInfo_line_endSequence(&jit->dwarfDebugInfoBuilder);
    sysbvm_dwarf_debugInfo_endLineInformation(&jit->dwarfDebugInfoBuilder);

    return true;
}

SYSBVM_API void sysbvm_bytecodeJit_addPCRelocation(sysbvm_bytecodeJit_t *jit, sysbvm_bytecodeJitPCRelocation_t relocation)
{
    sysbvm_dynarray_add(&jit->pcRelocations, &relocation);
}

SYSBVM_API void sysbvm_bytecodeJit_addRelocation(sysbvm_bytecodeJit_t *jit, sysbvm_bytecodeJitRelocation_t relocation)
{
    sysbvm_dynarray_add(&jit->relocations, &relocation);
}

SYSBVM_API void sysbvm_bytecodeJit_jitFree(sysbvm_bytecodeJit_t *jit)
{
    sysbvm_dynarray_destroy(&jit->instructions);
    sysbvm_dynarray_destroy(&jit->constants);
    sysbvm_dynarray_destroy(&jit->relocations);
    sysbvm_dynarray_destroy(&jit->pcRelocations);
    sysbvm_dynarray_destroy(&jit->sourcePositions);
    free(jit->pcDestinations);

    sysbvm_dynarray_destroy(&jit->unwindInfo);
    sysbvm_dynarray_destroy(&jit->unwindInfoBytecode);
    sysbvm_dwarf_cfi_destroy(&jit->dwarfEhBuilder);
    sysbvm_dwarf_debugInfo_destroy(&jit->dwarfDebugInfoBuilder);
    
    sysbvm_dynarray_destroy(&jit->objectFileHeader);
    sysbvm_dynarray_destroy(&jit->objectFileContent);
}

SYSBVM_API bool sysbvm_bytecodeJit_getLiteralValueForOperand(sysbvm_bytecodeJit_t *jit, int16_t operand, sysbvm_tuple_t *outLiteralValue)
{
    *outLiteralValue = SYSBVM_NULL_TUPLE;
    int16_t vectorType = operand & SYSBVM_OPERAND_VECTOR_BITMASK;
    int16_t vectorIndex = operand >> SYSBVM_OPERAND_VECTOR_BITS;
    sysbvm_tuple_t literalVector = *jit->literalVectorGCRoot;
    if(vectorType == SYSBVM_OPERAND_VECTOR_LITERAL &&
        vectorIndex >= 0 &&
        (size_t)vectorIndex < sysbvm_tuple_getSizeInSlots(literalVector))
    {
        *outLiteralValue = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(literalVector)->pointers[vectorIndex];
        return true;
    }

    return false;   
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeJit_slotAt(sysbvm_context_t *context, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot)
{
    size_t slotIndex = sysbvm_typeSlot_getIndex(typeSlot);
    return sysbvm_tuple_slotAt(context, tuple, slotIndex);
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeJit_slotReferenceAt(sysbvm_context_t *context, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot)
{
    sysbvm_tuple_t slotReferenceType = sysbvm_typeSlot_getValidReferenceType(context, typeSlot);
    return sysbvm_referenceType_withTupleAndTypeSlot(context, slotReferenceType, tuple, typeSlot);
}

SYSBVM_API void sysbvm_bytecodeJit_slotAtPut(sysbvm_context_t *context, sysbvm_tuple_t tuple, sysbvm_tuple_t typeSlot, sysbvm_tuple_t value)
{
    size_t slotIndex = sysbvm_typeSlot_getIndex(typeSlot);
    sysbvm_tuple_slotAtPut(context, tuple, slotIndex, value);
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeJit_refSlotAt(sysbvm_context_t *context, sysbvm_tuple_t tupleReference, sysbvm_tuple_t typeSlot)
{
    size_t slotIndex = sysbvm_typeSlot_getIndex(typeSlot);
    return sysbvm_tuple_slotAt(context, sysbvm_pointerLikeType_load(context, tupleReference), slotIndex);
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeJit_refSlotReferenceAt(sysbvm_context_t *context, sysbvm_tuple_t tupleReference, sysbvm_tuple_t typeSlot)
{
    sysbvm_tuple_t slotReferenceType = sysbvm_typeSlot_getValidReferenceType(context, typeSlot);
    return sysbvm_referenceType_incrementWithTypeSlot(context, slotReferenceType, tupleReference, typeSlot);
}

SYSBVM_API void sysbvm_bytecodeJit_refSlotAtPut(sysbvm_context_t *context, sysbvm_tuple_t tupleReference, sysbvm_tuple_t typeSlot, sysbvm_tuple_t value)
{
    size_t slotIndex = sysbvm_typeSlot_getIndex(typeSlot);
    sysbvm_tuple_slotAtPut(context, sysbvm_pointerLikeType_load(context, tupleReference), slotIndex, value);
}

SYSBVM_API sysbvm_tuple_t sysbvm_bytecodeJit_symbolValueBinding_getValue(sysbvm_context_t *context, sysbvm_tuple_t valueBinding)
{
    (void)context;
    return sysbvm_symbolValueBinding_getValue(valueBinding);
}

SYSBVM_API void sysbvm_jit_dumpCodeToFileNamed(const void *code, size_t codeSize, const char *fileName)
{
#ifdef _WIN32
    FILE *file = NULL;
    fopen_s(&file, fileName, "wb");
#else
    FILE *file = fopen(fileName, "wb");
#endif
    if(!file)
        return;

    (void)fwrite(code, codeSize, 1, file);
    fclose(file);
}

SYSBVM_API void sysbvm_bytecodeJit_jit(sysbvm_context_t *context, sysbvm_functionBytecode_t *functionBytecode)
{
    (void)context;
    sysbvm_bytecodeInterpreter_ensureTablesAreFilled();

    sysbvm_bytecodeJit_t jit;
    sysbvm_bytecodeJit_initialize(&jit, context);

    jit.sourcePosition = functionBytecode->sourcePosition;
    jit.compiledProgramEntity = functionBytecode->definition;

    jit.literalVectorGCRoot = sysbvm_heap_allocateGCRootTableEntry(&context->heap);
    *jit.literalVectorGCRoot = functionBytecode->literalVector;

    size_t instructionsSize = sysbvm_tuple_getSizeInBytes(functionBytecode->instructions);
    uint8_t *instructions = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(functionBytecode->instructions)->bytes;

    int16_t decodedOperands[SYSBVM_BYTECODE_FUNCTION_OPERAND_REGISTER_FILE_SIZE] = {0};
    jit.argumentCount = sysbvm_tuple_size_decode(functionBytecode->argumentCount);
    jit.captureVectorSize = sysbvm_tuple_size_decode(functionBytecode->captureVectorSize);
    jit.literalCount = sysbvm_tuple_getSizeInSlots(functionBytecode->literalVector);
    jit.localVectorSize = sysbvm_tuple_size_decode(functionBytecode->localVectorSize);

    jit.pcDestinations = (intptr_t*)malloc(sizeof(intptr_t)*instructionsSize);
    memset(jit.pcDestinations, -1, sizeof(intptr_t)*instructionsSize);

    sysbvm_jit_prologue(&jit);

    size_t pc = 0;
    size_t countExtension = 0;
    while(pc < instructionsSize)
    {
        jit.pcDestinations[pc] = jit.instructions.size;
        sysbvm_jit_storePC(&jit, (uint16_t)pc);

        sysbvm_bytecodeJit_addSourcePositionRecord(&jit, functionBytecode, (uint16_t)pc, jit.instructions.size);

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
        SYSBVM_ASSERT(operandCount < SYSBVM_BYTECODE_FUNCTION_OPERAND_REGISTER_FILE_SIZE);
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
            
            int16_t vectorIndex = decodedOperands[i] >> SYSBVM_OPERAND_VECTOR_BITS;
            if(vectorIndex >= 0 && (size_t)vectorIndex >= jit.localVectorSize)
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
                if((size_t)vectorIndex >= jit.argumentCount)
                    sysbvm_error("Bytecode operand is beyond the argument vector bounds.");
                break;
            case SYSBVM_OPERAND_VECTOR_CAPTURES:
                if((size_t)vectorIndex >= jit.captureVectorSize)
                    sysbvm_error("Bytecode operand is beyond the capture vector bounds.");
                break;
            case SYSBVM_OPERAND_VECTOR_LITERAL:
                if((size_t)vectorIndex >= jit.literalCount)
                    sysbvm_error("Bytecode operand is beyond the literal vector bounds.");
                break;
            case SYSBVM_OPERAND_VECTOR_LOCAL:
                if((size_t)vectorIndex >= jit.localVectorSize)
                    sysbvm_error("Bytecode operand is beyond the local vector bounds.");
                break;
            default:
                abort();
            }
        }

        // Execute the opcodes.
        switch(standardOpcode)
        {
        // Zero operands
        case SYSBVM_OPCODE_NOP:
            // Nothing is required here.
            break;
        case SYSBVM_OPCODE_BREAKPOINT:
            // Nothing is required here.
            sysbvm_jit_breakpoint(&jit);
            break;
        case SYSBVM_OPCODE_UNREACHABLE:
            // Nothing is required here.
            sysbvm_jit_unreachable(&jit);
            break;
        
        // One operands
        case SYSBVM_OPCODE_RETURN:
            sysbvm_jit_return(&jit, decodedOperands[0]);
            break;
        case SYSBVM_OPCODE_JUMP:
            if(decodedOperands[0] < 0)
                sysbvm_jit_callWithContextNoResult0(&jit, &sysbvm_gc_safepoint);
            sysbvm_jit_jumpRelative(&jit, pc + decodedOperands[0]);
            break;
        // Two operands.
        case SYSBVM_OPCODE_ALLOCA:
            sysbvm_jit_callWithContext1(&jit, &sysbvm_pointerLikeType_withEmptyBox, decodedOperands[0], decodedOperands[1]);
            break;
        case SYSBVM_OPCODE_LOAD:
            sysbvm_jit_callWithContext1(&jit, &sysbvm_pointerLikeType_load, decodedOperands[0], decodedOperands[1]);
            break;
        case SYSBVM_OPCODE_LOAD_SYMBOL_VALUE_BINDING:
            sysbvm_jit_callWithContext1(&jit, &sysbvm_bytecodeJit_symbolValueBinding_getValue, decodedOperands[0], decodedOperands[1]);
            break;
        case SYSBVM_OPCODE_STORE:
            sysbvm_jit_callWithContextNoResult2(&jit, &sysbvm_pointerLikeType_store, decodedOperands[0], decodedOperands[1]);
            break;
        case SYSBVM_OPCODE_MOVE:
            sysbvm_jit_moveOperandToOperand(&jit, decodedOperands[0], decodedOperands[1]);
            break;
        case SYSBVM_OPCODE_JUMP_IF_TRUE:
            if(decodedOperands[1] < 0)
                sysbvm_jit_callWithContextNoResult0(&jit, &sysbvm_gc_safepoint);
            sysbvm_jit_jumpRelativeIfTrue(&jit, decodedOperands[0], pc + decodedOperands[1]);
            break;
        case SYSBVM_OPCODE_JUMP_IF_FALSE:
            if(decodedOperands[1] < 0)
                sysbvm_jit_callWithContextNoResult0(&jit, &sysbvm_gc_safepoint);
            sysbvm_jit_jumpRelativeIfFalse(&jit, decodedOperands[0], pc + decodedOperands[1]);
            break;
        case SYSBVM_OPCODE_SET_DEBUG_VALUE:
            // NOP
            break;

        // Three operands.
        case SYSBVM_OPCODE_ALLOCA_WITH_VALUE:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_pointerLikeType_withBoxForValue, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_COERCE_VALUE:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_type_coerceValue, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_DOWNCAST_VALUE:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_type_downCastValue, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_UNCHECKED_DOWNCAST_VALUE:
            sysbvm_jit_moveOperandToOperand(&jit, decodedOperands[0], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_MAKE_ASSOCIATION:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_association_create, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_SLOT_AT:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_bytecodeJit_slotAt, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_SLOT_AT_PUT:
            sysbvm_jit_callWithContextNoResult3(&jit, &sysbvm_bytecodeJit_slotAtPut, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_SLOT_REFERENCE_AT:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_bytecodeJit_slotReferenceAt, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_REF_SLOT_AT:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_bytecodeJit_refSlotAt, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_REF_SLOT_AT_PUT:
            sysbvm_jit_callWithContextNoResult3(&jit, &sysbvm_bytecodeJit_refSlotAtPut, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;
        case SYSBVM_OPCODE_REF_SLOT_REFERENCE_AT:
            sysbvm_jit_callWithContext2(&jit, &sysbvm_bytecodeJit_refSlotReferenceAt, decodedOperands[0], decodedOperands[1], decodedOperands[2]);
            break;

        // Variable operand.
        case SYSBVM_OPCODE_CALL:
            sysbvm_jit_functionApply(&jit, decodedOperands[0], decodedOperands[1], opcode & 0xF, decodedOperands + 2, 0);
            break;
        case SYSBVM_OPCODE_UNCHECKED_CALL:
            sysbvm_jit_functionApply(&jit, decodedOperands[0], decodedOperands[1], opcode & 0xF, decodedOperands + 2, SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK);
            break;
        case SYSBVM_OPCODE_SEND:
            sysbvm_jit_send(&jit, decodedOperands[0], decodedOperands[1], opcode & 0xF, decodedOperands + 2, 0);
            break;
        case SYSBVM_OPCODE_SEND_WITH_LOOKUP:
            sysbvm_jit_sendWithReceiverType(&jit, decodedOperands[0], decodedOperands[1], decodedOperands[2], opcode & 0xF, decodedOperands + 3, 0);
            break;
        case SYSBVM_OPCODE_MAKE_ARRAY_WITH_ELEMENTS:
            sysbvm_jit_makeArray(&jit, decodedOperands[0], opcode & 0xF, decodedOperands + 1);
            break;
        case SYSBVM_OPCODE_MAKE_BYTE_ARRAY_WITH_ELEMENTS:
            sysbvm_jit_makeByteArray(&jit, decodedOperands[0], opcode & 0xF, decodedOperands + 1);
            break;
        case SYSBVM_OPCODE_MAKE_CLOSURE_WITH_CAPTURES:
            sysbvm_jit_makeClosureWithCaptures(&jit, decodedOperands[0], decodedOperands[1], opcode & 0xF, decodedOperands + 2);
            break;
        case SYSBVM_OPCODE_MAKE_DICTIONARY_WITH_ELEMENTS:
            sysbvm_jit_makeDictionary(&jit, decodedOperands[0], opcode & 0xF, decodedOperands + 1);
            break;

        case SYSBVM_OPCODE_CASE_JUMP:
            sysbvm_jit_caseJump(&jit, decodedOperands[0], caseCount, decodedOperands + 1, decodedOperands + 1 + caseCount, decodedOperands[operandCount - 1], pc);
            break;
        default:
            sysbvm_error("Unsupported bytecode instruction.");
            break;
        }
    }

    sysbvm_jit_finish(&jit);

    size_t objectFileHeaderSize = sysbvm_sizeAlignedTo(jit.objectFileHeader.size, 16);
    size_t textSectionSize = sysbvm_sizeAlignedTo(jit.instructions.size, 16);
    size_t rodataSectionSize = sysbvm_sizeAlignedTo(jit.constants.size, 16);
    size_t unwindInfoSize = sysbvm_sizeAlignedTo(jit.unwindInfo.size, 16)  + sysbvm_sizeAlignedTo(jit.dwarfEhBuilder.buffer.size, 16);
    size_t debugInfoSize = sysbvm_sizeAlignedTo(jit.dwarfDebugInfoBuilder.line.size, 16)
        + sysbvm_sizeAlignedTo(jit.dwarfDebugInfoBuilder.str.size, 16)
        + sysbvm_sizeAlignedTo(jit.dwarfDebugInfoBuilder.abbrev.size, 16)
        + sysbvm_sizeAlignedTo(jit.dwarfDebugInfoBuilder.info.size, 16);
    size_t objectFileContentSize = sysbvm_sizeAlignedTo(jit.objectFileContent.size, 16);

    size_t requiredCodeSize = objectFileHeaderSize + textSectionSize + rodataSectionSize + unwindInfoSize + debugInfoSize + objectFileContentSize;
    uint8_t *codeWriteablePointer = NULL;
    uint8_t *codeExecutablePointer = NULL;
    sysbvm_chunkedAllocator_allocateWithDualMapping(&context->heap.codeAllocator, requiredCodeSize, 16, (void**)&codeWriteablePointer, (void**)&codeExecutablePointer);
    if(!sysbvm_virtualMemory_lockCodePagesForWriting(codeWriteablePointer, codeExecutablePointer, requiredCodeSize))
        abort();

    memset(codeWriteablePointer + objectFileHeaderSize, 0xcc, textSectionSize); // int3;
    memset(codeWriteablePointer + objectFileHeaderSize + textSectionSize, 0, rodataSectionSize); // int3;
    uint8_t *entryPointPointer = sysbvm_jit_installIn(&jit, codeWriteablePointer, codeExecutablePointer);
    sysbvm_virtualMemory_unlockCodePagesForExecution(codeWriteablePointer, codeExecutablePointer, requiredCodeSize);

    // Register the object file with gdb.
    if(jit.objectFileHeader.size > 0 && jit.objectFileContent.size > 0)
    {
        sysbvm_gdb_jit_code_entry_t *entry = (sysbvm_gdb_jit_code_entry_t*)calloc(1, sizeof(sysbvm_gdb_jit_code_entry_t));
        sysbvm_dynarray_add(&context->jittedObjectFileEntries, &entry);
        sysbvm_gdb_registerObjectFile(entry, codeExecutablePointer, requiredCodeSize);
    }

    functionBytecode->jittedCode = sysbvm_tuple_systemHandle_encode(context, (sysbvm_systemHandle_t)(uintptr_t)entryPointPointer);
    functionBytecode->jittedCodeSessionToken = context->roots.sessionToken;

    // Patch the trampoline.
    sysbvm_jit_patchTrampolineWithRealEntryPoint(&jit, functionBytecode);

    sysbvm_bytecodeJit_jitFree(&jit);
}

#endif