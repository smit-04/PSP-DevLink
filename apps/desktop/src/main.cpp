#include "protocol/transport.h"
#include "protocol/packet.h"
#include "protocol/version.h"

#include <iostream>
#include <thread>
#include <chrono>

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