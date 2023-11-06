#include "sysbvm/interpreter.h"
#include "sysbvm/environment.h"
#include "sysbvm/array.h"
#include "sysbvm/orderedCollection.h"
#include "sysbvm/ast.h"
#include "sysbvm/assert.h"
#include "sysbvm/association.h"
#include "sysbvm/bytecodeCompiler.h"
#include "sysbvm/dictionary.h"
#include "sysbvm/function.h"
#include "sysbvm/gc.h"
#include "sysbvm/errors.h"
#include "sysbvm/filesystem.h"
#include "sysbvm/io.h"
#include "sysbvm/macro.h"
#include "sysbvm/message.h"
#include "sysbvm/parser.h"
#include "sysbvm/pragma.h"
#include "sysbvm/string.h"
#include "sysbvm/sourceCode.h"
#include "sysbvm/stackFrame.h"
#include "sysbvm/sysmelParser.h"
#include "sysbvm/type.h"
#include "internal/context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define sysbvm_gc_safepoint(x) false

static sysbvm_tuple_t sysbvm_interpreter_analyzeASTWithDecayedTypeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t environment);
static sysbvm_tuple_t sysbvm_interpreter_analyzeASTWithDirectTypeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t environment);
static sysbvm_tuple_t sysbvm_interpreter_analyzeASTWithCurrentExpectedTypeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t environment);
static sysbvm_tuple_t sysbvm_interpreter_analyzeASTWithReceiverTypeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t environment);
static sysbvm_tuple_t sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t expectedType, sysbvm_tuple_t environment);
static sysbvm_tuple_t sysbvm_interpreter_analyzeASTWithExpectedTypeExpressionWithEnvironmentAt(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t expectedTypeExpression, sysbvm_tuple_t environment, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t *outExpectedCanonicalType);

SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_analyzeASTWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t environment)
{
    struct {
        sysbvm_tuple_t function;
        sysbvm_tuple_t result;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.function = sysbvm_type_getAstNodeAnalysisFunction(context, sysbvm_tuple_getType(context, astNode));
    if(!gcFrame.function)
        sysbvm_error("Cannot analyze non AST node tuple.");

    gcFrame.result = sysbvm_function_applyNoCheck2(context, gcFrame.function, astNode, environment);
    if(!sysbvm_astNode_getAnalyzedType(gcFrame.result))
    {
        sysbvm_tuple_t nodeTypeString = sysbvm_tuple_printString(context, sysbvm_tuple_getType(context, gcFrame.result));
        sysbvm_errorWithMessageTuple(sysbvm_string_concat(context,
            nodeTypeString,
            sysbvm_string_createWithCString(context, " without analyzed type"))
        );
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_evaluateASTWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t environment)
{
    struct {
        sysbvm_tuple_t function;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.function = sysbvm_type_getAstNodeEvaluationFunction(context, sysbvm_tuple_getType(context, astNode));
    if(!gcFrame.function)
        sysbvm_error("Cannot evaluate non AST node tuple.");

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return sysbvm_function_applyNoCheck2(context, gcFrame.function, astNode, environment);
}

SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t environment)
{
    struct {
        sysbvm_tuple_t environment;
        sysbvm_tuple_t astNode;
        sysbvm_tuple_t analyzedAstNode;
        sysbvm_tuple_t function;
        sysbvm_tuple_t result;
    } gcFrame = {
        .environment = environment,
        .astNode = astNode,
    };

    gcFrame.function = sysbvm_type_getAstNodeAnalysisAndEvaluationFunction(context, sysbvm_tuple_getType(context, gcFrame.astNode));
    if(!gcFrame.function)
        sysbvm_error("Cannot analyze and evaluate non AST node tuple.");

    return sysbvm_function_applyNoCheck2(context, gcFrame.function, gcFrame.astNode, gcFrame.environment);
}

SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_validateThenAnalyzeAndEvaluateASTWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t environment)
{
    struct {
        sysbvm_tuple_t environment;
        sysbvm_tuple_t astNode;
        sysbvm_tuple_t function;
        sysbvm_tuple_t result;
    } gcFrame = {
        .environment = environment,
        .astNode = astNode,
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.function = sysbvm_type_getAstNodeValidationWithAnalysisAndEvaluationFunction(context, sysbvm_tuple_getType(context, gcFrame.astNode));
    if(!gcFrame.function)
    {
        gcFrame.function = sysbvm_type_getAstNodeAnalysisAndEvaluationFunction(context, sysbvm_tuple_getType(context, gcFrame.astNode));
        if(!gcFrame.function)
            sysbvm_error("Cannot validate, analyze and evaluate non AST node tuple.");
    }

    gcFrame.result = sysbvm_function_applyNoCheck2(context, gcFrame.function, gcFrame.astNode, gcFrame.environment);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_analyzeAndEvaluateSourceCodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t sourceCode)
{
    struct {
        sysbvm_tuple_t environment;
        sysbvm_tuple_t sourceCode;
        sysbvm_tuple_t ast;
        sysbvm_tuple_t result;
    } gcFrame = {
        .environment = environment,
        .sourceCode = sourceCode
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    if(sysbvm_string_equalsCString(sysbvm_sourceCode_getLanguage(gcFrame.sourceCode), "sysmel"))
        gcFrame.ast = sysbvm_sysmelParser_parseSourceCode(context, gcFrame.sourceCode);
    else
        gcFrame.ast = sysbvm_parser_parseSourceCode(context, gcFrame.sourceCode);
    gcFrame.result = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.ast, gcFrame.environment);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_validateThenAnalyzeAndEvaluateSourceCodeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t sourceCode)
{
    struct {
        sysbvm_tuple_t environment;
        sysbvm_tuple_t sourceCode;
        sysbvm_tuple_t ast;
        sysbvm_tuple_t result;
    } gcFrame = {
        .environment = environment,
        .sourceCode = sourceCode
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    if(sysbvm_string_equalsCString(sysbvm_sourceCode_getLanguage(gcFrame.sourceCode), "sysmel"))
        gcFrame.ast = sysbvm_sysmelParser_parseSourceCode(context, gcFrame.sourceCode);
    else
        gcFrame.ast = sysbvm_parser_parseSourceCode(context, gcFrame.sourceCode);
    gcFrame.result = sysbvm_interpreter_validateThenAnalyzeAndEvaluateASTWithEnvironment(context, gcFrame.ast, gcFrame.environment);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_analyzeAndEvaluateCStringWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t environment, const char *sourceCodeText, const char *sourceCodeName, const char *sourceCodeLanguage)
{
    struct {
        sysbvm_tuple_t environment;
        sysbvm_tuple_t astNode;
        sysbvm_tuple_t result;
    } gcFrame = {
        .environment = environment,
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    if(!strcmp(sourceCodeLanguage, "sysmel"))
        gcFrame.astNode = sysbvm_sysmelParser_parseCString(context, sourceCodeText, sourceCodeName);
    else
        gcFrame.astNode = sysbvm_parser_parseCString(context, sourceCodeText, sourceCodeName);
    gcFrame.result = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.astNode, gcFrame.environment);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_analyzeAndEvaluateStringWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t environment, sysbvm_tuple_t sourceCodeText, sysbvm_tuple_t sourceCodeDirectory, sysbvm_tuple_t sourceCodeName, sysbvm_tuple_t sourceCodeLanguage)
{
    struct {
        sysbvm_tuple_t environment;
        sysbvm_tuple_t astNode;
        sysbvm_tuple_t result;
    } gcFrame = {
        .environment = environment,
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.astNode = sysbvm_sourceCode_create(context, sourceCodeText, sourceCodeDirectory, sourceCodeName, sourceCodeLanguage);
    gcFrame.result = sysbvm_interpreter_analyzeAndEvaluateSourceCodeWithEnvironment(context, gcFrame.environment, gcFrame.astNode);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_analyzeASTIfNeededWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode_, sysbvm_tuple_t environment_)
{
    struct {
        sysbvm_tuple_t astNode;
        sysbvm_tuple_t environment;
        sysbvm_tuple_t astNodeAnalysisToken;
        sysbvm_tuple_t analysisToken;
        sysbvm_tuple_t result;
    } gcFrame = {
        .astNode = astNode_,
        .environment = environment_,
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.analysisToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, gcFrame.environment);
    gcFrame.astNodeAnalysisToken = sysbvm_astNode_getAnalyzerToken(gcFrame.astNode);
    if(gcFrame.astNodeAnalysisToken == gcFrame.analysisToken)
    {
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.astNode;
    }

    gcFrame.result = sysbvm_interpreter_analyzeASTWithEnvironment(context, gcFrame.astNode, gcFrame.environment);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_interpreter_analyzeASTWithDecayedTypeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t environment)
{
    return sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, astNode, SYSBVM_NULL_TUPLE, environment);
}

static sysbvm_tuple_t sysbvm_interpreter_analyzeASTWithDirectTypeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t environment)
{
    return sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, astNode, context->roots.directTypeInferenceType, environment);
}

static sysbvm_tuple_t sysbvm_interpreter_analyzeASTWithReceiverTypeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode, sysbvm_tuple_t environment)
{
    return sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, astNode, context->roots.receiverTypeInferenceType, environment);
}

static sysbvm_tuple_t sysbvm_interpreter_applyCoercionToASTNodeIntoType(sysbvm_context_t *context, sysbvm_tuple_t astNode_, sysbvm_tuple_t environment_, sysbvm_tuple_t targetType_)
{
    struct {
        sysbvm_tuple_t targetType;
        sysbvm_tuple_t environment;
        sysbvm_tuple_t nodeType;
        sysbvm_tuple_t result;

        sysbvm_tuple_t coercionExpressionType;
    } gcFrame = {
        .result = astNode_,
        .targetType = targetType_,
        .environment = environment_
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    if(!gcFrame.targetType)
        gcFrame.targetType = context->roots.decayedTypeInferenceType;

    // We do not need to perform type checking when just passing the type.
    if(gcFrame.targetType != context->roots.directTypeInferenceType)
    {
        gcFrame.nodeType = sysbvm_astNode_getAnalyzedType(gcFrame.result);
        bool isDecayedType = gcFrame.targetType == context->roots.decayedTypeInferenceType;
        bool isReceiverType = gcFrame.targetType == context->roots.receiverTypeInferenceType;

        if(isDecayedType)
            gcFrame.targetType = sysbvm_type_decay(context, gcFrame.nodeType);
        else if(isReceiverType)
            gcFrame.targetType = sysbvm_type_decayReceiver(context, gcFrame.nodeType);

        if(gcFrame.targetType && !sysbvm_type_isDirectSubtypeOf(gcFrame.nodeType, gcFrame.targetType))
        {
            gcFrame.result = sysbvm_tuple_send2(context, context->roots.coerceASTNodeWithEnvironmentSelector, gcFrame.targetType, gcFrame.result, gcFrame.environment);
            gcFrame.result = sysbvm_interpreter_analyzeASTIfNeededWithEnvironment(context, gcFrame.result, gcFrame.environment);
        }
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_interpreter_analyzeASTWithCurrentExpectedTypeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode_, sysbvm_tuple_t environment_)
{
    struct {
        sysbvm_tuple_t astNode;
        sysbvm_tuple_t environment;
        sysbvm_tuple_t targetType;
        sysbvm_tuple_t result;

        sysbvm_tuple_t coercionExpressionType;
    } gcFrame = {
        .astNode = astNode_,
        .environment = environment_
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.targetType = sysbvm_analysisAndEvaluationEnvironment_getExpectedType(context, gcFrame.environment);
    gcFrame.result = sysbvm_interpreter_analyzeASTIfNeededWithEnvironment(context, gcFrame.astNode, gcFrame.environment);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return sysbvm_interpreter_applyCoercionToASTNodeIntoType(context, gcFrame.result, gcFrame.environment, gcFrame.targetType);
}

static sysbvm_tuple_t sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(sysbvm_context_t *context, sysbvm_tuple_t astNode_, sysbvm_tuple_t expectedType_, sysbvm_tuple_t environment_)
{
    struct {
        sysbvm_tuple_t astNode;
        sysbvm_tuple_t expectedType;
        sysbvm_tuple_t environment;

        sysbvm_tuple_t oldExpectedType;
        sysbvm_tuple_t nodeType;
        sysbvm_tuple_t result;

        sysbvm_tuple_t coercionExpressionType;
    } gcFrame = {
        .astNode = astNode_,
        .expectedType = expectedType_,
        .environment = environment_
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    if(!gcFrame.expectedType)
        gcFrame.expectedType = context->roots.decayedTypeInferenceType;

    // Store the old expected type.
    gcFrame.oldExpectedType = sysbvm_analysisAndEvaluationEnvironment_getExpectedType(context, gcFrame.environment);
    sysbvm_analysisAndEvaluationEnvironment_setExpectedType(context, gcFrame.environment, gcFrame.expectedType);

    // Analyze the target node itself.
    gcFrame.result = sysbvm_interpreter_analyzeASTIfNeededWithEnvironment(context, gcFrame.astNode, gcFrame.environment);

    // Restore the old expected type.
    sysbvm_analysisAndEvaluationEnvironment_setExpectedType(context, gcFrame.environment, gcFrame.oldExpectedType);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return sysbvm_interpreter_applyCoercionToASTNodeIntoType(context, gcFrame.result, gcFrame.environment, gcFrame.expectedType);
}

static sysbvm_tuple_t sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironmentAt(sysbvm_context_t *context, sysbvm_tuple_t astNode_, sysbvm_tuple_t expectedType_, sysbvm_tuple_t environment_, sysbvm_tuple_t sourcePosition_)
{
    struct {
        sysbvm_tuple_t astNode;
        sysbvm_tuple_t expectedType;
        sysbvm_tuple_t environment;
        sysbvm_tuple_t sourcePosition;

        sysbvm_tuple_t defaultValue;
        sysbvm_tuple_t result;
    } gcFrame = {
        .astNode = astNode_,
        .expectedType = expectedType_,
        .environment = environment_,
        .sourcePosition = sourcePosition_
    };

    if(!gcFrame.astNode)
    {
        SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
        gcFrame.defaultValue = sysbvm_type_getDefaultValue(context, gcFrame.expectedType);
        gcFrame.result = sysbvm_astLiteralNode_create(context, gcFrame.defaultValue, gcFrame.sourcePosition);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.result;
    }
    else
    {
        return sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.astNode, gcFrame.expectedType, gcFrame.environment);
    }
}

static sysbvm_tuple_t sysbvm_interpreter_analyzeASTWithExpectedTypeExpressionWithEnvironmentAt(sysbvm_context_t *context, sysbvm_tuple_t astNode_, sysbvm_tuple_t expectedTypeExpression_, sysbvm_tuple_t environment_, sysbvm_tuple_t sourcePosition_, sysbvm_tuple_t *outExpectedCanonicalType)
{
    // Attempt to delegate first.
    if(!expectedTypeExpression_)
    {
        sysbvm_tuple_t result = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironmentAt(context, astNode_, SYSBVM_NULL_TUPLE, environment_, sourcePosition_);
        if(outExpectedCanonicalType)
            *outExpectedCanonicalType = sysbvm_astNode_getAnalyzedType(result);
        return result;
    }
    else if(sysbvm_astNode_isLiteralNode(context, expectedTypeExpression_))
    {
        if(outExpectedCanonicalType)
            *outExpectedCanonicalType = sysbvm_astLiteralNode_getValue(expectedTypeExpression_);
        return sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironmentAt(context, astNode_, sysbvm_astLiteralNode_getValue(expectedTypeExpression_), environment_, sourcePosition_);
    }

    struct {
        sysbvm_tuple_t astNode;
        sysbvm_tuple_t expectedTypeExpression;

        sysbvm_tuple_t analyzedTypeExpression;
        sysbvm_tuple_t analyzedTypeExpressionType;
        sysbvm_tuple_t resultType;

        sysbvm_tuple_t environment;
        sysbvm_tuple_t sourcePosition;

        sysbvm_tuple_t result;
    } gcFrame = {
        .astNode = astNode_,
        .expectedTypeExpression = expectedTypeExpression_,
        .environment = environment_,
        .sourcePosition = sourcePosition_
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    // Make sure the type expression is analyzed.
    gcFrame.analyzedTypeExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.expectedTypeExpression, context->roots.typeType, environment_);
    if(!gcFrame.analyzedTypeExpression)
    {
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.astNode, SYSBVM_NULL_TUPLE, gcFrame.environment);
    }
    else if(sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedTypeExpression))
    {
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.astNode, sysbvm_astLiteralNode_getValue(gcFrame.analyzedTypeExpression), gcFrame.environment);
    }

    gcFrame.analyzedTypeExpressionType = sysbvm_astNode_getAnalyzedType(gcFrame.analyzedTypeExpression);
    gcFrame.resultType = sysbvm_type_getCanonicalPendingInstanceType(context, gcFrame.analyzedTypeExpressionType);
    if(!gcFrame.astNode)
        gcFrame.astNode = sysbvm_astLiteralNode_create(context, sysbvm_type_getDefaultValue(context, gcFrame.resultType), gcFrame.sourcePosition);

    // Analyze the value expression.
    gcFrame.result = sysbvm_interpreter_analyzeASTWithDirectTypeWithEnvironment(context, gcFrame.astNode, gcFrame.environment);
    bool shouldAddCoercionNode = true;

    if(sysbvm_astNode_isCoerceValueNode(context, gcFrame.result))
    {
        sysbvm_astCoerceValueNode_t **coerceNode = (sysbvm_astCoerceValueNode_t **)&gcFrame.result;
        if((*coerceNode)->typeExpression == gcFrame.analyzedTypeExpression
        && (*coerceNode)->super.analyzedType == gcFrame.resultType)
            shouldAddCoercionNode = false;
    }

    // Add the coercion node
    if(shouldAddCoercionNode)
    {
        gcFrame.result = sysbvm_astCoerceValueNode_create(context, gcFrame.sourcePosition, gcFrame.analyzedTypeExpression, gcFrame.result);

        sysbvm_astCoerceValueNode_t **coerceNode = (sysbvm_astCoerceValueNode_t **)&gcFrame.result;
        (*coerceNode)->super.analyzedType = gcFrame.resultType;
        (*coerceNode)->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, gcFrame.environment);

    }

    if(outExpectedCanonicalType)
        *outExpectedCanonicalType = gcFrame.resultType;

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astSequenceNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *bodyNodes = &arguments[1];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    sysbvm_tuple_t pragmas = sysbvm_array_create(context, 0);
    return sysbvm_astSequenceNode_create(context, sourcePosition, pragmas, *bodyNodes);
}

static sysbvm_tuple_t sysbvm_astSequenceNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astSequenceNode_t **sequenceNode = (sysbvm_astSequenceNode_t**)node;

    struct {
        sysbvm_astSequenceNode_t *analyzedSequenceNode;

        sysbvm_tuple_t pragmas;
        sysbvm_tuple_t analyzedPragmas;
        sysbvm_tuple_t pragma;
        sysbvm_tuple_t analyzedPragma;

        sysbvm_tuple_t expressions;
        sysbvm_tuple_t analyzedExpressions;
        sysbvm_tuple_t expression;
        sysbvm_tuple_t analyzedExpression;

        sysbvm_tuple_t elementValue;
        sysbvm_tuple_t elementType;
        sysbvm_tuple_t elementMetaType;
        sysbvm_tuple_t resultType;
        sysbvm_tuple_t concretizeFunction;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*sequenceNode)->super.sourcePosition);

    gcFrame.pragmas = (*sequenceNode)->pragmas;
    gcFrame.expressions = (*sequenceNode)->expressions;
    size_t pragmaCount = sysbvm_array_getSize(gcFrame.pragmas);
    size_t expressionCount = sysbvm_array_getSize(gcFrame.expressions);
    if(pragmaCount == 0 && expressionCount == 0 && (*sequenceNode)->super.analyzedType)
    {
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return *node;
    }

    gcFrame.analyzedSequenceNode = (sysbvm_astSequenceNode_t *)sysbvm_context_shallowCopy(context, *node);
    gcFrame.analyzedSequenceNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);

    gcFrame.analyzedPragmas = sysbvm_array_create(context, pragmaCount);
    for(size_t i = 0; i < pragmaCount; ++i)
    {
        gcFrame.pragma = sysbvm_array_at(gcFrame.pragmas, i);
        gcFrame.analyzedPragma = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.pragma, context->roots.pragmaType, *environment);
        sysbvm_array_atPut(gcFrame.analyzedPragmas, i, gcFrame.analyzedPragma);
    }

    gcFrame.analyzedSequenceNode->pragmas = gcFrame.analyzedPragmas;

    gcFrame.analyzedExpressions = sysbvm_array_create(context, expressionCount);
    gcFrame.resultType = context->roots.voidType;
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = sysbvm_array_at(gcFrame.expressions, i);
        if(i + 1 < expressionCount)
            gcFrame.analyzedExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.expression, context->roots.voidType, *environment);
        else
            gcFrame.analyzedExpression = sysbvm_interpreter_analyzeASTWithCurrentExpectedTypeWithEnvironment(context, gcFrame.expression, *environment);

        // FIXME: Move this metavalue decay onto the type checker.
        if(sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedExpression))
        {
            gcFrame.elementValue = sysbvm_astLiteralNode_getValue(gcFrame.analyzedExpression);
            gcFrame.elementType = sysbvm_tuple_getType(context, gcFrame.elementValue);
            gcFrame.elementMetaType = sysbvm_tuple_getType(context, gcFrame.elementType);
            gcFrame.concretizeFunction = sysbvm_type_getAnalyzeConcreteMetaValueWithEnvironmentFunction(context, gcFrame.elementMetaType);
            if(gcFrame.concretizeFunction)
                gcFrame.analyzedExpression = sysbvm_function_applyNoCheck3(context, gcFrame.concretizeFunction, gcFrame.elementType, gcFrame.analyzedExpression, *environment);
        }

        gcFrame.resultType = sysbvm_astNode_getAnalyzedType(gcFrame.analyzedExpression);
        sysbvm_array_atPut(gcFrame.analyzedExpressions, i, gcFrame.analyzedExpression);
    }

    gcFrame.analyzedSequenceNode->expressions = gcFrame.analyzedExpressions;
    gcFrame.analyzedSequenceNode->super.analyzedType = gcFrame.resultType;
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);

    if(pragmaCount == 0 && expressionCount == 1)
        return sysbvm_array_at(gcFrame.analyzedExpressions, 0);

    return (sysbvm_tuple_t)gcFrame.analyzedSequenceNode;
}

static sysbvm_tuple_t sysbvm_astSequenceNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astSequenceNode_t **sequenceNode = (sysbvm_astSequenceNode_t**)node;

    struct {
        sysbvm_tuple_t expression;
        sysbvm_tuple_t result;
    } gcFrame = {
        .result = SYSBVM_VOID_TUPLE,
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*sequenceNode)->super.sourcePosition);

    size_t expressionCount = sysbvm_array_getSize((*sequenceNode)->expressions);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = sysbvm_array_at((*sequenceNode)->expressions, i);
        gcFrame.result = sysbvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.expression, *environment);
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);

    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astSequenceNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_tuple_t result;
        sysbvm_tuple_t expression;
        sysbvm_tuple_t elementType;
        sysbvm_tuple_t elementMetaType;
        sysbvm_tuple_t concretizeFunction;
    } gcFrame = {
        .result = SYSBVM_VOID_TUPLE
    };

    sysbvm_astSequenceNode_t **sequenceNode = (sysbvm_astSequenceNode_t**)node;

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*sequenceNode)->super.sourcePosition);

    size_t expressionCount = sysbvm_array_getSize((*sequenceNode)->expressions);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = sysbvm_array_at((*sequenceNode)->expressions, i);
        gcFrame.result = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expression, *environment);
        
        gcFrame.elementType = sysbvm_tuple_getType(context, gcFrame.result);
        gcFrame.elementMetaType = sysbvm_tuple_getType(context, gcFrame.elementType);
        gcFrame.concretizeFunction = sysbvm_type_getAnalyzeAndEvaluateConcreteMetaValueWithEnvironmentFunction(context, gcFrame.elementMetaType);
        if(gcFrame.concretizeFunction)
            gcFrame.result = sysbvm_function_applyNoCheck3(context, gcFrame.concretizeFunction, gcFrame.elementType, gcFrame.result, *environment);
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astLambdaNode_parseArgumentsNodes(sysbvm_context_t *context, sysbvm_tuple_t unsafeArgumentsNode, bool *hasVariadicArguments, sysbvm_tuple_t *resultTypeNode)
{
    struct {
        sysbvm_tuple_t argumentsNode;
        sysbvm_tuple_t argumentList;
        sysbvm_tuple_t nameNode;

        sysbvm_tuple_t unparsedArgumentNode;
        sysbvm_tuple_t isForAll;
        sysbvm_tuple_t nameExpression;
        sysbvm_tuple_t typeExpression;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.argumentsNode = unsafeArgumentsNode;
    gcFrame.argumentList = sysbvm_orderedCollection_create(context);
    size_t argumentNodeCount = sysbvm_array_getSize(gcFrame.argumentsNode);
    *hasVariadicArguments = false;
    *resultTypeNode = SYSBVM_NULL_TUPLE;
    for(size_t i = 0; i < argumentNodeCount; ++i)
    {
        gcFrame.unparsedArgumentNode = sysbvm_array_at(gcFrame.argumentsNode, i);
        if(sysbvm_astNode_isIdentifierReferenceNode(context, gcFrame.unparsedArgumentNode))
        {
            if(sysbvm_astIdentifierReferenceNode_isEllipsis(gcFrame.unparsedArgumentNode))
            {
                if(i + 1 < argumentNodeCount)
                    sysbvm_error("Ellipsis can only be present at the end.");
                else if(i == 0)
                    sysbvm_error("Ellipsis cannot be the first argument.");

                *hasVariadicArguments = true;
                continue;
            }
            else if(sysbvm_astIdentifierReferenceNode_isArrow(gcFrame.unparsedArgumentNode))
            {
                if(i + 2 != argumentNodeCount)
                    sysbvm_error("Result type expression can only be present at the end");

                *resultTypeNode = sysbvm_array_at(gcFrame.argumentsNode, i + 1);
                break;
            }

            gcFrame.isForAll = SYSBVM_FALSE_TUPLE;
            gcFrame.nameExpression = sysbvm_astLiteralNode_create(context, sysbvm_astNode_getSourcePosition(gcFrame.unparsedArgumentNode), sysbvm_astIdentifierReferenceNode_getValue(gcFrame.unparsedArgumentNode));
            gcFrame.typeExpression = SYSBVM_NULL_TUPLE;

        }
        else if(sysbvm_astNode_isUnexpandedSExpressionNode(context, gcFrame.unparsedArgumentNode))
        {
            sysbvm_astUnexpandedSExpressionNode_t *argumentNode = (sysbvm_astUnexpandedSExpressionNode_t*)gcFrame.unparsedArgumentNode;
            size_t elementCount = sysbvm_array_getSize(argumentNode->elements);
            gcFrame.isForAll = SYSBVM_FALSE_TUPLE;
            gcFrame.nameExpression = sysbvm_astLiteralNode_create(context, sysbvm_astNode_getSourcePosition(gcFrame.unparsedArgumentNode), sysbvm_astIdentifierReferenceNode_getValue(gcFrame.unparsedArgumentNode));
            gcFrame.typeExpression = SYSBVM_NULL_TUPLE;

            if(elementCount >= 1)
            {
                gcFrame.nameNode = sysbvm_array_at(argumentNode->elements, 0);
                if(!sysbvm_astNode_isIdentifierReferenceNode(context, gcFrame.nameNode))
                    sysbvm_error("Argument name must be an identifier.");
                gcFrame.nameExpression = sysbvm_astLiteralNode_create(context, sysbvm_astNode_getSourcePosition(gcFrame.nameNode), sysbvm_astIdentifierReferenceNode_getValue(gcFrame.nameNode));
            }

            if(elementCount >= 2)
                gcFrame.typeExpression = sysbvm_array_at(argumentNode->elements, 1);
        }
        else
        {
            sysbvm_error("Invalid argument definition node.");
        }

        sysbvm_orderedCollection_add(context, gcFrame.argumentList, sysbvm_astArgumentNode_create(context, sysbvm_astNode_getSourcePosition(gcFrame.unparsedArgumentNode), gcFrame.isForAll, gcFrame.nameExpression, gcFrame.typeExpression));
    }

    sysbvm_tuple_t result = sysbvm_orderedCollection_asArray(context, gcFrame.argumentList);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return result;
}

static sysbvm_tuple_t sysbvm_astLambdaNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *argumentsSExpressionNode = &arguments[1];
    sysbvm_tuple_t *bodyNodes = &arguments[2];

    struct {
        sysbvm_tuple_t argumentsNode;
        sysbvm_tuple_t sourcePosition;
        sysbvm_tuple_t argumentsArraySlice;
        sysbvm_tuple_t resultTypeNode;
        sysbvm_tuple_t pragmas;
        sysbvm_tuple_t bodyNodes;
        sysbvm_tuple_t bodySequence;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    if(!sysbvm_astNode_isUnexpandedSExpressionNode(context, *argumentsSExpressionNode))
        sysbvm_error("Expected a S-Expression with the arguments node.");

    bool hasVariadicArguments = false;
    gcFrame.argumentsNode = sysbvm_astUnexpandedSExpressionNode_getElements(*argumentsSExpressionNode);
    gcFrame.sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    gcFrame.argumentsArraySlice = sysbvm_astLambdaNode_parseArgumentsNodes(context, gcFrame.argumentsNode, &hasVariadicArguments, &gcFrame.resultTypeNode);
    gcFrame.pragmas = sysbvm_array_create(context, 0);
    gcFrame.bodyNodes = *bodyNodes;
    gcFrame.bodySequence = sysbvm_astSequenceNode_create(context, gcFrame.sourcePosition, gcFrame.pragmas, gcFrame.bodyNodes);
    sysbvm_tuple_t result = sysbvm_astLambdaNode_create(context, gcFrame.sourcePosition,
        sysbvm_tuple_bitflags_encode(hasVariadicArguments ? SYSBVM_FUNCTION_FLAGS_VARIADIC : SYSBVM_FUNCTION_FLAGS_NONE),
        gcFrame.argumentsArraySlice, gcFrame.resultTypeNode, gcFrame.bodySequence);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return result;
}

static sysbvm_tuple_t sysbvm_astArgumentNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astArgumentNode_t *argumentNode;
        sysbvm_tuple_t analyzedName;
        sysbvm_tuple_t analyzedType;
        sysbvm_tuple_t evaluatedName;
        sysbvm_tuple_t evaluatedType;
        sysbvm_tuple_t argumentBinding;

    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.argumentNode = (sysbvm_astArgumentNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.argumentNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);

    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.argumentNode->super.sourcePosition);

    if(gcFrame.argumentNode->name)
    {
        gcFrame.analyzedName = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.argumentNode->name, context->roots.symbolType, *environment);
        gcFrame.argumentNode->name = gcFrame.analyzedName;

        if(sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedName))
            gcFrame.evaluatedName = sysbvm_astLiteralNode_getValue(gcFrame.analyzedName);
    }
    if(gcFrame.argumentNode->type)
    {
        gcFrame.analyzedType = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.argumentNode->type, context->roots.typeType, *environment);
        gcFrame.argumentNode->type = gcFrame.analyzedType;
        if(sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedType))
            gcFrame.evaluatedType = sysbvm_astLiteralNode_getValue(gcFrame.analyzedType);
    }

    // TODO: Fetch or attempt to infer the default argument type from a more proper place.
    if(!gcFrame.argumentNode->type)
        gcFrame.argumentNode->type = sysbvm_astLiteralNode_create(context, gcFrame.argumentNode->super.sourcePosition, context->roots.anyValueType);
    if(!gcFrame.evaluatedType)
        gcFrame.evaluatedType = context->roots.anyValueType;
    gcFrame.argumentNode->super.analyzedType = gcFrame.evaluatedType;

    gcFrame.argumentBinding = sysbvm_analysisEnvironment_setNewSymbolArgumentBinding(context, *environment, gcFrame.argumentNode->super.sourcePosition, gcFrame.evaluatedName, gcFrame.evaluatedType);
    gcFrame.argumentNode->binding = gcFrame.argumentBinding;

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.argumentNode;
}

static sysbvm_tuple_t sysbvm_astCoerceValueNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *tupleExpression = &arguments[1];
    sysbvm_tuple_t *typeExpression = &arguments[2];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    return sysbvm_astCoerceValueNode_create(context, sourcePosition, *typeExpression, *tupleExpression);
}

