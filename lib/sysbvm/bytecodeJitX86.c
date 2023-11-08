#include "sysbvm/bytecodeJit.h"
#include "sysbvm/array.h"
#include "sysbvm/assert.h"
#include "sysbvm/dictionary.h"
#include "sysbvm/function.h"
#include "sysbvm/environment.h"
#include "sysbvm/programEntity.h"
#include "sysbvm/elf.h"
#include "sysbvm/pic.h"
#include "sysbvm/stackFrame.h"
#include "sysbvm/sourcePosition.h"
#include "sysbvm/sourceCode.h"
#include "internal/context.h"
#include "internal/virtualMemory.h"
#include <string.h>
#include <stdlib.h>

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>

#endif

#ifndef _WIN32
extern void __register_frame(const void*);
#endif

//define SYSBVM_EMIT_PERF_STACK_MAP

#if defined(SYSBVM_JIT_SUPPORTED) && defined(SYSBVM_ARCH_X86_64)
#define USE_OLD_STACK_LAYOUT 0

typedef enum sysbvm_x86_register_e
{
#if defined(SYSBVM_ARCH_X86_64) 
    SYSBVM_X86_RAX = 0,
    SYSBVM_X86_RCX = 1,
    SYSBVM_X86_RDX = 2,
    SYSBVM_X86_RBX = 3,
    SYSBVM_X86_RSP = 4,
    SYSBVM_X86_RBP = 5,
    SYSBVM_X86_RSI = 6,
    SYSBVM_X86_RDI = 7,

    SYSBVM_X86_R8 = 8,
    SYSBVM_X86_R9 = 9,
    SYSBVM_X86_R10 = 10,
    SYSBVM_X86_R11 = 11,
    SYSBVM_X86_R12 = 12,
    SYSBVM_X86_R13 = 13,
    SYSBVM_X86_R14 = 14,
    SYSBVM_X86_R15 = 15,
#endif

    SYSBVM_X86_EAX = 0,
    SYSBVM_X86_ECX = 1,
    SYSBVM_X86_EDX = 2,
    SYSBVM_X86_EBX = 3,
    SYSBVM_X86_ESP = 4,
    SYSBVM_X86_EBP = 5,
    SYSBVM_X86_ESI = 6,
    SYSBVM_X86_EDI = 7,

    SYSBVM_X86_REG_HALF_MASK = 7,

#if defined(SYSBVM_ARCH_X86_64)
#ifdef _WIN32
    SYSBVM_X86_WIN64_ARG0 = SYSBVM_X86_RCX,
    SYSBVM_X86_WIN64_ARG1 = SYSBVM_X86_RDX,
    SYSBVM_X86_WIN64_ARG2 = SYSBVM_X86_R8,
    SYSBVM_X86_WIN64_ARG3 = SYSBVM_X86_R9,
    SYSBVM_X86_WIN64_SHADOW_SPACE = 32,

    SYSBVM_X86_64_ARG0 = SYSBVM_X86_WIN64_ARG0,
    SYSBVM_X86_64_ARG1 = SYSBVM_X86_WIN64_ARG1,
    SYSBVM_X86_64_ARG2 = SYSBVM_X86_WIN64_ARG2,
    SYSBVM_X86_64_ARG3 = SYSBVM_X86_WIN64_ARG3,
    SYSBVM_X86_64_CALL_SHADOW_SPACE = SYSBVM_X86_WIN64_SHADOW_SPACE,
#else
    SYSBVM_X86_SYSV_ARG0 = SYSBVM_X86_RDI,
    SYSBVM_X86_SYSV_ARG1 = SYSBVM_X86_RSI,
    SYSBVM_X86_SYSV_ARG2 = SYSBVM_X86_RDX,
    SYSBVM_X86_SYSV_ARG3 = SYSBVM_X86_RCX,
    SYSBVM_X86_SYSV_ARG4 = SYSBVM_X86_R8,
    SYSBVM_X86_SYSV_ARG5 = SYSBVM_X86_R9,

    SYSBVM_X86_64_ARG0 = SYSBVM_X86_SYSV_ARG0,
    SYSBVM_X86_64_ARG1 = SYSBVM_X86_SYSV_ARG1,
    SYSBVM_X86_64_ARG2 = SYSBVM_X86_SYSV_ARG2,
    SYSBVM_X86_64_ARG3 = SYSBVM_X86_SYSV_ARG3,
    SYSBVM_X86_64_CALL_SHADOW_SPACE = 0,
#endif
#endif
} sysbvm_x86_register_t;

static void sysbvm_jit_x86_mov64Absolute(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, uint64_t value);
static void sysbvm_jit_moveRegisterToOperand(sysbvm_bytecodeJit_t *jit, int16_t operand, sysbvm_x86_register_t reg);
static void sysbvm_jit_moveOperandToRegister(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t reg, int16_t operand);
static void sysbvm_jit_moveOperandToCallArgumentVector(sysbvm_bytecodeJit_t *jit, int16_t operand, int32_t callArgumentVectorIndex);

static uint8_t sysbvm_jit_x86_modRM(int8_t rm, uint8_t regOpcode, uint8_t mod)
{
    return (rm & SYSBVM_X86_REG_HALF_MASK) | ((regOpcode & SYSBVM_X86_REG_HALF_MASK) << 3) | (mod << 6);
}

static uint8_t sysbvm_jit_x86_sibOnlyBase(uint8_t reg)
{
    return (reg & SYSBVM_X86_REG_HALF_MASK) | (4 << 3) ;
}

static uint8_t sysbvm_jit_x86_modRMRegister(sysbvm_x86_register_t rm, sysbvm_x86_register_t reg)
{
    return sysbvm_jit_x86_modRM(rm, reg, 3);
}

static uint8_t sysbvm_jit_x86_rex(bool W, bool R, bool X, bool B)
{
    return 0x40 | ((W ? 1 : 0) << 3) | ((R ? 1 : 0) << 2) | ((X ? 1 : 0) << 1) | (B ? 1 : 0);
}

