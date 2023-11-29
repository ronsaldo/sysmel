#include "sysbvm/environment.h"
#include "sysbvm/array.h"
#include "sysbvm/orderedCollection.h"
#include "sysbvm/assert.h"
#include "sysbvm/association.h"
#include "sysbvm/dictionary.h"
#include "sysbvm/errors.h"
#include "sysbvm/pragma.h"
#include "sysbvm/sourceCode.h"
#include "sysbvm/string.h"
#include "sysbvm/stackFrame.h"
#include "sysbvm/function.h"
#include "sysbvm/type.h"
#include "internal/context.h"
#include <stdlib.h>

static inline bool sysbvm_symbolBinding_isValueQuick(sysbvm_context_t *context, sysbvm_tuple_t binding)
{
    if(!sysbvm_tuple_isNonNullPointer(binding)) return false;

    return SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(binding)->header.typePointer == context->roots.symbolValueBindingType;
}

SYSBVM_API sysbvm_tuple_t sysbvm_analysisQueueEntry_create(sysbvm_context_t *context, sysbvm_tuple_t programEntity)
{
    sysbvm_analysisQueueEntry_t *result = (sysbvm_analysisQueueEntry_t*)sysbvm_context_allocatePointerTuple(context, context->roots.analysisQueueEntryType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_analysisQueueEntry_t));
    result->programEntity = programEntity;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_analysisQueue_create(sysbvm_context_t *context)
{
    return (sysbvm_tuple_t)sysbvm_context_allocatePointerTuple(context, context->roots.analysisQueueType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_analysisQueue_t));
}

SYSBVM_API void sysbvm_analysisQueue_enqueueProgramEntity(sysbvm_context_t *context, sysbvm_tuple_t queue, sysbvm_tuple_t programEntity)
{
    if(!sysbvm_tuple_isNonNullPointer(queue))
        sysbvm_error("Expected a valid analysis queue.");

    sysbvm_analysisQueue_t *queueObject = (sysbvm_analysisQueue_t*)queue;
    sysbvm_tuple_t newEntry = sysbvm_analysisQueueEntry_create(context, programEntity);
    if(queueObject->lastEntry)
        ((sysbvm_analysisQueueEntry_t*)queueObject->lastEntry)->nextEntry = newEntry;
    else
        queueObject->firstEntry = newEntry;
    queueObject->lastEntry = newEntry;
}

SYSBVM_API sysbvm_tuple_t sysbvm_analysisQueue_getDefault(sysbvm_context_t *context)
{
    sysbvm_tuple_t *defaultQueue = SYSBVM_CAST_OOP_TO_OBJECT_TUPLE(context->roots.defaultAnalysisQueueValueBox)->pointers;
    if(!*defaultQueue)
        *defaultQueue = sysbvm_analysisQueue_create(context);
    return *defaultQueue;
}

