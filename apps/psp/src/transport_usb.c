#include <protocol/transport.h>
#include <protocol/packet.h>
#include <protocol/version.h>
#include <protocol/payload.h>

#include <pspkernel.h>
#include <pspusb.h>
#include <pspusbbus.h>
#include <pspdebug.h>
#include <pspmodulemgr.h>
#include <stdio.h>
#include <string.h>

#include "usb_identity.h"

// Dynamic USB function bindings to prevent load-time dependency errors (8002013C)
typedef int (*sceUsbbdRegister_t)(struct UsbDriver *driver);
typedef int (*sceUsbbdUnregister_t)(struct UsbDriver *driver);
typedef int (*sceUsbbdReqSend_t)(struct UsbdDeviceReq *req);
typedef int (*sceUsbbdReqRecv_t)(struct UsbdDeviceReq *req);
typedef int (*sceUsbbdReqCancelAll_t)(struct UsbEndpoint *endp);

typedef int (*sceUsbStart_t)(const char *driverName, int size, void *args);
typedef int (*sceUsbStop_t)(const char *driverName, int size, void *args);
typedef int (*sceUsbActivate_t)(int pid);
typedef int (*sceUsbDeactivate_t)(int pid);

static sceUsbbdRegister_t p_sceUsbbdRegister = NULL;
static sceUsbbdUnregister_t p_sceUsbbdUnregister = NULL;
static sceUsbbdReqSend_t p_sceUsbbdReqSend = NULL;
static sceUsbbdReqRecv_t p_sceUsbbdReqRecv = NULL;
static sceUsbbdReqCancelAll_t p_sceUsbbdReqCancelAll = NULL;

static sceUsbStart_t p_sceUsbStart = NULL;
static sceUsbStop_t p_sceUsbStop = NULL;
static sceUsbActivate_t p_sceUsbActivate = NULL;
static sceUsbDeactivate_t p_sceUsbDeactivate = NULL;

// Helper to load and start kernel modules dynamically
static int LoadStartModule(const char *path)
{
    u32 loadResult = sceKernelLoadModule(path, 0, NULL);
    if (loadResult & 0x80000000)
        return -1;
    
    int status = 0;
    u32 startResult = sceKernelStartModule(loadResult, 0, NULL, &status, NULL);
    if (loadResult != startResult)
        return -2;
    
    return 0;
}

static void* find_export_by_nid(const char* mod_name, u32 nid) {
    SceModule *mod = sceKernelFindModuleByName(mod_name);
    if (!mod) return NULL;

    unsigned char *ent_top = (unsigned char *)mod->ent_top;
    unsigned int ent_size = mod->ent_size;
    unsigned int i = 0;

    while (i < ent_size) {
        SceLibraryEntryTable *entry = (SceLibraryEntryTable *)(ent_top + i);
        if (entry->len == 0) break;

        unsigned int *entry_table = (unsigned int *)entry->entrytable;
        if (entry_table) {
            int func_count = entry->stubcount;
            int var_count = entry->vstubcount;
            for (int f = 0; f < func_count; f++) {
                if (entry_table[f] == nid) {
                    return (void *)entry_table[func_count + var_count + f];
                }
            }
        }
        i += entry->len * 4;
    }
    return NULL;
}

#define USB_TRANSEVENT_BULKIN_DONE  1
#define USB_TRANSEVENT_BULKOUT_DONE 2

static SceUID g_transevent = -1;
static struct UsbdDeviceReq g_bulkin_req;
static struct UsbdDeviceReq g_bulkout_req;
static int g_read_queued = 0;
static int g_mock_mode = 0;

static unsigned char g_mock_buffer[2048];
static size_t g_mock_buffer_size = 0;

// Align buffers to 64 bytes for DMA transfers
static unsigned char g_tx_buf[8192] __attribute__((aligned(64)));
static unsigned char g_rx_buf[8192] __attribute__((aligned(64)));

