#ifndef SYSBVM_MESSAGE_H
#define SYSBVM_MESSAGE_H

#include "tuple.h"

typedef struct sysbvm_context_s sysbvm_context_t;

typedef struct sysbvm_message_s
{
    sysbvm_tuple_header_t header;
    sysbvm_tuple_t selector;
    sysbvm_tuple_t arguments;
    sysbvm_tuple_t sourcePosition;
} sysbvm_message_t;

/**
 * Creates a message.
 */
SYSBVM_API sysbvm_tuple_t sysbvm_message_create(sysbvm_context_t *context, sysbvm_tuple_t selector, sysbvm_tuple_t arguments, sysbvm_tuple_t sourcePosition);

#endif //SYSBVM_MESSAGE_H