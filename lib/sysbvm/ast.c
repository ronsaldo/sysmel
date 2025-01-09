#include "sysbvm/ast.h"
#include "sysbvm/array.h"
#include "sysbvm/string.h"
#include "sysbvm/function.h"
#include "internal/context.h"

SYSBVM_API bool sysbvm_astNode_isArgumentNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astArgumentNodeType);
}

SYSBVM_API bool sysbvm_astNode_isBinaryExpressionSequenceNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astBinaryExpressionSequenceNodeType);
}

SYSBVM_API bool sysbvm_astNode_isBreakNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astBreakNodeType);
}

SYSBVM_API bool sysbvm_astNode_isCaseNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astCaseNodeType);
}

SYSBVM_API bool sysbvm_astNode_isCoerceValueNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astCoerceValueNodeType);
}

SYSBVM_API bool sysbvm_astNode_isContinueNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astContinueNodeType);
}

SYSBVM_API bool sysbvm_astNode_isDoWhileContinueWithNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astDoWhileContinueWithNodeType);
}

SYSBVM_API bool sysbvm_astNode_isDownCastNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astDownCastNodeType);
}

SYSBVM_API bool sysbvm_astNode_isErrorNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astErrorNodeType);
}

SYSBVM_API bool sysbvm_astNode_isFunctionApplicationNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astFunctionApplicationNodeType);
}

SYSBVM_API bool sysbvm_astNode_isIdentifierReferenceNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astIdentifierReferenceNodeType);
}

SYSBVM_API bool sysbvm_astNode_isIfNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astIfNodeType);
}

SYSBVM_API bool sysbvm_astNode_isLambdaNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astLambdaNodeType);
}

SYSBVM_API bool sysbvm_astNode_isLexicalBlockNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astLexicalBlockNodeType);
}

SYSBVM_API bool sysbvm_astNode_isLiteralNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astLiteralNodeType);
}

SYSBVM_API bool sysbvm_astNode_isVariableDefinitionNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astVariableDefinitionNodeType);
}

SYSBVM_API bool sysbvm_astNode_isMakeAssociationNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astMakeAssociationNodeType);
}

SYSBVM_API bool sysbvm_astNode_isMakeByteArrayNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astMakeByteArrayNodeType);
}

SYSBVM_API bool sysbvm_astNode_isMakeDictionaryNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astMakeDictionaryNodeType);
}

SYSBVM_API bool sysbvm_astNode_isMakeArrayNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astMakeArrayNodeType);
}

SYSBVM_API bool sysbvm_astNode_isMessageSendNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astMessageSendNodeType);
}

SYSBVM_API bool sysbvm_astNode_isMessageChainNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astMessageChainNodeType);
}

SYSBVM_API bool sysbvm_astNode_isMessageChainMessageNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astMessageChainMessageNodeType);
}

SYSBVM_API bool sysbvm_astNode_isPragmaNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astPragmaNodeType);
}

SYSBVM_API bool sysbvm_astNode_isReturnNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astReturnNodeType);
}

SYSBVM_API bool sysbvm_astNode_isSequenceNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astSequenceNodeType);
}

SYSBVM_API bool sysbvm_astNode_isSwitchNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astSwitchNodeType);
}

SYSBVM_API bool sysbvm_astNode_isTupleSlotNamedAtNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astTupleSlotNamedAtNodeType);
}

SYSBVM_API bool sysbvm_astNode_isTupleSlotNamedAtPutNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astTupleSlotNamedAtPutNodeType);
}

SYSBVM_API bool sysbvm_astNode_isTupleSlotNamedReferenceAtNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astTupleSlotNamedReferenceAtNodeType);
}

SYSBVM_API bool sysbvm_astNode_isTupleWithLookupStartingFromNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astTupleWithLookupStartingFromNodeType);
}

SYSBVM_API bool sysbvm_astNode_isQuoteNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astQuoteNodeType);
}

SYSBVM_API bool sysbvm_astNode_isQuasiQuoteNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astQuasiQuoteNodeType);
}

SYSBVM_API bool sysbvm_astNode_isQuasiUnquoteNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astQuasiUnquoteNodeType);
}