// HI-Speed device descriptor
static struct DeviceDescriptor devdesc_hi = 
{
    .bLength = 18, 
    .bDescriptorType = 0x01, 
    .bcdUSB = 0x200, 
    .bDeviceClass = 0, 
    .bDeviceSubClass = 0, 
    .bDeviceProtocol = 0, 
    .bMaxPacketSize = 64, 
    .idVendor = PSPDL_USB_VENDOR_ID, 
    .idProduct = PSPDL_USB_PRODUCT_ID, 
    .bcdDevice = 0x100, 
    .iManufacturer = 0, 
    .iProduct = 0, 
    .iSerialNumber = 0,
    .bNumConfigurations = 1
};

// Hi-Speed configuration descriptor
static struct ConfigDescriptor confdesc_hi = 
{
    .bLength = 9, 
    .bDescriptorType = 2, 
    .wTotalLength = (9 + 9 + 2 * 7), // 32
    .bNumInterfaces = 1, 
    .bConfigurationValue = 1, 
    .iConfiguration = 0, 
    .bmAttributes = 0xC0, 
    .bMaxPower = 0
};

// Hi-Speed interface descriptor
static struct InterfaceDescriptor interdesc_hi = 
{
    .bLength = 9, 
    .bDescriptorType = 4, 
    .bInterfaceNumber = 0, 
    .bAlternateSetting = 0, 
    .bNumEndpoints = 2, 
    .bInterfaceClass = 0xFF, 
    .bInterfaceSubClass = 0x01, 
    .bInterfaceProtocol = 0xFF, 
    .iInterface = 0
};

// Hi-Speed endpoint descriptors
static struct EndpointDescriptor endpdesc_hi[2] = 
{
    {
        .bLength = 7, 
        .bDescriptorType = 5, 
        .bEndpointAddress = 0x81, // Bulk IN (Device-to-Host)
        .bmAttributes = 2, 
        .wMaxPacketSize = 512, 
        .bInterval = 0 
    },
    {
        .bLength = 7, 
        .bDescriptorType = 5, 
        .bEndpointAddress = 0x02, // Bulk OUT (Host-to-Device)
        .bmAttributes = 2, 
        .wMaxPacketSize = 512, 
        .bInterval = 0 
    }
};

// Full-Speed device descriptor
static struct DeviceDescriptor devdesc_full = 
{
    .bLength = 18, 
    .bDescriptorType = 0x01, 
    .bcdUSB = 0x200, 
    .bDeviceClass = 0, 
    .bDeviceSubClass = 0, 
    .bDeviceProtocol = 0, 
    .bMaxPacketSize = 64, 
    .idVendor = PSPDL_USB_VENDOR_ID, 
    .idProduct = PSPDL_USB_PRODUCT_ID, 
    .bcdDevice = 0x100, 
    .iManufacturer = 0, 
    .iProduct = 0, 
    .iSerialNumber = 0,
    .bNumConfigurations = 1
};

// Full-Speed configuration descriptor
static struct ConfigDescriptor confdesc_full = 
{
    .bLength = 9, 
    .bDescriptorType = 2, 
    .wTotalLength = (9 + 9 + 2 * 7), 
    .bNumInterfaces = 1, 
    .bConfigurationValue = 1, 
    .iConfiguration = 0, 
    .bmAttributes = 0xC0, 
    .bMaxPower = 0
};

// Full-Speed interface descriptor
static struct InterfaceDescriptor interdesc_full = 
{
    .bLength = 9, 
    .bDescriptorType = 4, 
    .bInterfaceNumber = 0, 
    .bAlternateSetting = 0, 
    .bNumEndpoints = 2, 
    .bInterfaceClass = 0xFF, 
    .bInterfaceSubClass = 0x01, 
    .bInterfaceProtocol = 0xFF, 
    .iInterface = 0
};

// Full-Speed endpoint descriptors
static struct EndpointDescriptor endpdesc_full[2] = 
{
    {
        .bLength = 7, 
        .bDescriptorType = 5, 
        .bEndpointAddress = 0x81, 
        .bmAttributes = 2, 
        .wMaxPacketSize = 64, 
        .bInterval = 0 
    },
    {
        .bLength = 7, 
        .bDescriptorType = 5, 
        .bEndpointAddress = 0x02, 
        .bmAttributes = 2, 
        .wMaxPacketSize = 64, 
        .bInterval = 0 
    }
};

