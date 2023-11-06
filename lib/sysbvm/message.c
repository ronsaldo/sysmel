#include "sysbvm/message.h"
#include "internal/context.h"
#include <stdlib.h>

SYSBVM_API sysbvm_tuple_t sysbvm_message_create(sysbvm_context_t *context, sysbvm_tuple_t selector, sysbvm_tuple_t arguments, sysbvm_tuple_t sourcePosition)
{
    sysbvm_message_t *result = (sysbvm_message_t*)sysbvm_context_allocatePointerTuple(context, context->roots.messageType, SYSBVM_SLOT_COUNT_FOR_STRUCTURE_TYPE(sysbvm_message_t));
    result->selector = selector;
    result->arguments = arguments;
    result->sourcePosition = sourcePosition;
    return (sysbvm_tuple_t)result;
}
