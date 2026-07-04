#include "protocol/packet.h"
#include <stddef.h>

int pspl_serialize_header(
    const PSPDL_PacketHeader *header,
    uint8_t *buffer,
    size_t size)
{
    if (header == NULL || buffer == NULL || size < PSPDL_PACKET_HEADER_SIZE)
    {
        return -1;
    }

    // Pack 16-bit fields (Little-Endian)
    buffer[0] = (uint8_t)(header->magic & 0xFF);
    buffer[1] = (uint8_t)((header->magic >> 8) & 0xFF);

    buffer[2] = (uint8_t)(header->protocol_version & 0xFF);
    buffer[3] = (uint8_t)((header->protocol_version >> 8) & 0xFF);

    // Pack 32-bit message_id (Little-Endian)
    uint32_t msg_id = (uint32_t)header->message_id;
    buffer[4] = (uint8_t)(msg_id & 0xFF);
    buffer[5] = (uint8_t)((msg_id >> 8) & 0xFF);
    buffer[6] = (uint8_t)((msg_id >> 16) & 0xFF);
    buffer[7] = (uint8_t)((msg_id >> 24) & 0xFF);

    // Pack 32-bit payload_size (Little-Endian)
    buffer[8] = (uint8_t)(header->payload_size & 0xFF);
    buffer[9] = (uint8_t)((header->payload_size >> 8) & 0xFF);
    buffer[10] = (uint8_t)((header->payload_size >> 16) & 0xFF);
    buffer[11] = (uint8_t)((header->payload_size >> 24) & 0xFF);

    return 0;
}

int pspl_deserialize_header(
    const uint8_t *buffer,
    size_t size,
    PSPDL_PacketHeader *header)
{
    if (buffer == NULL || header == NULL || size < PSPDL_PACKET_HEADER_SIZE)
    {
        return -1;
    }

    // Unpack 16-bit fields
    header->magic = (uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8);
    header->protocol_version = (uint16_t)buffer[2] | ((uint16_t)buffer[3] << 8);

    // Unpack 32-bit message_id
    uint32_t msg_id = (uint32_t)buffer[4] | 
                      ((uint32_t)buffer[5] << 8) | 
                      ((uint32_t)buffer[6] << 16) | 
                      ((uint32_t)buffer[7] << 24);
    header->message_id = (PSPDL_MessageId)msg_id;

    // Unpack 32-bit payload_size
    header->payload_size = (uint32_t)buffer[8] | 
                           ((uint32_t)buffer[9] << 8) | 
                           ((uint32_t)buffer[10] << 16) | 
                           ((uint32_t)buffer[11] << 24);

    // Validate magic signature
    if (header->magic != PSPDL_PROTOCOL_MAGIC)
    {
        return -2;
    }

    return 0;
}