// String descriptor
static unsigned char strp[] = 
{
    0x08, 0x03, '<', 0, '>', 0, 0, 0
};

// Endpoint structures list
static struct UsbEndpoint g_endpoints[3] = 
{
    { 0, 0, 0 },
    { 1, 0, 0 },
    { 2, 0, 0 }
};

// Interface definition
static struct UsbInterface g_interface = 
{
    0xFFFFFFFF, 0, 1
};

// Descriptor table wrapper
static struct UsbData g_usb_data[2] __attribute__((aligned(64)));

// Driver callbacks
static int usb_recvctl(int arg1, int arg2, struct DeviceRequest *req)
{
    return 0;
}

static int usb_func28(int arg1, int arg2, int arg3)
{
    return 0;
}

static int usb_attach(int speed, void *arg2, void *arg3)
{
    return 0;
}

static int usb_detach(int arg1, int arg2, int arg3)
{
    g_read_queued = 0;
    return 0;
}

static int usb_start(int size, void *p)
{
    memset(g_usb_data, 0, sizeof(g_usb_data));

    // Hi-Speed Configuration setup
    memcpy(g_usb_data[0].devdesc, &devdesc_hi, sizeof(devdesc_hi));
    g_usb_data[0].config.pconfdesc = &g_usb_data[0].confdesc;
    g_usb_data[0].config.pinterfaces = &g_usb_data[0].interfaces;
    g_usb_data[0].config.pinterdesc = &g_usb_data[0].interdesc;
    g_usb_data[0].config.pendp = g_usb_data[0].endp;
    
    memcpy(g_usb_data[0].confdesc.desc, &confdesc_hi, sizeof(confdesc_hi));
    g_usb_data[0].confdesc.pinterfaces = &g_usb_data[0].interfaces;
    g_usb_data[0].interfaces.pinterdesc[0] = &g_usb_data[0].interdesc;
    g_usb_data[0].interfaces.intcount = 1;
    
    memcpy(g_usb_data[0].interdesc.desc, &interdesc_hi, sizeof(interdesc_hi));
    g_usb_data[0].interdesc.pendp = g_usb_data[0].endp;
    
    memcpy(g_usb_data[0].endp[0].desc, &endpdesc_hi[0], sizeof(endpdesc_hi[0]));
    memcpy(g_usb_data[0].endp[1].desc, &endpdesc_hi[1], sizeof(endpdesc_hi[1]));

    // Full-Speed Configuration setup
    memcpy(g_usb_data[1].devdesc, &devdesc_full, sizeof(devdesc_full));
    g_usb_data[1].config.pconfdesc = &g_usb_data[1].confdesc;
    g_usb_data[1].config.pinterfaces = &g_usb_data[1].interfaces;
    g_usb_data[1].config.pinterdesc = &g_usb_data[1].interdesc;
    g_usb_data[1].config.pendp = g_usb_data[1].endp;
    
    memcpy(g_usb_data[1].confdesc.desc, &confdesc_full, sizeof(confdesc_full));
    g_usb_data[1].confdesc.pinterfaces = &g_usb_data[1].interfaces;
    g_usb_data[1].interfaces.pinterdesc[0] = &g_usb_data[1].interdesc;
    g_usb_data[1].interfaces.intcount = 1;
    
    memcpy(g_usb_data[1].interdesc.desc, &interdesc_full, sizeof(interdesc_full));
    g_usb_data[1].interdesc.pendp = g_usb_data[1].endp;
    
    memcpy(g_usb_data[1].endp[0].desc, &endpdesc_full[0], sizeof(endpdesc_full[0]));
    memcpy(g_usb_data[1].endp[1].desc, &endpdesc_full[1], sizeof(endpdesc_full[1]));

    return 0;
}

static int usb_stop(int size, void *p)
{
    return 0;
}

#define DRIVER_NAME "PSPDL_USB_Driver"

