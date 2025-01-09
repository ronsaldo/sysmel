#include "sysbvm/parser.h"
#include "sysbvm/ast.h"
#include "sysbvm/array.h"
#include "sysbvm/orderedCollection.h"
#include "sysbvm/arraySlice.h"
#include "sysbvm/function.h"
#include "sysbvm/gc.h"
#include "sysbvm/token.h"
#include "sysbvm/scanner.h"
#include "sysbvm/sourceCode.h"
#include "sysbvm/sourcePosition.h"
#include "sysbvm/string.h"
#include "sysbvm/stringStream.h"

typedef struct sysbvm_parser_state_s
{
    sysbvm_tuple_t sourceCode;
    sysbvm_tuple_t tokenSequence;
    size_t tokenPosition;
    size_t tokenSequenceSize;
} sysbvm_parser_state_t;

static sysbvm_tuple_t sysbvm_parser_parseLiteralArrayExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state);
static sysbvm_tuple_t sysbvm_parser_parseLiteralByteArrayExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state);

static sysbvm_tuple_t sysbvm_parser_parseUnaryExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state);
static sysbvm_tuple_t sysbvm_parser_parseBinaryExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state);
static sysbvm_tuple_t sysbvm_parser_parseCommaExpressionElement(sysbvm_context_t *context, sysbvm_parser_state_t *state);
static sysbvm_tuple_t sysbvm_parser_parseExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state);

static sysbvm_tuple_t sysbvm_parser_parseExpressionList(sysbvm_context_t *context, sysbvm_parser_state_t *state);
static sysbvm_tuple_t sysbvm_parser_parseSequence(sysbvm_context_t *context, sysbvm_parser_state_t *state);

static sysbvm_tuple_t sysbvm_parser_parseExpressionListUntil(sysbvm_context_t *context, sysbvm_parser_state_t *state, sysbvm_tokenKind_t delimiter);
static sysbvm_tuple_t sysbvm_parser_parseSequenceUntil(sysbvm_context_t *context, sysbvm_parser_state_t *state, sysbvm_tokenKind_t delimiter);

static sysbvm_tuple_t sysbvm_parser_parsePrimaryTerm(sysbvm_context_t *context, sysbvm_parser_state_t *state);

static sysbvm_tuple_t sysbvm_parser_lookAt(sysbvm_parser_state_t *state, size_t offset)
{
    size_t lookPosition = state->tokenPosition + offset;
    return lookPosition < state->tokenSequenceSize ? sysbvm_arraySlice_at(state->tokenSequence, lookPosition) : SYSBVM_NULL_TUPLE;
}

static int sysbvm_parser_lookKindAt(sysbvm_parser_state_t *state, size_t offset)
{
    sysbvm_tuple_t token = sysbvm_parser_lookAt(state, offset);
    return token == SYSBVM_NULL_TUPLE ? (-1) : (int)sysbvm_token_getKind(token);
}

static sysbvm_tuple_t sysbvm_parser_makeSourcePositionForTokenRange(sysbvm_context_t *context, sysbvm_tuple_t sourceCode, sysbvm_tuple_t tokenSequence, size_t startIndex, size_t endIndex)
{
    if(sysbvm_arraySlice_getSize(tokenSequence) == 0)
        return sysbvm_sourcePosition_createWithIndices(context, sourceCode, 0, 0);

    if(startIndex == endIndex)
        return sysbvm_token_getSourcePosition(sysbvm_arraySlice_at(tokenSequence, startIndex));

    sysbvm_tuple_t firstToken = sysbvm_arraySlice_at(tokenSequence, startIndex);
    sysbvm_tuple_t lastToken = sysbvm_arraySlice_at(tokenSequence, endIndex - 1);
    return sysbvm_sourcePosition_createWithUnion(context, sysbvm_token_getSourcePosition(firstToken), sysbvm_token_getSourcePosition(lastToken));
}

static sysbvm_tuple_t sysbvm_parser_makeSourcePositionForEndOfSourceCode(sysbvm_context_t *context, sysbvm_tuple_t sourceCode)
{
    size_t sourceCodeTextSize = sysbvm_tuple_getSizeInBytes(sysbvm_sourceCode_getText(sourceCode));
    return sysbvm_sourcePosition_createWithIndices(context, sourceCode, sourceCodeTextSize, sourceCodeTextSize);
}

static sysbvm_tuple_t sysbvm_parser_makeSourcePositionForParserState(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    sysbvm_tuple_t token = sysbvm_parser_lookAt(state, 0);
    if(token == SYSBVM_NULL_TUPLE)
        return sysbvm_parser_makeSourcePositionForEndOfSourceCode(context, state->sourceCode);
    else
        return sysbvm_token_getSourcePosition(token);
}

static sysbvm_tuple_t sysbvm_parser_makeUnaryMessageSend(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t receiver, sysbvm_tuple_t selector)
{
    sysbvm_tuple_t arguments = sysbvm_array_create(context, 0);
    return sysbvm_astMessageSendNode_create(context, sourcePosition, receiver, selector, arguments);
}

static sysbvm_tuple_t sysbvm_parser_makeBinaryMessageSend(sysbvm_context_t *context, sysbvm_tuple_t sourcePosition, sysbvm_tuple_t receiver, sysbvm_tuple_t selector, sysbvm_tuple_t argument)
{
    sysbvm_tuple_t arguments = sysbvm_array_create(context, 1);
    sysbvm_array_atPut(arguments, 0, argument);
    return sysbvm_astMessageSendNode_create(context, sourcePosition, receiver, selector, arguments);
}

static sysbvm_tuple_t sysbvm_parser_unexpectedTokenAt(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    sysbvm_tuple_t token = sysbvm_parser_lookAt(state, 0);
    if(token == SYSBVM_NULL_TUPLE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForEndOfSourceCode(context, state->sourceCode), "Unexpected end of source.");

    ++state->tokenPosition;
    return sysbvm_astErrorNode_createWithCString(context, sysbvm_token_getSourcePosition(token), "Unexpected token.");
}

