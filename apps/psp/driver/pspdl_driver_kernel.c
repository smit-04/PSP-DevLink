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
// Tightly packed string descriptors (LangID, Manufacturer, Product)
static unsigned char g_str_desc[] = {
    // String 0: Language IDs (0x0409 English US)
    4, 3, 0x09, 0x04,
    // String 1: Manufacturer ("PSP")
    8, 3, 'P', 0, 'S', 0, 'P', 0,
    // String 2: Product ("PSPDevLink")
    22, 3, 'P', 0, 'S', 0, 'P', 0, 'D', 0, 'e', 0, 'v', 0, 'L', 0, 'i', 0, 'n', 0, 'k', 0
};
static struct UsbDriver g_driver;

static struct DeviceDescriptor g_dev_desc = {
    18, 1, 0x0200, 0xFF, 0xFF, 0xFF, 64, PSPDL_USB_VID, PSPDL_USB_PID, 0x0100, 0, 1, 0, 1
};

static struct ConfigDescriptor g_conf_desc = {
    9, 2, 32, 1, 1, 0, 0xC0, 0
};

static struct InterfaceDescriptor g_int_desc = {
    9, 4, 0, 0, 2, 0xFF, 0xFF, 0xFF, 0
};

static struct EndpointDescriptor g_ep_in_desc = {
    7, 5, 0x80 | EP_IN, 2, 64, 0
};

static struct EndpointDescriptor g_ep_out_desc = {
    7, 5, EP_OUT, 2, 64, 0
};

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

    // Link everything up in the UsbData structure
    g_usb_data.config.pconfdesc = &g_conf_desc;
    g_usb_data.config.pinterfaces = &g_usb_data.interfaces;
    g_usb_data.config.pinterdesc = &g_usb_data.interdesc;
    g_usb_data.config.pendp = &g_usb_data.endp[0];
    
    g_usb_data.confdesc.pinterfaces = &g_usb_data.interfaces;
    g_usb_data.interfaces.pinterdesc[0] = &g_usb_data.interdesc;
    g_usb_data.interfaces.intcount = 1;
    g_usb_data.interdesc.pendp = &g_usb_data.endp[0];

    // Copy interface and endpoints to internal padded space
    memcpy(g_usb_data.interdesc.desc, &g_int_desc, sizeof(g_int_desc));
    memcpy(g_usb_data.endp[0].desc, &g_ep_in_desc, sizeof(g_ep_in_desc));
    memcpy(g_usb_data.endp[1].desc, &g_ep_out_desc, sizeof(g_ep_out_desc));

    // Configure g_driver
    g_driver.name = "PSPDevLinkDriver";
    g_driver.endpoints = 3;
    g_driver.endp = g_eps;
    g_driver.intp = &g_interface;
    g_driver.devp_hi = &g_dev_desc;
    g_driver.confp_hi = &g_usb_data.config;
    g_driver.devp = &g_dev_desc;
    g_driver.confp = &g_usb_data.config;
    g_driver.str = (struct StringDescriptor *)g_str_desc;
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