SYSBVM_API bool sysbvm_astNode_isSpliceNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astSpliceNodeType);
}

SYSBVM_API bool sysbvm_astNode_isMacroExpression(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    if(sysbvm_astNode_isLiteralNode(context, tuple))
        return sysbvm_function_isMacro(context, sysbvm_astLiteralNode_getValue(tuple));

    return false;
}

SYSBVM_API bool sysbvm_astNode_isUnexpandedApplicationNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astUnexpandedApplicationNodeType);
}

SYSBVM_API bool sysbvm_astNode_isUnexpandedSExpressionNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astUnexpandedSExpressionNodeType);
}

SYSBVM_API bool sysbvm_astNode_isUseNamedSlotsOfNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astUseNamedSlotsOfNodeType);
}

SYSBVM_API bool sysbvm_astNode_isWhileContinueWithNode(sysbvm_context_t *context, sysbvm_tuple_t tuple)
{
    return sysbvm_tuple_isKindOf(context, tuple, context->roots.astWhileContinueWithNodeType);
}

SYSBVM_API sysbvm_tuple_t sysbvm_astArgumentNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t isForAll, sysbvm_tuple_t name, sysbvm_tuple_t type)
{
    sysbvm_astArgumentNode_t *result = (sysbvm_astArgumentNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astArgumentNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astArgumentNode_t));
    result->super.sourcePosition = sourcePosition;
    result->isForAll = isForAll;
    result->name = name;
    result->type = type;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astBinaryExpressionSequenceNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t elements)
{
    sysbvm_astBinaryExpressionSequenceNode_t *result = (sysbvm_astBinaryExpressionSequenceNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astBinaryExpressionSequenceNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astBinaryExpressionSequenceNode_t));
    result->super.sourcePosition = sourcePosition;
    result->elements = elements;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astBreakNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition)
{
    sysbvm_astBreakNode_t *result = (sysbvm_astBreakNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astBreakNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astBreakNode_t));
    result->super.sourcePosition = sourcePosition;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astCoerceValueNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t typeExpression, sysbvm_tuple_t valueExpression)
{
    sysbvm_astCoerceValueNode_t *result = (sysbvm_astCoerceValueNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astCoerceValueNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astCoerceValueNode_t));
    result->super.sourcePosition = sourcePosition;
    result->typeExpression = typeExpression;
    result->valueExpression = valueExpression;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astContinueNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition)
{
    sysbvm_astContinueNode_t *result = (sysbvm_astContinueNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astContinueNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astContinueNode_t));
    result->super.sourcePosition = sourcePosition;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astDoWhileContinueWithNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t bodyExpression, sysbvm_tuple_t conditionExpression, sysbvm_tuple_t continueExpression)
{
    sysbvm_astDoWhileContinueWithNode_t *result = (sysbvm_astDoWhileContinueWithNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astDoWhileContinueWithNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astDoWhileContinueWithNode_t));
    result->super.sourcePosition = sourcePosition;
    result->bodyExpression = bodyExpression;
    result->conditionExpression = conditionExpression;
    result->continueExpression = continueExpression;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astDoWhileContinueWithNode_getConditionExpression(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astDoWhileContinueWithNode_t*)node)->conditionExpression;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astDoWhileContinueWithNode_getBodyExpression(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astDoWhileContinueWithNode_t*)node)->bodyExpression;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astDoWhileContinueWithNode_getContinueExpression(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astDoWhileContinueWithNode_t*)node)->continueExpression;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astDownCastNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t typeExpression, sysbvm_tuple_t valueExpression)
{
    sysbvm_astDownCastNode_t *result = (sysbvm_astDownCastNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astDownCastNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astDownCastNode_t));
    result->super.sourcePosition = sourcePosition;
    result->typeExpression = typeExpression;
    result->valueExpression = valueExpression;
    result->isUnchecked = SYSBVM_FALSE_TUPLE;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astDownCastNode_addOntoNodeWithTargetType(sysbvm_context_t *context, sysbvm_tuple_t valueExpression, sysbvm_tuple_t targetType)
{
    sysbvm_tuple_t sourcePosition = sysbvm_astNode_getSourcePosition(valueExpression);
    sysbvm_tuple_t targetTypeExpression = sysbvm_astLiteralNode_create(context, sourcePosition, targetType);
    return sysbvm_astDownCastNode_create(context, sourcePosition, targetTypeExpression, valueExpression);
}

