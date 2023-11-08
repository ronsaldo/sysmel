#ifndef SYSBVM_INTERNAL_VIRTUAL_MEMORY_H
#define SYSBVM_INTERNAL_VIRTUAL_MEMORY_H

#pragma once

#include "sysbvm/common.h"
#include <stddef.h>
#include <stdbool.h>

bool sysbvm_virtualMemory_hasSupportForRWXMapping(void);

void *sysbvm_virtualMemory_allocateSystemMemory(size_t sizeToAllocate);
void sysbvm_virtualMemory_freeSystemMemory(void *memory, size_t sizeToFree);

size_t sysbvm_virtualMemory_getSystemAllocationAlignment(void);

void *sysbvm_virtualMemory_allocateSystemMemoryWithDualMapping(size_t sizeToAllocate, void **writeableMapping, void **executableMapping);
void sysbvm_virtualMemory_freeSystemMemoryWithDualMapping(size_t sizeToFree, void *mappingHandle, void *writeableMapping, void *executableMapping);

bool sysbvm_virtualMemory_lockCodePagesForWriting(void *writePointer, void *executablePointer, size_t size);
void sysbvm_virtualMemory_unlockCodePagesForExecution(void *writePointer, void *executablePointer, size_t size);

#endif //SYSBVM_INTERNAL_VIRTUAL_MEMORY_H