static void sysbvm_jit_x86_int3(sysbvm_bytecodeJit_t *jit)
{
    uint8_t instruction[] = {
        0xCC,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_ud2(sysbvm_bytecodeJit_t *jit)
{
    uint8_t instruction[] = {
        0x0F, 0x0B,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

#if 0
static void sysbvm_jit_x86_callRelative32(sysbvm_bytecodeJit_t *jit, void *functionPointer)
{
    uint8_t instruction[] = {
        0xE8, 0x00, 0x00, 0x00, 0x00
    };

    size_t relocationOffset = sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;
    
    // This direct copy is placed here for debugging purposes.
    int32_t disp = (intptr_t) functionPointer - (intptr_t)(jit->instructions + jit->instructionsSize);
    memcpy(jit->instructions + relocationOffset, &disp, 4);

    sysbvm_bytecodeJitRelocation_t relocation = {
        .offset = relocationOffset,
        .type = SYSBVM_BYTECODE_JIT_RELOCATION_RELATIVE32,
        .value = (intptr_t)functionPointer,
        .addend = -4
    };
    sysbvm_bytecodeJit_addRelocation(jit, relocation);
}

static void sysbvm_jit_x86_callAbsolute64InlineConstant(sysbvm_bytecodeJit_t *jit, void *functionPointer)
{
    sysbvm_jit_x86_mov64Absolute(jit, SYSBVM_X86_RAX, (uint64_t)functionPointer);
    uint8_t instruction[] = {
        0xFF,
        sysbvm_jit_x86_modRMRegister(SYSBVM_X86_RAX, 2),
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}
#endif

static void sysbvm_jit_x86_callAbsoluteNon64InlineConstant(sysbvm_bytecodeJit_t *jit, void *functionPointer)
{
    size_t constantOffset = sysbvm_bytecodeJit_addConstantsBytes(jit, sizeof(functionPointer), (uint8_t*)&functionPointer);
    uint8_t instruction[] = {
        0xFF,
        sysbvm_jit_x86_modRM(5, 2, 0),
        0x00, 0x00, 0x00, 0x00,
    };

    size_t relocationOffset = sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;

    sysbvm_bytecodeJitRelocation_t relocation = {
        .offset = relocationOffset,
        .type = SYSBVM_BYTECODE_JIT_RELOCATION_RELATIVE32,
        .value = constantOffset,
        .addend = -4
    };
    sysbvm_bytecodeJit_addRelocation(jit, relocation);
}

static void sysbvm_jit_x86_call(sysbvm_bytecodeJit_t *jit, void *functionPointer)
{
    sysbvm_jit_x86_callAbsoluteNon64InlineConstant(jit, functionPointer);
}

static void sysbvm_jit_x86_pushRegister(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t reg)
{
    if(reg > SYSBVM_X86_REG_HALF_MASK)
        sysbvm_bytecodeJit_addByte(jit, sysbvm_jit_x86_rex(false, false, false, true));
    sysbvm_bytecodeJit_addByte(jit, 0x50 + (reg & SYSBVM_X86_REG_HALF_MASK));
}

static void sysbvm_jit_x86_popRegister(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t reg)
{
    if(reg > SYSBVM_X86_REG_HALF_MASK)
        sysbvm_bytecodeJit_addByte(jit, sysbvm_jit_x86_rex(false, false, false, true));
    sysbvm_bytecodeJit_addByte(jit, 0x58 + (reg & SYSBVM_X86_REG_HALF_MASK));
}

static void sysbvm_jit_x86_endbr64(sysbvm_bytecodeJit_t *jit)
{
    uint8_t instruction[] = {
        0xF3, 0x0F, 0x1E, 0xFA,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_ret(sysbvm_bytecodeJit_t *jit)
{
    sysbvm_bytecodeJit_addByte(jit, 0xc3);
}

static void sysbvm_jit_x86_mov64Register(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, sysbvm_x86_register_t source)
{
    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, destination > SYSBVM_X86_REG_HALF_MASK, false, source > SYSBVM_X86_REG_HALF_MASK),
        0x8B,
        sysbvm_jit_x86_modRMRegister(source, destination),
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_mov64Absolute(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, uint64_t value)
{
    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, false, false, destination > SYSBVM_X86_REG_HALF_MASK),
        0xB8 + (destination & SYSBVM_X86_REG_HALF_MASK),
        value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF,
        (value >> 32) & 0xFF, (value >> 40) & 0xFF, (value >> 48) & 0xFF, (value >> 56) & 0xFF,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_subImmediate32(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, int32_t value)
{
    if(value == 0)
        return;

    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, false, false, destination > SYSBVM_X86_REG_HALF_MASK),
        0x81,
        sysbvm_jit_x86_modRMRegister(destination, 5),
        value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_movImmediate32(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, int32_t value)
{
    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, false, false, destination > SYSBVM_X86_REG_HALF_MASK),
        0xC7,
        sysbvm_jit_x86_modRMRegister(destination, 0),
        value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF, (value >> 24) & 0xFF,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_leaRegisterWithOffset(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, sysbvm_x86_register_t source, int32_t offset)
{
    if((source & SYSBVM_X86_REG_HALF_MASK) == SYSBVM_X86_RSP)
    {
        uint8_t instruction[] = {
            sysbvm_jit_x86_rex(true, destination > SYSBVM_X86_REG_HALF_MASK, false, source > SYSBVM_X86_REG_HALF_MASK),
            0x8D,
            sysbvm_jit_x86_modRM(source, destination, 2),
            sysbvm_jit_x86_sibOnlyBase(source),
            offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
        };

        sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
    else
    {
        uint8_t instruction[] = {
            sysbvm_jit_x86_rex(true, destination > SYSBVM_X86_REG_HALF_MASK, false, source > SYSBVM_X86_REG_HALF_MASK),
            0x8D,
            sysbvm_jit_x86_modRM(source, destination, 2),
            offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
        };

        sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
}

static void sysbvm_jit_x86_mov64FromMemoryWithOffset(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, sysbvm_x86_register_t source, int32_t offset)
{
    if(offset == 0 && source != SYSBVM_X86_RBP)
    {
        if((source & SYSBVM_X86_REG_HALF_MASK) == SYSBVM_X86_RSP)
        {
            uint8_t instruction[] = {
                sysbvm_jit_x86_rex(true, destination > SYSBVM_X86_REG_HALF_MASK, false, source > SYSBVM_X86_REG_HALF_MASK),
                0x8B,
                sysbvm_jit_x86_modRM(source, destination, 0),
                sysbvm_jit_x86_sibOnlyBase(source),
            };

            sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
        }
        else
        {
            uint8_t instruction[] = {
                sysbvm_jit_x86_rex(true, destination > SYSBVM_X86_REG_HALF_MASK, false, source > SYSBVM_X86_REG_HALF_MASK),
                0x8B,
                sysbvm_jit_x86_modRM(source, destination, 0),
            };

            sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
        }
    }
    else
    {
        if((source & SYSBVM_X86_REG_HALF_MASK) == SYSBVM_X86_RSP)
        {
            uint8_t instruction[] = {
                sysbvm_jit_x86_rex(true, destination > SYSBVM_X86_REG_HALF_MASK, false, source > SYSBVM_X86_REG_HALF_MASK),
                0x8B,
                sysbvm_jit_x86_modRM(source, destination, 2),
                sysbvm_jit_x86_sibOnlyBase(source),
                offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
            };

            sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
        }
        else
        {
            uint8_t instruction[] = {
                sysbvm_jit_x86_rex(true, destination > SYSBVM_X86_REG_HALF_MASK, false, source > SYSBVM_X86_REG_HALF_MASK),
                0x8B,
                sysbvm_jit_x86_modRM(source, destination, 2),
                offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
            };

            sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
        }
    }
}

static void sysbvm_jit_x86_mov64IntoMemoryWithOffset(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, int32_t offset, sysbvm_x86_register_t source)
{
    if(offset == 0 && (destination & SYSBVM_X86_REG_HALF_MASK) != SYSBVM_X86_RBP)
    {
        if((destination & SYSBVM_X86_REG_HALF_MASK) == SYSBVM_X86_RSP)
        {
            uint8_t instruction[] = {
                sysbvm_jit_x86_rex(true, source > SYSBVM_X86_REG_HALF_MASK, false, destination > SYSBVM_X86_REG_HALF_MASK),
                0x89,
                sysbvm_jit_x86_sibOnlyBase(destination),
                sysbvm_jit_x86_modRM(destination, source, 0),
            };

            sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);            
        }
        else
        {
            uint8_t instruction[] = {
                sysbvm_jit_x86_rex(true, source > SYSBVM_X86_REG_HALF_MASK, false, destination > SYSBVM_X86_REG_HALF_MASK),
                0x89,
                sysbvm_jit_x86_modRM(destination, source, 0),
            };

            sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);            
        }
    }
    else
    {
        if((destination & SYSBVM_X86_REG_HALF_MASK) == SYSBVM_X86_RSP)
        {
            uint8_t instruction[] = {
                sysbvm_jit_x86_rex(true, source > SYSBVM_X86_REG_HALF_MASK, false, destination > SYSBVM_X86_REG_HALF_MASK),
                0x89,
                sysbvm_jit_x86_modRM(destination, source, 2),
                sysbvm_jit_x86_sibOnlyBase(destination),
                offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
            };

            sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
        }
        else
        {
            uint8_t instruction[] = {
                sysbvm_jit_x86_rex(true, source > SYSBVM_X86_REG_HALF_MASK, false, destination > SYSBVM_X86_REG_HALF_MASK),
                0x89,
                sysbvm_jit_x86_modRM(destination, source, 2),
                offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
            };

            sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
        }
    }
}

static void sysbvm_jit_x86_movS32IntoMemoryWithOffset(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, int32_t offset, int32_t immediate)
{
    if(offset == 0 && (destination & SYSBVM_X86_REG_HALF_MASK) != SYSBVM_X86_RBP)
    {
        if((destination & SYSBVM_X86_REG_HALF_MASK) == SYSBVM_X86_RSP)
        {
            uint8_t instruction[] = {
                sysbvm_jit_x86_rex(true, false, false, destination > SYSBVM_X86_REG_HALF_MASK),
                0xC7,
                sysbvm_jit_x86_modRM(destination, 0, 0),
                sysbvm_jit_x86_sibOnlyBase(destination),
                immediate & 0xFF, (immediate >> 8) & 0xFF, (immediate >> 16) & 0xFF, (immediate >> 24) & 0xFF
            };

            sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
        }
        else
        {
            uint8_t instruction[] = {
                sysbvm_jit_x86_rex(true, false, false, destination > SYSBVM_X86_REG_HALF_MASK),
                0xC7,
                sysbvm_jit_x86_modRM(destination, 0, 0),
                immediate & 0xFF, (immediate >> 8) & 0xFF, (immediate >> 16) & 0xFF, (immediate >> 24) & 0xFF
            };

            sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
        }
    }
    else
    {
        if((destination & SYSBVM_X86_REG_HALF_MASK) == SYSBVM_X86_RSP)
        {
            uint8_t instruction[] = {
                sysbvm_jit_x86_rex(true, false, false, destination > SYSBVM_X86_REG_HALF_MASK),
                0xC7,
                sysbvm_jit_x86_modRM(destination, 0, 2),
                sysbvm_jit_x86_sibOnlyBase(destination),
                offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
                immediate & 0xFF, (immediate >> 8) & 0xFF, (immediate >> 16) & 0xFF, (immediate >> 24) & 0xFF
            };

            sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
        }
        else
        {
            uint8_t instruction[] = {
                sysbvm_jit_x86_rex(true, false, false, destination > SYSBVM_X86_REG_HALF_MASK),
                0xC7,
                sysbvm_jit_x86_modRM(destination, 0, 2),
                offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
                immediate & 0xFF, (immediate >> 8) & 0xFF, (immediate >> 16) & 0xFF, (immediate >> 24) & 0xFF
            };

            sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
        }
    }
}

static void sysbvm_jit_x86_logicalShiftRightImmediate(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, uint8_t shiftAmount)
{
    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, false, false, destination > SYSBVM_X86_REG_HALF_MASK),
        0xC1,
        sysbvm_jit_x86_modRMRegister(destination, 5),
        shiftAmount
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_mov8IntoMemoryWithOffset(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, int32_t offset, sysbvm_x86_register_t source)
{
    if(offset == 0 && destination != SYSBVM_X86_RBP)
    {
        uint8_t instruction[] = {
            sysbvm_jit_x86_rex(true, source > SYSBVM_X86_REG_HALF_MASK, false, destination > SYSBVM_X86_REG_HALF_MASK),
            0x88,
            sysbvm_jit_x86_modRM(destination, source, 0),
        };

        sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
    else
    {
        uint8_t instruction[] = {
            sysbvm_jit_x86_rex(true, source > SYSBVM_X86_REG_HALF_MASK, false, destination > SYSBVM_X86_REG_HALF_MASK),
            0x88,
            sysbvm_jit_x86_modRM(destination, source, 2),
            offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
        };

        sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
    }
}

static void sysbvm_jit_x86_movImmediateI32IntoMemory64WithOffset(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, int32_t offset, int16_t immediate)
{
    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, false, false, destination > SYSBVM_X86_REG_HALF_MASK),
        0xC7,
        sysbvm_jit_x86_modRM(destination, 0, 2),
        offset & 0xFF, (offset >> 8) & 0xFF, (offset >> 16) & 0xFF, (offset >> 24) & 0xFF,
        immediate & 0xFF, (immediate >> 8) & 0xFF, (immediate >> 16) & 0xFF, (immediate >> 24) & 0xFF,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}


static void sysbvm_jit_x86_xorRegister(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t destination, sysbvm_x86_register_t source)
{
    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, destination > SYSBVM_X86_REG_HALF_MASK, false, source > SYSBVM_X86_REG_HALF_MASK),
        0x33,
        sysbvm_jit_x86_modRMRegister(source, destination),
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

static void sysbvm_jit_x86_jitLoadContextInRegister(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t reg)
{
    sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, SYSBVM_X86_RBP, jit->contextPointerOffset);
}

SYSBVM_API void sysbvm_jit_breakpoint(sysbvm_bytecodeJit_t *jit)
{
    sysbvm_jit_x86_int3(jit);
}

SYSBVM_API void sysbvm_jit_unreachable(sysbvm_bytecodeJit_t *jit)
{
    sysbvm_jit_x86_ud2(jit);
}

SYSBVM_API void sysbvm_jit_callNoResult0(sysbvm_bytecodeJit_t *jit, void *functionPointer)
{
    sysbvm_jit_x86_call(jit, functionPointer);
}

SYSBVM_API void sysbvm_jit_callWithContextNoResult0(sysbvm_bytecodeJit_t *jit, void *functionPointer)
{
    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);
    sysbvm_jit_x86_call(jit, functionPointer);
}

SYSBVM_API void sysbvm_jit_callWithContext1(sysbvm_bytecodeJit_t *jit, void *functionPointer, int16_t resultOperand, int16_t argumentOperand0)
{
    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG1, argumentOperand0);
    sysbvm_jit_x86_call(jit, functionPointer);
    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

SYSBVM_API void sysbvm_jit_callWithContext2(sysbvm_bytecodeJit_t *jit, void *functionPointer, int16_t resultOperand, int16_t argumentOperand0, int16_t argumentOperand1)
{
    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG1, argumentOperand0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG2, argumentOperand1);
    sysbvm_jit_x86_call(jit, functionPointer);
    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

SYSBVM_API void sysbvm_jit_callWithContextNoResult2(sysbvm_bytecodeJit_t *jit, void *functionPointer, int16_t argumentOperand0, int16_t argumentOperand1)
{
    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG1, argumentOperand0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG2, argumentOperand1);
    sysbvm_jit_x86_call(jit, functionPointer);
}

SYSBVM_API void sysbvm_jit_callWithContextNoResult3(sysbvm_bytecodeJit_t *jit, void *functionPointer, int16_t argumentOperand0, int16_t argumentOperand1, int16_t argumentOperand2)
{
    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG1, argumentOperand0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG2, argumentOperand1);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG3, argumentOperand2);
    sysbvm_jit_x86_call(jit, functionPointer);
}

static void sysbvm_jit_functionApplyVia(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t functionOperand, size_t argumentCount, int16_t *argumentOperands, int applicationFlags, void *calledFunctionPointer)
{
    // Move the arguments into the call vector.
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_jit_moveOperandToCallArgumentVector(jit, argumentOperands[i], (int32_t)i);

    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG1, functionOperand);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_64_ARG2, (int32_t)argumentCount);
    sysbvm_jit_x86_leaRegisterWithOffset(jit, SYSBVM_X86_64_ARG3, SYSBVM_X86_RBP, jit->callArgumentVectorOffset);
#ifdef _WIN32
    sysbvm_jit_x86_movS32IntoMemoryWithOffset(jit, SYSBVM_X86_RSP, 4*sizeof(void*), applicationFlags);
#else
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_SYSV_ARG4, applicationFlags);
#endif
    sysbvm_jit_x86_call(jit, calledFunctionPointer);

    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);

}

static void sysbvm_jit_functionApplyDirectVia(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t functionOperand, size_t argumentCount, int16_t *argumentOperands, void *calledFunctionPointer)
{
    // Move the arguments into the call vector.
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_jit_moveOperandToCallArgumentVector(jit, argumentOperands[i], (int32_t)i);

    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG1, functionOperand);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_64_ARG2, (int32_t)argumentCount);
    sysbvm_jit_x86_leaRegisterWithOffset(jit, SYSBVM_X86_64_ARG3, SYSBVM_X86_RBP, jit->callArgumentVectorOffset);

    sysbvm_jit_x86_call(jit, calledFunctionPointer);

    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