SYSBVM_API sysbvm_tuple_t sysbvm_astErrorNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t errorMessage)
{
    sysbvm_astErrorNode_t *result = (sysbvm_astErrorNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astErrorNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astErrorNode_t));
    result->super.sourcePosition = sourcePosition;
    result->errorMessage = errorMessage;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astErrorNode_createWithCString(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, const char *errorMessage)
{
    return sysbvm_astErrorNode_create(context, sourcePosition, sysbvm_string_createWithCString(context, errorMessage));
}

SYSBVM_API sysbvm_tuple_t sysbvm_astFunctionApplicationNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t functionExpression, sysbvm_tuple_t arguments)
{
    sysbvm_astFunctionApplicationNode_t *result = (sysbvm_astFunctionApplicationNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astFunctionApplicationNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astFunctionApplicationNode_t));
    result->super.sourcePosition = sourcePosition;
    result->functionExpression = functionExpression;
    result->arguments = arguments;
    result->applicationFlags = sysbvm_tuple_bitflags_encode(0);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astIdentifierReferenceNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t value)
{
    sysbvm_astIdentifierReferenceNode_t *result = (sysbvm_astIdentifierReferenceNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astIdentifierReferenceNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astIdentifierReferenceNode_t));
    result->super.sourcePosition = sourcePosition;
    result->value = value;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astIdentifierReferenceNode_getValue(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astIdentifierReferenceNode_t*)node)->value;
}

SYSBVM_API bool sysbvm_astIdentifierReferenceNode_isEllipsis(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return false;
    return sysbvm_string_equalsCString(((sysbvm_astIdentifierReferenceNode_t*)node)->value, "...");
}

SYSBVM_API bool sysbvm_astIdentifierReferenceNode_isArrow(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return false;
    return sysbvm_string_equalsCString(((sysbvm_astIdentifierReferenceNode_t*)node)->value, "=>");
}