static sysbvm_tuple_t sysbvm_astCoerceValueNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astCoerceValueNode_t **coerceValueNode = (sysbvm_astCoerceValueNode_t**)node;

    return sysbvm_interpreter_analyzeASTWithExpectedTypeExpressionWithEnvironmentAt(context, (*coerceValueNode)->valueExpression, (*coerceValueNode)->typeExpression, *environment, (*coerceValueNode)->super.sourcePosition, NULL);
}

static sysbvm_tuple_t sysbvm_astCoerceValueNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astCoerceValueNode_t **coerceValueNode = (sysbvm_astCoerceValueNode_t**)node;
    struct {
        sysbvm_tuple_t type;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*coerceValueNode)->super.sourcePosition);

    gcFrame.type = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*coerceValueNode)->typeExpression, *environment);
    gcFrame.result = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*coerceValueNode)->valueExpression, *environment);
    gcFrame.result = sysbvm_type_coerceValue(context, gcFrame.type, gcFrame.result);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astCoerceValueNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astCoerceValueNode_t **coerceValueNode = (sysbvm_astCoerceValueNode_t**)node;
    struct {
        sysbvm_tuple_t type;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*coerceValueNode)->super.sourcePosition);

    gcFrame.type = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*coerceValueNode)->typeExpression, *environment);
    gcFrame.type = sysbvm_type_coerceValue(context, context->roots.typeType, gcFrame.type);

    gcFrame.result = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*coerceValueNode)->valueExpression, *environment);
    gcFrame.result = sysbvm_type_coerceValue(context, gcFrame.type, gcFrame.result);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astDownCastNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *tupleExpression = &arguments[1];
    sysbvm_tuple_t *typeExpression = &arguments[2];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    return sysbvm_astDownCastNode_create(context, sourcePosition, *typeExpression, *tupleExpression);
}

static sysbvm_tuple_t sysbvm_astDownCastNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astDownCastNode_t *downCastNode;
        sysbvm_tuple_t typeExpression;
        sysbvm_tuple_t typeExpressionType;
        sysbvm_tuple_t type;

        sysbvm_tuple_t valueExpression;
        sysbvm_tuple_t valueExpressionType;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.downCastNode = (sysbvm_astDownCastNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.downCastNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.downCastNode->super.sourcePosition);

    gcFrame.typeExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.downCastNode->typeExpression, context->roots.typeType, *environment);
    gcFrame.downCastNode->typeExpression = gcFrame.typeExpression;
    bool hasLiteralType = sysbvm_astNode_isLiteralNode(context, gcFrame.typeExpression);
    if(hasLiteralType)
    {
        gcFrame.type = sysbvm_astLiteralNode_getValue(gcFrame.typeExpression);
    }
    else
    {
        gcFrame.typeExpressionType = sysbvm_astNode_getAnalyzedType(gcFrame.typeExpression);
        gcFrame.type = sysbvm_type_getCanonicalPendingInstanceType(context, gcFrame.typeExpressionType);
    }

    gcFrame.valueExpression = sysbvm_interpreter_analyzeASTWithDecayedTypeWithEnvironment(context, gcFrame.downCastNode->valueExpression, *environment);
    gcFrame.downCastNode->valueExpression = gcFrame.valueExpression;
    SYSBVM_ASSERT(gcFrame.type);
    gcFrame.downCastNode->super.analyzedType = gcFrame.type;

    // Is this already a value with the expected type?
    if(hasLiteralType)
    {
        gcFrame.valueExpressionType = sysbvm_astNode_getAnalyzedType(gcFrame.valueExpression);
        if(sysbvm_type_isDirectSubtypeOf(gcFrame.valueExpressionType, gcFrame.type))
        {
            SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return (sysbvm_tuple_t)gcFrame.valueExpression;
        }
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.downCastNode;
}

static sysbvm_tuple_t sysbvm_astDownCastNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astDownCastNode_t **downCastNode = (sysbvm_astDownCastNode_t**)node;
    struct {
        sysbvm_tuple_t type;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*downCastNode)->super.sourcePosition);

    gcFrame.type = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*downCastNode)->typeExpression, *environment);
    gcFrame.result = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*downCastNode)->valueExpression, *environment);
    if(!sysbvm_tuple_isKindOf(context, gcFrame.result, gcFrame.type))
        sysbvm_error_unexpectedType(gcFrame.type, gcFrame.result);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astDownCastNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astCoerceValueNode_t **coerceValueNode = (sysbvm_astCoerceValueNode_t**)node;
    struct {
        sysbvm_tuple_t type;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*coerceValueNode)->super.sourcePosition);

    gcFrame.type = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*coerceValueNode)->typeExpression, *environment);
    gcFrame.type = sysbvm_type_coerceValue(context, context->roots.typeType, gcFrame.type);

    gcFrame.result = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*coerceValueNode)->valueExpression, *environment);
    if(!sysbvm_tuple_isKindOf(context, gcFrame.result, gcFrame.type))
        sysbvm_error("Downcast failure.");

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astErrorNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    //sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astErrorNode_t **errorNode = (sysbvm_astErrorNode_t**)node;
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*errorNode)->super.sourcePosition);

    sysbvm_errorWithMessageTuple((*errorNode)->errorMessage);
    return SYSBVM_NULL_TUPLE;
}

sysbvm_tuple_t sysbvm_interpreter_recompileAndOptimizeFunction(sysbvm_context_t *context, sysbvm_function_t **functionObject)
{
    struct {
        sysbvm_functionDefinition_t *functionDefinition;
        sysbvm_functionSourceDefinition_t *functionSourceDefinition;
        sysbvm_tuple_t optimizedFunction;
        sysbvm_tuple_t optimizedDefinitionEnvironment;
        sysbvm_functionDefinition_t *optimizedFunctionDefinition;
        sysbvm_functionSourceDefinition_t *optimizedSourceDefinition;
        sysbvm_tuple_t captureList;
        sysbvm_tuple_t captureValue;
        sysbvm_tuple_t captureBinding;
    } gcFrame = {
        .functionDefinition = (sysbvm_functionDefinition_t *)(*functionObject)->definition,
    };

    // If the function is not yet optimized, just return it back.
    if(!gcFrame.functionDefinition || !gcFrame.functionDefinition->sourceDefinition || !(gcFrame.functionDefinition->sourceAnalyzedDefinition || gcFrame.functionDefinition->bytecode))
        return (sysbvm_tuple_t)*functionObject;

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.functionSourceDefinition = (sysbvm_functionSourceDefinition_t*)gcFrame.functionDefinition->sourceDefinition;

    if(gcFrame.functionDefinition->bytecode)
    {
        gcFrame.captureList = ((sysbvm_functionBytecode_t*)gcFrame.functionDefinition->bytecode)->captures;
    }
    else
    {
        SYSBVM_ASSERT(gcFrame.functionDefinition->sourceAnalyzedDefinition);
        gcFrame.captureList = ((sysbvm_functionSourceAnalyzedDefinition_t*)gcFrame.functionDefinition->sourceAnalyzedDefinition)->captures;
    }

    gcFrame.optimizedFunctionDefinition = (sysbvm_functionDefinition_t*)sysbvm_context_shallowCopy(context, (sysbvm_tuple_t)gcFrame.functionDefinition);
    gcFrame.optimizedFunctionDefinition->super.owner = SYSBVM_NULL_TUPLE;
    gcFrame.optimizedFunctionDefinition->super.name = SYSBVM_NULL_TUPLE;

    gcFrame.optimizedSourceDefinition = (sysbvm_functionSourceDefinition_t*)sysbvm_context_shallowCopy(context, (sysbvm_tuple_t)gcFrame.optimizedFunctionDefinition->sourceDefinition);
    gcFrame.optimizedFunctionDefinition->sourceDefinition = (sysbvm_tuple_t)gcFrame.optimizedSourceDefinition;

    // Construct the closure environment by reading the capture vector.
    gcFrame.optimizedDefinitionEnvironment = sysbvm_analysisAndEvaluationEnvironment_create(context, gcFrame.optimizedSourceDefinition->environment);
    sysbvm_analysisAndEvaluationEnvironment_clearAnalyzerToken(context, gcFrame.optimizedDefinitionEnvironment);
    gcFrame.optimizedSourceDefinition->environment = gcFrame.optimizedDefinitionEnvironment;

    size_t captureCount = sysbvm_array_getSize((*functionObject)->captureVector);
    for(size_t i = 0; i < captureCount; ++i)
    {
        gcFrame.captureValue = sysbvm_array_at((*functionObject)->captureVector, i);
        gcFrame.captureBinding = sysbvm_array_at(gcFrame.captureList, i);
        sysbvm_environment_setNewSymbolBindingWithValue(context, gcFrame.optimizedDefinitionEnvironment, sysbvm_symbolBinding_getName(gcFrame.captureBinding), gcFrame.captureValue);
    }

    gcFrame.optimizedFunctionDefinition->sourceAnalyzedDefinition = SYSBVM_NULL_TUPLE;
    gcFrame.optimizedFunctionDefinition->captureVectorType = SYSBVM_NULL_TUPLE;
    gcFrame.optimizedFunctionDefinition->type = SYSBVM_NULL_TUPLE;
    gcFrame.optimizedFunctionDefinition->pragmas = SYSBVM_NULL_TUPLE;
    gcFrame.optimizedFunctionDefinition->innerFunctions = SYSBVM_NULL_TUPLE;
    gcFrame.optimizedFunctionDefinition->primitiveName = SYSBVM_NULL_TUPLE;
    gcFrame.optimizedFunctionDefinition->annotations = SYSBVM_NULL_TUPLE;

    gcFrame.optimizedFunctionDefinition->bytecode = SYSBVM_NULL_TUPLE;
    gcFrame.optimizedFunctionDefinition->nativeCodeDefinition = SYSBVM_NULL_TUPLE;

    sysbvm_functionDefinition_ensureAnalysis(context, &gcFrame.optimizedFunctionDefinition);
    SYSBVM_ASSERT(sysbvm_array_getSize(((sysbvm_functionBytecode_t*)gcFrame.optimizedFunctionDefinition->bytecode)->captures) == 0);
    gcFrame.optimizedFunction = sysbvm_function_createClosureWithCaptureVector(context, (sysbvm_tuple_t)gcFrame.optimizedFunctionDefinition, sysbvm_sequenceTuple_create(context, gcFrame.optimizedFunctionDefinition->captureVectorType));
    ((sysbvm_function_t*)gcFrame.optimizedFunction)->flags = (*functionObject)->flags;
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.optimizedFunction;
}

static bool canCanonicalizeArgumentNodeType(sysbvm_context_t *context, sysbvm_tuple_t node)
{
    SYSBVM_ASSERT(sysbvm_astNode_isArgumentNode(context, node));
    
    sysbvm_astArgumentNode_t *argumentNode = (sysbvm_astArgumentNode_t*)node;
    return !sysbvm_tuple_boolean_decode(argumentNode->isForAll)
        && (!argumentNode->type || sysbvm_astNode_isLiteralNode(context, argumentNode->type));

}
SYSBVM_API sysbvm_tuple_t sysbvm_type_canonicalizeFunctionType(sysbvm_context_t *context, sysbvm_tuple_t functionType)
{
    if(!sysbvm_tuple_isKindOf(context, functionType, context->roots.dependentFunctionTypeType))
        return functionType;

    sysbvm_dependentFunctionType_t *dependentFunctionType = (sysbvm_dependentFunctionType_t*)functionType;
    
    // Only accept literal values for argument types.
    size_t argumentCount = sysbvm_array_getSize(dependentFunctionType->argumentNodes);
    for(size_t i = 0; i < argumentCount; ++i)
    {
        if(!canCanonicalizeArgumentNodeType(context, sysbvm_array_at(dependentFunctionType->argumentNodes, i)))
            return functionType;
    }

    // Check the result type.
    if(dependentFunctionType->resultTypeNode && !sysbvm_astNode_isLiteralNode(context, dependentFunctionType->resultTypeNode))
        return functionType;

    sysbvm_tuple_t argumentTypes = sysbvm_array_create(context, argumentCount);
    for(size_t i = 0; i < argumentCount; ++i)
    {
        sysbvm_astArgumentNode_t *argumentNode = (sysbvm_astArgumentNode_t*)sysbvm_array_at(dependentFunctionType->argumentNodes, i);
        sysbvm_tuple_t argumentType = context->roots.anyValueType;
        if(argumentNode->type)
            argumentType = sysbvm_astLiteralNode_getValue(argumentNode->type);
        sysbvm_array_atPut(argumentTypes, i, argumentType);
    }

    sysbvm_tuple_t resultType = context->roots.anyValueType;
    if(dependentFunctionType->resultTypeNode)
        resultType = sysbvm_astLiteralNode_getValue(dependentFunctionType->resultTypeNode);

    sysbvm_bitflags_t flags = sysbvm_tuple_bitflags_decode(dependentFunctionType->super.functionFlags);
    return sysbvm_type_createSimpleFunctionType(context, argumentTypes, flags, resultType);
}

static void sysbvm_functionDefinition_analyzeType(sysbvm_context_t *context, sysbvm_functionDefinition_t **functionDefinition)
{
    struct {
        sysbvm_functionSourceDefinition_t *sourceDefinition;
        sysbvm_functionSourceAnalyzedDefinition_t *sourceAnalyzedDefinition;

        sysbvm_tuple_t analysisEnvironment;
        sysbvm_functionAnalysisEnvironment_t *analysisEnvironmentObject;
        sysbvm_tuple_t analyzedArgumentNode;
        sysbvm_tuple_t analyzedArgumentsNode;
        sysbvm_tuple_t analyzedBodyNode;
        sysbvm_tuple_t analyzedResultTypeNode;
        sysbvm_tuple_t analyzedType;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourceDefinition = (sysbvm_functionSourceDefinition_t*)(*functionDefinition)->sourceDefinition;
    gcFrame.sourceAnalyzedDefinition = (sysbvm_functionSourceAnalyzedDefinition_t*)(*functionDefinition)->sourceAnalyzedDefinition;
    if(!gcFrame.sourceAnalyzedDefinition)
    {
        gcFrame.sourceAnalyzedDefinition = (sysbvm_functionSourceAnalyzedDefinition_t*)sysbvm_context_allocatePointerTuple(context, context->roots.functionSourceAnalyzedDefinitionType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_functionSourceAnalyzedDefinition_t));
        gcFrame.sourceAnalyzedDefinition->sourcePosition = gcFrame.sourceDefinition->sourcePosition;
        (*functionDefinition)->sourceAnalyzedDefinition = (sysbvm_tuple_t)gcFrame.sourceAnalyzedDefinition;
    }

    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.sourceAnalyzedDefinition->sourcePosition);

    gcFrame.analysisEnvironment = sysbvm_functionAnalysisEnvironment_create(context, gcFrame.sourceDefinition->environment, (sysbvm_tuple_t)*functionDefinition);
    gcFrame.analysisEnvironmentObject = (sysbvm_functionAnalysisEnvironment_t*)gcFrame.analysisEnvironment;
    size_t argumentNodeCount = sysbvm_array_getSize(gcFrame.sourceDefinition->argumentNodes);
    gcFrame.analyzedArgumentsNode = sysbvm_array_create(context, argumentNodeCount);
    for(size_t i = 0; i < argumentNodeCount; ++i)
    {
        gcFrame.analyzedArgumentNode = sysbvm_array_at(gcFrame.sourceDefinition->argumentNodes, i);
        gcFrame.analyzedArgumentNode = sysbvm_interpreter_analyzeASTIfNeededWithEnvironment(context, gcFrame.analyzedArgumentNode, gcFrame.analysisEnvironment);
        sysbvm_array_atPut(gcFrame.analyzedArgumentsNode, i, gcFrame.analyzedArgumentNode);
    }

    if(gcFrame.sourceDefinition->resultTypeNode)
        gcFrame.analyzedResultTypeNode = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.sourceDefinition->resultTypeNode, context->roots.typeType, gcFrame.analysisEnvironment);

    sysbvm_bitflags_t flags = sysbvm_tuple_bitflags_decode((*functionDefinition)->flags) & SYSBVM_FUNCTION_TYPE_FLAGS;
    gcFrame.analyzedType = sysbvm_type_createDependentFunctionType(context, gcFrame.analyzedArgumentsNode, flags, gcFrame.analyzedResultTypeNode,
        gcFrame.analysisEnvironment,
        sysbvm_orderedCollection_asArray(context, gcFrame.analysisEnvironmentObject->captureBindingList),
        sysbvm_orderedCollection_asArray(context, gcFrame.analysisEnvironmentObject->argumentBindingList),
        sysbvm_orderedCollection_asArray(context, gcFrame.analysisEnvironmentObject->localBindingList)
    );
    gcFrame.analyzedType = sysbvm_type_canonicalizeFunctionType(context, gcFrame.analyzedType);

    (*functionDefinition)->type = gcFrame.analyzedType;
    gcFrame.sourceAnalyzedDefinition->environment = gcFrame.analysisEnvironment;
    gcFrame.sourceAnalyzedDefinition->argumentNodes = gcFrame.analyzedArgumentsNode;
    gcFrame.sourceAnalyzedDefinition->resultTypeNode = gcFrame.analyzedResultTypeNode;
    gcFrame.analysisEnvironmentObject->returnTypeExpression = gcFrame.sourceAnalyzedDefinition->resultTypeNode;

    (*functionDefinition)->bytecode = SYSBVM_NULL_TUPLE;
    (*functionDefinition)->nativeCodeDefinition = SYSBVM_NULL_TUPLE;

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
}

static void sysbvm_functionDefinition_analyze(sysbvm_context_t *context, sysbvm_functionDefinition_t **functionDefinition)
{
    sysbvm_functionDefinition_ensureTypeAnalysis(context, functionDefinition);

    struct {
        sysbvm_functionSourceDefinition_t *sourceDefinition;
        sysbvm_functionSourceAnalyzedDefinition_t *sourceAnalyzedDefinition;
        sysbvm_tuple_t analysisEnvironment;
        sysbvm_functionAnalysisEnvironment_t *analysisEnvironmentObject;
        sysbvm_tuple_t analyzedBodyNode;
        sysbvm_tuple_t captureTypes;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourceDefinition = (sysbvm_functionSourceDefinition_t*)(*functionDefinition)->sourceDefinition;
    gcFrame.sourceAnalyzedDefinition = (sysbvm_functionSourceAnalyzedDefinition_t*)(*functionDefinition)->sourceAnalyzedDefinition;

    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.sourceAnalyzedDefinition->sourcePosition);

    gcFrame.analysisEnvironment = gcFrame.sourceAnalyzedDefinition->environment;
    gcFrame.analysisEnvironmentObject = (sysbvm_functionAnalysisEnvironment_t*)gcFrame.analysisEnvironment;

    gcFrame.analyzedBodyNode = sysbvm_interpreter_analyzeASTWithExpectedTypeExpressionWithEnvironmentAt(context, gcFrame.sourceDefinition->bodyNode, gcFrame.sourceAnalyzedDefinition->resultTypeNode, gcFrame.analysisEnvironment, gcFrame.sourceAnalyzedDefinition->sourcePosition, NULL);

    gcFrame.sourceAnalyzedDefinition->captures = sysbvm_orderedCollection_asArray(context, gcFrame.analysisEnvironmentObject->captureBindingList);
    gcFrame.sourceAnalyzedDefinition->arguments = sysbvm_orderedCollection_asArray(context, gcFrame.analysisEnvironmentObject->argumentBindingList);
    gcFrame.sourceAnalyzedDefinition->locals = sysbvm_orderedCollection_asArray(context, gcFrame.analysisEnvironmentObject->localBindingList);

    (*functionDefinition)->pragmas = sysbvm_orderedCollection_asArray(context, gcFrame.analysisEnvironmentObject->pragmaList);
    (*functionDefinition)->innerFunctions = sysbvm_orderedCollection_asArray(context, gcFrame.analysisEnvironmentObject->innerFunctionList);
    (*functionDefinition)->primitiveName = gcFrame.analysisEnvironmentObject->primitiveName;

    gcFrame.sourceAnalyzedDefinition->bodyNode = gcFrame.analyzedBodyNode;

    size_t captureCount = sysbvm_array_getSize(gcFrame.sourceAnalyzedDefinition->captures);
    gcFrame.captureTypes = sysbvm_array_create(context, captureCount);
    for(size_t i = 0; i < captureCount; ++i)
    {
        sysbvm_tuple_t captureType = sysbvm_symbolBinding_getType(sysbvm_array_at(gcFrame.sourceAnalyzedDefinition->captures, i));
        if(!captureType)
            captureType = context->roots.untypedType;
        sysbvm_array_atPut(gcFrame.captureTypes, i, captureType);
    }

    (*functionDefinition)->captureVectorType = sysbvm_type_createSequenceTupleType(context, gcFrame.captureTypes);

    (*functionDefinition)->bytecode = SYSBVM_NULL_TUPLE;
    (*functionDefinition)->nativeCodeDefinition = SYSBVM_NULL_TUPLE;
    if(!sysbvm_tuple_boolean_decode(gcFrame.analysisEnvironmentObject->keepSourceDefinition))
        (*functionDefinition)->sourceDefinition = SYSBVM_NULL_TUPLE;

    sysbvm_functionBytecodeDirectCompiler_compileFunctionDefinition(context, (*functionDefinition));

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
}

SYSBVM_API void sysbvm_functionDefinition_ensureAnalysis(sysbvm_context_t *context, sysbvm_functionDefinition_t **functionDefinition)
{
    // Make sure this is a valid function definition for analysis.
    if(!sysbvm_tuple_isKindOf(context, (sysbvm_tuple_t)*functionDefinition, context->roots.functionDefinitionType))
        return;

    // Is it already analyzed?
    if( ((*functionDefinition)->bytecode && ((sysbvm_functionBytecode_t*)(*functionDefinition)->bytecode)->captures) ||
        ((*functionDefinition)->sourceAnalyzedDefinition && ((sysbvm_functionSourceAnalyzedDefinition_t*)(*functionDefinition)->sourceAnalyzedDefinition)->captures) )
        return;

    sysbvm_functionSourceDefinition_t *sourceDefinition = (sysbvm_functionSourceDefinition_t *)(*functionDefinition)->sourceDefinition;
    if(!sourceDefinition->argumentNodes || !sourceDefinition->bodyNode)
        return;

    sysbvm_functionDefinition_analyze(context, functionDefinition);
}

SYSBVM_API void sysbvm_functionDefinition_ensureTypeAnalysis(sysbvm_context_t *context, sysbvm_functionDefinition_t **functionDefinition)
{
    // Make sure this is a valid function definition for analysis.
    if(!sysbvm_tuple_isKindOf(context, (sysbvm_tuple_t)*functionDefinition, context->roots.functionDefinitionType))
        return;

    sysbvm_functionSourceDefinition_t *sourceDefinition = (sysbvm_functionSourceDefinition_t *)(*functionDefinition)->sourceDefinition;
    if(!sourceDefinition->argumentNodes || !sourceDefinition->bodyNode)
        return;

    // Is it already analyzed?
    if((*functionDefinition)->type)
        return;

    sysbvm_functionDefinition_analyzeType(context, functionDefinition);
}

SYSBVM_API void sysbvm_function_ensureAnalysis(sysbvm_context_t *context, sysbvm_function_t **function)
{
    // Make sure this is an actual function
    if(!sysbvm_tuple_isFunction(context, (sysbvm_tuple_t)*function))
        return;

    // Does it require an analysis?
    if(!(*function)->captureEnvironment || !(*function)->definition)
        return;

    if((*function)->captureEnvironment == SYSBVM_PENDING_MEMOIZATION_VALUE)
        sysbvm_error("Function ensure analysis cycle.");

    // Ensure the function definition analysis.
    struct {
        sysbvm_functionDefinition_t *functionDefinition;
        sysbvm_tuple_t captureList;
        sysbvm_tuple_t captureEnvironment;
        sysbvm_tuple_t captureVector;
        sysbvm_tuple_t captureBinding;
        sysbvm_tuple_t captureValue;
    } gcFrame = {
        .functionDefinition = (sysbvm_functionDefinition_t*)(*function)->definition,
        .captureEnvironment = (*function)->captureEnvironment,
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    (*function)->captureEnvironment = SYSBVM_PENDING_MEMOIZATION_VALUE;
    sysbvm_functionDefinition_ensureAnalysis(context, &gcFrame.functionDefinition);

    // Create the actual capture vector.
    if(gcFrame.functionDefinition->bytecode)
    {
        gcFrame.captureList = ((sysbvm_functionBytecode_t*)gcFrame.functionDefinition->bytecode)->captures;
    }
    else
    {
        SYSBVM_ASSERT(gcFrame.functionDefinition->sourceAnalyzedDefinition);
        gcFrame.captureList = ((sysbvm_functionSourceAnalyzedDefinition_t*)gcFrame.functionDefinition->sourceAnalyzedDefinition)->captures;
    }

    size_t captureVectorSize = sysbvm_array_getSize(gcFrame.captureList);
    gcFrame.captureVector = sysbvm_sequenceTuple_create(context, gcFrame.functionDefinition->captureVectorType);
    for(size_t i = 0; i < captureVectorSize; ++i)
    {
        gcFrame.captureBinding = sysbvm_symbolCaptureBinding_getSourceBinding(sysbvm_array_at(gcFrame.captureList, i));
        gcFrame.captureValue = sysbvm_environment_evaluateSymbolBinding(context, gcFrame.captureEnvironment, gcFrame.captureBinding);
        sysbvm_array_atPut(gcFrame.captureVector, i, gcFrame.captureValue);
    }

    (*function)->captureVector = gcFrame.captureVector;
    (*function)->captureEnvironment = SYSBVM_NULL_TUPLE;
    if(!(*function)->primitiveName)
        (*function)->primitiveName = gcFrame.functionDefinition->primitiveName;
    sysbvm_tuple_setType((sysbvm_object_tuple_t*)*function, gcFrame.functionDefinition->type);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
}

static sysbvm_tuple_t sysbvm_functionDefinition_primitiveEnsureTypeAnalysis(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_functionDefinition_t **functionDefinition = (sysbvm_functionDefinition_t **)&arguments[0];
    sysbvm_functionDefinition_ensureTypeAnalysis(context, functionDefinition);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_functionDefinition_primitiveEnsureAnalysis(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_functionDefinition_t **functionDefinition = (sysbvm_functionDefinition_t **)&arguments[0];
    sysbvm_functionDefinition_ensureAnalysis(context, functionDefinition);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_function_primitiveEnsureAnalysis(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_function_t **function = (sysbvm_function_t **)&arguments[0];
    sysbvm_function_ensureAnalysis(context, function);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_astLambdaNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astLambdaNode_t *lambdaNode;
        sysbvm_tuple_t analyzedNameExpression;
        sysbvm_tuple_t argumentNode;
        sysbvm_tuple_t argumentCount;
        sysbvm_functionDefinition_t *functionDefinition;
        sysbvm_tuple_t capturelessFunction;
        sysbvm_tuple_t capturelessLiteral;
        sysbvm_tuple_t localBinding;
        sysbvm_tuple_t name;
        sysbvm_tuple_t captureList;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.lambdaNode = (sysbvm_astLambdaNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.lambdaNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.lambdaNode->super.sourcePosition);

    if(gcFrame.lambdaNode->name)
    {
        gcFrame.analyzedNameExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.lambdaNode->name, context->roots.symbolType, *environment);
        gcFrame.lambdaNode->name = gcFrame.analyzedNameExpression;
        if(!sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedNameExpression))
            sysbvm_error("Local lambda analyzed name must be a literal node.");
        gcFrame.name = sysbvm_astLiteralNode_getValue(gcFrame.analyzedNameExpression);
    }

    // Count the actual argument count.
    size_t lambdaArgumentCount = 0;
    size_t argumentNodeCount = sysbvm_array_getSize(gcFrame.lambdaNode->arguments);
    for(size_t i = 0; i < argumentNodeCount; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at(gcFrame.lambdaNode->arguments, i);
        if(!sysbvm_astArgumentNode_isForAll(gcFrame.argumentNode))
            ++lambdaArgumentCount;
    }

    gcFrame.argumentCount = sysbvm_tuple_size_encode(context, lambdaArgumentCount);
    gcFrame.functionDefinition = (sysbvm_functionDefinition_t *)sysbvm_functionDefinition_create(context,
        gcFrame.lambdaNode->super.sourcePosition, gcFrame.lambdaNode->flags, gcFrame.lambdaNode->callingConventionName,
        gcFrame.argumentCount, *environment,
        gcFrame.lambdaNode->arguments, gcFrame.lambdaNode->resultType, gcFrame.lambdaNode->body
    );
    gcFrame.lambdaNode->functionDefinition = (sysbvm_tuple_t)gcFrame.functionDefinition;

    // Register the inner function.
    sysbvm_analysisEnvironment_addInnerFunction(context, *environment, (sysbvm_tuple_t)gcFrame.functionDefinition);

    // Perform the lambda analysis.
    sysbvm_functionDefinition_ensureAnalysis(context, &gcFrame.functionDefinition);

    gcFrame.lambdaNode->super.analyzedType = gcFrame.functionDefinition->type;
    if(gcFrame.functionDefinition->bytecode)
    {
        gcFrame.captureList = ((sysbvm_functionBytecode_t*)gcFrame.functionDefinition->bytecode)->captures;
    }
    else
    {
        SYSBVM_ASSERT(gcFrame.functionDefinition->sourceAnalyzedDefinition);
        gcFrame.captureList = ((sysbvm_functionSourceAnalyzedDefinition_t*)gcFrame.functionDefinition->sourceAnalyzedDefinition)->captures;
    }

    // Optimize lambdas without captures by turning them onto a literal.
    if(sysbvm_array_getSize(gcFrame.captureList) == 0)
    {
        gcFrame.capturelessFunction = sysbvm_function_createClosureWithCaptureVector(context, (sysbvm_tuple_t)gcFrame.functionDefinition, sysbvm_sequenceTuple_create(context, gcFrame.functionDefinition->captureVectorType));
        gcFrame.capturelessLiteral = sysbvm_astLiteralNode_create(context, gcFrame.lambdaNode->super.sourcePosition, gcFrame.capturelessFunction);
        if(gcFrame.name)
            gcFrame.localBinding = sysbvm_analysisEnvironment_setNewValueBinding(context, *environment, gcFrame.lambdaNode->super.sourcePosition, gcFrame.name, gcFrame.capturelessFunction);

        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.capturelessLiteral;
    }

    if(gcFrame.name)
    {
        gcFrame.localBinding = sysbvm_analysisEnvironment_setNewSymbolLocalBinding(context, *environment, gcFrame.lambdaNode->super.sourcePosition, gcFrame.name, gcFrame.lambdaNode->super.analyzedType);
        gcFrame.lambdaNode->binding = gcFrame.localBinding;
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.lambdaNode;
}

static sysbvm_tuple_t sysbvm_astLambdaNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astLambdaNode_t **lambdaNode = (sysbvm_astLambdaNode_t**)node;

    sysbvm_functionDefinition_t *functionDefinition = (sysbvm_functionDefinition_t*)(*lambdaNode)->functionDefinition;
    sysbvm_functionSourceAnalyzedDefinition_t *sourceAnalyzedDefinition = (sysbvm_functionSourceAnalyzedDefinition_t*)functionDefinition->sourceAnalyzedDefinition;

    size_t captureVectorSize = sysbvm_array_getSize(sourceAnalyzedDefinition->captures);
    sysbvm_tuple_t captureVector = sysbvm_array_create(context, captureVectorSize);
    for(size_t i = 0; i < captureVectorSize; ++i)
    {
        sysbvm_tuple_t captureBinding = sysbvm_symbolCaptureBinding_getSourceBinding(sysbvm_array_at(sourceAnalyzedDefinition->captures, i));
        sysbvm_tuple_t captureValue = sysbvm_environment_evaluateSymbolBinding(context, *environment, captureBinding);
        sysbvm_array_atPut(captureVector, i, captureValue);
    }

    sysbvm_tuple_t lambdaClosure = sysbvm_function_createClosureWithCaptureVector(context, (*lambdaNode)->functionDefinition, captureVector);
    if((*lambdaNode)->binding)
        sysbvm_functionActivationEnvironment_setBindingActivationValue(context, *environment, (*lambdaNode)->binding, lambdaClosure, (*lambdaNode)->super.sourcePosition);
    return lambdaClosure;
}

static sysbvm_tuple_t sysbvm_astLambdaNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_tuple_t argumentNode;
        sysbvm_tuple_t argumentCount;
        sysbvm_functionDefinition_t* functionDefinition;
        
        sysbvm_tuple_t captureList;
        sysbvm_tuple_t captureVector;
        sysbvm_tuple_t captureBinding;
        sysbvm_tuple_t captureValue;
        sysbvm_tuple_t closure;
        sysbvm_tuple_t name;
    } gcFrame = {0};

    sysbvm_astLambdaNode_t **lambdaNode = (sysbvm_astLambdaNode_t**)node;
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*lambdaNode)->super.sourcePosition);

    // Count the actual argument count.
    size_t lambdaArgumentCount = 0;
    size_t argumentNodeCount = sysbvm_array_getSize((*lambdaNode)->arguments);
    for(size_t i = 0; i < argumentNodeCount; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at((*lambdaNode)->arguments, i);
        if(!sysbvm_astArgumentNode_isForAll(gcFrame.argumentNode))
            ++lambdaArgumentCount;
    }

    gcFrame.argumentCount = sysbvm_tuple_size_encode(context,  lambdaArgumentCount);
    gcFrame.functionDefinition = (sysbvm_functionDefinition_t*)sysbvm_functionDefinition_create(context,
        (*lambdaNode)->super.sourcePosition, (*lambdaNode)->flags, (*lambdaNode)->callingConventionName,
        gcFrame.argumentCount, *environment,
        (*lambdaNode)->arguments, (*lambdaNode)->resultType, (*lambdaNode)->body
    );
    
    if((*lambdaNode)->hasLazyAnalysis && !sysbvm_tuple_boolean_decode((*lambdaNode)->hasLazyAnalysis))
    {
        sysbvm_functionDefinition_ensureAnalysis(context, &gcFrame.functionDefinition);

        if(gcFrame.functionDefinition->bytecode)
        {
            gcFrame.captureList = ((sysbvm_functionBytecode_t*)gcFrame.functionDefinition->bytecode)->captures;
        }
        else
        {
            SYSBVM_ASSERT(gcFrame.functionDefinition->sourceAnalyzedDefinition);
            gcFrame.captureList = ((sysbvm_functionSourceAnalyzedDefinition_t*)gcFrame.functionDefinition->sourceAnalyzedDefinition)->captures;
        }

        size_t captureVectorSize = sysbvm_array_getSize(gcFrame.captureList);
        gcFrame.captureVector = sysbvm_sequenceTuple_create(context, gcFrame.functionDefinition->captureVectorType);
        for(size_t i = 0; i < captureVectorSize; ++i)
        {
            gcFrame.captureBinding = sysbvm_symbolCaptureBinding_getSourceBinding(sysbvm_array_at(gcFrame.captureList, i));
            gcFrame.captureValue = sysbvm_environment_evaluateSymbolBinding(context, *environment, gcFrame.captureBinding);
            sysbvm_array_atPut(gcFrame.captureVector, i, gcFrame.captureValue);
        }
        gcFrame.closure = sysbvm_function_createClosureWithCaptureVector(context, (sysbvm_tuple_t)gcFrame.functionDefinition, gcFrame.captureVector);
    }
    else
    {
        sysbvm_functionDefinition_ensureTypeAnalysis(context, &gcFrame.functionDefinition);
        gcFrame.closure = sysbvm_function_createClosureWithCaptureEnvironment(context, (sysbvm_tuple_t)gcFrame.functionDefinition, *environment);
        sysbvm_environment_enqueuePendingAnalysis(context, *environment, gcFrame.closure);
    }

    if((*lambdaNode)->name)
    {
        gcFrame.name = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*lambdaNode)->name, *environment);
        if(gcFrame.name)
            sysbvm_environment_setNewSymbolBindingWithValueAtSourcePosition(context, *environment, gcFrame.name, gcFrame.closure, (*lambdaNode)->super.sourcePosition);
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.closure;
}