static void sysbvm_jit_functionApplyDirectWithArgumentsRecord(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t functionOperand, size_t argumentCount, int16_t *argumentOperands, void *calledFunctionPointer)
{
    // Move the arguments into the call vector.
    for(size_t i = 0; i < argumentCount; ++i)
        sysbvm_jit_moveOperandToCallArgumentVector(jit, argumentOperands[i], (int32_t)i);
    sysbvm_jit_x86_movS32IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, jit->callArgumentVectorSizeOffset, (int32_t)argumentCount);

    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG1, functionOperand);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_64_ARG2, (int32_t)argumentCount);
    sysbvm_jit_x86_leaRegisterWithOffset(jit, SYSBVM_X86_64_ARG3, SYSBVM_X86_RBP, jit->callArgumentVectorOffset);

    sysbvm_jit_x86_call(jit, calledFunctionPointer);
    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
    sysbvm_jit_x86_movS32IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, jit->callArgumentVectorSizeOffset, 0);
}

static void *sysbvm_jit_getTrampolineOrEntryPointForBytecode(sysbvm_bytecodeJit_t *jit, sysbvm_functionBytecode_t *bytecode)
{
    // Attempt direct entry first.
    if(bytecode->jittedCode && bytecode->jittedCodeSessionToken == jit->context->roots.sessionToken)
        return (void*)sysbvm_tuple_systemHandle_decode(bytecode->jittedCode);

    if(bytecode->jittedCodeTrampoline && bytecode->jittedCodeTrampolineSessionToken == jit->context->roots.sessionToken)
        return (void*)sysbvm_tuple_systemHandle_decode(bytecode->jittedCodeTrampoline);

    uint64_t trampolineTargetAddress = (uint64_t)(uintptr_t)&sysbvm_bytecodeInterpreter_applyJitTrampolineDestination;

    uint8_t trampolineCode[] = {
        // Endbr64
        0xF3, 0x0F, 0x1E, 0xFA,

        // Mov64
        sysbvm_jit_x86_rex(true, false, false, false),
        0xB8 + SYSBVM_X86_RAX,
        trampolineTargetAddress & 0xFF, (trampolineTargetAddress >> 8) & 0xFF, (trampolineTargetAddress >> 16) & 0xFF, (trampolineTargetAddress >> 24) & 0xFF,
        (trampolineTargetAddress >> 32) & 0xFF, (trampolineTargetAddress >> 40) & 0xFF, (trampolineTargetAddress >> 48) & 0xFF, (trampolineTargetAddress >> 56) & 0xFF,

        // Jmp RAX
        0xFF,
        sysbvm_jit_x86_modRMRegister(SYSBVM_X86_RAX, 4),
    };

    // Install the trampoline in the code zone.
    size_t trampolineCodeSize = sizeof(trampolineCode);
    size_t requiredCodeSize = sysbvm_sizeAlignedTo(trampolineCodeSize, 16);

    uint8_t *trampolineWritePointer = NULL;
    uint8_t *trampolineExecutablePointer = NULL;
    sysbvm_chunkedAllocator_allocateWithDualMapping(&jit->context->heap.codeAllocator, requiredCodeSize, 16, (void**)&trampolineWritePointer, (void**)&trampolineExecutablePointer);
    if(!sysbvm_virtualMemory_lockCodePagesForWriting(trampolineWritePointer, trampolineExecutablePointer, requiredCodeSize))
        abort();

    memset(trampolineWritePointer, 0xcc, requiredCodeSize); // int3;
    memcpy(trampolineWritePointer, trampolineCode, trampolineCodeSize);

    sysbvm_virtualMemory_unlockCodePagesForExecution(trampolineWritePointer, trampolineExecutablePointer, requiredCodeSize);

    bytecode->jittedCodeTrampoline = sysbvm_tuple_systemHandle_encode(jit->context, (sysbvm_systemHandle_t)(uintptr_t)trampolineExecutablePointer);
    bytecode->jittedCodeTrampolineWritePointer = sysbvm_tuple_systemHandle_encode(jit->context, (sysbvm_systemHandle_t)(uintptr_t)trampolineWritePointer);
    bytecode->jittedCodeTrampolineSessionToken = jit->context->roots.sessionToken;

    return trampolineExecutablePointer;
}

SYSBVM_API void sysbvm_jit_patchTrampolineWithRealEntryPoint(sysbvm_bytecodeJit_t *jit, sysbvm_functionBytecode_t *bytecode)
{
    if(bytecode->jittedCodeTrampoline && bytecode->jittedCodeTrampolineWritePointer && bytecode->jittedCodeTrampolineSessionToken == jit->context->roots.sessionToken)
    {
        uint8_t *realEntryPoint = (uint8_t*)sysbvm_tuple_systemHandle_decode(bytecode->jittedCode);
        uint8_t *trampolineEntryPointExecutePointer = (uint8_t*)sysbvm_tuple_systemHandle_decode(bytecode->jittedCodeTrampoline);
        uint8_t *trampolineEntryPointWritePointer = (uint8_t*)sysbvm_tuple_systemHandle_decode(bytecode->jittedCodeTrampolineWritePointer);

        size_t trampolineCodeSize = 16;
        if(!sysbvm_virtualMemory_lockCodePagesForWriting(trampolineEntryPointWritePointer, trampolineEntryPointExecutePointer, trampolineCodeSize))
            abort();

        size_t targetAddressOffset = trampolineCodeSize - 2 - sizeof(void*);
        uintptr_t *jumpAddressLocation = (uintptr_t*)(trampolineEntryPointWritePointer + targetAddressOffset);
        *jumpAddressLocation = (uintptr_t)realEntryPoint;

        sysbvm_virtualMemory_unlockCodePagesForExecution(trampolineEntryPointWritePointer, trampolineEntryPointExecutePointer, trampolineCodeSize);
    }
}

SYSBVM_API void sysbvm_jit_functionApply(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t functionOperand, size_t argumentCount, int16_t *argumentOperands, int applicationFlags)
{
    bool isNoTypecheck = applicationFlags & SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK;

    // Are we calling a literal function?
    sysbvm_tuple_t literalFunction = SYSBVM_NULL_TUPLE;

    if(isNoTypecheck
        && sysbvm_bytecodeJit_getLiteralValueForOperand(jit, functionOperand, &literalFunction)
        && sysbvm_tuple_isFunction(jit->context, literalFunction)
        && !sysbvm_function_isMemoized(jit->context, literalFunction)
        && !sysbvm_function_isVariadic(jit->context, literalFunction))
    {
        // Is this a numbered primitive.
        sysbvm_function_t *literalFunctionObject = (sysbvm_function_t*)literalFunction;
        if(literalFunctionObject->primitiveTableIndex)
        {
            uint32_t primitiveNumber = sysbvm_tuple_uint32_decode(literalFunctionObject->primitiveTableIndex);
            sysbvm_functionEntryPoint_t entryPoint = sysbvm_function_getNumberedPrimitiveEntryPoint(jit->context, primitiveNumber);
            if(entryPoint)
            {
                sysbvm_jit_functionApplyDirectWithArgumentsRecord(jit, resultOperand, functionOperand, argumentCount, argumentOperands, entryPoint);
                return;
            }
        }
        
        if(literalFunctionObject->definition)
        {
            sysbvm_functionDefinition_t *literalFunctionDefinitionObject = (sysbvm_functionDefinition_t*)literalFunctionObject->definition;
            if(literalFunctionDefinitionObject->bytecode && literalFunctionDefinitionObject->bytecode != SYSBVM_PENDING_MEMOIZATION_VALUE)
            {
                void *trampolineOrEntryPoint = sysbvm_jit_getTrampolineOrEntryPointForBytecode(jit, (sysbvm_functionBytecode_t*)literalFunctionDefinitionObject->bytecode);
                if(trampolineOrEntryPoint)
                {
                    //printf("trampolineOrEntryPoint %p\n", trampolineOrEntryPoint);
                    sysbvm_jit_functionApplyDirectVia(jit, resultOperand, functionOperand, argumentCount, argumentOperands, trampolineOrEntryPoint);
                    return;
                }
            }
        }
    }

    sysbvm_jit_functionApplyVia(jit, resultOperand, functionOperand, argumentCount, argumentOperands, applicationFlags, &sysbvm_bytecodeInterpreter_functionApplyNoCopyArguments);
}

SYSBVM_API void sysbvm_jit_send(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t selectorOperand, size_t argumentCount, int16_t *argumentOperands, int applicationFlags)
{
    // Move the arguments into the call vector.
    for(size_t i = 0; i < argumentCount + 1; ++i)
        sysbvm_jit_moveOperandToCallArgumentVector(jit, argumentOperands[i], (int32_t)i);

    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);

    sysbvm_pic_t *pic = (sysbvm_pic_t*)sysbvm_chunkedAllocator_allocate(&jit->context->heap.picTableAllocator, sizeof(sysbvm_pic_t), sizeof(uintptr_t));
    sysbvm_jit_x86_mov64Absolute(jit, SYSBVM_X86_64_ARG1, (uint64_t)pic);

    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG2, selectorOperand);

    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_64_ARG3, (int32_t)argumentCount);

#ifdef _WIN32
    sysbvm_jit_x86_leaRegisterWithOffset(jit, SYSBVM_X86_RAX, SYSBVM_X86_RBP, jit->callArgumentVectorOffset);
    sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_RSP, 4*sizeof(void*), SYSBVM_X86_RAX);

    sysbvm_jit_x86_movS32IntoMemoryWithOffset(jit, SYSBVM_X86_RSP, 5*sizeof(void*), applicationFlags);
#else
    sysbvm_jit_x86_leaRegisterWithOffset(jit, SYSBVM_X86_SYSV_ARG4, SYSBVM_X86_RBP, jit->callArgumentVectorOffset);

    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_SYSV_ARG5, applicationFlags);
#endif
    sysbvm_jit_x86_call(jit, &sysbvm_bytecodeInterpreter_interpretSendNoCopyArguments);

    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

SYSBVM_API void sysbvm_jit_sendWithReceiverType(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t receiverTypeOperand, int16_t selectorOperand, size_t argumentCount, int16_t *argumentOperands, int applicationFlags)
{
    // Push all of the arguments in the stack.
    for(size_t i = 0; i < argumentCount + 1; ++i)
        sysbvm_jit_moveOperandToCallArgumentVector(jit, argumentOperands[i], (int32_t)i);

    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);

    sysbvm_pic_t *pic = (sysbvm_pic_t*)sysbvm_chunkedAllocator_allocate(&jit->context->heap.picTableAllocator, sizeof(sysbvm_pic_t), sizeof(uintptr_t));
    sysbvm_jit_x86_mov64Absolute(jit, SYSBVM_X86_64_ARG1, (uint64_t)pic);

    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG2, receiverTypeOperand);

    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG3, selectorOperand);
#ifdef _WIN32
    sysbvm_jit_x86_movS32IntoMemoryWithOffset(jit, SYSBVM_X86_RSP, 4*sizeof(void*), (int32_t)argumentCount);

    sysbvm_jit_x86_leaRegisterWithOffset(jit, SYSBVM_X86_RAX, SYSBVM_X86_RBP, jit->callArgumentVectorOffset);
    sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_RSP, 5*sizeof(void*), SYSBVM_X86_RAX);

    sysbvm_jit_x86_movS32IntoMemoryWithOffset(jit, SYSBVM_X86_RSP, 6*sizeof(void*), applicationFlags);
#else
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_SYSV_ARG4, (int32_t)argumentCount);

    sysbvm_jit_x86_leaRegisterWithOffset(jit, SYSBVM_X86_SYSV_ARG5, SYSBVM_X86_RBP, jit->callArgumentVectorOffset);

    sysbvm_jit_x86_movS32IntoMemoryWithOffset(jit, SYSBVM_X86_RSP, 0*sizeof(void*), applicationFlags);
