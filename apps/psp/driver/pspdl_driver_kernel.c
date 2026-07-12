/*
 * PSPDevLink USB Kernel Driver
 * Based directly on the PSPLINK usbhostfs driver pattern (main.c by TyRaNiD).
 * The kernel USB bus driver is extremely strict; this implementation mirrors
 * the working reference exactly to avoid 0x80243002 (INVALID_PARAM) errors.
 */
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspusbbus.h>
#include <string.h>

#include "pspdl_ioctl.h"

PSP_MODULE_INFO("pspdl_driver", PSP_MODULE_KERNEL, 1, 0);

/* ----- USB Descriptor Constants ----- */
#define PSPDL_DRIVER_NAME   "PSPDevLinkDriver"

/* Endpoint numbers. EP0=control, EP1=bulk-in, EP2=bulk-out, EP3=async-in */
/* We must mirror the 4-endpoint structure of the reference driver exactly   */
#define EP_CTRL   0
#define EP_IN     1
#define EP_OUT    2
#define EP_ASYNC  3

/* USB event flag bits */
#define USB_EVENT_ATTACH  0x01
#define USB_EVENT_DETACH  0x02

static SceUID g_usb_event = -1;

/* ----- String Descriptor (tightly packed raw bytes, USB format) ----- */
/* The kernel parses these by jumping bLength bytes forward each time.   */
/* Using struct[] would add padding and corrupt the address arithmetic.  */
unsigned char g_strp[] = {
    /* String 0: Language ID = 0x0409 (English US) */
    0x04, 0x03, 0x09, 0x04
};

/* ----- Endpoint blocks (4 total, matching reference exactly) ----- */
struct UsbEndpoint g_endp[4] = {
    { EP_CTRL,  0, 0 },
    { EP_IN,    0, 0 },
    { EP_OUT,   0, 0 },
    { EP_ASYNC, 0, 0 },
};

/* ----- Interface ----- */
struct UsbInterface g_intp = {
    0xFFFFFFFF, 0, 1,
};

/* ----- Hi-Speed USB Descriptors ----- */
struct DeviceDescriptor g_devdesc_hi = {
    .bLength            = 18,
    .bDescriptorType    = 0x01,
    .bcdUSB             = 0x200,
    .bDeviceClass       = 0,
    .bDeviceSubClass    = 0,
    .bDeviceProtocol    = 0,
    .bMaxPacketSize     = 64,
    .idVendor           = 0,      /* PSP ignores VID/PID here, uses sceUsbActivate */
    .idProduct          = 0,
    .bcdDevice          = 0x100,
    .iManufacturer      = 0,
    .iProduct           = 0,
    .iSerialNumber      = 0,
    .bNumConfigurations = 1,
};

struct ConfigDescriptor g_confdesc_hi = {
    .bLength             = 9,
    .bDescriptorType     = 2,
    .wTotalLength        = (9 + 9 + (3 * 7)),
    .bNumInterfaces      = 1,
    .bConfigurationValue = 1,
    .iConfiguration      = 0,
    .bmAttributes        = 0xC0,
    .bMaxPower           = 0,
};

struct InterfaceDescriptor g_interdesc_hi = {
    .bLength            = 9,
    .bDescriptorType    = 4,
    .bInterfaceNumber   = 0,
    .bAlternateSetting  = 0,
    .bNumEndpoints      = 3,  /* 3 data endpoints (EP1, EP2, EP3) */
    .bInterfaceClass    = 0xFF,
    .bInterfaceSubClass = 0x01,
    .bInterfaceProtocol = 0xFF,
    .iInterface         = 0,
};

struct EndpointDescriptor g_endpdesc_hi[3] = {
    { .bLength=7, .bDescriptorType=5, .bEndpointAddress=0x81, .bmAttributes=2, .wMaxPacketSize=512, .bInterval=0 },
    { .bLength=7, .bDescriptorType=5, .bEndpointAddress=0x02, .bmAttributes=2, .wMaxPacketSize=512, .bInterval=0 },
    { .bLength=7, .bDescriptorType=5, .bEndpointAddress=0x83, .bmAttributes=2, .wMaxPacketSize=512, .bInterval=0 },
};