static sysbvm_tuple_t sysbvm_astLiteralNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_astLiteralNode_t *literalNode = (sysbvm_astLiteralNode_t *)arguments[0];
    //sysbvm_tuple_t *environment = &arguments[1];

    if(literalNode->super.analyzedType)
        return (sysbvm_tuple_t)literalNode;

    sysbvm_astLiteralNode_t *analyzedNode = (sysbvm_astLiteralNode_t*)sysbvm_context_shallowCopy(context, (sysbvm_tuple_t)literalNode);
    analyzedNode->super.analyzedType = sysbvm_tuple_getType(context, analyzedNode->value);
    return (sysbvm_tuple_t)analyzedNode;
}

static sysbvm_tuple_t sysbvm_astLiteralNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t node = arguments[0];

    return ((sysbvm_astLiteralNode_t*)node)->value;
}

static sysbvm_tuple_t sysbvm_astVariableDefinitionNode_letWithPrimitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *name = &arguments[1];
    sysbvm_tuple_t *value = &arguments[2];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    return sysbvm_astVariableDefinitionNode_create(context, sourcePosition, *name, SYSBVM_NULL_TUPLE, *value, false);
}

static sysbvm_tuple_t sysbvm_astVariableDefinitionNode_letTypeWithPrimitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 4) sysbvm_error_argumentCountMismatch(4, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *name = &arguments[1];
    sysbvm_tuple_t *type = &arguments[2];
    sysbvm_tuple_t *value = &arguments[3];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    return sysbvm_astVariableDefinitionNode_create(context, sourcePosition, *name, *type, *value, false);
}

static sysbvm_tuple_t sysbvm_astVariableDefinitionNode_letMutableWithPrimitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *name = &arguments[1];
    sysbvm_tuple_t *value = &arguments[2];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    return sysbvm_astVariableDefinitionNode_create(context, sourcePosition, *name, SYSBVM_NULL_TUPLE, *value, true);
}

static sysbvm_tuple_t sysbvm_astVariableDefinitionNode_letTypeMutableWithPrimitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 4) sysbvm_error_argumentCountMismatch(4, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *name = &arguments[1];
    sysbvm_tuple_t *type = &arguments[2];
    sysbvm_tuple_t *value = &arguments[3];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    return sysbvm_astVariableDefinitionNode_create(context, sourcePosition, *name, *type, *value, true);
}

static sysbvm_tuple_t sysbvm_astVariableDefinitionNode_macroLetWithPrimitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *name = &arguments[1];
    sysbvm_tuple_t *value = &arguments[2];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    return sysbvm_astVariableDefinitionNode_createMacro(context, sourcePosition, *name, SYSBVM_NULL_TUPLE, *value);
}

static sysbvm_tuple_t sysbvm_astVariableDefinitionNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *nameOrLambdaSignature = &arguments[1];
    sysbvm_tuple_t *valueOrBodyNodes = &arguments[2];

    struct {
        sysbvm_tuple_t nameNode;
        sysbvm_tuple_t valueNode;
        sysbvm_tuple_t sourcePosition;
        sysbvm_tuple_t lambdaSignatureElements;
        sysbvm_tuple_t argumentsNode;
        sysbvm_tuple_t resultTypeNode;
        sysbvm_tuple_t arguments;
        sysbvm_tuple_t pragmas;
        sysbvm_tuple_t bodyNodes;
        sysbvm_tuple_t bodySequence;
        sysbvm_tuple_t nameExpression;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);

    if(sysbvm_astNode_isIdentifierReferenceNode(context, *nameOrLambdaSignature))
    {
        if(sysbvm_array_getSize(*valueOrBodyNodes) != 1)
            sysbvm_error("Expected a single value for a local define.");

        gcFrame.nameNode = *nameOrLambdaSignature;
        gcFrame.valueNode = sysbvm_array_at(*valueOrBodyNodes, 0);
    }
    else if(sysbvm_astNode_isUnexpandedSExpressionNode(context, *nameOrLambdaSignature))
    {
        gcFrame.lambdaSignatureElements = sysbvm_astUnexpandedSExpressionNode_getElements(*nameOrLambdaSignature);
        if(sysbvm_array_getSize(gcFrame.lambdaSignatureElements) < 1)
            sysbvm_error("Expected function definition requires a name.");

        gcFrame.nameNode = sysbvm_array_at(gcFrame.lambdaSignatureElements, 0);
        if(!sysbvm_astNode_isIdentifierReferenceNode(context, gcFrame.nameNode))
            sysbvm_error("Expected an identifier reference node for the name.");

        bool hasVariadicArguments = false;
        gcFrame.argumentsNode = sysbvm_array_fromOffset(context, gcFrame.lambdaSignatureElements, 1);
        gcFrame.arguments = sysbvm_astLambdaNode_parseArgumentsNodes(context, gcFrame.argumentsNode, &hasVariadicArguments, &gcFrame.resultTypeNode);
        gcFrame.pragmas = sysbvm_array_create(context, 0);
        gcFrame.bodyNodes = *valueOrBodyNodes;
        gcFrame.bodySequence = sysbvm_astSequenceNode_create(context, gcFrame.sourcePosition, gcFrame.pragmas, gcFrame.bodyNodes);
        gcFrame.valueNode = sysbvm_astLambdaNode_create(context, gcFrame.sourcePosition,
            sysbvm_tuple_bitflags_encode(hasVariadicArguments ? SYSBVM_FUNCTION_FLAGS_VARIADIC : SYSBVM_FUNCTION_FLAGS_NONE),
            gcFrame.arguments, gcFrame.resultTypeNode, gcFrame.bodySequence);
    }
    else
    {
        sysbvm_error("Invalid usage of (define)");
    }

    gcFrame.nameExpression = sysbvm_astLiteralNode_create(context, sysbvm_astNode_getSourcePosition(gcFrame.nameNode), sysbvm_astIdentifierReferenceNode_getValue(gcFrame.nameNode));
    gcFrame.result = sysbvm_astVariableDefinitionNode_create(context, gcFrame.sourcePosition, gcFrame.nameExpression, SYSBVM_NULL_TUPLE, gcFrame.valueNode, false);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astVariableDefinitionNode_primitiveDefineMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *nameOrLambdaSignature = &arguments[1];
    sysbvm_tuple_t *valueOrBodyNodes = &arguments[2];

    struct {
        sysbvm_tuple_t nameNode;
        sysbvm_tuple_t valueNode;
        sysbvm_tuple_t sourcePosition;
        sysbvm_tuple_t lambdaSignatureElements;
        sysbvm_tuple_t argumentsNode;
        sysbvm_tuple_t arguments;
        sysbvm_tuple_t resultTypeNode;
        sysbvm_tuple_t pragmas;
        sysbvm_tuple_t bodyNodes;
        sysbvm_tuple_t bodySequence;
        sysbvm_tuple_t nameExpression;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);

    if(sysbvm_astNode_isUnexpandedSExpressionNode(context, *nameOrLambdaSignature))
    {
        gcFrame.lambdaSignatureElements = sysbvm_astUnexpandedSExpressionNode_getElements(*nameOrLambdaSignature);
        if(sysbvm_array_getSize(gcFrame.lambdaSignatureElements) < 1)
            sysbvm_error("Expected function definition requires a name.");

        gcFrame.nameNode = sysbvm_array_at(gcFrame.lambdaSignatureElements, 0);
        if(!sysbvm_astNode_isIdentifierReferenceNode(context, gcFrame.nameNode))
            sysbvm_error("Expected an identifier reference node for the name.");

        bool hasVariadicArguments = false;
        gcFrame.argumentsNode = sysbvm_array_fromOffset(context, gcFrame.lambdaSignatureElements, 1);
        gcFrame.arguments = sysbvm_astLambdaNode_parseArgumentsNodes(context, gcFrame.argumentsNode, &hasVariadicArguments, &gcFrame.resultTypeNode);
        gcFrame.pragmas = sysbvm_array_create(context, 0);
        gcFrame.bodyNodes = *valueOrBodyNodes;
        gcFrame.bodySequence = sysbvm_astSequenceNode_create(context, gcFrame.sourcePosition, gcFrame.pragmas, gcFrame.bodyNodes);
        gcFrame.valueNode = sysbvm_astLambdaNode_create(context, gcFrame.sourcePosition,
            sysbvm_tuple_bitflags_encode((hasVariadicArguments ? SYSBVM_FUNCTION_FLAGS_VARIADIC : SYSBVM_FUNCTION_FLAGS_NONE) | SYSBVM_FUNCTION_FLAGS_MACRO),
            gcFrame.arguments, gcFrame.resultTypeNode, gcFrame.bodySequence);
    }
    else
    {
        sysbvm_error("Invalid usage of (define)");
    }

    gcFrame.nameExpression = sysbvm_astLiteralNode_create(context, sysbvm_astNode_getSourcePosition(gcFrame.nameNode), sysbvm_astIdentifierReferenceNode_getValue(gcFrame.nameNode));
    gcFrame.result = sysbvm_astVariableDefinitionNode_create(context, gcFrame.sourcePosition, gcFrame.nameExpression, SYSBVM_NULL_TUPLE, gcFrame.valueNode, false);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astVariableDefinitionNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astVariableDefinitionNode_t *variableDefinitionNode;
        sysbvm_tuple_t analyzedNameExpression;
        sysbvm_tuple_t analyzedTypeExpression;
        sysbvm_tuple_t analyzedValueExpression;
        
        sysbvm_tuple_t localBinding;
        sysbvm_tuple_t name;
        sysbvm_tuple_t type;
        sysbvm_tuple_t value;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.variableDefinitionNode = (sysbvm_astVariableDefinitionNode_t*)sysbvm_context_shallowCopy(context, *node);

    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.variableDefinitionNode->super.sourcePosition);
    gcFrame.variableDefinitionNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);

    gcFrame.analyzedNameExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.variableDefinitionNode->nameExpression, context->roots.symbolType, *environment);
    gcFrame.variableDefinitionNode->nameExpression = gcFrame.analyzedNameExpression;

    if(sysbvm_tuple_boolean_decode(gcFrame.variableDefinitionNode->isMacroSymbol))
    {
        sysbvm_environment_setNewMacroValueBinding(context, *environment, gcFrame.variableDefinitionNode->super.sourcePosition, gcFrame.name, gcFrame.variableDefinitionNode->valueExpression);
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return sysbvm_astLiteralNode_create(context, gcFrame.variableDefinitionNode->super.sourcePosition, SYSBVM_VOID_TUPLE);
    }

    if(gcFrame.variableDefinitionNode->typeExpression)
    {
        gcFrame.analyzedTypeExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.variableDefinitionNode->typeExpression, context->roots.typeType, *environment);

        if(sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedTypeExpression))
            gcFrame.type = sysbvm_astLiteralNode_getValue(gcFrame.analyzedTypeExpression);
    }

    gcFrame.analyzedValueExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeExpressionWithEnvironmentAt(context, gcFrame.variableDefinitionNode->valueExpression, gcFrame.analyzedTypeExpression, *environment, gcFrame.variableDefinitionNode->super.sourcePosition, &gcFrame.type);
    gcFrame.variableDefinitionNode->valueExpression = gcFrame.analyzedValueExpression;

    // Fallback to the type of the analyzed value.
    if(!gcFrame.type && !gcFrame.variableDefinitionNode->typeExpression)
        gcFrame.type = sysbvm_astNode_getAnalyzedType(gcFrame.analyzedValueExpression);
    gcFrame.variableDefinitionNode->analyzedValueType = gcFrame.type;

    bool isMutable = sysbvm_tuple_boolean_decode(gcFrame.variableDefinitionNode->isMutable);
    if(isMutable)
        gcFrame.type = sysbvm_type_createMemberReferenceType(context, gcFrame.type);

    gcFrame.variableDefinitionNode->typeExpression = SYSBVM_NULL_TUPLE;
    gcFrame.variableDefinitionNode->super.analyzedType = gcFrame.type;
    if(!gcFrame.analyzedNameExpression)
    {
        if(!gcFrame.analyzedValueExpression)
            gcFrame.analyzedValueExpression = sysbvm_astLiteralNode_create(context, gcFrame.variableDefinitionNode->super.sourcePosition, SYSBVM_NULL_TUPLE);

        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.analyzedValueExpression;
    }

    if(!sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedNameExpression))
        sysbvm_error("Local definition analyzed name must be a literal node.");

    gcFrame.name = sysbvm_astLiteralNode_getValue(gcFrame.analyzedNameExpression);
    if(!isMutable &&
        (!gcFrame.analyzedValueExpression || sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedValueExpression)))
    {
        if(gcFrame.analyzedValueExpression)
            gcFrame.value = sysbvm_astLiteralNode_getValue(gcFrame.analyzedValueExpression);
        gcFrame.analyzedValueExpression = sysbvm_astLiteralNode_create(context, gcFrame.variableDefinitionNode->super.sourcePosition, gcFrame.value);
        gcFrame.localBinding = sysbvm_analysisEnvironment_setNewValueBinding(context, *environment, gcFrame.variableDefinitionNode->super.sourcePosition, gcFrame.name, gcFrame.value);
        gcFrame.variableDefinitionNode->binding = gcFrame.localBinding;

        // Replace the node by its literal value.
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.analyzedValueExpression;
    }
    else
    {
        gcFrame.localBinding = sysbvm_analysisEnvironment_setNewSymbolLocalBinding(context, *environment, gcFrame.variableDefinitionNode->super.sourcePosition, gcFrame.name, gcFrame.type);
        gcFrame.variableDefinitionNode->binding = gcFrame.localBinding;
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.variableDefinitionNode;
}

static sysbvm_tuple_t sysbvm_astVariableDefinitionNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_tuple_t type;
        sysbvm_tuple_t value;
    } gcFrame = {
        .value = SYSBVM_NULL_TUPLE
    };

    sysbvm_astVariableDefinitionNode_t **variableDefinitionNode = (sysbvm_astVariableDefinitionNode_t**)node;

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*variableDefinitionNode)->super.sourcePosition);

    if((*variableDefinitionNode)->valueExpression)
        gcFrame.value = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*variableDefinitionNode)->valueExpression, *environment);

    bool isMutable = sysbvm_tuple_boolean_decode((*variableDefinitionNode)->isMutable);
    if(isMutable)
        gcFrame.value = sysbvm_referenceType_withBoxForValue(context, (*variableDefinitionNode)->super.analyzedType, gcFrame.value);

    sysbvm_functionActivationEnvironment_setBindingActivationValue(context, *environment, (*variableDefinitionNode)->binding, gcFrame.value, (*variableDefinitionNode)->super.sourcePosition);
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.value;
}

static sysbvm_tuple_t sysbvm_astVariableDefinitionNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_tuple_t name;
        sysbvm_tuple_t type;
        sysbvm_variableValueBox_t *valueBox;
        sysbvm_tuple_t value;
        sysbvm_tuple_t binding;
        sysbvm_tuple_t ownerProgramEntitySymbol;
        sysbvm_tuple_t ownerProgramEntity;
    } gcFrame = {
        .value = SYSBVM_NULL_TUPLE
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    sysbvm_astVariableDefinitionNode_t **variableDefinitionNode = (sysbvm_astVariableDefinitionNode_t**)node;
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*variableDefinitionNode)->super.sourcePosition);

    gcFrame.name = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*variableDefinitionNode)->nameExpression, *environment);
    if(sysbvm_tuple_boolean_decode((*variableDefinitionNode)->isMacroSymbol))
    {
        sysbvm_environment_setNewMacroValueBinding(context, *environment, (*variableDefinitionNode)->super.sourcePosition, gcFrame.name, (*variableDefinitionNode)->valueExpression);
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return SYSBVM_VOID_TUPLE;
    }

    if((*variableDefinitionNode)->typeExpression)
        gcFrame.type = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*variableDefinitionNode)->typeExpression, *environment);
    if((*variableDefinitionNode)->valueExpression)
        gcFrame.value = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*variableDefinitionNode)->valueExpression, *environment);
    if(gcFrame.type)
        gcFrame.value = sysbvm_type_coerceValue(context, gcFrame.type, gcFrame.value);

    bool isMutable = sysbvm_tuple_boolean_decode((*variableDefinitionNode)->isMutable);
    bool isExternal = sysbvm_tuple_boolean_decode((*variableDefinitionNode)->isExternal);
    bool isThreadLocal = sysbvm_tuple_boolean_decode((*variableDefinitionNode)->isThreadLocal);
    if(isMutable)
    {
        if(!gcFrame.type)
            gcFrame.type = sysbvm_tuple_getType(context, gcFrame.value);
        gcFrame.type = sysbvm_type_createGenericReferenceType(context, gcFrame.type);
        gcFrame.valueBox = (sysbvm_variableValueBox_t*)sysbvm_variableValueBox_with(context, gcFrame.value);
        gcFrame.value = sysbvm_referenceType_withStorage(context, gcFrame.type, (sysbvm_tuple_t)gcFrame.valueBox);
    }

    bool isPublic = sysbvm_tuple_boolean_decode((*variableDefinitionNode)->isPublic);
    if(isPublic)
    {
        if(!sysbvm_environment_lookSymbolRecursively(context, *environment, sysbvm_symbol_internWithCString(context, "__OwnerProgramEntity__"), &gcFrame.ownerProgramEntitySymbol))
            sysbvm_error("Failed to find __OwnerProgramEntity__");
        if(!sysbvm_symbolBinding_isValue(context, gcFrame.ownerProgramEntitySymbol))
            sysbvm_error("Expected a symbol value binding for __OwnerProgramEntity__");

        gcFrame.ownerProgramEntity = sysbvm_symbolValueBinding_getValue(gcFrame.ownerProgramEntitySymbol);
        if(sysbvm_tuple_isKindOf(context, gcFrame.ownerProgramEntity, context->roots.environmentType))
            gcFrame.binding = sysbvm_environment_setNewSymbolBindingWithValueAndFlagsAtSourcePosition(context, gcFrame.ownerProgramEntity, gcFrame.name, gcFrame.value, isMutable, isExternal, isThreadLocal, (*variableDefinitionNode)->super.sourcePosition);
        else
            sysbvm_error("TODO: Set public symbol in non-environment owner.");
    }
    else
    {
        gcFrame.binding = sysbvm_environment_setNewSymbolBindingWithValueAndFlagsAtSourcePosition(context, *environment, gcFrame.name, gcFrame.value, isMutable, isExternal, isThreadLocal, (*variableDefinitionNode)->super.sourcePosition);
    }

    // Store a copy of the binding itself in the value box.
    if(isMutable && gcFrame.binding && gcFrame.valueBox)
        gcFrame.valueBox->binding = gcFrame.binding;

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.value;
}

static sysbvm_tuple_t sysbvm_astIdentifierReferenceNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astIdentifierReferenceNode_t **referenceNode = (sysbvm_astIdentifierReferenceNode_t**)node;

    struct {
        sysbvm_astIdentifierReferenceNode_t *analyzedNode;
        sysbvm_tuple_t expansionNode;
        sysbvm_tuple_t binding;
        sysbvm_tuple_t symbolString;
        sysbvm_tuple_t errorMessage;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*referenceNode)->super.sourcePosition);

    // Attempt to replace the symbol with its binding, if it exists.
    if(sysbvm_analysisEnvironment_lookSymbolRecursively(context, *environment, (*referenceNode)->value, &gcFrame.binding))
    {
        if(sysbvm_symbolBinding_isValue(context, gcFrame.binding))
        {
            // Preserve the references into mutable bindings.
            bool isMutable = sysbvm_tuple_boolean_decode(((sysbvm_symbolValueBinding_t*)gcFrame.binding)->isMutable);
            if(!isMutable)
            {
                SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
                SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
                return sysbvm_astLiteralNode_create(context, (*referenceNode)->super.sourcePosition, sysbvm_symbolValueBinding_getValue(gcFrame.binding));
            }
        }
        else if(sysbvm_symbolBinding_isMacroValue(context, gcFrame.binding))
        {
            gcFrame.expansionNode = sysbvm_symbolMacroValueBinding_getExpansion(gcFrame.binding);
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            return sysbvm_interpreter_analyzeASTWithCurrentExpectedTypeWithEnvironment(context, gcFrame.expansionNode, *environment);
        }
    }
    else
    {
        gcFrame.symbolString = sysbvm_tuple_printString(context, (*referenceNode)->value);
        gcFrame.errorMessage = sysbvm_string_concat(context, sysbvm_string_createWithCString(context, "Failed to find symbol binding for "), gcFrame.symbolString);
        sysbvm_errorWithMessageTuple(gcFrame.errorMessage);
    }

    gcFrame.analyzedNode = (sysbvm_astIdentifierReferenceNode_t*)sysbvm_context_shallowCopy(context, (sysbvm_tuple_t)*referenceNode);
    gcFrame.analyzedNode->binding = gcFrame.binding;
    gcFrame.analyzedNode->super.analyzedType = sysbvm_symbolBinding_getType(gcFrame.binding);
    gcFrame.analyzedNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    return (sysbvm_tuple_t)gcFrame.analyzedNode;
}

static sysbvm_tuple_t sysbvm_astIdentifierReferenceNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astIdentifierReferenceNode_t **referenceNode = (sysbvm_astIdentifierReferenceNode_t**)node;

    struct {
        sysbvm_tuple_t binding;
        sysbvm_tuple_t expansionNode;
        sysbvm_tuple_t symbolString;
        sysbvm_tuple_t errorMessage;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*referenceNode)->super.sourcePosition);

    if(sysbvm_environment_lookSymbolRecursively(context, *environment, (*referenceNode)->value, &gcFrame.binding))
    {
        if(sysbvm_symbolBinding_isValue(context, gcFrame.binding))
        {
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            return sysbvm_symbolValueBinding_getValue(gcFrame.binding);
        }
        else if(sysbvm_symbolBinding_isMacroValue(context, gcFrame.binding))
        {
            gcFrame.expansionNode = sysbvm_symbolMacroValueBinding_getExpansion(gcFrame.binding);
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            return sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expansionNode, *environment);
        }
    }

    gcFrame.symbolString = sysbvm_tuple_printString(context, (*referenceNode)->value);
    gcFrame.errorMessage = sysbvm_string_concat(context, sysbvm_string_createWithCString(context, "Failed to find symbol binding for "), gcFrame.symbolString);
    sysbvm_errorWithMessageTuple(gcFrame.errorMessage);
    return SYSBVM_NULL_TUPLE;
}

static sysbvm_tuple_t sysbvm_astIdentifierReferenceNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astIdentifierReferenceNode_t **referenceNode = (sysbvm_astIdentifierReferenceNode_t**)node;
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*referenceNode)->super.sourcePosition);

    sysbvm_tuple_t result = sysbvm_environment_evaluateSymbolBinding(context, *environment, (*referenceNode)->binding);
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    return result;
}

static sysbvm_tuple_t sysbvm_astIfNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 4) sysbvm_error_argumentCountMismatch(4, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *conditionNode = &arguments[1];
    sysbvm_tuple_t *trueExpressionNode = &arguments[2];
    sysbvm_tuple_t *falseExpressionNode = &arguments[3];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);

    return sysbvm_astIfNode_create(context, sourcePosition, *conditionNode, *trueExpressionNode, *falseExpressionNode);
}

static sysbvm_tuple_t sysbvm_astIfNode_primitiveMacroIfThen(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *conditionNode = &arguments[1];
    sysbvm_tuple_t *trueExpressionNode = &arguments[2];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);

    return sysbvm_astIfNode_create(context, sourcePosition, *conditionNode, *trueExpressionNode, SYSBVM_NULL_TUPLE);
}

static sysbvm_tuple_t sysbvm_astIfNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astIfNode_t *ifNode;
        sysbvm_tuple_t analyzedCondition;
        sysbvm_tuple_t analyzedTrueExpression;
        sysbvm_tuple_t analyzedTrueExpressionType;
        sysbvm_tuple_t analyzedFalseExpression;
        sysbvm_tuple_t analyzedFalseExpressionType;

        sysbvm_tuple_t analyzedTakenBranch;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.ifNode = (sysbvm_astIfNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.ifNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.ifNode->super.sourcePosition);

    gcFrame.analyzedCondition = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.ifNode->conditionExpression, context->roots.booleanType, *environment);
    gcFrame.ifNode->conditionExpression = gcFrame.analyzedCondition;
    if(gcFrame.ifNode->trueExpression)
    {
        gcFrame.analyzedTrueExpression = sysbvm_interpreter_analyzeASTWithCurrentExpectedTypeWithEnvironment(context, gcFrame.ifNode->trueExpression, *environment);
        gcFrame.ifNode->trueExpression = gcFrame.analyzedTrueExpression;
    }
    if(gcFrame.ifNode->falseExpression)
    {
        gcFrame.analyzedFalseExpression = sysbvm_interpreter_analyzeASTWithCurrentExpectedTypeWithEnvironment(context, gcFrame.ifNode->falseExpression, *environment);
        gcFrame.ifNode->falseExpression = gcFrame.analyzedFalseExpression;
    }

    if(sysbvm_astNode_isLiteralNode(context, gcFrame.ifNode->conditionExpression))
    {
        bool conditionValue = sysbvm_tuple_boolean_decode(sysbvm_astLiteralNode_getValue(gcFrame.ifNode->conditionExpression));
        gcFrame.analyzedTakenBranch = conditionValue ? gcFrame.ifNode->trueExpression : gcFrame.ifNode->falseExpression;
        if(!gcFrame.analyzedTakenBranch)
            gcFrame.analyzedTakenBranch = sysbvm_astLiteralNode_create(context, gcFrame.ifNode->super.sourcePosition, SYSBVM_VOID_TUPLE);
        
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.analyzedTakenBranch;
    }

    // Require the same, otherwise fallback to void.
    gcFrame.ifNode->super.analyzedType = context->roots.anyValueType;
    if(gcFrame.analyzedTrueExpression && gcFrame.analyzedFalseExpression)
    {
        gcFrame.analyzedTrueExpressionType = sysbvm_astNode_getAnalyzedType(gcFrame.analyzedTrueExpression);
        gcFrame.analyzedFalseExpressionType = sysbvm_astNode_getAnalyzedType(gcFrame.analyzedFalseExpression);
        bool trueExpressionIsControlFlowEscape = sysbvm_type_isDirectSubtypeOf(gcFrame.analyzedTrueExpressionType, context->roots.controlFlowEscapeType);
        bool trueExpressionIsUndefinedType = gcFrame.analyzedTrueExpressionType == context->roots.undefinedObjectType;

        bool falseExpressionIsControlFlowEscape = sysbvm_type_isDirectSubtypeOf(gcFrame.analyzedFalseExpressionType, context->roots.controlFlowEscapeType);
        bool falseExpressionIsUndefinedType = gcFrame.analyzedFalseExpressionType == context->roots.undefinedObjectType;

        if(trueExpressionIsControlFlowEscape || trueExpressionIsUndefinedType)
        {
            gcFrame.ifNode->super.analyzedType = gcFrame.analyzedFalseExpressionType;
        }
        else if(falseExpressionIsControlFlowEscape || falseExpressionIsUndefinedType)
        {
            gcFrame.ifNode->super.analyzedType = gcFrame.analyzedTrueExpressionType;
        }
        else
        {
            gcFrame.ifNode->super.analyzedType = sysbvm_type_computeLCA(gcFrame.analyzedTrueExpressionType, gcFrame.analyzedFalseExpressionType);
            if(!gcFrame.ifNode->super.analyzedType)
                gcFrame.ifNode->super.analyzedType = context->roots.anyValueType;
        }
    }
    else if(!gcFrame.ifNode->trueExpression || !gcFrame.ifNode->falseExpression)
    {
        gcFrame.ifNode->super.analyzedType = context->roots.voidType;
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.ifNode;
}

static sysbvm_tuple_t sysbvm_astIfNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astIfNode_t **ifNode = (sysbvm_astIfNode_t**)node;

    struct {
        sysbvm_tuple_t condition;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*ifNode)->super.sourcePosition);
    gcFrame.condition = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*ifNode)->conditionExpression, *environment);
    if(sysbvm_tuple_boolean_decode(gcFrame.condition))
    {
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        if(!(*ifNode)->trueExpression)
            return SYSBVM_VOID_TUPLE;
        return sysbvm_interpreter_evaluateASTWithEnvironment(context, (*ifNode)->trueExpression, *environment);
    }
    else
    {
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        if(!(*ifNode)->falseExpression)
            return SYSBVM_VOID_TUPLE;
        return sysbvm_interpreter_evaluateASTWithEnvironment(context, (*ifNode)->falseExpression, *environment);
    }
}

static sysbvm_tuple_t sysbvm_astIfNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astIfNode_t **ifNode = (sysbvm_astIfNode_t**)node;
    struct {
        sysbvm_tuple_t condition;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*ifNode)->super.sourcePosition);
    gcFrame.condition = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*ifNode)->conditionExpression, *environment);
    if(sysbvm_tuple_boolean_decode(gcFrame.condition))
    {
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        if(!(*ifNode)->trueExpression)
            return SYSBVM_VOID_TUPLE;
        return sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*ifNode)->trueExpression, *environment);
    }
    else
    {
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        if(!(*ifNode)->falseExpression)
            return SYSBVM_VOID_TUPLE;
        return sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*ifNode)->falseExpression, *environment);
    }
}

