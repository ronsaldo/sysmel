#include "sysmel/pal.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
#include <dlfcn.h>

extern void __register_frame(const void*);

SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_getStdinFileHandle(void)
{
    return (sysmel_pal_filehandle_t)(uintptr_t)STDIN_FILENO;
}

SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_getStdoutFileHandle(void)
{
    return (sysmel_pal_filehandle_t)(uintptr_t)STDOUT_FILENO;
}

SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_getStderrFileHandle(void)
{
    return (sysmel_pal_filehandle_t)(uintptr_t)STDERR_FILENO;
}

SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_openFile(size_t nameSize, const char *name, uint32_t openFlags, uint32_t creationPermissions)
{
    char *nameBuffer = (char*)malloc(nameSize + 1);
    memcpy(nameBuffer, name, nameSize);
    nameBuffer[nameSize] = 0;

    int unixOpenFlags = 0;
    if(openFlags & SYSMEl_PAL_FILE_OPEN_FLAGS_READ_ONLY)
        unixOpenFlags |= O_RDONLY;
    if(openFlags & SYSMEl_PAL_FILE_OPEN_FLAGS_WRITE_ONLY)
        unixOpenFlags |= O_WRONLY;
    if(openFlags & SYSMEl_PAL_FILE_OPEN_FLAGS_READ_WRITE)
        unixOpenFlags |= O_RDWR;

    if(openFlags & SYSMEl_PAL_FILE_OPEN_FLAGS_CREATE)
        unixOpenFlags |= O_CREAT;
    if(openFlags & SYSMEl_PAL_FILE_OPEN_FLAGS_TRUNCATE)
        unixOpenFlags |= O_TRUNC;
    if(openFlags & SYSMEl_PAL_FILE_OPEN_FLAGS_APPEND)
        unixOpenFlags |= O_APPEND;
#ifdef O_CLOEXEC
    if(openFlags & SYSMEl_PAL_FILE_OPEN_FLAGS_CLOSE_ON_EXEC)
        unixOpenFlags |= O_CLOEXEC;
#endif

    int fd = open(nameBuffer, unixOpenFlags, creationPermissions);
    free(nameBuffer);

    return (sysmel_pal_filehandle_t)(intptr_t)fd;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_closeFile(sysmel_pal_filehandle_t handle)
{
    close((intptr_t)handle);
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_isFileHandleValid(sysmel_pal_filehandle_t handle)
{
    return (intptr_t)handle >= 0;
}

SYSMEL_PAL_EXTERN_C int64_t sysmel_pal_seek(sysmel_pal_filehandle_t handle, int64_t offset, int32_t mode)
{
    int unixSeekMode = SEEK_SET;
    switch(mode)
    {
    default:
    case SYSMEl_PAL_SEEK_MODE_SET:
        unixSeekMode = SEEK_SET;
        break;
    case SYSMEl_PAL_SEEK_MODE_CURRENT:
        unixSeekMode = SEEK_CUR;
        break;
    case SYSMEl_PAL_SEEK_MODE_END:
        unixSeekMode = SEEK_END;
        break;
    }

    return lseek((intptr_t)handle, offset, unixSeekMode);
}

SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_writeToFile(sysmel_pal_filehandle_t handle, size_t size, const void *buffer)
{
    return write((intptr_t)handle, buffer, size);
}

SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_writeToFileAtOffset(sysmel_pal_filehandle_t handle, uint64_t offset, size_t size, const void *buffer)
{
    return pwrite((intptr_t)handle, buffer, size, offset);
}

SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_readFromFile(sysmel_pal_filehandle_t handle, size_t size, void *buffer)
{
    return read((intptr_t)handle, buffer, size);
}

SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_readFromFileAtOffset(sysmel_pal_filehandle_t handle, uint64_t offset, size_t size, void *buffer)
{
    return pread((intptr_t)handle, buffer, size, offset);
}

SYSMEL_PAL_EXTERN_C void *sysmel_pal_allocateSystemMemory(size_t size)
{
    void *result = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if(result == MAP_FAILED)
        return 0;

    return result;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_freeSystemMemory(void *memoryPointer, size_t size)
{
    munmap(memoryPointer, size);
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_supportsMemoryWithDualMappingForJIT(void)
{
    return true;
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_allocateMemoryWithDualMappingForJIT(size_t size, void **outHandle, void **outWriteMemoryPointer, void **outExecuteMemoryPointer)
{
    *outHandle = NULL;
    *outWriteMemoryPointer = NULL;
    *outExecuteMemoryPointer = NULL;

    int fd = memfd_create("sysmel-pal-jit", MFD_CLOEXEC);
    if(fd < 0)
        return false;

    if(ftruncate(fd, size) < 0)
    {
        close(fd);
        return false;
    }

    void *writeMapping = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(writeMapping == MAP_FAILED)
    {
        close(fd);
        return false;
    }

    void *executableMapping = mmap(0, size, PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
    if(executableMapping == MAP_FAILED)
    {
        munmap(writeMapping, size);
        close(fd);
        return false;
    }

    *outHandle = (void*)(intptr_t)fd;
    *outWriteMemoryPointer = writeMapping;
    *outExecuteMemoryPointer = executableMapping;
    return true;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_freeMemoryWithDualMappingForJIT(size_t size, void *handle, void *writeMemoryPointer, void *executeMemoryPointer)
{
    int fd = (intptr_t)handle;

    munmap(writeMemoryPointer, size);
    munmap(executeMemoryPointer, size);
    close(fd);
}

SYSMEL_PAL_EXTERN_C int64_t sysmel_pal_microsecondsNow(void)
{
    struct timespec ts = {};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * (int64_t)1000000 + (int64_t)ts.tv_nsec / (int64_t)1000;
}

SYSMEL_PAL_EXTERN_C int64_t sysmel_pal_nanosecondsNow(void)
{
    struct timespec ts = {};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * (int64_t)1000000000 + (int64_t)ts.tv_nsec;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_mutex_initialize(sysmel_pal_mutex_t *handle)
{
    assert(sizeof(pthread_mutex_t) <= sizeof(sysmel_pal_mutex_t));
    pthread_mutex_init((pthread_mutex_t*)handle, NULL);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_mutex_finalize(sysmel_pal_mutex_t *handle)
{
    assert(sizeof(pthread_mutex_t) <= sizeof(sysmel_pal_mutex_t));
    pthread_mutex_destroy((pthread_mutex_t*)handle);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_mutex_lock(sysmel_pal_mutex_t *handle)
{
    assert(sizeof(pthread_mutex_t) <= sizeof(sysmel_pal_mutex_t));
    pthread_mutex_lock((pthread_mutex_t*)handle);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_mutex_unlock(sysmel_pal_mutex_t *handle)
{
    assert(sizeof(pthread_mutex_t) <= sizeof(sysmel_pal_mutex_t));
    pthread_mutex_unlock((pthread_mutex_t*)handle);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_condition_initialize(sysmel_pal_condition_t *handle)
{
    assert(sizeof(pthread_cond_t) <= sizeof(sysmel_pal_condition_t));
    pthread_cond_init((pthread_cond_t*)handle, NULL);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_condition_finalize(sysmel_pal_condition_t *handle)
{
    assert(sizeof(pthread_cond_t) <= sizeof(sysmel_pal_condition_t));
    pthread_cond_destroy((pthread_cond_t*)handle);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_condition_wait(sysmel_pal_condition_t *handle, sysmel_pal_mutex_t *mutex)
{
    assert(sizeof(pthread_cond_t) <= sizeof(sysmel_pal_condition_t));
    assert(sizeof(pthread_mutex_t) <= sizeof(sysmel_pal_mutex_t));
    pthread_cond_wait((pthread_cond_t*)handle, (pthread_mutex_t*)mutex);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_condition_signal(sysmel_pal_condition_t *handle)
{
    assert(sizeof(pthread_cond_t) <= sizeof(sysmel_pal_condition_t));
    pthread_cond_signal((pthread_cond_t*)handle);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_condition_broadcast(sysmel_pal_condition_t *handle)
{
    assert(sizeof(pthread_cond_t) <= sizeof(sysmel_pal_condition_t));
    pthread_cond_broadcast((pthread_cond_t*)handle);
}

SYSMEL_PAL_EXTERN_C void* sysmel_pal_openLibrary(size_t nameSize, const char *name)
{
    char *nameCString = malloc(nameSize + 1);
    memcpy(nameCString, name, nameSize);
    nameCString[nameSize] = 0;

    void *handle = dlopen(nameCString, RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
    free(nameCString);
    return handle;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_closeLibrary(void *handle)
{
    dlclose(handle);
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_getLibrarySymbol(void *handle, size_t nameSize, const char *name, void **outSymbol)
{
    char *nameCString = malloc(nameSize + 1);
    memcpy(nameCString, name, nameSize);
    nameCString[nameSize] = 0;

    dlerror();
    *outSymbol = dlsym(handle, nameCString);
    char *error = dlerror();

    free(nameCString);
    return error == NULL;
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_getApplicationSymbol(size_t nameSize, const char *name, void **outSymbol)
{
    return sysmel_pal_getLibrarySymbol(NULL, nameSize, name, outSymbol);
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_flushInstructionCache(size_t size, void *pointer)
{
    __builtin___clear_cache(pointer, (char*)pointer + size);
}

SYSMEL_PAL_EXTERN_C bool sysmel_pal_eh_frame_shouldRegisterFDE(void)
{
    return false;
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_eh_frame_registerSection(const void *section)
{
#ifdef __APPLE__
    uint32_t *position = (uint32_t*)section;
    while(*position)
    {
        uint32_t length = position[0];
        uint32_t cieId = position[1];
        if(cieId)
            __register_frame(position);

        position = (uint8_t*)position + 4 + length;
    }
#else
    __register_frame(section);
#endif
}
