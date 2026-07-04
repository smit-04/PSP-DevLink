#ifndef PSPDL_MESSAGE_ROUTER_H
#define PSPDL_MESSAGE_ROUTER_H

#include "protocol/packet.h"
#include "protocol/payload.h"

// State stores
extern PSPDL_SystemStatsPayload g_current_stats;
extern PSPDL_GitStatusPayload g_current_git;
extern PSPDL_NotificationPayload g_current_notif;
extern uint8_t g_remote_command;
extern uint8_t g_new_notification_received;

// Dispatch function
int router_dispatch(const PSPDL_PacketHeader *header, const uint8_t *payload_buf);

#endif /* PSPDL_MESSAGE_ROUTER_H */