SYSBVM_API sysbvm_tuple_t sysbvm_astIfNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t conditionExpression, sysbvm_tuple_t trueExpression, sysbvm_tuple_t falseExpression)
{
    sysbvm_astIfNode_t *result = (sysbvm_astIfNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astIfNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astIfNode_t));
    result->super.sourcePosition = sourcePosition;
    result->conditionExpression = conditionExpression;
    result->trueExpression = trueExpression;
    result->falseExpression = falseExpression;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astIfNode_getConditionExpression(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astIfNode_t*)node)->conditionExpression;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astIfNode_getTrueExpression(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astIfNode_t*)node)->trueExpression;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astIfNode_getFalseExpression(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astIfNode_t*)node)->falseExpression;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astLambdaNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t flags, sysbvm_tuple_t arguments, sysbvm_tuple_t resultType, sysbvm_tuple_t body)
{
    sysbvm_astLambdaNode_t *result = (sysbvm_astLambdaNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astLambdaNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astLambdaNode_t));
    result->super.sourcePosition = sourcePosition;
    result->flags = flags;
    result->arguments = arguments;
    result->resultType = resultType;
    result->body = body;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astLambdaNode_getFlags(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astLambdaNode_t*)node)->flags;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astLambdaNode_getArguments(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astLambdaNode_t*)node)->arguments;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astLambdaNode_getBody(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astLambdaNode_t*)node)->body;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astLexicalBlockNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t body)
{
    sysbvm_astLexicalBlockNode_t *result = (sysbvm_astLexicalBlockNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astLexicalBlockNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astLexicalBlockNode_t));
    result->super.sourcePosition = sourcePosition;
    result->body = body;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astLiteralNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t value)
{
    sysbvm_astLiteralNode_t *result = (sysbvm_astLiteralNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astLiteralNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astLiteralNode_t));
    result->super.sourcePosition = sourcePosition;
    result->super.analyzedType = sysbvm_tuple_getType(context, value);
    result->value = value;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astLiteralNode_getValue(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return 0;
    return ((sysbvm_astLiteralNode_t*)node)->value;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astVariableDefinitionNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t nameExpression, sysbvm_tuple_t typeExpression, sysbvm_tuple_t valueExpression, bool isMutable)
{
    sysbvm_astVariableDefinitionNode_t *result = (sysbvm_astVariableDefinitionNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astVariableDefinitionNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astVariableDefinitionNode_t));
    result->super.sourcePosition = sourcePosition;
    result->nameExpression = nameExpression;
    result->typeExpression = typeExpression;
    result->valueExpression = valueExpression;
    result->isMacroSymbol = SYSBVM_FALSE_TUPLE;
    result->isMutable = sysbvm_tuple_boolean_encode(isMutable);
    result->isPublic = SYSBVM_FALSE_TUPLE;
    result->isExternal = SYSBVM_FALSE_TUPLE;
    result->isThreadLocal = SYSBVM_FALSE_TUPLE;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astVariableDefinitionNode_createMacro(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t nameExpression, sysbvm_tuple_t typeExpression, sysbvm_tuple_t valueExpression)
{
    sysbvm_astVariableDefinitionNode_t *result = (sysbvm_astVariableDefinitionNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astVariableDefinitionNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astVariableDefinitionNode_t));
    result->super.sourcePosition = sourcePosition;
    result->nameExpression = nameExpression;
    result->typeExpression = typeExpression;
    result->valueExpression = valueExpression;
    result->isMacroSymbol = SYSBVM_TRUE_TUPLE;
    result->isMutable = SYSBVM_FALSE_TUPLE;
    result->isPublic = SYSBVM_FALSE_TUPLE;
    result->isExternal = SYSBVM_FALSE_TUPLE;
    result->isThreadLocal = SYSBVM_FALSE_TUPLE;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astVariableDefinitionNode_getNameExpression(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return 0;
    return ((sysbvm_astVariableDefinitionNode_t*)node)->nameExpression;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astVariableDefinitionNode_getTypeExpression(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return 0;
    return ((sysbvm_astVariableDefinitionNode_t*)node)->typeExpression;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astVariableDefinitionNode_getValueExpression(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return 0;
    return ((sysbvm_astVariableDefinitionNode_t*)node)->valueExpression;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astMakeAssociationNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t key, sysbvm_tuple_t value)
{
    sysbvm_astMakeAssociationNode_t *result = (sysbvm_astMakeAssociationNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astMakeAssociationNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astMakeAssociationNode_t));
    result->super.sourcePosition = sourcePosition;
    result->key = key;
    result->value = value;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astMakeByteArrayNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t elements)
{
    sysbvm_astMakeByteArrayNode_t *result = (sysbvm_astMakeByteArrayNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astMakeByteArrayNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astMakeByteArrayNode_t));
    result->super.sourcePosition = sourcePosition;
    result->elements = elements;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astMakeDictionaryNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t elements)
{
    sysbvm_astMakeDictionaryNode_t *result = (sysbvm_astMakeDictionaryNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astMakeDictionaryNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astMakeDictionaryNode_t));
    result->super.sourcePosition = sourcePosition;
    result->elements = elements;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astMakeArrayNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t elements)
{
    sysbvm_astMakeArrayNode_t *result = (sysbvm_astMakeArrayNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astMakeArrayNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astMakeArrayNode_t));
    result->super.sourcePosition = sourcePosition;
    result->elements = elements;
    return (sysbvm_tuple_t)result;

}

