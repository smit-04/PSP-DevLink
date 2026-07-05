// PSP DevLink - USB Transport (User-Mode)
//
// NOTE: On PSP, raw USB bulk transfers (sceUsbbd*) are kernel-mode only.
// This file implements the user-mode transport layer which:
//   1. Uses sceUsb* (user-mode) for USB state management
//   2. Falls back to mock/demo mode for testing EBOOT launch
//   3. Will use a helper kernel PRX for real bulk transfers in future
//
// This approach ensures the EBOOT.PBP (user-mode static ELF) loads correctly
// on PSP firmware 6.60 PRO-B9 CFW without any kernel PRX format issues.

#include <protocol/transport.h>
#include <protocol/packet.h>
#include <protocol/version.h>
#include <protocol/payload.h>

#include <pspkernel.h>
#include <pspusb.h>
#include <pspdebug.h>
#include <stdio.h>
#include <string.h>

#include "usb_identity.h"

static int g_mock_mode = 1;  // Always mock mode in user-mode build

static unsigned char g_mock_buffer[2048];
static size_t g_mock_buffer_size = 0;

PSPDL_TransportResult transport_initialize(void)
{
    g_mock_mode = 1;
    g_mock_buffer_size = 0;

    // Try to activate USB in high-level user mode just to show the USB icon
    // Full bulk transfer support requires a kernel PRX (future milestone)
    int ret = sceUsbActivate(PSPDL_USB_PRODUCT_ID);
    if (ret == 0)
    {
        pspDebugScreenPrintf("[INFO] USB Activated (user-mode, demo). PID=0x%04X\n", PSPDL_USB_PRODUCT_ID);
    }
    else
    {
        // Expected on first launch before USB is fully initialized
        pspDebugScreenPrintf("[INFO] USB Demo Mode (no cable or kernel driver needed).\n");
    }

    pspDebugScreenPrintf("[INFO] Running in Mock/Demo Mode on Hardware.\n");
    return PSPDL_TRANSPORT_OK;
}

PSPDL_TransportResult transport_shutdown(void)
{
    if (!g_mock_mode)
    {
        sceUsbDeactivate(PSPDL_USB_PRODUCT_ID);
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

    return PSPDL_TRANSPORT_ERROR;
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

        // Inject HEARTBEAT packets every 2 seconds (200 ticks) after connection
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

    return PSPDL_TRANSPORT_ERROR;
}