/* ----- Full-Speed USB Descriptors ----- */
struct DeviceDescriptor g_devdesc_full = {
    .bLength            = 18,
    .bDescriptorType    = 0x01,
    .bcdUSB             = 0x200,
    .bDeviceClass       = 0,
    .bDeviceSubClass    = 0,
    .bDeviceProtocol    = 0,
    .bMaxPacketSize     = 64,
    .idVendor           = 0,
    .idProduct          = 0,
    .bcdDevice          = 0x100,
    .iManufacturer      = 0,
    .iProduct           = 0,
    .iSerialNumber      = 0,
    .bNumConfigurations = 1,
};

struct ConfigDescriptor g_confdesc_full = {
    .bLength             = 9,
    .bDescriptorType     = 2,
    .wTotalLength        = (9 + 9 + (3 * 7)),
    .bNumInterfaces      = 1,
    .bConfigurationValue = 1,
    .iConfiguration      = 0,
    .bmAttributes        = 0xC0,
    .bMaxPower           = 0,
};

struct InterfaceDescriptor g_interdesc_full = {
    .bLength            = 9,
    .bDescriptorType    = 4,
    .bInterfaceNumber   = 0,
    .bAlternateSetting  = 0,
    .bNumEndpoints      = 3,
    .bInterfaceClass    = 0xFF,
    .bInterfaceSubClass = 0x01,
    .bInterfaceProtocol = 0xFF,
    .iInterface         = 0,
};

struct EndpointDescriptor g_endpdesc_full[3] = {
    { .bLength=7, .bDescriptorType=5, .bEndpointAddress=0x81, .bmAttributes=2, .wMaxPacketSize=64, .bInterval=0 },
    { .bLength=7, .bDescriptorType=5, .bEndpointAddress=0x02, .bmAttributes=2, .wMaxPacketSize=64, .bInterval=0 },
    { .bLength=7, .bDescriptorType=5, .bEndpointAddress=0x83, .bmAttributes=2, .wMaxPacketSize=64, .bInterval=0 },
};

/* ----- UsbData scratch buffers (hi-speed [0] and full-speed [1]) ----- */
/* MUST be 64-byte aligned for PSP USB DMA engine compatibility           */
struct UsbData g_usbdata[2] __attribute__((aligned(64)));

/* ----- USB Event callbacks ----- */
static int usb_request(int arg1, int arg2, struct DeviceRequest *req)
{
    return 0;
}

static int func28(int arg1, int arg2, int arg3)
{
    return 0;
}

static int usb_attach(int speed, void *arg2, void *arg3)
{
    /* Signal the event flag from interrupt/callback context safely */
    if (g_usb_event >= 0)
        sceKernelSetEventFlag(g_usb_event, USB_EVENT_ATTACH);
    return 0;
}

static int usb_detach(int arg1, int arg2, int arg3)
{
    if (g_usb_event >= 0)
        sceKernelSetEventFlag(g_usb_event, USB_EVENT_DETACH);
    return 0;
}

/* start_func: called by the kernel when sceUsbStart("PSPDevLinkDriver") is invoked.
 * This is where the devp_hi/confp_hi/devp/confp pointers MUST be filled in,
 * exactly as the reference usbhostfs driver does in its start_func. */
