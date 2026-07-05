/**
 * PSP DevLink — Kernel USB Bulk Transfer Driver
 *
 * This is a kernel-mode PRX loaded at runtime by the user-mode EBOOT via
 * sceKernelLoadModule("ms0:/PSP/GAME/PSPDevLink/pspdl_driver.prx").
 *
 * It registers a custom USB bulk-transfer device (VID 0xFFFF, PID 0x0001)
 * and exposes 4 user-callable functions via its export table:
 *   pspdl_usb_init      — register driver, start USB, activate
 *   pspdl_usb_send      — bulk IN  (PSP → Host), blocking with timeout
 *   pspdl_usb_recv      — bulk OUT (Host → PSP), non-blocking poll
 *   pspdl_usb_shutdown  — deactivate and unregister
 */

#include <pspkernel.h>
#include <pspusb.h>
#include <pspusbbus.h>
#include <string.h>

PSP_MODULE_INFO("pspdl_driver", PSP_MODULE_KERNEL, 1, 0);
PSP_HEAP_SIZE_KB(64);

/* ---- USB identity ---- */
#define PSPDL_USB_VENDOR_ID   0xFFFF
#define PSPDL_USB_PRODUCT_ID  0x0001

/* ---- Event flag bits ---- */
#define EVT_BULKIN_DONE   (1 << 0)   /* send complete  */
#define EVT_BULKOUT_DONE  (1 << 1)   /* receive ready  */

/* ---- Transfer state ---- */
static SceUID g_event = -1;
static int    g_read_queued = 0;
static int    g_usb_running = 0;

/* Aligned DMA buffers (64-byte alignment required by PSP GE DMA) */
static unsigned char g_tx_buf[8192] __attribute__((aligned(64)));
static unsigned char g_rx_buf[8192] __attribute__((aligned(64)));

static struct UsbdDeviceReq g_bulkin_req;
static struct UsbdDeviceReq g_bulkout_req;

/* ---- USB descriptors ---- */

static struct DeviceDescriptor g_devdesc_hi = {
    .bLength            = 18,
    .bDescriptorType    = 0x01,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0,
    .bDeviceSubClass    = 0,
    .bDeviceProtocol    = 0,
    .bMaxPacketSize     = 64,
    .idVendor           = PSPDL_USB_VENDOR_ID,
    .idProduct          = PSPDL_USB_PRODUCT_ID,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0,
    .iProduct           = 0,
    .iSerialNumber      = 0,
    .bNumConfigurations = 1,
};

static struct DeviceDescriptor g_devdesc_full = {
    .bLength            = 18,
    .bDescriptorType    = 0x01,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0,
    .bDeviceSubClass    = 0,
    .bDeviceProtocol    = 0,
    .bMaxPacketSize     = 64,
    .idVendor           = PSPDL_USB_VENDOR_ID,
    .idProduct          = PSPDL_USB_PRODUCT_ID,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0,
    .iProduct           = 0,
    .iSerialNumber      = 0,
    .bNumConfigurations = 1,
};

static struct ConfigDescriptor g_confdesc_hi = {
    .bLength             = 9,
    .bDescriptorType     = 2,
    .wTotalLength        = 9 + 9 + 2 * 7,
    .bNumInterfaces      = 1,
    .bConfigurationValue = 1,
    .iConfiguration      = 0,
    .bmAttributes        = 0xC0,
    .bMaxPower           = 0,
};

static struct ConfigDescriptor g_confdesc_full = {
    .bLength             = 9,
    .bDescriptorType     = 2,
    .wTotalLength        = 9 + 9 + 2 * 7,
    .bNumInterfaces      = 1,
    .bConfigurationValue = 1,
    .iConfiguration      = 0,
    .bmAttributes        = 0xC0,
    .bMaxPower           = 0,
};

static struct InterfaceDescriptor g_interdesc_hi = {
    .bLength            = 9,
    .bDescriptorType    = 4,
    .bInterfaceNumber   = 0,
    .bAlternateSetting  = 0,
    .bNumEndpoints      = 2,
    .bInterfaceClass    = 0xFF,
    .bInterfaceSubClass = 0x01,
    .bInterfaceProtocol = 0xFF,
    .iInterface         = 0,
};

static struct InterfaceDescriptor g_interdesc_full = {
    .bLength            = 9,
    .bDescriptorType    = 4,
    .bInterfaceNumber   = 0,
    .bAlternateSetting  = 0,
    .bNumEndpoints      = 2,
    .bInterfaceClass    = 0xFF,
    .bInterfaceSubClass = 0x01,
    .bInterfaceProtocol = 0xFF,
    .iInterface         = 0,
};

static struct EndpointDescriptor g_endpdesc_hi[2] = {
    { .bLength = 7, .bDescriptorType = 5, .bEndpointAddress = 0x81,
      .bmAttributes = 2, .wMaxPacketSize = 512, .bInterval = 0 },
    { .bLength = 7, .bDescriptorType = 5, .bEndpointAddress = 0x02,
      .bmAttributes = 2, .wMaxPacketSize = 512, .bInterval = 0 },
};

