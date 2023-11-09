#include "sysmel/pal.h"

SYSMEL_PAL_EXTERN_C void *sysmel_pal_allocateSystemMemory(size_t size)
{
    return malloc(size);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_freeSystemMemory(void *memoryPointer, size_t size)
{
    (void)size;
    free(memoryPointer);
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_supportsMemoryWithRWXPermissions(void)
{
    return false;
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_lockCodeMemoryForWriting(void *memoryPointer, size_t size)
{
    (void)memoryPointer;
    (void)size;
    return false;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_unlockCodeMemoryForExecution(void *memoryPointer, size_t size)
{
    (void)memoryPointer;
    (void)size;
    return false;
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_supportsMemoryWithDualMappingForJIT(void)
{
    return false;
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_allocateMemoryWithDualMappingForJIT(size_t size, void **outHandle, void **outWriteMemoryPointer, void **outExecuteMemoryPointer)
{
    (void)size;
    *outHandle = NULL;
    *outWriteMemoryPointer = NULL;
    *outExecuteMemoryPointer = NULL;
    return false;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_freeMemoryWithDualMappingForJIT(size_t size, void *handle, void *writeMemoryPointer, void *executeMemoryPointer)
{
    (void)size;
    (void)handle;
    (void)writeMemoryPointer;
    (void)executeMemoryPointer;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_flushInstructionCache(size_t size, void *pointer)
{
    (void)size;
    (void)pointer;
}
