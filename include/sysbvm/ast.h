#ifndef SYSBVM_AST_H
#define SYSBVM_AST_H

#pragma once

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_astNode_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t sourcePosition;
    sysbvm_tuple_t analyzerToken;
    sysbvm_tuple_t analyzedType;
} sysbvm_astNode_t;

typedef struct sysbvm_astArgumentNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t isForAll;
    sysbvm_tuple_t name;
    sysbvm_tuple_t type;
    sysbvm_tuple_t binding;
} sysbvm_astArgumentNode_t;

typedef struct sysbvm_astBinaryExpressionSequenceNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t elements;
    sysbvm_tuple_t operands;
    sysbvm_tuple_t operators;
} sysbvm_astBinaryExpressionSequenceNode_t;

typedef struct sysbvm_astBreakNode_s
{
    sysbvm_astNode_t super;
} sysbvm_astBreakNode_t;

typedef struct sysbvm_astCaseNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t keyExpression;
    sysbvm_tuple_t valueExpression;
} sysbvm_astCaseNode_t;

typedef struct sysbvm_astCoerceValueNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t typeExpression;
    sysbvm_tuple_t valueExpression;
} sysbvm_astCoerceValueNode_t;

typedef struct sysbvm_astContinueNode_s
{
    sysbvm_astNode_t super;
} sysbvm_astContinueNode_t;

typedef struct sysbvm_astDoWhileContinueWithNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t bodyExpression;
    sysbvm_tuple_t conditionExpression;
    sysbvm_tuple_t continueExpression;
} sysbvm_astDoWhileContinueWithNode_t;

typedef struct sysbvm_astDownCastNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t typeExpression;
    sysbvm_tuple_t valueExpression;
    sysbvm_tuple_t isUnchecked;
} sysbvm_astDownCastNode_t;

typedef struct sysbvm_astErrorNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t errorMessage;
} sysbvm_astErrorNode_t;

typedef struct sysbvm_astFunctionApplicationNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t functionExpression;
    sysbvm_tuple_t arguments;
    sysbvm_tuple_t applicationFlags;
} sysbvm_astFunctionApplicationNode_t;

typedef struct sysbvm_astLambdaNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t name;
    sysbvm_tuple_t flags;
    sysbvm_tuple_t callingConventionName;
    sysbvm_tuple_t arguments;
    sysbvm_tuple_t resultType;
    sysbvm_tuple_t body;
    sysbvm_tuple_t hasLazyAnalysis;
    sysbvm_tuple_t functionDefinition;
    sysbvm_tuple_t binding;
} sysbvm_astLambdaNode_t;

typedef struct sysbvm_astLexicalBlockNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t body;
    sysbvm_tuple_t bodyEnvironment;
} sysbvm_astLexicalBlockNode_t;

typedef struct sysbvm_astLiteralNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t value;
} sysbvm_astLiteralNode_t;

typedef struct sysbvm_astVariableDefinitionNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t nameExpression;
    sysbvm_tuple_t typeExpression;
    sysbvm_tuple_t valueExpression;
    sysbvm_tuple_t binding;
    sysbvm_tuple_t isMacroSymbol;
    sysbvm_tuple_t isMutable;
    sysbvm_tuple_t isPublic;
    sysbvm_tuple_t isExternal;
    sysbvm_tuple_t isThreadLocal;
    sysbvm_tuple_t analyzedValueType;
} sysbvm_astVariableDefinitionNode_t;

typedef struct sysbvm_astIdentifierReferenceNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t value;
    sysbvm_tuple_t binding;
} sysbvm_astIdentifierReferenceNode_t;

typedef struct sysbvm_astIfNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t conditionExpression;
    sysbvm_tuple_t trueExpression;
    sysbvm_tuple_t falseExpression;
} sysbvm_astIfNode_t;

typedef struct sysbvm_astMakeAssociationNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t key;
    sysbvm_tuple_t value;
} sysbvm_astMakeAssociationNode_t;

typedef struct sysbvm_astMakeByteArrayNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t elements;
} sysbvm_astMakeByteArrayNode_t;

typedef struct sysbvm_astMakeDictionaryNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t elements;
} sysbvm_astMakeDictionaryNode_t;

typedef struct sysbvm_astMakeArrayNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t elements;
} sysbvm_astMakeArrayNode_t;