#endif
    sysbvm_jit_x86_call(jit, &sysbvm_bytecodeInterpreter_interpretSendWithReceiverTypeNoCopyArguments);

    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

SYSBVM_API void sysbvm_jit_makeArray(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, size_t elementCount, int16_t *elementOperands)
{
    if(resultOperand < 0)
        return;

    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_64_ARG1, (int32_t)elementCount);
    sysbvm_jit_x86_call(jit, &sysbvm_array_create);

    for(size_t i = 0; i < elementCount; ++i)
    {
        sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG2, elementOperands[i]);
        sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_RAX, (int32_t)(sizeof(sysbvm_tuple_header_t) + i * sizeof(void*)), SYSBVM_X86_64_ARG2);
    }

    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

SYSBVM_API void sysbvm_jit_makeByteArray(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, size_t elementCount, int16_t *elementOperands)
{
    if(resultOperand < 0)
        return;

    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_64_ARG1, (int32_t)elementCount);
    sysbvm_jit_x86_call(jit, &sysbvm_byteArray_create);

    for(size_t i = 0; i < elementCount; ++i)
    {
        sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG2, elementOperands[i]);
        sysbvm_jit_x86_logicalShiftRightImmediate(jit, SYSBVM_X86_64_ARG2, SYSBVM_TUPLE_TAG_BIT_COUNT);
        sysbvm_jit_x86_mov8IntoMemoryWithOffset(jit, SYSBVM_X86_RAX, (int32_t) (sizeof(sysbvm_tuple_header_t) + i), SYSBVM_X86_64_ARG2);
    }

    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

SYSBVM_API void sysbvm_jit_makeDictionary(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, size_t elementCount, int16_t *elementOperands)
{
    if(resultOperand < 0)
        return;

    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);
    sysbvm_jit_x86_movImmediate32(jit, SYSBVM_X86_64_ARG1, (int32_t)elementCount);
    sysbvm_jit_x86_call(jit, &sysbvm_dictionary_createWithCapacity);
    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);

    for(size_t i = 0; i < elementCount; ++i)
    {
        sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);
        sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG1, resultOperand);
        sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG2, elementOperands[i]);
        sysbvm_jit_x86_call(jit, &sysbvm_dictionary_add);
    }
}

SYSBVM_API void sysbvm_jit_makeClosureWithCaptures(sysbvm_bytecodeJit_t *jit, int16_t resultOperand, int16_t functionDefinitionOperand, size_t captureCount, int16_t *elementOperands)
{
    if(resultOperand < 0)
        return;

    // Make the capture vector.
    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG1, functionDefinitionOperand);
    sysbvm_jit_x86_call(jit, &sysbvm_sequenceTuple_createForFunctionDefinition);

    sysbvm_jit_x86_mov64Register(jit, SYSBVM_X86_64_ARG2, SYSBVM_X86_RAX);
    for(size_t i = 0; i < captureCount; ++i)
    {
        sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG1, elementOperands[i]);
        sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_64_ARG2, (int32_t) (sizeof(sysbvm_tuple_header_t) + i * sizeof(void*)), SYSBVM_X86_64_ARG1);
    }

    // Now construct the actual closure
    sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG1, functionDefinitionOperand);
    sysbvm_jit_x86_call(jit, &sysbvm_function_createClosureWithCaptureVector);
    sysbvm_jit_moveRegisterToOperand(jit, resultOperand, SYSBVM_X86_RAX);
}

SYSBVM_API void sysbvm_jit_jumpRelative(sysbvm_bytecodeJit_t *jit, size_t targetPC)
{
    uint8_t instruction[] = {
        0xE9, 0x00, 0x00, 0x00, 0x00,
    };

    size_t relocationOffset = sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;
    sysbvm_bytecodeJitPCRelocation_t relocation = {
        .offset = relocationOffset,
        .targetPC = targetPC,
        .addend = -4,
    };
    sysbvm_bytecodeJit_addPCRelocation(jit, relocation);
}

static void sysbvm_jit_x86_cmpRAXWithImmediate32(sysbvm_bytecodeJit_t *jit, int32_t immediate)
{
    uint8_t instruction[] = {
        sysbvm_jit_x86_rex(true, false, false, false),
        0x3D,
        immediate & 0xFF, (immediate >> 8) & 0xFF, (immediate >> 16) & 0xFF, (immediate >> 24) & 0xFF,
    };

    sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction);
}

SYSBVM_API void sysbvm_jit_jumpRelativeIfTrue(sysbvm_bytecodeJit_t *jit, int16_t conditionOperand, size_t targetPC)
{
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_RAX, conditionOperand);
    sysbvm_jit_x86_cmpRAXWithImmediate32(jit, SYSBVM_TRUE_TUPLE);

    uint8_t instruction[] = {
        // Jeq
        0x0F, 0x84, 0x00, 0x00, 0x00, 0x00,
    };

    size_t relocationOffset = sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;
    sysbvm_bytecodeJitPCRelocation_t relocation = {
        .offset = relocationOffset,
        .targetPC = targetPC,
        .addend = -4,
    };
    sysbvm_bytecodeJit_addPCRelocation(jit, relocation);
}

SYSBVM_API void sysbvm_jit_jumpRelativeIfFalse(sysbvm_bytecodeJit_t *jit, int16_t conditionOperand, size_t targetPC)
{
    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_RAX, conditionOperand);
    sysbvm_jit_x86_cmpRAXWithImmediate32(jit, SYSBVM_TRUE_TUPLE);

    uint8_t instruction[] = {
        // Jne
        0x0F, 0x85, 0x00, 0x00, 0x00, 0x00,
    };

    size_t relocationOffset = sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;
    sysbvm_bytecodeJitPCRelocation_t relocation = {
        .offset = relocationOffset,
        .targetPC = targetPC,
        .addend = -4,
    };
    sysbvm_bytecodeJit_addPCRelocation(jit, relocation);
}


SYSBVM_API void sysbvm_jit_caseJump(sysbvm_bytecodeJit_t *jit, int16_t valueOperand, size_t caseCount, int16_t *caseKeyOperands, int16_t *caseLabelOperands, int16_t defaultLabelOperand, size_t pc)
{
    for(size_t i = 0; i < caseCount; ++i)
    {
        sysbvm_jit_x86_jitLoadContextInRegister(jit, SYSBVM_X86_64_ARG0);
        sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG1, valueOperand);
        sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_64_ARG2, caseKeyOperands[i]);
        sysbvm_jit_x86_call(jit, &sysbvm_tuple_equals);

        uint8_t instruction[] = {
            // test AL, AL
            0x84, sysbvm_jit_x86_modRMRegister(SYSBVM_X86_RAX, SYSBVM_X86_RAX),

            // jnz
            0x0F, 0x85, 0x00, 0x00, 0x00, 0x00,
        };

        size_t relocationOffset = sysbvm_bytecodeJit_addBytes(jit, sizeof(instruction), instruction) - 4;
        sysbvm_bytecodeJitPCRelocation_t relocation = {
            .offset = relocationOffset,
            .targetPC = pc + caseLabelOperands[i],
            .addend = -4,
        };
        sysbvm_bytecodeJit_addPCRelocation(jit, relocation);
    }

    sysbvm_jit_jumpRelative(jit, pc + defaultLabelOperand);
}

static void sysbvm_jit_cfi_beginPrologue(sysbvm_bytecodeJit_t *jit)
{
    sysbvm_dwarf_cie_t ehCie = {0};
    ehCie.codeAlignmentFactor = 1;
    ehCie.dataAlignmentFactor = -sizeof(uintptr_t);
    ehCie.pointerSize = sizeof(uintptr_t);
    ehCie.returnAddressRegister = sizeof(uintptr_t) == 8 ? DW_X64_REG_RA : DW_X86_REG_RA;
    jit->dwarfEhBuilder.initialStackFrameSize = 1; // Return address
    jit->dwarfEhBuilder.stackPointerRegister = sizeof(uintptr_t) == 8 ? DW_X64_REG_RSP : DW_X86_REG_ESP;
    sysbvm_dwarf_cfi_beginCIE(&jit->dwarfEhBuilder, &ehCie);
    sysbvm_dwarf_cfi_cfaInRegisterWithFactoredOffset(&jit->dwarfEhBuilder, jit->dwarfEhBuilder.stackPointerRegister, 1);
    sysbvm_dwarf_cfi_registerValueAtFactoredOffset(&jit->dwarfEhBuilder, sizeof(uintptr_t) == 8 ? DW_X64_REG_RA : DW_X86_REG_RA, 1);

    sysbvm_dwarf_cfi_endCIE(&jit->dwarfEhBuilder);
    sysbvm_dwarf_cfi_beginFDE(&jit->dwarfEhBuilder, jit->instructions.size);
}

static void sysbvm_jit_cfi_pushRBP(sysbvm_bytecodeJit_t *jit)
{
#ifdef _WIN32
    sysbvm_bytecodeJit_uwop_pushNonVol(jit, 5);
#endif
    sysbvm_dwarf_cfi_setPC(&jit->dwarfEhBuilder, jit->instructions.size);
    sysbvm_dwarf_cfi_pushRegister(&jit->dwarfEhBuilder, sizeof(uintptr_t) == 8 ? DW_X64_REG_RBP : DW_X86_REG_EBP);
}

static void sysbvm_jit_cfi_storeStackInFramePointer(sysbvm_bytecodeJit_t *jit, int32_t offset)
{
#ifdef _WIN32
    SYSBVM_ASSERT((offset % 16) == 0);
    jit->cfiFrameOffset = offset / 16;
    sysbvm_bytecodeJit_uwop_setFPReg(jit);
#endif
    sysbvm_dwarf_cfi_setPC(&jit->dwarfEhBuilder, jit->instructions.size);
    sysbvm_dwarf_cfi_saveFramePointerInRegister(&jit->dwarfEhBuilder, sizeof(uintptr_t) == 8 ? DW_X64_REG_RBP : DW_X86_REG_EBP, offset);
}

static void sysbvm_jit_cfi_subtract(sysbvm_bytecodeJit_t *jit, size_t subtractionAmount)
{
    if(!subtractionAmount) return;
#ifdef _WIN32
    sysbvm_bytecodeJit_uwop_alloc(jit, subtractionAmount);
#endif
    sysbvm_dwarf_cfi_stackSizeAdvance(&jit->dwarfEhBuilder, jit->instructions.size, subtractionAmount);
}

static void sysbvm_jit_cfi_endPrologue(sysbvm_bytecodeJit_t *jit)
{
    jit->prologueSize = jit->instructions.size;
    sysbvm_dwarf_cfi_endPrologue(&jit->dwarfEhBuilder);
}

