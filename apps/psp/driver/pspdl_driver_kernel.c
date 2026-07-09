#include <pspkernel.h>
#include <pspdebug.h>
#include <pspusbbus.h>
#include <string.h>

#include "pspdl_ioctl.h"

// Kernel mode module
PSP_MODULE_INFO("pspdl_driver", PSP_MODULE_KERNEL, 1, 0);

// Hardware specific
#define PSPDL_USB_PID 0x011A
#define PSPDL_USB_VID 0x1234

// Endpoints
#define EP_IN  1
#define EP_OUT 2

static struct UsbEndpoint g_eps[3];
static struct UsbInterface g_interface;
static struct UsbInterfaces g_interfaces;
static struct StringDescriptor g_str_desc[3] = {
    { 4, 3, { 0x0409 } },
    { 24, 3, { 'P', 'S', 'P', ' ', 'D', 'e', 'v', 'e', 'l', 'o', 'p' } },
    { 22, 3, { 'P', 'S', 'P', 'D', 'e', 'v', 'L', 'i', 'n', 'k' } }
};
static struct UsbDriver g_driver;

static int g_is_initialized = 0;
static int g_usb_driver_registered = 0;
static int g_usb_driver_started = 0;

static unsigned char g_tx_buf[8192] __attribute__((aligned(64)));
static unsigned char g_rx_buf[8192] __attribute__((aligned(64)));

// Forward declarations of driver functions
static int pspdl_usb_init(void);
static int pspdl_usb_send(const void *data, int size);
static int pspdl_usb_recv(void *buf, int size);
static int pspdl_usb_shutdown(void);

// IO Driver Functions
static int pspdl_io_init(PspIoDrvArg *arg) { return 0; }
static int pspdl_io_exit(PspIoDrvArg *arg) { return 0; }
static int pspdl_io_open(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) { return 0; }
static int pspdl_io_close(PspIoDrvFileArg *arg) { return 0; }

static int pspdl_io_ioctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
    switch (cmd)
    {
        case PSPDL_IOCTL_INIT:
            return pspdl_usb_init();
            
        case PSPDL_IOCTL_SEND:
        {
            if (indata == NULL || inlen != sizeof(struct pspdl_send_args)) return -1;
            struct pspdl_send_args *args = (struct pspdl_send_args *)indata;
            return pspdl_usb_send(args->data, args->size);
        }
        
        case PSPDL_IOCTL_RECV:
        {
            if (indata == NULL || inlen != sizeof(struct pspdl_recv_args)) return -1;
            struct pspdl_recv_args *args = (struct pspdl_recv_args *)indata;
            int ret = pspdl_usb_recv(args->buf, args->size);
            if (args->received != NULL) {
                *(args->received) = ret;
            }
            return (ret >= 0) ? 0 : ret;
        }
            
        case PSPDL_IOCTL_SHUTDOWN:
            return pspdl_usb_shutdown();
            
        default:
            return -1;
    }
}

static PspIoDrvFuncs g_io_funcs = {
    pspdl_io_init,
    pspdl_io_exit,
    pspdl_io_open,
    pspdl_io_close,
    NULL, // read
    NULL, // write
    NULL, // lseek
    pspdl_io_ioctl,
    NULL, // remove
    NULL, // mkdir
    NULL, // rmdir
    NULL, // dopen
    NULL, // dclose
    NULL, // dread
    NULL, // getstat
    NULL, // chstat
    NULL, // rename
    NULL, // chdir
    NULL, // mount
    NULL, // umount
    NULL, // devctl
    NULL  // unk
};

static PspIoDrv g_io_drv = { "pspdl", 0x10, 0x800, "PSPDevLink", &g_io_funcs };

// USB Event Handlers
static int usb_attach(int speed, void *arg2, void *arg3)
{
    return 0;
}

static int usb_detach(int arg1, int arg2, int arg3)
{
    return 0;
}

static int usb_start_func(int size, void *args)
{
    return 0;
}

static int usb_stop_func(int size, void *args)
{
    return 0;
}

static int usb_recvctl(int arg1, int arg2, struct DeviceRequest *req)
{
    return 0;
}

static int usb_func28(int arg1, int arg2, int arg3)
{
    return 0;
}

// USB Descriptors and padding structure (Required by sceUsbBusDriver)
static struct UsbData g_usb_data __attribute__((aligned(64)));

