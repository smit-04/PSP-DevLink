#include "protocol/transport.h"
#include <libusb.h>
#include <iostream>

#define PSPDL_USB_VENDOR_ID  0xFFFF
#define PSPDL_USB_PRODUCT_ID 0x0001
#define BULK_OUT_ENDPOINT    0x02  // Host to PSP
#define BULK_IN_ENDPOINT     0x81  // PSP to Host
#define USB_TIMEOUT_MS       5000

static libusb_device_handle *g_usb_handle = nullptr;
static libusb_context *g_usb_ctx = nullptr;

PSPDL_TransportResult transport_initialize(void)
{
    if (g_usb_handle != nullptr)
    {
        return PSPDL_TRANSPORT_OK;
    }

    int r = libusb_init(&g_usb_ctx);
    if (r < 0)
    {
        std::cerr << "[ERROR] libusb_init failed: " << r << std::endl;
        return PSPDL_TRANSPORT_ERROR;
    }

    // Set debugging info to warning/error levels
    libusb_set_option(g_usb_ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_WARNING);

    g_usb_handle = libusb_open_device_with_vid_pid(g_usb_ctx, PSPDL_USB_VENDOR_ID, PSPDL_USB_PRODUCT_ID);
    if (g_usb_handle == nullptr)
    {
        std::cerr << "[ERROR] Could not open PSP DevLink device (VID: 0xFFFF, PID: 0x0001)" << std::endl;
        libusb_exit(g_usb_ctx);
        g_usb_ctx = nullptr;
        return PSPDL_TRANSPORT_ERROR;
    }

    // Detach kernel driver if active
    if (libusb_kernel_driver_active(g_usb_handle, 0) == 1)
    {
        r = libusb_detach_kernel_driver(g_usb_handle, 0);
        if (r < 0)
        {
            std::cerr << "[ERROR] libusb_detach_kernel_driver failed: " << r << std::endl;
            libusb_close(g_usb_handle);
            g_usb_handle = nullptr;
            libusb_exit(g_usb_ctx);
            g_usb_ctx = nullptr;
            return PSPDL_TRANSPORT_ERROR;
        }
    }

    r = libusb_claim_interface(g_usb_handle, 0);
    if (r < 0)
    {
        std::cerr << "[ERROR] libusb_claim_interface failed: " << r << std::endl;
        libusb_close(g_usb_handle);
        g_usb_handle = nullptr;
        libusb_exit(g_usb_ctx);
        g_usb_ctx = nullptr;
        return PSPDL_TRANSPORT_ERROR;
    }

    std::cout << "[INFO] USB Transport initialized and claimed interface 0 successfully." << std::endl;
    return PSPDL_TRANSPORT_OK;
}

PSPDL_TransportResult transport_shutdown(void)
{
    if (g_usb_handle != nullptr)
    {
        libusb_release_interface(g_usb_handle, 0);
        libusb_close(g_usb_handle);
        g_usb_handle = nullptr;
    }
    if (g_usb_ctx != nullptr)
    {
        libusb_exit(g_usb_ctx);
        g_usb_ctx = nullptr;
    }
    std::cout << "[INFO] USB Transport shut down successfully." << std::endl;
    return PSPDL_TRANSPORT_OK;
}

PSPDL_TransportResult transport_send(
    const void *data,
    size_t size)
{
    if (g_usb_handle == nullptr)
    {
        return PSPDL_TRANSPORT_ERROR;
    }

    int transferred = 0;
    int r = libusb_bulk_transfer(
        g_usb_handle,
        BULK_OUT_ENDPOINT,
        (unsigned char *)data,
        (int)size,
        &transferred,
        USB_TIMEOUT_MS);

    if (r < 0)
    {
        std::cerr << "[ERROR] libusb_bulk_transfer OUT failed: " 
                  << libusb_error_name(r) << " (" << r << ")" << std::endl;
        return PSPDL_TRANSPORT_ERROR;
    }

    if ((size_t)transferred != size)
    {
        std::cerr << "[WARN] Sent " << transferred << " bytes instead of expected " << size << std::endl;
        return PSPDL_TRANSPORT_ERROR;
    }

    return PSPDL_TRANSPORT_OK;
}

PSPDL_TransportResult transport_receive(
    void *buffer,
    size_t size,
    size_t *received)
{
    if (received != nullptr)
    {
        *received = 0;
    }

    if (g_usb_handle == nullptr)
    {
        return PSPDL_TRANSPORT_ERROR;
    }

    int transferred = 0;
    int r = libusb_bulk_transfer(
        g_usb_handle,
        BULK_IN_ENDPOINT,
        (unsigned char *)buffer,
        (int)size,
        &transferred,
        USB_TIMEOUT_MS);

    if (r < 0)
    {
        if (r == LIBUSB_ERROR_TIMEOUT)
        {
            if (received != nullptr)
            {
                *received = (size_t)transferred;
            }
            return PSPDL_TRANSPORT_OK;
        }
        std::cerr << "[ERROR] libusb_bulk_transfer IN failed: " 
                  << libusb_error_name(r) << " (" << r << ")" << std::endl;
        return PSPDL_TRANSPORT_ERROR;
    }

    if (received != nullptr)
    {
        *received = (size_t)transferred;
    }

    return PSPDL_TRANSPORT_OK;
}