SYSBVM_API void sysbvm_jit_prologue(sysbvm_bytecodeJit_t *jit)
{
    sysbvm_jit_cfi_beginPrologue(jit);
#ifndef _WIN32
    sysbvm_jit_x86_endbr64(jit);
#endif

    //(sysbvm_context_t *context, sysbvm_tuple_t function, size_t argumentCount, sysbvm_tuple_t *arguments)
    sysbvm_jit_x86_pushRegister(jit, SYSBVM_X86_RBP);
    sysbvm_jit_cfi_pushRBP(jit);

    // Allocate the stack storage.
    size_t requiredStackSize = jit->localVectorSize * sizeof(intptr_t)
        + (sizeof(sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t) - sizeof(intptr_t));
    jit->stackFrameSize = (requiredStackSize + 15) & (-16);
    jit->stackFrameRecordOffset = 0;

#ifdef _WIN32
    jit->stackCallReservationSize = SYSBVM_X86_64_CALL_SHADOW_SPACE + 32;
    sysbvm_jit_x86_subImmediate32(jit, SYSBVM_X86_RSP, jit->stackFrameSize + jit->stackCallReservationSize);
    sysbvm_jit_cfi_subtract(jit, jit->stackFrameSize + jit->stackCallReservationSize);

    sysbvm_jit_x86_leaRegisterWithOffset(jit, SYSBVM_X86_RBP, SYSBVM_X86_RSP, jit->stackCallReservationSize);
    sysbvm_jit_cfi_storeStackInFramePointer(jit, jit->stackCallReservationSize);

#else
    jit->stackCallReservationSize = 16;

    sysbvm_jit_x86_mov64Register(jit, SYSBVM_X86_RBP, SYSBVM_X86_RSP);
    sysbvm_jit_cfi_storeStackInFramePointer(jit, 0);

    sysbvm_jit_x86_subImmediate32(jit, SYSBVM_X86_RSP, jit->stackFrameSize + jit->stackCallReservationSize);
    sysbvm_jit_cfi_subtract(jit, jit->stackFrameSize + jit->stackCallReservationSize);
    jit->stackFrameRecordOffset = -jit->stackFrameSize;
#endif

    sysbvm_jit_cfi_endPrologue(jit);

    // Build the stack frame record.
    {
        sysbvm_jit_x86_movS32IntoMemoryWithOffset(jit,
            SYSBVM_X86_RBP, jit->stackFrameRecordOffset + offsetof(sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t, previous),
            0);
        sysbvm_jit_x86_movS32IntoMemoryWithOffset(jit,
            SYSBVM_X86_RBP, jit->stackFrameRecordOffset + offsetof(sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t, type),
            SYSBVM_STACK_FRAME_RECORD_TYPE_BYTECODE_JIT_FUNCTION_ACTIVATION);

        jit->pcOffset = jit->stackFrameRecordOffset + offsetof(sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t, pc);
        sysbvm_jit_x86_movS32IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, jit->pcOffset, 0);

        jit->contextPointerOffset = jit->stackFrameRecordOffset + offsetof(sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t, context);
        sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, jit->contextPointerOffset, SYSBVM_X86_64_ARG0);

        sysbvm_jit_x86_mov64Absolute(jit, SYSBVM_X86_RAX, (uintptr_t)jit->literalVectorGCRoot); // Pointer to GC root with the literal vector.
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, SYSBVM_X86_RAX, 0, SYSBVM_X86_RAX);
        jit->literalVectorOffset = jit->stackFrameRecordOffset + offsetof(sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t, literalVector);
        sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, jit->literalVectorOffset, SYSBVM_X86_RAX);

        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, SYSBVM_X86_RAX, SYSBVM_X86_64_ARG1, offsetof(sysbvm_function_t, captureVector));
        jit->captureVectorOffset = jit->stackFrameRecordOffset + offsetof(sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t, captureVector);
        sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, jit->captureVectorOffset, SYSBVM_X86_RAX);

        size_t functionOffset = jit->stackFrameRecordOffset + offsetof(sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t, function);
        sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, (int32_t)functionOffset, SYSBVM_X86_64_ARG1);

        size_t argumentCountOffset = jit->stackFrameRecordOffset + offsetof(sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t, argumentCount);
        sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, (int32_t)argumentCountOffset, SYSBVM_X86_64_ARG2);

        jit->callArgumentVectorSizeOffset = jit->stackFrameRecordOffset + offsetof(sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t, callArgumentVectorSize);
        sysbvm_jit_x86_movS32IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, jit->callArgumentVectorSizeOffset, 0);

        jit->callArgumentVectorOffset = jit->stackFrameRecordOffset + offsetof(sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t, callArgumentVector);
        // This is not needed to be cleared.

        jit->argumentVectorOffset = jit->stackFrameRecordOffset + offsetof(sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t, arguments);
        sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, jit->argumentVectorOffset, SYSBVM_X86_64_ARG3);

        size_t inlineLocalVectorSizeOffset = jit->stackFrameRecordOffset + offsetof(sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t, inlineLocalVectorSize);
        sysbvm_jit_x86_movS32IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, (int32_t)inlineLocalVectorSizeOffset, (int32_t)jit->localVectorSize);

        jit->localVectorOffset = jit->stackFrameRecordOffset + offsetof(sysbvm_stackFrameBytecodeFunctionJitActivationRecord_t, inlineLocalVector);

        // Initialize the locals
        if(jit->localVectorSize > 0)
        {
            sysbvm_jit_x86_xorRegister(jit, SYSBVM_X86_RAX, SYSBVM_X86_RAX);
            for(size_t i = 0; i < jit->localVectorSize; ++i)
            {
                size_t localOffset = jit->localVectorOffset + i*sizeof(void*);
                sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, (int32_t)localOffset, SYSBVM_X86_RAX);
            }
        }

        // Connect with the stack unwinder.
        sysbvm_jit_x86_leaRegisterWithOffset(jit, SYSBVM_X86_64_ARG0, SYSBVM_X86_RBP, jit->stackFrameRecordOffset);
        sysbvm_jit_x86_call(jit, &sysbvm_stackFrame_pushRecord);
    }
}

static void sysbvm_jit_epilogue(sysbvm_bytecodeJit_t *jit)
{
#ifdef _WIN32
    sysbvm_jit_x86_leaRegisterWithOffset(jit, SYSBVM_X86_RSP, SYSBVM_X86_RBP, jit->stackFrameSize);
#else
    sysbvm_jit_x86_mov64Register(jit, SYSBVM_X86_RSP, SYSBVM_X86_RBP);
#endif
    sysbvm_jit_x86_popRegister(jit, SYSBVM_X86_RBP);
    sysbvm_jit_x86_ret(jit);
}

static void sysbvm_jit_moveRegisterToOperand(sysbvm_bytecodeJit_t *jit, int16_t operand, sysbvm_x86_register_t reg)
{
    sysbvm_operandVectorName_t vectorType = (sysbvm_operandVectorName_t) (operand & SYSBVM_OPERAND_VECTOR_BITMASK);
    int16_t vectorIndex = operand >> SYSBVM_OPERAND_VECTOR_BITS;
    if(vectorIndex < 0)
        return;

    int32_t vectorOffset = vectorIndex * sizeof(void*);
    switch(vectorType)
    {
    case SYSBVM_OPERAND_VECTOR_LOCAL:
        sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, jit->localVectorOffset + vectorOffset, reg);
        break;
    default:
        abort();
        break;
    }
}

static void sysbvm_jit_moveOperandToRegister(sysbvm_bytecodeJit_t *jit, sysbvm_x86_register_t reg, int16_t operand)
{
    sysbvm_operandVectorName_t vectorType = (sysbvm_operandVectorName_t) (operand & SYSBVM_OPERAND_VECTOR_BITMASK);
    int16_t vectorIndex = operand >> SYSBVM_OPERAND_VECTOR_BITS;
    if(vectorIndex < 0)
    {
        sysbvm_jit_x86_xorRegister(jit, reg, reg);
        return;
    }

    int32_t vectorOffset = (int32_t)vectorIndex * sizeof(void*);
    switch(vectorType)
    {
    case SYSBVM_OPERAND_VECTOR_ARGUMENTS:
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, SYSBVM_X86_RBP, jit->argumentVectorOffset);
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, reg, vectorOffset);
        break;
    case SYSBVM_OPERAND_VECTOR_CAPTURES:
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, SYSBVM_X86_RBP, jit->captureVectorOffset);
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, reg, sizeof(sysbvm_tuple_header_t) + vectorOffset);
        break;
    case SYSBVM_OPERAND_VECTOR_LITERAL:
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, SYSBVM_X86_RBP, jit->literalVectorOffset);
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, reg, sizeof(sysbvm_tuple_header_t) + vectorOffset);
        break;
    case SYSBVM_OPERAND_VECTOR_LOCAL:
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, reg, SYSBVM_X86_RBP, jit->localVectorOffset + vectorOffset);
        break;
    }
}

static void sysbvm_jit_moveOperandToCallArgumentVector(sysbvm_bytecodeJit_t *jit, int16_t operand, int32_t callArgumentVectorIndex)
{
    SYSBVM_ASSERT(callArgumentVectorIndex < SYSBVM_BYTECODE_FUNCTION_MAX_CALL_ARGUMENTS);
    int32_t callArgumentVectorOffset = jit->callArgumentVectorOffset + callArgumentVectorIndex * sizeof(void*);
    sysbvm_operandVectorName_t vectorType = (sysbvm_operandVectorName_t) (operand & SYSBVM_OPERAND_VECTOR_BITMASK);
    int16_t vectorIndex = operand >> SYSBVM_OPERAND_VECTOR_BITS;
    if(vectorIndex < 0)
    {
        sysbvm_jit_x86_movS32IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, callArgumentVectorOffset, 0);
        return;
    }

    int32_t vectorOffset = vectorIndex * sizeof(void*);
    sysbvm_x86_register_t scratchRegister = SYSBVM_X86_RAX;
    switch(vectorType)
    {
    case SYSBVM_OPERAND_VECTOR_ARGUMENTS:
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, scratchRegister, SYSBVM_X86_RBP, jit->argumentVectorOffset);
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, scratchRegister, scratchRegister, vectorOffset);
        break;
    case SYSBVM_OPERAND_VECTOR_CAPTURES:
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, scratchRegister, SYSBVM_X86_RBP, jit->captureVectorOffset);
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, scratchRegister, scratchRegister, sizeof(sysbvm_tuple_header_t) + vectorOffset);
        break;
    case SYSBVM_OPERAND_VECTOR_LITERAL:
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, scratchRegister, SYSBVM_X86_RBP, jit->literalVectorOffset);
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, scratchRegister, scratchRegister, sizeof(sysbvm_tuple_header_t) + vectorOffset);
        break;
    case SYSBVM_OPERAND_VECTOR_LOCAL:
        sysbvm_jit_x86_mov64FromMemoryWithOffset(jit, scratchRegister, SYSBVM_X86_RBP, jit->localVectorOffset + vectorOffset);
        break;
    }

    sysbvm_jit_x86_mov64IntoMemoryWithOffset(jit, SYSBVM_X86_RBP, callArgumentVectorOffset, scratchRegister);
}

SYSBVM_API void sysbvm_jit_moveOperandToOperand(sysbvm_bytecodeJit_t *jit, int16_t destinationOperand, int16_t sourceOperand)
{
    if(destinationOperand < 0)
        return;

    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_RAX, sourceOperand);
    sysbvm_jit_moveRegisterToOperand(jit, destinationOperand, SYSBVM_X86_RAX);
}

SYSBVM_API void sysbvm_jit_return(sysbvm_bytecodeJit_t *jit, int16_t operand)
{
    // Disconnect from the stack unwinder.
    sysbvm_jit_x86_leaRegisterWithOffset(jit, SYSBVM_X86_64_ARG0, SYSBVM_X86_RBP, jit->stackFrameRecordOffset);
    sysbvm_jit_x86_call(jit, &sysbvm_stackFrame_popRecord);

    sysbvm_jit_moveOperandToRegister(jit, SYSBVM_X86_RAX, operand);
    sysbvm_jit_epilogue(jit);
}

SYSBVM_API void sysbvm_jit_storePC(sysbvm_bytecodeJit_t *jit, uint16_t pc)
{
    sysbvm_jit_x86_movImmediateI32IntoMemory64WithOffset(jit, SYSBVM_X86_RBP, jit->pcOffset, pc);
}

