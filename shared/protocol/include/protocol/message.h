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

    /* System Telemetry */
    PSPDL_MESSAGE_SYSTEM_STATS = 3,
    PSPDL_MESSAGE_GIT_STATUS = 4,
    PSPDL_MESSAGE_NOTIFICATION = 5,
    PSPDL_MESSAGE_CONTROL = 6

} PSPDL_MessageId;

#endif /* PSPDL_PROTOCOL_MESSAGE_H */