SYSBVM_API void sysbvm_analysisQueue_waitPendingAnalysis(sysbvm_context_t *context, sysbvm_tuple_t queue)
{
    if(!sysbvm_tuple_isNonNullPointer(queue))
        sysbvm_error("Expected a valid analysis queue.");

    struct {
        sysbvm_analysisQueue_t *queue;
        sysbvm_analysisQueueEntry_t *entry;
    } gcFrame = {
        .queue = (sysbvm_analysisQueue_t*)queue,
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    while((gcFrame.entry = (sysbvm_analysisQueueEntry_t*)gcFrame.queue->firstEntry) != NULL)
    {
        gcFrame.queue->firstEntry = gcFrame.entry->nextEntry;
        if(!gcFrame.queue->firstEntry)
            gcFrame.queue->lastEntry = SYSBVM_NULL_TUPLE;
        sysbvm_tuple_send0(context, context->roots.ensureAnalysisSelector, gcFrame.entry->programEntity);
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
}

SYSBVM_API sysbvm_tuple_t sysbvm_analyzerToken_create(sysbvm_context_t *context)
{
    return (sysbvm_tuple_t)sysbvm_context_allocatePointerTuple(context, context->roots.objectType, SYSBVM_NULL_TUPLE);
}

SYSBVM_API bool sysbvm_symbolBinding_isValue(sysbvm_context_t *context, sysbvm_tuple_t binding)
{
    return sysbvm_tuple_isKindOf(context, binding, context->roots.symbolValueBindingType);
}

SYSBVM_API bool sysbvm_symbolBinding_isMacroValue(sysbvm_context_t *context, sysbvm_tuple_t binding)
{
    return sysbvm_tuple_isKindOf(context, binding, context->roots.symbolMacroValueBindingType);
}

SYSBVM_API bool sysbvm_symbolBinding_isAnalysisBinding(sysbvm_context_t *context, sysbvm_tuple_t binding)
{
    return sysbvm_tuple_isKindOf(context, binding, context->roots.symbolAnalysisBindingType);
}

SYSBVM_API bool sysbvm_symbolBinding_isTupleSlotBinding(sysbvm_context_t *context, sysbvm_tuple_t binding)
{
    return sysbvm_tuple_isKindOf(context, binding, context->roots.symbolTupleSlotBindingType);
}

SYSBVM_API sysbvm_tuple_t sysbvm_symbolArgumentBinding_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t type, sysbvm_tuple_t ownerFunction, size_t vectorIndex)
{
    sysbvm_symbolArgumentBinding_t *result = (sysbvm_symbolArgumentBinding_t*)sysbvm_context_allocatePointerTuple(context, context->roots.symbolArgumentBindingType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_symbolArgumentBinding_t));
    result->super.super.super.name = name;
    result->super.super.sourcePosition = sourcePosition;
    result->super.super.type = type;
    result->super.ownerFunction = ownerFunction;
    result->super.vectorIndex = sysbvm_tuple_size_encode(context, vectorIndex);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_symbolLocalBinding_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t type, sysbvm_tuple_t ownerFunction, size_t vectorIndex)
{
    sysbvm_symbolLocalBinding_t *result = (sysbvm_symbolLocalBinding_t*)sysbvm_context_allocatePointerTuple(context, context->roots.symbolLocalBindingType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_symbolLocalBinding_t));
    result->super.super.super.name = name;
    result->super.super.sourcePosition = sourcePosition;
    result->super.super.type = type;
    result->super.ownerFunction = ownerFunction;
    result->super.vectorIndex = sysbvm_tuple_size_encode(context, vectorIndex);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_symbolCaptureBinding_create(sysbvm_context_t *context, sysbvm_tuple_t capturedBinding, sysbvm_tuple_t ownerFunction, size_t vectorIndex)
{
    if(!sysbvm_tuple_isKindOf(context, capturedBinding, context->roots.symbolBindingType))
        sysbvm_error("Expected a symbol binding");

    sysbvm_symbolBinding_t *capturedBindingObject = (sysbvm_symbolBinding_t*)capturedBinding;

    sysbvm_symbolCaptureBinding_t *result = (sysbvm_symbolCaptureBinding_t*)sysbvm_context_allocatePointerTuple(context, context->roots.symbolCaptureBindingType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_symbolCaptureBinding_t));
    result->super.super.sourcePosition = capturedBindingObject->sourcePosition;
    result->super.super.super.name = capturedBindingObject->super.name;
    result->super.super.type = capturedBindingObject->type;
    result->super.ownerFunction = ownerFunction;
    result->super.vectorIndex = sysbvm_tuple_size_encode(context, vectorIndex);
    result->capturedBinding = capturedBinding;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_symbolMacroValueBinding_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t expansion)
{
    sysbvm_symbolMacroValueBinding_t *result = (sysbvm_symbolMacroValueBinding_t*)sysbvm_context_allocatePointerTuple(context, context->roots.symbolMacroValueBindingType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_symbolMacroValueBinding_t));
    result->super.super.name = name;
    result->super.sourcePosition = sourcePosition;
    result->super.type = sysbvm_tuple_getType(context, expansion);
    result->expansion = expansion;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_symbolValueBinding_createWithFlags(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t value, bool isMutable, bool isExternal, bool isThreadLocal)
{
    sysbvm_symbolValueBinding_t *result = (sysbvm_symbolValueBinding_t*)sysbvm_context_allocatePointerTuple(context, context->roots.symbolValueBindingType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_symbolValueBinding_t));
    result->super.super.name = name;
    result->super.sourcePosition = sourcePosition;
    result->super.type = sysbvm_tuple_getType(context, value);
    result->value = value;
    result->isMutable = sysbvm_tuple_boolean_encode(isMutable);
    result->isExternal = sysbvm_tuple_boolean_encode(isExternal);
    result->isThreadLocal = sysbvm_tuple_boolean_encode(isThreadLocal);
    result->virtualAddress = sysbvm_tuple_uintptr_encode(context, 0);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_symbolValueBinding_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t value)
{
    return sysbvm_symbolValueBinding_createWithFlags(context, sourcePosition, name, value, false, false, false);
}

SYSBVM_API sysbvm_tuple_t sysbvm_symbolTupleSlotBinding_create(sysbvm_context_t *context, sysbvm_tuple_t tupleBinding, sysbvm_tuple_t typeSlot)
{
    sysbvm_symbolTupleSlotBinding_t *result = (sysbvm_symbolTupleSlotBinding_t*)sysbvm_context_allocatePointerTuple(context, context->roots.symbolTupleSlotBindingType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_symbolTupleSlotBinding_t));
    result->super.super.type = sysbvm_typeSlot_getValidReferenceType(context, typeSlot);
    result->super.vectorIndex = sysbvm_tuple_size_encode(context, 0);
    result->tupleBinding = tupleBinding;
    result->typeSlot = typeSlot;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_environment_create(sysbvm_context_t *context, sysbvm_tuple_t parent)
{
    sysbvm_environment_t *result = (sysbvm_environment_t*)sysbvm_context_allocatePointerTuple(context, context->roots.environmentType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_environment_t));
    result->parent = parent;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_environment_createWithAnalysisQueue(sysbvm_context_t *context, sysbvm_tuple_t parent, sysbvm_tuple_t analysisQueue)
{
    sysbvm_environment_t *result = (sysbvm_environment_t*)sysbvm_context_allocatePointerTuple(context, context->roots.environmentType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_environment_t));
    result->parent = parent;
    result->analysisQueue = analysisQueue;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_analysisAndEvaluationEnvironment_create(sysbvm_context_t *context, sysbvm_tuple_t parent)
{
    sysbvm_analysisAndEvaluationEnvironment_t *result = (sysbvm_analysisAndEvaluationEnvironment_t*)sysbvm_context_allocatePointerTuple(context, context->roots.analysisAndEvaluationEnvironmentType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_analysisAndEvaluationEnvironment_t));
    result->super.parent = parent;

    // Inherit the analyzer token, if possible.
    if(parent && sysbvm_environment_isAnalysisAndEvaluationEnvironment(context, parent))
        result->analyzerToken = ((sysbvm_analysisAndEvaluationEnvironment_t*)parent)->analyzerToken;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_environment_lookupAnalysisQueue(sysbvm_context_t *context, sysbvm_tuple_t environment)
{
    if(!sysbvm_tuple_isNonNullPointer(environment))
        return SYSBVM_NULL_TUPLE;
    
    sysbvm_environment_t *environmentObject = (sysbvm_environment_t*)environment;
    if(environmentObject->analysisQueue)
        return environmentObject->analysisQueue;

    return sysbvm_environment_lookupAnalysisQueue(context, environmentObject->parent);
}

