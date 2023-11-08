#include "sysbvm/chunkedAllocator.h"
#include "sysbvm/assert.h"
#include "internal/virtualMemory.h"
#include <stdlib.h>
#include <string.h>

static size_t sysbvm_chunkedAllocator_sizeAlignedTo(size_t size, size_t alignment)
{
    return (size + alignment - 1) & (~(alignment - 1));
}

SYSBVM_API void sysbvm_chunkedAllocator_initialize(sysbvm_chunkedAllocator_t *allocator, size_t chunkSize, bool requiresExecutableMapping)
{
    allocator->chunkSize = chunkSize;
    allocator->requiresExecutableMapping = requiresExecutableMapping;
}

SYSBVM_API void sysbvm_chunkedAllocator_destroy(sysbvm_chunkedAllocator_t *allocator)
{
    sysbvm_chunkedAllocatorChunk_t *chunk = allocator->firstChunk;
    while(chunk)
    {
        sysbvm_chunkedAllocatorChunk_t *nextChunk = chunk->next;
        size_t fullChunkSize = sizeof(sysbvm_chunkedAllocatorChunk_t) + chunk->capacity;

        if(allocator->requiresExecutableMapping && chunk->writeableMapping != chunk->executableMapping)
        {
            sysbvm_chunkedAllocatorChunk_t *writeableMapping = chunk->writeableMapping;
            sysbvm_chunkedAllocatorChunk_t *executableMapping = chunk->executableMapping;
            sysbvm_virtualMemory_freeSystemMemoryWithDualMapping(fullChunkSize, chunk->dualMappingHandle, writeableMapping, executableMapping);
            chunk = nextChunk;

        }
        else
        {
            sysbvm_chunkedAllocatorChunk_t *writeableMapping = chunk->writeableMapping;
            if(writeableMapping)
                sysbvm_virtualMemory_freeSystemMemory(writeableMapping, fullChunkSize);
        }

        chunk = nextChunk;
    }
}

static sysbvm_chunkedAllocatorChunk_t *sysbvm_chunkedAllocator_ensureChunkWithRequiredCapacity(sysbvm_chunkedAllocator_t *allocator, size_t size, size_t alignment)
{
    sysbvm_chunkedAllocatorChunk_t **currentChunk = &allocator->currentChunk;

    // Advance the current chunk.
    size_t requiredAlignedSize = sysbvm_chunkedAllocator_sizeAlignedTo(size, alignment);
    while(*currentChunk && ((*currentChunk)->capacity - sysbvm_chunkedAllocator_sizeAlignedTo((*currentChunk)->size, alignment)) < requiredAlignedSize)
        *currentChunk = (*currentChunk)->next;

    // Create a new chunk if needed
    if(*currentChunk == NULL)
    {
        sysbvm_chunkedAllocatorChunk_t *newChunkWriteableMapping = NULL;
        sysbvm_chunkedAllocatorChunk_t *newChunkExecutableMapping = NULL;

        if(allocator->requiresExecutableMapping)
        {
            if(sysbvm_virtualMemory_hasSupportForRWXMapping())
            {
                newChunkExecutableMapping = newChunkWriteableMapping = (sysbvm_chunkedAllocatorChunk_t*)sysbvm_virtualMemory_allocateSystemMemory(allocator->chunkSize);
                memset(newChunkWriteableMapping, 0, sizeof(sysbvm_chunkedAllocatorChunk_t));
            }
            else
            {
                void *handle = sysbvm_virtualMemory_allocateSystemMemoryWithDualMapping(allocator->chunkSize, (void**)&newChunkWriteableMapping, (void**)&newChunkExecutableMapping);
                memset(newChunkWriteableMapping, 0, sizeof(sysbvm_chunkedAllocatorChunk_t));
                newChunkWriteableMapping->dualMappingHandle = handle;
            }
        }
        else
        {
            newChunkWriteableMapping = (sysbvm_chunkedAllocatorChunk_t*)sysbvm_virtualMemory_allocateSystemMemory(allocator->chunkSize);
            memset(newChunkWriteableMapping, 0, sizeof(sysbvm_chunkedAllocatorChunk_t));
        }
        
        newChunkWriteableMapping->capacity = allocator->chunkSize - sizeof(sysbvm_chunkedAllocatorChunk_t);
        newChunkWriteableMapping->writeableMapping = newChunkWriteableMapping;
        newChunkWriteableMapping->executableMapping = newChunkExecutableMapping;

        if(!allocator->firstChunk)
            allocator->firstChunk = newChunkWriteableMapping;

        if(allocator->lastChunk)
        {
            if(allocator->requiresExecutableMapping &&
                !sysbvm_virtualMemory_lockCodePagesForWriting(allocator->lastChunk->writeableMapping, allocator->lastChunk->executableMapping, sizeof(sysbvm_chunkedAllocatorChunk_t)))
                abort();

            allocator->lastChunk->next = newChunkWriteableMapping;
            newChunkWriteableMapping->previous = allocator->lastChunk;

            if(allocator->requiresExecutableMapping)
                sysbvm_virtualMemory_unlockCodePagesForExecution(allocator->lastChunk->writeableMapping, allocator->lastChunk->executableMapping, sizeof(sysbvm_chunkedAllocatorChunk_t));
        }
        allocator->lastChunk = newChunkWriteableMapping;

        *currentChunk = newChunkWriteableMapping;
    }

    return *currentChunk;

}

SYSBVM_API void* sysbvm_chunkedAllocator_allocate(sysbvm_chunkedAllocator_t *allocator, size_t size, size_t alignment)
{
    sysbvm_chunkedAllocatorChunk_t *chunk = sysbvm_chunkedAllocator_ensureChunkWithRequiredCapacity(allocator, size, alignment);
    SYSBVM_ASSERT(chunk);

    size_t alignedOffset = sysbvm_chunkedAllocator_sizeAlignedTo(chunk->size, alignment);
    uint8_t *result = (uint8_t*)(chunk + 1) + alignedOffset;
    chunk->size = alignedOffset + size;
    SYSBVM_ASSERT(chunk->size <= chunk->capacity);

    return result;
}

SYSBVM_API void sysbvm_chunkedAllocator_allocateWithDualMapping(sysbvm_chunkedAllocator_t *allocator, size_t size, size_t alignment, void **writeableMapping, void **executableMapping)
{

    sysbvm_chunkedAllocatorChunk_t *chunk = sysbvm_chunkedAllocator_ensureChunkWithRequiredCapacity(allocator, size, alignment);
    SYSBVM_ASSERT(chunk);

    size_t alignedOffset = sysbvm_chunkedAllocator_sizeAlignedTo(chunk->size, alignment);
    if(writeableMapping)
        *writeableMapping = (uint8_t*)(chunk->writeableMapping + 1) + alignedOffset;
    if(executableMapping)
        *executableMapping = (uint8_t*)(chunk->executableMapping + 1) + alignedOffset;

    if(!sysbvm_virtualMemory_lockCodePagesForWriting(chunk->writeableMapping, chunk->executableMapping, sizeof(sysbvm_chunkedAllocatorChunk_t)))
        abort();

    chunk->size = alignedOffset + size;
    SYSBVM_ASSERT(chunk->size <= chunk->capacity);

    sysbvm_virtualMemory_unlockCodePagesForExecution(chunk->writeableMapping, chunk->executableMapping, sizeof(sysbvm_chunkedAllocatorChunk_t));
}