static sysbvm_tuple_t sysbvm_parser_parseIdentifierReferenceReference(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    sysbvm_tuple_t token = sysbvm_parser_lookAt(state, 0);
    if(token == SYSBVM_NULL_TUPLE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected an identifier.");

    ++state->tokenPosition;
    return sysbvm_astIdentifierReferenceNode_create(context, sysbvm_token_getSourcePosition(token), sysbvm_token_getValue(token));
}

static sysbvm_tuple_t sysbvm_parser_parseLiteralTokenValue(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    sysbvm_tuple_t token = sysbvm_parser_lookAt(state, 0);
    if(token == SYSBVM_NULL_TUPLE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected an atom token.");

    ++state->tokenPosition;
    return sysbvm_astLiteralNode_create(context, sysbvm_token_getSourcePosition(token), sysbvm_token_getValue(token));
}

static sysbvm_tuple_t sysbvm_parser_parseQuote(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_QUOTE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t node = sysbvm_parser_parsePrimaryTerm(context, state);
    size_t endPosition = state->tokenPosition;
    return sysbvm_astQuoteNode_create(context, sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), node);
}

static sysbvm_tuple_t sysbvm_parser_parseQuasiQuote(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_QUASI_QUOTE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t node = sysbvm_parser_parsePrimaryTerm(context, state);
    size_t endPosition = state->tokenPosition;
    return sysbvm_astQuasiQuoteNode_create(context, sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), node);
}

static sysbvm_tuple_t sysbvm_parser_parseQuasiUnquote(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_QUASI_UNQUOTE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t node = sysbvm_parser_parsePrimaryTerm(context, state);
    size_t endPosition = state->tokenPosition;
    return sysbvm_astQuasiUnquoteNode_create(context, sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), node);
}

static sysbvm_tuple_t sysbvm_parser_parseSplice(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_SPLICE)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a quote token.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t node = sysbvm_parser_parsePrimaryTerm(context, state);
    size_t endPosition = state->tokenPosition;
    return sysbvm_astSpliceNode_create(context, sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), node);
}

static sysbvm_tuple_t sysbvm_parser_parseParenthesesExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LPARENT)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a left parenthesis.");
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    // Delimited keyword.
    if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_KEYWORD &&
        sysbvm_parser_lookKindAt(state, 1) == SYSBVM_TOKEN_KIND_RPARENT)
    {
        sysbvm_tuple_t result = sysbvm_parser_parseIdentifierReferenceReference(context, state);
        if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RPARENT)
            return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a right parenthesis.");
        
        ++state->tokenPosition;
        return result;
    }

    if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_RPARENT)
    {
        ++state->tokenPosition;
        size_t endPosition = state->tokenPosition;
        return sysbvm_astMakeArrayNode_create(context, sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), sysbvm_array_create(context, 0));
    }
    else
    {
        sysbvm_tuple_t expression = sysbvm_parser_parseExpression(context, state);

        if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RPARENT)
            return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a right parenthesis.");
        ++state->tokenPosition;
        return expression;
    }
}

static sysbvm_tuple_t sysbvm_parser_parseBlockArgument(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_COLON)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a colon to delimit a block argument.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t isForAll = SYSBVM_FALSE_TUPLE;
    if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_STAR)
    {
        isForAll = SYSBVM_TRUE_TUPLE;
        ++state->tokenPosition;
    }

    sysbvm_tuple_t typeExpression = SYSBVM_NULL_TUPLE;

    if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_LPARENT)
    {
        ++state->tokenPosition;
        typeExpression = sysbvm_parser_parseExpression(context, state);

        if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RPARENT)
            return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a right parentheses that specifies the type.");
        ++state->tokenPosition;
    }

    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_IDENTIFIER)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected an identifier with the argument name.");

    sysbvm_tuple_t nameExpression = sysbvm_parser_parseLiteralTokenValue(context, state);

    size_t endPosition = state->tokenPosition;
    sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);

    return sysbvm_astArgumentNode_create(context, sourcePosition, isForAll, nameExpression, typeExpression);
}

static sysbvm_tuple_t sysbvm_parser_parseBlockExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
   if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LCBRACKET)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a left curly bracket.");

    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t argumentList = SYSBVM_NULL_TUPLE;
    sysbvm_tuple_t resultTypeExpression = SYSBVM_NULL_TUPLE;
    bool hasArguments = false;
    bool hasEllipsis = false;
    bool hasResultType = false;
    bool hasBlockBar = false;
    while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_COLON)
    {
        sysbvm_tuple_t argument = sysbvm_parser_parseBlockArgument(context, state);

        if(!hasArguments)
        {
            argumentList = sysbvm_orderedCollection_create(context);
            hasArguments = true;
        }

        sysbvm_orderedCollection_add(context, argumentList, argument);
    }

    if(hasArguments && sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_ELLIPSIS)
    {
        hasEllipsis = true;
        ++state->tokenPosition;
    }

    if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_COLON_COLON)
    {
        ++state->tokenPosition;
        hasResultType = true;
        resultTypeExpression = sysbvm_parser_parseUnaryExpression(context, state);
    }

    if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_BAR)
    {
        hasBlockBar = true;
        ++state->tokenPosition;
    }

    if((hasBlockBar || hasResultType) && !hasBlockBar)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a lambda block back.");

    bool isLambda = hasBlockBar;

    sysbvm_tuple_t sequenceNode = sysbvm_parser_parseSequenceUntil(context, state, SYSBVM_TOKEN_KIND_RCBRACKET);

    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RCBRACKET)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a right curly bracket.");

    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;
    sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);

    if(isLambda)
    {
        if(!argumentList)
            argumentList = sysbvm_orderedCollection_create(context);

        sysbvm_tuple_t flags = sysbvm_tuple_bitflags_encode(hasEllipsis ? SYSBVM_FUNCTION_FLAGS_VARIADIC : SYSBVM_FUNCTION_FLAGS_NONE);
        return sysbvm_astLambdaNode_create(context, sourcePosition, flags,
            sysbvm_orderedCollection_asArray(context, argumentList),
            resultTypeExpression, sequenceNode);
    }
    else
    {
        return sysbvm_astLexicalBlockNode_create(context, sourcePosition, sequenceNode);
    }
}

