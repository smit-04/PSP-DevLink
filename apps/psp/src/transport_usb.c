// PSP DevLink - USB Transport (User-Mode wrapper for Kernel PRX)
//
// This file runs in user-mode (EBOOT) but dynamically loads
// "pspdl_driver.prx" at runtime using a direct pointer bridge,
// avoiding any static import stubs to prevent launch corruption.

#include <protocol/transport.h>
#include <protocol/packet.h>
#include <protocol/version.h>
#include <protocol/payload.h>

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspmodulemgr.h>
#include <stdio.h>
#include <string.h>

#define PRX_PATH "ms0:/PSP/GAME/PSPDevLink/pspdl_driver.prx"

// Bridge structure matching driver
struct UsbBridge {
    int (*init)(void);
    int (*send)(const void *data, int size);
    int (*recv)(void *buf, int size, int *received);
    int (*shutdown)(void);
};

static struct UsbBridge g_bridge;
static SceUID g_driver_mod = -1;
static int g_mock_mode = 0;

static unsigned char g_mock_buffer[2048];
static size_t g_mock_buffer_size = 0;

extern char g_status_msg[128];

PSPDL_TransportResult transport_initialize(const char *launch_path)
{
    g_mock_mode = 0;
    g_mock_buffer_size = 0;
    memset(&g_bridge, 0, sizeof(g_bridge));

    char resolved_prx_path[256];
    if (launch_path != NULL && strlen(launch_path) > 0)
    {
        size_t len = strlen(launch_path);
        size_t last_slash = len;
        // Find the directory part (locate last slash)
        for (size_t i = len; i > 0; i--)
        {
            if (launch_path[i - 1] == '/' || launch_path[i - 1] == '\\')
            {
                last_slash = i - 1;
                break;
            }
        }

        if (last_slash < len && last_slash < (sizeof(resolved_prx_path) - 20))
        {
            strncpy(resolved_prx_path, launch_path, last_slash + 1);
            resolved_prx_path[last_slash + 1] = '\0';
            strcat(resolved_prx_path, "pspdl_driver.prx");
        }
        else
        {
            strncpy(resolved_prx_path, PRX_PATH, sizeof(resolved_prx_path) - 1);
            resolved_prx_path[sizeof(resolved_prx_path) - 1] = '\0';
        }
    }
    else
    {
        strncpy(resolved_prx_path, PRX_PATH, sizeof(resolved_prx_path) - 1);
        resolved_prx_path[sizeof(resolved_prx_path) - 1] = '\0';
    }

    // Load system USB modules from flash0 first. This ensures our driver's static imports
    // (from libpspusb_driver.a and libpspusbbus_driver.a) are successfully resolved by the OS loader.
    // If they are already loaded, these calls will return negative error codes (e.g. 0x80020139), which we safely ignore.
    // SceUID usb_mod = sceKernelLoadModule("flash0:/kd/usb.prx", 0, NULL);
    // if (usb_mod >= 0)
    // {
    //     int status = 0;
    //     sceKernelStartModule(usb_mod, 0, NULL, &status, NULL);
    // }

    // SceUID usbbd_mod = sceKernelLoadModule("flash0:/kd/usbbd.prx", 0, NULL);
    // if (usbbd_mod >= 0)
    // {
    //     int status = 0;
    //     sceKernelStartModule(usbbd_mod, 0, NULL, &status, NULL);
    // }

    // Load kernel PRX from memory stick at runtime
    g_driver_mod = sceKernelLoadModule(resolved_prx_path, 0, NULL);
    if (g_driver_mod < 0)
    {
        snprintf(g_status_msg, sizeof(g_status_msg), "PRX load fail (0x%08X). Mock Mode.", (unsigned int)g_driver_mod);
        g_mock_mode = 1;
        return PSPDL_TRANSPORT_OK;
    }

    // Start module and pass the address of our bridge structure in argp
    int status = 0;
    struct UsbBridge *p_bridge = &g_bridge;
    
    // Pass pointer to bridge structure in argp (unified memory space, kernel can read/write directly)
    int ret = sceKernelStartModule(g_driver_mod, sizeof(void *), &p_bridge, &status, NULL);
    if (ret != g_driver_mod)
    {
        snprintf(g_status_msg, sizeof(g_status_msg), "PRX start fail (0x%08X). Mock Mode.", (unsigned int)ret);
        sceKernelUnloadModule(g_driver_mod);
        g_driver_mod = -1;
        g_mock_mode = 1;
        return PSPDL_TRANSPORT_OK;
    }

    // Verify bridge structure was filled by driver
    if (g_bridge.init == NULL || g_bridge.send == NULL || g_bridge.recv == NULL || g_bridge.shutdown == NULL)
    {
        snprintf(g_status_msg, sizeof(g_status_msg), "PRX bridge binding failed. Mock Mode.");
        sceKernelStopModule(g_driver_mod, 0, NULL, &status, NULL);
        sceKernelUnloadModule(g_driver_mod);
        g_driver_mod = -1;
        g_mock_mode = 1;
        return PSPDL_TRANSPORT_OK;
    }

    // Call init through the bridge
    ret = g_bridge.init();
    if (ret < 0)
    {
        snprintf(g_status_msg, sizeof(g_status_msg), "USB Init failed (0x%08X). Mock Mode.", (unsigned int)ret);
        g_bridge.shutdown();
        sceKernelStopModule(g_driver_mod, 0, NULL, &status, NULL);
        sceKernelUnloadModule(g_driver_mod);
        g_driver_mod = -1;
        g_mock_mode = 1;
        return PSPDL_TRANSPORT_OK;
    }

    snprintf(g_status_msg, sizeof(g_status_msg), "PRX & USB Driver Active.");
    return PSPDL_TRANSPORT_OK;
}

PSPDL_TransportResult transport_shutdown(void)
{
    if (g_mock_mode)
    {
        return PSPDL_TRANSPORT_OK;
    }

    if (g_bridge.shutdown)
    {
        g_bridge.shutdown();
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

    if (g_bridge.send == NULL)
    {
        return PSPDL_TRANSPORT_ERROR;
    }

    int ret = g_bridge.send(data, (int)size);
    if (ret < 0)
    {
        return PSPDL_TRANSPORT_ERROR;
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

        // Inject HELLO packet after 3 seconds (300 ticks)
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

        // Inject HEARTBEAT packets every 2 seconds after connection
        if (mock_ticks > 300 && (mock_ticks - 300) % 200 == 0)
        {
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

    if (g_bridge.recv == NULL)
    {
        return PSPDL_TRANSPORT_ERROR;
    }

    int rx_bytes = 0;
    int ret = g_bridge.recv(buffer, (int)size, &rx_bytes);
    if (ret < 0)
    {
        return PSPDL_TRANSPORT_ERROR;
    }
    if (received != NULL)
    {
        *received = (size_t)rx_bytes;
    }
    return PSPDL_TRANSPORT_OK;
}