SYSBVM_API sysbvm_tuple_t sysbvm_astMessageSendNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t receiver, sysbvm_tuple_t selector, sysbvm_tuple_t arguments)
{
    sysbvm_astMessageSendNode_t *result = (sysbvm_astMessageSendNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astMessageSendNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astMessageSendNode_t));
    result->super.sourcePosition = sourcePosition;
    result->receiver = receiver;
    result->selector = selector;
    result->arguments = arguments;
    result->isDynamic = SYSBVM_FALSE_TUPLE;
    result->applicationFlags = sysbvm_tuple_bitflags_encode(0);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astMessageChainNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t receiver, sysbvm_tuple_t messages)
{
    sysbvm_astMessageChainNode_t *result = (sysbvm_astMessageChainNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astMessageChainNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astMessageChainNode_t));
    result->super.sourcePosition = sourcePosition;
    result->receiver = receiver;
    result->messages = messages;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astMessageChainMessageNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t selector, sysbvm_tuple_t arguments)
{
    sysbvm_astMessageChainMessageNode_t *result = (sysbvm_astMessageChainMessageNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astMessageChainMessageNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astMessageChainMessageNode_t));
    result->super.sourcePosition = sourcePosition;
    result->selector = selector;
    result->arguments = arguments;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astPragmaNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t selector, sysbvm_tuple_t arguments)
{
    sysbvm_astPragmaNode_t *result = (sysbvm_astPragmaNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astPragmaNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astPragmaNode_t));
    result->super.sourcePosition = sourcePosition;
    result->selector = selector;
    result->arguments = arguments;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astReturnNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t expression)
{
    sysbvm_astReturnNode_t *result = (sysbvm_astReturnNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astReturnNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astReturnNode_t));
    result->super.sourcePosition = sourcePosition;
    result->expression = expression;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astSequenceNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t pragmas, sysbvm_tuple_t expressions)
{
    sysbvm_astSequenceNode_t *result = (sysbvm_astSequenceNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astSequenceNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astSequenceNode_t));
    result->super.sourcePosition = sourcePosition;
    result->pragmas = pragmas;
    result->expressions = expressions;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API size_t sysbvm_astSequenceNode_getExpressionCount(sysbvm_tuple_t sequenceNode)
{
    if(!sysbvm_tuple_isNonNullPointer(sequenceNode)) return 0;
    return sysbvm_array_getSize( ((sysbvm_astSequenceNode_t*)sequenceNode)->expressions );
}

SYSBVM_API sysbvm_tuple_t sysbvm_astSequenceNode_getExpressionAt(sysbvm_tuple_t sequenceNode, size_t index)
{
    if(!sysbvm_tuple_isNonNullPointer(sequenceNode)) return SYSBVM_NULL_TUPLE;
    return sysbvm_array_at( ((sysbvm_astSequenceNode_t*)sequenceNode)->expressions, index);
}