static sysbvm_tuple_t sysbvm_parser_parseLiteralArrayElement(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    switch(sysbvm_parser_lookKindAt(state, 0))
    {
    case -1: return SYSBVM_NULL_TUPLE;
    case SYSBVM_TOKEN_KIND_IDENTIFIER:
    case SYSBVM_TOKEN_KIND_ELLIPSIS:
    case SYSBVM_TOKEN_KIND_MULTI_KEYWORD:
    case SYSBVM_TOKEN_KIND_KEYWORD:
    case SYSBVM_TOKEN_KIND_OPERATOR:
    case SYSBVM_TOKEN_KIND_STAR:
    case SYSBVM_TOKEN_KIND_LESS_THAN:
    case SYSBVM_TOKEN_KIND_GREATER_THAN:
    case SYSBVM_TOKEN_KIND_COLON:
    case SYSBVM_TOKEN_KIND_COLON_COLON:
    case SYSBVM_TOKEN_KIND_BAR:
    case SYSBVM_TOKEN_KIND_COMMA:
    case SYSBVM_TOKEN_KIND_DOT:

    case SYSBVM_TOKEN_KIND_CHARACTER:
    case SYSBVM_TOKEN_KIND_INTEGER:
    case SYSBVM_TOKEN_KIND_FLOAT:
    case SYSBVM_TOKEN_KIND_STRING:
    case SYSBVM_TOKEN_KIND_SYMBOL:
        return sysbvm_parser_parseLiteralTokenValue(context, state);

    case SYSBVM_TOKEN_KIND_LPARENT:
    case SYSBVM_TOKEN_KIND_LITERAL_ARRAY_START:
        return sysbvm_parser_parseLiteralArrayExpression(context, state);
    case SYSBVM_TOKEN_KIND_BYTE_ARRAY_START:
        return sysbvm_parser_parseLiteralByteArrayExpression(context, state);
    default:
        return sysbvm_parser_unexpectedTokenAt(context, state);
    }
}

static sysbvm_tuple_t sysbvm_parser_parseLiteralByteArrayExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_BYTE_ARRAY_START)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a left literal byte array start.");

    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t elements = sysbvm_orderedCollection_create(context);
    while(sysbvm_parser_lookKindAt(state, 0) >= 0
        && sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RBRACKET)
    {
        if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_DOT)
        {
            ++state->tokenPosition;
            continue;
        }
        else if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_INTEGER)
        {
            sysbvm_tuple_t element = sysbvm_parser_parseLiteralTokenValue(context, state);
            if(!element)
                break;
            sysbvm_orderedCollection_add(context, elements, element);
        }
        else
        {
            break;
        }
    }

    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RBRACKET)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a right parent.");

    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;
    return sysbvm_astMakeByteArrayNode_create(context,
        sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition),
        sysbvm_orderedCollection_asArray(context, elements));
}

static sysbvm_tuple_t sysbvm_parser_parseLiteralArrayExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LITERAL_ARRAY_START && sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LPARENT)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a left literal array start.");

    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t elements = sysbvm_orderedCollection_create(context);
    while(sysbvm_parser_lookKindAt(state, 0) >= 0
        && sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RPARENT)
    {
        sysbvm_tuple_t element = sysbvm_parser_parseLiteralArrayElement(context, state);
        if(!element)
            break;
        
        sysbvm_orderedCollection_add(context, elements, element);
    }

    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RPARENT)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a right parent.");

    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;
    return sysbvm_astMakeArrayNode_create(context,
        sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition),
        sysbvm_orderedCollection_asArray(context, elements));
}

static sysbvm_tuple_t sysbvm_parser_parseByteArrayExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_BYTE_ARRAY_START)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a byte array start.");
    
    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t elements = sysbvm_parser_parseExpressionListUntil(context, state, SYSBVM_TOKEN_KIND_RBRACKET);

    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RBRACKET)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a right bracket.");
    
    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;

    return sysbvm_astMakeByteArrayNode_create(context, sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), elements);
}

static sysbvm_tuple_t sysbvm_parser_parseDictionaryElement(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    sysbvm_tuple_t key = SYSBVM_NULL_TUPLE;

    // Parse the key
    if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_KEYWORD)
    {
        sysbvm_tuple_t keyToken = sysbvm_parser_lookAt(state, 0);
        sysbvm_tuple_t keyValue = sysbvm_token_getValue(keyToken);
        keyValue = sysbvm_symbol_internFromTuple(context, sysbvm_string_createWithoutSuffix(context, keyValue, ":"));
        key = sysbvm_astLiteralNode_create(context, sysbvm_token_getSourcePosition(keyToken), keyValue);
        ++state->tokenPosition;
    }
    else
    {
        key = sysbvm_parser_parseBinaryExpression(context, state);
        if(!key || sysbvm_astNode_isErrorNode(context, key))
            return key;
        
        if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_COLON)
        {
            sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, state->tokenPosition);
            return sysbvm_astMakeAssociationNode_create(context, sourcePosition, key, SYSBVM_NULL_TUPLE);
        }
        ++state->tokenPosition;
    }

    // Do we have a value?.
    sysbvm_tuple_t value = SYSBVM_NULL_TUPLE;
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_DOT &&
        sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RCBRACKET)
        value = sysbvm_parser_parseExpression(context, state);

    sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, state->tokenPosition);
    return sysbvm_astMakeAssociationNode_create(context, sourcePosition, key, value);
}

static sysbvm_tuple_t sysbvm_parser_parseDictionaryElements(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_DOT)
        ++state->tokenPosition;

    sysbvm_tuple_t associations = sysbvm_orderedCollection_create(context);
    while(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RCBRACKET)
    {
        sysbvm_tuple_t association = sysbvm_parser_parseDictionaryElement(context, state);
        if(!association)
            break;

        sysbvm_orderedCollection_add(context, associations, association);
        if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_DOT || sysbvm_astNode_isErrorNode(context, association))
            break;
        
        while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_DOT)
            ++state->tokenPosition;
    }

    return sysbvm_orderedCollection_asArray(context, associations);
}

static sysbvm_tuple_t sysbvm_parser_parseDictionaryExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_DICTIONARY_START)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a dictionary start.");

    size_t startPosition = state->tokenPosition;
    ++state->tokenPosition;

    sysbvm_tuple_t elements = sysbvm_parser_parseDictionaryElements(context, state);

    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RCBRACKET)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a right bracket.");

    ++state->tokenPosition;
    size_t endPosition = state->tokenPosition;

    return sysbvm_astMakeDictionaryNode_create(context, sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), elements); 
}

