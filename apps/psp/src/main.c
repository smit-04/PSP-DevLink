#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>

PSP_MODULE_INFO("PSPDevLink", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

int exit_callback(int arg1, int arg2, void *common)
{
    sceKernelExitGame();
    return 0;
}

int callback_thread(SceSize args, void *argp)
{
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

int setup_callbacks(void)
{
    int thid = sceKernelCreateThread(
        "update_thread",
        callback_thread,
        0x11,
        0xFA0,
        0,
        0);

    if (thid >= 0)
        sceKernelStartThread(thid, 0, 0);

    return thid;
}

#include <protocol/transport.h>
#include <protocol/packet.h>
#include <protocol/version.h>
#include <psppower.h>
#include "message_router.h"
#include "ui.h"

typedef enum
{
    STATE_DISCONNECTED,
    STATE_CONNECTED
} ConnectionState;

char g_status_msg[128] = "Initializing USB...";


int main(int argc, char *argv[])
{
    setup_callbacks();

    ui_init();

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    // Initialize transport backend using our launch path
    const char *launch_path = (argc > 0 && argv[0] != NULL) ? argv[0] : NULL;
    PSPDL_TransportResult trans_res = transport_initialize(launch_path);

    if (trans_res != PSPDL_TRANSPORT_OK)
    {
        // Fail-safe init if UI framework isn't initialized
        pspDebugScreenInit();
        pspDebugScreenPrintf("[FAIL] USB Transport Init Error: %d\n", trans_res);
        sceKernelDelayThread(5000000);
        sceKernelExitGame();
        return 0;
    }

    ConnectionState state = STATE_DISCONNECTED;
    uint32_t ticks_since_last_packet = 0;

    while (1)
    {
        SceCtrlData pad;
        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons & PSP_CTRL_START || g_remote_command == 1)
            break;

        // Local shortcuts to simulate remote command packets in Emulator Mode
        if ((pad.Buttons & PSP_CTRL_SELECT) && (pad.Buttons & PSP_CTRL_SQUARE))
        {
            g_remote_command = 1; // Simulate Exit command
        }
        if ((pad.Buttons & PSP_CTRL_SELECT) && (pad.Buttons & PSP_CTRL_TRIANGLE))
        {
            g_remote_command = 2; // Simulate Reboot command
        }

        if (g_remote_command == 2)
        {
            sceKernelDelayThread(100000);
            transport_shutdown();
            scePowerRequestColdReset(0);
            sceKernelExitGame();
            return 0;
        }

        // Handle SELECT button toggle history
        static int select_was_pressed = 0;
        if ((pad.Buttons & PSP_CTRL_SELECT) && 
            !(pad.Buttons & PSP_CTRL_SQUARE) && 
            !(pad.Buttons & PSP_CTRL_TRIANGLE))
        {
            if (!select_was_pressed)
            {
                ui_toggle_history();
                select_was_pressed = 1;
            }
        }
        else
        {
            select_was_pressed = 0;
        }

        // Handle CIRCLE button overlays / clears
        static int circle_was_pressed = 0;
        if (pad.Buttons & PSP_CTRL_CIRCLE)
        {
            if (!circle_was_pressed)
            {
                ui_handle_circle_press();
                circle_was_pressed = 1;
            }
        }
        else
        {
            circle_was_pressed = 0;
        }

        // Try to receive a packet header
        uint8_t rx_buf[PSPDL_PACKET_HEADER_SIZE];
        size_t received = 0;
        PSPDL_TransportResult read_res = transport_receive(rx_buf, sizeof(rx_buf), &received);

        if (read_res == PSPDL_TRANSPORT_OK && received >= PSPDL_PACKET_HEADER_SIZE)
        {
            PSPDL_PacketHeader rx_hdr;
            if (pspl_deserialize_header(rx_buf, received, &rx_hdr) == 0)
            {
                ticks_since_last_packet = 0; // reset watchdog

                // Read payload if present
                static uint8_t payload_buf[512];
                int payload_ok = 1;
                if (rx_hdr.payload_size > 0)
                {
                    if (rx_hdr.payload_size <= sizeof(payload_buf))
                    {
                        size_t pay_received = 0;
                        uint32_t pay_ticks = 0;
                        while (pay_received < rx_hdr.payload_size && pay_ticks < 100)
                        {
                            size_t chunk_rec = 0;
                            PSPDL_TransportResult pay_res = transport_receive(
                                payload_buf + pay_received,
                                rx_hdr.payload_size - pay_received,
                                &chunk_rec);
                            if (pay_res == PSPDL_TRANSPORT_OK)
                            {
                                pay_received += chunk_rec;
                            }
                            else
                            {
                                payload_ok = 0;
                                break;
                            }
                            if (chunk_rec == 0)
                            {
                                sceKernelDelayThread(1000);
                                pay_ticks++;
                            }
                        }
                        if (pay_received < rx_hdr.payload_size)
                        {
                            payload_ok = 0;
                        }
                    }
                    else
                    {
                        payload_ok = 0;
                    }
                }

                if (payload_ok)
                {
                    if (state == STATE_DISCONNECTED && rx_hdr.message_id == PSPDL_MESSAGE_HELLO)
                    {
                        // Validate protocol version
                        uint16_t expected_ver = (PSPDL_PROTOCOL_VERSION_MAJOR << 8) | PSPDL_PROTOCOL_VERSION_MINOR;
                        if (rx_hdr.protocol_version == expected_ver)
                        {
                            // Send HELLO packet response back to Host
                            PSPDL_PacketHeader tx_hdr;
                            tx_hdr.magic = PSPDL_PROTOCOL_MAGIC;
                            tx_hdr.protocol_version = expected_ver;
                            tx_hdr.message_id = PSPDL_MESSAGE_HELLO;
                            tx_hdr.payload_size = 0;

                            uint8_t tx_buf[PSPDL_PACKET_HEADER_SIZE];
                            pspl_serialize_header(&tx_hdr, tx_buf, sizeof(tx_buf));

                            if (transport_send(tx_buf, sizeof(tx_buf)) == PSPDL_TRANSPORT_OK)
                            {
                                state = STATE_CONNECTED;
                            }
                        }
                    }
                    else if (state == STATE_CONNECTED)
                    {
                        // Route packet payload to dispatcher
                        router_dispatch(&rx_hdr, payload_buf);
                    }
                }
            }
        }
        else if (read_res == PSPDL_TRANSPORT_ERROR)
        {
            if (state == STATE_CONNECTED)
            {
                state = STATE_DISCONNECTED;
            }
        }

        // Connection Watchdog timeout check
        if (state == STATE_CONNECTED)
        {
            ticks_since_last_packet++;
            if (ticks_since_last_packet > 500) // 5 seconds (500 * 10ms)
            {
                state = STATE_DISCONNECTED;
            }
        }

        // Add new notification to UI history cache if received
        if (g_new_notification_received)
        {
            ui_add_notification(&g_current_notif);
            g_new_notification_received = 0;
        }

        // Render dashboard GUI
        UIConnectionState ui_state = (state == STATE_CONNECTED) ? UI_CONN_CONNECTED : UI_CONN_DISCONNECTED;
        ui_render(ui_state, &g_current_stats, &g_current_git, &g_current_notif);

        // Delay 10ms per iteration (10,000 microseconds)
        sceKernelDelayThread(10000);
    }

    transport_shutdown();
    sceKernelExitGame();
    return 0;
}