static void sysbvm_jit_emitUnwindInfo(sysbvm_bytecodeJit_t *jit)
{
#ifdef _WIN32
    RUNTIME_FUNCTION runtimeFunction = {0};
    runtimeFunction.BeginAddress = 0;
    runtimeFunction.EndAddress = (DWORD)jit->instructions.size;
    sysbvm_bytecodeJit_addUnwindInfoBytes(jit, sizeof(runtimeFunction), (uint8_t*)&runtimeFunction);

    // Unwind_info
    size_t codeCount = jit->unwindInfoBytecode.size/2;
    int frameRegister = /* RBP */ 5;
    int frameOffset = jit->cfiFrameOffset;

    sysbvm_bytecodeJit_addUnwindInfoByte(jit, /*Version*/1  | (/* Flags*/0 << 3));
    sysbvm_bytecodeJit_addUnwindInfoByte(jit, (uint8_t)jit->prologueSize);
    sysbvm_bytecodeJit_addUnwindInfoByte(jit, (uint8_t)codeCount);
    sysbvm_bytecodeJit_addUnwindInfoByte(jit, (uint8_t) ((frameRegister) | (frameOffset << 4)));

    // Unwind codes must be sorted in descending order.
    uint16_t *unwindCodes = (uint16_t *)jit->unwindInfoBytecode.data;
    for(size_t i = 0; i < codeCount; ++i)
        sysbvm_dynarray_addAll(&jit->unwindInfo, 2, unwindCodes + codeCount - i - 1);

    if((codeCount % 2) != 0)
    {
        sysbvm_bytecodeJit_addUnwindInfoByte(jit, 0);
        sysbvm_bytecodeJit_addUnwindInfoByte(jit, 0);
    }
#endif
    sysbvm_dwarf_cfi_endFDE(&jit->dwarfEhBuilder, jit->instructions.size);
    sysbvm_dwarf_cfi_finish(&jit->dwarfEhBuilder);
}

typedef struct sysbvm_jit_x64_elfSectionHeaders_s
{
    sysbvm_elf64_sectionHeader_t null;
    sysbvm_elf64_sectionHeader_t text;
    sysbvm_elf64_sectionHeader_t eh_frame;
    sysbvm_elf64_sectionHeader_t debug_line;
    sysbvm_elf64_sectionHeader_t debug_str;
    sysbvm_elf64_sectionHeader_t debug_abbrev;
    sysbvm_elf64_sectionHeader_t debug_info;
    sysbvm_elf64_sectionHeader_t symtab;
    sysbvm_elf64_sectionHeader_t str;
    sysbvm_elf64_sectionHeader_t shstr;
} sysbvm_jit_x64_elfSectionHeaders_t;

typedef struct sysbvm_jit_x64_elfSymbolTable_s
{
    sysbvm_elf64_symbol_t null;
    sysbvm_elf64_symbol_t sourceFile;
    sysbvm_elf64_symbol_t text;
    sysbvm_elf64_symbol_t jittedFunction;
} sysbvm_jit_x64_elfSymbolTable_t;

typedef struct sysbvm_jit_x64_elfContentFooter_s
{
    sysbvm_jit_x64_elfSymbolTable_t symbols;
    sysbvm_jit_x64_elfSectionHeaders_t sections;
} sysbvm_jit_x64_elfContentFooter_t;

static size_t sysbvm_jit_emitObjectFileCString(sysbvm_bytecodeJit_t *jit, const char *cstring)
{
    size_t result = jit->objectFileContent.size;
    sysbvm_dynarray_addAll(&jit->objectFileContent, strlen(cstring) + 1, cstring);
    return result;
}

static size_t sysbvm_jit_emitObjectFileSourceFileName(sysbvm_bytecodeJit_t *jit)
{
    if(!sysbvm_tuple_isNonNullPointer(jit->sourcePosition))
        return 0;

    sysbvm_sourcePosition_t *sourcePosition = (sysbvm_sourcePosition_t*)jit->sourcePosition;
    if(!sysbvm_tuple_isNonNullPointer(sourcePosition->sourceCode))
        return 0;

    sysbvm_sourceCode_t *sourceCode = (sysbvm_sourceCode_t*)sourcePosition->sourceCode;
    
    size_t nameOffset = jit->objectFileContent.size;
    bool hasEmittedName = false;

    if(sourceCode->directory)
    {
        size_t byteSize = sysbvm_tuple_getSizeInBytes(sourceCode->directory);
        if(byteSize > 0)
        {
            sysbvm_dynarray_addAll(&jit->objectFileContent, byteSize, SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(sourceCode->directory)->bytes);
            hasEmittedName = true;

            char slash = '/';
            sysbvm_dynarray_add(&jit->objectFileContent, &slash);
        }
    }

    if(sourceCode->name)
    {
        size_t byteSize = sysbvm_tuple_getSizeInBytes(sourceCode->name);
        if(byteSize > 0)
        {
            sysbvm_dynarray_addAll(&jit->objectFileContent, byteSize, SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(sourceCode->name)->bytes);
            hasEmittedName = true;
        }
    }

    if(!hasEmittedName)
        return 0;

    char nullTerminator = 0;
    sysbvm_dynarray_add(&jit->objectFileContent, &nullTerminator);
    return nameOffset;
}

static bool sysbvm_jit_emitObjectFileProgramEntityRecursiveName(sysbvm_bytecodeJit_t *jit, sysbvm_programEntity_t *programEntity)
{
    if(!programEntity)
        return false;

    bool hasEmittedName = sysbvm_jit_emitObjectFileProgramEntityRecursiveName(jit, (sysbvm_programEntity_t*)programEntity->owner);
    if(sysbvm_tuple_isBytes(programEntity->name))
    {
        if(hasEmittedName)
        {
            char dot = '.';
            sysbvm_dynarray_add(&jit->objectFileContent, &dot);
        }
        size_t byteSize = sysbvm_tuple_getSizeInBytes(programEntity->name);
        sysbvm_dynarray_addAll(&jit->objectFileContent, byteSize, SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(programEntity->name)->bytes);
        hasEmittedName = hasEmittedName || byteSize > 0;
    }

    return hasEmittedName;
}

static size_t sysbvm_jit_emitObjectFileJittedFunctionName(sysbvm_bytecodeJit_t *jit)
{
    size_t nameOffset = jit->objectFileContent.size;
    jit->objectFileContentJittedFunctionNameOffset = nameOffset;

    // Emit the program entity name;
    bool hasEmittedName = sysbvm_jit_emitObjectFileProgramEntityRecursiveName(jit, (sysbvm_programEntity_t*)jit->compiledProgramEntity);

    // Function source location and pointer number.
    if(!hasEmittedName)
    {
        char pointerBuffer[32];
        uint32_t sourceLine, sourceColumn;
        sysbvm_sourcePosition_getStartLineAndColumn(jit->context, jit->sourcePosition, &sourceLine, &sourceColumn);

        snprintf(pointerBuffer, sizeof(pointerBuffer), "%d:%d-%08llx", sourceLine, sourceColumn, (unsigned long long)sysbvm_tuple_identityHash(jit->compiledProgramEntity));
        sysbvm_dynarray_addAll(&jit->objectFileContent, strlen(pointerBuffer), pointerBuffer);
    }

    char nullTerminator = 0;
    sysbvm_dynarray_add(&jit->objectFileContent, &nullTerminator);

    return nameOffset;
}

static void sysbvm_jit_emitObjectFile(sysbvm_bytecodeJit_t *jit)
{
    sysbvm_elf64_header_t header = {0};
    sysbvm_jit_x64_elfContentFooter_t footer = {0};

    size_t stringTableOffset = jit->objectFileContent.size;
    footer.sections.null.name = (sysbvm_elf64_word_t)sysbvm_jit_emitObjectFileCString(jit, ""); // Null string
    footer.sections.text.name = (sysbvm_elf64_word_t)sysbvm_jit_emitObjectFileCString(jit, ".text");
    footer.sections.eh_frame.name = (sysbvm_elf64_word_t)sysbvm_jit_emitObjectFileCString(jit, ".eh_frame");
    footer.sections.debug_line.name = (sysbvm_elf64_word_t)sysbvm_jit_emitObjectFileCString(jit, ".debug_line");
    footer.sections.debug_str.name = (sysbvm_elf64_word_t)sysbvm_jit_emitObjectFileCString(jit, ".debug_str");
    footer.sections.debug_abbrev.name = (sysbvm_elf64_word_t)sysbvm_jit_emitObjectFileCString(jit, ".debug_abbrev");
    footer.sections.debug_info.name = (sysbvm_elf64_word_t)sysbvm_jit_emitObjectFileCString(jit, ".debug_info");
    footer.sections.symtab.name = (sysbvm_elf64_word_t)sysbvm_jit_emitObjectFileCString(jit, ".symtab");
    footer.sections.str.name = (sysbvm_elf64_word_t)sysbvm_jit_emitObjectFileCString(jit, ".str");
    footer.sections.shstr.name = (sysbvm_elf64_word_t)sysbvm_jit_emitObjectFileCString(jit, ".shstr");

    footer.symbols.sourceFile.name = (sysbvm_elf64_word_t)sysbvm_jit_emitObjectFileSourceFileName(jit);
    footer.symbols.sourceFile.info = SYSBVM_ELF64_SYM_INFO(SYSBVM_STT_FILE, SYSBVM_STB_LOCAL);

    footer.symbols.text.sectionHeaderIndex = 1;
    footer.symbols.text.info = SYSBVM_ELF64_SYM_INFO(SYSBVM_STT_SECTION, SYSBVM_STB_LOCAL);

    footer.symbols.jittedFunction.name = (sysbvm_elf64_word_t)sysbvm_jit_emitObjectFileJittedFunctionName(jit);
    footer.symbols.jittedFunction.info = SYSBVM_ELF64_SYM_INFO(SYSBVM_STT_FUNC, SYSBVM_STB_LOCAL);
    footer.symbols.jittedFunction.sectionHeaderIndex = 1;
    footer.symbols.jittedFunction.value = 0;
    footer.symbols.jittedFunction.size = jit->instructions.size;

    header.ident[SYSBVM_EI_MAG0] = 0x7f;
    header.ident[SYSBVM_EI_MAG1] = 'E';
    header.ident[SYSBVM_EI_MAG2] = 'L';
    header.ident[SYSBVM_EI_MAG3] = 'F';
    header.ident[SYSBVM_EI_CLASS] = SYSBVM_ELFCLASS64;
    header.ident[SYSBVM_EI_DATA] = SYSBVM_ELFDATA2LSB;
    header.ident[SYSBVM_EI_VERSION] = SYSBVM_ELFCURRENT_VERSION;
    header.type = SYSBVM_ET_REL;
    header.machine = SYSBVM_EM_X86_64;
    header.elfHeaderSize = sizeof(header);
    header.version = SYSBVM_ELFCURRENT_VERSION;
    header.sectionHeaderEntrySize = sizeof(footer.sections.null);
    header.sectionHeaderNum = sizeof(footer.sections) / sizeof(footer.sections.null);
    header.sectionHeaderNameStringTableIndex = offsetof(sysbvm_jit_x64_elfSectionHeaders_t, shstr) / sizeof(sysbvm_elf64_sectionHeader_t);

    size_t stringTableEnd = jit->objectFileContent.size;
    size_t stringTableSize = stringTableEnd - stringTableOffset;

    footer.sections.text.type = SYSBVM_SHT_PROGBITS;
    footer.sections.text.flags = SYSBVM_SHF_ALLOC | SYSBVM_SHF_EXECINSTR;
    footer.sections.text.addressAlignment = 1;

    footer.sections.eh_frame.type = sizeof(uintptr_t) == 8 ? SHT_X86_64_UNWIND : SYSBVM_SHT_PROGBITS;
    footer.sections.eh_frame.flags = SYSBVM_SHF_ALLOC;
    footer.sections.eh_frame.addressAlignment = sizeof(uintptr_t);

    footer.sections.debug_line.type = SYSBVM_SHT_PROGBITS;
    footer.sections.debug_line.addressAlignment = sizeof(uintptr_t);

    footer.sections.debug_str.type = SYSBVM_SHT_PROGBITS;
    footer.sections.debug_str.addressAlignment = sizeof(uintptr_t);

    footer.sections.debug_abbrev.type = SYSBVM_SHT_PROGBITS;
    footer.sections.debug_abbrev.addressAlignment = sizeof(uintptr_t);

    footer.sections.debug_info.type = SYSBVM_SHT_PROGBITS;
    footer.sections.debug_info.addressAlignment = sizeof(uintptr_t);

    footer.sections.str.type = SYSBVM_SHT_STRTAB;
    footer.sections.str.offset = stringTableOffset;
    footer.sections.str.addressAlignment = 1;
    footer.sections.str.size = stringTableSize;

    footer.sections.shstr.type = SYSBVM_SHT_STRTAB;
    footer.sections.shstr.offset = stringTableOffset;
    footer.sections.shstr.addressAlignment = 1;
    footer.sections.shstr.size = stringTableSize;

    size_t symbolTableOffset = jit->objectFileContent.size;
    footer.sections.symtab.type = SYSBVM_SHT_SYMTAB;
    footer.sections.symtab.offset = symbolTableOffset;
    footer.sections.symtab.entrySize = sizeof(sysbvm_elf64_symbol_t);
    footer.sections.symtab.addressAlignment = 1;
    footer.sections.symtab.link = offsetof(sysbvm_jit_x64_elfSectionHeaders_t, str) / sizeof(sysbvm_elf64_sectionHeader_t);
    footer.sections.symtab.info = sizeof(sysbvm_jit_x64_elfSymbolTable_t) / sizeof(sysbvm_elf64_symbol_t);
    footer.sections.symtab.size = sizeof(sysbvm_jit_x64_elfSymbolTable_t);

    sysbvm_dynarray_addAll(&jit->objectFileHeader, sizeof(header), &header);
    sysbvm_dynarray_addAll(&jit->objectFileContent, sizeof(footer), &footer);
}