static sysbvm_tuple_t sysbvm_parser_parsePrimaryTerm(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    switch(sysbvm_parser_lookKindAt(state, 0))
    {
    case -1: return SYSBVM_NULL_TUPLE;
    case SYSBVM_TOKEN_KIND_IDENTIFIER:
    case SYSBVM_TOKEN_KIND_ELLIPSIS:
    case SYSBVM_TOKEN_KIND_OPERATOR:
    case SYSBVM_TOKEN_KIND_BAR:
    case SYSBVM_TOKEN_KIND_STAR:
    case SYSBVM_TOKEN_KIND_LESS_THAN:
    case SYSBVM_TOKEN_KIND_GREATER_THAN:
    case SYSBVM_TOKEN_KIND_MULTI_KEYWORD:
        return sysbvm_parser_parseIdentifierReferenceReference(context, state);

    case SYSBVM_TOKEN_KIND_CHARACTER:
    case SYSBVM_TOKEN_KIND_INTEGER:
    case SYSBVM_TOKEN_KIND_FLOAT:
    case SYSBVM_TOKEN_KIND_STRING:
    case SYSBVM_TOKEN_KIND_SYMBOL:
        return sysbvm_parser_parseLiteralTokenValue(context, state);

    case SYSBVM_TOKEN_KIND_LPARENT:
        return sysbvm_parser_parseParenthesesExpression(context, state);
    case SYSBVM_TOKEN_KIND_LCBRACKET:
        return sysbvm_parser_parseBlockExpression(context, state);
    case SYSBVM_TOKEN_KIND_LITERAL_ARRAY_START:
        return sysbvm_parser_parseLiteralArrayExpression(context, state);
    case SYSBVM_TOKEN_KIND_BYTE_ARRAY_START:
        return sysbvm_parser_parseByteArrayExpression(context, state);
    case SYSBVM_TOKEN_KIND_DICTIONARY_START:
        return sysbvm_parser_parseDictionaryExpression(context, state);

    default:
        return sysbvm_parser_unexpectedTokenAt(context, state);
    }
}

static sysbvm_tuple_t sysbvm_parser_parsePrimaryExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    switch(sysbvm_parser_lookKindAt(state, 0))
    {
    case SYSBVM_TOKEN_KIND_QUOTE: return sysbvm_parser_parseQuote(context, state);
    case SYSBVM_TOKEN_KIND_QUASI_QUOTE: return sysbvm_parser_parseQuasiQuote(context, state);
    case SYSBVM_TOKEN_KIND_QUASI_UNQUOTE: return sysbvm_parser_parseQuasiUnquote(context, state);
    case SYSBVM_TOKEN_KIND_SPLICE: return sysbvm_parser_parseSplice(context, state);

    default:
        return sysbvm_parser_parsePrimaryTerm(context, state);
    }
}

static sysbvm_tuple_t sysbvm_parser_parseCallExpressionWithReceiver(sysbvm_context_t *context, sysbvm_parser_state_t *state, sysbvm_tuple_t receiver, size_t receiverPosition)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LPARENT)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a left parenthesis.");
    ++state->tokenPosition;

    // Optional arguments.
    sysbvm_tuple_t argumentOrderedCollection = sysbvm_orderedCollection_create(context);
    sysbvm_orderedCollection_add(context, argumentOrderedCollection, receiver);
    if(sysbvm_parser_lookKindAt(state, 0) >= 0 && sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RPARENT)
    {
        sysbvm_tuple_t argument = sysbvm_parser_parseCommaExpressionElement(context, state);
        sysbvm_orderedCollection_add(context, argumentOrderedCollection, argument);

        while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_COMMA)
        {
            ++state->tokenPosition;
            argument = sysbvm_parser_parseCommaExpressionElement(context, state);
            sysbvm_orderedCollection_add(context, argumentOrderedCollection, argument);
        }
    }

    sysbvm_tuple_t arguments = sysbvm_orderedCollection_asArray(context, argumentOrderedCollection);
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RPARENT)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a right parenthesis.");
    ++state->tokenPosition;

    sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, receiverPosition, state->tokenPosition);
    return sysbvm_astUnexpandedSExpressionNode_create(context, sourcePosition, arguments);
}

static sysbvm_tuple_t sysbvm_parser_parseSubscriptExpressionWithReceiver(sysbvm_context_t *context, sysbvm_parser_state_t *state, sysbvm_tuple_t receiver, size_t receiverPosition)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LBRACKET)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a subscript left bracket.");
    ++state->tokenPosition;

    sysbvm_tuple_t argument = SYSBVM_NULL_TUPLE;
    if(sysbvm_parser_lookKindAt(state, 0) >= 0 && sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RBRACKET)
        argument = sysbvm_parser_parseExpression(context, state);

    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_RBRACKET)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a subscript right bracket.");
    ++state->tokenPosition;

    sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, receiverPosition, state->tokenPosition);
    if(argument)
        return sysbvm_parser_makeBinaryMessageSend(context, sourcePosition, receiver, sysbvm_astLiteralNode_create(context, sourcePosition, sysbvm_symbol_internWithCString(context, "[]:")), argument);
    else
        return sysbvm_parser_makeUnaryMessageSend(context, sourcePosition, receiver, sysbvm_astLiteralNode_create(context, sourcePosition, sysbvm_symbol_internWithCString(context, "[]")));
}

static sysbvm_tuple_t sysbvm_parser_parseApplyBlockExpressionWithReceiver(sysbvm_context_t *context, sysbvm_parser_state_t *state, sysbvm_tuple_t receiver, size_t receiverPosition)
{
    sysbvm_tuple_t blockExpression = sysbvm_parser_parseBlockExpression(context, state);
    sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, receiverPosition, state->tokenPosition);
    return sysbvm_parser_makeBinaryMessageSend(context, sourcePosition, receiver, sysbvm_astLiteralNode_create(context, sourcePosition, sysbvm_symbol_internWithCString(context, "{}:")), blockExpression);
}