typedef struct sysbvm_astMessageSendNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t receiver;
    sysbvm_tuple_t receiverLookupType;
    sysbvm_tuple_t selector;
    sysbvm_tuple_t arguments;
    sysbvm_tuple_t isDynamic;
    sysbvm_tuple_t boundMethod;
    sysbvm_tuple_t boundMethodOwner;
    sysbvm_tuple_t applicationFlags;
} sysbvm_astMessageSendNode_t;

typedef struct sysbvm_astMessageChainNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t receiver;
    sysbvm_tuple_t receiverLookupType;
    sysbvm_tuple_t messages;
} sysbvm_astMessageChainNode_t;

typedef struct sysbvm_astMessageChainMessageNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t selector;
    sysbvm_tuple_t arguments;
} sysbvm_astMessageChainMessageNode_t;

typedef struct sysbvm_astPragmaNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t selector;
    sysbvm_tuple_t arguments;
} sysbvm_astPragmaNode_t;

typedef struct sysbvm_astReturnNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t expression;
} sysbvm_astReturnNode_t;

typedef struct sysbvm_astSequenceNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t pragmas;
    sysbvm_tuple_t expressions;
} sysbvm_astSequenceNode_t;

typedef struct sysbvm_astSwitchNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t expression;
    sysbvm_tuple_t caseExpressions;
    sysbvm_tuple_t defaultExpression;
} sysbvm_astSwitchNode_t;

typedef struct sysbvm_astUnexpandedApplicationNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t functionOrMacroExpression;
    sysbvm_tuple_t arguments;
} sysbvm_astUnexpandedApplicationNode_t;

typedef struct sysbvm_astUnexpandedSExpressionNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t elements;
} sysbvm_astUnexpandedSExpressionNode_t;

typedef struct sysbvm_astQuoteNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t node;
} sysbvm_astQuoteNode_t;

typedef struct sysbvm_astQuasiQuoteNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t node;
} sysbvm_astQuasiQuoteNode_t;

typedef struct sysbvm_astQuasiUnquoteNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t expression;
    sysbvm_tuple_t astTemplateParameterIndex;
} sysbvm_astQuasiUnquoteNode_t;

typedef struct sysbvm_astSpliceNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t expression;
    sysbvm_tuple_t astTemplateParameterIndex;
} sysbvm_astSpliceNode_t;

typedef struct sysbvm_astTupleSlotNamedAtNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t tupleExpression;
    sysbvm_tuple_t nameExpression;
    sysbvm_tuple_t boundSlot;
} sysbvm_astTupleSlotNamedAtNode_t;

typedef struct sysbvm_astTupleSlotNamedAtPutNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t tupleExpression;
    sysbvm_tuple_t nameExpression;
    sysbvm_tuple_t valueExpression;
    sysbvm_tuple_t boundSlot;
} sysbvm_astTupleSlotNamedAtPutNode_t;

typedef struct sysbvm_astTupleSlotNamedReferenceAtNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t tupleExpression;
    sysbvm_tuple_t nameExpression;
    sysbvm_tuple_t boundSlot;
} sysbvm_astTupleSlotNamedReferenceAtNode_t;

typedef struct sysbvm_astTupleWithLookupStartingFromNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t tupleExpression;
    sysbvm_tuple_t typeExpression;
} sysbvm_astTupleWithLookupStartingFromNode_t;

typedef struct sysbvm_astUseNamedSlotsOf_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t tupleExpression;
    sysbvm_tuple_t binding;
} sysbvm_astUseNamedSlotsOfNode_t;

typedef struct sysbvm_astWhileContinueWithNode_s
{
    sysbvm_astNode_t super;
    sysbvm_tuple_t conditionExpression;
    sysbvm_tuple_t bodyExpression;
    sysbvm_tuple_t continueExpression;
} sysbvm_astWhileContinueWithNode_t;

/**
 * Is this an argument node?
 */ 
SYSBVM_API bool sysbvm_astNode_isArgumentNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a binary expression sequence node?
 */ 
SYSBVM_API bool sysbvm_astNode_isBinaryExpressionSequenceNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a break node?
 */ 
SYSBVM_API bool sysbvm_astNode_isBreakNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a case node?
 */ 
SYSBVM_API bool sysbvm_astNode_isCaseNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a continue node?
 */ 
SYSBVM_API bool sysbvm_astNode_isCoerceValueNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a continue node?
 */ 
SYSBVM_API bool sysbvm_astNode_isContinueNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a do while continue with node?
 */ 
SYSBVM_API bool sysbvm_astNode_isDoWhileContinueWithNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a downcast node?
 */ 
