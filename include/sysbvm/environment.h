#ifndef SYSBVM_ENVIRONMENT_H
#define SYSBVM_ENVIRONMENT_H

#pragma once

#include "programEntity.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_analysisQueueEntry_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t programEntity;
    sysbvm_tuple_t nextEntry;
} sysbvm_analysisQueueEntry_t;

typedef struct sysbvm_analysisQueue_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t firstEntry;
    sysbvm_tuple_t lastEntry;
} sysbvm_analysisQueue_t;

typedef struct sysbvm_environment_s
{
    sysbvm_programEntityWithChildren_t super;
    sysbvm_tuple_t parent;
    sysbvm_tuple_t analysisQueue;
    sysbvm_tuple_t symbolTable;
} sysbvm_environment_t;

typedef sysbvm_environment_t sysbvm_namespace_t;

typedef struct sysbvm_analysisAndEvaluationEnvironment_s
{
    sysbvm_environment_t super;
    sysbvm_tuple_t usedTuplesWithNamedSlots;
    sysbvm_tuple_t analyzerToken;
    sysbvm_tuple_t expectedType;
    sysbvm_tuple_t returnTarget;
    sysbvm_tuple_t breakTarget;
    sysbvm_tuple_t continueTarget;
} sysbvm_analysisAndEvaluationEnvironment_t;

typedef struct sysbvm_functionActivationEnvironment_s
{
    sysbvm_analysisAndEvaluationEnvironment_t super;
    sysbvm_tuple_t function;
    sysbvm_tuple_t functionDefinition;
    sysbvm_tuple_t dependentFunctionType;
    sysbvm_tuple_t argumentVectorSize;
    sysbvm_tuple_t captureVector;
    sysbvm_tuple_t valueVector;
} sysbvm_functionActivationEnvironment_t;

typedef struct sysbvm_analysisEnvironment_s
{
    sysbvm_analysisAndEvaluationEnvironment_t super;
    sysbvm_tuple_t hasBreakTarget;
    sysbvm_tuple_t hasContinueTarget;
} sysbvm_analysisEnvironment_t;

typedef struct sysbvm_functionAnalysisEnvironment_s
{
    sysbvm_analysisEnvironment_t super;
    sysbvm_tuple_t functionDefinition;
    sysbvm_tuple_t captureBindingTable;
    sysbvm_tuple_t captureBindingList;
    sysbvm_tuple_t argumentBindingList;
    sysbvm_tuple_t localBindingList;
    sysbvm_tuple_t pragmaList;
    sysbvm_tuple_t innerFunctionList;
    sysbvm_tuple_t primitiveName;
    sysbvm_tuple_t keepSourceDefinition;
    sysbvm_tuple_t returnTypeExpression;
} sysbvm_functionAnalysisEnvironment_t;

typedef struct sysbvm_localAnalysisEnvironment_s
{
    sysbvm_analysisEnvironment_t super;
} sysbvm_localAnalysisEnvironment_t;

typedef struct sysbvm_symbolBinding_s
{
    sysbvm_programEntity_t super;
    sysbvm_tuple_t sourcePosition;
    sysbvm_tuple_t type;
} sysbvm_symbolBinding_t;

typedef struct sysbvm_symbolAnalysisBinding_s
{
    sysbvm_symbolBinding_t super;
    sysbvm_tuple_t ownerFunction;
    sysbvm_tuple_t vectorIndex;
} sysbvm_symbolAnalysisBinding_t;

typedef struct sysbvm_symbolArgumentBinding_s
{
    sysbvm_symbolAnalysisBinding_t super;
} sysbvm_symbolArgumentBinding_t;

typedef struct sysbvm_symbolLocalBinding_s
{
    sysbvm_symbolAnalysisBinding_t super;
} sysbvm_symbolLocalBinding_t;

typedef struct sysbvm_symbolCaptureBinding_s
{
    sysbvm_symbolAnalysisBinding_t super;
    sysbvm_tuple_t capturedBinding;
} sysbvm_symbolCaptureBinding_t;

typedef struct sysbvm_symbolMacroValueBinding_s
{
    sysbvm_symbolBinding_t super;
    sysbvm_tuple_t expansion;
} sysbvm_symbolMacroValueBinding_t;