SYSBVM_API void sysbvm_environment_enqueuePendingAnalysis(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t programEntity)
{
    sysbvm_tuple_t analysisQueue = sysbvm_environment_lookupAnalysisQueue(context, environment);
    if(!analysisQueue)
        sysbvm_error("A delayed analysis queue is not available in this context.");

    sysbvm_analysisQueue_enqueueProgramEntity(context, analysisQueue, programEntity);
}

SYSBVM_API void sysbvm_environment_waitPendingAnalysis(sysbvm_context_t *context, sysbvm_tuple_t environment)
{
    sysbvm_tuple_t analysisQueue = sysbvm_environment_lookupAnalysisQueue(context, environment);
    if(!analysisQueue)
        sysbvm_error("A delayed analysis queue is not available in this context.");

    sysbvm_analysisQueue_waitPendingAnalysis(context, analysisQueue);
}

SYSBVM_API sysbvm_tuple_t sysbvm_namespace_create(sysbvm_context_t *context, sysbvm_tuple_t parent)
{
    sysbvm_namespace_t *result = (sysbvm_namespace_t*)sysbvm_context_allocatePointerTuple(context, context->roots.namespaceType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_namespace_t));
    result->parent = parent;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionActivationEnvironment_create(sysbvm_context_t *context, sysbvm_tuple_t parent, sysbvm_tuple_t function)
{
    if(!function || !sysbvm_tuple_isFunction(context, function))
        sysbvm_error("Expected a function");

    sysbvm_function_t *functionObject = (sysbvm_function_t*)function;
    if(!functionObject->definition)
        sysbvm_error("Expected a function with a definition.");

    sysbvm_functionDefinition_t *functionDefinitionObject = (sysbvm_functionDefinition_t*)functionObject->definition;
    if(!functionDefinitionObject->sourceAnalyzedDefinition)
        sysbvm_error("Expected an analyzed source definition");

    sysbvm_functionSourceAnalyzedDefinition_t *sourceAnalyzedDefinition = (sysbvm_functionSourceAnalyzedDefinition_t*)functionDefinitionObject->sourceAnalyzedDefinition;

    sysbvm_functionActivationEnvironment_t *result = (sysbvm_functionActivationEnvironment_t*)sysbvm_context_allocatePointerTuple(context, context->roots.functionActivationEnvironmentType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_functionActivationEnvironment_t));
    result->super.super.parent = parent;
    result->function = function;
    result->functionDefinition = functionObject->definition;
    result->captureVector = functionObject->captureVector;
    
    size_t argumentCount = sysbvm_array_getSize(sourceAnalyzedDefinition->arguments);
    size_t localCount = sysbvm_array_getSize(sourceAnalyzedDefinition->locals);
    result->argumentVectorSize = sysbvm_tuple_size_encode(context, argumentCount);
    result->valueVector = sysbvm_array_create(context, argumentCount + localCount);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionActivationEnvironment_createForDependentFunctionType(sysbvm_context_t *context, sysbvm_tuple_t parent, sysbvm_tuple_t dependentFunctionType)
{
    if(!dependentFunctionType || !sysbvm_tuple_isKindOf(context, dependentFunctionType, context->roots.dependentFunctionTypeType))
        sysbvm_error("Expected a dependent function type");

    sysbvm_dependentFunctionType_t *dependentFunctionTypeObject = (sysbvm_dependentFunctionType_t*)dependentFunctionType;
 
    sysbvm_functionActivationEnvironment_t *result = (sysbvm_functionActivationEnvironment_t*)sysbvm_context_allocatePointerTuple(context, context->roots.functionActivationEnvironmentType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_functionActivationEnvironment_t));
    result->super.super.parent = parent;
    result->dependentFunctionType = dependentFunctionType;
    result->captureVector = sysbvm_array_create(context, 0);
    
    size_t argumentCount = sysbvm_array_getSize(dependentFunctionTypeObject->argumentBindings);
    size_t localCount = sysbvm_array_getSize(dependentFunctionTypeObject->localBindings);
    result->argumentVectorSize = sysbvm_tuple_size_encode(context, argumentCount);
    result->valueVector = sysbvm_array_create(context, argumentCount + localCount);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionAnalysisEnvironment_create(sysbvm_context_t *context, sysbvm_tuple_t parent, sysbvm_tuple_t functionDefinition)
{
    sysbvm_functionAnalysisEnvironment_t *result = (sysbvm_functionAnalysisEnvironment_t*)sysbvm_context_allocatePointerTuple(context, context->roots.functionAnalysisEnvironmentType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_functionAnalysisEnvironment_t));
    result->super.super.super.parent = parent;
    result->functionDefinition = functionDefinition;
    result->captureBindingTable = sysbvm_identityDictionary_create(context);
    result->captureBindingList = sysbvm_orderedCollection_create(context);
    result->argumentBindingList = sysbvm_orderedCollection_create(context);
    result->localBindingList = sysbvm_orderedCollection_create(context);
    result->innerFunctionList = sysbvm_orderedCollection_create(context);
    result->pragmaList = sysbvm_orderedCollection_create(context);
    result->keepSourceDefinition = SYSBVM_FALSE_TUPLE;
    result->super.hasBreakTarget = SYSBVM_FALSE_TUPLE;
    result->super.hasContinueTarget = SYSBVM_FALSE_TUPLE;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_localAnalysisEnvironment_create(sysbvm_context_t *context, sysbvm_tuple_t parent)
{
    sysbvm_localAnalysisEnvironment_t *result = (sysbvm_localAnalysisEnvironment_t*)sysbvm_context_allocatePointerTuple(context, context->roots.localAnalysisEnvironmentType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_localAnalysisEnvironment_t));
    result->super.super.super.parent = parent;
    result->super.super.super.symbolTable = sysbvm_identityDictionary_create(context);
    result->super.hasBreakTarget = SYSBVM_FALSE_TUPLE;
    result->super.hasContinueTarget = SYSBVM_FALSE_TUPLE;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_environment_getIntrinsicsBuiltInEnvironment(sysbvm_context_t *context)
{
    return context->roots.globalNamespace;
}