SYSBVM_API bool sysbvm_astNode_isDownCastNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this an error node?
 */ 
SYSBVM_API bool sysbvm_astNode_isErrorNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a function application node?
 */ 
SYSBVM_API bool sysbvm_astNode_isFunctionApplicationNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this an identifier reference node?
 */ 
SYSBVM_API bool sysbvm_astNode_isIdentifierReferenceNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this an if node?
 */ 
SYSBVM_API bool sysbvm_astNode_isIfNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a lambda node?
 */ 
SYSBVM_API bool sysbvm_astNode_isLambdaNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a lexical block node?
 */ 
SYSBVM_API bool sysbvm_astNode_isLexicalBlockNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a literal node?
 */ 
SYSBVM_API bool sysbvm_astNode_isLiteralNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a local definition node?
 */ 
SYSBVM_API bool sysbvm_astNode_isVariableDefinitionNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a make association node?
 */ 
SYSBVM_API bool sysbvm_astNode_isMakeAssociationNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a make byte array node?
 */ 
SYSBVM_API bool sysbvm_astNode_isMakeByteArrayNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a make dictionary node?
 */ 
SYSBVM_API bool sysbvm_astNode_isMakeDictionaryNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a make tuple node?
 */ 
SYSBVM_API bool sysbvm_astNode_isMakeArrayNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a message send node?
 */ 
SYSBVM_API bool sysbvm_astNode_isMessageSendNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a message chain node?
 */ 
SYSBVM_API bool sysbvm_astNode_isMessageChainNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a message chain message node?
 */ 
SYSBVM_API bool sysbvm_astNode_isMessageChainMessageNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a pragma node?
 */ 
SYSBVM_API bool sysbvm_astNode_isPragmaNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a return node?
 */ 
SYSBVM_API bool sysbvm_astNode_isReturnNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a sequence node?
 */ 
SYSBVM_API bool sysbvm_astNode_isSequenceNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a switch node?
 */ 
SYSBVM_API bool sysbvm_astNode_isSwitchNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a tuple slot named at node?
 */ 
SYSBVM_API bool sysbvm_astNode_isTupleSlotNamedAtNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a tuple slot named at put node?
 */ 
SYSBVM_API bool sysbvm_astNode_isTupleSlotNamedAtPutNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a tuple slot named reference at node?
 */ 
SYSBVM_API bool sysbvm_astNode_isTupleSlotNamedReferenceAtNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a tuple with lookup starting from node?
 */ 
SYSBVM_API bool sysbvm_astNode_isTupleWithLookupStartingFromNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this an unexpanded application node?
 */ 
SYSBVM_API bool sysbvm_astNode_isUnexpandedApplicationNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this an unexpanded s-expression node?
 */ 
SYSBVM_API bool sysbvm_astNode_isUnexpandedSExpressionNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a quote node?
 */ 
SYSBVM_API bool sysbvm_astNode_isQuoteNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a quasi quote node?
 */ 
SYSBVM_API bool sysbvm_astNode_isQuasiQuoteNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a quasi unquote node?
 */ 
SYSBVM_API bool sysbvm_astNode_isQuasiUnquoteNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a splice node?
 */ 
SYSBVM_API bool sysbvm_astNode_isSpliceNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this a macro evaluable expression?
 */ 
SYSBVM_API bool sysbvm_astNode_isMacroExpression(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this an use named slots of node?
 */ 
SYSBVM_API bool sysbvm_astNode_isUseNamedSlotsOfNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Is this an while continue with node?
 */ 
SYSBVM_API bool sysbvm_astNode_isWhileContinueWithNode(sysbvm_context_t *context, sysbvm_tuple_t tuple);

/**
 * Gets the source position from the ast node.
 */ 
SYSBVM_INLINE sysbvm_tuple_t sysbvm_astNode_getSourcePosition(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astNode_t*)node)->sourcePosition;
}

/**
 * Gets the analyzed type from the ast node.
 */ 
SYSBVM_INLINE sysbvm_tuple_t sysbvm_astNode_getAnalyzedType(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astNode_t*)node)->analyzedType;
}

/**
 * Gets the analyzer token from the ast node.
 */ 
SYSBVM_INLINE sysbvm_tuple_t sysbvm_astNode_getAnalyzerToken(sysbvm_tuple_t node)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return SYSBVM_NULL_TUPLE;
    return ((sysbvm_astNode_t*)node)->analyzerToken;
}