typedef struct sysbvm_symbolTupleSlotBinding_s
{
    sysbvm_symbolAnalysisBinding_t super;
    sysbvm_tuple_t tupleBinding;
    sysbvm_tuple_t typeSlot;
} sysbvm_symbolTupleSlotBinding_t;

typedef struct sysbvm_symbolValueBinding_s
{
    sysbvm_symbolBinding_t super;
    sysbvm_tuple_t value;
    sysbvm_tuple_t isMutable;
    sysbvm_tuple_t isExternal;
    sysbvm_tuple_t isThreadLocal;
    sysbvm_tuple_t nativeCodeDefinition;
    sysbvm_tuple_t virtualAddress;
} sysbvm_symbolValueBinding_t;

/**
 * Creates an analysis queue entry.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_analysisQueueEntry_create(sysbvm_context_t *context, sysbvm_tuple_t programEntity);

/**
 * Creates an analysis queue.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_analysisQueue_create(sysbvm_context_t *context);

/**
 * Enqueue the analysis of a program entity.
 */ 
SYSBVM_API void sysbvm_analysisQueue_enqueueProgramEntity(sysbvm_context_t *context, sysbvm_tuple_t queue, sysbvm_tuple_t programEntity);

/**
 * Get the default analysis queue.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_analysisQueue_getDefault(sysbvm_context_t *context);

/**
 * Wait for the pending analysis.
 */
SYSBVM_API void sysbvm_analysisQueue_waitPendingAnalysis(sysbvm_context_t *context, sysbvm_tuple_t queue);

/**
 * Creates an analyzer token.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_analyzerToken_create(sysbvm_context_t *context);

/**
 * Is the symbol binding a value?
 */
SYSBVM_API bool sysbvm_symbolBinding_isValue(sysbvm_context_t *context, sysbvm_tuple_t binding);

/**
 * Is the symbol binding a value?
 */
SYSBVM_API bool sysbvm_symbolBinding_isMacroValue(sysbvm_context_t *context, sysbvm_tuple_t binding);

/**
 * Is the symbol binding an analysis specific binding?
 */
SYSBVM_API bool sysbvm_symbolBinding_isAnalysisBinding(sysbvm_context_t *context, sysbvm_tuple_t binding);

/**
 * Gets the name from the symbol value binding.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_symbolBinding_getName(sysbvm_tuple_t binding)
{
    if(!sysbvm_tuple_isNonNullPointer(binding)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_symbolBinding_t*)binding)->super.name;
}

/**
 * Gets the value from the symbol value binding.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_symbolBinding_getType(sysbvm_tuple_t binding)
{
    if(!sysbvm_tuple_isNonNullPointer(binding)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_symbolBinding_t*)binding)->type;
}

/**
 * Creates a symbol argument binding.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_symbolArgumentBinding_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t type, sysbvm_tuple_t ownerFunction, size_t vectorIndex);

/**
 * Creates a symbol local binding.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_symbolLocalBinding_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t type, sysbvm_tuple_t ownerFunction, size_t vectorIndex);

/**
 * Creates a symbol capture binding.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_symbolCaptureBinding_create(sysbvm_context_t *context, sysbvm_tuple_t capturedBinding, sysbvm_tuple_t ownerFunction, size_t vectorIndex);

/**
 * Gets the value from the symbol value binding.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_symbolCaptureBinding_getSourceBinding(sysbvm_tuple_t binding)
{
    if(!sysbvm_tuple_isNonNullPointer(binding)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_symbolCaptureBinding_t*)binding)->capturedBinding;
}

/**
 * Creates a symbol macro value binding.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_symbolMacroValueBinding_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t expansion);

/**
 * Gets the expansion from the symbol macro value binding.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_symbolMacroValueBinding_getExpansion(sysbvm_tuple_t binding)
{
    if(!sysbvm_tuple_isNonNullPointer(binding) || sysbvm_tuple_getSizeInSlots(binding) < SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_symbolMacroValueBinding_t)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_symbolMacroValueBinding_t*)binding)->expansion;
}

/**
 * Creates a symbol value binding.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_symbolValueBinding_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t value);

/**
 * Creates a symbol value binding with the specified flags.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_symbolValueBinding_createWithFlags(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t value, bool isMutable, bool isExternal, bool isThreadLocal);

/**
 * Gets the value from the symbol value binding.
 */
