#ifndef SYSBVM_INTERNAL_CONTEXT_H
#define SYSBVM_INTERNAL_CONTEXT_H

#pragma once

#include "sysbvm/context.h"
#include "sysbvm/pic.h"
#include "heap.h"
#include "sysbvm/dynarray.h"

#define GLOBAL_LOOKUP_CACHE_ENTRY_COUNT 256
#define PIC_ENTRY_COUNT 16

typedef struct sysbvm_globalLookupCacheEntry_s
{
    sysbvm_tuple_t type;
    sysbvm_tuple_t selector;
    sysbvm_tuple_t method;
}sysbvm_globalLookupCacheEntry_t;

typedef struct sysbvm_context_roots_s
{
    sysbvm_tuple_t immediateTypeTable[SYSBVM_TUPLE_TAG_COUNT];
    sysbvm_tuple_t immediateTrivialTypeTable[SYSBVM_TUPLE_IMMEDIATE_TRIVIAL_COUNT];
    sysbvm_tuple_t internedSymbolSet;

    sysbvm_tuple_t sessionToken;

    sysbvm_tuple_t identityEqualsFunction;
    sysbvm_tuple_t identityNotEqualsFunction;
    sysbvm_tuple_t identityHashFunction;
    sysbvm_tuple_t stringEqualsFunction;
    sysbvm_tuple_t stringHashFunction;

    sysbvm_tuple_t equalsSelector;
    sysbvm_tuple_t hashSelector;
    sysbvm_tuple_t asStringSelector;
    sysbvm_tuple_t printStringSelector;
    sysbvm_tuple_t doesNotUnderstandSelector;

    sysbvm_tuple_t loadFromAtOffsetWithTypeSelector;
    sysbvm_tuple_t storeInAtOffsetWithTypeSelector;

    sysbvm_tuple_t assignmentSelector;
    sysbvm_tuple_t underscoreSelector;
    sysbvm_tuple_t addressSelector;
    sysbvm_tuple_t plusSelector;
    sysbvm_tuple_t subscriptSelector;
    sysbvm_tuple_t loadSelector;
    sysbvm_tuple_t storeSelector;
    sysbvm_tuple_t refLoadSelector;
    sysbvm_tuple_t refStoreSelector;
    sysbvm_tuple_t tempRefAsRefSelector;

    sysbvm_tuple_t pointerLikeLoadPrimitive;
    sysbvm_tuple_t pointerLikeStorePrimitive;
    sysbvm_tuple_t pointerLikeReinterpretCast;
    sysbvm_tuple_t pointerLikeElementAt;
    sysbvm_tuple_t pointerTypeTemplate;
    sysbvm_tuple_t referenceTypeTemplate;
    sysbvm_tuple_t temporaryReferenceTypeTemplate;
    sysbvm_tuple_t simpleFunctionTypeTemplate;
    sysbvm_tuple_t sequenceTupleTypeTemplate;
    
    sysbvm_tuple_t anyValueToVoidPrimitive;
    sysbvm_tuple_t anyValueToVoidPrimitiveName;
    sysbvm_tuple_t pointerLikeLoadPrimitiveName;
    sysbvm_tuple_t pointerLikeStorePrimitiveName;

    sysbvm_tuple_t applyWithoutArgumentsSelector;
    sysbvm_tuple_t applyWithArgumentsSelector;

    sysbvm_tuple_t primitiveNamedSelector;
    sysbvm_tuple_t keepSourceDefinitionSelector;

    sysbvm_tuple_t astNodeAnalysisSelector;
    sysbvm_tuple_t astNodeEvaluationSelector;
    sysbvm_tuple_t astNodeAnalysisAndEvaluationSelector;
    sysbvm_tuple_t astNodeValidateThenAnalyzeAndEvaluateWithEnvironmentSelector;
    sysbvm_tuple_t astNodeCompileIntoBytecodeSelector;
    sysbvm_tuple_t ensureAnalysisSelector;

    sysbvm_tuple_t analyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentSelector;
    sysbvm_tuple_t analyzeMessageSendNodeWithEnvironmentSelector;
    sysbvm_tuple_t analyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentSelector;
    sysbvm_tuple_t analyzeMessageChainNodeWithEnvironmentSelector;
    sysbvm_tuple_t analyzeAndEvaluateConcreteMetaValueWithEnvironmentSelector;
    sysbvm_tuple_t analyzeConcreteMetaValueWithEnvironmentSelector;
    sysbvm_tuple_t analyzeAndEvaluateUnexpandedApplicationNodeOfWithEnvironmentSelector;
    sysbvm_tuple_t analyzeUnexpandedApplicationNodeWithEnvironmentSelector;

    sysbvm_tuple_t coerceValueSelector;
    sysbvm_tuple_t coerceASTNodeWithEnvironmentSelector;
    sysbvm_tuple_t analyzeAndTypeCheckFunctionApplicationNodeWithEnvironmentSelector;
    sysbvm_tuple_t analyzeAndTypeCheckSolvedMessageSendNodeWithEnvironmentSelector;
    sysbvm_tuple_t getOrCreateDependentApplicationValueForNodeSelector;
    sysbvm_tuple_t defaultValueSelector;

    sysbvm_tuple_t untypedType;
    sysbvm_tuple_t anyValueType;
    sysbvm_tuple_t anyReferenceType;
    sysbvm_tuple_t anyTemporaryReferenceType;
    sysbvm_tuple_t anyPointerType;

    sysbvm_tuple_t anySequenceTupleType;
    sysbvm_tuple_t sequenceTupleType;

    sysbvm_tuple_t arrayType;
    sysbvm_tuple_t arraySliceType;
    sysbvm_tuple_t orderedCollectionType;
    sysbvm_tuple_t arrayedCollectionType;
    sysbvm_tuple_t associationType;
    sysbvm_tuple_t addressSpaceType;
    sysbvm_tuple_t booleanType;
    sysbvm_tuple_t byteArrayType;
    sysbvm_tuple_t byteStreamType;
    sysbvm_tuple_t classType;
    sysbvm_tuple_t collectionType;
    sysbvm_tuple_t dictionaryType;
    sysbvm_tuple_t analysisAndEvaluationEnvironmentType;
    sysbvm_tuple_t analysisEnvironmentType;
    sysbvm_tuple_t analysisQueueType;
    sysbvm_tuple_t analysisQueueEntryType;
    sysbvm_tuple_t environmentType;
    sysbvm_tuple_t falseType;
    sysbvm_tuple_t functionActivationEnvironmentType;
    sysbvm_tuple_t functionAnalysisEnvironmentType;
    sysbvm_tuple_t functionType;
    sysbvm_tuple_t functionBytecodeType;
    sysbvm_tuple_t functionDefinitionType;
    sysbvm_tuple_t functionSourceDefinitionType;
    sysbvm_tuple_t functionSourceAnalyzedDefinitionType;
    sysbvm_tuple_t functionNativeCodeType;
    sysbvm_tuple_t dependentFunctionTypeType;
    sysbvm_tuple_t simpleFunctionTypeType;
    sysbvm_tuple_t functionTypeType;
    sysbvm_tuple_t gcLayoutBuilderType;
    sysbvm_tuple_t gcLayoutType;
    sysbvm_tuple_t generatedSymbolType;
    sysbvm_tuple_t genericAddressSpaceType;
    sysbvm_tuple_t memberAddressSpaceType;
    sysbvm_tuple_t hashedCollectionType;
    sysbvm_tuple_t hashtableEmptyType;
    sysbvm_tuple_t identityDictionaryType;
    sysbvm_tuple_t identitySetType;
    sysbvm_tuple_t integerArrayType;
    sysbvm_tuple_t integerType;
    sysbvm_tuple_t largeIntegerType;
    sysbvm_tuple_t largePositiveIntegerType;
    sysbvm_tuple_t largeNegativeIntegerType;
    sysbvm_tuple_t localAnalysisEnvironmentType;
    sysbvm_tuple_t lookupKeyType;
    sysbvm_tuple_t macroContextType;
    sysbvm_tuple_t messageType;
    sysbvm_tuple_t metaclassType;
    sysbvm_tuple_t methodDictionaryType;
    sysbvm_tuple_t namespaceType;
    sysbvm_tuple_t nativeCodeType;
    sysbvm_tuple_t nativeCodeBindingLocationType;
    sysbvm_tuple_t nativeCodeCallFrameInformation;
    sysbvm_tuple_t nativeCodeLocationListType;
    sysbvm_tuple_t nativeCodeProgramEntityType;
    sysbvm_tuple_t nativeCodeSectionType;
    sysbvm_tuple_t nativeCodeStackMapType;
    sysbvm_tuple_t nativeCodeSymbolTableType;
    sysbvm_tuple_t nativeCodeSymbolType;    
    sysbvm_tuple_t nativeCodeRelocationTableType;
    sysbvm_tuple_t objectType;
    sysbvm_tuple_t orderedOffsetTableType;
    sysbvm_tuple_t orderedOffsetTableBuilderType;
    sysbvm_tuple_t pendingMemoizationValueType;
    sysbvm_tuple_t pendingDefinitionFragmentType;
    sysbvm_tuple_t pendingDefinitionFragmentsType;
    sysbvm_tuple_t packageType;
    sysbvm_tuple_t pointerLikeType;
    sysbvm_tuple_t pointerType;
    sysbvm_tuple_t pragmaType;
    sysbvm_tuple_t primitiveValueType;
    sysbvm_tuple_t programEntityType;
    sysbvm_tuple_t referenceLikeType;
    sysbvm_tuple_t referenceType;
    sysbvm_tuple_t sequenceableCollectionType;
    sysbvm_tuple_t setType;
    sysbvm_tuple_t streamType;
    sysbvm_tuple_t stringType;
    sysbvm_tuple_t stringStreamType;
    sysbvm_tuple_t stringSymbolType;
    sysbvm_tuple_t structureType;
    sysbvm_tuple_t smallIntegerType;
    sysbvm_tuple_t symbolType;
    sysbvm_tuple_t symbolBindingType;
    sysbvm_tuple_t symbolAnalysisBindingType;
    sysbvm_tuple_t symbolArgumentBindingType;
    sysbvm_tuple_t symbolCaptureBindingType;
    sysbvm_tuple_t symbolTupleSlotBindingType;
    sysbvm_tuple_t symbolLocalBindingType;
    sysbvm_tuple_t symbolMacroValueBindingType;
    sysbvm_tuple_t symbolValueBindingType;
    sysbvm_tuple_t symbolValueBindingNativeCodeDefinitionType;
    sysbvm_tuple_t tombstoneType;
    sysbvm_tuple_t temporaryReferenceType;
    sysbvm_tuple_t trueType;
    sysbvm_tuple_t typeType;
    sysbvm_tuple_t metatypeType;
    sysbvm_tuple_t typeSlotType;
    sysbvm_tuple_t undefinedObjectType;
    sysbvm_tuple_t valueBoxType;
    sysbvm_tuple_t variableValueBoxType;
    sysbvm_tuple_t valueType;
    sysbvm_tuple_t valueMetatypeType;
    sysbvm_tuple_t voidType;
    sysbvm_tuple_t virtualTableLayoutType;
    sysbvm_tuple_t virtualTableType;
    sysbvm_tuple_t weakArrayType;
    sysbvm_tuple_t weakOrderedCollectionType;
    sysbvm_tuple_t weakSetType;
    sysbvm_tuple_t weakIdentitySetType;
    sysbvm_tuple_t weakKeyDictionaryType;
    sysbvm_tuple_t weakValueAssociationType;
    sysbvm_tuple_t weakValueDictionaryType;
    sysbvm_tuple_t wordArrayType;

    sysbvm_tuple_t controlFlowEscapeType;
    sysbvm_tuple_t controlFlowBreakType;
    sysbvm_tuple_t controlFlowContinueType;
    sysbvm_tuple_t controlFlowReturnType;
    sysbvm_tuple_t noReturnType;
    sysbvm_tuple_t unwindsType;
    sysbvm_tuple_t typeInferenceType;
    sysbvm_tuple_t decayedTypeInferenceType;
    sysbvm_tuple_t receiverTypeInferenceType;
    sysbvm_tuple_t directTypeInferenceType;

    sysbvm_tuple_t primitiveNumberType;
    sysbvm_tuple_t primitiveIntegerType;
    sysbvm_tuple_t primitiveCharacterType;
    sysbvm_tuple_t primitiveUnsignedIntegerType;
    sysbvm_tuple_t primitiveSignedIntegerType;
    sysbvm_tuple_t primitiveFloatType;

    sysbvm_tuple_t char8Type;
    sysbvm_tuple_t uint8Type;
    sysbvm_tuple_t int8Type;

    sysbvm_tuple_t char16Type;
    sysbvm_tuple_t uint16Type;
    sysbvm_tuple_t int16Type;

    sysbvm_tuple_t char32Type;
    sysbvm_tuple_t uint32Type;
    sysbvm_tuple_t int32Type;

    sysbvm_tuple_t uint64Type;
    sysbvm_tuple_t int64Type;

    sysbvm_tuple_t bitflagsType;
    sysbvm_tuple_t systemHandleType;

    sysbvm_tuple_t sizeType;
    sysbvm_tuple_t uintptrType;
    sysbvm_tuple_t intptrType;

    sysbvm_tuple_t float32Type;
    sysbvm_tuple_t float64Type;

    sysbvm_tuple_t programmingLanguageType;
    sysbvm_tuple_t sourceCodeType;
    sysbvm_tuple_t sourcePositionType;
    sysbvm_tuple_t tokenType;

    sysbvm_tuple_t astNodeType;
    sysbvm_tuple_t astArgumentNodeType;
    sysbvm_tuple_t astBinaryExpressionSequenceNodeType;
    sysbvm_tuple_t astBreakNodeType;
    sysbvm_tuple_t astCaseNodeType;
    sysbvm_tuple_t astCoerceValueNodeType;
    sysbvm_tuple_t astContinueNodeType;
    sysbvm_tuple_t astDoWhileContinueWithNodeType;
    sysbvm_tuple_t astDownCastNodeType;
    sysbvm_tuple_t astErrorNodeType;
    sysbvm_tuple_t astFunctionApplicationNodeType;
    sysbvm_tuple_t astLambdaNodeType;
    sysbvm_tuple_t astLexicalBlockNodeType;
    sysbvm_tuple_t astLiteralNodeType;
    sysbvm_tuple_t astVariableDefinitionNodeType;
    sysbvm_tuple_t astIdentifierReferenceNodeType;
    sysbvm_tuple_t astIfNodeType;
    sysbvm_tuple_t astMakeAssociationNodeType;
    sysbvm_tuple_t astMakeByteArrayNodeType;
    sysbvm_tuple_t astMakeDictionaryNodeType;
    sysbvm_tuple_t astMakeArrayNodeType;
    sysbvm_tuple_t astMessageSendNodeType;
    sysbvm_tuple_t astMessageChainNodeType;
    sysbvm_tuple_t astMessageChainMessageNodeType;
    sysbvm_tuple_t astPragmaNodeType;
    sysbvm_tuple_t astReturnNodeType;
    sysbvm_tuple_t astSequenceNodeType;
    sysbvm_tuple_t astSwitchNodeType;
    sysbvm_tuple_t astTupleSlotNamedAtNodeType;
    sysbvm_tuple_t astTupleSlotNamedAtPutNodeType;
    sysbvm_tuple_t astTupleSlotNamedReferenceAtNodeType;
    sysbvm_tuple_t astTupleWithLookupStartingFromNodeType;
    sysbvm_tuple_t astUnexpandedApplicationNodeType;
    sysbvm_tuple_t astUnexpandedSExpressionNodeType;
    sysbvm_tuple_t astUseNamedSlotsOfNodeType;
    sysbvm_tuple_t astWhileContinueWithNodeType;

    sysbvm_tuple_t astQuoteNodeType;
    sysbvm_tuple_t astQuasiQuoteNodeType;
    sysbvm_tuple_t astQuasiUnquoteNodeType;
    sysbvm_tuple_t astSpliceNodeType;

    sysbvm_tuple_t exceptionType;
    sysbvm_tuple_t errorType;
    sysbvm_tuple_t argumentsCountMismatchType;
    sysbvm_tuple_t arithmeticErrorType;
    sysbvm_tuple_t compilationErrorType;
    sysbvm_tuple_t domainErrorType;
    sysbvm_tuple_t evaluationErrorType;
    sysbvm_tuple_t parseErrorType;
    sysbvm_tuple_t scanErrorType;
    sysbvm_tuple_t semanticAnalysisErrorType;
    sysbvm_tuple_t zeroDivideType;
    sysbvm_tuple_t assertionFailureType;
    sysbvm_tuple_t cannotReturnType;
    sysbvm_tuple_t modificationForbiddenType;
    sysbvm_tuple_t messageNotUnderstoodType;

    sysbvm_tuple_t functionBytecodeAssembler;
    sysbvm_tuple_t functionBytecodeAssemblerAbstractOperand;
    sysbvm_tuple_t functionBytecodeAssemblerAbstractInstruction;
    sysbvm_tuple_t functionBytecodeAssemblerLabel;
    sysbvm_tuple_t functionBytecodeAssemblerInstruction;
    sysbvm_tuple_t functionBytecodeAssemblerVectorOperand;
    sysbvm_tuple_t functionBytecodeCompiler;
    sysbvm_tuple_t functionBytecodeDirectCompiler;

    sysbvm_tuple_t functionNativeCodeCompiler;

    sysbvm_tuple_t globalNamespace;
    sysbvm_tuple_t defaultAnalysisQueueValueBox;
    sysbvm_tuple_t intrinsicTypes;
    
    sysbvm_globalLookupCacheEntry_t globalMethodLookupCache[GLOBAL_LOOKUP_CACHE_ENTRY_COUNT];
} sysbvm_context_roots_t;

struct sysbvm_context_s
{
    sysbvm_heap_t heap;
    sysbvm_context_roots_t roots;
    uint32_t targetWordSize;
    size_t identityHashSeed;
    bool jitEnabled;
    bool gcDisabled;
    sysbvm_dynarray_t markingStack;
    sysbvm_dynarray_t jittedObjectFileEntries;
    sysbvm_dynarray_t jittedRegisteredFrames;

    sysbvm_pic_t *analyzeASTWithEnvironmentPIC;
    sysbvm_pic_t *evaluateASTWithEnvironment;
    sysbvm_pic_t *evaluateAndAnalyzeASTWithEnvironment;
};

#endif //SYSBVM_INTERNAL_CONTEXT_H