static sysbvm_tuple_t sysbvm_parser_parseApplyByteArrayExpressionWithReceiver(sysbvm_context_t *context, sysbvm_parser_state_t *state, sysbvm_tuple_t receiver, size_t receiverPosition)
{
    sysbvm_tuple_t byteArrayExpression = sysbvm_parser_parseByteArrayExpression(context, state);
    sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, receiverPosition, state->tokenPosition);
    return sysbvm_parser_makeBinaryMessageSend(context, sourcePosition, receiver, sysbvm_astLiteralNode_create(context, sourcePosition, sysbvm_symbol_internWithCString(context, "#[]:")), byteArrayExpression);
}

static sysbvm_tuple_t sysbvm_parser_parseApplyDictionaryWithReceiver(sysbvm_context_t *context, sysbvm_parser_state_t *state, sysbvm_tuple_t receiver, size_t receiverPosition)
{
    sysbvm_tuple_t dictionaryExpression = sysbvm_parser_parseDictionaryExpression(context, state);
    sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, receiverPosition, state->tokenPosition);
    return sysbvm_parser_makeBinaryMessageSend(context, sourcePosition, receiver, sysbvm_astLiteralNode_create(context, sourcePosition, sysbvm_symbol_internWithCString(context, "#{}:")), dictionaryExpression);
}

static sysbvm_tuple_t sysbvm_parser_parseUnaryExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    size_t startPosition = state->tokenPosition;

    sysbvm_tuple_t receiver = sysbvm_parser_parsePrimaryExpression(context, state);
    bool attemptToContinue = true;
    while(attemptToContinue)
    {
        switch(sysbvm_parser_lookKindAt(state, 0))
        {
        case SYSBVM_TOKEN_KIND_IDENTIFIER:
            {
                sysbvm_tuple_t selector = sysbvm_parser_parseLiteralTokenValue(context, state);
                size_t endPosition = state->tokenPosition;

                receiver = sysbvm_parser_makeUnaryMessageSend(context, sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), receiver, selector);
            }
            break;
        case SYSBVM_TOKEN_KIND_QUASI_UNQUOTE:
            {
                sysbvm_tuple_t selector = sysbvm_parser_parseQuasiUnquote(context, state);
                size_t endPosition = state->tokenPosition;

                receiver = sysbvm_parser_makeUnaryMessageSend(context, sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), receiver, selector);
            }
            break;
        case SYSBVM_TOKEN_KIND_LPARENT:
            receiver = sysbvm_parser_parseCallExpressionWithReceiver(context, state, receiver, startPosition);
            break;
        case SYSBVM_TOKEN_KIND_LCBRACKET:
            receiver = sysbvm_parser_parseApplyBlockExpressionWithReceiver(context, state, receiver, startPosition);
            break;
        case SYSBVM_TOKEN_KIND_LBRACKET:
            receiver = sysbvm_parser_parseSubscriptExpressionWithReceiver(context, state, receiver, startPosition);
            break;
        case SYSBVM_TOKEN_KIND_BYTE_ARRAY_START:
            receiver = sysbvm_parser_parseApplyByteArrayExpressionWithReceiver(context, state, receiver, startPosition);
            break;
        case SYSBVM_TOKEN_KIND_DICTIONARY_START:
            receiver = sysbvm_parser_parseApplyDictionaryWithReceiver(context, state, receiver, startPosition);
            break;
        default:
            attemptToContinue = false;
            break;
        }
    }

    return receiver;
}

static bool sysbvm_parser_isBinaryExpressionOperator(int tokenKind)
{
    switch(tokenKind)
    {
    case SYSBVM_TOKEN_KIND_OPERATOR:
    case SYSBVM_TOKEN_KIND_STAR:
    case SYSBVM_TOKEN_KIND_LESS_THAN:
    case SYSBVM_TOKEN_KIND_GREATER_THAN:
    case SYSBVM_TOKEN_KIND_BAR:
        return true;
    default:
        return false;
    }
}
static sysbvm_tuple_t sysbvm_parser_parseBinaryExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    sysbvm_tuple_t firstOperand = sysbvm_parser_parseUnaryExpression(context, state);
    if(sysbvm_parser_isBinaryExpressionOperator(sysbvm_parser_lookKindAt(state, 0)))
    {
        sysbvm_tuple_t elements = sysbvm_orderedCollection_create(context);
        sysbvm_orderedCollection_add(context, elements, firstOperand);

        while(sysbvm_parser_isBinaryExpressionOperator(sysbvm_parser_lookKindAt(state, 0)))
        {
            sysbvm_tuple_t binaryOperator = sysbvm_parser_parseLiteralTokenValue(context, state);
            sysbvm_orderedCollection_add(context, elements, binaryOperator);

            sysbvm_tuple_t nextOperand = sysbvm_parser_parseUnaryExpression(context, state);
            sysbvm_orderedCollection_add(context, elements, nextOperand);
        }

        size_t endPosition = state->tokenPosition;
        sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);

        // Collapse single binary operation into a message send here.
        if(sysbvm_orderedCollection_getSize(elements) == 3)
        {
            sysbvm_tuple_t receiver = sysbvm_orderedCollection_at(elements, 0);
            sysbvm_tuple_t selector = sysbvm_orderedCollection_at(elements, 1);
            sysbvm_tuple_t argument = sysbvm_orderedCollection_at(elements, 2);
            return sysbvm_parser_makeBinaryMessageSend(context, sourcePosition, receiver, selector, argument);
        }

        return sysbvm_astBinaryExpressionSequenceNode_create(context, sourcePosition, sysbvm_orderedCollection_asArray(context, elements));
    }
    else
    {
        return firstOperand;
    }
}

static sysbvm_tuple_t sysbvm_parser_parseMessageWithoutReceiver(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_KEYWORD)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a subscript left bracket.");
    
    size_t startPosition = state->tokenPosition;
    size_t keywordEndPosition = state->tokenPosition;
    sysbvm_tuple_t selectorBuilder = sysbvm_stringStream_create(context);
    sysbvm_tuple_t argumentOrderedCollection = sysbvm_orderedCollection_create(context);
    while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_KEYWORD)
    {
        sysbvm_stringStream_nextPutString(context, selectorBuilder, sysbvm_token_getValue(sysbvm_parser_lookAt(state, 0)));
        ++state->tokenPosition;
        keywordEndPosition = state->tokenPosition;

        sysbvm_tuple_t argument = sysbvm_parser_parseBinaryExpression(context, state);
        sysbvm_orderedCollection_add(context, argumentOrderedCollection, argument);
    }

    sysbvm_tuple_t keywordSourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, keywordEndPosition);
    sysbvm_tuple_t functionExpression = sysbvm_astIdentifierReferenceNode_create(context, keywordSourcePosition, sysbvm_stringStream_asSymbol(context, selectorBuilder));

    size_t endPosition = state->tokenPosition;
    sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
    return sysbvm_astUnexpandedApplicationNode_create(context, sourcePosition, functionExpression, sysbvm_orderedCollection_asArray(context, argumentOrderedCollection));
}