SYSBVM_INLINE sysbvm_tuple_t sysbvm_symbolValueBinding_getValue(sysbvm_tuple_t binding)
{
    if(!sysbvm_tuple_isNonNullPointer(binding) || sysbvm_tuple_getSizeInSlots(binding) < SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_symbolValueBinding_t)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_symbolValueBinding_t*)binding)->value;
}

/**
 * Creates a tuple slot binding.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_symbolTupleSlotBinding_create(sysbvm_context_t *context, sysbvm_tuple_t tupleBinding, sysbvm_tuple_t typeSlot);

/**
 * Creates an environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_environment_create(sysbvm_context_t *context, sysbvm_tuple_t parent);

/**
 * Creates an environment with analysis queue.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_environment_createWithAnalysisQueue(sysbvm_context_t *context, sysbvm_tuple_t parent, sysbvm_tuple_t analysisQueue);

/**
 * Creates an environment that is used for simultaneous analysis and evaluation.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_analysisAndEvaluationEnvironment_create(sysbvm_context_t *context, sysbvm_tuple_t parent);

/**
 * Lookup the analysis queue
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_environment_lookupAnalysisQueue(sysbvm_context_t *context, sysbvm_tuple_t environment);

/**
 * Enqueues the pending analysis of a program entity.
 */
SYSBVM_API void sysbvm_environment_enqueuePendingAnalysis(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t programEntity);

/**
 * Waits for the pending analysis of program entities.
 */
SYSBVM_API void sysbvm_environment_waitPendingAnalysis(sysbvm_context_t *context, sysbvm_tuple_t environment);

/**
 * Creates a namespace.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_namespace_create(sysbvm_context_t *context, sysbvm_tuple_t parent);

/**
 * Creates an environment used for function action.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_functionActivationEnvironment_create(sysbvm_context_t *context, sysbvm_tuple_t parent, sysbvm_tuple_t function);

/**
 * Creates an environment used for dependent function type activation and evaluation.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_functionActivationEnvironment_createForDependentFunctionType(sysbvm_context_t *context, sysbvm_tuple_t parent, sysbvm_tuple_t dependentFunctionType);

/**
 * Creates an environment used for function definition analysis.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_functionAnalysisEnvironment_create(sysbvm_context_t *context, sysbvm_tuple_t parent, sysbvm_tuple_t functionDefinition);

/**
 * Creates an environment used for lexical scope analysis.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_localAnalysisEnvironment_create(sysbvm_context_t *context, sysbvm_tuple_t parent);

/**
 * Creates an environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_environment_getIntrinsicsBuiltInEnvironment(sysbvm_context_t *context);

/**
 * Creates an environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_environment_createDefaultForEvaluation(sysbvm_context_t *context);

/**
 * Creates an environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_environment_createDefaultForSourceCodeEvaluation(sysbvm_context_t *context, sysbvm_tuple_t sourceCode);

/**
 * Sets a new symbol binding in the environment.
 */ 
SYSBVM_API void sysbvm_environment_setNewBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t binding);

/**
 * Sets a symbol binding in the environment.
 */ 
SYSBVM_API void sysbvm_environment_setBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t binding);

/**
 * Sets a new symbol binding with value the environment.
 */ 
SYSBVM_API void sysbvm_environment_setNewSymbolBindingWithValue(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t symbol, sysbvm_tuple_t value);

/**
 * Sets a new symbol binding with value in the namespace.
 */ 
SYSBVM_API void sysbvm_namespace_setNewSymbolBindingWithValue(sysbvm_context_t *context, sysbvm_tuple_t nspace, sysbvm_tuple_t symbol, sysbvm_tuple_t value);

/**
 * Sets a new symbol binding with value in the environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_environment_setNewSymbolBindingWithValueAtSourcePosition(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t symbol, sysbvm_tuple_t value, sysbvm_tuple_t sourcePosition);

/**
 * Sets a new symbol binding with value and flags in the environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_environment_setNewSymbolBindingWithValueAndFlagsAtSourcePosition(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t symbol, sysbvm_tuple_t value, bool isMutable, bool isExternal, bool isThreadLocal, sysbvm_tuple_t sourcePosition);

/**
 * Sets a symbol binding with value in the environment.
 */ 
SYSBVM_API void sysbvm_environment_setSymbolBindingWithValue(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t symbol, sysbvm_tuple_t value);

/**
 * Looks a symbol recursively on an environment.
 */ 
