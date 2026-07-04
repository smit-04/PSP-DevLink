#include "message_router.h"
#include <pspdebug.h>
#include <stddef.h>

PSPDL_SystemStatsPayload g_current_stats;
PSPDL_GitStatusPayload g_current_git;
PSPDL_NotificationPayload g_current_notif;

int router_dispatch(
    const PSPDL_PacketHeader *header,
    const uint8_t *payload_buf)
{
    if (header == NULL || payload_buf == NULL)
    {
        return -1;
    }

    if (header->message_id == PSPDL_MESSAGE_SYSTEM_STATS)
    {
        int ret = pspl_deserialize_system_stats(payload_buf, header->payload_size, &g_current_stats);
        if (ret == 0)
        {
            return 0;
        }
        else
        {
            pspDebugScreenPrintf("[ERR] Failed to deserialize System Stats: %d\n", ret);
            return -2;
        }
    }
    else if (header->message_id == PSPDL_MESSAGE_GIT_STATUS)
    {
        int ret = pspl_deserialize_git_status(payload_buf, header->payload_size, &g_current_git);
        if (ret == 0)
        {
            return 0;
        }
        else
        {
            pspDebugScreenPrintf("[ERR] Failed to deserialize Git Status: %d\n", ret);
            return -3;
        }
    }
    else if (header->message_id == PSPDL_MESSAGE_NOTIFICATION)
    {
        int ret = pspl_deserialize_notification(payload_buf, header->payload_size, &g_current_notif);
        if (ret == 0)
        {
            return 0;
        }
        else
        {
            pspDebugScreenPrintf("[ERR] Failed to deserialize Notification: %d\n", ret);
            return -4;
        }
    }

    return 0;
}
