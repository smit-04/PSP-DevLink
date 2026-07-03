#ifndef PSPDL_PROTOCOL_PACKET_H
#define PSPDL_PROTOCOL_PACKET_H

#include <stdint.h>

#include "protocol/protocol.h"
#include "protocol/message.h"
/*
 * Every packet transmitted between the Desktop and PSP
 * begins with this fixed header.
 */


typedef struct
{
    uint16_t magic;
    uint16_t protocol_version;

    PSPDL_MessageId message_id;

    uint32_t payload_size;

} PSPDL_PacketHeader;

#endif /* PSPDL_PROTOCOL_PACKET_H */