static void sysbvm_jit_fixupObjectFile(sysbvm_bytecodeJit_t *jit,
    sysbvm_elf64_header_t *header, sysbvm_elf64_header_t *headerExecutablePointer,
    uint8_t *instructionsExecutablePointer,
    uint8_t *ehFramePointer, uint8_t *ehFrameExecutablePointer,
    uint8_t *debugLineExecutablePointer,
    uint8_t *debugStrExecutablePointer,
    uint8_t *debugAbbrevExecutablePointer,
    uint8_t *debugInfoExecutablePointer,
    uint8_t *objectFileContentExecutablePointer,
    sysbvm_jit_x64_elfContentFooter_t *footer)
{
    if(jit->dwarfEhBuilder.fdeInitialLocationOffset > 0)
    {
        int32_t *initialLocationPointer = (int32_t*)(ehFramePointer + jit->dwarfEhBuilder.fdeInitialLocationOffset);
        int32_t *initialLocationExecutablePointer = (int32_t*)(ehFrameExecutablePointer + jit->dwarfEhBuilder.fdeInitialLocationOffset);
        *initialLocationPointer = (int32_t) ((uintptr_t)instructionsExecutablePointer - (uintptr_t)initialLocationExecutablePointer);
    }

    header->sectionHeadersOffset = (uintptr_t)&footer->sections - (uintptr_t)header;
    sysbvm_elf64_off_t contentOffset = (uintptr_t)objectFileContentExecutablePointer - (uintptr_t)headerExecutablePointer;

    footer->sections.text.offset = (uintptr_t)instructionsExecutablePointer - (uintptr_t)headerExecutablePointer;
    footer->sections.text.address = (sysbvm_elf64_addr_t)instructionsExecutablePointer;
    footer->sections.text.size = jit->instructions.size;

    footer->sections.eh_frame.offset = (uintptr_t)ehFrameExecutablePointer - (uintptr_t)headerExecutablePointer;
    footer->sections.eh_frame.address = (sysbvm_elf64_addr_t)ehFrameExecutablePointer;
    footer->sections.eh_frame.size = jit->dwarfEhBuilder.buffer.size;

    footer->sections.debug_line.offset = (uintptr_t)debugLineExecutablePointer - (uintptr_t)headerExecutablePointer;
    footer->sections.debug_line.size = jit->dwarfDebugInfoBuilder.line.size;

    footer->sections.debug_str.offset = (uintptr_t)debugStrExecutablePointer - (uintptr_t)headerExecutablePointer;
    footer->sections.debug_str.size = jit->dwarfDebugInfoBuilder.str.size;

    footer->sections.debug_abbrev.offset = (uintptr_t)debugAbbrevExecutablePointer - (uintptr_t)headerExecutablePointer;
    footer->sections.debug_abbrev.size = jit->dwarfDebugInfoBuilder.abbrev.size;

    footer->sections.debug_info.offset = (uintptr_t)debugInfoExecutablePointer - (uintptr_t)headerExecutablePointer;
    footer->sections.debug_info.size = jit->dwarfDebugInfoBuilder.info.size;

    footer->sections.symtab.offset += contentOffset;
    footer->sections.str.offset += contentOffset;
    footer->sections.shstr.offset += contentOffset;
}