static int pspdl_usb_init(void)
{
    if (g_is_initialized) return 0;

    memset(&g_eps, 0, sizeof(g_eps));
    memset(&g_interface, 0, sizeof(g_interface));
    memset(&g_interfaces, 0, sizeof(g_interfaces));
    memset(&g_driver, 0, sizeof(g_driver));
    memset(&g_usb_data, 0, sizeof(g_usb_data));

    // Endpoints (Must include EP0 control endpoint)
    g_eps[0].endpnum = 0;
    g_eps[1].endpnum = EP_IN;
    g_eps[2].endpnum = EP_OUT;

    g_interface.expect_interface = -1;
    g_interface.num_interface = 1;
    struct DeviceDescriptor *dev = (struct DeviceDescriptor *)g_usb_data.devdesc;
    dev->bLength = 18;
    dev->bDescriptorType = 1; // Device
    dev->bcdUSB = 0x0200; // USB 2.0
    dev->bDeviceClass = 0xFF; // Vendor specific
    dev->bDeviceSubClass = 0xFF;
    dev->bDeviceProtocol = 0xFF;
    dev->bMaxPacketSize = 64;
    dev->idVendor = PSPDL_USB_VID;
    dev->idProduct = PSPDL_USB_PID;
    dev->bcdDevice = 0x0100;
    dev->iManufacturer = 0;
    dev->iProduct = 1;
    dev->iSerialNumber = 0;
    dev->bNumConfigurations = 1;

    // Build the Configuration Descriptor
    struct ConfigDescriptor *conf = (struct ConfigDescriptor *)g_usb_data.confdesc.desc;
    conf->bLength = 9;
    conf->bDescriptorType = 2; // Configuration
    conf->wTotalLength = 9 + 9 + 7 + 7; // Config(9) + Interface(9) + 2*Endpoint(7)
    conf->bNumInterfaces = 1;
    conf->bConfigurationValue = 1;
    conf->iConfiguration = 0;
    conf->bmAttributes = 0xC0; // Self-powered
    conf->bMaxPower = 0;

    // Build the Interface Descriptor
    struct InterfaceDescriptor *inf = (struct InterfaceDescriptor *)g_usb_data.interdesc.desc;
    inf->bLength = 9;
    inf->bDescriptorType = 4; // Interface
    inf->bInterfaceNumber = 0;
    inf->bAlternateSetting = 0;
    inf->bNumEndpoints = 2;
    inf->bInterfaceClass = 0xFF;
    inf->bInterfaceSubClass = 0xFF;
    inf->bInterfaceProtocol = 0xFF;
    inf->iInterface = 0;

    // Build Endpoint Descriptors
    struct EndpointDescriptor *ep_in = (struct EndpointDescriptor *)g_usb_data.endp[0].desc;
    ep_in->bLength = 7;
    ep_in->bDescriptorType = 5; // Endpoint
    ep_in->bEndpointAddress = 0x80 | EP_IN; // IN
    ep_in->bmAttributes = 2; // Bulk
    ep_in->wMaxPacketSize = 64;
    ep_in->bInterval = 0;

    struct EndpointDescriptor *ep_out = (struct EndpointDescriptor *)g_usb_data.endp[1].desc;
    ep_out->bLength = 7;
    ep_out->bDescriptorType = 5; // Endpoint
    ep_out->bEndpointAddress = EP_OUT; // OUT
    ep_out->bmAttributes = 2; // Bulk
    ep_out->wMaxPacketSize = 64;
    ep_out->bInterval = 0;

    // Link everything up in the UsbData structure
    g_usb_data.config.pconfdesc = conf;
    g_usb_data.config.pinterfaces = &g_usb_data.interfaces;
    g_usb_data.config.pinterdesc = &g_usb_data.interdesc;
    g_usb_data.config.pendp = &g_usb_data.endp[0];
    
    g_usb_data.confdesc.pinterfaces = &g_usb_data.interfaces;
    g_usb_data.interfaces.pinterdesc[0] = &g_usb_data.interdesc;
    g_usb_data.interfaces.intcount = 1;
    g_usb_data.interdesc.pendp = &g_usb_data.endp[0];

    // Configure g_driver
    g_driver.name = "PSPDevLinkDriver";
    g_driver.endpoints = 3;
    g_driver.endp = g_eps;
    g_driver.intp = &g_interface;
    g_driver.devp_hi = dev;
    g_driver.confp_hi = &g_usb_data.config;
    g_driver.devp = dev;
    g_driver.confp = &g_usb_data.config;
    g_driver.str = &g_str_desc[0];
    g_driver.recvctl = usb_recvctl;
    g_driver.func28 = usb_func28;
    g_driver.attach = usb_attach;
    g_driver.detach = usb_detach;
    g_driver.start_func = usb_start_func;
    g_driver.stop_func = usb_stop_func;

    int ret = sceUsbbdRegister(&g_driver);
    if (ret < 0) return ret;
    g_usb_driver_registered = 1;

    g_is_initialized = 1;
    return 0;
}

static int pspdl_usb_send(const void *data, int size)
{
    if (!g_is_initialized) return -1;
    if (size > sizeof(g_tx_buf)) size = sizeof(g_tx_buf);
    
    memcpy(g_tx_buf, data, size);
    
    struct UsbdDeviceReq req;
    memset(&req, 0, sizeof(req));
    req.endp = &g_eps[1]; // EP_IN
    req.data = g_tx_buf;
    req.size = size;

    int ret = sceUsbbdReqSend(&req);
    return (ret == 0) ? req.recvsize : ret;
}

static int pspdl_usb_recv(void *buf, int size)
{
    if (!g_is_initialized) return -1;
    if (size > sizeof(g_rx_buf)) size = sizeof(g_rx_buf);
    
    struct UsbdDeviceReq req;
    memset(&req, 0, sizeof(req));
    req.endp = &g_eps[2]; // EP_OUT
    req.data = g_rx_buf;
    req.size = size;

    int ret = sceUsbbdReqRecv(&req);
    if (ret == 0 && req.recvsize > 0)
    {
        memcpy(buf, g_rx_buf, req.recvsize);
        return req.recvsize;
    }
    return ret;
}

static int pspdl_usb_shutdown(void)
{
    if (g_is_initialized)
    {
        if (g_usb_driver_registered)
        {
            // Safely cancel pending USB operations and clear FIFOs to prevent kernel panics
            sceUsbbdReqCancelAll(&g_eps[1]);
            sceUsbbdReqCancelAll(&g_eps[2]);
            sceUsbbdClearFIFO(&g_eps[1]);
            sceUsbbdClearFIFO(&g_eps[2]);
            
            sceUsbbdUnregister(&g_driver);
            g_usb_driver_registered = 0;
        }
        g_is_initialized = 0;
    }
    return 0;
}

int module_start(SceSize args, void *argp)
{
    // Register the custom IO device "pspdl0:"
    sceIoAddDrv(&g_io_drv);
    return 0;
}

int module_stop(SceSize args, void *argp)
{
    pspdl_usb_shutdown();
    sceIoDelDrv("pspdl");
    return 0;
}