static struct UsbDriver g_driver = 
{
    DRIVER_NAME,
    3,
    g_endpoints,
    &g_interface,
    NULL, NULL, NULL, NULL,
    (struct StringDescriptor *) strp,
    usb_recvctl, usb_func28, usb_attach, usb_detach,
    0, 
    usb_start,
    usb_stop,
    NULL
};

// Callbacks for when transfer requests finish
static int bulkin_req_done(struct UsbdDeviceReq *req, int arg2, int arg3)
{
    sceKernelSetEventFlag(g_transevent, USB_TRANSEVENT_BULKIN_DONE);
    return 0;
}

static int bulkout_req_done(struct UsbdDeviceReq *req, int arg2, int arg3)
{
    sceKernelSetEventFlag(g_transevent, USB_TRANSEVENT_BULKOUT_DONE);
    return 0;
}

PSPDL_TransportResult transport_initialize(void)
{
    g_read_queued = 0;
    g_mock_mode = 0;
    g_mock_buffer_size = 0;

    g_transevent = sceKernelCreateEventFlag("PSPDL_TransEvent", 0, 0, NULL);
    if (g_transevent < 0)
    {
        return PSPDL_TRANSPORT_ERROR;
    }

    // Load modules dynamically from flash0
    LoadStartModule("flash0:/kd/usb.prx");
    LoadStartModule("flash0:/kd/usbbd.prx");

    // Resolve functions thread-safely via export tables
    p_sceUsbStart = (sceUsbStart_t)find_export_by_nid("sceUSB_Service", 0xAE5DEB97);
    p_sceUsbStop = (sceUsbStop_t)find_export_by_nid("sceUSB_Service", 0x3C2007C0);
    p_sceUsbActivate = (sceUsbActivate_t)find_export_by_nid("sceUSB_Service", 0x586205D8);
    p_sceUsbDeactivate = (sceUsbDeactivate_t)find_export_by_nid("sceUSB_Service", 0x959E6A1E);

    p_sceUsbbdRegister = (sceUsbbdRegister_t)find_export_by_nid("sceUsbBusDriver", 0x5C2AE2C4);
    p_sceUsbbdUnregister = (sceUsbbdUnregister_t)find_export_by_nid("sceUsbBusDriver", 0xD94541CE);
    p_sceUsbbdReqSend = (sceUsbbdReqSend_t)find_export_by_nid("sceUsbBusDriver", 0xC5266858);
    p_sceUsbbdReqRecv = (sceUsbbdReqRecv_t)find_export_by_nid("sceUsbBusDriver", 0xF349CD03);
    p_sceUsbbdReqCancelAll = (sceUsbbdReqCancelAll_t)find_export_by_nid("sceUsbBusDriver", 0x83B367BF);

    // Fall back to emulator mode if dynamic binding failed (e.g. running on OFW/PPSSPP)
    if (!p_sceUsbbdRegister || !p_sceUsbStart || !p_sceUsbActivate || !p_sceUsbbdReqSend || !p_sceUsbbdReqRecv)
    {
        g_mock_mode = 1;
        pspDebugScreenPrintf("[WARN] Dynamic USB binding failed. Emulator Mode.\n");
        return PSPDL_TRANSPORT_OK;
    }

    int ret = p_sceUsbbdRegister(&g_driver);
    if (ret < 0)
    {
        g_mock_mode = 1;
        pspDebugScreenPrintf("[WARN] USB Register failed (0x%08X). Emulator Mode.\n", ret);
        return PSPDL_TRANSPORT_OK;
    }

    ret = p_sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
    if (ret != 0)
    {
        p_sceUsbbdUnregister(&g_driver);
        g_mock_mode = 1;
        pspDebugScreenPrintf("[WARN] USB Bus start failed (0x%08X). Emulator Mode.\n", ret);
        return PSPDL_TRANSPORT_OK;
    }

    ret = p_sceUsbStart(g_driver.name, 0, 0);
    if (ret != 0)
    {
        p_sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
        p_sceUsbbdUnregister(&g_driver);
        g_mock_mode = 1;
        pspDebugScreenPrintf("[WARN] Driver start failed (0x%08X). Emulator Mode.\n", ret);
        return PSPDL_TRANSPORT_OK;
    }

    ret = p_sceUsbActivate(PSPDL_USB_PRODUCT_ID);
    if (ret != 0)
    {
        p_sceUsbStop(g_driver.name, 0, 0);
        p_sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
        p_sceUsbbdUnregister(&g_driver);
        g_mock_mode = 1;
        pspDebugScreenPrintf("[WARN] USB Activate failed (0x%08X). Emulator Mode.\n", ret);
        return PSPDL_TRANSPORT_OK;
    }

    pspDebugScreenPrintf("[INFO] USB Transport Initialized Successfully (Hardware Mode).\n");
    return PSPDL_TRANSPORT_OK;
}