SYSBVM_API sysbvm_tuple_t sysbvm_astTupleSlotNamedAtNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t tupleExpression, sysbvm_tuple_t nameExpression)
{
    sysbvm_astTupleSlotNamedAtNode_t *result = (sysbvm_astTupleSlotNamedAtNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astTupleSlotNamedAtNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astTupleSlotNamedAtNode_t));
    result->super.sourcePosition = sourcePosition;
    result->tupleExpression = tupleExpression;
    result->nameExpression = nameExpression;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astTupleSlotNamedAtPutNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t tupleExpression, sysbvm_tuple_t nameExpression, sysbvm_tuple_t valueExpression)
{
    sysbvm_astTupleSlotNamedAtPutNode_t *result = (sysbvm_astTupleSlotNamedAtPutNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astTupleSlotNamedAtPutNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astTupleSlotNamedAtPutNode_t));
    result->super.sourcePosition = sourcePosition;
    result->tupleExpression = tupleExpression;
    result->nameExpression = nameExpression;
    result->valueExpression = valueExpression;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astTupleSlotNamedReferenceAtNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t tupleExpression, sysbvm_tuple_t nameExpression)
{
    sysbvm_astTupleSlotNamedReferenceAtNode_t *result = (sysbvm_astTupleSlotNamedReferenceAtNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astTupleSlotNamedReferenceAtNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astTupleSlotNamedReferenceAtNode_t));
    result->super.sourcePosition = sourcePosition;
    result->tupleExpression = tupleExpression;
    result->nameExpression = nameExpression;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astTupleWithLookupStartingFromNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t tupleExpression, sysbvm_tuple_t typeExpression)
{
    sysbvm_astTupleWithLookupStartingFromNode_t *result = (sysbvm_astTupleWithLookupStartingFromNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astTupleWithLookupStartingFromNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astTupleWithLookupStartingFromNode_t));
    result->super.sourcePosition = sourcePosition;
    result->tupleExpression = tupleExpression;
    result->typeExpression = typeExpression;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astUnexpandedApplicationNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t functionOrMacroExpression, sysbvm_tuple_t arguments)
{
    sysbvm_astUnexpandedApplicationNode_t *result = (sysbvm_astUnexpandedApplicationNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astUnexpandedApplicationNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astUnexpandedApplicationNode_t));
    result->super.sourcePosition = sourcePosition;
    result->functionOrMacroExpression = functionOrMacroExpression;
    result->arguments = arguments;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astUnexpandedApplicationNode_getFunctionOrMacroExpression(sysbvm_tuple_t unexpandedApplication)
{
    if(!sysbvm_tuple_isNonNullPointer(unexpandedApplication)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astUnexpandedApplicationNode_t*)unexpandedApplication)->functionOrMacroExpression;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astUnexpandedApplicationNode_getArguments(sysbvm_tuple_t unexpandedApplication)
{
    if(!sysbvm_tuple_isNonNullPointer(unexpandedApplication)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astUnexpandedApplicationNode_t*)unexpandedApplication)->arguments;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astUnexpandedSExpressionNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t elements)
{
    sysbvm_astUnexpandedSExpressionNode_t *result = (sysbvm_astUnexpandedSExpressionNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astUnexpandedSExpressionNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astUnexpandedSExpressionNode_t));
    result->super.sourcePosition = sourcePosition;
    result->elements = elements;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astUnexpandedSExpressionNode_getElements(sysbvm_tuple_t unexpandedSExpressionNode)
{
    if(!sysbvm_tuple_isNonNullPointer(unexpandedSExpressionNode)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astUnexpandedSExpressionNode_t*)unexpandedSExpressionNode)->elements;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astQuoteNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t node)
{
    sysbvm_astQuoteNode_t *result = (sysbvm_astQuoteNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astQuoteNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astQuoteNode_t));
    result->super.sourcePosition = sourcePosition;
    result->node = node;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astQuasiQuoteNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t node)
{
    sysbvm_astQuasiQuoteNode_t *result = (sysbvm_astQuasiQuoteNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astQuasiQuoteNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astQuasiQuoteNode_t));
    result->super.sourcePosition = sourcePosition;
    result->node = node;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astQuasiUnquoteNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t expression)
{
    sysbvm_astQuasiUnquoteNode_t *result = (sysbvm_astQuasiUnquoteNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astQuasiUnquoteNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astQuasiUnquoteNode_t));
    result->super.sourcePosition = sourcePosition;
    result->expression = expression;
    result->astTemplateParameterIndex = sysbvm_tuple_size_encode(context, 0);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astSpliceNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t expression)
{
    sysbvm_astSpliceNode_t *result = (sysbvm_astSpliceNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astSpliceNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astSpliceNode_t));
    result->super.sourcePosition = sourcePosition;
    result->expression = expression;
    result->astTemplateParameterIndex = sysbvm_tuple_size_encode(context, 0);
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astUseNamedSlotsOfNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t tupleExpression)
{
    sysbvm_astUseNamedSlotsOfNode_t *result = (sysbvm_astUseNamedSlotsOfNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astUseNamedSlotsOfNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astUseNamedSlotsOfNode_t));
    result->super.sourcePosition = sourcePosition;
    result->tupleExpression = tupleExpression;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astWhileContinueWithNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t conditionExpression, sysbvm_tuple_t bodyExpression, sysbvm_tuple_t continueExpression)
{
    sysbvm_astWhileContinueWithNode_t *result = (sysbvm_astWhileContinueWithNode_t*)sysbvm_context_allocatePointerTuple(context, context->roots.astWhileContinueWithNodeType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_astWhileContinueWithNode_t));
    result->super.sourcePosition = sourcePosition;
    result->conditionExpression = conditionExpression;
    result->bodyExpression = bodyExpression;
    result->continueExpression = continueExpression;
    return (sysbvm_tuple_t)result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astWhileContinueWithNode_getConditionExpression(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astWhileContinueWithNode_t*)node)->conditionExpression;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astWhileContinueWithNode_getBodyExpression(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astWhileContinueWithNode_t*)node)->bodyExpression;
}

SYSBVM_API sysbvm_tuple_t sysbvm_astWhileContinueWithNode_getContinueExpression(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astWhileContinueWithNode_t*)node)->continueExpression;
}