/**
 * Sets the analyzer token in the ast node.
 */ 
SYSBVM_INLINE void sysbvm_astNode_setAnalyzerToken(sysbvm_tuple_t node, sysbvm_tuple_t token)
{
    if(!sysbvm_tuple_isNonNullPointer(node)) return;
    ((sysbvm_astNode_t*)node)->analyzerToken = token;
}


/**
 * Creates an argument node
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astArgumentNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t isForAll, sysbvm_tuple_t name, sysbvm_tuple_t type);

/**
 * Is this a for all argument? 
 */
SYSBVM_INLINE bool sysbvm_astArgumentNode_isForAll(sysbvm_tuple_t argumentNode)
{
    if(!sysbvm_tuple_isNonNullPointer(argumentNode)) return false;
    return sysbvm_tuple_boolean_decode(((sysbvm_astArgumentNode_t*)argumentNode)->isForAll);
}

/**
 * Creates a binary expression sequence node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astBinaryExpressionSequenceNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t operands, sysbvm_tuple_t operators);

/**
 * Creates a break node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astBreakNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition);

/**
 * Creates a coerce value node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astCoerceValueNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t typeExpression, sysbvm_tuple_t valueExpression);

/**
 * Creates a continue node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astContinueNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition);

/**
 * Creates a do while node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astDoWhileContinueWithNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t bodyExpression, sysbvm_tuple_t conditionExpression, sysbvm_tuple_t continueExpression);

/**
 * Gets the condition expression from a while node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astDoWhileContinueWithNode_getConditionExpression(sysbvm_tuple_t node);

/**
 * Gets the body expression from a while node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astDoWhileContinueWithNode_getBodyExpression(sysbvm_tuple_t node);

/**
 * Gets the continue expression from a while node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astDoWhileContinueWithNode_getContinueExpression(sysbvm_tuple_t node);

/**
 * Creates a down cast node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astDownCastNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t typeExpression, sysbvm_tuple_t valueExpression);

/**
 * Creates a down cast node for the specified node with the given target type.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astDownCastNode_addOntoNodeWithTargetType(sysbvm_context_t *context, sysbvm_tuple_t valueExpression, sysbvm_tuple_t targetType);

/**
 * Creates an error node
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astErrorNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t errorMessage);

/**
 * Creates an error node
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astErrorNode_createWithCString(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, const char *errorMessage);

/**
 * Creates a function application node
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astFunctionApplicationNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t functionExpression, sysbvm_tuple_t arguments);

/**
 * Creates an identifier reference node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astIdentifierReferenceNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t value);

/**
 * Gets the value from an identifier reference node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astIdentifierReferenceNode_getValue(sysbvm_tuple_t node);

/**
 * Is this an ellipsis identifier?
 */ 
SYSBVM_API bool sysbvm_astIdentifierReferenceNode_isEllipsis(sysbvm_tuple_t node);

/**
 * Is this an arrow identifier?
 */
SYSBVM_API bool sysbvm_astIdentifierReferenceNode_isArrow(sysbvm_tuple_t node);