static sysbvm_tuple_t sysbvm_astUnexpandedApplicationNode_expandNodeWithMacro(sysbvm_context_t *context, sysbvm_tuple_t *node, sysbvm_tuple_t *macro, sysbvm_tuple_t *environment)
{
    sysbvm_astUnexpandedApplicationNode_t **unexpandedNode = (sysbvm_astUnexpandedApplicationNode_t**)node;

    size_t applicationArgumentCount = sysbvm_array_getSize((*unexpandedNode)->arguments);

    sysbvm_functionCallFrameStack_t callFrameStack = {0};
    struct {
        sysbvm_tuple_t macroContext;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    sysbvm_functionCallFrameStack_begin(context, &callFrameStack, *macro, 1 + applicationArgumentCount, 0);
    gcFrame.macroContext = sysbvm_macroContext_create(context, *node, (*unexpandedNode)->super.sourcePosition, *environment);
    sysbvm_functionCallFrameStack_push(&callFrameStack, gcFrame.macroContext);
    for(size_t i = 0; i < applicationArgumentCount; ++i)
        sysbvm_functionCallFrameStack_push(&callFrameStack, sysbvm_array_at((*unexpandedNode)->arguments, i));

    SYSBVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return sysbvm_functionCallFrameStack_finish(context, &callFrameStack);
}

static sysbvm_tuple_t sysbvm_astUnexpandedApplicationNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astUnexpandedApplicationNode_t **unexpandedNode = (sysbvm_astUnexpandedApplicationNode_t**)node;

    struct {
        sysbvm_tuple_t macro;
        sysbvm_astUnexpandedApplicationNode_t *unexpandedNode;
        sysbvm_tuple_t functionOrMacroExpression;
        sysbvm_tuple_t functionOrMacroExpressionType;
        sysbvm_tuple_t analysisFunction;
        sysbvm_tuple_t expansionResult;
        sysbvm_tuple_t applicationNode;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*unexpandedNode)->super.sourcePosition);

    gcFrame.functionOrMacroExpression = sysbvm_interpreter_analyzeASTWithDirectTypeWithEnvironment(context, (*unexpandedNode)->functionOrMacroExpression, *environment);
    gcFrame.functionOrMacroExpressionType = sysbvm_astNode_getAnalyzedType(gcFrame.functionOrMacroExpression);
    gcFrame.analysisFunction = sysbvm_type_getAnalyzeUnexpandedApplicationNodeWithEnvironmentFunction(context, sysbvm_tuple_getType(context, gcFrame.functionOrMacroExpressionType));
    if(gcFrame.analysisFunction)
    {
        gcFrame.unexpandedNode = (sysbvm_astUnexpandedApplicationNode_t*)sysbvm_context_shallowCopy(context, (sysbvm_tuple_t)*unexpandedNode);
        gcFrame.unexpandedNode->functionOrMacroExpression = gcFrame.functionOrMacroExpression;

        gcFrame.result = sysbvm_function_apply3(context, gcFrame.analysisFunction, gcFrame.functionOrMacroExpressionType, (sysbvm_tuple_t)gcFrame.unexpandedNode, *environment);
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.result;
    }

    // Is this a macro?
    bool isMacro = sysbvm_astNode_isMacroExpression(context, gcFrame.functionOrMacroExpression);
    if(isMacro)
    {
        SYSBVM_ASSERT(sysbvm_astNode_isLiteralNode(context, gcFrame.functionOrMacroExpression));
        gcFrame.macro = sysbvm_astLiteralNode_getValue(gcFrame.functionOrMacroExpression);
        gcFrame.expansionResult = sysbvm_astUnexpandedApplicationNode_expandNodeWithMacro(context, node, &gcFrame.macro, environment);
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return sysbvm_interpreter_analyzeASTWithCurrentExpectedTypeWithEnvironment(context, gcFrame.expansionResult, *environment);
    }

    // Convert into application node and then analyze it.
    gcFrame.applicationNode = sysbvm_astFunctionApplicationNode_create(context, (*unexpandedNode)->super.sourcePosition, gcFrame.functionOrMacroExpression, (*unexpandedNode)->arguments);
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return sysbvm_interpreter_analyzeASTWithCurrentExpectedTypeWithEnvironment(context, gcFrame.applicationNode, *environment);
}

static sysbvm_tuple_t sysbvm_astUnexpandedApplicationNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astUnexpandedApplicationNode_t **unexpandedNode = (sysbvm_astUnexpandedApplicationNode_t**)node;

    struct {
        sysbvm_tuple_t functionOrMacro;
        sysbvm_tuple_t functionOrMacroExpressionType;
        sysbvm_tuple_t analysisFunction;
        sysbvm_tuple_t expansionResult;
        sysbvm_tuple_t argumentNode;
        sysbvm_tuple_t argument;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*unexpandedNode)->super.sourcePosition);

    gcFrame.functionOrMacro = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*unexpandedNode)->functionOrMacroExpression, *environment);
    gcFrame.functionOrMacroExpressionType = sysbvm_tuple_getType(context, gcFrame.functionOrMacro);
    gcFrame.analysisFunction = sysbvm_type_getAnalyzeAndEvaluateUnexpandedApplicationNodeOfWithEnvironmentFunction(context, sysbvm_tuple_getType(context, gcFrame.functionOrMacroExpressionType));
    if(gcFrame.analysisFunction)
    {
        gcFrame.result = sysbvm_function_apply4(context, gcFrame.analysisFunction, gcFrame.functionOrMacroExpressionType, (sysbvm_tuple_t)*unexpandedNode, gcFrame.functionOrMacro, *environment);
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.result;
    }

    bool isMacro = sysbvm_function_isMacro(context, gcFrame.functionOrMacro);
    if(isMacro)
    {
        gcFrame.expansionResult = sysbvm_astUnexpandedApplicationNode_expandNodeWithMacro(context, node, &gcFrame.functionOrMacro, environment);
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expansionResult, *environment);
    }

    sysbvm_functionCallFrameStack_t callFrameStack = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    size_t applicationArgumentCount = sysbvm_array_getSize((*unexpandedNode)->arguments);
    sysbvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.functionOrMacro, applicationArgumentCount, 0);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at((*unexpandedNode)->arguments, i);
        gcFrame.argument = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
        sysbvm_functionCallFrameStack_push(&callFrameStack, gcFrame.argument);
    }

    //SYSBVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return sysbvm_functionCallFrameStack_finish(context, &callFrameStack);
}

static sysbvm_tuple_t sysbvm_astUnexpandedSExpressionNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astUnexpandedSExpressionNode_t **unexpandedSExpressionNode = (sysbvm_astUnexpandedSExpressionNode_t**)node;

    struct {
        sysbvm_tuple_t array;
        sysbvm_tuple_t literalNode;
        sysbvm_tuple_t functionOrMacroExpression;
        sysbvm_tuple_t argumentExpressions;
        sysbvm_tuple_t unexpandedApplicationNode;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*unexpandedSExpressionNode)->super.sourcePosition);

    size_t elementCount = sysbvm_array_getSize((*unexpandedSExpressionNode)->elements);
    if(elementCount == 0)
    {
        // Empty array.
        gcFrame.array = sysbvm_array_create(context, 0);
        gcFrame.literalNode = sysbvm_astLiteralNode_create(context, (*unexpandedSExpressionNode)->super.sourcePosition, gcFrame.array);
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return sysbvm_interpreter_analyzeASTWithCurrentExpectedTypeWithEnvironment(context, gcFrame.literalNode, *environment);
    }
    else
    {
        // Unexpanded application node.
        gcFrame.functionOrMacroExpression = sysbvm_array_at((*unexpandedSExpressionNode)->elements, 0);
        gcFrame.argumentExpressions = sysbvm_array_fromOffset(context, (*unexpandedSExpressionNode)->elements, 1);
        gcFrame.unexpandedApplicationNode = sysbvm_astUnexpandedApplicationNode_create(context, (*unexpandedSExpressionNode)->super.sourcePosition, gcFrame.functionOrMacroExpression, gcFrame.argumentExpressions);
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return sysbvm_interpreter_analyzeASTWithCurrentExpectedTypeWithEnvironment(context, gcFrame.unexpandedApplicationNode, *environment);
    }
}

static sysbvm_tuple_t sysbvm_astUnexpandedSExpressionNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astUnexpandedSExpressionNode_t **unexpandedSExpressionNode = (sysbvm_astUnexpandedSExpressionNode_t**)node;

    struct {
        sysbvm_tuple_t functionOrMacroExpression;
        sysbvm_tuple_t argumentExpressions;
        sysbvm_tuple_t unexpandedApplicationNode;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*unexpandedSExpressionNode)->super.sourcePosition);

    size_t elementCount = sysbvm_array_getSize((*unexpandedSExpressionNode)->elements);
    if(elementCount == 0)
    {
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return sysbvm_array_create(context, 0);
    }
    else
    {
        // Unexpanded application node.
        gcFrame.functionOrMacroExpression = sysbvm_array_at((*unexpandedSExpressionNode)->elements, 0);
        gcFrame.argumentExpressions = sysbvm_array_fromOffset(context, (*unexpandedSExpressionNode)->elements, 1);
        gcFrame.unexpandedApplicationNode = sysbvm_astUnexpandedApplicationNode_create(context, (*unexpandedSExpressionNode)->super.sourcePosition, gcFrame.functionOrMacroExpression, gcFrame.argumentExpressions);
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.unexpandedApplicationNode, *environment);
    }
}

static sysbvm_tuple_t sysbvm_astPragmaNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astPragmaNode_t *pragmaNode;
        sysbvm_tuple_t analyzedSelector;
        sysbvm_tuple_t argumentNode;
        sysbvm_tuple_t analyzedArgument;
        sysbvm_tuple_t analyzedArguments;

        sysbvm_tuple_t literalPragmaSelector;
        sysbvm_tuple_t literalPragmaArguments;
        sysbvm_tuple_t literalPragma;
        sysbvm_tuple_t literalPragmaNode;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.pragmaNode = (sysbvm_astPragmaNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.pragmaNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);

    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.pragmaNode->super.sourcePosition);

    gcFrame.analyzedSelector = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.pragmaNode->selector, context->roots.symbolType, *environment);
    bool isLiteral = sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedSelector);

    size_t pragmaArgumentCount = sysbvm_array_getSize(gcFrame.pragmaNode->arguments);
    gcFrame.analyzedArguments = sysbvm_array_create(context, pragmaArgumentCount);
    for(size_t i = 0; i < pragmaArgumentCount; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at(gcFrame.pragmaNode->arguments, i);
        gcFrame.analyzedArgument = sysbvm_interpreter_analyzeASTWithDecayedTypeWithEnvironment(context, gcFrame.argumentNode, *environment);
        isLiteral = isLiteral && sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedArgument);
        sysbvm_array_atPut(gcFrame.analyzedArguments, i, gcFrame.analyzedArgument);
    }

    if(isLiteral)
    {
        gcFrame.literalPragmaSelector = sysbvm_astLiteralNode_getValue(gcFrame.analyzedSelector);
        gcFrame.literalPragmaArguments = sysbvm_array_create(context, pragmaArgumentCount);
        for(size_t i = 0; i < pragmaArgumentCount; ++i)
            sysbvm_array_atPut(gcFrame.literalPragmaArguments, i, sysbvm_astLiteralNode_getValue(sysbvm_array_at(gcFrame.analyzedArguments, i)));
        
        gcFrame.literalPragma = sysbvm_pragma_create(context, gcFrame.literalPragmaSelector, gcFrame.literalPragmaArguments);
        gcFrame.literalPragmaNode = sysbvm_astLiteralNode_create(context, gcFrame.pragmaNode->super.sourcePosition, gcFrame.literalPragma);
        sysbvm_analysisEnvironment_addPragma(context, *environment, gcFrame.literalPragma);
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return (sysbvm_tuple_t)gcFrame.literalPragmaNode;
    }

    gcFrame.pragmaNode->arguments = gcFrame.analyzedArguments;
    gcFrame.pragmaNode->super.analyzedType = context->roots.pragmaType;
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.pragmaNode;
}


static sysbvm_tuple_t sysbvm_astFunctionApplicationNode_optimizePureApplication(sysbvm_context_t *context, sysbvm_astFunctionApplicationNode_t **applicationNode)
{
    struct {
        sysbvm_tuple_t argumentNode;
        
        sysbvm_tuple_t literalFunction;
        sysbvm_tuple_t pureCallResult;
        sysbvm_tuple_t argumentValue;
        sysbvm_tuple_t functionType;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    bool canOptimizeCall = false;
    if(sysbvm_astNode_isLiteralNode(context, (*applicationNode)->functionExpression))
    {
        gcFrame.literalFunction = sysbvm_astLiteralNode_getValue((*applicationNode)->functionExpression);
        canOptimizeCall = sysbvm_function_isPure(context, gcFrame.literalFunction);
    }

    size_t applicationArgumentCount = sysbvm_array_getSize((*applicationNode)->arguments);
    for(size_t i = 0; canOptimizeCall && i < applicationArgumentCount ; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at((*applicationNode)->arguments, i);
        canOptimizeCall = canOptimizeCall && sysbvm_astNode_isLiteralNode(context, gcFrame.argumentNode);
    }

    if(!canOptimizeCall)
    {
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return (sysbvm_tuple_t)*applicationNode;
    }

    // Optimize pure function call
    sysbvm_functionCallFrameStack_t callFrameStack = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    sysbvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.literalFunction, applicationArgumentCount, sysbvm_tuple_bitflags_decode((*applicationNode)->applicationFlags));

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at((*applicationNode)->arguments, i);
        gcFrame.argumentValue = sysbvm_astLiteralNode_getValue(gcFrame.argumentNode);
        sysbvm_functionCallFrameStack_push(&callFrameStack, gcFrame.argumentValue);
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    gcFrame.pureCallResult = sysbvm_functionCallFrameStack_finish(context, &callFrameStack);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return sysbvm_astLiteralNode_create(context, (*applicationNode)->super.sourcePosition, gcFrame.pureCallResult);
}

static sysbvm_tuple_t sysbvm_astFunctionApplicationNode_defaultTypeCheck(sysbvm_context_t *context, sysbvm_astFunctionApplicationNode_t **applicationNode, sysbvm_tuple_t *environment)
{
    struct {
        sysbvm_tuple_t argumentNode;
        sysbvm_tuple_t analyzedArgument;
        sysbvm_tuple_t analyzedArguments;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t applicationArgumentCount = sysbvm_array_getSize((*applicationNode)->arguments);
    gcFrame.analyzedArguments = sysbvm_array_create(context, applicationArgumentCount);
    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at((*applicationNode)->arguments, i);
        gcFrame.analyzedArgument = sysbvm_interpreter_analyzeASTWithDecayedTypeWithEnvironment(context, gcFrame.argumentNode, *environment);
        sysbvm_array_atPut(gcFrame.analyzedArguments, i, gcFrame.analyzedArgument);
    }

    (*applicationNode)->arguments = gcFrame.analyzedArguments;
    (*applicationNode)->super.analyzedType = context->roots.anyValueType;

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return sysbvm_astFunctionApplicationNode_optimizePureApplication(context, applicationNode);
}

static sysbvm_tuple_t sysbvm_astFunctionApplicationNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astFunctionApplicationNode_t *applicationNode;
        sysbvm_tuple_t analyzedFunctionExpression;
        sysbvm_tuple_t argumentNode;
        sysbvm_tuple_t analyzedArgument;
        sysbvm_tuple_t analyzedArguments;
        
        sysbvm_tuple_t literalFunction;
        sysbvm_tuple_t pureCallResult;
        sysbvm_tuple_t argumentValue;
        sysbvm_tuple_t functionType;

        sysbvm_tuple_t typeCheckFunction;

        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.applicationNode = (sysbvm_astFunctionApplicationNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.applicationNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);

    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.applicationNode->super.sourcePosition);

    gcFrame.analyzedFunctionExpression = sysbvm_interpreter_analyzeASTWithDecayedTypeWithEnvironment(context, gcFrame.applicationNode->functionExpression, *environment);
    gcFrame.applicationNode->functionExpression = gcFrame.analyzedFunctionExpression;
    gcFrame.functionType = sysbvm_astNode_getAnalyzedType(gcFrame.analyzedFunctionExpression);

    // Type check the function application.
    gcFrame.typeCheckFunction = sysbvm_type_getAnalyzeAndTypeCheckFunctionApplicationNodeWithEnvironment(context, sysbvm_tuple_getType(context, gcFrame.functionType));
    if(gcFrame.typeCheckFunction)
        gcFrame.result = sysbvm_function_apply3(context, gcFrame.typeCheckFunction, gcFrame.functionType, (sysbvm_tuple_t)gcFrame.applicationNode, *environment);
    else
        gcFrame.result = sysbvm_astFunctionApplicationNode_defaultTypeCheck(context, &gcFrame.applicationNode, environment);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astFunctionApplicationNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astFunctionApplicationNode_t **applicationNode = (sysbvm_astFunctionApplicationNode_t**)node;

    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*applicationNode)->super.sourcePosition);

    sysbvm_functionCallFrameStack_t callFrameStack = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    size_t applicationArgumentCount = sysbvm_array_getSize((*applicationNode)->arguments);
    sysbvm_functionCallFrameStack_begin(context, &callFrameStack, sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*applicationNode)->functionExpression, *environment), applicationArgumentCount, 0);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        sysbvm_tuple_t argumentNode = sysbvm_array_at((*applicationNode)->arguments, i);
        sysbvm_functionCallFrameStack_push(&callFrameStack, sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, argumentNode, *environment));
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    sysbvm_tuple_t result = sysbvm_functionCallFrameStack_finish(context, &callFrameStack);
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    return result;
}

static sysbvm_tuple_t sysbvm_astFunctionApplicationNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astFunctionApplicationNode_t **applicationNode = (sysbvm_astFunctionApplicationNode_t**)node;
    struct {
        sysbvm_tuple_t function;
        sysbvm_tuple_t argumentNode;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*applicationNode)->super.sourcePosition);

    size_t applicationArgumentCount = sysbvm_array_getSize((*applicationNode)->arguments);
    gcFrame.function = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*applicationNode)->functionExpression, *environment);

    sysbvm_functionCallFrameStack_t callFrameStack = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);
    sysbvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.function, applicationArgumentCount, sysbvm_tuple_bitflags_decode((*applicationNode)->applicationFlags));

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at((*applicationNode)->arguments, i);
        sysbvm_functionCallFrameStack_push(&callFrameStack, sysbvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment));
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    gcFrame.result = sysbvm_functionCallFrameStack_finish(context, &callFrameStack);
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astLexicalBlockNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astLexicalBlockNode_t *lexicalBlockNode;
        sysbvm_tuple_t childEnvironment;
        sysbvm_tuple_t analyzedBodyNode;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.lexicalBlockNode = (sysbvm_astLexicalBlockNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.lexicalBlockNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);

    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.lexicalBlockNode->super.sourcePosition);

    gcFrame.childEnvironment = sysbvm_localAnalysisEnvironment_create(context, *environment);
    gcFrame.analyzedBodyNode = sysbvm_interpreter_analyzeASTWithCurrentExpectedTypeWithEnvironment(context, gcFrame.lexicalBlockNode->body, gcFrame.childEnvironment);
    gcFrame.lexicalBlockNode->body = gcFrame.analyzedBodyNode;
    gcFrame.lexicalBlockNode->bodyEnvironment = gcFrame.childEnvironment;
    gcFrame.lexicalBlockNode->super.analyzedType = sysbvm_astNode_getAnalyzedType(gcFrame.analyzedBodyNode);
    
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.lexicalBlockNode;
}

static sysbvm_tuple_t sysbvm_astLexicalBlockNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astLexicalBlockNode_t **lexicalBlockNode = (sysbvm_astLexicalBlockNode_t **)node;

    struct {
        sysbvm_tuple_t childEnvironment;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*lexicalBlockNode)->super.sourcePosition);
    
    gcFrame.childEnvironment = sysbvm_analysisAndEvaluationEnvironment_create(context, *environment);
    gcFrame.result = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*lexicalBlockNode)->body, gcFrame.childEnvironment);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astLexicalBlockNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astLexicalBlockNode_t **lexicalBlockNode = (sysbvm_astLexicalBlockNode_t **)node;

    return sysbvm_interpreter_evaluateASTWithEnvironment(context, (*lexicalBlockNode)->body, *environment);
}

static sysbvm_tuple_t sysbvm_astMakeAssociationNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMakeAssociationNode_t **associationNode = (sysbvm_astMakeAssociationNode_t**)node;

    struct {
        sysbvm_tuple_t key;
        sysbvm_tuple_t value;
        sysbvm_tuple_t result;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*associationNode)->super.sourcePosition);

    gcFrame.key = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*associationNode)->key, *environment);
    if((*associationNode)->value)
        gcFrame.value = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*associationNode)->value, *environment);
    gcFrame.result = sysbvm_association_create(context, gcFrame.key, gcFrame.value);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMakeAssociationNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMakeAssociationNode_t **associationNode = (sysbvm_astMakeAssociationNode_t**)node;

    struct {
        sysbvm_astMakeAssociationNode_t *analyzedAssociationNode;
        sysbvm_tuple_t analyzedKey;
        sysbvm_tuple_t analyzedValue;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*associationNode)->super.sourcePosition);

    gcFrame.analyzedAssociationNode = (sysbvm_astMakeAssociationNode_t *)sysbvm_context_shallowCopy(context, *node);
    gcFrame.analyzedAssociationNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);

    gcFrame.analyzedKey = sysbvm_interpreter_analyzeASTWithDecayedTypeWithEnvironment(context, gcFrame.analyzedAssociationNode->key, *environment);
    gcFrame.analyzedAssociationNode->key = gcFrame.analyzedKey;
    if(gcFrame.analyzedAssociationNode->value)
    {
        gcFrame.analyzedValue = sysbvm_interpreter_analyzeASTWithDecayedTypeWithEnvironment(context, gcFrame.analyzedAssociationNode->value, *environment);
        gcFrame.analyzedAssociationNode->value = gcFrame.analyzedValue;
    }

    gcFrame.analyzedAssociationNode->super.analyzedType = context->roots.associationType;
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.analyzedAssociationNode;
}

static sysbvm_tuple_t sysbvm_astMakeAssociationNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMakeAssociationNode_t **associationNode = (sysbvm_astMakeAssociationNode_t**)node;

    struct {
        sysbvm_tuple_t key;
        sysbvm_tuple_t value;
        sysbvm_tuple_t result;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*associationNode)->super.sourcePosition);

    gcFrame.key = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*associationNode)->key, *environment);
    if((*associationNode)->value)
        gcFrame.value = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*associationNode)->value, *environment);
    gcFrame.result = sysbvm_association_create(context, gcFrame.key, gcFrame.value);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMakeByteArrayNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMakeByteArrayNode_t **tupleNode = (sysbvm_astMakeByteArrayNode_t**)node;

    struct {
        sysbvm_astMakeByteArrayNode_t *analyzedTupleNode;
        sysbvm_tuple_t expressions;
        sysbvm_tuple_t analyzedExpressions;
        sysbvm_tuple_t expression;
        sysbvm_tuple_t analyzedExpression;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*tupleNode)->super.sourcePosition);

    gcFrame.expressions = (*tupleNode)->elements;
    size_t expressionCount = sysbvm_array_getSize(gcFrame.expressions);
    if(expressionCount == 0 && (*tupleNode)->super.analyzedType)
    {
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return *node;
    }

    gcFrame.analyzedTupleNode = (sysbvm_astMakeByteArrayNode_t *)sysbvm_context_shallowCopy(context, *node);
    gcFrame.analyzedTupleNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
    
    gcFrame.analyzedExpressions = sysbvm_array_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = sysbvm_array_at(gcFrame.expressions, i);
        gcFrame.analyzedExpression = sysbvm_interpreter_analyzeASTWithDecayedTypeWithEnvironment(context, gcFrame.expression, *environment);
        sysbvm_array_atPut(gcFrame.analyzedExpressions, i, gcFrame.analyzedExpression);
    }

    gcFrame.analyzedTupleNode->super.analyzedType = context->roots.byteArrayType;
    gcFrame.analyzedTupleNode->elements = gcFrame.analyzedExpressions;
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.analyzedTupleNode;
}

static sysbvm_tuple_t sysbvm_astMakeByteArrayNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMakeByteArrayNode_t **byteArrayNode = (sysbvm_astMakeByteArrayNode_t**)node;

    struct {
        sysbvm_tuple_t result;
        sysbvm_tuple_t element;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*byteArrayNode)->super.sourcePosition);

    size_t expressionCount = sysbvm_array_getSize((*byteArrayNode)->elements);
    gcFrame.result = sysbvm_byteArray_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        sysbvm_tuple_t expression = sysbvm_array_at((*byteArrayNode)->elements, i);
        gcFrame.element = sysbvm_interpreter_evaluateASTWithEnvironment(context, expression, *environment);
        sysbvm_arrayOrByteArray_atPut(gcFrame.result, i, gcFrame.element);
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMakeByteArrayNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMakeByteArrayNode_t **byteArrayNode = (sysbvm_astMakeByteArrayNode_t**)node;

    struct {
        sysbvm_tuple_t result;
        sysbvm_tuple_t element;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*byteArrayNode)->super.sourcePosition);

    size_t expressionCount = sysbvm_array_getSize((*byteArrayNode)->elements);
    gcFrame.result = sysbvm_byteArray_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        sysbvm_tuple_t expression = sysbvm_array_at((*byteArrayNode)->elements, i);
        gcFrame.element = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, expression, *environment);
        sysbvm_arrayOrByteArray_atPut(gcFrame.result, i, gcFrame.element);
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMakeArrayNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMakeArrayNode_t **tupleNode = (sysbvm_astMakeArrayNode_t**)node;

    struct {
        sysbvm_astMakeArrayNode_t *analyzedTupleNode;
        sysbvm_tuple_t expressions;
        sysbvm_tuple_t analyzedExpressions;
        sysbvm_tuple_t expression;
        sysbvm_tuple_t analyzedExpression;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*tupleNode)->super.sourcePosition);

    gcFrame.expressions = (*tupleNode)->elements;
    size_t expressionCount = sysbvm_array_getSize(gcFrame.expressions);
    if(expressionCount == 0 && (*tupleNode)->super.analyzedType)
    {
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return *node;
    }

    gcFrame.analyzedTupleNode = (sysbvm_astMakeArrayNode_t *)sysbvm_context_shallowCopy(context, *node);
    gcFrame.analyzedTupleNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
    
    gcFrame.analyzedExpressions = sysbvm_array_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = sysbvm_array_at(gcFrame.expressions, i);
        gcFrame.analyzedExpression = sysbvm_interpreter_analyzeASTWithDecayedTypeWithEnvironment(context, gcFrame.expression, *environment);
        sysbvm_array_atPut(gcFrame.analyzedExpressions, i, gcFrame.analyzedExpression);
    }

    gcFrame.analyzedTupleNode->super.analyzedType = context->roots.arrayType;
    gcFrame.analyzedTupleNode->elements = gcFrame.analyzedExpressions;
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.analyzedTupleNode;
}

static sysbvm_tuple_t sysbvm_astMakeArrayNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMakeArrayNode_t **tupleNode = (sysbvm_astMakeArrayNode_t**)node;

    struct {
        sysbvm_tuple_t result;
        sysbvm_tuple_t element;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*tupleNode)->super.sourcePosition);

    size_t expressionCount = sysbvm_array_getSize((*tupleNode)->elements);
    gcFrame.result = sysbvm_array_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        sysbvm_tuple_t expression = sysbvm_array_at((*tupleNode)->elements, i);
        gcFrame.element = sysbvm_interpreter_evaluateASTWithEnvironment(context, expression, *environment);
        sysbvm_array_atPut(gcFrame.result, i, gcFrame.element);
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMakeDictionaryNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMakeDictionaryNode_t **dictionaryNode = (sysbvm_astMakeDictionaryNode_t**)node;

    struct {
        sysbvm_tuple_t result;
        sysbvm_tuple_t expression;
        sysbvm_tuple_t element;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*dictionaryNode)->super.sourcePosition);

    size_t expressionCount = sysbvm_array_getSize((*dictionaryNode)->elements);
    gcFrame.result = sysbvm_dictionary_createWithCapacity(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = sysbvm_array_at((*dictionaryNode)->elements, i);
        gcFrame.element = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expression, *environment);
        sysbvm_dictionary_add(context, gcFrame.result, gcFrame.element);
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMakeDictionaryNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMakeDictionaryNode_t **dictionaryNode = (sysbvm_astMakeDictionaryNode_t**)node;

    struct {
        sysbvm_astMakeDictionaryNode_t *analyzedDictionaryNode;
        sysbvm_tuple_t expressions;
        sysbvm_tuple_t analyzedExpressions;
        sysbvm_tuple_t expression;
        sysbvm_tuple_t analyzedExpression;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*dictionaryNode)->super.sourcePosition);

    gcFrame.expressions = (*dictionaryNode)->elements;
    size_t expressionCount = sysbvm_array_getSize(gcFrame.expressions);
    if(expressionCount == 0 && (*dictionaryNode)->super.analyzedType)
    {
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return *node;
    }

    gcFrame.analyzedDictionaryNode = (sysbvm_astMakeDictionaryNode_t *)sysbvm_context_shallowCopy(context, *node);
    gcFrame.analyzedDictionaryNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
    
    gcFrame.analyzedExpressions = sysbvm_array_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = sysbvm_array_at(gcFrame.expressions, i);
        gcFrame.analyzedExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.expression, context->roots.associationType, *environment);
        sysbvm_array_atPut(gcFrame.analyzedExpressions, i, gcFrame.analyzedExpression);
    }

    gcFrame.analyzedDictionaryNode->super.analyzedType = context->roots.arrayType;
    gcFrame.analyzedDictionaryNode->elements = gcFrame.analyzedExpressions;
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.analyzedDictionaryNode;
}

static sysbvm_tuple_t sysbvm_astMakeDictionaryNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMakeDictionaryNode_t **dictionaryNode = (sysbvm_astMakeDictionaryNode_t**)node;

    struct {
        sysbvm_tuple_t result;
        sysbvm_tuple_t element;
        sysbvm_tuple_t expression;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*dictionaryNode)->super.sourcePosition);

    size_t expressionCount = sysbvm_array_getSize((*dictionaryNode)->elements);
    gcFrame.result = sysbvm_dictionary_createWithCapacity(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = sysbvm_array_at((*dictionaryNode)->elements, i);
        gcFrame.element = sysbvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.expression, *environment);
        sysbvm_dictionary_add(context, gcFrame.result, gcFrame.element);
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMakeArrayNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMakeArrayNode_t **tupleNode = (sysbvm_astMakeArrayNode_t**)node;

    struct {
        sysbvm_tuple_t result;
        sysbvm_tuple_t expression;
        sysbvm_tuple_t element;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*tupleNode)->super.sourcePosition);

    size_t expressionCount = sysbvm_array_getSize((*tupleNode)->elements);
    gcFrame.result = sysbvm_array_create(context, expressionCount);
    for(size_t i = 0; i < expressionCount; ++i)
    {
        gcFrame.expression = sysbvm_array_at((*tupleNode)->elements, i);
        gcFrame.element = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expression, *environment);
        sysbvm_array_atPut(gcFrame.result, i, gcFrame.element);
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMessageChainMessageNode_expandToMessageWithReceiver(sysbvm_context_t *context, sysbvm_tuple_t chainMessageNode, sysbvm_tuple_t receiver, sysbvm_tuple_t receiverLookupType)
{
    if(!sysbvm_astNode_isMessageChainMessageNode(context, chainMessageNode))
        sysbvm_error("Expected a message chain message node.");

    sysbvm_astMessageChainMessageNode_t *chainMessageNodeObject = (sysbvm_astMessageChainMessageNode_t*)chainMessageNode;
    sysbvm_astMessageSendNode_t *messageSendNode = (sysbvm_astMessageSendNode_t*)sysbvm_astMessageSendNode_create(context, chainMessageNodeObject->super.sourcePosition, receiver, chainMessageNodeObject->selector, chainMessageNodeObject->arguments);
    messageSendNode->receiverLookupType = receiverLookupType;
    return (sysbvm_tuple_t)messageSendNode;
}