SYSBVM_API sysbvm_tuple_t sysbvm_environment_createDefaultForEvaluation(sysbvm_context_t *context)
{
    return sysbvm_environment_createWithAnalysisQueue(context,
        sysbvm_environment_getIntrinsicsBuiltInEnvironment(context),
        sysbvm_analysisQueue_getDefault(context));
}

SYSBVM_API sysbvm_tuple_t sysbvm_environment_createDefaultForSourceCodeEvaluation(sysbvm_context_t *context, sysbvm_tuple_t sourceCode)
{
    sysbvm_tuple_t environment = sysbvm_environment_createDefaultForEvaluation(context);
    sysbvm_environment_setNewSymbolBindingWithValue(context, environment, sysbvm_symbol_internWithCString(context, "__SourceDirectory__"), sysbvm_sourceCode_getDirectory(sourceCode));
    sysbvm_environment_setNewSymbolBindingWithValue(context, environment, sysbvm_symbol_internWithCString(context, "__SourceName__"), sysbvm_sourceCode_getName(sourceCode));
    sysbvm_environment_setNewSymbolBindingWithValue(context, environment, sysbvm_symbol_internWithCString(context, "__SourceLanguage__"), sysbvm_sourceCode_getLanguage(sourceCode));
    return environment;
}

SYSBVM_API void sysbvm_environment_setBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t binding)
{
    if(!sysbvm_tuple_isNonNullPointer(environment))
        return;

    sysbvm_environment_t *environmentObject = (sysbvm_environment_t*)environment;
    if(!environmentObject->symbolTable)
        environmentObject->symbolTable = sysbvm_identityDictionary_create(context);
    sysbvm_identityDictionary_add(context, environmentObject->symbolTable, binding);
}

SYSBVM_API void sysbvm_environment_setNewBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t binding)
{
    if(!sysbvm_tuple_isNonNullPointer(environment))
        return;

    sysbvm_environment_t *environmentObject = (sysbvm_environment_t*)environment;
    sysbvm_tuple_t existentBinding = SYSBVM_NULL_TUPLE;
    sysbvm_tuple_t symbol = sysbvm_association_getKey(binding);

    if(environmentObject->symbolTable && sysbvm_identityDictionary_findAssociation(environmentObject->symbolTable, symbol, &existentBinding))
        sysbvm_error("Overriding existent symbol binding.");

    if(!environmentObject->symbolTable)
        environmentObject->symbolTable = sysbvm_identityDictionary_create(context);

    sysbvm_identityDictionary_add(context, environmentObject->symbolTable, binding);
}

SYSBVM_API void sysbvm_environment_setSymbolBindingWithValue(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t symbol, sysbvm_tuple_t value)
{
    if(!sysbvm_tuple_isNonNullPointer(environment))
        return;

    sysbvm_environment_t *environmentObject = (sysbvm_environment_t*)environment;
    sysbvm_tuple_t binding = sysbvm_symbolValueBinding_create(context, SYSBVM_NULL_TUPLE, symbol, value);
    sysbvm_identityDictionary_add(context, environmentObject->symbolTable, binding);
}

SYSBVM_API sysbvm_tuple_t sysbvm_environment_setNewSymbolBindingWithValueAtSourcePosition(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t symbol, sysbvm_tuple_t value, sysbvm_tuple_t sourcePosition)
{
    if(!sysbvm_tuple_isNonNullPointer(environment))
        sysbvm_error("Expected an environment.");

    sysbvm_tuple_t binding = sysbvm_symbolValueBinding_create(context, sourcePosition, symbol, value);
    sysbvm_environment_setNewBinding(context, environment, binding);
    return binding;
}

SYSBVM_API sysbvm_tuple_t sysbvm_environment_setNewSymbolBindingWithValueAndFlagsAtSourcePosition(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t symbol, sysbvm_tuple_t value, bool isMutable, bool isExternal, bool isThreadLocal, sysbvm_tuple_t sourcePosition)
{
    if(!sysbvm_tuple_isNonNullPointer(environment))
        sysbvm_error("Expected an environment.");

    sysbvm_tuple_t binding = sysbvm_symbolValueBinding_createWithFlags(context, sourcePosition, symbol, value, isMutable, isExternal, isThreadLocal);
    sysbvm_environment_setNewBinding(context, environment, binding);
    return binding;
}

SYSBVM_API void sysbvm_environment_setNewSymbolBindingWithValue(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t symbol, sysbvm_tuple_t value)
{
   sysbvm_environment_setNewSymbolBindingWithValueAtSourcePosition(context, environment, symbol, value, SYSBVM_NULL_TUPLE);
}

SYSBVM_API void sysbvm_namespace_setNewSymbolBindingWithValue(sysbvm_context_t *context, sysbvm_tuple_t nspace, sysbvm_tuple_t symbol, sysbvm_tuple_t value)
{
    if(!sysbvm_tuple_isNonNullPointer(nspace))
        sysbvm_error("Expected a namespace.");

    sysbvm_tuple_t binding = sysbvm_symbolValueBinding_create(context, SYSBVM_NULL_TUPLE, symbol, value);
    sysbvm_environment_setNewBinding(context, nspace, binding);

    if(sysbvm_tuple_isFunction(context, value))
        sysbvm_function_recordBindingWithOwnerAndName(context, value, nspace, symbol);
    else
        sysbvm_programEntity_recordBindingWithOwnerAndName(context, value, nspace, symbol);
}

SYSBVM_API bool sysbvm_environment_lookSymbolRecursively(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t symbol, sysbvm_tuple_t *outBinding)
{
    *outBinding = SYSBVM_NULL_TUPLE;
    if(!sysbvm_tuple_isNonNullPointer(environment))
        return false;

    sysbvm_environment_t *environmentObject = (sysbvm_environment_t*)environment;
    if(environmentObject->symbolTable && sysbvm_identityDictionary_findAssociation(environmentObject->symbolTable, symbol, outBinding))
        return true;

    return sysbvm_environment_lookSymbolRecursively(context, environmentObject->parent, symbol, outBinding);
}

