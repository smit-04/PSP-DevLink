#include "protocol/transport.h"
#include "protocol/packet.h"
#include "protocol/version.h"
#include "protocol/payload.h"
#include "system_service.h"
#include "git_service.h"

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

    ConnectionState state = STATE_DISCONNECTED;
    auto last_hello_time = std::chrono::steady_clock::now();
    auto last_heartbeat_time = std::chrono::steady_clock::now();
    auto last_stats_time = std::chrono::steady_clock::now();
    auto last_git_time = std::chrono::steady_clock::now();

    SystemService system_service;
    GitService git_service;

    while (true)
    {
        if (state == STATE_DISCONNECTED)
        {
            std::cout << "[INFO] Searching for PSP DevLink device..." << std::endl;
            if (transport_initialize() == PSPDL_TRANSPORT_OK)
            {
                std::cout << "[INFO] PSP Connected. Initiating Handshake..." << std::endl;
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
                        state = STATE_CONNECTED;
                        last_heartbeat_time = std::chrono::steady_clock::now();
                        last_stats_time = std::chrono::steady_clock::now();
                        last_git_time = std::chrono::steady_clock::now();
                    }
                }
            }
            else if (res == PSPDL_TRANSPORT_ERROR)
            {
                std::cerr << "[ERROR] Transport read error during handshaking. Disconnecting..." << std::endl;
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

                std::cout << "[INFO] Sending HEARTBEAT packet..." << std::endl;
                if (transport_send(tx_buf, sizeof(tx_buf)) != PSPDL_TRANSPORT_OK)
                {
                    std::cerr << "[ERROR] Failed to send HEARTBEAT. Disconnecting..." << std::endl;
                    transport_shutdown();
                    state = STATE_DISCONNECTED;
                    continue;
                }
                last_heartbeat_time = now;
            }

            // Send SYSTEM STATS payload every 1 second
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_stats_time).count() >= 1)
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

                std::cout << "[INFO] Sending System Stats: CPU " << (int)stats.cpu_usage << "%, RAM " 
                          << (int)stats.ram_usage << "%, Temp " << (stats.cpu_temp / 10.0) << " C" << std::endl;

                if (transport_send(tx_buf, sizeof(tx_buf)) != PSPDL_TRANSPORT_OK)
                {
                    std::cerr << "[ERROR] Failed to send System Stats. Disconnecting..." << std::endl;
                    transport_shutdown();
                    state = STATE_DISCONNECTED;
                    continue;
                }
                last_stats_time = now;
            }

            // Send GIT STATUS payload every 3 seconds
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_git_time).count() >= 3)
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

                std::cout << "[INFO] Sending Git Status: branch " << git.branch_name << ", mod files " 
                          << git.modified_files << ", untracked " << git.untracked_files << std::endl;

                if (transport_send(tx_buf, sizeof(tx_buf)) != PSPDL_TRANSPORT_OK)
                {
                    std::cerr << "[ERROR] Failed to send Git Status. Disconnecting..." << std::endl;
                    transport_shutdown();
                    state = STATE_DISCONNECTED;
                    continue;
                }
                last_git_time = now;
            }

            // Receive check to keep the transport read channel clear
            uint8_t rx_buf[PSPDL_PACKET_HEADER_SIZE];
            size_t received = 0;
            PSPDL_TransportResult res = transport_receive(rx_buf, sizeof(rx_buf), &received);
            if (res == PSPDL_TRANSPORT_ERROR)
            {
                std::cerr << "[ERROR] Connection lost. Disconnecting..." << std::endl;
                transport_shutdown();
                state = STATE_DISCONNECTED;
                continue;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    transport_shutdown();
    return 0;
}