static struct EndpointDescriptor g_endpdesc_full[2] = {
    { .bLength = 7, .bDescriptorType = 5, .bEndpointAddress = 0x81,
      .bmAttributes = 2, .wMaxPacketSize = 64, .bInterval = 0 },
    { .bLength = 7, .bDescriptorType = 5, .bEndpointAddress = 0x02,
      .bmAttributes = 2, .wMaxPacketSize = 64, .bInterval = 0 },
};

static unsigned char g_strp[] = { 0x08, 0x03, '<', 0, '>', 0, 0, 0 };

static struct UsbEndpoint g_endpoints[3] = {
    { 0, 0, 0 },
    { 1, 0, 0 },
    { 2, 0, 0 },
};

static struct UsbInterface g_interface = { 0xFFFFFFFF, 0, 1 };
static struct UsbData      g_usb_data[2] __attribute__((aligned(64)));

/* ---- Driver callbacks ---- */

static int usb_recvctl(int arg1, int arg2, struct DeviceRequest *req) { return 0; }
static int usb_func28(int arg1, int arg2, int arg3)                   { return 0; }
static int usb_attach(int speed, void *arg2, void *arg3)              { return 0; }
static int usb_detach(int arg1, int arg2, int arg3)
{
    g_read_queued = 0;
    return 0;
}

static int usb_start(int size, void *p)
{
    memset(g_usb_data, 0, sizeof(g_usb_data));

    /* Hi-Speed */
    memcpy(g_usb_data[0].devdesc, &g_devdesc_hi, sizeof(g_devdesc_hi));
    g_usb_data[0].config.pconfdesc   = &g_usb_data[0].confdesc;
    g_usb_data[0].config.pinterfaces = &g_usb_data[0].interfaces;
    g_usb_data[0].config.pinterdesc  = &g_usb_data[0].interdesc;
    g_usb_data[0].config.pendp       = g_usb_data[0].endp;
    memcpy(g_usb_data[0].confdesc.desc, &g_confdesc_hi, sizeof(g_confdesc_hi));
    g_usb_data[0].confdesc.pinterfaces          = &g_usb_data[0].interfaces;
    g_usb_data[0].interfaces.pinterdesc[0]      = &g_usb_data[0].interdesc;
    g_usb_data[0].interfaces.intcount           = 1;
    memcpy(g_usb_data[0].interdesc.desc, &g_interdesc_hi, sizeof(g_interdesc_hi));
    g_usb_data[0].interdesc.pendp = g_usb_data[0].endp;
    memcpy(g_usb_data[0].endp[0].desc, &g_endpdesc_hi[0], sizeof(g_endpdesc_hi[0]));
    memcpy(g_usb_data[0].endp[1].desc, &g_endpdesc_hi[1], sizeof(g_endpdesc_hi[1]));

    /* Full-Speed */
    memcpy(g_usb_data[1].devdesc, &g_devdesc_full, sizeof(g_devdesc_full));
    g_usb_data[1].config.pconfdesc   = &g_usb_data[1].confdesc;
    g_usb_data[1].config.pinterfaces = &g_usb_data[1].interfaces;
    g_usb_data[1].config.pinterdesc  = &g_usb_data[1].interdesc;
    g_usb_data[1].config.pendp       = g_usb_data[1].endp;
    memcpy(g_usb_data[1].confdesc.desc, &g_confdesc_full, sizeof(g_confdesc_full));
    g_usb_data[1].confdesc.pinterfaces          = &g_usb_data[1].interfaces;
    g_usb_data[1].interfaces.pinterdesc[0]      = &g_usb_data[1].interdesc;
    g_usb_data[1].interfaces.intcount           = 1;
    memcpy(g_usb_data[1].interdesc.desc, &g_interdesc_full, sizeof(g_interdesc_full));
    g_usb_data[1].interdesc.pendp = g_usb_data[1].endp;
    memcpy(g_usb_data[1].endp[0].desc, &g_endpdesc_full[0], sizeof(g_endpdesc_full[0]));
    memcpy(g_usb_data[1].endp[1].desc, &g_endpdesc_full[1], sizeof(g_endpdesc_full[1]));

    return 0;
}

static int usb_stop(int size, void *p) { return 0; }

#define DRIVER_NAME "PSPDevLink"

static struct UsbDriver g_driver = {
    DRIVER_NAME,
    3,
    g_endpoints,
    &g_interface,
    NULL, NULL, NULL, NULL,
    (struct StringDescriptor *)g_strp,
    usb_recvctl, usb_func28, usb_attach, usb_detach,
    0,
    usb_start,
    usb_stop,
    NULL,
};

/* ---- Transfer completion callbacks ---- */

static int bulkin_done(struct UsbdDeviceReq *req, int arg2, int arg3)
{
    sceKernelSetEventFlag(g_event, EVT_BULKIN_DONE);
    return 0;
}

static int bulkout_done(struct UsbdDeviceReq *req, int arg2, int arg3)
{
    sceKernelSetEventFlag(g_event, EVT_BULKOUT_DONE);
    return 0;
}

/* ======================================================================
 * Exported API — called from user-mode EBOOT via generated stubs
 * ====================================================================== */