static int start_func(int size, void *p)
{
    /* Create event flag for safe attach/detach signaling from interrupt context */
    if (g_usb_event < 0)
        g_usb_event = sceKernelCreateEventFlag("PSPDLUsbEvent", 0x200, 0, NULL);

    memset(g_usbdata, 0, sizeof(g_usbdata));

    /* Hi-speed */
    memcpy(g_usbdata[0].devdesc, &g_devdesc_hi, sizeof(g_devdesc_hi));
    g_usbdata[0].config.pconfdesc     = &g_usbdata[0].confdesc;
    g_usbdata[0].config.pinterfaces   = &g_usbdata[0].interfaces;
    g_usbdata[0].config.pinterdesc    = &g_usbdata[0].interdesc;
    g_usbdata[0].config.pendp         = g_usbdata[0].endp;
    memcpy(g_usbdata[0].confdesc.desc, &g_confdesc_hi, sizeof(g_confdesc_hi));
    g_usbdata[0].confdesc.pinterfaces = &g_usbdata[0].interfaces;
    g_usbdata[0].interfaces.pinterdesc[0] = &g_usbdata[0].interdesc;
    g_usbdata[0].interfaces.intcount  = 1;
    memcpy(g_usbdata[0].interdesc.desc, &g_interdesc_hi, sizeof(g_interdesc_hi));
    g_usbdata[0].interdesc.pendp      = g_usbdata[0].endp;
    memcpy(g_usbdata[0].endp[0].desc, &g_endpdesc_hi[0], sizeof(g_endpdesc_hi[0]));
    memcpy(g_usbdata[0].endp[1].desc, &g_endpdesc_hi[1], sizeof(g_endpdesc_hi[1]));
    memcpy(g_usbdata[0].endp[2].desc, &g_endpdesc_hi[2], sizeof(g_endpdesc_hi[2]));

    /* Full-speed */
    memcpy(g_usbdata[1].devdesc, &g_devdesc_full, sizeof(g_devdesc_full));
    g_usbdata[1].config.pconfdesc     = &g_usbdata[1].confdesc;
    g_usbdata[1].config.pinterfaces   = &g_usbdata[1].interfaces;
    g_usbdata[1].config.pinterdesc    = &g_usbdata[1].interdesc;
    g_usbdata[1].config.pendp         = g_usbdata[1].endp;
    memcpy(g_usbdata[1].confdesc.desc, &g_confdesc_full, sizeof(g_confdesc_full));
    g_usbdata[1].confdesc.pinterfaces = &g_usbdata[1].interfaces;
    g_usbdata[1].interfaces.pinterdesc[0] = &g_usbdata[1].interdesc;
    g_usbdata[1].interfaces.intcount  = 1;
    memcpy(g_usbdata[1].interdesc.desc, &g_interdesc_full, sizeof(g_interdesc_full));
    g_usbdata[1].interdesc.pendp      = g_usbdata[1].endp;
    memcpy(g_usbdata[1].endp[0].desc, &g_endpdesc_full[0], sizeof(g_endpdesc_full[0]));
    memcpy(g_usbdata[1].endp[1].desc, &g_endpdesc_full[1], sizeof(g_endpdesc_full[1]));
    memcpy(g_usbdata[1].endp[2].desc, &g_endpdesc_full[2], sizeof(g_endpdesc_full[2]));

    /* CRITICAL: Set hi-speed and full-speed device/config pointers in driver struct.
     * The kernel requires these to be set HERE (inside start_func), not at registration time. */
    extern struct UsbDriver g_usb_driver;
    g_usb_driver.devp_hi  = g_usbdata[0].devdesc;
    g_usb_driver.confp_hi = &g_usbdata[0].config;
    g_usb_driver.devp     = g_usbdata[1].devdesc;
    g_usb_driver.confp    = &g_usbdata[1].config;

    /* CRITICAL: Flush ALL descriptor data from CPU cache to DRAM.
     * Without this, the USB DMA engine reads stale zeros and the kernel panics. */
    sceKernelDcacheWritebackRange(g_usbdata, sizeof(g_usbdata));

    return 0;
}

static int stop_func(int size, void *p)
{
    if (g_usb_event >= 0) {
        sceKernelDeleteEventFlag(g_usb_event);
        g_usb_event = -1;
    }
    return 0;
}

/* ----- USB Driver struct — initialized statically, matching reference layout exactly ----- */
/* NOTE: devp_hi/confp_hi/devp/confp are NULL here; they are set in start_func. */
struct UsbDriver g_usb_driver = {
    PSPDL_DRIVER_NAME,
    4,                                  /* endpoints count MUST be 4 */
    g_endp,
    &g_intp,
    NULL, NULL, NULL, NULL,             /* devp_hi, confp_hi, devp, confp — set in start_func */
    (struct StringDescriptor *) g_strp,
    usb_request, func28, usb_attach, usb_detach,
    0,
    start_func,
    stop_func,
    NULL
};