static sysbvm_tuple_t sysbvm_astMessageChainNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astMessageChainNode_t *chainNode;
        sysbvm_tuple_t analyzedReceiver;
        sysbvm_tuple_t analyzedReceiverTypeExpression;
        sysbvm_tuple_t expandedChainedMessages;
        sysbvm_tuple_t expandedChainedMessageNode;
        sysbvm_tuple_t chainedMessageNode;

        sysbvm_tuple_t receiverType;
        sysbvm_tuple_t receiverMetaType;
        sysbvm_tuple_t analysisFunction;

        sysbvm_tuple_t receiverSourcePosition;
        sysbvm_tuple_t expansionReceiverSymbol;
        sysbvm_tuple_t expansionReceiverExpression;
        sysbvm_tuple_t expansionReceiverIdentifier;
        sysbvm_tuple_t expansionMessageSequence;
        sysbvm_tuple_t expansionLocalAndMessageSequenceArray;

        sysbvm_tuple_t analyzedExpandedChainedMessages;
        sysbvm_tuple_t analyzedExpansion;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    
    gcFrame.chainNode = (sysbvm_astMessageChainNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.chainNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.chainNode->super.sourcePosition);

    size_t chainedMessageCount = sysbvm_array_getSize(gcFrame.chainNode->messages);

    if(gcFrame.chainNode->receiver)
    {
        gcFrame.analyzedReceiver = sysbvm_interpreter_analyzeASTWithDirectTypeWithEnvironment(context, gcFrame.chainNode->receiver, *environment);
        gcFrame.chainNode->receiver = gcFrame.analyzedReceiver;

        // Inline the object with lookup starting from node.
        if(sysbvm_astNode_isTupleWithLookupStartingFromNode(context, gcFrame.analyzedReceiver))
        {
            sysbvm_astTupleWithLookupStartingFromNode_t *objectLookup = (sysbvm_astTupleWithLookupStartingFromNode_t*)gcFrame.analyzedReceiver;
            gcFrame.analyzedReceiver = objectLookup->tupleExpression;
            gcFrame.chainNode->receiver = gcFrame.analyzedReceiver;
            
            gcFrame.analyzedReceiverTypeExpression = objectLookup->typeExpression;
            gcFrame.chainNode->receiverLookupType = gcFrame.analyzedReceiverTypeExpression;
        }

        gcFrame.receiverType = sysbvm_astNode_getAnalyzedType(gcFrame.analyzedReceiver);
        // HACK: remove this literal check when properly implementing this analysis in the target system.
        if(gcFrame.receiverType && sysbvm_astNode_isLiteralNode(context, gcFrame.chainNode->receiver))
        {
            gcFrame.receiverMetaType = sysbvm_tuple_getType(context, gcFrame.receiverType);
            gcFrame.analysisFunction = sysbvm_type_getAnalyzeMessageChainNodeWithEnvironmentFunction(context, gcFrame.receiverMetaType);
            if(gcFrame.analysisFunction)
            {
                SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
                SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
                return sysbvm_function_apply3(context, gcFrame.analysisFunction, gcFrame.receiverType, (sysbvm_tuple_t)gcFrame.chainNode, *environment);
            }
        }

        // Simple cases.
        if(chainedMessageCount == 0)
        {
            SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return gcFrame.analyzedReceiver;
        }
        else if(chainedMessageCount == 1)
        {
            gcFrame.chainedMessageNode = sysbvm_array_at(gcFrame.chainNode->messages, 0);
            gcFrame.expansionMessageSequence = sysbvm_astMessageChainMessageNode_expandToMessageWithReceiver(context, gcFrame.chainedMessageNode, gcFrame.analyzedReceiver, gcFrame.analyzedReceiverTypeExpression);
            SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return sysbvm_interpreter_analyzeASTWithCurrentExpectedTypeWithEnvironment(context, gcFrame.analyzedReceiver, *environment);
        }

        // Do we need to define a variable for the receiver?
        if(sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedReceiver))
        {
            gcFrame.expansionReceiverExpression = gcFrame.analyzedReceiver;
            gcFrame.expansionReceiverIdentifier = gcFrame.analyzedReceiver;
        }
        else
        {
            gcFrame.receiverSourcePosition = sysbvm_astNode_getSourcePosition(gcFrame.analyzedReceiver);
            gcFrame.expansionReceiverSymbol = sysbvm_generatedSymbol_create(context, sysbvm_symbol_internWithCString(context, "<messageChainReceiver>"), gcFrame.receiverSourcePosition);

            gcFrame.expansionReceiverExpression = sysbvm_astVariableDefinitionNode_create(context, gcFrame.receiverSourcePosition, sysbvm_astLiteralNode_create(context, gcFrame.receiverSourcePosition, gcFrame.expansionReceiverSymbol), SYSBVM_NULL_TUPLE, gcFrame.analyzedReceiver, false);
            sysbvm_astVariableDefinitionNode_t *receiverLocalDefinition = (sysbvm_astVariableDefinitionNode_t *)gcFrame.expansionReceiverExpression;
            receiverLocalDefinition->super.analyzedType = gcFrame.receiverType;
            receiverLocalDefinition->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
            receiverLocalDefinition->binding = sysbvm_analysisEnvironment_setNewSymbolLocalBinding(context, *environment, gcFrame.receiverSourcePosition, gcFrame.expansionReceiverSymbol, gcFrame.receiverType);

            gcFrame.expansionReceiverIdentifier = sysbvm_astIdentifierReferenceNode_create(context, gcFrame.receiverSourcePosition, gcFrame.expansionReceiverSymbol);
        }
    }

    // Message sequence.
    gcFrame.expandedChainedMessages = sysbvm_array_create(context, chainedMessageCount);
    gcFrame.chainNode->super.analyzedType = SYSBVM_NULL_TUPLE;
    for(size_t i = 0; i < chainedMessageCount; ++i)
    {
        gcFrame.chainedMessageNode = sysbvm_array_at(gcFrame.chainNode->messages, i);
        gcFrame.expandedChainedMessageNode = sysbvm_astMessageChainMessageNode_expandToMessageWithReceiver(context, gcFrame.chainedMessageNode, gcFrame.expansionReceiverIdentifier, gcFrame.analyzedReceiverTypeExpression);
        sysbvm_array_atPut(gcFrame.expandedChainedMessages, i, gcFrame.expandedChainedMessageNode);
    }
    gcFrame.expansionMessageSequence = sysbvm_astSequenceNode_create(context, gcFrame.chainNode->super.sourcePosition, sysbvm_array_create(context, 0), gcFrame.expandedChainedMessages);
    gcFrame.analyzedExpandedChainedMessages = sysbvm_interpreter_analyzeASTWithCurrentExpectedTypeWithEnvironment(context, gcFrame.expansionMessageSequence, *environment);

    // Local definition and message sequence.
    gcFrame.analyzedExpansion = gcFrame.analyzedExpandedChainedMessages;
    if(gcFrame.expansionReceiverExpression != gcFrame.expansionReceiverIdentifier)
    {
        gcFrame.expansionLocalAndMessageSequenceArray = sysbvm_array_create(context, 2);
        sysbvm_array_atPut(gcFrame.expansionLocalAndMessageSequenceArray, 0, gcFrame.expansionReceiverExpression);
        sysbvm_array_atPut(gcFrame.expansionLocalAndMessageSequenceArray, 1, gcFrame.analyzedExpandedChainedMessages);

        gcFrame.analyzedExpansion = sysbvm_astSequenceNode_create(context, gcFrame.chainNode->super.sourcePosition, sysbvm_array_create(context, 0), gcFrame.expansionLocalAndMessageSequenceArray);
        ((sysbvm_astNode_t*)gcFrame.analyzedExpansion)->analyzedType = sysbvm_astNode_getAnalyzedType(gcFrame.analyzedExpandedChainedMessages);
        ((sysbvm_astNode_t*)gcFrame.analyzedExpansion)->analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
    }
    
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.analyzedExpansion;
}

static sysbvm_tuple_t sysbvm_astMessageChainMessageNode_analyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t chainedMessage, bool hasReceiver, sysbvm_tuple_t *receiver, sysbvm_tuple_t *environment)
{
    struct {
        sysbvm_astMessageChainMessageNode_t *node;
        sysbvm_tuple_t selector;
        sysbvm_tuple_t receiverType;
        sysbvm_tuple_t methodBinding;
        sysbvm_tuple_t method;
        sysbvm_tuple_t result;

        sysbvm_tuple_t message;
        sysbvm_tuple_t arguments;
        sysbvm_tuple_t argumentNode;
        sysbvm_tuple_t argument;
    } gcFrame = {
        .node = (sysbvm_astMessageChainMessageNode_t*)chainedMessage
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.node->super.sourcePosition);

    gcFrame.selector = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.node->selector, *environment);
    size_t applicationArgumentCount = sysbvm_array_getSize(gcFrame.node->arguments);

    if(hasReceiver)
    {
        gcFrame.receiverType = sysbvm_tuple_getType(context, *receiver);
        gcFrame.method = sysbvm_type_lookupSelector(context, gcFrame.receiverType, gcFrame.selector);
        if(!gcFrame.method)
        {
            gcFrame.method = sysbvm_type_lookupSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
            if(!gcFrame.method)
                sysbvm_error("Message not understood.");

            gcFrame.arguments = sysbvm_array_create(context, applicationArgumentCount);
            for(size_t i = 0; i < applicationArgumentCount; ++i)
            {
                gcFrame.argumentNode = sysbvm_array_at(gcFrame.node->arguments, i);
                gcFrame.argument = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
                sysbvm_array_atPut(gcFrame.argument, i, gcFrame.argument);
            }

            gcFrame.message = sysbvm_message_create(context, gcFrame.selector, gcFrame.arguments, gcFrame.node->super.sourcePosition);
            gcFrame.result = sysbvm_function_apply2(context, gcFrame.method, *receiver, gcFrame.message);
            SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return gcFrame.result;
        }
    }
    else
    {
        if(!sysbvm_environment_lookSymbolRecursively(context, *environment, gcFrame.selector, &gcFrame.methodBinding)
            || !sysbvm_symbolBinding_isValue(context, gcFrame.methodBinding))
            sysbvm_error("Failed to find symbol for message send without receiver.");
        gcFrame.method = sysbvm_symbolValueBinding_getValue(gcFrame.methodBinding);
    }


    sysbvm_functionCallFrameStack_t callFrameStack = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    sysbvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.method, applicationArgumentCount + (hasReceiver ? 1 : 0), 0);

    if(hasReceiver)
        sysbvm_functionCallFrameStack_push(&callFrameStack, *receiver);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at(gcFrame.node->arguments, i);
        gcFrame.argument = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
        sysbvm_functionCallFrameStack_push(&callFrameStack, gcFrame.argument);
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    gcFrame.result = sysbvm_functionCallFrameStack_finish(context, &callFrameStack);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMessageChainNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMessageChainNode_t **chainNode = (sysbvm_astMessageChainNode_t**)node;

    struct {
        sysbvm_tuple_t receiver;
        sysbvm_tuple_t receiverType;
        sysbvm_tuple_t analysisFunction;
        sysbvm_tuple_t chainedMessage;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*chainNode)->super.sourcePosition);

    bool hasReceiver = false;
    if((*chainNode)->receiver)
    {
        gcFrame.receiver = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*chainNode)->receiver, *environment);
        hasReceiver = true;

        gcFrame.receiverType = sysbvm_tuple_getType(context, gcFrame.receiver);
        gcFrame.analysisFunction = sysbvm_type_getAnalyzeAndEvaluateMessageChainNodeForReceiverWithEnvironmentFunction(context, sysbvm_tuple_getType(context, gcFrame.receiverType));
        if(gcFrame.analysisFunction)
        {
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return sysbvm_function_apply4(context, gcFrame.analysisFunction, gcFrame.receiverType, *node, gcFrame.receiver, *environment);
        }
    }

    size_t chainedMessageCount = sysbvm_array_getSize((*chainNode)->messages);
    for(size_t i = 0; i < chainedMessageCount; ++i)
    {
        gcFrame.chainedMessage = sysbvm_array_at((*chainNode)->messages, i);
        gcFrame.result = sysbvm_astMessageChainMessageNode_analyzeAndEvaluate(context, gcFrame.chainedMessage, hasReceiver, &gcFrame.receiver, environment);
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMessageChainMessageNode_evaluate(sysbvm_context_t *context, sysbvm_tuple_t chainedMessage, bool hasReceiver, sysbvm_tuple_t *receiver, sysbvm_tuple_t *environment)
{
    struct {
        sysbvm_astMessageChainMessageNode_t *node;
        sysbvm_tuple_t receiverType;
        sysbvm_tuple_t selector;
        sysbvm_tuple_t methodBinding;
        sysbvm_tuple_t method;
        sysbvm_tuple_t arguments;
        sysbvm_tuple_t argumentNode;
        sysbvm_tuple_t argument;
        sysbvm_tuple_t message;
        sysbvm_tuple_t result;
    } gcFrame = {
        .node = (sysbvm_astMessageChainMessageNode_t*)chainedMessage
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.node->super.sourcePosition);

    size_t applicationArgumentCount = sysbvm_array_getSize(gcFrame.node->arguments);
    gcFrame.selector = sysbvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.node->selector, *environment);
    if(hasReceiver)
    {
        gcFrame.receiverType = sysbvm_tuple_getType(context, *receiver);
        gcFrame.method = sysbvm_type_lookupSelector(context, gcFrame.receiverType, gcFrame.selector);
        if(!gcFrame.method)
        {
            gcFrame.method = sysbvm_type_lookupSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
            if(!gcFrame.method)
                sysbvm_error("Message not understood.");

            gcFrame.arguments = sysbvm_array_create(context, applicationArgumentCount);
            for(size_t i = 0; i < applicationArgumentCount; ++i)
            {
                gcFrame.argumentNode = sysbvm_array_at(gcFrame.node->arguments, i);
                gcFrame.argument = sysbvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
                sysbvm_array_atPut(gcFrame.argument, i, gcFrame.argument);
            }

            gcFrame.message = sysbvm_message_create(context, gcFrame.selector, gcFrame.arguments, gcFrame.node->super.sourcePosition);
            gcFrame.result = sysbvm_function_applyNoCheck2(context, gcFrame.method, *receiver, gcFrame.message);
            SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return gcFrame.result;
        }
    }
    else
    {
        if(!sysbvm_environment_lookSymbolRecursively(context, *environment, gcFrame.selector, &gcFrame.methodBinding)
            || !sysbvm_symbolBinding_isValue(context, gcFrame.methodBinding))
            sysbvm_error("Failed to find symbol for message send without receiver.");
        gcFrame.method = sysbvm_symbolValueBinding_getValue(gcFrame.methodBinding);
    }


    sysbvm_functionCallFrameStack_t callFrameStack = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    sysbvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.method, applicationArgumentCount + (hasReceiver ? 1 : 0), 0);

    if(hasReceiver)
        sysbvm_functionCallFrameStack_push(&callFrameStack, *receiver);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at(gcFrame.node->arguments, i);
        gcFrame.argument = sysbvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
        sysbvm_functionCallFrameStack_push(&callFrameStack, gcFrame.argument);
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    gcFrame.result = sysbvm_functionCallFrameStack_finish(context, &callFrameStack);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMessageChainNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMessageChainNode_t **chainNode = (sysbvm_astMessageChainNode_t**)node;

    struct {
        sysbvm_tuple_t receiver;
        sysbvm_tuple_t chainedMessage;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*chainNode)->super.sourcePosition);

    bool hasReceiver = false;
    if((*chainNode)->receiver)
    {
        gcFrame.receiver = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*chainNode)->receiver, *environment);
        hasReceiver = true;
    }

    size_t chainedMessageCount = sysbvm_array_getSize((*chainNode)->messages);
    for(size_t i = 0; i < chainedMessageCount; ++i)
    {
        gcFrame.chainedMessage = sysbvm_array_at((*chainNode)->messages, i);
        gcFrame.result = sysbvm_astMessageChainMessageNode_evaluate(context, gcFrame.chainedMessage, hasReceiver, &gcFrame.receiver, environment);
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astMessageSendNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 4) sysbvm_error_argumentCountMismatch(4, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *selectorNode = &arguments[1];
    sysbvm_tuple_t *receiverNode = &arguments[2];
    sysbvm_tuple_t *argumentNodes = &arguments[3];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    return sysbvm_astMessageSendNode_create(context, sourcePosition, *receiverNode, *selectorNode, *argumentNodes);
}

static sysbvm_tuple_t sysbvm_astMessageSendNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astMessageSendNode_t *sendNode;
        sysbvm_tuple_t analyzedReceiver;
        sysbvm_tuple_t analyzedReceiverTypeExpression;
        sysbvm_tuple_t analyzedSelector;
        sysbvm_tuple_t analyzedArguments;

        sysbvm_tuple_t receiverType;
        sysbvm_tuple_t receiverMetaType;
        sysbvm_tuple_t methodOwner;
        sysbvm_tuple_t selector;
        sysbvm_tuple_t method;
        sysbvm_tuple_t methodType;
        sysbvm_tuple_t analysisFunction;
        sysbvm_tuple_t result;
        sysbvm_tuple_t newMethodNode;
        sysbvm_tuple_t newArgumentNodes;
        sysbvm_tuple_t unexpandedApplicationNode;

        sysbvm_tuple_t argumentNode;
        sysbvm_tuple_t analyzedArgumentNode;

        sysbvm_tuple_t macroContext;
        sysbvm_tuple_t expansionResult;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sendNode = (sysbvm_astMessageSendNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.sendNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.sendNode->super.sourcePosition);

    bool isDoesNotUnderstand = false;
    if(gcFrame.sendNode->receiver)
    {
        gcFrame.analyzedReceiver = sysbvm_interpreter_analyzeASTWithDirectTypeWithEnvironment(context, gcFrame.sendNode->receiver, *environment);
        gcFrame.sendNode->receiver = gcFrame.analyzedReceiver;
        gcFrame.receiverType = sysbvm_astNode_getAnalyzedType(gcFrame.analyzedReceiver);

        // Inline the object with lookup starting from node.
        if(sysbvm_astNode_isTupleWithLookupStartingFromNode(context, gcFrame.analyzedReceiver))
        {
            sysbvm_astTupleWithLookupStartingFromNode_t *objectLookup = (sysbvm_astTupleWithLookupStartingFromNode_t*)gcFrame.analyzedReceiver;
            gcFrame.analyzedReceiver = objectLookup->tupleExpression;
            gcFrame.sendNode->receiver = gcFrame.analyzedReceiver;
            
            gcFrame.analyzedReceiverTypeExpression = objectLookup->typeExpression;
            gcFrame.sendNode->receiverLookupType = gcFrame.analyzedReceiverTypeExpression;
            
        }
        else if(gcFrame.sendNode->receiverLookupType)
        {
            gcFrame.analyzedReceiverTypeExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.sendNode->receiverLookupType, context->roots.typeType, *environment);
            gcFrame.sendNode->receiverLookupType = gcFrame.analyzedReceiverTypeExpression;
        }

        if(gcFrame.receiverType)
        {
            // If the receiver is a literal node, we can attempt to forward the message send node analysis.
            gcFrame.receiverMetaType = sysbvm_tuple_getType(context, gcFrame.receiverType);
            gcFrame.analysisFunction = sysbvm_type_getAnalyzeMessageSendNodeWithEnvironmentFunction(context, gcFrame.receiverMetaType);

            // HACK: Remove this literal check when properly implementing this.
            if(gcFrame.analysisFunction && sysbvm_astNode_isLiteralNode(context, gcFrame.sendNode->receiver))
            {
                gcFrame.result = sysbvm_function_apply3(context, gcFrame.analysisFunction, gcFrame.receiverType, (sysbvm_tuple_t)gcFrame.sendNode, *environment);
                SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
                SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
                return gcFrame.result;
            }

            // If the selector is a literal, attempt to perform a static lookup.
            gcFrame.analyzedSelector = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.sendNode->selector, context->roots.symbolType, *environment);
            gcFrame.sendNode->selector = gcFrame.analyzedSelector;
            if(sysbvm_astNode_isLiteralNode(context, gcFrame.sendNode->selector))
            {
                gcFrame.selector = sysbvm_astLiteralNode_getValue(gcFrame.sendNode->selector);
                gcFrame.methodOwner = gcFrame.receiverType;
                gcFrame.method = sysbvm_type_lookupMacroSelector(context, gcFrame.receiverType, gcFrame.selector);
                if(!gcFrame.method)
                    gcFrame.method = sysbvm_type_lookupSelector(context, gcFrame.receiverType, gcFrame.selector);
                if(!gcFrame.method)
                    gcFrame.method = sysbvm_type_lookupFallbackSelector(context, gcFrame.receiverType, gcFrame.selector);

                if(!gcFrame.method && sysbvm_type_isReferenceType(gcFrame.receiverType))
                {
                    gcFrame.methodOwner = sysbvm_type_decay(context, gcFrame.receiverType);
                    gcFrame.method = sysbvm_type_lookupMacroSelector(context, gcFrame.methodOwner, gcFrame.selector);
                    if(!gcFrame.method)
                        gcFrame.method = sysbvm_type_lookupSelector(context, gcFrame.methodOwner, gcFrame.selector);
                    if(!gcFrame.method)
                        gcFrame.method = sysbvm_type_lookupFallbackSelector(context, gcFrame.methodOwner, gcFrame.selector);
                }

                // does not understand macro?
                if(!gcFrame.method)
                {
                    gcFrame.methodOwner = gcFrame.receiverType;
                    if(!gcFrame.method)
                        gcFrame.method = sysbvm_type_lookupMacroSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
                    if(!gcFrame.method)
                        gcFrame.method = sysbvm_type_lookupSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
                    if(!gcFrame.method)
                        gcFrame.method = sysbvm_type_lookupFallbackSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);

                    if(!sysbvm_function_isMacro(context, gcFrame.method))
                        gcFrame.method = SYSBVM_NULL_TUPLE;
                    isDoesNotUnderstand = gcFrame.method != SYSBVM_NULL_TUPLE;
                }
            }
        }
        else
        {
            gcFrame.analyzedSelector = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.sendNode->selector, context->roots.symbolType, *environment);
            gcFrame.sendNode->selector = gcFrame.analyzedSelector;
            if(sysbvm_astNode_isLiteralNode(context, gcFrame.sendNode->selector))
            {
                gcFrame.selector = sysbvm_astLiteralNode_getValue(gcFrame.sendNode->selector);
                gcFrame.method = sysbvm_type_lookupMacroSelector(context, context->roots.anyValueType, gcFrame.selector);
            }
        }
    }
    else
    {
        gcFrame.analyzedSelector = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.sendNode->selector, context->roots.symbolType, *environment);
        gcFrame.sendNode->selector = gcFrame.analyzedSelector;
    }

    // Does not understand: 
    if(isDoesNotUnderstand && sysbvm_function_isMacro(context, gcFrame.method))
    {
        // Clear the analyzer token.
        gcFrame.macroContext = sysbvm_macroContext_create(context, *node, gcFrame.sendNode->super.sourcePosition, *environment);
        gcFrame.expansionResult = sysbvm_function_apply3(context, gcFrame.method, gcFrame.macroContext, gcFrame.sendNode->receiver, (sysbvm_tuple_t)gcFrame.sendNode);
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);

        // Analyze and evaluate the resulting node.
        return sysbvm_interpreter_analyzeASTWithCurrentExpectedTypeWithEnvironment(context, gcFrame.expansionResult, *environment);
    }

    // Turn this node onto an unexpanded application.
    bool hasExplicitSolvedLookupType = sysbvm_astNode_isLiteralNode(context, gcFrame.sendNode->receiverLookupType);
    if(gcFrame.method && (hasExplicitSolvedLookupType || sysbvm_function_shouldOptimizeLookup(context, gcFrame.method, gcFrame.methodOwner, sysbvm_astNode_isLiteralNode(context, gcFrame.sendNode->receiver))))
    {
        size_t applicationArgumentCount = sysbvm_array_getSize(gcFrame.sendNode->arguments);
        gcFrame.newMethodNode = sysbvm_astLiteralNode_create(context, gcFrame.sendNode->super.sourcePosition, gcFrame.method);

        gcFrame.newArgumentNodes = sysbvm_array_create(context, 1 + applicationArgumentCount);
        sysbvm_array_atPut(gcFrame.newArgumentNodes, 0, gcFrame.sendNode->receiver);
        for(size_t i = 0; i < applicationArgumentCount; ++i)
            sysbvm_array_atPut(gcFrame.newArgumentNodes, i + 1, sysbvm_array_at(gcFrame.sendNode->arguments, i));

        gcFrame.unexpandedApplicationNode = sysbvm_astUnexpandedApplicationNode_create(context, gcFrame.sendNode->super.sourcePosition, gcFrame.newMethodNode, gcFrame.newArgumentNodes);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return sysbvm_interpreter_analyzeASTWithCurrentExpectedTypeWithEnvironment(context, gcFrame.unexpandedApplicationNode, *environment);
    }

    if(gcFrame.method)
    {
        gcFrame.sendNode->boundMethod = gcFrame.method;
        gcFrame.sendNode->boundMethodOwner = gcFrame.methodOwner;
        gcFrame.methodType = sysbvm_tuple_getType(context, gcFrame.method);

        gcFrame.analysisFunction = sysbvm_type_getAnalyzeAndTypeCheckMessageSendNodeWithEnvironment(context, sysbvm_tuple_getType(context, gcFrame.methodType));
        if(gcFrame.analysisFunction)
        {
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return sysbvm_function_apply3(context, gcFrame.analysisFunction, gcFrame.methodType, (sysbvm_tuple_t)gcFrame.sendNode, *environment);
        }
    }
    else
    {
        if(!sysbvm_tuple_boolean_decode(gcFrame.sendNode->isDynamic) && !sysbvm_type_isDynamic(sysbvm_type_decay(context, gcFrame.receiverType)))
            sysbvm_error("Cannot send undeclared message to receiver without a dynamic type.");
    }

    gcFrame.analyzedReceiver = sysbvm_interpreter_analyzeASTWithDecayedTypeWithEnvironment(context, gcFrame.analyzedReceiver, *environment);
    gcFrame.sendNode->receiver = gcFrame.analyzedReceiver;

    size_t applicationArgumentCount = sysbvm_array_getSize(gcFrame.sendNode->arguments);
    gcFrame.analyzedArguments = sysbvm_array_create(context, applicationArgumentCount);
    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at(gcFrame.sendNode->arguments, i);
        gcFrame.analyzedArgumentNode = sysbvm_interpreter_analyzeASTWithDecayedTypeWithEnvironment(context, gcFrame.argumentNode, *environment);
        sysbvm_array_atPut(gcFrame.analyzedArguments, i, gcFrame.analyzedArgumentNode);
    }

    gcFrame.sendNode->arguments = gcFrame.analyzedArguments;
    gcFrame.sendNode->super.analyzedType = context->roots.anyValueType;
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.sendNode;
}

static sysbvm_tuple_t sysbvm_astMessageSendNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMessageSendNode_t **sendNode = (sysbvm_astMessageSendNode_t**)node;

    struct {
        sysbvm_tuple_t receiver;
        sysbvm_tuple_t receiverType;
        sysbvm_tuple_t selector;
        sysbvm_tuple_t method;
        sysbvm_tuple_t methodBinding;

        sysbvm_tuple_t analysisAndEvaluationFunction;
        sysbvm_tuple_t macroContext;
        sysbvm_tuple_t expansionResult;
        sysbvm_tuple_t receiverLiteralNode;
        sysbvm_tuple_t selectorLiteralNode;
        sysbvm_astMessageSendNode_t *messageNode;
        sysbvm_tuple_t arguments;
        sysbvm_tuple_t argumentNode;
        sysbvm_tuple_t argument;
        sysbvm_tuple_t message;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*sendNode)->super.sourcePosition);

    bool hasReceiver = false;
    if((*sendNode)->receiver)
    {
        gcFrame.receiver = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*sendNode)->receiver, *environment);
        hasReceiver = true;

        if((*sendNode)->receiverLookupType)
            gcFrame.receiverType = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*sendNode)->receiverLookupType, *environment);
        else
            gcFrame.receiverType = sysbvm_tuple_getType(context, gcFrame.receiver);
        gcFrame.analysisAndEvaluationFunction = sysbvm_type_getAnalyzeAndEvaluateMessageSendNodeForReceiverWithEnvironmentFunction(context, sysbvm_tuple_getType(context, gcFrame.receiverType));
        if(gcFrame.analysisAndEvaluationFunction)
        {
            gcFrame.result = sysbvm_function_apply4(context, gcFrame.analysisAndEvaluationFunction, gcFrame.receiverType, *node, gcFrame.receiver, *environment);
            SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return gcFrame.result;
        }

        gcFrame.selector = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*sendNode)->selector, *environment);
        gcFrame.method = sysbvm_type_lookupMacroSelector(context, gcFrame.receiverType, gcFrame.selector);
        if(!gcFrame.method)
            gcFrame.method = sysbvm_type_lookupSelector(context, gcFrame.receiverType, gcFrame.selector);
        if(!gcFrame.method)
            gcFrame.method = sysbvm_type_lookupFallbackSelector(context, gcFrame.receiverType, gcFrame.selector);
        if(!gcFrame.method)
        {
            size_t applicationArgumentCount = sysbvm_array_getSize((*sendNode)->arguments);
            gcFrame.method = sysbvm_type_lookupMacroSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
            if(!gcFrame.method)
                gcFrame.method = sysbvm_type_lookupSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
            if(!gcFrame.method)
                gcFrame.method = sysbvm_type_lookupFallbackSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
            if(!gcFrame.method)
                sysbvm_error("Message not understood.");

            // Does not understand: 
            if(sysbvm_function_isMacro(context, gcFrame.method))
            {
                gcFrame.macroContext = sysbvm_macroContext_create(context, *node, (*sendNode)->super.sourcePosition, *environment);
                gcFrame.messageNode = (sysbvm_astMessageSendNode_t *)sysbvm_context_shallowCopy(context, (sysbvm_tuple_t)*sendNode);

                gcFrame.receiverLiteralNode = sysbvm_astLiteralNode_create(context, sysbvm_astNode_getSourcePosition((*sendNode)->receiver), gcFrame.receiver);
                gcFrame.messageNode->receiver = gcFrame.receiverLiteralNode;

                gcFrame.selectorLiteralNode = sysbvm_astLiteralNode_create(context, sysbvm_astNode_getSourcePosition((*sendNode)->selector), gcFrame.selector);
                gcFrame.messageNode->selector = gcFrame.selectorLiteralNode;

                gcFrame.expansionResult = sysbvm_function_apply3(context, gcFrame.method, gcFrame.macroContext, gcFrame.receiverLiteralNode, (sysbvm_tuple_t)gcFrame.messageNode);
                SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
                SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);

                // Analyze and evaluate the resulting node.
                return sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expansionResult, *environment);
            }
            else
            {
                gcFrame.arguments = sysbvm_array_create(context, applicationArgumentCount);
                for(size_t i = 0; i < applicationArgumentCount; ++i)
                {
                    gcFrame.argumentNode = sysbvm_array_at((*sendNode)->arguments, i);
                    gcFrame.argument = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
                    sysbvm_array_atPut(gcFrame.argument, i, gcFrame.argument);
                }

                gcFrame.message = sysbvm_message_create(context, gcFrame.selector, gcFrame.arguments, (*sendNode)->super.sourcePosition);
                gcFrame.result = sysbvm_function_applyNoCheck2(context, gcFrame.method, gcFrame.receiver, gcFrame.message);
                SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
                SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
                return gcFrame.result;
            }
        }
    }
    else
    {
        gcFrame.selector = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*sendNode)->selector, *environment);
        if(!sysbvm_environment_lookSymbolRecursively(context, *environment, gcFrame.selector, &gcFrame.methodBinding)
            || !sysbvm_symbolBinding_isValue(context, gcFrame.methodBinding))
            sysbvm_error("Failed to find symbol for message send without receiver.");
        gcFrame.method = sysbvm_symbolValueBinding_getValue(gcFrame.methodBinding);
    }

    bool isMacro = sysbvm_function_isMacro(context, gcFrame.method);
    sysbvm_functionCallFrameStack_t callFrameStack = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    if(isMacro)
    {
        size_t applicationArgumentCount = sysbvm_array_getSize((*sendNode)->arguments);
        sysbvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.method, 1 + applicationArgumentCount + (hasReceiver ? 1 : 0), 0);

        gcFrame.macroContext = sysbvm_macroContext_create(context, *node, (*sendNode)->super.sourcePosition, *environment);
        sysbvm_functionCallFrameStack_push(&callFrameStack, gcFrame.macroContext);

        // We need to push the receiver as a node, so wrap it in a literal node here.
        if(hasReceiver)
        {
            gcFrame.receiverLiteralNode = sysbvm_astLiteralNode_create(context, sysbvm_astNode_getSourcePosition((*sendNode)->receiver), gcFrame.receiver);
            sysbvm_functionCallFrameStack_push(&callFrameStack, gcFrame.receiverLiteralNode);
        }

        // Push the argument nodes.
        for(size_t i = 0; i < applicationArgumentCount; ++i)
            sysbvm_functionCallFrameStack_push(&callFrameStack, sysbvm_array_at((*sendNode)->arguments, i));

        SYSBVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
        gcFrame.expansionResult = sysbvm_functionCallFrameStack_finish(context, &callFrameStack);
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);

        // Analyze and evaluate the resulting node.
        return sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.expansionResult, *environment);
    }
    else
    {
        size_t applicationArgumentCount = sysbvm_array_getSize((*sendNode)->arguments);
        sysbvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.method, applicationArgumentCount + (hasReceiver ? 1 : 0), 0);

        if(hasReceiver)
            sysbvm_functionCallFrameStack_push(&callFrameStack, gcFrame.receiver);

        for(size_t i = 0; i < applicationArgumentCount; ++i)
        {
            gcFrame.argumentNode = sysbvm_array_at((*sendNode)->arguments, i);
            gcFrame.argument = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
            sysbvm_functionCallFrameStack_push(&callFrameStack, gcFrame.argument);
        }

        SYSBVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
        gcFrame.result = sysbvm_functionCallFrameStack_finish(context, &callFrameStack);
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.result;
    }
}

