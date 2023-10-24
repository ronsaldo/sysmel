#include "sysmel/pal.h"

#ifdef _MSC_VER
#define SYSBVM_NOINLINE __declspec(noinline)
#else
#define SYSBVM_NOINLINE __attribute__((noinline))
#endif

void SYSBVM_NOINLINE __jit_debug_register_code() {
#ifdef _MSC_VER
    __nop();
#else
    asm volatile("" ::: "memory");
#endif
};

SYSMEL_PAL_EXTERN_C sysmel_pal_gdb_jit_descriptor_t __jit_debug_descriptor = { 1, 0, 0, 0 };

SYSMEL_PAL_EXTERN_C void sysmel_pal_gdb_registerObjectFile(sysmel_pal_gdb_jit_code_entry_t *entry)
{
    entry->next_entry = __jit_debug_descriptor.first_entry;
    if(entry->next_entry)
        entry->next_entry = entry->next_entry->prev_entry;
    __jit_debug_descriptor.relevant_entry = entry;
    __jit_debug_descriptor.action_flag = SYSMEL_PAL_GDB_JIT_REGISTER_FN;
    __jit_debug_register_code();
}

SYSMEL_PAL_EXTERN_C void sysmel_pal_gdb_unregisterObjectFile(sysmel_pal_gdb_jit_code_entry_t *entry)
{
    if(entry->prev_entry)
        entry->prev_entry->next_entry = entry->next_entry;
    else
        __jit_debug_descriptor.first_entry = entry->next_entry;

    if(entry->next_entry)
        entry->next_entry->prev_entry = entry->prev_entry;

    entry->prev_entry = entry->next_entry = NULL;
    
    __jit_debug_descriptor.relevant_entry = entry;
    __jit_debug_descriptor.action_flag = SYSMEL_PAL_GDB_JIT_UNREGISTER_FN;
    __jit_debug_register_code();
}
