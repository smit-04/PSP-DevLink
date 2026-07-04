#ifndef PSPDL_UI_H
#define PSPDL_UI_H

#include "protocol/payload.h"

// Connection states matching main application connection state
typedef enum
{
    UI_CONN_DISCONNECTED,
    UI_CONN_CONNECTED
} UIConnectionState;

void ui_init(void);
void ui_render(
    UIConnectionState conn_state,
    const PSPDL_SystemStatsPayload *stats,
    const PSPDL_GitStatusPayload *git,
    const PSPDL_NotificationPayload *notif);

void ui_add_notification(const PSPDL_NotificationPayload *notif);
void ui_toggle_history(void);
void ui_close_active_popup(void);
void ui_clear_history(void);
void ui_handle_circle_press(void);

#endif // PSPDL_UI_H