static sysbvm_tuple_t sysbvm_astMessageSendNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astMessageSendNode_t **sendNode = (sysbvm_astMessageSendNode_t**)node;
    struct {
        sysbvm_tuple_t receiverType;
        sysbvm_tuple_t receiver;
        sysbvm_tuple_t selector;
        sysbvm_tuple_t method;
        sysbvm_tuple_t methodBinding;
        sysbvm_tuple_t argumentNode;
        sysbvm_tuple_t argument;
        sysbvm_tuple_t arguments;
        sysbvm_tuple_t message;
        sysbvm_tuple_t result;

        sysbvm_tuple_t receiverString;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*sendNode)->super.sourcePosition);

    bool hasReceiver = false;
    size_t applicationArgumentCount = sysbvm_array_getSize((*sendNode)->arguments);
    if((*sendNode)->receiver)
    {
        gcFrame.receiver = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*sendNode)->receiver, *environment);
        hasReceiver = true;

        if((*sendNode)->receiverLookupType)
            gcFrame.receiverType = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*sendNode)->receiverLookupType, *environment);
        else
            gcFrame.receiverType = sysbvm_tuple_getType(context, gcFrame.receiver);
        gcFrame.selector = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*sendNode)->selector, *environment);
        gcFrame.method = sysbvm_type_lookupSelector(context, gcFrame.receiverType, gcFrame.selector);
        if(!gcFrame.method)
        {
            gcFrame.method = sysbvm_type_lookupSelector(context, gcFrame.receiverType, context->roots.doesNotUnderstandSelector);
            if(!gcFrame.method)
                sysbvm_error("Message not understood.");

            gcFrame.arguments = sysbvm_array_create(context, applicationArgumentCount);
            for(size_t i = 0; i < applicationArgumentCount; ++i)
            {
                gcFrame.argumentNode = sysbvm_array_at((*sendNode)->arguments, i);
                gcFrame.argument = sysbvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
                sysbvm_array_atPut(gcFrame.argument, i, gcFrame.argument);
            }

            gcFrame.message = sysbvm_message_create(context, gcFrame.selector, gcFrame.arguments, (*sendNode)->super.sourcePosition);
            gcFrame.result = sysbvm_function_applyNoCheck2(context, gcFrame.method, gcFrame.receiver, gcFrame.message);
            SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
            SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
            return gcFrame.result;
        }
    }
    else
    {
        gcFrame.selector = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*sendNode)->selector, *environment);
        if(!sysbvm_environment_lookSymbolRecursively(context, *environment, gcFrame.selector, &gcFrame.methodBinding)
            || !sysbvm_symbolBinding_isValue(context, gcFrame.methodBinding))
            sysbvm_error("Failed to find symbol for message send without receiver.");
        gcFrame.method = sysbvm_symbolValueBinding_getValue(gcFrame.methodBinding);
    }


    sysbvm_functionCallFrameStack_t callFrameStack = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(callFrameStackRecord, callFrameStack.gcRoots);

    sysbvm_functionCallFrameStack_begin(context, &callFrameStack, gcFrame.method, applicationArgumentCount + (hasReceiver ? 1 : 0), 0);

    if(hasReceiver)
        sysbvm_functionCallFrameStack_push(&callFrameStack, gcFrame.receiver);

    for(size_t i = 0; i < applicationArgumentCount; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at((*sendNode)->arguments, i);
        gcFrame.argument = sysbvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.argumentNode, *environment);
        sysbvm_functionCallFrameStack_push(&callFrameStack, gcFrame.argument);
    }

    SYSBVM_STACKFRAME_POP_GC_ROOTS(callFrameStackRecord);
    gcFrame.result = sysbvm_functionCallFrameStack_finish(context, &callFrameStack);
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astTupleWithLookupStartingFromNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *tupleExpression = &arguments[1];
    sysbvm_tuple_t *typeExpression = &arguments[2];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    return sysbvm_astTupleWithLookupStartingFromNode_create(context, sourcePosition, *tupleExpression, *typeExpression);
}

static sysbvm_tuple_t sysbvm_astTupleWithLookupStartingFromNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astTupleWithLookupStartingFromNode_t *objectWithLookupStartingFromNode;
        sysbvm_tuple_t analyzedObjectExpression;
        sysbvm_tuple_t analyzedTypeExpression;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.objectWithLookupStartingFromNode = (sysbvm_astTupleWithLookupStartingFromNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.objectWithLookupStartingFromNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);

    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.objectWithLookupStartingFromNode->super.sourcePosition);

    gcFrame.analyzedObjectExpression = sysbvm_interpreter_analyzeASTWithDecayedTypeWithEnvironment(context, gcFrame.objectWithLookupStartingFromNode->tupleExpression, *environment);
    gcFrame.objectWithLookupStartingFromNode->tupleExpression = gcFrame.analyzedObjectExpression;

    gcFrame.analyzedTypeExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.objectWithLookupStartingFromNode->typeExpression, context->roots.typeType, *environment);
    gcFrame.objectWithLookupStartingFromNode->typeExpression = gcFrame.analyzedTypeExpression;

    gcFrame.objectWithLookupStartingFromNode->super.analyzedType = sysbvm_astNode_getAnalyzedType(gcFrame.analyzedObjectExpression);
    if(sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedTypeExpression))
        gcFrame.objectWithLookupStartingFromNode->super.analyzedType = sysbvm_astLiteralNode_getValue(gcFrame.analyzedTypeExpression);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.objectWithLookupStartingFromNode;
}

static sysbvm_tuple_t sysbvm_astTupleWithLookupStartingFromNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_astTupleWithLookupStartingFromNode_t **node = (sysbvm_astTupleWithLookupStartingFromNode_t**)&arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];
    return sysbvm_interpreter_evaluateASTWithEnvironment(context, (*node)->tupleExpression, *environment);
}

static sysbvm_tuple_t sysbvm_astTupleWithLookupStartingFromNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_astTupleWithLookupStartingFromNode_t **node = (sysbvm_astTupleWithLookupStartingFromNode_t**)&arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];
    return sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*node)->tupleExpression, *environment);
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedAtNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *tupleExpression = &arguments[1];
    sysbvm_tuple_t *nameExpression = &arguments[2];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    return sysbvm_astTupleSlotNamedAtNode_create(context, sourcePosition, *tupleExpression, *nameExpression);
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedAtNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astTupleSlotNamedAtNode_t *tupleSlotNamedAtNode;
        sysbvm_tuple_t analyzedTupleExpression;
        sysbvm_tuple_t analyzedNameExpression;
        sysbvm_tuple_t analyzedTupleExpressionType;
        sysbvm_tuple_t slotName;
        sysbvm_tuple_t slot;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.tupleSlotNamedAtNode = (sysbvm_astTupleSlotNamedAtNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.tupleSlotNamedAtNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);

    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.tupleSlotNamedAtNode->super.sourcePosition);

    gcFrame.analyzedTupleExpression = sysbvm_interpreter_analyzeASTWithReceiverTypeWithEnvironment(context, gcFrame.tupleSlotNamedAtNode->tupleExpression, *environment);
    gcFrame.tupleSlotNamedAtNode->tupleExpression = gcFrame.analyzedTupleExpression;

    gcFrame.analyzedNameExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.tupleSlotNamedAtNode->nameExpression, context->roots.symbolType, *environment);
    gcFrame.tupleSlotNamedAtNode->nameExpression = gcFrame.analyzedNameExpression;

    if(!sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedNameExpression))
        sysbvm_error("Expected a literal slot name.");

    gcFrame.slotName = sysbvm_astLiteralNode_getValue(gcFrame.analyzedNameExpression);
    gcFrame.analyzedTupleExpressionType = sysbvm_astNode_getAnalyzedType(gcFrame.analyzedTupleExpression);
    SYSBVM_ASSERT(gcFrame.analyzedTupleExpressionType);

    gcFrame.slot = sysbvm_type_lookupSlot(context, gcFrame.analyzedTupleExpressionType, gcFrame.slotName);
    if(!gcFrame.slot)
        sysbvm_error("Type does not have a slot with the specified name.");

    gcFrame.tupleSlotNamedAtNode->boundSlot = gcFrame.slot;
    gcFrame.tupleSlotNamedAtNode->super.analyzedType = sysbvm_typeSlot_getType(gcFrame.slot);
    if(!gcFrame.tupleSlotNamedAtNode->super.analyzedType)
        gcFrame.tupleSlotNamedAtNode->super.analyzedType = context->roots.anyValueType;

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.tupleSlotNamedAtNode;
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedAtNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_astTupleSlotNamedAtNode_t **node = (sysbvm_astTupleSlotNamedAtNode_t**)&arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_tuple_t tuple;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*node)->super.sourcePosition);

    gcFrame.tuple = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*node)->tupleExpression, *environment);
    size_t slotIndex = sysbvm_typeSlot_getIndex((*node)->boundSlot);
    gcFrame.result = sysbvm_tuple_slotAt(context, gcFrame.tuple, slotIndex);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);

    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedAtNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_astTupleSlotNamedAtNode_t **node = (sysbvm_astTupleSlotNamedAtNode_t**)&arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_tuple_t tuple;
        sysbvm_tuple_t name;
        sysbvm_tuple_t slot;
        sysbvm_tuple_t result;
        sysbvm_tuple_t resultType;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*node)->super.sourcePosition);

    gcFrame.tuple = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*node)->tupleExpression, *environment);
    if((*node)->boundSlot)
    {
        gcFrame.slot = (*node)->boundSlot;
    }
    else
    {
        gcFrame.name = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*node)->nameExpression, *environment);
        gcFrame.slot = sysbvm_type_lookupSlot(context, sysbvm_tuple_getType(context, gcFrame.tuple), gcFrame.name);
        if(!gcFrame.slot)
            sysbvm_error("Failed to find the slot with the specified name.");
    }

    size_t slotIndex = sysbvm_typeSlot_getIndex(gcFrame.slot);
    gcFrame.resultType = sysbvm_typeSlot_getType(gcFrame.slot);
    gcFrame.result = sysbvm_tuple_slotAt(context, gcFrame.tuple, slotIndex);
    gcFrame.result = sysbvm_type_coerceValue(context, gcFrame.resultType, gcFrame.result);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedReferenceAtNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *tupleExpression = &arguments[1];
    sysbvm_tuple_t *nameExpression = &arguments[2];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    return sysbvm_astTupleSlotNamedReferenceAtNode_create(context, sourcePosition, *tupleExpression, *nameExpression);
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedReferenceAtNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astTupleSlotNamedReferenceAtNode_t *tupleSlotNamedReferenceAtNode;
        sysbvm_tuple_t analyzedTupleExpression;
        sysbvm_tuple_t analyzedNameExpression;
        sysbvm_tuple_t analyzedTupleExpressionType;
        sysbvm_tuple_t slotName;
        sysbvm_tuple_t slot;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.tupleSlotNamedReferenceAtNode = (sysbvm_astTupleSlotNamedReferenceAtNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.tupleSlotNamedReferenceAtNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);

    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.tupleSlotNamedReferenceAtNode->super.sourcePosition);

    gcFrame.analyzedTupleExpression = sysbvm_interpreter_analyzeASTWithReceiverTypeWithEnvironment(context, gcFrame.tupleSlotNamedReferenceAtNode->tupleExpression, *environment);
    gcFrame.tupleSlotNamedReferenceAtNode->tupleExpression = gcFrame.analyzedTupleExpression;

    gcFrame.analyzedNameExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.tupleSlotNamedReferenceAtNode->nameExpression, context->roots.symbolType, *environment);
    gcFrame.tupleSlotNamedReferenceAtNode->nameExpression = gcFrame.analyzedNameExpression;

    if(!sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedNameExpression))
        sysbvm_error("Expected a literal slot name.");

    gcFrame.slotName = sysbvm_astLiteralNode_getValue(gcFrame.analyzedNameExpression);
    gcFrame.analyzedTupleExpressionType = sysbvm_astNode_getAnalyzedType(gcFrame.analyzedTupleExpression);
    SYSBVM_ASSERT(gcFrame.analyzedTupleExpressionType);

    gcFrame.slot = sysbvm_type_lookupSlot(context, gcFrame.analyzedTupleExpressionType, gcFrame.slotName);
    if(!gcFrame.slot)
        sysbvm_error("Type does not have a slot with the specified name.");

    gcFrame.tupleSlotNamedReferenceAtNode->boundSlot = gcFrame.slot;
    gcFrame.tupleSlotNamedReferenceAtNode->super.analyzedType = sysbvm_typeSlot_getValidReferenceType(context, gcFrame.slot);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.tupleSlotNamedReferenceAtNode;
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedReferenceAtNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_astTupleSlotNamedAtNode_t **node = (sysbvm_astTupleSlotNamedAtNode_t**)&arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_tuple_t tuple;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*node)->super.sourcePosition);

    gcFrame.tuple = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*node)->tupleExpression, *environment);
    gcFrame.result = sysbvm_referenceType_withTupleAndTypeSlot(context, (*node)->super.analyzedType, gcFrame.tuple, (*node)->boundSlot);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);

    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedReferenceAtNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_astTupleSlotNamedAtNode_t **node = (sysbvm_astTupleSlotNamedAtNode_t**)&arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_tuple_t tuple;
        sysbvm_tuple_t name;
        sysbvm_tuple_t slot;
        sysbvm_tuple_t result;
        sysbvm_tuple_t resultType;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*node)->super.sourcePosition);

    gcFrame.tuple = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*node)->tupleExpression, *environment);
    if((*node)->boundSlot)
    {
        gcFrame.slot = (*node)->boundSlot;
    }
    else
    {
        gcFrame.name = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*node)->nameExpression, *environment);
        gcFrame.slot = sysbvm_type_lookupSlot(context, sysbvm_tuple_getType(context, gcFrame.tuple), gcFrame.name);
        if(!gcFrame.slot)
            sysbvm_error("Failed to find the slot with the specified name.");
    }

    gcFrame.resultType = sysbvm_typeSlot_getValidReferenceType(context, gcFrame.slot);
    gcFrame.result = sysbvm_referenceType_withTupleAndTypeSlot(context, gcFrame.resultType, gcFrame.tuple, gcFrame.slot);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedAtPutNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 4) sysbvm_error_argumentCountMismatch(4, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *tupleExpression = &arguments[1];
    sysbvm_tuple_t *nameExpression = &arguments[2];
    sysbvm_tuple_t *valueExpression = &arguments[3];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    return sysbvm_astTupleSlotNamedAtPutNode_create(context, sourcePosition, *tupleExpression, *nameExpression, *valueExpression);
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedAtPutNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astTupleSlotNamedAtPutNode_t *tupleSlotNamedAtPutNode;
        sysbvm_tuple_t analyzedTupleExpression;
        sysbvm_tuple_t analyzedNameExpression;
        sysbvm_tuple_t analyzedTupleExpressionType;
        sysbvm_tuple_t slotName;
        sysbvm_tuple_t slot;
        sysbvm_tuple_t analyzedValueExpression;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.tupleSlotNamedAtPutNode = (sysbvm_astTupleSlotNamedAtPutNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.tupleSlotNamedAtPutNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);

    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.tupleSlotNamedAtPutNode->super.sourcePosition);

    gcFrame.analyzedTupleExpression = sysbvm_interpreter_analyzeASTWithReceiverTypeWithEnvironment(context, gcFrame.tupleSlotNamedAtPutNode->tupleExpression, *environment);
    gcFrame.tupleSlotNamedAtPutNode->tupleExpression = gcFrame.analyzedTupleExpression;

    if(gcFrame.tupleSlotNamedAtPutNode->nameExpression)
    {
        gcFrame.analyzedNameExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.tupleSlotNamedAtPutNode->nameExpression, context->roots.symbolType, *environment);
        gcFrame.tupleSlotNamedAtPutNode->nameExpression = gcFrame.analyzedNameExpression;
        if(!sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedNameExpression))
            sysbvm_error("Expected a literal slot name.");

        gcFrame.slotName = sysbvm_astLiteralNode_getValue(gcFrame.analyzedNameExpression);

        gcFrame.analyzedTupleExpressionType = sysbvm_astNode_getAnalyzedType(gcFrame.analyzedTupleExpression);
        SYSBVM_ASSERT(gcFrame.analyzedTupleExpressionType);

        gcFrame.slot = sysbvm_type_lookupSlot(context, gcFrame.analyzedTupleExpressionType, gcFrame.slotName);
        if(!gcFrame.slot)
            sysbvm_error("Type does not have a slot with the specified name.");

        gcFrame.tupleSlotNamedAtPutNode->boundSlot = gcFrame.slot;
    }
    else
    {
        if(!gcFrame.tupleSlotNamedAtPutNode->boundSlot)
            sysbvm_error("A name or bound slot is required.");
    }

    gcFrame.tupleSlotNamedAtPutNode->super.analyzedType = sysbvm_typeSlot_getType(gcFrame.tupleSlotNamedAtPutNode->boundSlot);
    if(!gcFrame.tupleSlotNamedAtPutNode->super.analyzedType)
        gcFrame.tupleSlotNamedAtPutNode->super.analyzedType = context->roots.anyValueType;

    gcFrame.analyzedValueExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.tupleSlotNamedAtPutNode->valueExpression, gcFrame.tupleSlotNamedAtPutNode->super.analyzedType, *environment);
    gcFrame.tupleSlotNamedAtPutNode->valueExpression = gcFrame.analyzedValueExpression;

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.tupleSlotNamedAtPutNode;
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedAtPutNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_astTupleSlotNamedAtPutNode_t **node = (sysbvm_astTupleSlotNamedAtPutNode_t**)&arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_tuple_t tuple;
        sysbvm_tuple_t value;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*node)->super.sourcePosition);

    gcFrame.tuple = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*node)->tupleExpression, *environment);
    gcFrame.value = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*node)->valueExpression, *environment);
    size_t slotIndex = sysbvm_typeSlot_getIndex((*node)->boundSlot);
    sysbvm_tuple_slotAtPut(context, gcFrame.tuple, slotIndex, gcFrame.value);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);

    return gcFrame.value;
}

static sysbvm_tuple_t sysbvm_astTupleSlotNamedAtPutNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_astTupleSlotNamedAtPutNode_t **node = (sysbvm_astTupleSlotNamedAtPutNode_t**)&arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_tuple_t tuple;
        sysbvm_tuple_t name;
        sysbvm_tuple_t slot;
        sysbvm_tuple_t value;
        sysbvm_tuple_t valueType;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*node)->super.sourcePosition);

    gcFrame.tuple = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*node)->tupleExpression, *environment);
    if((*node)->boundSlot)
    {
        gcFrame.slot = (*node)->boundSlot;
    }
    else
    {
        gcFrame.name = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*node)->nameExpression, *environment);
        gcFrame.slot = sysbvm_type_lookupSlot(context, sysbvm_tuple_getType(context, gcFrame.tuple), gcFrame.name);
        if(!gcFrame.slot)
            sysbvm_error("Failed to find the slot with the specified name.");
    }

    size_t slotIndex = sysbvm_typeSlot_getIndex(gcFrame.slot);
    gcFrame.valueType = sysbvm_typeSlot_getType(gcFrame.slot);
    gcFrame.value = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*node)->valueExpression, *environment);
    gcFrame.value = sysbvm_type_coerceValue(context, gcFrame.valueType, gcFrame.value);
    sysbvm_tuple_slotAtPut(context, gcFrame.tuple, slotIndex, gcFrame.value);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.value;
}

static sysbvm_tuple_t sysbvm_astDoWhileContinueWithNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 4) sysbvm_error_argumentCountMismatch(4, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *bodyExpressionNode = &arguments[1];
    sysbvm_tuple_t *conditionNode = &arguments[2];
    sysbvm_tuple_t *continueExpressionNode = &arguments[3];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);

    return sysbvm_astDoWhileContinueWithNode_create(context, sourcePosition, *bodyExpressionNode, *conditionNode, *continueExpressionNode);
}

static sysbvm_tuple_t sysbvm_astDoWhileContinueWithNode_primitiveDoWhileMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *bodyExpressionNode = &arguments[1];
    sysbvm_tuple_t *conditionNode = &arguments[2];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);

    return sysbvm_astDoWhileContinueWithNode_create(context, sourcePosition, *bodyExpressionNode, *conditionNode, SYSBVM_NULL_TUPLE);
}

static sysbvm_tuple_t sysbvm_astDoWhileContinueWithNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astDoWhileContinueWithNode_t *doWhileNode;
        sysbvm_tuple_t analyzedBodyExpression;
        sysbvm_tuple_t analyzedConditionExpression;
        sysbvm_tuple_t analyzedContinueExpression;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.doWhileNode = (sysbvm_astDoWhileContinueWithNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.doWhileNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
    gcFrame.doWhileNode->super.analyzedType = context->roots.voidType;
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.doWhileNode->super.sourcePosition);

    if(gcFrame.doWhileNode->bodyExpression)
    {
        gcFrame.analyzedBodyExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.doWhileNode->bodyExpression, context->roots.voidType, *environment);
        gcFrame.doWhileNode->bodyExpression = gcFrame.analyzedBodyExpression;
    }

    if(gcFrame.doWhileNode->conditionExpression)
    {
        gcFrame.analyzedConditionExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.doWhileNode->conditionExpression, context->roots.booleanType, *environment);
        gcFrame.doWhileNode->conditionExpression = gcFrame.analyzedConditionExpression;
    }

    if(gcFrame.doWhileNode->continueExpression)
    {
        gcFrame.analyzedContinueExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.doWhileNode->continueExpression, context->roots.voidType, *environment);
        gcFrame.doWhileNode->continueExpression = gcFrame.analyzedContinueExpression;
    }

    if(gcFrame.doWhileNode->conditionExpression &&
        sysbvm_astNode_isLiteralNode(context, gcFrame.doWhileNode->conditionExpression) &&
        !sysbvm_tuple_boolean_decode(sysbvm_astLiteralNode_getValue(gcFrame.doWhileNode->conditionExpression)))
    {
        if(!gcFrame.analyzedBodyExpression)
            gcFrame.analyzedBodyExpression = sysbvm_astLiteralNode_create(context, gcFrame.doWhileNode->super.sourcePosition, SYSBVM_VOID_TUPLE);

        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return gcFrame.analyzedBodyExpression;
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.doWhileNode;
}

static sysbvm_tuple_t sysbvm_astDoWhileContinueWithNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astDoWhileContinueWithNode_t **doWhileNode = (sysbvm_astDoWhileContinueWithNode_t**)node;
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*doWhileNode)->super.sourcePosition);
    bool shouldContinue = false;
    do
    {
        if((*doWhileNode)->bodyExpression)
            sysbvm_interpreter_evaluateASTWithEnvironment(context, (*doWhileNode)->bodyExpression, *environment);
        if((*doWhileNode)->conditionExpression)
            shouldContinue = sysbvm_tuple_boolean_decode(sysbvm_interpreter_evaluateASTWithEnvironment(context, (*doWhileNode)->conditionExpression, *environment));
        else
            shouldContinue = true;

        if(shouldContinue)
        {
            if((*doWhileNode)->continueExpression)
                sysbvm_interpreter_evaluateASTWithEnvironment(context, (*doWhileNode)->continueExpression, *environment);
        }

        sysbvm_gc_safepoint(context);
    } while (shouldContinue);
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);

    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_astDoWhileContinueWithNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astDoWhileContinueWithNode_t **doWhileNode = (sysbvm_astDoWhileContinueWithNode_t**)node;
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*doWhileNode)->super.sourcePosition);
    bool shouldContinue = false;
    do
    {
        if((*doWhileNode)->bodyExpression)
            sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*doWhileNode)->bodyExpression, *environment);
        if((*doWhileNode)->conditionExpression)
            shouldContinue = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*doWhileNode)->conditionExpression, *environment);
        else
            shouldContinue = true;

        if(shouldContinue)
        {
            if((*doWhileNode)->continueExpression)
                sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*doWhileNode)->continueExpression, *environment);
        }

        sysbvm_gc_safepoint(context);
    } while (shouldContinue);
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);

    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_astWhileContinueWithNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 4) sysbvm_error_argumentCountMismatch(4, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *conditionNode = &arguments[1];
    sysbvm_tuple_t *bodyExpressionNode = &arguments[2];
    sysbvm_tuple_t *continueNode = &arguments[3];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);

    return sysbvm_astWhileContinueWithNode_create(context, sourcePosition, *conditionNode, *bodyExpressionNode, *continueNode);
}

static sysbvm_tuple_t sysbvm_astWhileContinueWithNode_primitiveWhileDoMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *conditionNode = &arguments[1];
    sysbvm_tuple_t *bodyExpressionNode = &arguments[2];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);

    return sysbvm_astWhileContinueWithNode_create(context, sourcePosition, *conditionNode, *bodyExpressionNode, SYSBVM_NULL_TUPLE);
}

static sysbvm_tuple_t sysbvm_astWhileContinueWithNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astWhileContinueWithNode_t *whileNode;
        sysbvm_tuple_t analyzedConditionExpression;
        sysbvm_tuple_t analyzedBodyExpression;
        sysbvm_tuple_t analyzedContinueExpression;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.whileNode = (sysbvm_astWhileContinueWithNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.whileNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
    gcFrame.whileNode->super.analyzedType = context->roots.voidType;

    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.whileNode->super.sourcePosition);
    
    if(gcFrame.whileNode->conditionExpression)
    {
        gcFrame.analyzedConditionExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.whileNode->conditionExpression, context->roots.booleanType, *environment);
        gcFrame.whileNode->conditionExpression = gcFrame.analyzedConditionExpression;
    }

    if(gcFrame.whileNode->bodyExpression)
    {
        gcFrame.analyzedBodyExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.whileNode->bodyExpression, context->roots.voidType, *environment);
        gcFrame.whileNode->bodyExpression = gcFrame.analyzedBodyExpression;
    }

    if(gcFrame.whileNode->continueExpression)
    {
        gcFrame.analyzedContinueExpression = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.whileNode->continueExpression, context->roots.voidType, *environment);
        gcFrame.whileNode->continueExpression = gcFrame.analyzedContinueExpression;
    }
    
    // Optimize out the loop if the condition is the literal false.
    if(gcFrame.whileNode->conditionExpression &&
        sysbvm_astNode_isLiteralNode(context, gcFrame.whileNode->conditionExpression) &&
        !sysbvm_tuple_boolean_decode(sysbvm_astLiteralNode_getValue(gcFrame.whileNode->conditionExpression)))
    {
        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return sysbvm_astLiteralNode_create(context, gcFrame.whileNode->super.sourcePosition, SYSBVM_VOID_TUPLE);
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.whileNode;
}

static sysbvm_tuple_t sysbvm_astWhileContinueWithNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astWhileContinueWithNode_t **whileNode = (sysbvm_astWhileContinueWithNode_t**)node;
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*whileNode)->super.sourcePosition);
    while(!(*whileNode)->conditionExpression
        || sysbvm_tuple_boolean_decode(sysbvm_interpreter_evaluateASTWithEnvironment(context, (*whileNode)->conditionExpression, *environment)))
    {
        if((*whileNode)->bodyExpression)
            sysbvm_interpreter_evaluateASTWithEnvironment(context, (*whileNode)->bodyExpression, *environment);

        if((*whileNode)->continueExpression)
            sysbvm_interpreter_evaluateASTWithEnvironment(context, (*whileNode)->continueExpression, *environment);

        sysbvm_gc_safepoint(context);
    }
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);

    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_astWhileContinueWithNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_tuple_t loopEnvironment;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.loopEnvironment = sysbvm_analysisAndEvaluationEnvironment_create(context, *environment);

    sysbvm_astWhileContinueWithNode_t **whileNode = (sysbvm_astWhileContinueWithNode_t**)node;
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*whileNode)->super.sourcePosition);

    bool shouldContinue = true;
    
    sysbvm_stackFrameBreakTargetRecord_t breakTargetRecord = {
        .type = SYSBVM_STACK_FRAME_RECORD_TYPE_BREAK_TARGET,
        .environment = gcFrame.loopEnvironment
    };
    sysbvm_stackFrame_pushRecord((sysbvm_stackFrameRecord_t*)&breakTargetRecord);  
    sysbvm_analysisAndEvaluationEnvironment_setBreakTarget(context, gcFrame.loopEnvironment, sysbvm_tuple_uintptr_encode(context, (uintptr_t)&breakTargetRecord));

    if(!_setjmp(breakTargetRecord.jmpbuffer))
    {
        if((*whileNode)->conditionExpression)
            shouldContinue = sysbvm_tuple_boolean_decode(sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*whileNode)->conditionExpression, gcFrame.loopEnvironment));

        while(shouldContinue)
        {
            if((*whileNode)->bodyExpression)
            {
                sysbvm_stackFrameContinueTargetRecord_t continueTargetRecord = {
                    .type = SYSBVM_STACK_FRAME_RECORD_TYPE_CONTINUE_TARGET,
                    .environment = gcFrame.loopEnvironment
                };
                sysbvm_stackFrame_pushRecord((sysbvm_stackFrameRecord_t*)&continueTargetRecord);
                sysbvm_analysisAndEvaluationEnvironment_setContinueTarget(context, gcFrame.loopEnvironment, sysbvm_tuple_uintptr_encode(context, (uintptr_t)&breakTargetRecord));

                if(!_setjmp(continueTargetRecord.jmpbuffer))
                {
                    sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*whileNode)->bodyExpression, gcFrame.loopEnvironment);
                }

                sysbvm_analysisAndEvaluationEnvironment_setContinueTarget(context, gcFrame.loopEnvironment, SYSBVM_NULL_TUPLE);
                sysbvm_stackFrame_popRecord((sysbvm_stackFrameRecord_t*)&continueTargetRecord);  
            }

            if((*whileNode)->continueExpression)
                sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*whileNode)->continueExpression, gcFrame.loopEnvironment);

            if((*whileNode)->conditionExpression)
                shouldContinue = sysbvm_tuple_boolean_decode(sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*whileNode)->conditionExpression, gcFrame.loopEnvironment));

            if(shouldContinue)
                sysbvm_gc_safepoint(context);
        }

        sysbvm_analysisAndEvaluationEnvironment_setBreakTarget(context, gcFrame.loopEnvironment, SYSBVM_NULL_TUPLE);
    }

    sysbvm_stackFrame_popRecord((sysbvm_stackFrameRecord_t*)&breakTargetRecord);  
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);

    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_astUseNamedSlotsOfNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *tupleNode = &arguments[1];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);

    return sysbvm_astUseNamedSlotsOfNode_create(context, sourcePosition, *tupleNode);
}

static sysbvm_tuple_t sysbvm_astUseNamedSlotsOfNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astUseNamedSlotsOfNode_t *useNode;
        sysbvm_tuple_t analyzedTupleNode;
        sysbvm_tuple_t localBinding;
        sysbvm_tuple_t literalVoid;
        sysbvm_tuple_t value;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.useNode = (sysbvm_astUseNamedSlotsOfNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.useNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
    gcFrame.useNode->super.analyzedType = context->roots.voidType;
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.useNode->super.sourcePosition);

    gcFrame.analyzedTupleNode = sysbvm_interpreter_analyzeASTWithReceiverTypeWithEnvironment(context, gcFrame.useNode->tupleExpression, *environment);
    gcFrame.useNode->tupleExpression = gcFrame.analyzedTupleNode;

    if(sysbvm_astNode_isLiteralNode(context, gcFrame.analyzedTupleNode))
    {
        gcFrame.value = sysbvm_astLiteralNode_getValue(gcFrame.analyzedTupleNode);
        gcFrame.localBinding = sysbvm_analysisEnvironment_setNewValueBinding(context, *environment, gcFrame.useNode->super.sourcePosition, SYSBVM_NULL_TUPLE, gcFrame.value);
        gcFrame.literalVoid = sysbvm_astLiteralNode_create(context, gcFrame.useNode->super.sourcePosition, SYSBVM_VOID_TUPLE);
        sysbvm_analysisAndEvaluationEnvironment_addUseTupleWithNamedSlotsBinding(context, *environment, gcFrame.localBinding);

        SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
        SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
        return (sysbvm_tuple_t)gcFrame.useNode;
    }
    else
    {
        gcFrame.localBinding = sysbvm_analysisEnvironment_setNewSymbolLocalBinding(context, *environment, gcFrame.useNode->super.sourcePosition, SYSBVM_NULL_TUPLE, sysbvm_astNode_getAnalyzedType(gcFrame.analyzedTupleNode));
        sysbvm_analysisAndEvaluationEnvironment_addUseTupleWithNamedSlotsBinding(context, *environment, gcFrame.localBinding);
        gcFrame.useNode->binding = gcFrame.localBinding;
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.useNode;
}

