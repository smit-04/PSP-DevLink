#include "protocol/transport.h"
#include "protocol/packet.h"
#include "protocol/version.h"
#include "protocol/payload.h"
#include "system_service.h"
#include "git_service.h"
#include "notification_service.h"
#include "config_service.h"
#include "http_server.h"

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
    std::cout << "=========================================\n";
    std::cout << "     PSP DevLink Desktop Companion       \n";
    std::cout << "=========================================\n\n";

    ConfigService config_service("config.ini");
    config_service.load();

    HttpServer http_server(config_service);
    if (!http_server.start(8080))
    {
        std::cerr << "[ERROR] Failed to start HTTP dashboard server. Exiting..." << std::endl;
        return 1;
    }

    std::cout << "[INFO] Web Dashboard started successfully!" << std::endl;
    std::cout << "[INFO] Open: http://localhost:8080 in your Windows web browser." << std::endl;
    std::cout << "[INFO] Keep this terminal open to maintain the USB connection.\n" << std::endl;

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
        // Poll for Web GUI Remote control commands
        if (state == STATE_CONNECTED)
        {
            if (http_server.check_and_clear_exit())
            {
                http_server.add_log("Sending remote Exit to XMB command...", "info");
                std::cout << "[INFO] Sending remote Exit to XMB command..." << std::endl;
                
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
            else if (http_server.check_and_clear_reboot())
            {
                http_server.add_log("Sending remote Reboot Console command...", "info");
                std::cout << "[INFO] Sending remote Reboot Console command..." << std::endl;
                
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
        }

        if (state == STATE_DISCONNECTED)
        {
            http_server.update_connection_state("DISCONNECTED");
            std::cout << "[INFO] Searching for PSP DevLink device..." << std::endl;
            
            if (transport_initialize(nullptr) == PSPDL_TRANSPORT_OK)

            {
                std::cout << "[INFO] PSP Connected. Initiating Handshake..." << std::endl;
                http_server.update_connection_state("HANDSHAKING");
                http_server.add_log("PSP Connected. Handshaking...", "info");
                
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

                std::cout << "[INFO] Sending HELLO packet to PSP..." << std::endl;
                if (transport_send(tx_buf, sizeof(tx_buf)) != PSPDL_TRANSPORT_OK)
                {
                    std::cerr << "[ERROR] Failed to send HELLO packet. Disconnecting..." << std::endl;
                    http_server.add_log("Failed to send HELLO packet. Disconnecting...", "error");
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
                        std::cout << "[OK] Received HELLO response from PSP (Version: " 
                                  << (rx_hdr.protocol_version >> 8) << "." 
                                  << (rx_hdr.protocol_version & 0xFF) << "). Handshake Complete!" << std::endl;
                        
                        http_server.update_connection_state("CONNECTED");
                        http_server.add_log("Handshake Complete. Connected successfully!", "success");

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
                std::cerr << "[ERROR] Transport read error during handshaking. Disconnecting..." << std::endl;
                http_server.add_log("Transport error during handshaking. Disconnecting...", "error");
                transport_shutdown();
                state = STATE_DISCONNECTED;
                continue;
            }
        }

        if (state == STATE_CONNECTED)
        {
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
                    std::cerr << "[ERROR] Failed to send HEARTBEAT. Disconnecting..." << std::endl;
                    http_server.add_log("Connection lost. Disconnecting...", "error");
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

                // Update http status server cache thread-safely
                SharedTelemetry shared_tel;
                shared_tel.cpu_usage = metrics.cpu_usage;
                shared_tel.ram_usage = metrics.ram_usage;
                shared_tel.cpu_temp = metrics.cpu_temp;
                shared_tel.ram_total = metrics.ram_total;
                shared_tel.ram_free = metrics.ram_free;
                http_server.update_telemetry(shared_tel);

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
                    std::cerr << "[ERROR] Failed to send System Stats. Disconnecting..." << std::endl;
                    http_server.add_log("Connection lost. Disconnecting...", "error");
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

                // Update http status server cache thread-safely
                SharedGit shared_git;
                shared_git.branch = metrics.branch_name;
                shared_git.modified = metrics.modified_files;
                shared_git.untracked = metrics.untracked_files;
                http_server.update_git(shared_git);

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
                    std::cerr << "[ERROR] Failed to send Git Status. Disconnecting..." << std::endl;
                    http_server.add_log("Connection lost. Disconnecting...", "error");
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
                    http_server.add_log("Notification captured: [" + metrics.app_name + "] " + metrics.summary, "info");

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
                        std::cerr << "[ERROR] Failed to send Notification. Disconnecting..." << std::endl;
                        http_server.add_log("Connection lost. Disconnecting...", "error");
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
                std::cerr << "[ERROR] Connection lost. Disconnecting..." << std::endl;
                http_server.add_log("Connection lost. Disconnecting...", "error");
                transport_shutdown();
                state = STATE_DISCONNECTED;
                continue;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    http_server.stop();
    transport_shutdown();
    return 0;
}