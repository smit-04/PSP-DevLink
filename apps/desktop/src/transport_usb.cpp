#include "protocol/transport.h"

PSPDL_TransportResult transport_initialize(void)
{
    return PSPDL_TRANSPORT_NOT_IMPLEMENTED;
}

PSPDL_TransportResult transport_shutdown(void)
{
    return PSPDL_TRANSPORT_NOT_IMPLEMENTED;
}

PSPDL_TransportResult transport_send(
    const void *data,
    size_t size)
{
    (void)data;
    (void)size;

    return PSPDL_TRANSPORT_NOT_IMPLEMENTED;
}

PSPDL_TransportResult transport_receive(
    void *buffer,
    size_t size,
    size_t *received)
{
    (void)buffer;
    (void)size;

    if (received != NULL)
    {
        *received = 0;
    }

    return PSPDL_TRANSPORT_NOT_IMPLEMENTED;
}