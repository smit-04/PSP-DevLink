#include "protocol/transport.h"
#include "protocol/packet.h"
#include "protocol/version.h"
#include "protocol/payload.h"
#include "system_service.h"
#include "git_service.h"
#include "notification_service.h"
#include "config_service.h"
#include "tui_service.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

enum ConnectionState
{
    STATE_DISCONNECTED,
    STATE_HANDSHAKING,
    STATE_CONNECTED
};

int main()
{
    ConfigService config_service("config.ini");
    config_service.load();

    TuiService tui_service(config_service);
    tui_service.initialize();

    ConnectionState state = STATE_DISCONNECTED;
    auto last_hello_time = std::chrono::steady_clock::now();
    auto last_heartbeat_time = std::chrono::steady_clock::now();
    auto last_stats_time = std::chrono::steady_clock::now();
    auto last_git_time = std::chrono::steady_clock::now();
    auto last_notif_time = std::chrono::steady_clock::now();

    SystemService system_service;
    GitService git_service;
    NotificationService notification_service;

    while (true)
    {
        // Poll for TUI keyboard events
        char c = tui_service.check_input();
        if (c == 'q' || c == 'Q')
        {
            break;
        }
        else if ((c == 'x' || c == 'X') && state == STATE_CONNECTED)
        {
            // Exit to XMB Control packet
            PSPDL_ControlPayload ctrl = { 1 };
            PSPDL_PacketHeader hdr;
            hdr.magic = PSPDL_PROTOCOL_MAGIC;
            hdr.protocol_version = (PSPDL_PROTOCOL_VERSION_MAJOR << 8) | PSPDL_PROTOCOL_VERSION_MINOR;
            hdr.message_id = PSPDL_MESSAGE_CONTROL;
            hdr.payload_size = PSPDL_PAYLOAD_CONTROL_SIZE;

            uint8_t tx_buf[PSPDL_PACKET_HEADER_SIZE + PSPDL_PAYLOAD_CONTROL_SIZE];
            pspl_serialize_header(&hdr, tx_buf, PSPDL_PACKET_HEADER_SIZE);
            pspl_serialize_control(&ctrl, tx_buf + PSPDL_PACKET_HEADER_SIZE, PSPDL_PAYLOAD_CONTROL_SIZE);

            transport_send(tx_buf, sizeof(tx_buf));
        }
        else if ((c == 'r' || c == 'R') && state == STATE_CONNECTED)
        {
            // Reboot Console Control packet
            PSPDL_ControlPayload ctrl = { 2 };
            PSPDL_PacketHeader hdr;
            hdr.magic = PSPDL_PROTOCOL_MAGIC;
            hdr.protocol_version = (PSPDL_PROTOCOL_VERSION_MAJOR << 8) | PSPDL_PROTOCOL_VERSION_MINOR;
            hdr.message_id = PSPDL_MESSAGE_CONTROL;
            hdr.payload_size = PSPDL_PAYLOAD_CONTROL_SIZE;

            uint8_t tx_buf[PSPDL_PACKET_HEADER_SIZE + PSPDL_PAYLOAD_CONTROL_SIZE];
            pspl_serialize_header(&hdr, tx_buf, PSPDL_PACKET_HEADER_SIZE);
            pspl_serialize_control(&ctrl, tx_buf + PSPDL_PACKET_HEADER_SIZE, PSPDL_PAYLOAD_CONTROL_SIZE);

            transport_send(tx_buf, sizeof(tx_buf));
        }

        if (state == STATE_DISCONNECTED)
        {
            tui_service.render("SEARCHING FOR HOST...");
            if (transport_initialize() == PSPDL_TRANSPORT_OK)
            {
                state = STATE_HANDSHAKING;
                last_hello_time = std::chrono::steady_clock::now() - std::chrono::seconds(2);
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                continue;
            }
        }

        if (state == STATE_HANDSHAKING)
        {
            tui_service.render("HANDSHAKING...");
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_hello_time).count() >= 1)
            {
                // Send HELLO packet
                PSPDL_PacketHeader hello_hdr;
                hello_hdr.magic = PSPDL_PROTOCOL_MAGIC;
                hello_hdr.protocol_version = (PSPDL_PROTOCOL_VERSION_MAJOR << 8) | PSPDL_PROTOCOL_VERSION_MINOR;
                hello_hdr.message_id = PSPDL_MESSAGE_HELLO;
                hello_hdr.payload_size = 0;

                uint8_t tx_buf[PSPDL_PACKET_HEADER_SIZE];
                pspl_serialize_header(&hello_hdr, tx_buf, sizeof(tx_buf));

                if (transport_send(tx_buf, sizeof(tx_buf)) != PSPDL_TRANSPORT_OK)
                {
                    transport_shutdown();
                    state = STATE_DISCONNECTED;
                    continue;
                }
                last_hello_time = now;
            }

            // Check for HELLO response from PSP
            uint8_t rx_buf[PSPDL_PACKET_HEADER_SIZE];
            size_t received = 0;
            PSPDL_TransportResult res = transport_receive(rx_buf, sizeof(rx_buf), &received);

            if (res == PSPDL_TRANSPORT_OK && received >= PSPDL_PACKET_HEADER_SIZE)
            {
                PSPDL_PacketHeader rx_hdr;
                if (pspl_deserialize_header(rx_buf, received, &rx_hdr) == 0)
                {
                    if (rx_hdr.message_id == PSPDL_MESSAGE_HELLO)
                    {
                        state = STATE_CONNECTED;
                        last_heartbeat_time = std::chrono::steady_clock::now();
                        last_stats_time = std::chrono::steady_clock::now();
                        last_git_time = std::chrono::steady_clock::now();
                        last_notif_time = std::chrono::steady_clock::now();
                    }
                }
            }
            else if (res == PSPDL_TRANSPORT_ERROR)
            {
                transport_shutdown();
                state = STATE_DISCONNECTED;
                continue;
            }
        }

        if (state == STATE_CONNECTED)
        {
            tui_service.render("CONNECTED");
            auto now = std::chrono::steady_clock::now();

            // Send HEARTBEAT packet every 2 seconds
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_heartbeat_time).count() >= 2)
            {
                PSPDL_PacketHeader hb_hdr;
                hb_hdr.magic = PSPDL_PROTOCOL_MAGIC;
                hb_hdr.protocol_version = (PSPDL_PROTOCOL_VERSION_MAJOR << 8) | PSPDL_PROTOCOL_VERSION_MINOR;
                hb_hdr.message_id = PSPDL_MESSAGE_HEARTBEAT;
                hb_hdr.payload_size = 0;

                uint8_t tx_buf[PSPDL_PACKET_HEADER_SIZE];
                pspl_serialize_header(&hb_hdr, tx_buf, sizeof(tx_buf));

                if (transport_send(tx_buf, sizeof(tx_buf)) != PSPDL_TRANSPORT_OK)
                {
                    transport_shutdown();
                    state = STATE_DISCONNECTED;
                    continue;
                }
                last_heartbeat_time = now;
            }

            // Send SYSTEM STATS payload
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_stats_time).count() >= config_service.telemetry_interval_ms)
            {
                SystemMetrics metrics = system_service.get_metrics();

                PSPDL_SystemStatsPayload stats;
                stats.cpu_usage = metrics.cpu_usage;
                stats.ram_usage = metrics.ram_usage;
                stats.cpu_temp = metrics.cpu_temp;
                stats.ram_total = metrics.ram_total;
                stats.ram_free = metrics.ram_free;

                PSPDL_PacketHeader hdr;
                hdr.magic = PSPDL_PROTOCOL_MAGIC;
                hdr.protocol_version = (PSPDL_PROTOCOL_VERSION_MAJOR << 8) | PSPDL_PROTOCOL_VERSION_MINOR;
                hdr.message_id = PSPDL_MESSAGE_SYSTEM_STATS;
                hdr.payload_size = PSPDL_PAYLOAD_SYSTEM_STATS_SIZE;

                uint8_t tx_buf[PSPDL_PACKET_HEADER_SIZE + PSPDL_PAYLOAD_SYSTEM_STATS_SIZE];
                pspl_serialize_header(&hdr, tx_buf, PSPDL_PACKET_HEADER_SIZE);
                pspl_serialize_system_stats(&stats, tx_buf + PSPDL_PACKET_HEADER_SIZE, PSPDL_PAYLOAD_SYSTEM_STATS_SIZE);

                if (transport_send(tx_buf, sizeof(tx_buf)) != PSPDL_TRANSPORT_OK)
                {
                    transport_shutdown();
                    state = STATE_DISCONNECTED;
                    continue;
                }
                last_stats_time = now;
            }

            // Send GIT STATUS payload (if enabled)
            if (config_service.enable_git && 
                std::chrono::duration_cast<std::chrono::milliseconds>(now - last_git_time).count() >= config_service.git_interval_ms)
            {
                GitMetrics metrics = git_service.get_metrics();

                PSPDL_GitStatusPayload git;
                git.modified_files = metrics.modified_files;
                git.untracked_files = metrics.untracked_files;
                memset(git.branch_name, 0, sizeof(git.branch_name));
                strncpy(git.branch_name, metrics.branch_name.c_str(), sizeof(git.branch_name) - 1);

                PSPDL_PacketHeader hdr;
                hdr.magic = PSPDL_PROTOCOL_MAGIC;
                hdr.protocol_version = (PSPDL_PROTOCOL_VERSION_MAJOR << 8) | PSPDL_PROTOCOL_VERSION_MINOR;
                hdr.message_id = PSPDL_MESSAGE_GIT_STATUS;
                hdr.payload_size = PSPDL_PAYLOAD_GIT_STATUS_SIZE;

                uint8_t tx_buf[PSPDL_PACKET_HEADER_SIZE + PSPDL_PAYLOAD_GIT_STATUS_SIZE];
                pspl_serialize_header(&hdr, tx_buf, PSPDL_PACKET_HEADER_SIZE);
                pspl_serialize_git_status(&git, tx_buf + PSPDL_PACKET_HEADER_SIZE, PSPDL_PAYLOAD_GIT_STATUS_SIZE);

                if (transport_send(tx_buf, sizeof(tx_buf)) != PSPDL_TRANSPORT_OK)
                {
                    transport_shutdown();
                    state = STATE_DISCONNECTED;
                    continue;
                }
                last_git_time = now;
            }

            // Send GIT STATUS payload check if we should reset timer if disabled
            if (!config_service.enable_git)
            {
                last_git_time = now;
            }

            // Send NOTIFICATION payload (if enabled)
            if (config_service.enable_notif && 
                std::chrono::duration_cast<std::chrono::milliseconds>(now - last_notif_time).count() >= config_service.notif_interval_ms)
            {
                NotificationMetrics metrics = notification_service.get_latest();
                if (metrics.is_new)
                {
                    PSPDL_NotificationPayload notif;
                    memset(&notif, 0, sizeof(notif));
                    strncpy(notif.app_name, metrics.app_name.c_str(), sizeof(notif.app_name) - 1);
                    strncpy(notif.summary, metrics.summary.c_str(), sizeof(notif.summary) - 1);
                    strncpy(notif.body, metrics.body.c_str(), sizeof(notif.body) - 1);

                    PSPDL_PacketHeader hdr;
                    hdr.magic = PSPDL_PROTOCOL_MAGIC;
                    hdr.protocol_version = (PSPDL_PROTOCOL_VERSION_MAJOR << 8) | PSPDL_PROTOCOL_VERSION_MINOR;
                    hdr.message_id = PSPDL_MESSAGE_NOTIFICATION;
                    hdr.payload_size = PSPDL_PAYLOAD_NOTIFICATION_SIZE;

                    uint8_t tx_buf[PSPDL_PACKET_HEADER_SIZE + PSPDL_PAYLOAD_NOTIFICATION_SIZE];
                    pspl_serialize_header(&hdr, tx_buf, PSPDL_PACKET_HEADER_SIZE);
                    pspl_serialize_notification(&notif, tx_buf + PSPDL_PACKET_HEADER_SIZE, PSPDL_PAYLOAD_NOTIFICATION_SIZE);

                    if (transport_send(tx_buf, sizeof(tx_buf)) != PSPDL_TRANSPORT_OK)
                    {
                        transport_shutdown();
                        state = STATE_DISCONNECTED;
                        continue;
                    }
                }
                last_notif_time = now;
            }

            if (!config_service.enable_notif)
            {
                last_notif_time = now;
            }

            // Receive check to keep the transport read channel clear
            uint8_t rx_buf[PSPDL_PACKET_HEADER_SIZE];
            size_t received = 0;
            PSPDL_TransportResult res = transport_receive(rx_buf, sizeof(rx_buf), &received);
            if (res == PSPDL_TRANSPORT_ERROR)
            {
                transport_shutdown();
                state = STATE_DISCONNECTED;
                continue;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    tui_service.shutdown();
    transport_shutdown();
    return 0;
}