static void sysbvm_parser_parseKeywordMessageParts(sysbvm_context_t *context, sysbvm_parser_state_t *state, sysbvm_tuple_t *outSelector, sysbvm_tuple_t *outArguments)
{
    size_t keywordStartPosition = state->tokenPosition;
    size_t keywordEndPosition = state->tokenPosition;
    sysbvm_tuple_t selectorBuilder = sysbvm_stringStream_create(context);
    sysbvm_tuple_t argumentOrderedCollection = sysbvm_orderedCollection_create(context);
    while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_KEYWORD)
    {
        sysbvm_stringStream_nextPutString(context, selectorBuilder, sysbvm_token_getValue(sysbvm_parser_lookAt(state, 0)));
        ++state->tokenPosition;
        keywordEndPosition = state->tokenPosition;

        sysbvm_tuple_t argument = sysbvm_parser_parseBinaryExpression(context, state);
        sysbvm_orderedCollection_add(context, argumentOrderedCollection, argument);
    }

    sysbvm_tuple_t keywordSourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, keywordStartPosition, keywordEndPosition);
    *outSelector = sysbvm_astLiteralNode_create(context, keywordSourcePosition, sysbvm_stringStream_asSymbol(context, selectorBuilder));
    *outArguments = sysbvm_orderedCollection_asArray(context, argumentOrderedCollection);
}

static sysbvm_tuple_t sysbvm_parser_parseMessageChainList(sysbvm_context_t *context, sysbvm_parser_state_t *state, size_t startPosition, sysbvm_tuple_t receiver, sysbvm_tuple_t firstChainedMessage)
{
    sysbvm_tuple_t chainedMessageList = sysbvm_orderedCollection_create(context);
    sysbvm_orderedCollection_add(context, chainedMessageList, firstChainedMessage);

    while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_SEMICOLON)
    {
        ++state->tokenPosition;
        switch(sysbvm_parser_lookKindAt(state, 0))
        {
        case SYSBVM_TOKEN_KIND_KEYWORD:
            {
                size_t chainedStartPosition = state->tokenPosition;
                sysbvm_tuple_t selector = SYSBVM_NULL_TUPLE;
                sysbvm_tuple_t arguments = SYSBVM_NULL_TUPLE;

                sysbvm_parser_parseKeywordMessageParts(context, state, &selector, &arguments);
                sysbvm_tuple_t chainedSourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, chainedStartPosition, state->tokenPosition);
                sysbvm_orderedCollection_add(context, chainedMessageList, sysbvm_astMessageChainMessageNode_create(context, chainedSourcePosition, selector, arguments));
            }
            break;
        
        case SYSBVM_TOKEN_KIND_OPERATOR:
        case SYSBVM_TOKEN_KIND_BAR:
        case SYSBVM_TOKEN_KIND_STAR:
        case SYSBVM_TOKEN_KIND_LESS_THAN:
        case SYSBVM_TOKEN_KIND_GREATER_THAN:
            {
                size_t chainedStartPosition = state->tokenPosition;
                sysbvm_tuple_t selector = sysbvm_parser_parseLiteralTokenValue(context, state);
                sysbvm_tuple_t argument = sysbvm_parser_parseBinaryExpression(context, state);
                sysbvm_tuple_t chainedSourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, chainedStartPosition, state->tokenPosition);

                sysbvm_tuple_t arguments = sysbvm_array_create(context, 1);
                sysbvm_array_atPut(arguments, 0, argument);
                sysbvm_orderedCollection_add(context, chainedMessageList, sysbvm_astMessageChainMessageNode_create(context, chainedSourcePosition, selector, arguments));
            }
            break;
        case SYSBVM_TOKEN_KIND_IDENTIFIER:
            {
                size_t chainedStartPosition = state->tokenPosition;
                sysbvm_tuple_t selector = sysbvm_parser_parseLiteralTokenValue(context, state);
                sysbvm_tuple_t arguments = sysbvm_array_create(context, 0);
                sysbvm_tuple_t chainedSourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, chainedStartPosition, state->tokenPosition);
                sysbvm_orderedCollection_add(context, chainedMessageList, sysbvm_astMessageChainMessageNode_create(context, chainedSourcePosition, selector, arguments));
            }
            break;
        default:
            sysbvm_orderedCollection_add(context, chainedMessageList, sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a chained message."));
            ++state->tokenPosition;
            break;
        }
    }

    size_t endPosition = state->tokenPosition;
    sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
    return sysbvm_astMessageChainNode_create(context, sourcePosition, receiver, sysbvm_orderedCollection_asArray(context, chainedMessageList));
}

