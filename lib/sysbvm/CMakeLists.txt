set(sysbvm_sources
    array.c
    arraySlice.c
    assert.c
    association.c
    ast.c
    backtrace.c
    boolean.c
    bytecode.c
    bytecodeCompiler.c
    bytecodeJitCommon.c
    bytecodeJitX86.c
    byteStream.c
    chunkedAllocator.c
    context.c
    dictionary.c
    dwarf.c
    dynarray.c
    environment.c
    errors.c
    exceptions.c
    filesystem.c
    float.c
    function.c
    elf.c
    gc.c
    gdb.c
    heap.c
    integer.c
    interpreter.c
    io.c
    macro.c
    message.c
    orderedCollection.c
    orderedOffsetTable.c
    package.c
    parser.c
    pic.c
    pragma.c
    programEntity.c
    primitiveIntegers.c
    scanner.c
    set.c
    sourceCode.c
    sourcePosition.c
    stackFrame.c
    string.c
    stringStream.c
    system.c
    time.c
    token.c
    tuple.c
    type.c
    virtualMemory.c
)

set(USE_UNITY_BUILD False)
if(USE_UNITY_BUILD)
    set(sysbvm_sources unityBuild.c)
endif()

add_definitions(-DBUILD_SYSBVM)
add_library(sysbvm STATIC ${sysbvm_sources})
target_link_libraries(sysbvm ${SYSBVM_DEP_LIBS})