SYSBVM_API bool sysbvm_environment_lookSymbolRecursively(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t symbol, sysbvm_tuple_t *outBinding);

/**
 * Evaluates the given symbol binding in this environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_environment_evaluateSymbolBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t binding);

/**
 * Sets the activation value for the given binding in this environment.
 */ 
SYSBVM_API void sysbvm_functionActivationEnvironment_setBindingActivationValue(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t binding, sysbvm_tuple_t value, sysbvm_tuple_t sourcePosition);

/**
 * Looks for the return target on an environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_environment_lookReturnTargetRecursively(sysbvm_context_t *context, sysbvm_tuple_t environment);

/**
 * Looks for the break target on an environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_environment_lookBreakTargetRecursively(sysbvm_context_t *context, sysbvm_tuple_t environment);

/**
 * Looks for the continue target on an environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_environment_lookContinueTargetRecursively(sysbvm_context_t *context, sysbvm_tuple_t environment);

/**
 * Sets the break target.
 */ 
SYSBVM_API void sysbvm_analysisAndEvaluationEnvironment_setBreakTarget(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t breakTarget);

/**
 * Sets the continue target.
 */ 
SYSBVM_API void sysbvm_analysisAndEvaluationEnvironment_setContinueTarget(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t continueTarget);

/**
 * Sets the return target.
 */ 
SYSBVM_API void sysbvm_analysisAndEvaluationEnvironment_setReturnTarget(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t returnTarget);

/**
 * Gets the expected type.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_analysisAndEvaluationEnvironment_getExpectedType(sysbvm_context_t *context, sysbvm_tuple_t environment);

/**
 * Sets the expected type.
 */ 
SYSBVM_API void sysbvm_analysisAndEvaluationEnvironment_setExpectedType(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t newExpectedType);

/**
 * Clears the unwinding record fields in the environment
 */ 

SYSBVM_API void sysbvm_analysisAndEvaluationEnvironment_clearUnwindingRecords(sysbvm_context_t *context, sysbvm_tuple_t environment);

/**
 * Clears the analyzer token.
 */ 
SYSBVM_API void sysbvm_analysisAndEvaluationEnvironment_clearAnalyzerToken(sysbvm_context_t *context, sysbvm_tuple_t environment);

/**
 * Ensures a valid analyzer token
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(sysbvm_context_t *context, sysbvm_tuple_t environment);

/**
 * Add an used tuple with named slots binding
 */ 
SYSBVM_API void sysbvm_analysisAndEvaluationEnvironment_addUseTupleWithNamedSlotsBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t binding);

/**
 * Is this an analysis and evaluation environment?
 */
SYSBVM_API bool sysbvm_environment_isAnalysisAndEvaluationEnvironment(sysbvm_context_t *context, sysbvm_tuple_t environment);

/**
 * Is this an analysis environment?
 */
SYSBVM_API bool sysbvm_environment_isAnalysisEnvironment(sysbvm_context_t *context, sysbvm_tuple_t environment);

/**
 * Is this an analysis environment?
 */
SYSBVM_API bool sysbvm_environment_isFunctionAnalysisEnvironment(sysbvm_context_t *context, sysbvm_tuple_t environment);

/**
 * Look recursively for the function analysis environment.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_environment_lookFunctionAnalysisEnvironmentRecursively(sysbvm_context_t *context, sysbvm_tuple_t environment);

/**
 * Looks a symbol recursively on an analysis environment.
 */ 
SYSBVM_API bool sysbvm_analysisEnvironment_lookSymbolRecursively(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t symbol, sysbvm_tuple_t *outBinding);

/**
 * Makes a new argument binding in the analysis environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_analysisEnvironment_setNewSymbolArgumentBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t type);

/**
 * Makes a new local binding in the analysis environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_analysisEnvironment_setNewSymbolLocalBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t type);

/**
 * Adds a pragma
 */ 
SYSBVM_API void sysbvm_analysisEnvironment_addPragma(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t pragma);

/**
 * Adds a closure
 */ 
SYSBVM_API void sysbvm_analysisEnvironment_addInnerFunction(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t innerFunction);

/**
 * Makes a new macro value binding in the analysis environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_environment_setNewMacroValueBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t expansion);

/**
 * Makes a new value binding in the analysis environment.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_analysisEnvironment_setNewValueBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t value);

#endif //SYSBVM_ENVIRONMENT_H