static sysbvm_tuple_t sysbvm_parser_parseChainExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_KEYWORD)
    {
        sysbvm_tuple_t messageWithoutReceiver = sysbvm_parser_parseMessageWithoutReceiver(context, state);
        if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_SEMICOLON)
        {
            sysbvm_tuple_t messages = sysbvm_orderedCollection_create(context);
            sysbvm_orderedCollection_add(context, messages, messageWithoutReceiver);
            while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_SEMICOLON)
            {
                ++state->tokenPosition;
                messageWithoutReceiver = sysbvm_parser_parseMessageWithoutReceiver(context, state);
                sysbvm_orderedCollection_add(context, messages, messageWithoutReceiver);
            }

            size_t endPosition = state->tokenPosition;
            sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
            sysbvm_tuple_t pragmas = sysbvm_array_create(context, 0);
            return sysbvm_astSequenceNode_create(context, sourcePosition, pragmas, sysbvm_orderedCollection_asArray(context, messages));
        }
        else
        {
            return messageWithoutReceiver;
        }
    }

    sysbvm_tuple_t receiver = sysbvm_parser_parseBinaryExpression(context, state);
    if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_KEYWORD)
    {
        sysbvm_tuple_t selector = SYSBVM_NULL_TUPLE;
        sysbvm_tuple_t arguments = SYSBVM_NULL_TUPLE;
        size_t keywordStartPosition = state->tokenPosition;
        sysbvm_parser_parseKeywordMessageParts(context, state, &selector, &arguments);

        if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_SEMICOLON)
        {
            sysbvm_tuple_t chainedSourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, keywordStartPosition, state->tokenPosition);
            sysbvm_tuple_t firstChainedMessage = sysbvm_astMessageChainMessageNode_create(context, chainedSourcePosition, selector, arguments);
            return sysbvm_parser_parseMessageChainList(context, state, startPosition, receiver, firstChainedMessage);
        }
        else
        {
            size_t endPosition = state->tokenPosition;
            sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
            return sysbvm_astMessageSendNode_create(context, sourcePosition, receiver, selector, arguments);
        }
    }
    else if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_SEMICOLON)
    {
        if(!sysbvm_astNode_isMessageSendNode(context, receiver))
            return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Message chain requires a starting message expression for its receiver.");

        sysbvm_astMessageSendNode_t *receiverMessageSend = (sysbvm_astMessageSendNode_t *)receiver;
        sysbvm_tuple_t firstChainedMessage = sysbvm_astMessageChainMessageNode_create(context, receiverMessageSend->super.sourcePosition, receiverMessageSend->selector, receiverMessageSend->arguments);
        return sysbvm_parser_parseMessageChainList(context, state, startPosition, receiverMessageSend->receiver, firstChainedMessage);
    }

    return receiver;
}

static sysbvm_tuple_t sysbvm_parser_parseLowPrecedenceExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    sysbvm_tuple_t lastExpression = sysbvm_parser_parseChainExpression(context, state);
    while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_COLON_COLON && sysbvm_parser_isBinaryExpressionOperator(sysbvm_parser_lookKindAt(state, 1)))
    {
        ++state->tokenPosition;
        sysbvm_tuple_t binaryOperator = sysbvm_parser_parseLiteralTokenValue(context, state);

        sysbvm_tuple_t argument = sysbvm_parser_parseChainExpression(context, state);
        size_t endPosition = state->tokenPosition;
        lastExpression = sysbvm_parser_makeBinaryMessageSend(context, sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), lastExpression, binaryOperator, argument);
    }
    
    return lastExpression;
}

static sysbvm_tuple_t sysbvm_parser_parseAssignmentExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    sysbvm_tuple_t reference = sysbvm_parser_parseLowPrecedenceExpression(context, state);
    if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_ASSIGNMENT)
    {
        sysbvm_tuple_t assignmentOperator = sysbvm_parser_parseLiteralTokenValue(context, state);
        
        sysbvm_tuple_t value = sysbvm_parser_parseAssignmentExpression(context, state);
        size_t endPosition = state->tokenPosition;
        return sysbvm_parser_makeBinaryMessageSend(context, sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition), reference, assignmentOperator, value);
    }
    else
    {
        return reference;
    }
}

static sysbvm_tuple_t sysbvm_parser_parseCommaExpressionElement(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    return sysbvm_parser_parseAssignmentExpression(context, state);
}

static sysbvm_tuple_t sysbvm_parser_parseCommaExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    sysbvm_tuple_t firstElement = sysbvm_parser_parseCommaExpressionElement(context, state);
    if(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_COMMA)
    {
        sysbvm_tuple_t elements = sysbvm_orderedCollection_create(context);
        sysbvm_orderedCollection_add(context, elements, firstElement);

        while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_COMMA)
        {
            ++state->tokenPosition;
            sysbvm_parser_state_t savedState = *state;
            sysbvm_tuple_t nextElement = sysbvm_parser_parseCommaExpressionElement(context, state);
            if(!nextElement || sysbvm_astNode_isErrorNode(context, nextElement))
            {
                *state = savedState;
                break;
            }
            sysbvm_orderedCollection_add(context, elements, nextElement);
        }

        size_t endPosition = state->tokenPosition;
        return sysbvm_astMakeArrayNode_create(context,
            sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition),
            sysbvm_orderedCollection_asArray(context, elements));
    }
    else
    {
        return firstElement;
    }
}

static sysbvm_tuple_t sysbvm_parser_parseExpression(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    return sysbvm_parser_parseCommaExpression(context, state);
}

static sysbvm_tuple_t sysbvm_parser_parseExpressionList(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    sysbvm_tuple_t expressionOrderedCollection = sysbvm_orderedCollection_create(context);

    // Parse the expressions on the sequence.
    while(sysbvm_parser_lookKindAt(state, 0) >= 0)
    {
        // Skip the dots at the beginning
        while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_DOT)
            ++state->tokenPosition;

        sysbvm_tuple_t expression = sysbvm_parser_parseExpression(context, state);
        if(SYSBVM_NULL_TUPLE == expression)
            break;

        sysbvm_orderedCollection_add(context, expressionOrderedCollection, expression);

        // We need at least a single dot before the next expression.
        if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_DOT)
            break;

        // Skip the extra dots.
        while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_DOT)
            ++state->tokenPosition;
    }

    return sysbvm_orderedCollection_asArray(context, expressionOrderedCollection);
}