SYSBVM_API sysbvm_tuple_t sysbvm_environment_evaluateSymbolBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t binding)
{
    if(sysbvm_tuple_getType(context, environment) == context->roots.functionActivationEnvironmentType)
    {
        sysbvm_functionActivationEnvironment_t *activationEnvironment = (sysbvm_functionActivationEnvironment_t*)environment;
        sysbvm_symbolAnalysisBinding_t *analysisBinding = (sysbvm_symbolAnalysisBinding_t*)binding;
        if(analysisBinding->ownerFunction == activationEnvironment->functionDefinition || activationEnvironment->dependentFunctionType)
        {
            size_t vectorIndex = sysbvm_tuple_size_decode(analysisBinding->vectorIndex);
            sysbvm_tuple_t bindingType = sysbvm_tuple_getType(context, binding);
            if(bindingType == context->roots.symbolArgumentBindingType)
                return sysbvm_array_at(activationEnvironment->valueVector, vectorIndex);
            else if(bindingType == context->roots.symbolLocalBindingType)
                return sysbvm_array_at(activationEnvironment->valueVector, sysbvm_tuple_size_decode(activationEnvironment->argumentVectorSize) + vectorIndex);
            else if(bindingType == context->roots.symbolCaptureBindingType)
                return sysbvm_array_at(activationEnvironment->captureVector, vectorIndex);
            abort();
        }
    }

    sysbvm_tuple_t foundBinding = SYSBVM_NULL_TUPLE;
    if(!sysbvm_environment_lookSymbolRecursively(context, environment, sysbvm_symbolBinding_getName(binding), &foundBinding))
        sysbvm_error("Failed to evaluate analyzed symbol binding.");

    if(!sysbvm_symbolBinding_isValueQuick(context, foundBinding))
        sysbvm_error("A value binding in the evaluation context is required.");

    return sysbvm_symbolValueBinding_getValue(foundBinding);
}

SYSBVM_API void sysbvm_functionActivationEnvironment_setBindingActivationValue(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t binding, sysbvm_tuple_t value, sysbvm_tuple_t sourcePosition)
{
    (void)sourcePosition;
    sysbvm_functionActivationEnvironment_t *activationEnvironment = (sysbvm_functionActivationEnvironment_t*)environment;
    sysbvm_symbolAnalysisBinding_t *analysisBinding = (sysbvm_symbolAnalysisBinding_t*)binding;
    if(analysisBinding->ownerFunction == activationEnvironment->functionDefinition || activationEnvironment->dependentFunctionType)
    {
        size_t vectorIndex = sysbvm_tuple_size_decode(analysisBinding->vectorIndex);
        sysbvm_tuple_t bindingType = sysbvm_tuple_getType(context, binding);
        if(bindingType == context->roots.symbolLocalBindingType)
        {
            sysbvm_array_atPut(activationEnvironment->valueVector, sysbvm_tuple_size_decode(activationEnvironment->argumentVectorSize) + vectorIndex, value);
            return;
        }
        else if(bindingType == context->roots.symbolArgumentBindingType)
        {
            sysbvm_array_atPut(activationEnvironment->valueVector, vectorIndex, value);
            return;
        }
    }
    abort();
}