static void sysbvm_jit_emitPerfSymbolFor(sysbvm_bytecodeJit_t *jit, uint8_t *instructionsPointers)
{
#if defined(__linux__) && defined(SYSBVM_EMIT_PERF_STACK_MAP)
    static int sysbvm_jit_perfMapFD;

    char buffer[2048];

    if(!sysbvm_jit_perfMapFD)
    {
        snprintf(buffer, sizeof(buffer), "/tmp/perf-%d.map", getpid());
        sysbvm_jit_perfMapFD = open(buffer, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }

    if(sysbvm_jit_perfMapFD < 0) return;

    int symbolRecordSize = snprintf(buffer, sizeof(buffer), "%llx %llx %s\n", (long long)instructionsPointers, (long long)jit->instructions.size, (const char *)jit->objectFileContent.data + jit->objectFileContentJittedFunctionNameOffset);
    ssize_t writeSize = write(sysbvm_jit_perfMapFD, buffer, symbolRecordSize);
    if(writeSize != symbolRecordSize)
        perror("Failed write map file entry");
#else
    (void)jit;
    (void)instructionsPointers;
#endif
}

static void sysbvm_jit_emitArgumentDebugInfo(sysbvm_bytecodeJit_t *jit, size_t oopTypeDie, size_t index, sysbvm_tuple_t binding)
{
    char nameBuffer[32];
    snprintf(nameBuffer, sizeof(nameBuffer), "arg%d", (int)index);

    sysbvm_dwarf_debugInfo_beginDIE(&jit->dwarfDebugInfoBuilder, DW_TAG_formal_parameter, false);
    sysbvm_dwarf_debugInfo_attribute_stringTupleWithDefaultString(&jit->dwarfDebugInfoBuilder, DW_AT_name, sysbvm_symbolBinding_getName(binding), nameBuffer);
    sysbvm_dwarf_debugInfo_attribute_ref1(&jit->dwarfDebugInfoBuilder, DW_AT_type, (uint8_t)oopTypeDie);

    sysbvm_dwarf_debugInfo_attribute_beginLocationExpression(&jit->dwarfDebugInfoBuilder, DW_AT_location);

    sysbvm_dwarf_debugInfo_location_frameBaseOffset(&jit->dwarfDebugInfoBuilder, jit->argumentVectorOffset);
    sysbvm_dwarf_debugInfo_location_deref(&jit->dwarfDebugInfoBuilder);

    if(index > 0)
    {
        sysbvm_dwarf_debugInfo_location_constUnsigned(&jit->dwarfDebugInfoBuilder, index * sizeof(sysbvm_tuple_t));
        sysbvm_dwarf_debugInfo_location_plus(&jit->dwarfDebugInfoBuilder);
    }
    sysbvm_dwarf_debugInfo_attribute_endLocationExpression(&jit->dwarfDebugInfoBuilder);

    sysbvm_dwarf_debugInfo_endDIE(&jit->dwarfDebugInfoBuilder);
}

static void sysbvm_jit_emitCaptureDebugInfo(sysbvm_bytecodeJit_t *jit, size_t oopTypeDie, size_t index, sysbvm_tuple_t binding)
{
    char nameBuffer[32];
    snprintf(nameBuffer, sizeof(nameBuffer), "capture%d", (int)index);

    sysbvm_dwarf_debugInfo_beginDIE(&jit->dwarfDebugInfoBuilder, DW_TAG_variable, false);
    sysbvm_dwarf_debugInfo_attribute_stringTupleWithDefaultString(&jit->dwarfDebugInfoBuilder, DW_AT_name, sysbvm_symbolBinding_getName(binding), nameBuffer);
    sysbvm_dwarf_debugInfo_attribute_ref1(&jit->dwarfDebugInfoBuilder, DW_AT_type, (uint8_t)oopTypeDie);

    sysbvm_dwarf_debugInfo_attribute_beginLocationExpression(&jit->dwarfDebugInfoBuilder, DW_AT_location);

    sysbvm_dwarf_debugInfo_location_frameBaseOffset(&jit->dwarfDebugInfoBuilder, jit->captureVectorOffset);
    sysbvm_dwarf_debugInfo_location_deref(&jit->dwarfDebugInfoBuilder);

    sysbvm_dwarf_debugInfo_location_constUnsigned(&jit->dwarfDebugInfoBuilder, sizeof(sysbvm_tuple_header_t) + index * sizeof(sysbvm_tuple_t));
    sysbvm_dwarf_debugInfo_location_plus(&jit->dwarfDebugInfoBuilder);
    sysbvm_dwarf_debugInfo_attribute_endLocationExpression(&jit->dwarfDebugInfoBuilder);

    sysbvm_dwarf_debugInfo_endDIE(&jit->dwarfDebugInfoBuilder);
}

static void sysbvm_jit_emitDebugInfo(sysbvm_bytecodeJit_t *jit)
{
    bool hasLineInfo = sysbvm_jit_emitDebugLineInfo(jit);

    sysbvm_dwarf_debugInfo_beginDIE(&jit->dwarfDebugInfoBuilder, DW_TAG_compile_unit, true);
    sysbvm_dwarf_debugInfo_attribute_string(&jit->dwarfDebugInfoBuilder, DW_AT_producer, "Sysbvmi"); // Use the line info.
    if(hasLineInfo)
        sysbvm_dwarf_debugInfo_attribute_secOffset(&jit->dwarfDebugInfoBuilder, DW_AT_stmt_list, 0);
    sysbvm_dwarf_debugInfo_attribute_textAddress(&jit->dwarfDebugInfoBuilder, DW_AT_low_pc, 0);
    sysbvm_dwarf_debugInfo_attribute_textAddress(&jit->dwarfDebugInfoBuilder, DW_AT_high_pc, jit->instructions.size);
    sysbvm_dwarf_debugInfo_endDIE(&jit->dwarfDebugInfoBuilder);

    size_t oopTypeDie = sysbvm_dwarf_debugInfo_beginDIE(&jit->dwarfDebugInfoBuilder, DW_TAG_base_type, false);
    {
        sysbvm_dwarf_debugInfo_attribute_string(&jit->dwarfDebugInfoBuilder, DW_AT_name, "Oop");
        sysbvm_dwarf_debugInfo_attribute_uleb128(&jit->dwarfDebugInfoBuilder, DW_AT_encoding, DW_ATE_signed);
        sysbvm_dwarf_debugInfo_attribute_uleb128(&jit->dwarfDebugInfoBuilder, DW_AT_byte_size, sizeof(sysbvm_tuple_t));
    }
    sysbvm_dwarf_debugInfo_endDIE(&jit->dwarfDebugInfoBuilder);

    {
        sysbvm_functionBytecode_t* bytecode = (sysbvm_functionBytecode_t*)((sysbvm_functionDefinition_t*)jit->compiledProgramEntity)->bytecode;
        size_t argumentCount = sysbvm_array_getSize(bytecode->arguments);
        size_t captureCount = sysbvm_array_getSize(bytecode->captures);
        bool hasVariables = argumentCount != 0 || captureCount != 0;

        sysbvm_dwarf_debugInfo_beginDIE(&jit->dwarfDebugInfoBuilder, DW_TAG_subprogram, hasVariables);
        if(hasLineInfo && jit->sourcePosition)
        {
            sysbvm_sourcePosition_t *sourcePositionObject = (sysbvm_sourcePosition_t*)jit->sourcePosition;
            uint32_t line = 0;
            if(sysbvm_sourcePosition_getStartLineAndColumn(jit->context, jit->sourcePosition, &line, NULL))
            {
                sysbvm_dwarf_debugInfo_attribute_uleb128(&jit->dwarfDebugInfoBuilder, DW_AT_decl_file, sysbvm_jit_dwarfLineInfoEmissionState_indexOfFile(&jit->dwarfLineEmissionState, sourcePositionObject->sourceCode));
                sysbvm_dwarf_debugInfo_attribute_uleb128(&jit->dwarfDebugInfoBuilder, DW_AT_decl_line, line);
            }

        }
        sysbvm_dwarf_debugInfo_attribute_textAddress(&jit->dwarfDebugInfoBuilder, DW_AT_low_pc, 0);
        sysbvm_dwarf_debugInfo_attribute_textAddress(&jit->dwarfDebugInfoBuilder, DW_AT_high_pc, jit->instructions.size);
        sysbvm_dwarf_debugInfo_attribute_textAddress(&jit->dwarfDebugInfoBuilder, DW_AT_type, oopTypeDie);
        sysbvm_dwarf_debugInfo_attribute_beginLocationExpression(&jit->dwarfDebugInfoBuilder, DW_AT_frame_base);
        sysbvm_dwarf_debugInfo_location_register(&jit->dwarfDebugInfoBuilder, sizeof(uintptr_t) == 8 ? DW_X64_REG_RBP : DW_X86_REG_EBP);
        sysbvm_dwarf_debugInfo_attribute_endLocationExpression(&jit->dwarfDebugInfoBuilder);
        sysbvm_dwarf_debugInfo_endDIE(&jit->dwarfDebugInfoBuilder);

        // Emit the arguments.
        for(size_t i = 0; i < argumentCount; ++i)
            sysbvm_jit_emitArgumentDebugInfo(jit, oopTypeDie, i, sysbvm_array_at(bytecode->arguments, i));

        // Emit the captures.
        for(size_t i = 0; i < captureCount; ++i)
            sysbvm_jit_emitCaptureDebugInfo(jit, oopTypeDie, i, sysbvm_array_at(bytecode->captures, i));

        if(hasVariables)
            sysbvm_dwarf_debugInfo_endDIEChildren(&jit->dwarfDebugInfoBuilder);
    }

    sysbvm_dwarf_debugInfo_endDIEChildren(&jit->dwarfDebugInfoBuilder);

    sysbvm_dwarf_debugInfo_finish(&jit->dwarfDebugInfoBuilder);
}

SYSBVM_API void sysbvm_jit_finish(sysbvm_bytecodeJit_t *jit)
{
    // Apply the PC target relative relocations.
    for(size_t i = 0; i < jit->pcRelocations.size; ++i)
    {
        sysbvm_bytecodeJitPCRelocation_t *relocation = sysbvm_dynarray_entryOfTypeAt(jit->pcRelocations, sysbvm_bytecodeJitPCRelocation_t, i);
        *((int32_t*)(jit->instructions.data + relocation->offset)) = (int32_t)(jit->pcDestinations[relocation->targetPC] - (intptr_t)relocation->offset + relocation->addend);
    }

    sysbvm_jit_emitUnwindInfo(jit);
    sysbvm_jit_emitDebugInfo(jit);
    sysbvm_jit_emitObjectFile(jit);
}

SYSBVM_API uint8_t *sysbvm_jit_installIn(sysbvm_bytecodeJit_t *jit, uint8_t *codeWriteablePointer, uint8_t *codeExecutablePointer)
{
    size_t objectFileHeaderOffset = 0;
    size_t codeOffset = sysbvm_sizeAlignedTo(jit->objectFileHeader.size, 16);
    size_t constantsOffset = codeOffset + sysbvm_sizeAlignedTo(jit->instructions.size, 16);

    size_t unwindInfoOffset = constantsOffset + sysbvm_sizeAlignedTo(jit->constants.size, 16);
    size_t ehFrameOffset = unwindInfoOffset + sysbvm_sizeAlignedTo(jit->unwindInfo.size, 16);
    size_t debugLineOffset = ehFrameOffset + sysbvm_sizeAlignedTo(jit->dwarfEhBuilder.buffer.size, 16);
    size_t debugStrOffset = debugLineOffset + sysbvm_sizeAlignedTo(jit->dwarfDebugInfoBuilder.line.size, 16);
    size_t debugAbbrevOffset = debugStrOffset + sysbvm_sizeAlignedTo(jit->dwarfDebugInfoBuilder.str.size, 16);
    size_t debugInfoOffset = debugAbbrevOffset + sysbvm_sizeAlignedTo(jit->dwarfDebugInfoBuilder.abbrev.size, 16);

    size_t objectFileContentOffset = debugInfoOffset + sysbvm_sizeAlignedTo(jit->dwarfDebugInfoBuilder.info.size, 16);

    uint8_t *objectFileHeaderPointer = codeWriteablePointer + objectFileHeaderOffset;
    uint8_t *objectFileHeaderExecutablePointer = codeExecutablePointer + objectFileHeaderOffset;
    memcpy(objectFileHeaderPointer, jit->objectFileHeader.data, jit->objectFileHeader.size);

    uint8_t *instructionsPointers = codeWriteablePointer + codeOffset;
    uint8_t *instructionsExecutablePointers = codeExecutablePointer + codeOffset;
    memcpy(instructionsPointers, jit->instructions.data, jit->instructions.size);

    uint8_t *constantZonePointer = codeWriteablePointer + constantsOffset;
    uint8_t *constantZoneExecutablePointer = codeExecutablePointer + constantsOffset;
    memcpy(constantZonePointer, jit->constants.data, jit->constants.size);

    for(size_t i = 0; i < jit->relocations.size; ++i)
    {
        sysbvm_bytecodeJitRelocation_t *relocation = sysbvm_dynarray_entryOfTypeAt(jit->relocations, sysbvm_bytecodeJitRelocation_t, i);
        uint8_t *relocationTarget = instructionsPointers + relocation->offset;
        uint8_t *relocationExecutableTarget = instructionsExecutablePointers + relocation->offset;
        intptr_t relocationTargetAddress = (intptr_t)relocationExecutableTarget;

        intptr_t relativeValue = (intptr_t)constantZoneExecutablePointer + relocation->value - relocationTargetAddress + relocation->addend;
        switch(relocation->type)
        {
        case SYSBVM_BYTECODE_JIT_RELOCATION_RELATIVE32:
            *((int32_t*)relocationTarget) = (int32_t)relativeValue;
            break;
        case SYSBVM_BYTECODE_JIT_RELOCATION_RELATIVE64:
            *((int64_t*)relocationTarget) = (int64_t)relativeValue;
            break;
        }
    }

    uint8_t *unwindInfoZonePointer = codeWriteablePointer + unwindInfoOffset;
    uint8_t *unwindInfoZoneExecutablePointer = codeExecutablePointer + unwindInfoOffset;
    memcpy(unwindInfoZonePointer, jit->unwindInfo.data, jit->unwindInfo.size);

    uint8_t *ehFrameZonePointer = codeWriteablePointer + ehFrameOffset;
    uint8_t *ehFrameZoneExecutablePointer = codeExecutablePointer + ehFrameOffset;
    memcpy(ehFrameZonePointer, jit->dwarfEhBuilder.buffer.data, jit->dwarfEhBuilder.buffer.size);

    sysbvm_dwarf_debugInfo_patchTextAddressesRelativeTo(&jit->dwarfDebugInfoBuilder, (uintptr_t)instructionsExecutablePointers);

    uint8_t *debugLineZonePointer = codeWriteablePointer + debugLineOffset;
    uint8_t *debugLineZoneExecutablePointer = codeExecutablePointer + debugLineOffset;
    memcpy(debugLineZonePointer, jit->dwarfDebugInfoBuilder.line.data, jit->dwarfDebugInfoBuilder.line.size);

    uint8_t *debugStrZonePointer = codeWriteablePointer + debugStrOffset;
    uint8_t *debugStrZoneExecutablePointer = codeExecutablePointer + debugStrOffset;
    memcpy(debugStrZonePointer, jit->dwarfDebugInfoBuilder.str.data, jit->dwarfDebugInfoBuilder.str.size);

    uint8_t *debugAbbrevZonePointer = codeWriteablePointer + debugAbbrevOffset;
    uint8_t *debugAbbrevZoneExecutablePointer = codeExecutablePointer + debugAbbrevOffset;
    memcpy(debugAbbrevZonePointer, jit->dwarfDebugInfoBuilder.abbrev.data, jit->dwarfDebugInfoBuilder.abbrev.size);

    uint8_t *debugInfoZonePointer = codeWriteablePointer + debugInfoOffset;
    uint8_t *debugInfoZoneExecutablePointer = codeExecutablePointer + debugInfoOffset;
    memcpy(debugInfoZonePointer, jit->dwarfDebugInfoBuilder.info.data, jit->dwarfDebugInfoBuilder.info.size);

    uint8_t *objectFileContentPointer = codeWriteablePointer + objectFileContentOffset;
    uint8_t *objectFileContentExecutablePointer = codeExecutablePointer + objectFileContentOffset;
    memcpy(objectFileContentPointer, jit->objectFileContent.data, jit->objectFileContent.size);

    sysbvm_jit_fixupObjectFile(jit,
        (sysbvm_elf64_header_t*)objectFileHeaderPointer,
        (sysbvm_elf64_header_t*)objectFileHeaderExecutablePointer,
        instructionsExecutablePointers,
        ehFrameZonePointer, ehFrameZoneExecutablePointer,
        debugLineZoneExecutablePointer,
        debugStrZoneExecutablePointer,
        debugAbbrevZoneExecutablePointer,
        debugInfoZoneExecutablePointer,
        objectFileContentExecutablePointer,
        (sysbvm_jit_x64_elfContentFooter_t*) (objectFileContentPointer + jit->objectFileContent.size - sizeof(sysbvm_jit_x64_elfContentFooter_t))
    );

    sysbvm_jit_emitPerfSymbolFor(jit, instructionsPointers);

#ifdef _WIN32
    RUNTIME_FUNCTION *runtimeFunction = (RUNTIME_FUNCTION*)unwindInfoZoneExecutablePointer;
    runtimeFunction->UnwindInfoAddress = (DWORD)(sizeof(RUNTIME_FUNCTION) + unwindInfoZoneExecutablePointer - instructionsExecutablePointers);
    if(RtlAddFunctionTable(runtimeFunction, 1, (DWORD64)(uintptr_t)instructionsExecutablePointers))
    {
        // Store the handle in the context for cleanup.
    }
#else
    (void)unwindInfoZoneExecutablePointer;
    if(jit->dwarfEhBuilder.buffer.size > 0)
    {
#   ifdef __APPLE__
        // It takes the FDE parameter
        if(jit->dwarfEhBuilder.fdeOffset > 0)
        {
            void *fdePointer = ehFrameZoneExecutablePointer + jit->dwarfEhBuilder.fdeOffset;
            sysbvm_dynarray_add(&jit->context->jittedRegisteredFrames, &fdePointer);
            __register_frame(fdePointer);
        }
#   else
        // Send the eh_frame section.
        sysbvm_dynarray_add(&jit->context->jittedRegisteredFrames, &ehFrameZoneExecutablePointer);
        __register_frame(ehFrameZoneExecutablePointer);
#   endif
    }
#endif

    return instructionsExecutablePointers;
}

#endif // SYSBVM_ARCH_X86_64