static sysbvm_tuple_t sysbvm_parser_parsePragma(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LESS_THAN)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a subscript left bracket.");
    ++state->tokenPosition;

    sysbvm_tuple_t selector = SYSBVM_NULL_TUPLE;
    sysbvm_tuple_t arguments = SYSBVM_NULL_TUPLE;

    switch(sysbvm_parser_lookKindAt(state, 0))
    {
    case SYSBVM_TOKEN_KIND_IDENTIFIER:
        selector = sysbvm_parser_parseLiteralTokenValue(context, state);
        arguments = sysbvm_array_create(context, 0);
        break;
    case SYSBVM_TOKEN_KIND_KEYWORD:
        {
            size_t keywordStartPosition = state->tokenPosition;
            size_t keywordEndPosition = keywordStartPosition;
            sysbvm_tuple_t selectorBuilder = sysbvm_stringStream_create(context);
            sysbvm_tuple_t argumentOrderedCollection = sysbvm_orderedCollection_create(context);
            while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_KEYWORD)
            {
                sysbvm_stringStream_nextPutString(context, selectorBuilder, sysbvm_token_getValue(sysbvm_parser_lookAt(state, 0)));
                ++state->tokenPosition;

                sysbvm_tuple_t argument = sysbvm_parser_parseUnaryExpression(context, state);
                sysbvm_orderedCollection_add(context, argumentOrderedCollection, argument);
            }

            sysbvm_tuple_t selectorSourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, keywordStartPosition, keywordEndPosition);
            selector = sysbvm_astLiteralNode_create(context, selectorSourcePosition, sysbvm_stringStream_asSymbol(context, selectorBuilder));
            arguments = sysbvm_orderedCollection_asArray(context, argumentOrderedCollection);
        }
        break;
    default:
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a valid pragma content.");
    }

    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_GREATER_THAN)
        return sysbvm_astErrorNode_createWithCString(context, sysbvm_parser_makeSourcePositionForParserState(context, state), "Expected a pragma end.");
    ++state->tokenPosition;

    sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, state->tokenPosition);
    return sysbvm_astPragmaNode_create(context, sourcePosition, selector, arguments);
}

static sysbvm_tuple_t sysbvm_parser_parsePragmaList(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_LESS_THAN)
        return sysbvm_array_create(context, 0);

    sysbvm_tuple_t list = sysbvm_orderedCollection_create(context);
    while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_LESS_THAN)
    {
        sysbvm_tuple_t pragma = sysbvm_parser_parsePragma(context, state);
        sysbvm_orderedCollection_add(context, list, pragma);
    }

    return sysbvm_orderedCollection_asArray(context, list);
}

static sysbvm_tuple_t sysbvm_parser_parseSequence(sysbvm_context_t *context, sysbvm_parser_state_t *state)
{
    size_t startPosition = state->tokenPosition;
    sysbvm_tuple_t pragmas = sysbvm_parser_parsePragmaList(context, state);
    sysbvm_tuple_t expressionsArraySlice = sysbvm_parser_parseExpressionList(context, state);
    size_t endPosition = state->tokenPosition;

    sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
    return sysbvm_astSequenceNode_create(context, sourcePosition, pragmas, expressionsArraySlice);
}

static sysbvm_tuple_t sysbvm_parser_parseExpressionListUntil(sysbvm_context_t *context, sysbvm_parser_state_t *state, sysbvm_tokenKind_t delimiter)
{
    sysbvm_tuple_t expressionOrderedCollection = sysbvm_orderedCollection_create(context);

    // Parse the expressions on the sequence.
    while(sysbvm_parser_lookKindAt(state, 0) >= 0 && sysbvm_parser_lookKindAt(state, 0) != (int)delimiter)
    {
        // Skip the dots at the beginning
        while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_DOT)
            ++state->tokenPosition;

        sysbvm_tuple_t expression = sysbvm_parser_parseExpression(context, state);
        if(SYSBVM_NULL_TUPLE == expression)
            break;

        sysbvm_orderedCollection_add(context, expressionOrderedCollection, expression);

        // We need at least a single dot before the next expression.
        if(sysbvm_parser_lookKindAt(state, 0) != SYSBVM_TOKEN_KIND_DOT)
            break;

        // Skip the extra dots.
        while(sysbvm_parser_lookKindAt(state, 0) == SYSBVM_TOKEN_KIND_DOT)
            ++state->tokenPosition;
    }

    return sysbvm_orderedCollection_asArray(context, expressionOrderedCollection);
}

static sysbvm_tuple_t sysbvm_parser_parseSequenceUntil(sysbvm_context_t *context, sysbvm_parser_state_t *state, sysbvm_tokenKind_t delimiter)
{
    size_t startPosition = state->tokenPosition;
    sysbvm_tuple_t pragmas = sysbvm_parser_parsePragmaList(context, state);
    sysbvm_tuple_t expressionsArraySlice = sysbvm_parser_parseExpressionListUntil(context, state, delimiter);
    size_t endPosition = state->tokenPosition;

    sysbvm_tuple_t sourcePosition = sysbvm_parser_makeSourcePositionForTokenRange(context, state->sourceCode, state->tokenSequence, startPosition, endPosition);
    return sysbvm_astSequenceNode_create(context, sourcePosition, pragmas, expressionsArraySlice);
}

SYSBVM_API sysbvm_tuple_t sysbvm_parser_parseTokens(sysbvm_context_t *context, sysbvm_tuple_t sourceCode, sysbvm_tuple_t tokenSequence)
{
    struct {
        sysbvm_tuple_t result;
    } gcFrame = {0};

    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    sysbvm_gc_lock(context);
    sysbvm_parser_state_t parserState = {
        .sourceCode = sourceCode,
        .tokenSequence = tokenSequence,
        .tokenPosition = 0,
        .tokenSequenceSize = sysbvm_arraySlice_getSize(tokenSequence),
    };

    gcFrame.result = sysbvm_parser_parseSequence(context, &parserState);
    sysbvm_gc_unlock(context);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_parser_parseSourceCode(sysbvm_context_t *context, sysbvm_tuple_t sourceCode)
{
    struct {
        sysbvm_tuple_t sourceCode;
        sysbvm_tuple_t tokenSequence;
        sysbvm_tuple_t result;
    } gcFrame = {0};
    SYSBVM_STACKFRAME_PUSH_GC_ROOTS(gcFrameRecord, gcFrame);

    gcFrame.sourceCode = sourceCode;
    gcFrame.tokenSequence = sysbvm_scanner_scan(context, gcFrame.sourceCode);
    gcFrame.result = sysbvm_parser_parseTokens(context, gcFrame.sourceCode, gcFrame.tokenSequence);
    SYSBVM_STACKFRAME_POP_GC_ROOTS(gcFrameRecord);
    return gcFrame.result;
}

SYSBVM_API sysbvm_tuple_t sysbvm_parser_parseCString(sysbvm_context_t *context, const char *sourceCodeText, const char *sourceCodeName)
{
    sysbvm_tuple_t sourceCode = sysbvm_sourceCode_createWithCStrings(context, sourceCodeText, "", sourceCodeName, "sysmel");
    return sysbvm_parser_parseSourceCode(context, sourceCode);
}
