#ifndef PSPDL_PROTOCOL_PACKET_H
#define PSPDL_PROTOCOL_PACKET_H

#include <stdint.h>
#include <stddef.h>

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

#define PSPDL_PACKET_HEADER_SIZE 12

#ifdef __cplusplus
extern "C" {
#endif

int pspl_serialize_header(const PSPDL_PacketHeader *header, uint8_t *buffer, size_t size);
int pspl_deserialize_header(const uint8_t *buffer, size_t size, PSPDL_PacketHeader *header);

#ifdef __cplusplus
}
#endif

#endif /* PSPDL_PROTOCOL_PACKET_H */
