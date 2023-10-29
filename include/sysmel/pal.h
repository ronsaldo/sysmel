#ifndef SYSMEL_PAL_H
#define SYSMEL_PAL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
#define SYSMEL_PAL_EXTERN_C extern "C"
#else
#define SYSMEL_PAL_EXTERN_C
#endif

#define SYSMEl_PAL_FILE_OPEN_FLAGS_READ_ONLY (1<<0)
#define SYSMEl_PAL_FILE_OPEN_FLAGS_WRITE_ONLY (1<<1)
#define SYSMEl_PAL_FILE_OPEN_FLAGS_READ_WRITE (1<<2)

#define SYSMEl_PAL_FILE_OPEN_FLAGS_CREATE (1<<3)
#define SYSMEl_PAL_FILE_OPEN_FLAGS_TRUNCATE (1<<4)
#define SYSMEl_PAL_FILE_OPEN_FLAGS_APPEND (1<<5)
#define SYSMEl_PAL_FILE_OPEN_FLAGS_CLOSE_ON_EXEC (1<<6)

#define SYSMEl_PAL_SEEK_MODE_SET 0
#define SYSMEl_PAL_SEEK_MODE_CURRENT 1
#define SYSMEl_PAL_SEEK_MODE_END 2

typedef struct sysmel_pal_file_s *sysmel_pal_filehandle_t;

typedef struct sysmel_pal_thread_s
{
    uint64_t reserved;
} sysmel_pal_thread_t;

typedef struct sysmel_pal_mutex_s
{
    uint64_t reserved[8];
} sysmel_pal_mutex_t;

typedef struct sysmel_pal_condition_s
{
    uint64_t reserved[8];
} sysmel_pal_condition_t;

typedef enum
{
    SYSMEL_PAL_GDB_JIT_NOACTION = 0,
    SYSMEL_PAL_GDB_JIT_REGISTER_FN,
    SYSMEL_PAL_GDB_JIT_UNREGISTER_FN
} sysmel_pal_gdb_jit_actions_t;

typedef struct sysmel_pal_gdb_jit_code_entry_s
{
    struct sysmel_pal_gdb_jit_code_entry_s *next_entry;
    struct sysmel_pal_gdb_jit_code_entry_s *prev_entry;
    const char *symfile_addr;
    uint64_t symfile_size;
} sysmel_pal_gdb_jit_code_entry_t;

typedef struct sysmel_pal_gdb_jit_descriptor_s
{
    uint32_t version;
    /* This type should be jit_actions_t, but we use uint32_t
        to be explicit about the bitwidth.  */
    uint32_t action_flag;
    sysmel_pal_gdb_jit_code_entry_t *relevant_entry;
    sysmel_pal_gdb_jit_code_entry_t *first_entry;
} sysmel_pal_gdb_jit_descriptor_t;

SYSMEL_PAL_EXTERN_C void sysmel_pal_abort(void);

SYSMEL_PAL_EXTERN_C void* sysmel_pal_malloc(size_t size);
SYSMEL_PAL_EXTERN_C void sysmel_pal_free(void *pointer);

SYSMEL_PAL_EXTERN_C void *sysmel_pal_allocateSystemMemory(size_t size);
SYSMEL_PAL_EXTERN_C void sysmel_pal_freeSystemMemory(void *memoryPointer, size_t size);

SYSMEL_PAL_EXTERN_C bool sysmel_pal_supportsMemoryWithDualMappingForJIT(void);
SYSMEL_PAL_EXTERN_C bool sysmel_pal_allocateMemoryWithDualMappingForJIT(size_t size, void **outHandle, void **outWriteMemoryPointer, void **outExecuteMemoryPointer);
SYSMEL_PAL_EXTERN_C void sysmel_pal_freeMemoryWithDualMappingForJIT(size_t size, void *handle, void *writeMemoryPointer, void *executeMemoryPointer);

SYSMEL_PAL_EXTERN_C void sysmel_pal_flushInstructionCache(size_t size, void *pointer);

SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_getStdinFileHandle(void);
SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_getStdoutFileHandle(void);
SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_getStderrFileHandle(void);

SYSMEL_PAL_EXTERN_C sysmel_pal_filehandle_t sysmel_pal_openFile( size_t nameSize, const char *name, uint32_t openFlags, uint32_t creationPermissions);
SYSMEL_PAL_EXTERN_C void sysmel_pal_closeFile(sysmel_pal_filehandle_t handle);
SYSMEL_PAL_EXTERN_C bool sysmel_pal_isFileHandleValid(sysmel_pal_filehandle_t handle);

SYSMEL_PAL_EXTERN_C int64_t sysmel_pal_seek(sysmel_pal_filehandle_t handle, int64_t offset, int32_t mode);

SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_writeToFile(sysmel_pal_filehandle_t handle, size_t size, const void *buffer);
SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_writeToFileAtOffset(sysmel_pal_filehandle_t handle, uint64_t offset, size_t size, const void *buffer);

SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_readFromFile(sysmel_pal_filehandle_t handle, size_t size, void *buffer);
SYSMEL_PAL_EXTERN_C intptr_t sysmel_pal_readFromFileAtOffset(sysmel_pal_filehandle_t handle, uint64_t offset, size_t size, void *buffer);

SYSMEL_PAL_EXTERN_C double sysmel_pal_parseFloat64(size_t size, const char *string);
SYSMEL_PAL_EXTERN_C char *sysmel_pal_float64ToString(double value);

SYSMEL_PAL_EXTERN_C int64_t sysmel_pal_microsecondsNow(void);
SYSMEL_PAL_EXTERN_C int64_t sysmel_pal_nanosecondsNow(void);

SYSMEL_PAL_EXTERN_C void sysmel_pal_mutex_initialize(sysmel_pal_mutex_t *handle);
SYSMEL_PAL_EXTERN_C void sysmel_pal_mutex_finalize(sysmel_pal_mutex_t *handle);
SYSMEL_PAL_EXTERN_C void sysmel_pal_mutex_lock(sysmel_pal_mutex_t *handle);
SYSMEL_PAL_EXTERN_C void sysmel_pal_mutex_unlock(sysmel_pal_mutex_t *handle);

SYSMEL_PAL_EXTERN_C void sysmel_pal_condition_initialize(sysmel_pal_condition_t *handle);
SYSMEL_PAL_EXTERN_C void sysmel_pal_condition_finalize(sysmel_pal_condition_t *handle);
SYSMEL_PAL_EXTERN_C void sysmel_pal_condition_wait(sysmel_pal_condition_t *handle, sysmel_pal_mutex_t *mutex);
SYSMEL_PAL_EXTERN_C void sysmel_pal_condition_signal(sysmel_pal_condition_t *handle);
SYSMEL_PAL_EXTERN_C void sysmel_pal_condition_broadcast(sysmel_pal_condition_t *handle);

SYSMEL_PAL_EXTERN_C void* sysmel_pal_openLibrary(size_t nameSize, const char *name);
SYSMEL_PAL_EXTERN_C void sysmel_pal_closeLibrary(void *handle);
SYSMEL_PAL_EXTERN_C bool sysmel_pal_getLibrarySymbol(void *handle, size_t nameSize, const char *name, void **outSymbol);
SYSMEL_PAL_EXTERN_C bool sysmel_pal_getApplicationSymbol(size_t nameSize, const char *name, void **outSymbol);

SYSMEL_PAL_EXTERN_C void sysmel_pal_gdb_registerObjectFile(sysmel_pal_gdb_jit_code_entry_t *entry);
SYSMEL_PAL_EXTERN_C void sysmel_pal_gdb_unregisterObjectFile(sysmel_pal_gdb_jit_code_entry_t *entry);

SYSMEL_PAL_EXTERN_C void sysmel_pal_eh_frame_registerSection(const void *section);

#endif //SYSMEL_PAL_H