/* ----- IO Driver for user-mode communication via sceIoIoctl ----- */
static unsigned char g_tx_buf[8192] __attribute__((aligned(64)));
static unsigned char g_rx_buf[8192] __attribute__((aligned(64)));

static int pspdl_io_init(PspIoDrvArg *arg) { return 0; }
static int pspdl_io_exit(PspIoDrvArg *arg) { return 0; }
static int pspdl_io_open(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) { return 0; }
static int pspdl_io_close(PspIoDrvFileArg *arg) { return 0; }

static int pspdl_io_ioctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
    switch (cmd)
    {
        case PSPDL_IOCTL_SEND:
        {
            if (indata == NULL || inlen != sizeof(struct pspdl_send_args)) return -1;
            struct pspdl_send_args *a = (struct pspdl_send_args *)indata;
            int size = a->size;
            if (size > (int)sizeof(g_tx_buf)) size = sizeof(g_tx_buf);
            memcpy(g_tx_buf, a->data, size);
            /* CRITICAL: Flush CPU cache before DMA transfer to prevent kernel panic */
            sceKernelDcacheWritebackRange(g_tx_buf, size);
            struct UsbdDeviceReq req;
            memset(&req, 0, sizeof(req));
            req.endp = &g_endp[EP_IN];
            req.data = g_tx_buf;
            req.size = size;
            int ret = sceUsbbdReqSend(&req);
            return (ret == 0) ? (int)req.recvsize : ret;
        }
        case PSPDL_IOCTL_RECV:
        {
            if (indata == NULL || inlen != sizeof(struct pspdl_recv_args)) return -1;
            struct pspdl_recv_args *a = (struct pspdl_recv_args *)indata;
            int size = a->size;
            if (size > (int)sizeof(g_rx_buf)) size = sizeof(g_rx_buf);
            /* CRITICAL: Invalidate CPU cache before DMA writes to prevent stale reads */
            sceKernelDcacheInvalidateRange(g_rx_buf, size);
            struct UsbdDeviceReq req;
            memset(&req, 0, sizeof(req));
            req.endp = &g_endp[EP_OUT];
            req.data = g_rx_buf;
            req.size = size;
            int ret = sceUsbbdReqRecv(&req);
            if (ret == 0 && req.recvsize > 0) {
                memcpy(a->buf, g_rx_buf, req.recvsize);
                if (a->received) *(a->received) = (int)req.recvsize;
            }
            return (ret >= 0) ? 0 : ret;
        }
        default:
            return -1;
    }
}

static PspIoDrvFuncs g_io_funcs = {
    pspdl_io_init,
    pspdl_io_exit,
    pspdl_io_open,
    pspdl_io_close,
    NULL, NULL, NULL,
    pspdl_io_ioctl,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static PspIoDrv g_io_drv = { "pspdl", 0x10, 0x800, "PSPDevLink", &g_io_funcs };

int module_start(SceSize args, void *argp)
{
    /* Pre-initialize g_usbdata so devp_hi/devp are NEVER NULL.
     * The kernel may look at the driver struct before start_func is called.
     * A NULL devp causes an immediate kernel panic (null pointer dereference). */
    memset(g_usbdata, 0, sizeof(g_usbdata));
    memcpy(g_usbdata[0].devdesc, &g_devdesc_hi,   sizeof(g_devdesc_hi));
    memcpy(g_usbdata[1].devdesc, &g_devdesc_full,  sizeof(g_devdesc_full));
    g_usb_driver.devp_hi  = g_usbdata[0].devdesc;
    g_usb_driver.confp_hi = &g_usbdata[0].config;
    g_usb_driver.devp     = g_usbdata[1].devdesc;
    g_usb_driver.confp    = &g_usbdata[1].config;
    sceKernelDcacheWritebackRange(g_usbdata, sizeof(g_usbdata));

    sceIoAddDrv(&g_io_drv);
    sceUsbbdRegister(&g_usb_driver);
    return 0;
}

int module_stop(SceSize args, void *argp)
{
    sceUsbbdUnregister(&g_usb_driver);
    sceIoDelDrv("pspdl");
    return 0;
}