PSPDL_TransportResult transport_shutdown(void)
{
    if (g_mock_mode)
    {
        if (g_transevent >= 0)
        {
            sceKernelDeleteEventFlag(g_transevent);
            g_transevent = -1;
        }
        return PSPDL_TRANSPORT_OK;
    }

    if (p_sceUsbbdReqCancelAll)
    {
        p_sceUsbbdReqCancelAll(&g_endpoints[1]);
        p_sceUsbbdReqCancelAll(&g_endpoints[2]);
    }

    if (p_sceUsbDeactivate) p_sceUsbDeactivate(PSPDL_USB_PRODUCT_ID);
    if (p_sceUsbStop) p_sceUsbStop(g_driver.name, 0, 0);
    if (p_sceUsbStop) p_sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
    if (p_sceUsbbdUnregister) p_sceUsbbdUnregister(&g_driver);

    if (g_transevent >= 0)
    {
        sceKernelDeleteEventFlag(g_transevent);
        g_transevent = -1;
    }

    g_read_queued = 0;
    return PSPDL_TRANSPORT_OK;
}

PSPDL_TransportResult transport_send(
    const void *data,
    size_t size)
{
    if (g_mock_mode)
    {
        if (g_mock_buffer_size + size <= sizeof(g_mock_buffer))
        {
            memcpy(g_mock_buffer + g_mock_buffer_size, data, size);
            g_mock_buffer_size += size;
            return PSPDL_TRANSPORT_OK;
        }
        return PSPDL_TRANSPORT_ERROR;
    }

    int nextsize = 0;
    int writelen = 0;
    int ret;
    u32 result;

    // Check if data is already 64-byte aligned and doesn't cross cache lines
    if (((u32)data & 63) == 0)
    {
        while (writelen < (int)size)
        {
            nextsize = ((int)size - writelen) > (int)sizeof(g_tx_buf) ? (int)sizeof(g_tx_buf) : ((int)size - writelen);
            
            // Invalidate/Writeback cache line before DMA triggers
            sceKernelDcacheWritebackRange((void *)((u32)data + writelen), nextsize);

            memset(&g_bulkin_req, 0, sizeof(g_bulkin_req));
            g_bulkin_req.endp = &g_endpoints[1];
            g_bulkin_req.data = (void *)((u32)data + writelen);
            g_bulkin_req.size = nextsize;
            g_bulkin_req.func = bulkin_req_done;

            sceKernelClearEventFlag(g_transevent, ~USB_TRANSEVENT_BULKIN_DONE);
            
            ret = p_sceUsbbdReqSend(&g_bulkin_req);
            if (ret < 0)
            {
                return PSPDL_TRANSPORT_ERROR;
            }

            // Wait with a 100ms timeout
            SceUInt timeout = 100000;
            ret = sceKernelWaitEventFlag(g_transevent, USB_TRANSEVENT_BULKIN_DONE, PSP_EVENT_WAITOR | PSP_EVENT_WAITCLEAR, &result, &timeout);
            if (ret < 0)
            {
                p_sceUsbbdReqCancelAll(&g_endpoints[1]);
                return PSPDL_TRANSPORT_ERROR;
            }

            if (g_bulkin_req.retcode != 0 || g_bulkin_req.recvsize <= 0)
            {
                return PSPDL_TRANSPORT_ERROR;
            }

            writelen += g_bulkin_req.recvsize;
        }
    }
    else
    {
        while (writelen < (int)size)
        {
            nextsize = ((int)size - writelen) > (int)sizeof(g_tx_buf) ? (int)sizeof(g_tx_buf) : ((int)size - writelen);
            memcpy(g_tx_buf, (void *)((u32)data + writelen), nextsize);

            sceKernelDcacheWritebackRange(g_tx_buf, nextsize);

            memset(&g_bulkin_req, 0, sizeof(g_bulkin_req));
            g_bulkin_req.endp = &g_endpoints[1];
            g_bulkin_req.data = g_tx_buf;
            g_bulkin_req.size = nextsize;
            g_bulkin_req.func = bulkin_req_done;

            sceKernelClearEventFlag(g_transevent, ~USB_TRANSEVENT_BULKIN_DONE);
            
            ret = p_sceUsbbdReqSend(&g_bulkin_req);
            if (ret < 0)
            {
                return PSPDL_TRANSPORT_ERROR;
            }

            SceUInt timeout = 100000;
            ret = sceKernelWaitEventFlag(g_transevent, USB_TRANSEVENT_BULKIN_DONE, PSP_EVENT_WAITOR | PSP_EVENT_WAITCLEAR, &result, &timeout);
            if (ret < 0)
            {
                p_sceUsbbdReqCancelAll(&g_endpoints[1]);
                return PSPDL_TRANSPORT_ERROR;
            }

            if (g_bulkin_req.retcode != 0 || g_bulkin_req.recvsize <= 0)
            {
                return PSPDL_TRANSPORT_ERROR;
            }

            writelen += g_bulkin_req.recvsize;
        }
    }

    return PSPDL_TRANSPORT_OK;
}

