#ifndef SYSBVM_CHUNKED_ALLOCATOR_H
#define SYSBVM_CHUNKED_ALLOCATOR_H

#include "common.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define SYSBVM_CHUNKED_ALLOCATOR_DEFAULT_CHUNK_SIZE ((size_t)(2<<20))

typedef struct sysbvm_chunkedAllocatorChunk_s
{
    struct sysbvm_chunkedAllocatorChunk_s *previous;
    struct sysbvm_chunkedAllocatorChunk_s *next;

    struct sysbvm_chunkedAllocatorChunk_s *writeableMapping;
    struct sysbvm_chunkedAllocatorChunk_s *executableMapping;

    void *dualMappingHandle;
    void *reserved;

    size_t capacity;
    size_t size;
} sysbvm_chunkedAllocatorChunk_t;

typedef struct sysbvm_chunkedAllocator_s
{
    sysbvm_chunkedAllocatorChunk_t* firstChunk;
    sysbvm_chunkedAllocatorChunk_t* lastChunk;
    sysbvm_chunkedAllocatorChunk_t* currentChunk;
    size_t chunkSize;
    bool requiresExecutableMapping;
} sysbvm_chunkedAllocator_t;

typedef struct sysbvm_chunkedAllocatorIterator_s
{
    sysbvm_chunkedAllocatorChunk_t* chunk;
    size_t size;
    uint8_t *data;
} sysbvm_chunkedAllocatorIterator_t;

SYSBVM_INLINE void sysbvm_chunkedAllocatorIterator_setForChunk(sysbvm_chunkedAllocatorIterator_t *iterator, sysbvm_chunkedAllocatorChunk_t* chunk)
{
    iterator->chunk = chunk;
    if(chunk)
    {
        iterator->size = chunk->size;
        iterator->data = (uint8_t*)(chunk + 1);
    }
    else
    {
        iterator->size = 0;
        iterator->data = NULL;
    }
}

SYSBVM_INLINE void sysbvm_chunkedAllocatorIterator_begin(sysbvm_chunkedAllocator_t *allocator, sysbvm_chunkedAllocatorIterator_t *iterator)
{
    sysbvm_chunkedAllocatorIterator_setForChunk(iterator, allocator->firstChunk);
}

SYSBVM_INLINE void sysbvm_chunkedAllocatorIterator_advance(sysbvm_chunkedAllocatorIterator_t *iterator)
{
    if(iterator->chunk)
        sysbvm_chunkedAllocatorIterator_setForChunk(iterator, iterator->chunk->next);
}

SYSBVM_INLINE bool sysbvm_chunkedAllocatorIterator_isValid(sysbvm_chunkedAllocatorIterator_t *iterator)
{
    return iterator->chunk != NULL;
}

SYSBVM_API void sysbvm_chunkedAllocator_initialize(sysbvm_chunkedAllocator_t *allocator, size_t chunkSize, bool requiresExecutableMapping);
SYSBVM_API void sysbvm_chunkedAllocator_destroy(sysbvm_chunkedAllocator_t *allocator);

SYSBVM_API void* sysbvm_chunkedAllocator_allocate(sysbvm_chunkedAllocator_t *allocator, size_t size, size_t alignment);
SYSBVM_API void sysbvm_chunkedAllocator_allocateWithDualMapping(sysbvm_chunkedAllocator_t *allocator, size_t size, size_t alignment, void **writeableMapping, void **executableMapping);

#endif //SYSBVM_CHUNKED_ALLOCATOR_H
