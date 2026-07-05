#ifndef PSPDL_PROTOCOL_TRANSPORT_H
#define PSPDL_PROTOCOL_TRANSPORT_H

#include <stddef.h>
#include <stdint.h>

/*
 * Generic transport interface.
 *
 * Platform-specific implementations provide these functions.
 * Milestone 3 provides only stub implementations.
 */

typedef enum
{
    PSPDL_TRANSPORT_OK = 0,
    PSPDL_TRANSPORT_NOT_IMPLEMENTED,
    PSPDL_TRANSPORT_ERROR

} PSPDL_TransportResult;

PSPDL_TransportResult transport_initialize(const char *launch_path);


PSPDL_TransportResult transport_shutdown(void);

PSPDL_TransportResult transport_send(
    const void *data,
    size_t size);

PSPDL_TransportResult transport_receive(
    void *buffer,
    size_t size,
    size_t *received);

#endif /* PSPDL_PROTOCOL_TRANSPORT_H */
