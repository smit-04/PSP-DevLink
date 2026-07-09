#include "protocol/transport.h"
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspusb.h>
#include <pspusbbus.h>
#include <stdio.h>
#include <string.h>

#include "../driver/pspdl_ioctl.h"

// Reference to global variables declared in main.c
extern char g_status_msg[256];
int g_mock_mode = 0;

static SceUID g_usb_mod = -1;
static SceUID g_usbbd_mod = -1;
static SceUID g_driver_mod = -1;
static SceUID g_io_fd = -1;

PSPDL_TransportResult transport_initialize(const char *launch_path)
{
    // 1. Load system USB core modules from flash0.
    // If already loaded, we ignore the error.
    g_usb_mod = sceKernelLoadModule("flash0:/kd/usb.prx", 0, NULL);
    if (g_usb_mod >= 0)
    {
        int status = 0;
        sceKernelStartModule(g_usb_mod, 0, NULL, &status, NULL);
    }

    g_usbbd_mod = sceKernelLoadModule("flash0:/kd/usbbd.prx", 0, NULL);
    if (g_usbbd_mod >= 0)
    {
        int status = 0;
        sceKernelStartModule(g_usbbd_mod, 0, NULL, &status, NULL);
    }

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

    // Start USB hardware and activate our driver on the bus
    int usb_start_bus = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
    int usb_start_drv = sceUsbStart("PSPDevLinkDriver", 0, 0);
    int usb_act = sceUsbActivate(0x011A); // Use the custom PID we registered (0x011A)

    if (usb_start_bus < 0 || usb_start_drv < 0 || usb_act < 0)
    {
        snprintf(g_status_msg, sizeof(g_status_msg), "USB Start Err (Bus:0x%X, Drv:0x%X, Act:0x%X)", 
                 (unsigned int)usb_start_bus, (unsigned int)usb_start_drv, (unsigned int)usb_act);
    }
    else
    {
        snprintf(g_status_msg, sizeof(g_status_msg), "Waiting for Host...");
    }
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
    // Let the PSP OS automatically clean up open file descriptors, USB drivers, 
    // and kernel modules upon sceKernelExitGame(). Manual teardown of low-level 
    // USB structures often triggers kernel race conditions on exit.
    return PSPDL_TRANSPORT_OK;
}