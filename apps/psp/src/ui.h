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
    const PSPDL_GitStatusPayload *git);

#endif // PSPDL_UI_H