/**
 * Creates an if node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astIfNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t conditionExpression, sysbvm_tuple_t trueExpression, sysbvm_tuple_t falseExpression);

/**
 * Gets the condition expression from an if node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astIfNode_getConditionExpression(sysbvm_tuple_t node);

/**
 * Gets the true expression from an if node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astIfNode_getTrueExpression(sysbvm_tuple_t node);

/**
 * Gets the false expression from an if node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astIfNode_getFalseExpression(sysbvm_tuple_t node);

/**
 * Creates a lambda node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astLambdaNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t flags, sysbvm_tuple_t arguments, sysbvm_tuple_t resultType, sysbvm_tuple_t body);

/**
 * Gets the flags from the lambda node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astLambdaNode_getFlags(sysbvm_tuple_t node);

/**
 * Gets the arguments from the lambda node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astLambdaNode_getArguments(sysbvm_tuple_t node);

/**
 * Gets the body from the lambda node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astLambdaNode_getBody(sysbvm_tuple_t node);

/**
 * Creates a lexical block node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astLexicalBlockNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t body);

/**
 * Creates a literal node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astLiteralNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t value);

/**
 * Gets the value from a literal node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astLiteralNode_getValue(sysbvm_tuple_t node);

/**
 * Creates a local definition node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astVariableDefinitionNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t nameExpression, sysbvm_tuple_t typeExpression, sysbvm_tuple_t valueExpression, bool isMutable);

/**
 * Creates a local definition node for a macro symbol.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astVariableDefinitionNode_createMacro(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t nameExpression, sysbvm_tuple_t typeExpression, sysbvm_tuple_t valueExpression);

/**
 * Gets the value from a local definition node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astVariableDefinitionNode_getNameExpression(sysbvm_tuple_t node);

/**
 * Gets the type from a local definition node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astVariableDefinitionNode_getTypeExpression(sysbvm_tuple_t node);

/**
 * Gets the value from a local definition node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astVariableDefinitionNode_getValueExpression(sysbvm_tuple_t node);

/**
 * Creates a make association node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astMakeAssociationNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t key, sysbvm_tuple_t value);

/**
 * Creates a make byte array node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astMakeByteArrayNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t elements);

/**
 * Creates a make dictionary node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astMakeDictionaryNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t elements);

/**
 * Creates a make tuple node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astMakeArrayNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t elements);

/**
 * Creates a message send node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astMessageSendNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t receiver, sysbvm_tuple_t selector, sysbvm_tuple_t arguments);

/**
 * Creates a message chain node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astMessageChainNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t receiver, sysbvm_tuple_t messages);

/**
 * Creates a message chain node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astMessageChainMessageNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t selector, sysbvm_tuple_t arguments);

/**
 * Creates a pragma node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astPragmaNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t selector, sysbvm_tuple_t arguments);

/**
 * Creates a return node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astReturnNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t expression);

/**
 * Creates a sequence node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astSequenceNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t pragmas, sysbvm_tuple_t expressions);

/**
 * Gets the number of expressions in the sequence node.
 */
SYSBVM_API size_t sysbvm_astSequenceNode_getExpressionCount(sysbvm_tuple_t sequenceNode);

/**
 * Gets an expression with the given index in the sequence node.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_astSequenceNode_getExpressionAt(sysbvm_tuple_t sequenceNode, size_t index);

/**
 * Creates a tuple slot named at node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astTupleSlotNamedAtNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t tupleExpression, sysbvm_tuple_t nameExpression);

/**
 * Creates a tuple slot named at node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astTupleSlotNamedAtPutNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t tupleExpression, sysbvm_tuple_t nameExpression, sysbvm_tuple_t valueExpression);

/**
 * Creates a tuple slot named reference at node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astTupleSlotNamedReferenceAtNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t tupleExpression, sysbvm_tuple_t nameExpression);

/**
 * Creates a tuple with lookup starting from node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astTupleWithLookupStartingFromNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t tupleExpression, sysbvm_tuple_t typeExpression);

/**
 * Creates an unexpanded application node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astUnexpandedApplicationNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t functionOrMacroExpression, sysbvm_tuple_t arguments);

/**
 * Gets the function or macro expression from the unexpanded application node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astUnexpandedApplicationNode_getFunctionOrMacroExpression(sysbvm_tuple_t unexpandedApplication);

/**
 * Gets the arguments from the unexpanded application node
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astUnexpandedApplicationNode_getArguments(sysbvm_tuple_t unexpandedApplication);

/**
 * Creates an unexpanded s-expression node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astUnexpandedSExpressionNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t elements);

/**
 * Gets the elements from the unexpanded s-expression node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astUnexpandedSExpressionNode_getElements(sysbvm_tuple_t unexpandedSExpressionNode);

/**
 * Creates a quote node
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astQuoteNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t node);

/**
 * Creates a quasi quote node
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astQuasiQuoteNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t node);

/**
 * Creates a quasi unquote node
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astQuasiUnquoteNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t expression);

/**
 * Creates a splice node
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astSpliceNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t expression);

/**
 * Creates an use named slots of node
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astUseNamedSlotsOfNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t tupleExpression);

/**
 * Creates a while node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astWhileContinueWithNode_create(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t conditionExpression, sysbvm_tuple_t bodyExpression, sysbvm_tuple_t continueExpression);

/**
 * Gets the condition expression from a while node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astWhileContinueWithNode_getConditionExpression(sysbvm_tuple_t node);

/**
 * Gets the body expression from a while node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astWhileContinueWithNode_getBodyExpression(sysbvm_tuple_t node);

/**
 * Gets the continue expression from a while node.
 */ 
SYSBVM_API sysbvm_tuple_t sysbvm_astWhileContinueWithNode_getContinueExpression(sysbvm_tuple_t node);

#endif //SYSBVM_AST_H
