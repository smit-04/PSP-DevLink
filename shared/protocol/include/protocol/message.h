#ifndef PSPDL_PROTOCOL_MESSAGE_H
#define PSPDL_PROTOCOL_MESSAGE_H

#include <stdint.h>

/*
 * Protocol message identifiers.
 *
 * These IDs identify the type of payload carried by a protocol packet.
 * Payload formats will be defined in future milestones.
 */
typedef enum
{
    PSPDL_MESSAGE_INVALID = 0,

    /* Connection management */
    PSPDL_MESSAGE_HELLO = 1,
    PSPDL_MESSAGE_HEARTBEAT = 2,

    /* Reserved for future expansion */

} PSPDL_MessageId;

#endif /* PSPDL_PROTOCOL_MESSAGE_H */