PSPDL_TransportResult transport_receive(
    void *buffer,
    size_t size,
    size_t *received)
{
    if (received != NULL)
    {
        *received = 0;
    }

    if (g_mock_mode)
    {
        static uint32_t mock_ticks = 0;
        mock_ticks++;

        // Inject HELLO packet after 3 seconds (300 ticks, assuming 10ms loop delay)
        if (mock_ticks == 300)
        {
            PSPDL_PacketHeader hello_hdr;
            hello_hdr.magic = PSPDL_PROTOCOL_MAGIC;
            hello_hdr.protocol_version = (PSPDL_PROTOCOL_VERSION_MAJOR << 8) | PSPDL_PROTOCOL_VERSION_MINOR;
            hello_hdr.message_id = PSPDL_MESSAGE_HELLO;
            hello_hdr.payload_size = 0;

            uint8_t temp[PSPDL_PACKET_HEADER_SIZE];
            pspl_serialize_header(&hello_hdr, temp, sizeof(temp));

            if (g_mock_buffer_size + PSPDL_PACKET_HEADER_SIZE <= sizeof(g_mock_buffer))
            {
                memcpy(g_mock_buffer + g_mock_buffer_size, temp, PSPDL_PACKET_HEADER_SIZE);
                g_mock_buffer_size += PSPDL_PACKET_HEADER_SIZE;
            }
        }

        // Inject simulated System Stats payload after 6 seconds (600 ticks)
        if (mock_ticks == 600)
        {
            PSPDL_SystemStatsPayload stats;
            stats.cpu_usage = 35;
            stats.ram_usage = 55;
            stats.cpu_temp = 425; // 42.5C
            stats.ram_total = 17179869184ULL; // 16GB
            stats.ram_free = stats.ram_total * (100 - stats.ram_usage) / 100;

            PSPDL_PacketHeader hdr;
            hdr.magic = PSPDL_PROTOCOL_MAGIC;
            hdr.protocol_version = (PSPDL_PROTOCOL_VERSION_MAJOR << 8) | PSPDL_PROTOCOL_VERSION_MINOR;
            hdr.message_id = PSPDL_MESSAGE_SYSTEM_STATS;
            hdr.payload_size = PSPDL_PAYLOAD_SYSTEM_STATS_SIZE;

            uint8_t temp[PSPDL_PACKET_HEADER_SIZE + PSPDL_PAYLOAD_SYSTEM_STATS_SIZE];
            pspl_serialize_header(&hdr, temp, PSPDL_PACKET_HEADER_SIZE);
            pspl_serialize_system_stats(&stats, temp + PSPDL_PACKET_HEADER_SIZE, PSPDL_PAYLOAD_SYSTEM_STATS_SIZE);

            if (g_mock_buffer_size + sizeof(temp) <= sizeof(g_mock_buffer))
            {
                memcpy(g_mock_buffer + g_mock_buffer_size, temp, sizeof(temp));
                g_mock_buffer_size += sizeof(temp);
            }
        }

        // Inject simulated Git Status payload after 9 seconds (900 ticks)
        if (mock_ticks == 900)
        {
            PSPDL_GitStatusPayload git;
            git.modified_files = 5;
            git.untracked_files = 2;
            memset(git.branch_name, 0, sizeof(git.branch_name));
            // Warning-free assignment
            const char *branch = "dev-branch";
            for (int i = 0; i < 31 && branch[i] != '\0'; i++)
            {
                git.branch_name[i] = branch[i];
            }

            PSPDL_PacketHeader hdr;
            hdr.magic = PSPDL_PROTOCOL_MAGIC;
            hdr.protocol_version = (PSPDL_PROTOCOL_VERSION_MAJOR << 8) | PSPDL_PROTOCOL_VERSION_MINOR;
            hdr.message_id = PSPDL_MESSAGE_GIT_STATUS;
            hdr.payload_size = PSPDL_PAYLOAD_GIT_STATUS_SIZE;

            uint8_t temp[PSPDL_PACKET_HEADER_SIZE + PSPDL_PAYLOAD_GIT_STATUS_SIZE];
            pspl_serialize_header(&hdr, temp, PSPDL_PACKET_HEADER_SIZE);
            pspl_serialize_git_status(&git, temp + PSPDL_PACKET_HEADER_SIZE, PSPDL_PAYLOAD_GIT_STATUS_SIZE);

            if (g_mock_buffer_size + sizeof(temp) <= sizeof(g_mock_buffer))
            {
                memcpy(g_mock_buffer + g_mock_buffer_size, temp, sizeof(temp));
                g_mock_buffer_size += sizeof(temp);
            }
        }

        // Inject simulated Notification payload after 12 seconds (1200 ticks)
        if (mock_ticks == 1200)
        {
            PSPDL_NotificationPayload notif;
            memset(&notif, 0, sizeof(notif));
            
            const char *app = "Slack";
            const char *summary = "New message from @alex";
            const char *body = "Hey, did you finish compiling the PSP EBOOT?";
            
            for (int i = 0; i < 23 && app[i] != '\0'; i++) notif.app_name[i] = app[i];
            for (int i = 0; i < 51 && summary[i] != '\0'; i++) notif.summary[i] = summary[i];
            for (int i = 0; i < 51 && body[i] != '\0'; i++) notif.body[i] = body[i];

            PSPDL_PacketHeader hdr;
            hdr.magic = PSPDL_PROTOCOL_MAGIC;
            hdr.protocol_version = (PSPDL_PROTOCOL_VERSION_MAJOR << 8) | PSPDL_PROTOCOL_VERSION_MINOR;
            hdr.message_id = PSPDL_MESSAGE_NOTIFICATION;
            hdr.payload_size = PSPDL_PAYLOAD_NOTIFICATION_SIZE;

            uint8_t temp[PSPDL_PACKET_HEADER_SIZE + PSPDL_PAYLOAD_NOTIFICATION_SIZE];
            pspl_serialize_header(&hdr, temp, PSPDL_PACKET_HEADER_SIZE);
            pspl_serialize_notification(&notif, temp + PSPDL_PACKET_HEADER_SIZE, PSPDL_PAYLOAD_NOTIFICATION_SIZE);

            if (g_mock_buffer_size + sizeof(temp) <= sizeof(g_mock_buffer))
            {
                memcpy(g_mock_buffer + g_mock_buffer_size, temp, sizeof(temp));
                g_mock_buffer_size += sizeof(temp);
            }
        }

        // Inject HEARTBEAT packets every 2 seconds (200 ticks) after connection is established (ticks > 300)
        if (mock_ticks > 300 && (mock_ticks - 300) % 200 == 0)
        {
            // Inject HEARTBEAT only if mock_ticks is less than 1500 (simulate connection for 12 seconds, then stop to test watchdog!)
            if (mock_ticks < 1500)
            {
                PSPDL_PacketHeader hb_hdr;
                hb_hdr.magic = PSPDL_PROTOCOL_MAGIC;
                hb_hdr.protocol_version = (PSPDL_PROTOCOL_VERSION_MAJOR << 8) | PSPDL_PROTOCOL_VERSION_MINOR;
                hb_hdr.message_id = PSPDL_MESSAGE_HEARTBEAT;
                hb_hdr.payload_size = 0;

                uint8_t temp[PSPDL_PACKET_HEADER_SIZE];
                pspl_serialize_header(&hb_hdr, temp, sizeof(temp));

                if (g_mock_buffer_size + PSPDL_PACKET_HEADER_SIZE <= sizeof(g_mock_buffer))
                {
                    memcpy(g_mock_buffer + g_mock_buffer_size, temp, PSPDL_PACKET_HEADER_SIZE);
                    g_mock_buffer_size += PSPDL_PACKET_HEADER_SIZE;
                }
            }
        }

        if (g_mock_buffer_size > 0)
        {
            size_t read_size = size < g_mock_buffer_size ? size : g_mock_buffer_size;
            memcpy(buffer, g_mock_buffer, read_size);
            
            memmove(g_mock_buffer, g_mock_buffer + read_size, g_mock_buffer_size - read_size);
            g_mock_buffer_size -= read_size;

            if (received != NULL)
            {
                *received = read_size;
            }
        }
        return PSPDL_TRANSPORT_OK;
    }

    int nextsize = 0;
    int readlen = 0;
    int ret;
    u32 result;

    if (g_read_queued == 0)
    {
        nextsize = size > sizeof(g_rx_buf) ? sizeof(g_rx_buf) : size;

        sceKernelDcacheInvalidateRange(g_rx_buf, nextsize);

        memset(&g_bulkout_req, 0, sizeof(g_bulkout_req));
        g_bulkout_req.endp = &g_endpoints[2];
        g_bulkout_req.data = g_rx_buf;
        g_bulkout_req.size = nextsize;
        g_bulkout_req.func = bulkout_req_done;

        sceKernelClearEventFlag(g_transevent, ~USB_TRANSEVENT_BULKOUT_DONE);

        ret = p_sceUsbbdReqRecv(&g_bulkout_req);
        if (ret < 0)
        {
            return PSPDL_TRANSPORT_ERROR;
        }
        g_read_queued = 1;
    }

    // Poll the event flag (timeout = 0) to avoid blocking the main frame rendering loop
    SceUInt timeout = 0;
    ret = sceKernelWaitEventFlag(g_transevent, USB_TRANSEVENT_BULKOUT_DONE, PSP_EVENT_WAITOR | PSP_EVENT_WAITCLEAR, &result, &timeout);
    if (ret == 0)
    {
        g_read_queued = 0;
        if (g_bulkout_req.retcode == 0 && g_bulkout_req.recvsize > 0)
        {
            readlen = g_bulkout_req.recvsize < (int)size ? g_bulkout_req.recvsize : (int)size;
            memcpy(buffer, g_rx_buf, readlen);

            if (received != NULL)
            {
                *received = (size_t)readlen;
            }
            return PSPDL_TRANSPORT_OK;
        }
        else
        {
            return PSPDL_TRANSPORT_ERROR;
        }
    }
    else if (ret == (int)SCE_KERNEL_ERROR_WAIT_TIMEOUT)
    {
        // Request still pending, no bytes received yet. Returning OK with 0 bytes is non-blocking.
        if (received != NULL)
        {
            *received = 0;
        }
        return PSPDL_TRANSPORT_OK;
    }
    else
    {
        g_read_queued = 0;
        return PSPDL_TRANSPORT_ERROR;
    }
}