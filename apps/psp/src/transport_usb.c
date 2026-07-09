#include "protocol/transport.h"
#include <pspkernel.h>
#include <pspdebug.h>
#include <stdio.h>
#include <string.h>

#include "../driver/pspdl_ioctl.h"

// Reference to global variables declared in main.c
extern char g_status_msg[256];
int g_mock_mode = 0; // Define it here instead of extern since it seems it was missing or private

static SceUID g_driver_mod = -1;
static SceUID g_io_fd = -1;

PSPDL_TransportResult transport_initialize(const char *launch_path)
{
    // Try to open the driver directly if it's already loaded
    g_io_fd = sceIoOpen("pspdl0:", PSP_O_RDWR, 0777);
    
    if (g_io_fd < 0)
    {
        // Try to load the PRX from the same directory as the EBOOT
        g_driver_mod = sceKernelLoadModule("ms0:/PSP/GAME/PSPDevLink/pspdl_driver.prx", 0, NULL);
        if (g_driver_mod < 0)
        {
            snprintf(g_status_msg, sizeof(g_status_msg), "PRX load fail (0x%08X). Mock Mode.", (unsigned int)g_driver_mod);
            g_mock_mode = 1;
            return PSPDL_TRANSPORT_OK;
        }

        int status = 0;
        int ret = sceKernelStartModule(g_driver_mod, 0, NULL, &status, NULL);
        if (ret < 0)
        {
            snprintf(g_status_msg, sizeof(g_status_msg), "PRX start fail (0x%08X). Mock Mode.", (unsigned int)ret);
            g_mock_mode = 1;
            return PSPDL_TRANSPORT_OK;
        }
        
        // Open the driver now that it is started
        g_io_fd = sceIoOpen("pspdl0:", PSP_O_RDWR, 0777);
        if (g_io_fd < 0)
        {
            snprintf(g_status_msg, sizeof(g_status_msg), "IO open fail (0x%08X). Mock Mode.", (unsigned int)g_io_fd);
            g_mock_mode = 1;
            return PSPDL_TRANSPORT_OK;
        }
    }

    // Initialize the USB driver through IOCTL
    int ret = sceIoIoctl(g_io_fd, PSPDL_IOCTL_INIT, NULL, 0, NULL, 0);
    if (ret < 0)
    {
        snprintf(g_status_msg, sizeof(g_status_msg), "IOCTL INIT fail (0x%08X). Mock Mode.", (unsigned int)ret);
        g_mock_mode = 1;
        return PSPDL_TRANSPORT_OK;
    }

    snprintf(g_status_msg, sizeof(g_status_msg), "Waiting for Host...");
    return PSPDL_TRANSPORT_OK;
}

PSPDL_TransportResult transport_send(const void *data, size_t size)
{
    if (g_mock_mode || g_io_fd < 0) return PSPDL_TRANSPORT_OK;

    struct pspdl_send_args args;
    args.data = data;
    args.size = size;

    int ret = sceIoIoctl(g_io_fd, PSPDL_IOCTL_SEND, &args, sizeof(args), NULL, 0);
    if (ret < 0) return PSPDL_TRANSPORT_ERROR;

    return PSPDL_TRANSPORT_OK;
}

PSPDL_TransportResult transport_receive(void *buffer, size_t max_size, size_t *out_size)
{
    if (g_mock_mode || g_io_fd < 0) return PSPDL_TRANSPORT_OK;

    int received = 0;
    struct pspdl_recv_args args;
    args.buf = buffer;
    args.size = max_size;
    args.received = &received;

    int ret = sceIoIoctl(g_io_fd, PSPDL_IOCTL_RECV, &args, sizeof(args), NULL, 0);
    if (ret < 0) return PSPDL_TRANSPORT_ERROR;

    if (out_size)
    {
        *out_size = received;
    }

    return PSPDL_TRANSPORT_OK;
}

PSPDL_TransportResult transport_shutdown(void)
{
    if (!g_mock_mode && g_io_fd >= 0)
    {
        sceIoIoctl(g_io_fd, PSPDL_IOCTL_SHUTDOWN, NULL, 0, NULL, 0);
        sceIoClose(g_io_fd);
        g_io_fd = -1;
    }

    if (g_driver_mod >= 0)
    {
        int status = 0;
        sceKernelStopModule(g_driver_mod, 0, NULL, &status, NULL);
        sceKernelUnloadModule(g_driver_mod);
        g_driver_mod = -1;
    }
    return PSPDL_TRANSPORT_OK;
}