int pspdl_usb_init(void)
{
    if (g_usb_running)
        return 0;

    g_read_queued = 0;

    g_event = sceKernelCreateEventFlag("pspdl_evt", 0, 0, NULL);
    if (g_event < 0)
        return g_event;

    int ret = sceUsbbdRegister(&g_driver);
    if (ret < 0) { sceKernelDeleteEventFlag(g_event); g_event = -1; return ret; }

    ret = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
    if (ret != 0) { sceUsbbdUnregister(&g_driver); sceKernelDeleteEventFlag(g_event); g_event = -1; return ret; }

    ret = sceUsbStart(DRIVER_NAME, 0, 0);
    if (ret != 0) { sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0); sceUsbbdUnregister(&g_driver); sceKernelDeleteEventFlag(g_event); g_event = -1; return ret; }

    ret = sceUsbActivate(PSPDL_USB_PRODUCT_ID);
    if (ret != 0) { sceUsbStop(DRIVER_NAME, 0, 0); sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0); sceUsbbdUnregister(&g_driver); sceKernelDeleteEventFlag(g_event); g_event = -1; return ret; }

    g_usb_running = 1;
    return 0;
}

int pspdl_usb_send(const void *data, int size)
{
    if (!g_usb_running || g_event < 0)
        return -1;

    /* Copy to aligned TX buffer */
    int chunk = size > (int)sizeof(g_tx_buf) ? (int)sizeof(g_tx_buf) : size;
    memcpy(g_tx_buf, data, chunk);
    sceKernelDcacheWritebackRange(g_tx_buf, chunk);

    memset(&g_bulkin_req, 0, sizeof(g_bulkin_req));
    g_bulkin_req.endp    = &g_endpoints[1];
    g_bulkin_req.data    = g_tx_buf;
    g_bulkin_req.size    = chunk;
    g_bulkin_req.func    = bulkin_done;

    sceKernelClearEventFlag(g_event, ~EVT_BULKIN_DONE);
    int ret = sceUsbbdReqSend(&g_bulkin_req);
    if (ret < 0) return ret;

    SceUInt timeout = 200000; /* 200 ms */
    u32 result = 0;
    ret = sceKernelWaitEventFlag(g_event, EVT_BULKIN_DONE,
                                  PSP_EVENT_WAITOR | PSP_EVENT_WAITCLEAR,
                                  &result, &timeout);
    if (ret < 0) { sceUsbbdReqCancelAll(&g_endpoints[1]); return ret; }
    if (g_bulkin_req.retcode != 0) return -1;

    return g_bulkin_req.recvsize;
}

int pspdl_usb_recv(void *buf, int size, int *received)
{
    if (!g_usb_running || g_event < 0)
        return -1;

    if (received) *received = 0;

    int chunk = size > (int)sizeof(g_rx_buf) ? (int)sizeof(g_rx_buf) : size;

    if (!g_read_queued)
    {
        sceKernelDcacheInvalidateRange(g_rx_buf, chunk);
        memset(&g_bulkout_req, 0, sizeof(g_bulkout_req));
        g_bulkout_req.endp = &g_endpoints[2];
        g_bulkout_req.data = g_rx_buf;
        g_bulkout_req.size = chunk;
        g_bulkout_req.func = bulkout_done;

        sceKernelClearEventFlag(g_event, ~EVT_BULKOUT_DONE);
        int ret = sceUsbbdReqRecv(&g_bulkout_req);
        if (ret < 0) return ret;
        g_read_queued = 1;
    }

    /* Non-blocking poll — timeout = 0 */
    SceUInt timeout = 0;
    u32 result = 0;
    int ret = sceKernelWaitEventFlag(g_event, EVT_BULKOUT_DONE,
                                      PSP_EVENT_WAITOR | PSP_EVENT_WAITCLEAR,
                                      &result, &timeout);
    if (ret == 0)
    {
        g_read_queued = 0;
        if (g_bulkout_req.retcode == 0 && g_bulkout_req.recvsize > 0)
        {
            int n = g_bulkout_req.recvsize < chunk ? g_bulkout_req.recvsize : chunk;
            memcpy(buf, g_rx_buf, n);
            if (received) *received = n;
            return 0;
        }
        return -1;
    }
    /* SCE_KERNEL_ERROR_WAIT_TIMEOUT — no data yet, not an error */
    return 0;
}

int pspdl_usb_shutdown(void)
{
    if (!g_usb_running)
        return 0;

    sceUsbbdReqCancelAll(&g_endpoints[1]);
    sceUsbbdReqCancelAll(&g_endpoints[2]);
    sceUsbDeactivate(PSPDL_USB_PRODUCT_ID);
    sceUsbStop(DRIVER_NAME, 0, 0);
    sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
    sceUsbbdUnregister(&g_driver);

    if (g_event >= 0) { sceKernelDeleteEventFlag(g_event); g_event = -1; }
    g_usb_running  = 0;
    g_read_queued  = 0;
    return 0;
}

/* Exported API functions only. The toolchain crt0_prx.o automatically implements module_start/stop. */

int main(int argc, char *argv[])
{
    return 0;
}