static sysbvm_tuple_t sysbvm_astUseNamedSlotsOfNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astUseNamedSlotsOfNode_t **useNode = (sysbvm_astUseNamedSlotsOfNode_t**)node;

    struct {
        sysbvm_tuple_t usedTuple;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*useNode)->super.sourcePosition);

    gcFrame.usedTuple = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*useNode)->tupleExpression, *environment);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_astUseNamedSlotsOfNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astUseNamedSlotsOfNode_t **useNode = (sysbvm_astUseNamedSlotsOfNode_t**)node;

    struct {
        sysbvm_tuple_t usedTuple;
        sysbvm_tuple_t localBinding;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*useNode)->super.sourcePosition);

    gcFrame.usedTuple = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*useNode)->tupleExpression, *environment);
    gcFrame.localBinding = sysbvm_analysisEnvironment_setNewValueBinding(context, *environment, (*useNode)->super.sourcePosition, SYSBVM_NULL_TUPLE, gcFrame.usedTuple);
    sysbvm_analysisAndEvaluationEnvironment_addUseTupleWithNamedSlotsBinding(context, *environment, gcFrame.localBinding);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return SYSBVM_VOID_TUPLE;
}

static sysbvm_tuple_t sysbvm_astBreakNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];

    struct {
        sysbvm_tuple_t sourcePosition;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    gcFrame.result = sysbvm_astBreakNode_create(context, gcFrame.sourcePosition);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astBreakNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    //sysbvm_tuple_t *environment = &arguments[1];

    return *node;
}

static sysbvm_tuple_t sysbvm_astBreakNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astBreakNode_t **breakNode = (sysbvm_astBreakNode_t**)node;
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*breakNode)->super.sourcePosition);

    sysbvm_tuple_t breakTarget = sysbvm_environment_lookBreakTargetRecursively(context, *environment);
    if(!breakTarget)
        sysbvm_error("No target available for break.");

    sysbvm_stackFrame_breakInto((sysbvm_stackFrameRecord_t*)sysbvm_tuple_uintptr_decode(breakTarget));
    return SYSBVM_NULL_TUPLE;
}

static sysbvm_tuple_t sysbvm_astContinueNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];

    struct {
        sysbvm_tuple_t sourcePosition;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    gcFrame.result = sysbvm_astContinueNode_create(context, gcFrame.sourcePosition);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astContinueNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    //sysbvm_tuple_t *environment = &arguments[1];

    return *node;
}

static sysbvm_tuple_t sysbvm_astContinueNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astContinueNode_t **continueNode = (sysbvm_astContinueNode_t**)node;
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*continueNode)->super.sourcePosition);

    sysbvm_tuple_t continueTarget = sysbvm_environment_lookContinueTargetRecursively(context, *environment);
    if(!continueTarget)
        sysbvm_error("No target available for continue.");

    sysbvm_stackFrame_continueInto((sysbvm_stackFrameRecord_t*)sysbvm_tuple_uintptr_decode(continueTarget));
    return SYSBVM_NULL_TUPLE;
}

static sysbvm_tuple_t sysbvm_astReturnNode_primitiveMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *expression = &arguments[1];

    struct {
        sysbvm_tuple_t sourcePosition;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);
    gcFrame.result = sysbvm_astReturnNode_create(context, gcFrame.sourcePosition, *expression);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_astReturnNode_primitiveAnalyze(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    struct {
        sysbvm_astReturnNode_t *returnNode;
        sysbvm_tuple_t analyzedExpression;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.returnNode = (sysbvm_astReturnNode_t*)sysbvm_context_shallowCopy(context, *node);
    gcFrame.returnNode->super.analyzerToken = sysbvm_analysisAndEvaluationEnvironment_ensureValidAnalyzerToken(context, *environment);
    gcFrame.returnNode->super.analyzedType = context->roots.controlFlowReturnType;
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.returnNode->super.sourcePosition);

    if(gcFrame.returnNode->expression)
    {
        // TODO: Coerce the value onto the current expected result type.
        gcFrame.analyzedExpression = sysbvm_interpreter_analyzeASTWithDecayedTypeWithEnvironment(context, gcFrame.returnNode->expression, *environment);
        gcFrame.returnNode->expression = gcFrame.analyzedExpression;
    }

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)gcFrame.returnNode;
}

static sysbvm_tuple_t sysbvm_astReturnNode_primitiveEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astReturnNode_t **returnNode = (sysbvm_astReturnNode_t**)node;

    struct {
        sysbvm_tuple_t result;
    } gcFrame = {
        .result = SYSBVM_VOID_TUPLE
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*returnNode)->super.sourcePosition);

    if((*returnNode)->expression)
        gcFrame.result = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*returnNode)->expression, *environment);
    
    sysbvm_tuple_t returnTarget = sysbvm_environment_lookReturnTargetRecursively(context, *environment);
    if(!returnTarget)
        sysbvm_error("No target available for returning value.");

    sysbvm_stackFrame_returnValueInto(gcFrame.result, (sysbvm_stackFrameRecord_t*)sysbvm_tuple_uintptr_decode(returnTarget));
    return SYSBVM_NULL_TUPLE;
}

static sysbvm_tuple_t sysbvm_astReturnNode_primitiveAnalyzeAndEvaluate(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *node = &arguments[0];
    sysbvm_tuple_t *environment = &arguments[1];

    sysbvm_astReturnNode_t **returnNode = (sysbvm_astReturnNode_t**)node;

    struct {
        sysbvm_tuple_t result;
    } gcFrame = {
        .result = SYSBVM_VOID_TUPLE
    };
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, (*returnNode)->super.sourcePosition);

    if((*returnNode)->expression)
        gcFrame.result = sysbvm_interpreter_analyzeAndEvaluateASTWithEnvironment(context, (*returnNode)->expression, *environment);
    
    sysbvm_tuple_t returnTarget = sysbvm_environment_lookReturnTargetRecursively(context, *environment);
    if(!returnTarget)
        sysbvm_error("No target available for returning value.");

    sysbvm_stackFrame_returnValueInto(gcFrame.result, (sysbvm_stackFrameRecord_t*)sysbvm_tuple_uintptr_decode(returnTarget));
    return SYSBVM_NULL_TUPLE;
}

static sysbvm_tuple_t sysbvm_interpreter_evaluateArgumentNodeTypeInEnvironment(sysbvm_context_t *context, sysbvm_tuple_t argumentNode, sysbvm_tuple_t *activationEnvironment)
{
    struct {
        sysbvm_astArgumentNode_t *argumentNode;
        sysbvm_tuple_t expectedType;
    } gcFrame = {
        .argumentNode = (sysbvm_astArgumentNode_t*)argumentNode,
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.argumentNode->super.sourcePosition);

    if(gcFrame.argumentNode->type)
        gcFrame.expectedType = sysbvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.argumentNode->type, *activationEnvironment);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);

    return gcFrame.expectedType;
}

static void sysbvm_interpreter_bindArgumentNodeValueInEnvironment(sysbvm_context_t *context, size_t argumentNodeIndex, sysbvm_tuple_t *activationEnvironment, sysbvm_tuple_t argumentValue)
{
    (void)context;
    sysbvm_functionActivationEnvironment_t **activationEnvironmentObject = (sysbvm_functionActivationEnvironment_t **)activationEnvironment;
    sysbvm_array_atPut((*activationEnvironmentObject)->valueVector, argumentNodeIndex, argumentValue);
}

static void sysbvm_interpreter_evaluateArgumentNodeInEnvironment(sysbvm_context_t *context, size_t argumentNodeIndex, sysbvm_tuple_t argumentNode, sysbvm_tuple_t *activationEnvironment, sysbvm_tuple_t *argumentValue)
{
    struct {
        sysbvm_astArgumentNode_t *argumentNode;
        sysbvm_tuple_t expectedType;
        sysbvm_tuple_t value;
    } gcFrame = {
        .argumentNode = (sysbvm_astArgumentNode_t*)argumentNode,
        .value = *argumentValue
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.argumentNode->super.sourcePosition);

    if(gcFrame.argumentNode->type)
    {
        gcFrame.expectedType = sysbvm_interpreter_evaluateASTWithEnvironment(context, gcFrame.argumentNode->type, *activationEnvironment);
        if(gcFrame.expectedType)
            gcFrame.value = sysbvm_type_coerceValue(context, gcFrame.expectedType, gcFrame.value);
    }

    sysbvm_interpreter_bindArgumentNodeValueInEnvironment(context, argumentNodeIndex, activationEnvironment, gcFrame.value);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
}

static sysbvm_tuple_t sysbvm_simpleFunctionType_primitiveAnalyzeAndTypeCheckMessageSendNode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_simpleFunctionType_t **simpleFunctionType = (sysbvm_simpleFunctionType_t**)&arguments[0];
    sysbvm_astMessageSendNode_t **sendNode = (sysbvm_astMessageSendNode_t**)&arguments[1];
    sysbvm_tuple_t *environment = &arguments[2];

    struct {
        sysbvm_tuple_t analyzedReceiver;

        sysbvm_tuple_t argumentNode;
        sysbvm_tuple_t analyzedArgument;
        sysbvm_tuple_t analyzedArguments;
        sysbvm_tuple_t expectedArgumentType;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    size_t typeArgumentCount = sysbvm_array_getSize((*simpleFunctionType)->argumentTypes);
    size_t sendArgumentCount = sysbvm_array_getSize((*sendNode)->arguments);
    size_t sendArgumentStartIndex = (*sendNode)->receiver ? 1 : 0;

    size_t applicationArgumentCount = sendArgumentStartIndex + sendArgumentCount;
    if(applicationArgumentCount != typeArgumentCount)
        sysbvm_error("Expected number of arguments is mismatching.");

    sysbvm_bitflags_t flags = sysbvm_tuple_bitflags_decode((*simpleFunctionType)->super.functionFlags);
    bool isMemoizedTemplate = (flags & SYSBVM_FUNCTION_FLAGS_MEMOIZED_TEMPLATE) == SYSBVM_FUNCTION_FLAGS_MEMOIZED_TEMPLATE;

    // Analyze the receiver
    if(sendArgumentStartIndex > 0)
    {
        gcFrame.expectedArgumentType = sysbvm_array_at((*simpleFunctionType)->argumentTypes, 0);
        gcFrame.analyzedReceiver = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, (*sendNode)->receiver, gcFrame.expectedArgumentType, *environment);
        (*sendNode)->receiver = gcFrame.analyzedReceiver;
    }

    // Analyze the send arguments.
    gcFrame.analyzedArguments = sysbvm_array_create(context, sendArgumentCount);
    for(size_t i = 0; i < sendArgumentCount; ++i)
    {
        gcFrame.expectedArgumentType = sysbvm_array_at((*simpleFunctionType)->argumentTypes, sendArgumentStartIndex + i);
        gcFrame.argumentNode = sysbvm_array_at((*sendNode)->arguments, i);
        gcFrame.analyzedArgument = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.argumentNode, gcFrame.expectedArgumentType, *environment);
        sysbvm_array_atPut(gcFrame.analyzedArguments, i, gcFrame.analyzedArgument);
    }

    (*sendNode)->arguments = gcFrame.analyzedArguments;
    (*sendNode)->super.analyzedType = (*simpleFunctionType)->resultType;
    (*sendNode)->applicationFlags = sysbvm_tuple_bitflags_encode(sysbvm_tuple_bitflags_decode((*sendNode)->applicationFlags)
        | (isMemoizedTemplate ? 0 : SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK)
        | SYSBVM_FUNCTION_APPLICATION_FLAGS_VARIADIC_EXPANDED);

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return (sysbvm_tuple_t)*sendNode;
}

static sysbvm_tuple_t sysbvm_simpleFunctionType_primitiveAnalyzeAndTypeCheckFunctionApplicationNode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_simpleFunctionType_t **simpleFunctionType = (sysbvm_simpleFunctionType_t**)&arguments[0];
    sysbvm_astFunctionApplicationNode_t **functionApplicationNode = (sysbvm_astFunctionApplicationNode_t**)&arguments[1];
    sysbvm_tuple_t *environment = &arguments[2];

    struct {
        sysbvm_tuple_t argumentNode;
        sysbvm_tuple_t analyzedArgument;
        sysbvm_tuple_t analyzedArguments;
        sysbvm_tuple_t expectedArgumentType;
        sysbvm_tuple_t variadicArguments;
        sysbvm_tuple_t variadicArgumentsNode;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    sysbvm_bitflags_t flags = sysbvm_tuple_bitflags_decode((*simpleFunctionType)->super.functionFlags);
    bool isVariadic = (flags & SYSBVM_FUNCTION_FLAGS_VARIADIC) != 0;
    bool isMemoizedTemplate = (flags & SYSBVM_FUNCTION_FLAGS_MEMOIZED_TEMPLATE) == SYSBVM_FUNCTION_FLAGS_MEMOIZED_TEMPLATE;

    size_t typeArgumentCount = sysbvm_array_getSize((*simpleFunctionType)->argumentTypes);
    size_t expectedArgumentCount = typeArgumentCount;
    size_t applicationArgumentCount = sysbvm_array_getSize((*functionApplicationNode)->arguments);
    size_t startingArgumentIndex = isMemoizedTemplate ? 1 : 0;
    if(isVariadic && typeArgumentCount == 0)
        sysbvm_error("Variadic applications require at least a single argument.");

    size_t directApplicationArgumentCount = isVariadic ? typeArgumentCount - 1 : typeArgumentCount;
    if(isMemoizedTemplate)
    {
        if(expectedArgumentCount == 0)
            sysbvm_error("Memoized template requires at least a single argument.");
        --expectedArgumentCount;
        --directApplicationArgumentCount;
    }

    if(isVariadic && applicationArgumentCount < directApplicationArgumentCount)
        sysbvm_error("Missing required arguments.");
    else if(!isVariadic && applicationArgumentCount != expectedArgumentCount)
        sysbvm_error("Expected number of arguments is mismatching.");

    gcFrame.analyzedArguments = sysbvm_array_create(context, expectedArgumentCount);
    for(size_t i = 0; i < directApplicationArgumentCount; ++i)
    {
        gcFrame.expectedArgumentType = sysbvm_array_at((*simpleFunctionType)->argumentTypes, startingArgumentIndex + i);
        gcFrame.argumentNode = sysbvm_array_at((*functionApplicationNode)->arguments, i);
        gcFrame.analyzedArgument = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.argumentNode, gcFrame.expectedArgumentType, *environment);
        sysbvm_array_atPut(gcFrame.analyzedArguments, i, gcFrame.analyzedArgument);
    }

    // Analyze the variadic arguments.
    if(isVariadic)
    {
        gcFrame.variadicArguments = sysbvm_array_create(context, applicationArgumentCount - directApplicationArgumentCount);
        for(size_t i = directApplicationArgumentCount; i < applicationArgumentCount; ++i)
            sysbvm_array_atPut(gcFrame.variadicArguments, i - directApplicationArgumentCount, sysbvm_array_at((*functionApplicationNode)->arguments, i));

        gcFrame.variadicArgumentsNode = sysbvm_astMakeArrayNode_create(context, (*functionApplicationNode)->super.sourcePosition, gcFrame.variadicArguments);
        gcFrame.variadicArgumentsNode = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.variadicArgumentsNode, context->roots.arrayType, *environment);
        sysbvm_array_atPut(gcFrame.analyzedArguments, directApplicationArgumentCount, gcFrame.variadicArgumentsNode);
    }

    (*functionApplicationNode)->applicationFlags = sysbvm_tuple_bitflags_encode(sysbvm_tuple_bitflags_decode((*functionApplicationNode)->applicationFlags)
        | (isMemoizedTemplate ? 0 : SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK)
        | SYSBVM_FUNCTION_APPLICATION_FLAGS_VARIADIC_EXPANDED);
    (*functionApplicationNode)->arguments = gcFrame.analyzedArguments;
    (*functionApplicationNode)->super.analyzedType = (*simpleFunctionType)->resultType;

    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return sysbvm_astFunctionApplicationNode_optimizePureApplication(context, functionApplicationNode);
}

static sysbvm_tuple_t sysbvm_dependentFunctionType_getOrCreateDependentApplicationValueForNode(sysbvm_context_t *context, sysbvm_tuple_t node)
{
    // Unwrap the literal values.
    if(sysbvm_astNode_isLiteralNode(context, node))
        return sysbvm_astLiteralNode_getValue(node);

    // Get the analyzed type of the node.
    sysbvm_tuple_t analyzedType = sysbvm_astNode_getAnalyzedType(node);

    // Find a method.
    sysbvm_tuple_t method = sysbvm_type_lookupSelector(context, sysbvm_tuple_getType(context, analyzedType), context->roots.getOrCreateDependentApplicationValueForNodeSelector);
    if(method)
        return sysbvm_function_apply2(context, method, analyzedType, node);

    // Use the base type in the case of reference types.
    if(sysbvm_type_isReferenceType(analyzedType))
    {
        analyzedType = ((sysbvm_referenceType_t*)analyzedType)->super.baseType;
        sysbvm_tuple_t baseTypeMethod = sysbvm_type_lookupSelector(context, sysbvm_tuple_getType(context, analyzedType), context->roots.getOrCreateDependentApplicationValueForNodeSelector);
        if(baseTypeMethod)
            return sysbvm_function_apply2(context, baseTypeMethod, analyzedType, node);
    }

    // If the node is a subtype of metatype, and this type is defined then return the type.
    if(sysbvm_tuple_isKindOf(context, analyzedType, context->roots.metatypeType))
    {
        sysbvm_metatype_t *metatype = (sysbvm_metatype_t*)analyzedType;
        if(metatype->thisType)
            return metatype->thisType;
    }

    // Construct a dummy value that has the same type
    sysbvm_tuple_t dummyValue = (sysbvm_tuple_t)sysbvm_context_allocatePointerTuple(context, analyzedType, 0);
    sysbvm_tuple_markDummyValue(dummyValue);
    return dummyValue;
}

static sysbvm_tuple_t sysbvm_dependentFunctionType_primitiveAnalyzeAndTypeCheckFunctionApplicationNode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_dependentFunctionType_t **dependentFunctionType = (sysbvm_dependentFunctionType_t **)&arguments[0];
    sysbvm_tuple_t *node = &arguments[1];
    sysbvm_tuple_t *environment = &arguments[2];

    struct {
        sysbvm_astFunctionApplicationNode_t *functionApplicationNode;

        sysbvm_tuple_t applicationEnvironment;
        sysbvm_tuple_t argumentNode;

        sysbvm_tuple_t applicationArgumentNode;
        sysbvm_tuple_t applicationArgumentValue;
        sysbvm_tuple_t resultType;

        sysbvm_tuple_t analyzedArgument;
        sysbvm_tuple_t analyzedArguments;
        sysbvm_tuple_t expectedArgumentType;
    } gcFrame = {
        .functionApplicationNode = (sysbvm_astFunctionApplicationNode_t*)*node,
    };

    sysbvm_stackFrameFunctionActivationRecord_t functionActivationRecord = {
        .type = SYSBVM_STACK_FRAME_RECORD_TYPE_FUNCTION_ACTIVATION,
    };
    sysbvm_stackFrame_pushRecord((sysbvm_stackFrameRecord_t*)&functionActivationRecord); 
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.functionApplicationNode->super.sourcePosition);

    gcFrame.applicationEnvironment = sysbvm_functionActivationEnvironment_createForDependentFunctionType(context, SYSBVM_NULL_TUPLE, (sysbvm_tuple_t)*dependentFunctionType);

    size_t argumentNodeCount = sysbvm_array_getSize((*dependentFunctionType)->argumentNodes);
    sysbvm_bitflags_t flags = sysbvm_tuple_bitflags_decode((*dependentFunctionType)->super.functionFlags);
    bool isVariadic = (flags & SYSBVM_FUNCTION_FLAGS_VARIADIC) != 0;
    if(isVariadic && argumentNodeCount == 0)
        sysbvm_error("Variadic functions at least one extra argument node.");
    
    size_t directEvaluationArgumentNodeCount = isVariadic ? argumentNodeCount - 1 : argumentNodeCount;

    size_t applicationArgumentCount = sysbvm_array_getSize(gcFrame.functionApplicationNode->arguments);
    size_t sourceArgumentIndex = 0;

    gcFrame.analyzedArguments = sysbvm_array_create(context, applicationArgumentCount);
    for(size_t i = 0; i < directEvaluationArgumentNodeCount; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at((*dependentFunctionType)->argumentNodes, i);
        if(sysbvm_astArgumentNode_isForAll(argumentNodeCount))
        {
            sysbvm_error("TODO: Support forall argument types");
        }
        else
        {
            if(sourceArgumentIndex >= applicationArgumentCount)
                sysbvm_error("Function application is missing required arguments.");

            gcFrame.applicationArgumentNode = sysbvm_array_at(gcFrame.functionApplicationNode->arguments, sourceArgumentIndex);
            gcFrame.expectedArgumentType = sysbvm_interpreter_evaluateArgumentNodeTypeInEnvironment(context, gcFrame.argumentNode, &gcFrame.applicationEnvironment);
            gcFrame.analyzedArgument = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.applicationArgumentNode, gcFrame.expectedArgumentType, *environment);
            sysbvm_array_atPut(gcFrame.analyzedArguments, sourceArgumentIndex, gcFrame.analyzedArgument);
            gcFrame.applicationArgumentValue = sysbvm_dependentFunctionType_getOrCreateDependentApplicationValueForNode(context, gcFrame.analyzedArgument);
            sysbvm_interpreter_bindArgumentNodeValueInEnvironment(context, i, &gcFrame.applicationEnvironment, gcFrame.applicationArgumentValue);
            ++sourceArgumentIndex;
        }
    }

    if(!isVariadic && sourceArgumentIndex != applicationArgumentCount)
        sysbvm_error("Function application is not receiving the expected number of arguments.");

    for(size_t i = sourceArgumentIndex; i < applicationArgumentCount; ++i)
    {
        gcFrame.applicationArgumentNode = sysbvm_array_at(gcFrame.functionApplicationNode->arguments, i);
        gcFrame.analyzedArgument = sysbvm_interpreter_analyzeASTWithDecayedTypeWithEnvironment(context, gcFrame.applicationArgumentNode, *environment);
        sysbvm_array_atPut(gcFrame.analyzedArguments, i, gcFrame.analyzedArgument);
    }

    gcFrame.functionApplicationNode->arguments = gcFrame.analyzedArguments;

    if((*dependentFunctionType)->resultTypeNode)
        gcFrame.resultType = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*dependentFunctionType)->resultTypeNode, gcFrame.applicationEnvironment);
    gcFrame.resultType = sysbvm_type_canonicalizeDependentResultType(context, gcFrame.resultType);
    gcFrame.functionApplicationNode->applicationFlags = sysbvm_tuple_bitflags_encode(sysbvm_tuple_bitflags_decode(gcFrame.functionApplicationNode->applicationFlags)
        | SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK | SYSBVM_FUNCTION_APPLICATION_FLAGS_VARIADIC_EXPANDED);
    gcFrame.functionApplicationNode->super.analyzedType = gcFrame.resultType;

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    sysbvm_stackFrame_popRecord((sysbvm_stackFrameRecord_t*)&functionActivationRecord);  
    return sysbvm_astFunctionApplicationNode_optimizePureApplication(context, (sysbvm_astFunctionApplicationNode_t**)node);
}

static sysbvm_tuple_t sysbvm_dependentFunctionType_primitiveAnalyzeAndTypeCheckMessageSendNode(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 3) sysbvm_error_argumentCountMismatch(3, argumentCount);

    sysbvm_dependentFunctionType_t **dependentFunctionType = (sysbvm_dependentFunctionType_t **)&arguments[0];
    sysbvm_tuple_t *node = &arguments[1];
    sysbvm_tuple_t *environment = &arguments[2];

    struct {
        sysbvm_astMessageSendNode_t *sendNode;

        sysbvm_tuple_t applicationEnvironment;
        sysbvm_tuple_t argumentNode;

        sysbvm_tuple_t applicationArgumentNode;
        sysbvm_tuple_t applicationArgumentValue;
        sysbvm_tuple_t resultType;

        sysbvm_tuple_t analyzedArgument;
        sysbvm_tuple_t analyzedArguments;
        sysbvm_tuple_t expectedArgumentType;
    } gcFrame = {
        .sendNode = (sysbvm_astMessageSendNode_t*)*node,
    };

    sysbvm_stackFrameFunctionActivationRecord_t functionActivationRecord = {
        .type = SYSBVM_STACK_FRAME_RECORD_TYPE_FUNCTION_ACTIVATION,
    };
    sysbvm_stackFrame_pushRecord((sysbvm_stackFrameRecord_t*)&functionActivationRecord); 
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, gcFrame.sendNode->super.sourcePosition);

    gcFrame.applicationEnvironment = sysbvm_functionActivationEnvironment_createForDependentFunctionType(context, SYSBVM_NULL_TUPLE, (sysbvm_tuple_t)*dependentFunctionType);

    size_t argumentNodeCount = sysbvm_array_getSize((*dependentFunctionType)->argumentNodes);
    
    size_t directEvaluationArgumentNodeCount = argumentNodeCount;

    size_t applicationArgumentCount = sysbvm_array_getSize(gcFrame.sendNode->arguments);
    size_t sourceArgumentIndex = 0;
    bool hasAnalyzedReceiver = false;

    gcFrame.analyzedArguments = sysbvm_array_create(context, applicationArgumentCount);
    for(size_t i = 0; i < directEvaluationArgumentNodeCount; ++i)
    {
        gcFrame.argumentNode = sysbvm_array_at((*dependentFunctionType)->argumentNodes, i);
        if(sysbvm_astArgumentNode_isForAll(argumentNodeCount))
        {
            sysbvm_error("TODO: Support forall argument types");
        }
        else
        {
            bool isReceiverArgument = !hasAnalyzedReceiver && gcFrame.sendNode->receiver;
            if(isReceiverArgument)
            {
                gcFrame.applicationArgumentNode = gcFrame.sendNode->receiver;
            }
            else
            {
                if(sourceArgumentIndex >= applicationArgumentCount)
                    sysbvm_error("Message send is missing required arguments.");
                gcFrame.applicationArgumentNode = sysbvm_array_at(gcFrame.sendNode->arguments, sourceArgumentIndex);
            }

            gcFrame.expectedArgumentType = sysbvm_interpreter_evaluateArgumentNodeTypeInEnvironment(context, gcFrame.argumentNode, &gcFrame.applicationEnvironment);
            gcFrame.analyzedArgument = sysbvm_interpreter_analyzeASTWithExpectedTypeWithEnvironment(context, gcFrame.applicationArgumentNode, gcFrame.expectedArgumentType, *environment);
            if(isReceiverArgument)
                gcFrame.sendNode->receiver = gcFrame.analyzedArgument;
            else
                sysbvm_array_atPut(gcFrame.analyzedArguments, sourceArgumentIndex, gcFrame.analyzedArgument);
            gcFrame.applicationArgumentValue = sysbvm_dependentFunctionType_getOrCreateDependentApplicationValueForNode(context, gcFrame.analyzedArgument);
            sysbvm_interpreter_bindArgumentNodeValueInEnvironment(context, i, &gcFrame.applicationEnvironment, gcFrame.applicationArgumentValue);
            if(isReceiverArgument)
                hasAnalyzedReceiver = true;
            else
                ++sourceArgumentIndex;
        }
    }

    if(sourceArgumentIndex != applicationArgumentCount)
        sysbvm_error("Message send is not receiving the expected number of arguments.");

    gcFrame.sendNode->arguments = gcFrame.analyzedArguments;

    if((*dependentFunctionType)->resultTypeNode)
        gcFrame.resultType = sysbvm_interpreter_evaluateASTWithEnvironment(context, (*dependentFunctionType)->resultTypeNode, gcFrame.applicationEnvironment);
    gcFrame.resultType = sysbvm_type_canonicalizeDependentResultType(context, gcFrame.resultType);
    gcFrame.sendNode->super.analyzedType = gcFrame.resultType;
    gcFrame.sendNode->applicationFlags = sysbvm_tuple_bitflags_encode(sysbvm_tuple_bitflags_decode(gcFrame.sendNode->applicationFlags)
        | SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK | SYSBVM_FUNCTION_APPLICATION_FLAGS_VARIADIC_EXPANDED);

    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    sysbvm_stackFrame_popRecord((sysbvm_stackFrameRecord_t*)&functionActivationRecord);  
    return sysbvm_astFunctionApplicationNode_optimizePureApplication(context, (sysbvm_astFunctionApplicationNode_t**)node);
}

SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_applyClosureASTFunction(sysbvm_context_t *context, sysbvm_tuple_t function_, size_t argumentCount, sysbvm_tuple_t *arguments, sysbvm_bitflags_t applicationFlags)
{
    sysbvm_stackFrameFunctionActivationRecord_t functionActivationRecord = {
        .type = SYSBVM_STACK_FRAME_RECORD_TYPE_FUNCTION_ACTIVATION,
        .function = function_,
    };
    sysbvm_stackFrame_pushRecord((sysbvm_stackFrameRecord_t*)&functionActivationRecord);  
    sysbvm_function_t **closure = (sysbvm_function_t**)&functionActivationRecord.function;

    functionActivationRecord.functionDefinition = (*closure)->definition;
    sysbvm_functionDefinition_t **functionDefinition = (sysbvm_functionDefinition_t**)&functionActivationRecord.functionDefinition;

    sysbvm_functionSourceAnalyzedDefinition_t *sourceAnalyzedDefinition = (sysbvm_functionSourceAnalyzedDefinition_t*)(*functionDefinition)->sourceAnalyzedDefinition;

    SYSBVM_STACKFRAME_PUSH_SOURCE_POSITION(sourcePositionRecord, sourceAnalyzedDefinition->sourcePosition);
    sysbvm_functionDefinition_ensureAnalysis(context, functionDefinition);

    size_t expectedArgumentCount = sysbvm_array_getSize(sourceAnalyzedDefinition->argumentNodes);
    if(argumentCount != expectedArgumentCount)
        sysbvm_error_argumentCountMismatch(expectedArgumentCount, argumentCount);
    functionActivationRecord.applicationEnvironment = sysbvm_functionActivationEnvironment_create(context, SYSBVM_NULL_TUPLE, functionActivationRecord.function);
    sysbvm_analysisAndEvaluationEnvironment_setReturnTarget(context, functionActivationRecord.applicationEnvironment, sysbvm_tuple_uintptr_encode(context, (uintptr_t)&functionActivationRecord));

    bool isNoTypecheck = (applicationFlags & SYSBVM_FUNCTION_APPLICATION_FLAGS_NO_TYPECHECK) != 0;
    if(isNoTypecheck)
    {
        // Avoid the coercion here.
        for(size_t i = 0; i < argumentCount; ++i)
            sysbvm_interpreter_bindArgumentNodeValueInEnvironment(context, i, &functionActivationRecord.applicationEnvironment, arguments[i]);
    }
    else
    {
        // FIXME: Add support for the forall arguments here.
        for(size_t i = 0; i < argumentCount; ++i)
            sysbvm_interpreter_evaluateArgumentNodeInEnvironment(context, i, sysbvm_array_at(sourceAnalyzedDefinition->argumentNodes, i), &functionActivationRecord.applicationEnvironment, &arguments[i]);
    }

    // Use setjmp for implementing the #return: statement.
    if(!_setjmp(functionActivationRecord.jmpbuffer))
    {
        sysbvm_gc_safepoint(context);
        functionActivationRecord.result = sysbvm_interpreter_evaluateASTWithEnvironment(context, sourceAnalyzedDefinition->bodyNode, functionActivationRecord.applicationEnvironment);
    }

    sysbvm_gc_safepoint(context);
    
    SYSBVM_STACKFRAME_POP_SOURCE_POSITION(sourcePositionRecord);
    sysbvm_stackFrame_popRecord((sysbvm_stackFrameRecord_t*)&functionActivationRecord);  
    return functionActivationRecord.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_evaluateScript(sysbvm_context_t *context, sysbvm_tuple_t script, sysbvm_tuple_t name, sysbvm_tuple_t language)
{
    struct {
        sysbvm_tuple_t script;
        sysbvm_tuple_t name;
        sysbvm_tuple_t language;
        sysbvm_tuple_t sourceLanguage;
        sysbvm_tuple_t sourceCode;
        sysbvm_tuple_t sourceEnvironment;
        sysbvm_tuple_t result;
    } gcFrame = {
        .script = script,
        .name = name,
        .language = language
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.sourceCode = sysbvm_sourceCode_create(context, gcFrame.script, SYSBVM_NULL_TUPLE, gcFrame.name, gcFrame.language);
    gcFrame.sourceEnvironment = sysbvm_environment_createDefaultForSourceCodeEvaluation(context, gcFrame.sourceCode);
    gcFrame.result = sysbvm_interpreter_validateThenAnalyzeAndEvaluateSourceCodeWithEnvironment(context, gcFrame.sourceEnvironment, gcFrame.sourceCode);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_loadSourceCodeWithSolvedPath(sysbvm_context_t *context, sysbvm_tuple_t filename)
{
    struct {
        sysbvm_tuple_t filename;
        sysbvm_tuple_t sourceString;
        sysbvm_tuple_t sourceDirectory;
        sysbvm_tuple_t sourceName;
        sysbvm_tuple_t sourceLanguage;
        sysbvm_tuple_t sourceCode;
    } gcFrame = {
        .filename = filename,
    };

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.sourceString = sysbvm_io_readWholeFileNamedAsString(context, gcFrame.filename);
    if(!gcFrame.sourceString)
        sysbvm_errorWithMessageTuple(sysbvm_string_concat(context,
            sysbvm_symbol_internWithCString(context, "Failed to load source file from: "),
            gcFrame.filename));

    gcFrame.sourceDirectory = sysbvm_filesystem_dirname(context, gcFrame.filename);
    gcFrame.sourceName = sysbvm_filesystem_basename(context, gcFrame.filename);
    gcFrame.sourceLanguage = sysbvm_sourceCode_inferLanguageFromSourceName(context, gcFrame.sourceName);
    gcFrame.sourceCode = sysbvm_sourceCode_create(context, gcFrame.sourceString, gcFrame.sourceDirectory, gcFrame.sourceName, gcFrame.sourceLanguage);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.sourceCode;
}

SYSBVM_API sysbvm_tuple_t sysbvm_interpreter_loadSourceNamedWithSolvedPath(sysbvm_context_t *context, sysbvm_tuple_t filename)
{
    struct {
        sysbvm_tuple_t sourceCode;
        sysbvm_tuple_t sourceEnvironment;
        sysbvm_tuple_t result;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);
    gcFrame.sourceCode = sysbvm_interpreter_loadSourceCodeWithSolvedPath(context, filename);
    gcFrame.sourceEnvironment = sysbvm_environment_createDefaultForSourceCodeEvaluation(context, gcFrame.sourceCode);
    gcFrame.result = sysbvm_interpreter_validateThenAnalyzeAndEvaluateSourceCodeWithEnvironment(context, gcFrame.sourceEnvironment, gcFrame.sourceCode);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

static sysbvm_tuple_t sysbvm_interpreter_primitive_loadSourceNamedWithSolvedPath(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)closure;
    if(argumentCount != 1) sysbvm_error_argumentCountMismatch(1, argumentCount);

    return sysbvm_interpreter_loadSourceNamedWithSolvedPath(context, arguments[0]);
}

static sysbvm_tuple_t sysbvm_interpreter_primitive_loadSourceNamedMacro(sysbvm_context_t *context, sysbvm_tuple_t closure, size_t argumentCount, sysbvm_tuple_t *arguments)
{
    (void)context;
    (void)closure;
    if(argumentCount != 2) sysbvm_error_argumentCountMismatch(2, argumentCount);

    sysbvm_tuple_t *macroContext = &arguments[0];
    sysbvm_tuple_t *sourceName = &arguments[1];

    sysbvm_tuple_t sourcePosition = sysbvm_macroContext_getSourcePosition(*macroContext);

    sysbvm_tuple_t sourceDirectory = sysbvm_astIdentifierReferenceNode_create(context, sourcePosition, sysbvm_symbol_internWithCString(context, "__SourceDirectory__"));
    sysbvm_tuple_t solveNameArguments = sysbvm_array_create(context, 2);
    sysbvm_array_atPut(solveNameArguments, 0, sourceDirectory);
    sysbvm_array_atPut(solveNameArguments, 1, *sourceName);

    sysbvm_tuple_t solveNameFunction = sysbvm_astIdentifierReferenceNode_create(context, sourcePosition, sysbvm_symbol_internWithCString(context, "FileSystem::joinPath:"));
    sysbvm_tuple_t solveNameCall = sysbvm_astUnexpandedApplicationNode_create(context, sourcePosition, solveNameFunction, solveNameArguments);

    sysbvm_tuple_t loadSourceArguments = sysbvm_array_create(context, 1);
    sysbvm_array_atPut(loadSourceArguments, 0, solveNameCall);

    sysbvm_tuple_t loadSourceNamedWithSolvedPath = sysbvm_astIdentifierReferenceNode_create(context, sourcePosition, sysbvm_symbol_internWithCString(context, "loadSourceNamedWithSolvedPath:"));
    sysbvm_tuple_t loadSourceCall = sysbvm_astUnexpandedApplicationNode_create(context, sourcePosition, loadSourceNamedWithSolvedPath, loadSourceArguments);
    return loadSourceCall;
}

void sysbvm_astInterpreter_registerPrimitives(void)
{
    sysbvm_primitiveTable_registerFunction(sysbvm_interpreter_primitive_loadSourceNamedWithSolvedPath, "Interpreter::loadSourceNamedWithSolvedPath:");
    sysbvm_primitiveTable_registerFunction(sysbvm_interpreter_primitive_loadSourceNamedMacro, "Interpreter::loadSourceNamedMacro:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astSequenceNode_primitiveMacro, "ASTSequenceNode::beginMacro");

    sysbvm_primitiveTable_registerFunction(sysbvm_astArgumentNode_primitiveAnalyze, "ASTArgumentNode::analyzeWithEnvironment:");
    
    sysbvm_primitiveTable_registerFunction(sysbvm_astCoerceValueNode_primitiveMacro, "ASTCoerceValueNode::macro");
    sysbvm_primitiveTable_registerFunction(sysbvm_astCoerceValueNode_primitiveAnalyze, "ASTCoerceValueNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astCoerceValueNode_primitiveEvaluate, "ASTCoerceValueNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astCoerceValueNode_primitiveAnalyzeAndEvaluate, "ASTCoerceValueNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astDownCastNode_primitiveMacro, "ASTDownCastNode::macro");
    sysbvm_primitiveTable_registerFunction(sysbvm_astDownCastNode_primitiveAnalyze, "ASTDownCastNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astDownCastNode_primitiveEvaluate, "ASTDownCastNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astDownCastNode_primitiveAnalyzeAndEvaluate, "ASTDownCastNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astErrorNode_primitiveEvaluate, "ASTErrorNode::analyzeWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astPragmaNode_primitiveAnalyze, "ASTPragmaNode::analyzeWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astSequenceNode_primitiveAnalyze, "ASTSequenceNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astSequenceNode_primitiveEvaluate, "ASTSequenceNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astSequenceNode_primitiveAnalyzeAndEvaluate, "ASTSequenceNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astLiteralNode_primitiveAnalyze, "ASTLiteralNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astLiteralNode_primitiveEvaluate, "ASTLiteralNode::evaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astIdentifierReferenceNode_primitiveAnalyze, "ASTIdentifierReferenceNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astIdentifierReferenceNode_primitiveEvaluate, "ASTIdentifierReferenceNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astIdentifierReferenceNode_primitiveAnalyzeAndEvaluate, "ASTIdentifierReferenceNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astUnexpandedApplicationNode_primitiveAnalyze, "ASTUnexpandedApplicationNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astUnexpandedApplicationNode_primitiveAnalyzeAndEvaluate, "ASTUnexpandedApplicationNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astUnexpandedSExpressionNode_primitiveAnalyze, "ASTUnexpandedSExpressionNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astUnexpandedSExpressionNode_primitiveAnalyzeAndEvaluate, "ASTUnexpandedSExpressionNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astFunctionApplicationNode_primitiveAnalyze, "ASTFunctionApplicationNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astFunctionApplicationNode_primitiveEvaluate, "ASTFunctionApplicationNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astFunctionApplicationNode_primitiveAnalyzeAndEvaluate, "ASTFunctionApplicationNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astLexicalBlockNode_primitiveAnalyze, "ASTLexicalBlockNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astLexicalBlockNode_primitiveEvaluate, "ASTLexicalBlockNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astLexicalBlockNode_primitiveAnalyzeAndEvaluate, "ASTLexicalBlockNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeAssociationNode_primitiveAnalyze, "ASTMakeAssociationNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeAssociationNode_primitiveEvaluate, "ASTMakeAssociationNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeAssociationNode_primitiveAnalyzeAndEvaluate, "ASTMakeAssociationNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeByteArrayNode_primitiveAnalyze, "ASTMakeByteArrayNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeByteArrayNode_primitiveEvaluate, "ASTMakeByteArrayNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeByteArrayNode_primitiveAnalyzeAndEvaluate, "ASTMakeByteArrayNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeDictionaryNode_primitiveAnalyze, "ASTMakeDictionaryNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeDictionaryNode_primitiveEvaluate, "ASTMakeDictionaryNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeDictionaryNode_primitiveAnalyzeAndEvaluate, "ASTMakeDictionaryNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeArrayNode_primitiveAnalyze, "ASTMakeArrayNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeArrayNode_primitiveEvaluate, "ASTMakeArrayNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMakeArrayNode_primitiveAnalyzeAndEvaluate, "ASTMakeArrayNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astMessageChainNode_primitiveAnalyze, "ASTMessageChainNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMessageChainNode_primitiveEvaluate, "ASTMessageChainNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMessageChainNode_primitiveAnalyzeAndEvaluate, "ASTMessageChainNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astMessageSendNode_primitiveMacro, "ASTMessageSendNode::sendMacro");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMessageSendNode_primitiveAnalyze, "ASTMessageSendNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMessageSendNode_primitiveEvaluate, "ASTMessageSendNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astMessageSendNode_primitiveAnalyzeAndEvaluate, "ASTMessageSendNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astLambdaNode_primitiveMacro, "ASTLambdaNode::lambdaMacro");
    sysbvm_primitiveTable_registerFunction(sysbvm_astLambdaNode_primitiveAnalyze, "ASTLambdaNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astLambdaNode_primitiveEvaluate, "ASTLambdaNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astLambdaNode_primitiveAnalyzeAndEvaluate, "ASTLambdaNode::analyzeWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astVariableDefinitionNode_primitiveMacro, "ASTVariableDefinitionNode::macro:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astVariableDefinitionNode_primitiveDefineMacro, "ASTVariableDefinitionNode::defineMacro");
    sysbvm_primitiveTable_registerFunction(sysbvm_astVariableDefinitionNode_letWithPrimitiveMacro, "ASTVariableDefinitionNode::let:with:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astVariableDefinitionNode_letTypeWithPrimitiveMacro, "ASTVariableDefinitionNode::let:type:with:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astVariableDefinitionNode_letMutableWithPrimitiveMacro, "ASTVariableDefinitionNode::let:mutableWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astVariableDefinitionNode_letTypeMutableWithPrimitiveMacro, "ASTVariableDefinitionNode::let:type:mutableWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astVariableDefinitionNode_macroLetWithPrimitiveMacro, "ASTVariableDefinitionNode::macroLet:with:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astVariableDefinitionNode_primitiveAnalyze, "ASTVariableDefinitionNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astVariableDefinitionNode_primitiveEvaluate, "ASTVariableDefinitionNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astVariableDefinitionNode_primitiveAnalyzeAndEvaluate, "ASTVariableDefinitionNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleWithLookupStartingFromNode_primitiveMacro, "ASTTupleWithLookupStartingFromNode::macro");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleWithLookupStartingFromNode_primitiveAnalyze, "ASTTupleWithLookupStartingFromNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleWithLookupStartingFromNode_primitiveEvaluate, "ASTTupleWithLookupStartingFromNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleWithLookupStartingFromNode_primitiveAnalyzeAndEvaluate, "ASTTupleWithLookupStartingFromNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedAtNode_primitiveMacro, "ASTTupleSlotNamedAtNode::macro");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedAtNode_primitiveAnalyze, "ASTTupleSlotNamedAtNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedAtNode_primitiveEvaluate, "ASTTupleSlotNamedAtNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedAtNode_primitiveAnalyzeAndEvaluate, "ASTTupleSlotNamedAtNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedReferenceAtNode_primitiveMacro, "ASTTupleSlotNamedReferenceAtNode::macro");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedReferenceAtNode_primitiveAnalyze, "ASTTupleSlotNamedReferenceAtNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedReferenceAtNode_primitiveEvaluate, "ASTTupleSlotNamedReferenceAtNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedReferenceAtNode_primitiveAnalyzeAndEvaluate, "ASTTupleSlotNamedReferenceAtNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedAtPutNode_primitiveMacro, "ASTTupleSlotNamedAtPutNode::macro");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedAtPutNode_primitiveAnalyze, "ASTTupleSlotNamedAtPutNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedAtPutNode_primitiveEvaluate, "ASTTupleSlotNamedAtPutNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astTupleSlotNamedAtPutNode_primitiveAnalyzeAndEvaluate, "ASTTupleSlotNamedAtPutNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astIfNode_primitiveMacro, "ASTIfNode::if:then:else:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astIfNode_primitiveMacroIfThen, "ASTIfNode::if:then:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astIfNode_primitiveAnalyze, "ASTIfNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astIfNode_primitiveEvaluate, "ASTIfNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astIfNode_primitiveAnalyzeAndEvaluate, "ASTIfNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astDoWhileContinueWithNode_primitiveMacro, "ASTDoWhileContinueWithNode::do:while:continueWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astDoWhileContinueWithNode_primitiveDoWhileMacro, "ASTDoWhileContinueWithNode::do:while:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astDoWhileContinueWithNode_primitiveAnalyze, "ASTDoWhileContinueWithNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astDoWhileContinueWithNode_primitiveEvaluate, "ASTDoWhileContinueWithNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astDoWhileContinueWithNode_primitiveAnalyzeAndEvaluate, "ASTDoWhileContinueWithNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astUseNamedSlotsOfNode_primitiveMacro, "ASTUseNamedSlotsOfNode::useNamedSlotsOf:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astUseNamedSlotsOfNode_primitiveAnalyze, "ASTUseNamedSlotsOfNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astUseNamedSlotsOfNode_primitiveEvaluate, "ASTUseNamedSlotsOfNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astUseNamedSlotsOfNode_primitiveAnalyzeAndEvaluate, "ASTUseNamedSlotsOfNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astWhileContinueWithNode_primitiveMacro, "ASTWhileContinueWithNode::while:do:continueWith:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astWhileContinueWithNode_primitiveWhileDoMacro, "ASTWhileContinueWithNode::while:do:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astWhileContinueWithNode_primitiveAnalyze, "ASTWhileContinueWithNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astWhileContinueWithNode_primitiveEvaluate, "ASTWhileContinueWithNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astWhileContinueWithNode_primitiveAnalyzeAndEvaluate, "ASTWhileContinueWithNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astBreakNode_primitiveMacro, "ASTBreakNode::macro");
    sysbvm_primitiveTable_registerFunction(sysbvm_astBreakNode_primitiveAnalyze, "ASTBreakNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astBreakNode_primitiveEvaluate, "ASTBreakNode::evaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astContinueNode_primitiveMacro, "ASTContinueNode::macro");
    sysbvm_primitiveTable_registerFunction(sysbvm_astContinueNode_primitiveAnalyze, "ASTContinueNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astContinueNode_primitiveEvaluate, "ASTContinueNode::evaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_astReturnNode_primitiveMacro, "ASTReturnNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astReturnNode_primitiveAnalyze, "ASTReturnNode::analyzeWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astReturnNode_primitiveEvaluate, "ASTReturnNode::evaluateWithEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_astReturnNode_primitiveAnalyzeAndEvaluate, "ASTReturnNode::analyzeAndEvaluateWithEnvironment:");

    sysbvm_primitiveTable_registerFunction(sysbvm_function_primitiveEnsureAnalysis, "Function::ensureAnalysis");
    sysbvm_primitiveTable_registerFunction(sysbvm_functionDefinition_primitiveEnsureTypeAnalysis, "FunctionDefinition::ensureTypeAnalysis");
    sysbvm_primitiveTable_registerFunction(sysbvm_functionDefinition_primitiveEnsureAnalysis, "FunctionDefinition::ensureAnalysis");

    sysbvm_primitiveTable_registerFunction(sysbvm_simpleFunctionType_primitiveAnalyzeAndTypeCheckFunctionApplicationNode, "SimpleFunctionType::analyzeAndTypeCheckFunctionApplicationNode:withEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_simpleFunctionType_primitiveAnalyzeAndTypeCheckMessageSendNode, "SimpleFunctionType::analyzeAndTypeCheckSolvedMessageSendNode:withEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_dependentFunctionType_primitiveAnalyzeAndTypeCheckFunctionApplicationNode, "DependentFunctionType::analyzeAndTypeCheckFunctionApplicationNode:withEnvironment:");
    sysbvm_primitiveTable_registerFunction(sysbvm_dependentFunctionType_primitiveAnalyzeAndTypeCheckMessageSendNode, "DependentFunctionType::analyzeAndTypeCheckSolvedMessageSendNode:withEnvironment:");
}

static void sysbvm_astInterpreter_setupNodeInterpretationFunctions(sysbvm_context_t *context, sysbvm_tuple_t astNodeType, sysbvm_functionEntryPoint_t analysisFunction, sysbvm_functionEntryPoint_t evaluationFunction, sysbvm_functionEntryPoint_t analysisAndEvaluationFunction)
{
    if(analysisFunction)
        sysbvm_type_setAstNodeAnalysisFunction(context, astNodeType, sysbvm_function_createPrimitive(context, 2, SYSBVM_FUNCTION_FLAGS_NONE, NULL, analysisFunction));
    if(evaluationFunction)
        sysbvm_type_setAstNodeEvaluationFunction(context, astNodeType, sysbvm_function_createPrimitive(context, 2, SYSBVM_FUNCTION_FLAGS_NONE, NULL, evaluationFunction));
    if(analysisAndEvaluationFunction)
        sysbvm_type_setAstNodeAnalysisAndEvaluationFunction(context, astNodeType, sysbvm_function_createPrimitive(context, 2, SYSBVM_FUNCTION_FLAGS_NONE, NULL, analysisAndEvaluationFunction));
}

void sysbvm_astInterpreter_setupASTInterpreter(sysbvm_context_t *context)
{
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "loadSourceNamedWithSolvedPath:", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_interpreter_primitive_loadSourceNamedWithSolvedPath);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "loadSourceNamed:", 2, SYSBVM_FUNCTION_FLAGS_MACRO | SYSBVM_FUNCTION_FLAGS_NONE, NULL, sysbvm_interpreter_primitive_loadSourceNamedMacro);

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "begin", 2, SYSBVM_FUNCTION_FLAGS_MACRO | SYSBVM_FUNCTION_FLAGS_VARIADIC, NULL, sysbvm_astSequenceNode_primitiveMacro);

    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.functionType, "ensureAnalysis", 1, SYSBVM_FUNCTION_FLAGS_NONE, NULL, sysbvm_function_primitiveEnsureAnalysis);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.functionDefinitionType, "ensureTypeAnalysis", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_functionDefinition_primitiveEnsureTypeAnalysis);
    sysbvm_context_setIntrinsicPrimitiveMethod(context, context->roots.functionDefinitionType, "ensureAnalysis", 1, SYSBVM_FUNCTION_FLAGS_CORE_PRIMITIVE, NULL, sysbvm_functionDefinition_primitiveEnsureAnalysis);

    sysbvm_type_setAnalyzeAndTypeCheckFunctionApplicationNodeWithEnvironment(context, context->roots.simpleFunctionTypeType, sysbvm_function_createPrimitive(context, 3, SYSBVM_FUNCTION_FLAGS_NONE, NULL, sysbvm_simpleFunctionType_primitiveAnalyzeAndTypeCheckFunctionApplicationNode));
    sysbvm_type_setAnalyzeAndTypeCheckMessageSendNodeWithEnvironment(context, context->roots.simpleFunctionTypeType, sysbvm_function_createPrimitive(context, 3, SYSBVM_FUNCTION_FLAGS_NONE, NULL, sysbvm_simpleFunctionType_primitiveAnalyzeAndTypeCheckMessageSendNode));

    sysbvm_type_setAnalyzeAndTypeCheckFunctionApplicationNodeWithEnvironment(context, context->roots.dependentFunctionTypeType, sysbvm_function_createPrimitive(context, 3, SYSBVM_FUNCTION_FLAGS_NONE, NULL, sysbvm_dependentFunctionType_primitiveAnalyzeAndTypeCheckFunctionApplicationNode));
    sysbvm_type_setAnalyzeAndTypeCheckMessageSendNodeWithEnvironment(context, context->roots.dependentFunctionTypeType, sysbvm_function_createPrimitive(context, 3, SYSBVM_FUNCTION_FLAGS_NONE, NULL, sysbvm_dependentFunctionType_primitiveAnalyzeAndTypeCheckMessageSendNode));

    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astArgumentNodeType,
        sysbvm_astArgumentNode_primitiveAnalyze,
        NULL,
        NULL
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "tuple:coerceTo:", 3, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astCoerceValueNode_primitiveMacro);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astCoerceValueNodeType,
        sysbvm_astCoerceValueNode_primitiveAnalyze,
        sysbvm_astCoerceValueNode_primitiveEvaluate,
        sysbvm_astCoerceValueNode_primitiveAnalyzeAndEvaluate
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "tuple:downCastTo:", 3, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astDownCastNode_primitiveMacro);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astDownCastNodeType,
        sysbvm_astDownCastNode_primitiveAnalyze,
        sysbvm_astDownCastNode_primitiveEvaluate,
        sysbvm_astDownCastNode_primitiveAnalyzeAndEvaluate
    );

    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astErrorNodeType,
        sysbvm_astErrorNode_primitiveEvaluate,
        sysbvm_astErrorNode_primitiveEvaluate,
        sysbvm_astErrorNode_primitiveEvaluate
    );
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astPragmaNodeType,
        sysbvm_astPragmaNode_primitiveAnalyze,
        NULL,
        NULL
    );
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astSequenceNodeType,
        sysbvm_astSequenceNode_primitiveAnalyze,
        sysbvm_astSequenceNode_primitiveEvaluate,
        sysbvm_astSequenceNode_primitiveAnalyzeAndEvaluate
    );
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astLiteralNodeType,
        sysbvm_astLiteralNode_primitiveAnalyze,
        sysbvm_astLiteralNode_primitiveEvaluate,
        sysbvm_astLiteralNode_primitiveEvaluate
    );
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astIdentifierReferenceNodeType,
        sysbvm_astIdentifierReferenceNode_primitiveAnalyze,
        sysbvm_astIdentifierReferenceNode_primitiveEvaluate,
        sysbvm_astIdentifierReferenceNode_primitiveAnalyzeAndEvaluate
    );
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astUnexpandedApplicationNodeType,
        sysbvm_astUnexpandedApplicationNode_primitiveAnalyze,
        NULL,
        sysbvm_astUnexpandedApplicationNode_primitiveAnalyzeAndEvaluate
    );
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astUnexpandedSExpressionNodeType,
        sysbvm_astUnexpandedSExpressionNode_primitiveAnalyze,
        NULL,
        sysbvm_astUnexpandedSExpressionNode_primitiveAnalyzeAndEvaluate
    );
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astFunctionApplicationNodeType,
        sysbvm_astFunctionApplicationNode_primitiveAnalyze,
        sysbvm_astFunctionApplicationNode_primitiveEvaluate,
        sysbvm_astFunctionApplicationNode_primitiveAnalyzeAndEvaluate
    );
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astLexicalBlockNodeType,
        sysbvm_astLexicalBlockNode_primitiveAnalyze,
        sysbvm_astLexicalBlockNode_primitiveEvaluate,
        sysbvm_astLexicalBlockNode_primitiveAnalyzeAndEvaluate
    );
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astMakeAssociationNodeType,
        sysbvm_astMakeAssociationNode_primitiveAnalyze,
        sysbvm_astMakeAssociationNode_primitiveEvaluate,
        sysbvm_astMakeAssociationNode_primitiveAnalyzeAndEvaluate
    );
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astMakeByteArrayNodeType,
        sysbvm_astMakeByteArrayNode_primitiveAnalyze,
        sysbvm_astMakeByteArrayNode_primitiveEvaluate,
        sysbvm_astMakeByteArrayNode_primitiveAnalyzeAndEvaluate
    );
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astMakeDictionaryNodeType,
        sysbvm_astMakeDictionaryNode_primitiveAnalyze,
        sysbvm_astMakeDictionaryNode_primitiveEvaluate,
        sysbvm_astMakeDictionaryNode_primitiveAnalyzeAndEvaluate
    );
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astMakeArrayNodeType,
        sysbvm_astMakeArrayNode_primitiveAnalyze,
        sysbvm_astMakeArrayNode_primitiveEvaluate,
        sysbvm_astMakeArrayNode_primitiveAnalyzeAndEvaluate
    );
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astMessageChainNodeType,
        sysbvm_astMessageChainNode_primitiveAnalyze,
        sysbvm_astMessageChainNode_primitiveEvaluate,
        sysbvm_astMessageChainNode_primitiveAnalyzeAndEvaluate
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "send", 4, SYSBVM_FUNCTION_FLAGS_MACRO | SYSBVM_FUNCTION_FLAGS_VARIADIC, NULL, sysbvm_astMessageSendNode_primitiveMacro);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astMessageSendNodeType,
        sysbvm_astMessageSendNode_primitiveAnalyze,
        sysbvm_astMessageSendNode_primitiveEvaluate,
        sysbvm_astMessageSendNode_primitiveAnalyzeAndEvaluate
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "lambda", 3, SYSBVM_FUNCTION_FLAGS_MACRO | SYSBVM_FUNCTION_FLAGS_VARIADIC, NULL, sysbvm_astLambdaNode_primitiveMacro);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astLambdaNodeType,
        sysbvm_astLambdaNode_primitiveAnalyze,
        sysbvm_astLambdaNode_primitiveEvaluate,
        sysbvm_astLambdaNode_primitiveAnalyzeAndEvaluate
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "define", 3, SYSBVM_FUNCTION_FLAGS_MACRO | SYSBVM_FUNCTION_FLAGS_VARIADIC, NULL, sysbvm_astVariableDefinitionNode_primitiveMacro);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "defineMacro", 3, SYSBVM_FUNCTION_FLAGS_MACRO | SYSBVM_FUNCTION_FLAGS_VARIADIC, NULL, sysbvm_astVariableDefinitionNode_primitiveDefineMacro);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "let:with:", 3, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astVariableDefinitionNode_letWithPrimitiveMacro);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "let:type:with:", 4, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astVariableDefinitionNode_letTypeWithPrimitiveMacro);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "let:mutableWith:", 3, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astVariableDefinitionNode_letMutableWithPrimitiveMacro);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "let:type:mutableWith:", 4, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astVariableDefinitionNode_letTypeMutableWithPrimitiveMacro);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "macroLet:with:", 3, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astVariableDefinitionNode_macroLetWithPrimitiveMacro);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astVariableDefinitionNodeType,
        sysbvm_astVariableDefinitionNode_primitiveAnalyze,
        sysbvm_astVariableDefinitionNode_primitiveEvaluate,
        sysbvm_astVariableDefinitionNode_primitiveAnalyzeAndEvaluate
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "tuple:withLookupStartingFrom:", 3, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astTupleWithLookupStartingFromNode_primitiveMacro);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astTupleWithLookupStartingFromNodeType,
        sysbvm_astTupleWithLookupStartingFromNode_primitiveAnalyze,
        sysbvm_astTupleWithLookupStartingFromNode_primitiveEvaluate,
        sysbvm_astTupleWithLookupStartingFromNode_primitiveAnalyzeAndEvaluate
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "tuple:slotNamedAt:", context->roots.anyValueType, "__slotNamedAt__:", 3, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astTupleSlotNamedAtNode_primitiveMacro);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astTupleSlotNamedAtNodeType,
        sysbvm_astTupleSlotNamedAtNode_primitiveAnalyze,
        sysbvm_astTupleSlotNamedAtNode_primitiveEvaluate,
        sysbvm_astTupleSlotNamedAtNode_primitiveAnalyzeAndEvaluate
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "tuple:slotNamedReferenceAt:", context->roots.anyValueType, "__slotNamedReferenceAt__:", 3, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astTupleSlotNamedReferenceAtNode_primitiveMacro);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astTupleSlotNamedReferenceAtNodeType,
        sysbvm_astTupleSlotNamedReferenceAtNode_primitiveAnalyze,
        sysbvm_astTupleSlotNamedReferenceAtNode_primitiveEvaluate,
        sysbvm_astTupleSlotNamedReferenceAtNode_primitiveAnalyzeAndEvaluate
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "tuple:slotNamedAt:put:", context->roots.anyValueType, "__slotNamedAt__:put:", 4, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astTupleSlotNamedAtPutNode_primitiveMacro);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astTupleSlotNamedAtPutNodeType,
        sysbvm_astTupleSlotNamedAtPutNode_primitiveAnalyze,
        sysbvm_astTupleSlotNamedAtPutNode_primitiveEvaluate,
        sysbvm_astTupleSlotNamedAtPutNode_primitiveAnalyzeAndEvaluate
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "if:then:else:", 4, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astIfNode_primitiveMacro);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "if:then:", 3, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astIfNode_primitiveMacroIfThen);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astIfNodeType,
        sysbvm_astIfNode_primitiveAnalyze,
        sysbvm_astIfNode_primitiveEvaluate,
        sysbvm_astIfNode_primitiveAnalyzeAndEvaluate
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "do:while:continueWith:", 4, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astDoWhileContinueWithNode_primitiveMacro);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "do:while:", 3, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astDoWhileContinueWithNode_primitiveDoWhileMacro);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astDoWhileContinueWithNodeType,
        sysbvm_astDoWhileContinueWithNode_primitiveAnalyze,
        sysbvm_astDoWhileContinueWithNode_primitiveEvaluate,
        sysbvm_astDoWhileContinueWithNode_primitiveAnalyzeAndEvaluate
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveMethod(context, "useNamedSlotsOf:", context->roots.anyValueType, "__useNamedSlots__", 2, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astUseNamedSlotsOfNode_primitiveMacro);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astUseNamedSlotsOfNodeType,
        sysbvm_astUseNamedSlotsOfNode_primitiveAnalyze,
        sysbvm_astUseNamedSlotsOfNode_primitiveEvaluate,
        sysbvm_astUseNamedSlotsOfNode_primitiveAnalyzeAndEvaluate
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "while:do:continueWith:", 4, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astWhileContinueWithNode_primitiveMacro);
    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "while:do:", 3, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astWhileContinueWithNode_primitiveWhileDoMacro);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astWhileContinueWithNodeType,
        sysbvm_astWhileContinueWithNode_primitiveAnalyze,
        sysbvm_astWhileContinueWithNode_primitiveEvaluate,
        sysbvm_astWhileContinueWithNode_primitiveAnalyzeAndEvaluate
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "break", 1, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astBreakNode_primitiveMacro);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astBreakNodeType,
        sysbvm_astBreakNode_primitiveAnalyze,
        sysbvm_astBreakNode_primitiveEvaluate,
        sysbvm_astBreakNode_primitiveEvaluate
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "continue", 1, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astContinueNode_primitiveMacro);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astContinueNodeType,
        sysbvm_astContinueNode_primitiveAnalyze,
        sysbvm_astContinueNode_primitiveEvaluate,
        sysbvm_astContinueNode_primitiveEvaluate
    );

    sysbvm_context_setIntrinsicSymbolBindingValueWithPrimitiveFunction(context, "return:", 2, SYSBVM_FUNCTION_FLAGS_MACRO, NULL, sysbvm_astReturnNode_primitiveMacro);
    sysbvm_astInterpreter_setupNodeInterpretationFunctions(context, context->roots.astReturnNodeType,
        sysbvm_astReturnNode_primitiveAnalyze,
        sysbvm_astReturnNode_primitiveEvaluate,
        sysbvm_astReturnNode_primitiveAnalyzeAndEvaluate
    );
}
