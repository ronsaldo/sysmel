#ifndef _WIN32

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#   ifndef _DEFAULT_SOURCE
#       define _DEFAULT_SOURCE // for realpath
#   endif

#   ifndef _XOPEN_SOURCE
#       define _XOPEN_SOURCE 600 // CLOCK_MONOTONIC
#   endif
#endif

#include "array.c"
#include "arraySlice.c"
#include "assert.c"
#include "association.c"
#include "ast.c"
#include "backtrace.c"
#include "boolean.c"
#include "bytecode.c"
#include "bytecodeCompiler.c"
#include "bytecodeJitCommon.c"
#include "bytecodeJitX86.c"
#include "byteStream.c"
#include "chunkedAllocator.c"
#include "context.c"
#include "dictionary.c"
#include "dwarf.c"
#include "dynarray.c"
#include "environment.c"
#include "errors.c"
#include "exceptions.c"
#include "filesystem.c"
#include "float.c"
#include "function.c"
#include "elf.c"
#include "gc.c"
#include "gdb.c"
#include "heap.c"
#include "integer.c"
#include "interpreter.c"
#include "io.c"
#include "macro.c"
#include "message.c"
#include "orderedCollection.c"
#include "orderedOffsetTable.c"
#include "parser.c"
#include "pic.c"
#include "pragma.c"
#include "primitiveIntegers.c"
#include "programEntity.c"
#include "scanner.c"
#include "set.c"
#include "sourceCode.c"
#include "sourcePosition.c"
#include "stackFrame.c"
#include "string.c"
#include "stringStream.c"
#include "system.c"
#include "time.c"
#include "token.c"
#include "tuple.c"
#include "type.c"
#include "virtualMemory.c"