SYSBVM_API sysbvm_tuple_t sysbvm_environment_lookReturnTargetRecursively(sysbvm_context_t *context, sysbvm_tuple_t environment)
{
    if(!sysbvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return SYSBVM_NULL_TUPLE;

    sysbvm_analysisAndEvaluationEnvironment_t *environmentObject = (sysbvm_analysisAndEvaluationEnvironment_t*)environment;
    if(environmentObject->returnTarget)
        return environmentObject->returnTarget;

    return sysbvm_environment_lookReturnTargetRecursively(context, environmentObject->super.parent);
}

SYSBVM_API sysbvm_tuple_t sysbvm_environment_lookBreakTargetRecursively(sysbvm_context_t *context, sysbvm_tuple_t environment)
{
    if(!sysbvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return SYSBVM_NULL_TUPLE;

    sysbvm_analysisAndEvaluationEnvironment_t *environmentObject = (sysbvm_analysisAndEvaluationEnvironment_t*)environment;
    if(environmentObject->breakTarget)
        return environmentObject->breakTarget;

    return sysbvm_environment_lookReturnTargetRecursively(context, environmentObject->super.parent);
}

SYSBVM_API sysbvm_tuple_t sysbvm_environment_lookContinueTargetRecursively(sysbvm_context_t *context, sysbvm_tuple_t environment)
{
    if(!sysbvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return SYSBVM_NULL_TUPLE;

    sysbvm_analysisAndEvaluationEnvironment_t *environmentObject = (sysbvm_analysisAndEvaluationEnvironment_t*)environment;
    if(environmentObject->continueTarget)
        return environmentObject->continueTarget;

    return sysbvm_environment_lookReturnTargetRecursively(context, environmentObject->super.parent);
}

SYSBVM_API void sysbvm_analysisAndEvaluationEnvironment_setBreakTarget(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t breakTarget)
{
    if(!sysbvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return;

    sysbvm_analysisAndEvaluationEnvironment_t *environmentObject = (sysbvm_analysisAndEvaluationEnvironment_t*)environment;
    environmentObject->breakTarget = breakTarget;
}

SYSBVM_API void sysbvm_analysisAndEvaluationEnvironment_setContinueTarget(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t continueTarget)
{
    if(!sysbvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return;

    sysbvm_analysisAndEvaluationEnvironment_t *environmentObject = (sysbvm_analysisAndEvaluationEnvironment_t*)environment;
    environmentObject->continueTarget = continueTarget;
}

SYSBVM_API void sysbvm_analysisAndEvaluationEnvironment_setReturnTarget(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t returnTarget)
{
    if(!sysbvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return;

    sysbvm_analysisAndEvaluationEnvironment_t *environmentObject = (sysbvm_analysisAndEvaluationEnvironment_t*)environment;
    environmentObject->returnTarget = returnTarget;
}

SYSBVM_API sysbvm_tuple_t sysbvm_analysisAndEvaluationEnvironment_getExpectedType(sysbvm_context_t *context, sysbvm_tuple_t environment)
{
    if(!sysbvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return SYSBVM_NULL_TUPLE;

    sysbvm_analysisAndEvaluationEnvironment_t *environmentObject = (sysbvm_analysisAndEvaluationEnvironment_t*)environment;
    return environmentObject->expectedType;
}

SYSBVM_API void sysbvm_analysisAndEvaluationEnvironment_setExpectedType(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t newExpectedType)
{
    if(!sysbvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return;

    sysbvm_analysisAndEvaluationEnvironment_t *environmentObject = (sysbvm_analysisAndEvaluationEnvironment_t*)environment;
    environmentObject->expectedType = newExpectedType;
}

SYSBVM_API void sysbvm_analysisAndEvaluationEnvironment_clearUnwindingRecords(sysbvm_context_t *context, sysbvm_tuple_t environment)
{
    if(!sysbvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return;

    sysbvm_analysisAndEvaluationEnvironment_t *environmentObject = (sysbvm_analysisAndEvaluationEnvironment_t*)environment;
    environmentObject->breakTarget = SYSBVM_NULL_TUPLE;
    environmentObject->continueTarget = SYSBVM_NULL_TUPLE;
    environmentObject->returnTarget = SYSBVM_NULL_TUPLE;
}

SYSBVM_API void sysbvm_analysisAndEvaluationEnvironment_clearAnalyzerToken(sysbvm_context_t *context, sysbvm_tuple_t environment)
{
    if(!sysbvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        return;

    sysbvm_analysisAndEvaluationEnvironment_t *environmentObject = (sysbvm_analysisAndEvaluationEnvironment_t*)environment;
    environmentObject->analyzerToken = SYSBVM_NULL_TUPLE;
}

SYSBVM_API sysbvm_tuple_t sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(sysbvm_context_t *context, sysbvm_tuple_t environment)
{
    if(!sysbvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        sysbvm_error("Expected an analysis and evaluation environment.");

    sysbvm_analysisAndEvaluationEnvironment_t *environmentObject = (sysbvm_analysisAndEvaluationEnvironment_t*)environment;
    if(!environmentObject->analyzerToken)
        environmentObject->analyzerToken = sysbvm_analyzerToken_create(context);
    return environmentObject->analyzerToken;
}

SYSBVM_API void sysbvm_analysisAndEvaluationEnvironment_addUseTupleWithNamedSlotsBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t binding)
{
    if(!sysbvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
        sysbvm_error("Expected an analysis and evaluation environment.");

    sysbvm_analysisAndEvaluationEnvironment_t *environmentObject = (sysbvm_analysisAndEvaluationEnvironment_t*)environment;
    if(!environmentObject->usedTuplesWithNamedSlots)
        environmentObject->usedTuplesWithNamedSlots = sysbvm_orderedCollection_create(context);
    sysbvm_orderedCollection_add(context, environmentObject->usedTuplesWithNamedSlots, binding);
}

SYSBVM_API bool sysbvm_environment_isAnalysisAndEvaluationEnvironment(sysbvm_context_t *context, sysbvm_tuple_t environment)
{
    return sysbvm_tuple_isKindOf(context, environment, context->roots.analysisAndEvaluationEnvironmentType);
}

SYSBVM_API bool sysbvm_environment_isAnalysisEnvironment(sysbvm_context_t *context, sysbvm_tuple_t environment)
{
    return sysbvm_tuple_isKindOf(context, environment, context->roots.analysisEnvironmentType);
}

SYSBVM_API bool sysbvm_environment_isFunctionAnalysisEnvironment(sysbvm_context_t *context, sysbvm_tuple_t environment)
{
    return sysbvm_tuple_isKindOf(context, environment, context->roots.functionAnalysisEnvironmentType);
}

SYSBVM_API sysbvm_tuple_t sysbvm_environment_lookFunctionAnalysisEnvironmentRecursively(sysbvm_context_t *context, sysbvm_tuple_t environment)
{
    if(!environment) return SYSBVM_NULL_TUPLE;
    if(sysbvm_environment_isFunctionAnalysisEnvironment(context, environment)) return environment;
    return sysbvm_environment_lookFunctionAnalysisEnvironmentRecursively(context, ((sysbvm_environment_t*)environment)->parent);
}

SYSBVM_API sysbvm_tuple_t sysbvm_functionAnalysisEnvironment_getOrCreateCaptureFor(sysbvm_context_t *context, sysbvm_functionAnalysisEnvironment_t *environment, sysbvm_tuple_t sourceBinding)
{
    sysbvm_tuple_t capturedBinding;
    sysbvm_symbolBinding_t *sourceBindingObject = (sysbvm_symbolBinding_t*)sourceBinding;
    sysbvm_tuple_t capturedBindingKey = sourceBindingObject->super.name ? sourceBindingObject->super.name : sourceBinding;
    if(sysbvm_identityDictionary_find(environment->captureBindingTable, capturedBindingKey, &capturedBinding))
        return capturedBinding;

    capturedBinding = sysbvm_symbolCaptureBinding_create(context, sourceBinding, environment->functionDefinition, sysbvm_orderedCollection_getSize(environment->captureBindingList));
    sysbvm_identityDictionary_atPut(context, environment->captureBindingTable, capturedBindingKey, capturedBinding);
    sysbvm_orderedCollection_add(context, environment->captureBindingList, capturedBinding);
    return capturedBinding;
}

SYSBVM_API bool sysbvm_analysisEnvironment_lookSymbolRecursively(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t symbol, sysbvm_tuple_t *outBinding)
{
    *outBinding = SYSBVM_NULL_TUPLE;
    if(!sysbvm_tuple_isNonNullPointer(environment))
        return false;

    sysbvm_environment_t *environmentObject = (sysbvm_environment_t*)environment;
    if(environmentObject->symbolTable && sysbvm_identityDictionary_findAssociation(environmentObject->symbolTable, symbol, outBinding))
        return true;

    // Look for the used symbols.
    if(sysbvm_environment_isAnalysisAndEvaluationEnvironment(context, environment))
    {
        sysbvm_analysisAndEvaluationEnvironment_t *analyzeAndEvalEnvironment = (sysbvm_analysisAndEvaluationEnvironment_t*)environment;
        if(analyzeAndEvalEnvironment->usedTuplesWithNamedSlots)
        {
            size_t usedObjectCount = sysbvm_orderedCollection_getSize(analyzeAndEvalEnvironment->usedTuplesWithNamedSlots);
            for(size_t i = 0; i < usedObjectCount; ++i)
            {
                sysbvm_tuple_t binding = sysbvm_orderedCollection_at(analyzeAndEvalEnvironment->usedTuplesWithNamedSlots, i);
                sysbvm_tuple_t bindingType = sysbvm_symbolBinding_getType(binding);
                sysbvm_tuple_t foundSlot = sysbvm_type_lookupSlot(context, bindingType, symbol);
                if(foundSlot)
                {
                    *outBinding = sysbvm_symbolTupleSlotBinding_create(context, binding, foundSlot);
                    return true;
                }
            }
        }
    }

    // If the symbol is not found on the local table, we might need to capture it.
    if(sysbvm_environment_isFunctionAnalysisEnvironment(context, environment))
    {
        // Find a previously captured binding.
        sysbvm_functionAnalysisEnvironment_t *functionAnalysisEnvironment = (sysbvm_functionAnalysisEnvironment_t*)environment;
        if(sysbvm_identityDictionary_find(functionAnalysisEnvironment->captureBindingTable, symbol, outBinding))
            return true;

        sysbvm_tuple_t parentSymbol = SYSBVM_NULL_TUPLE;
        if(!sysbvm_analysisEnvironment_lookSymbolRecursively(context, environmentObject->parent, symbol, &parentSymbol))
            return false;

        // Should we capture the symbol?
        if(sysbvm_symbolBinding_isAnalysisBinding(context, parentSymbol))
        {
            // Create the capture and store it.
            if(sysbvm_symbolBinding_isTupleSlotBinding(context, parentSymbol))
            {
                sysbvm_symbolTupleSlotBinding_t *sourceTupleSlotBinding = (sysbvm_symbolTupleSlotBinding_t*)parentSymbol;
                sysbvm_tuple_t captureBinding = sysbvm_functionAnalysisEnvironment_getOrCreateCaptureFor(context, functionAnalysisEnvironment, sourceTupleSlotBinding->tupleBinding);
                *outBinding = sysbvm_symbolTupleSlotBinding_create(context, captureBinding, sourceTupleSlotBinding->typeSlot);
                return true;
            }
            else
            {
                sysbvm_tuple_t captureBinding = sysbvm_functionAnalysisEnvironment_getOrCreateCaptureFor(context, functionAnalysisEnvironment, parentSymbol);
                *outBinding = captureBinding;
                return true;
            }
        }

        *outBinding = parentSymbol;
        return true;
    }

    return sysbvm_analysisEnvironment_lookSymbolRecursively(context, environmentObject->parent, symbol, outBinding);
}

SYSBVM_API sysbvm_tuple_t sysbvm_analysisEnvironment_setNewSymbolArgumentBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t type)
{
    sysbvm_tuple_t functionAnalysisEnvironment = sysbvm_environment_lookFunctionAnalysisEnvironmentRecursively(context, environment);
    if(!functionAnalysisEnvironment)
        sysbvm_error("A function analysis environment is required here.");

    sysbvm_functionAnalysisEnvironment_t *functionAnalysisEnvironmentObject = (sysbvm_functionAnalysisEnvironment_t*)functionAnalysisEnvironment;
    sysbvm_tuple_t binding = sysbvm_symbolArgumentBinding_create(context, sourcePosition, name, type, functionAnalysisEnvironmentObject->functionDefinition, sysbvm_orderedCollection_getSize(functionAnalysisEnvironmentObject->argumentBindingList));
    if(name)
        sysbvm_environment_setNewBinding(context, environment, binding);
    sysbvm_orderedCollection_add(context, functionAnalysisEnvironmentObject->argumentBindingList, binding);
    return binding;
}

SYSBVM_API sysbvm_tuple_t sysbvm_analysisEnvironment_setNewSymbolLocalBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t type)
{
    sysbvm_tuple_t functionAnalysisEnvironment = sysbvm_environment_lookFunctionAnalysisEnvironmentRecursively(context, environment);
    if(!functionAnalysisEnvironment)
        sysbvm_error("A function analysis environment is required here.");

    sysbvm_functionAnalysisEnvironment_t *functionAnalysisEnvironmentObject = (sysbvm_functionAnalysisEnvironment_t*)functionAnalysisEnvironment;
    sysbvm_tuple_t binding = sysbvm_symbolLocalBinding_create(context, sourcePosition, name, type, functionAnalysisEnvironmentObject->functionDefinition, sysbvm_orderedCollection_getSize(functionAnalysisEnvironmentObject->localBindingList));
    if(name)
        sysbvm_environment_setNewBinding(context, environment, binding);
    sysbvm_orderedCollection_add(context, functionAnalysisEnvironmentObject->localBindingList, binding);
    return binding;
}

SYSBVM_API void sysbvm_analysisEnvironment_addPragma(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t pragma)
{
    sysbvm_tuple_t functionAnalysisEnvironment = sysbvm_environment_lookFunctionAnalysisEnvironmentRecursively(context, environment);
    if(!functionAnalysisEnvironment)
        sysbvm_error("A function analysis environment is required here.");

    sysbvm_functionAnalysisEnvironment_t *functionAnalysisEnvironmentObject = (sysbvm_functionAnalysisEnvironment_t*)functionAnalysisEnvironment;
    sysbvm_pragma_t *pragmaObject = (sysbvm_pragma_t*)pragma;
    sysbvm_orderedCollection_add(context, functionAnalysisEnvironmentObject->pragmaList, pragma);

    if(pragmaObject->selector == context->roots.primitiveNamedSelector && !functionAnalysisEnvironmentObject->primitiveName)
        functionAnalysisEnvironmentObject->primitiveName = sysbvm_array_at(pragmaObject->arguments, 0);
    else if(pragmaObject->selector == context->roots.keepSourceDefinitionSelector)
        functionAnalysisEnvironmentObject->keepSourceDefinition = SYSBVM_TRUE_TUPLE;
}

SYSBVM_API void sysbvm_analysisEnvironment_addInnerFunction(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t innerFunction)
{
    sysbvm_tuple_t functionAnalysisEnvironment = sysbvm_environment_lookFunctionAnalysisEnvironmentRecursively(context, environment);
    if(!functionAnalysisEnvironment)
        sysbvm_error("A function analysis environment is required here.");

    sysbvm_functionAnalysisEnvironment_t *functionAnalysisEnvironmentObject = (sysbvm_functionAnalysisEnvironment_t*)functionAnalysisEnvironment;
    sysbvm_functionDefinition_t *innerFunctionDefinitionObject = (sysbvm_functionDefinition_t*)innerFunction;
    innerFunctionDefinitionObject->super.owner = functionAnalysisEnvironmentObject->functionDefinition;
    sysbvm_orderedCollection_add(context, functionAnalysisEnvironmentObject->innerFunctionList, innerFunction);
}

SYSBVM_API sysbvm_tuple_t sysbvm_environment_setNewMacroValueBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t expansion)
{
    sysbvm_tuple_t binding = sysbvm_symbolMacroValueBinding_create(context, sourcePosition, name, expansion);
    sysbvm_environment_setNewBinding(context, environment, binding);
    return binding;
}

SYSBVM_API sysbvm_tuple_t sysbvm_analysisEnvironment_setNewValueBinding(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t name, sysbvm_tuple_t value)
{
    sysbvm_tuple_t binding = sysbvm_symbolValueBinding_create(context, sourcePosition, name, value);
    if(name)
        sysbvm_environment_setNewBinding(context, environment, binding);
    return binding;
}

static sysbvm_tuple_t sysbvm_analysisQueue_primitive_waitPendingAnalysis(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_analysisQueue_waitPendingAnalysis(context, arguments[0]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_environment_primitive_setNewBinding(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_environment_setNewBinding(context, arguments[0], arguments[1]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_environment_primitive_setBinding(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_environment_setBinding(context, arguments[0], arguments[1]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_environment_primitive_setNewSymbolBindingWithValue(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_environment_setNewSymbolBindingWithValue(context, arguments[0], arguments[1], arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_environment_primitive_setSymbolBindingWithValue(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_environment_setSymbolBindingWithValue(context, arguments[0], arguments[1], arguments[2]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_environment_primitive_enqueuePendingAnalysis(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_environment_enqueuePendingAnalysis(context, arguments[0], arguments[1]);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_environment_primitive_defaultForEvaluation(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    (void)arguments;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_environment_createDefaultForEvaluation(context);;
}

void sysbvm_environment_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_analysisQueue_primitive_waitPendingAnalysis, "AnalysisQueue::waitPendingAnalysis");
    sysbvm_primitiveTable_registerFunction(sysbvm_environment_primitive_enqueuePendingAnalysis, "Environment::enqueuePendingAnalysis:");
    sysbvm_primitiveTable_registerFunction(sysbvm_environment_primitive_defaultForEvaluation, "Environment::defaultForEvaluation");

    sysbvm_primitiveTable_registerFunction(sysbvm_environment_primitive_setNewBinding, "Environment::setNewBinding:");
    sysbvm_primitiveTable_registerFunction(sysbvm_environment_primitive_setBinding, "Environment::setBinding:");
    sysbvm_primitiveTable_registerFunction(sysbvm_environment_primitive_setNewSymbolBindingWithValue, "Environment::setNewSymbol:bindingWithValue:");
    sysbvm_primitiveTable_registerFunction(sysbvm_environment_primitive_setSymbolBindingWithValue, "Environment::setSymbol:bindingWithValue:");
}

void sysbvm_environment_setupPrimitives(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.analysisQueueType, "waitPendingAnalysis", 1, SYSBVM_FUNCTION_FLAGS_NONE, NULL, sysbvm_analysisQueue_primitive_waitPendingAnalysis);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.environmentType, "enqueuePendingAnalysis:", 2, SYSBVM_FUNCTION_FLAGS_NONE, NULL, sysbvm_environment_primitive_enqueuePendingAnalysis);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, sysbvm_tuple_getType(context, context->roots.environmentType), "defaultForEvaluation", 1, SYSBVM_FUNCTION_FLAGS_NONE, NULL, sysbvm_environment_primitive_defaultForEvaluation);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.environmentType, "setNewBinding:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_environment_primitive_setNewBinding);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.environmentType, "setBinding:", 2, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_environment_primitive_setBinding);
    
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.environmentType, "setNewSymbol:bindingWithValue:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_environment_primitive_setNewSymbolBindingWithValue);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.environmentType, "setSymbol:bindingWithValue:", 3, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_environment_primitive_setSymbolBindingWithValue);
}