#include "ui.h"
#include <pspdebug.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <string.h>

// Double-buffering for flicker-free rendering.
// PSP VRAM layout: 0x04000000 base, each 512x272 RGBA8888 frame = 0x88000 bytes
#define DISPLAY_WIDTH   480
#define DISPLAY_HEIGHT  272
#define DISPLAY_STRIDE  512   // row stride in pixels (must be power of 2)
// Frame buffer 0 at VRAM offset 0, frame buffer 1 right after
static void *g_draw_buf  = (void *)0x04000000; // back buffer  (we draw here)
static void *g_disp_buf  = (void *)0x04088000; // front buffer (displayed)

static PSPDL_NotificationPayload s_notif_history[5];
static int s_notif_history_count = 0;
static int s_showing_history = 0;

static int s_popup_active = 0;
static int s_popup_ticks = 0; // 100 ticks = 1 second
static PSPDL_NotificationPayload s_active_popup_notif;

void ui_add_notification(const PSPDL_NotificationPayload *notif)
{
    if (notif == NULL || notif->app_name[0] == '\0')
    {
        return;
    }

    // Check if this notification matches the most recent one in history to avoid duplicates
    if (s_notif_history_count > 0 && 
        strcmp(s_notif_history[0].app_name, notif->app_name) == 0 &&
        strcmp(s_notif_history[0].summary, notif->summary) == 0 &&
        strcmp(s_notif_history[0].body, notif->body) == 0)
    {
        // Already recorded; if not active, trigger the popup again
        if (!s_popup_active)
        {
            s_active_popup_notif = *notif;
            s_popup_active = 1;
            s_popup_ticks = 500; // 5 seconds
        }
        return;
    }

    // Shift history right
    for (int i = 4; i > 0; i--)
    {
        s_notif_history[i] = s_notif_history[i - 1];
    }
    s_notif_history[0] = *notif;
    
    if (s_notif_history_count < 5)
    {
        s_notif_history_count++;
    }

    // Trigger popup
    s_active_popup_notif = *notif;
    s_popup_active = 1;
    s_popup_ticks = 500; // 5 seconds
}

void ui_toggle_history(void)
{
    s_showing_history = !s_showing_history;
    // No manual clear needed: next ui_render() call clears the back buffer
    // at frame start before drawing, so there are no residues.
}

void ui_close_active_popup(void)
{
    s_popup_active = 0;
    s_popup_ticks = 0;
}

void ui_clear_history(void)
{
    memset(s_notif_history, 0, sizeof(s_notif_history));
    s_notif_history_count = 0;
    // No manual clear needed: back buffer is wiped at the start of every frame.
}

void ui_handle_circle_press(void)
{
    if (s_popup_active)
    {
        s_popup_active = 0;
        s_popup_ticks = 0;
        // No manual clear needed: frame start clears back buffer each render.
    }
    else if (s_showing_history)
    {
        ui_clear_history();
    }
}

static void ui_draw_popup(void)
{
    int start_row = 7;
    int end_row = 15;
    int start_col = 10;
    int end_col = 57;

    // Overwrite standard cards backgrounds
    for (int r = start_row; r <= end_row; r++)
    {
        pspDebugScreenSetXY(start_col, r);
        for (int c = start_col; c <= end_col; c++)
        {
            if (r == start_row && c == start_col) {
                pspDebugScreenSetTextColor(0xFF888888);
                pspDebugScreenPrintf("+");
            } else if (r == start_row && c == end_col) {
                pspDebugScreenPrintf("+");
            } else if (r == end_row && c == start_col) {
                pspDebugScreenPrintf("+");
            } else if (r == end_row && c == end_col) {
                pspDebugScreenPrintf("+");
            } else if (r == start_row || r == end_row) {
                pspDebugScreenSetTextColor(0xFF888888);
                pspDebugScreenPrintf("=");
            } else if (c == start_col || c == end_col) {
                pspDebugScreenSetTextColor(0xFF888888);
                pspDebugScreenPrintf("|");
            } else {
                pspDebugScreenPrintf(" ");
            }
        }
    }

    // Draw Title
    pspDebugScreenSetXY(start_col + 2, start_row + 1);
    pspDebugScreenSetTextColor(0xFF00FFFF); // Yellow
    pspDebugScreenPrintf("[NEW NOTIFICATION]");

    // Draw App Name
    pspDebugScreenSetXY(start_col + 2, start_row + 2);
    pspDebugScreenSetTextColor(0xFFFFFF00); // Cyan
    pspDebugScreenPrintf("App  : %s", s_active_popup_notif.app_name);

    // Draw Summary
    pspDebugScreenSetXY(start_col + 2, start_row + 3);
    pspDebugScreenSetTextColor(0xFFFFFFFF); // White
    char sum_truncated[41];
    strncpy(sum_truncated, s_active_popup_notif.summary, 40);
    sum_truncated[40] = '\0';
    pspDebugScreenPrintf("Title: %s", sum_truncated);

    // Draw Body
    pspDebugScreenSetXY(start_col + 2, start_row + 4);
    pspDebugScreenSetTextColor(0xFFCCCCCC); // Grey
    char body_truncated[41];
    strncpy(body_truncated, s_active_popup_notif.body, 40);
    body_truncated[40] = '\0';
    pspDebugScreenPrintf("Body : %s", body_truncated);

    // Draw countdown details
    pspDebugScreenSetXY(start_col + 2, start_row + 6);
    pspDebugScreenSetTextColor(0xFF555555); // Dark grey
    int secs_left = (s_popup_ticks + 99) / 100;
    pspDebugScreenPrintf("[ Auto-closing in %ds | O to dismiss ]", secs_left);
}

void ui_init(void)
{
    // Set up double buffering:
    // - Draw buffer (back) at VRAM offset 0x00000
    // - Display buffer (front) at VRAM offset 0x88000
    // pspDebugScreenInit() sets the draw target to VRAM base by default.
    pspDebugScreenInit();
    pspDebugScreenSetOffset(0); // draw to back buffer (offset 0 in VRAM)

    // Point display hardware at the FRONT buffer initially
    sceDisplaySetFrameBuf(g_disp_buf, DISPLAY_STRIDE, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);

    pspDebugScreenSetTextColor(0xFFFFFFFF);
    pspDebugScreenSetBackColor(0xFF000000);
    pspDebugScreenClear();
}

/* Draw a 10-char wide progress bar at absolute column/row position */
static void draw_progress_bar(int x, int y, int val, int max_val, uint32_t bar_color)
{
    pspDebugScreenSetXY(x, y);
    pspDebugScreenSetTextColor(0xFFCCCCCC);
    pspDebugScreenPrintf("[");

    int bar_width = 10;
    int filled = (max_val > 0) ? (val * bar_width) / max_val : 0;
    if (filled > bar_width) filled = bar_width;
    if (filled < 0)         filled = 0;

    pspDebugScreenSetTextColor(bar_color);
    for (int i = 0; i < filled; i++)
        pspDebugScreenPrintf("#");

    pspDebugScreenSetTextColor(0xFF444444);
    for (int i = filled; i < bar_width; i++)
        pspDebugScreenPrintf("-");

    pspDebugScreenSetTextColor(0xFFCCCCCC);
    pspDebugScreenPrintf("]");
}

/*
 * PSP debug-screen character grid: 68 cols x 34 rows.
 * Layout (col indices are 0-based):
 *
 *  Row 0-2   : header border + title
 *  Row 4     : link-status line
 *  Row 6-16  : two 30-char cards side by side
 *               Left  card : cols  1 .. 32
 *               Right card : cols 34 .. 65
 *  Row 18-21 : footer border + help text
 */
void ui_render(
    UIConnectionState conn_state,
    const PSPDL_SystemStatsPayload *stats,
    const PSPDL_GitStatusPayload *git,
    const PSPDL_NotificationPayload *notif)
{
    /* ---- Double-buffer flip ----------------------------------------
     * 1. Tell pspDebugScreen to render into the BACK buffer (g_draw_buf)
     * 2. Draw the complete frame
     * 3. Wait for vblank
     * 4. Swap: the back buffer becomes the display, the front becomes the
     *    new back buffer for the next frame
     * The scan line always reads from the buffer that is fully rendered.
     * ---------------------------------------------------------------- */

    // Step 1: point debug screen at the back (draw) buffer
    int draw_offset = (int)((char*)g_draw_buf - (char*)0x04000000);
    pspDebugScreenSetOffset(draw_offset);
    pspDebugScreenClear();

    /* ===== HEADER ===== */
    pspDebugScreenSetXY(0, 0);
    pspDebugScreenSetTextColor(0xFF888888);
    pspDebugScreenPrintf("+---------------------------------------------------------------+");

    pspDebugScreenSetXY(0, 1);
    pspDebugScreenPrintf("|              ");
    pspDebugScreenSetTextColor(0xFFFFFF00);           /* cyan */
    pspDebugScreenPrintf("PSP DEVLINK COMPANION DASHBOARD");
    pspDebugScreenSetTextColor(0xFF888888);
    pspDebugScreenPrintf("               |");

    pspDebugScreenSetXY(0, 2);
    pspDebugScreenPrintf("+---------------------------------------------------------------+");

    /* ===== LINK STATUS ===== */
    pspDebugScreenSetXY(2, 4);
    pspDebugScreenSetTextColor(0xFFFFFFFF);
    pspDebugScreenPrintf("Link Status: ");
    if (conn_state == UI_CONN_CONNECTED)
    {
        pspDebugScreenSetTextColor(0xFF00FF00);       /* green  */
        pspDebugScreenPrintf("[ CONNECTED ]              ");
    }
    else
    {
        pspDebugScreenSetTextColor(0xFF00FFFF);       /* yellow */
        pspDebugScreenPrintf("[ SEARCHING FOR HOST... ]  ");
    }

    // Display driver load/USB status on Row 5
    extern char g_status_msg[128];
    pspDebugScreenSetXY(2, 5);
    pspDebugScreenSetTextColor(0xFF888888); // Grey text for status
    pspDebugScreenPrintf("System Status: %s", g_status_msg);


    if (s_showing_history)
    {
        // History drawer: no need to clear rows 6-16 separately;
        // pspDebugScreenClear() at frame start already wiped the back buffer.

        // Print Header
        pspDebugScreenSetXY(2, 6);
        pspDebugScreenSetTextColor(0xFF00FFFF); // Yellow
        pspDebugScreenPrintf("--- NOTIFICATION HISTORY DRAWER (Last %d) ---", s_notif_history_count);

        if (s_notif_history_count == 0)
        {
            pspDebugScreenSetXY(5, 10);
            pspDebugScreenSetTextColor(0xFF555555); // Grey
            pspDebugScreenPrintf("[ No notifications in history ]");
        }
        else
        {
            for (int i = 0; i < s_notif_history_count; i++)
            {
                int row = 8 + i; // Row 8, 9, 10, 11, 12
                pspDebugScreenSetXY(2, row);
                
                // Color tags: highlight Yellow for "[X] ", Cyan for app, White for text
                pspDebugScreenSetTextColor(0xFF00FFFF); // Yellow
                pspDebugScreenPrintf("[%d] ", i + 1);
                
                pspDebugScreenSetTextColor(0xFFFFFF00); // Cyan
                pspDebugScreenPrintf("%s: ", s_notif_history[i].app_name);
                
                pspDebugScreenSetTextColor(0xFFFFFFFF); // White
                char text_to_print[128];
                snprintf(text_to_print, sizeof(text_to_print), "%s - %s", s_notif_history[i].summary, s_notif_history[i].body);
                
                int prefix_len = 4 + strlen(s_notif_history[i].app_name) + 2;
                int remaining_cols = 62 - prefix_len;
                if (remaining_cols > 0)
                {
                    char final_msg[128];
                    strncpy(final_msg, text_to_print, remaining_cols);
                    final_msg[remaining_cols] = '\0';
                    if (strlen(text_to_print) > (size_t)remaining_cols && remaining_cols >= 3)
                    {
                        final_msg[remaining_cols - 1] = '.';
                        final_msg[remaining_cols - 2] = '.';
                        final_msg[remaining_cols - 3] = '.';
                    }
                    pspDebugScreenPrintf("%s", final_msg);
                }
            }
        }

        // Print help instructions inside the panel
        pspDebugScreenSetXY(2, 15);
        pspDebugScreenSetTextColor(0xFF555555); // Grey
        pspDebugScreenPrintf("[ SELECT: Back | CIRCLE: Clear History ]");
    }
    else
    {
        /* ===== LEFT CARD border — cols 1-32, rows 6-16 ===== */
        pspDebugScreenSetTextColor(0xFF888888);

        pspDebugScreenSetXY(1, 6);
        pspDebugScreenPrintf("+---- SYSTEM TELEMETRY ----+");

        for (int j = 1; j <= 9; j++)
        {
            pspDebugScreenSetXY(1, 6 + j);
            pspDebugScreenPrintf("|                          |");
        }

        pspDebugScreenSetXY(1, 16);
        pspDebugScreenPrintf("+--------------------------+");

        /* ===== RIGHT CARD border — cols 34-65, rows 6-16 ===== */
        pspDebugScreenSetXY(34, 6);
        pspDebugScreenPrintf("+--- GIT WORKSPACE STATUS --+");

        for (int j = 1; j <= 9; j++)
        {
            pspDebugScreenSetXY(34, 6 + j);
            pspDebugScreenPrintf("|                           |");
        }

        pspDebugScreenSetXY(34, 16);
        pspDebugScreenPrintf("+---------------------------+");

        /* ===== LEFT CARD content ===== */
        if (conn_state == UI_CONN_CONNECTED && stats != NULL)
        {
            /* CPU bar */
            pspDebugScreenSetXY(3, 8);
            pspDebugScreenSetTextColor(0xFFCCCCCC);
            pspDebugScreenPrintf("CPU: ");
            draw_progress_bar(8, 8, (int)stats->cpu_usage, 100, 0xFF00FF00);
            pspDebugScreenSetXY(20, 8);
            pspDebugScreenSetTextColor(0xFFFFFFFF);
            pspDebugScreenPrintf("%3d%%", (int)stats->cpu_usage);

            /* RAM bar */
            pspDebugScreenSetXY(3, 10);
            pspDebugScreenSetTextColor(0xFFCCCCCC);
            pspDebugScreenPrintf("RAM: ");
            draw_progress_bar(8, 10, (int)stats->ram_usage, 100, 0xFF00FF00);
            pspDebugScreenSetXY(20, 10);
            pspDebugScreenSetTextColor(0xFFFFFFFF);
            pspDebugScreenPrintf("%3d%%", (int)stats->ram_usage);

            /* CPU temp */
            pspDebugScreenSetXY(3, 12);
            pspDebugScreenSetTextColor(0xFFCCCCCC);
            pspDebugScreenPrintf("Temp: ");
            pspDebugScreenSetTextColor(0xFFFFFFFF);
            pspDebugScreenPrintf("%3d.%d C ", (int)(stats->cpu_temp / 10),
                                              (int)(stats->cpu_temp % 10));

            /* Free memory */
            double total_gb = (double)stats->ram_total / (1024.0 * 1024.0 * 1024.0);
            double free_gb  = (double)stats->ram_free  / (1024.0 * 1024.0 * 1024.0);
            pspDebugScreenSetXY(3, 14);
            pspDebugScreenSetTextColor(0xFFCCCCCC);
            pspDebugScreenPrintf("Mem:  ");
            pspDebugScreenSetTextColor(0xFFFFFFFF);
            pspDebugScreenPrintf("%.1f/%.1f GB ", free_gb, total_gb);
        }
        else
        {
            pspDebugScreenSetTextColor(0xFF555555);
            pspDebugScreenSetXY(3, 8);
            pspDebugScreenPrintf("CPU: [----------] --%%");
            pspDebugScreenSetXY(3, 10);
            pspDebugScreenPrintf("RAM: [----------] --%%");
            pspDebugScreenSetXY(3, 12);
            pspDebugScreenPrintf("Temp:  --.- C      ");
            pspDebugScreenSetXY(3, 14);
            pspDebugScreenPrintf("Mem:  --.-- GB    ");
        }

        /* ===== RIGHT CARD content ===== */
        if (conn_state == UI_CONN_CONNECTED && git != NULL)
        {
            /* Branch */
            pspDebugScreenSetXY(36, 8);
            pspDebugScreenSetTextColor(0xFFCCCCCC);
            pspDebugScreenPrintf("Branch: ");
            pspDebugScreenSetTextColor(0xFFFFFF00);       /* cyan */
            pspDebugScreenPrintf("%-14s", git->branch_name);

            /* Modified */
            pspDebugScreenSetXY(36, 10);
            pspDebugScreenSetTextColor(0xFFCCCCCC);
            pspDebugScreenPrintf("Modif:  ");
            pspDebugScreenSetTextColor(git->modified_files > 0 ? 0xFF0000FF : 0xFF00FF00);
            pspDebugScreenPrintf("%-3u files ", (unsigned int)git->modified_files);

            /* Untracked */
            pspDebugScreenSetXY(36, 12);
            pspDebugScreenSetTextColor(0xFFCCCCCC);
            pspDebugScreenPrintf("Untrk:  ");
            pspDebugScreenSetTextColor(git->untracked_files > 0 ? 0xFF00FFFF : 0xFF00FF00);
            pspDebugScreenPrintf("%-3u files ", (unsigned int)git->untracked_files);

            /* Worktree */
            pspDebugScreenSetXY(36, 14);
            pspDebugScreenSetTextColor(0xFFCCCCCC);
            pspDebugScreenPrintf("Tree:   ");
            pspDebugScreenSetTextColor(0xFF00FF00);
            pspDebugScreenPrintf("ACTIVE  ");
        }
        else
        {
            pspDebugScreenSetTextColor(0xFF555555);
            pspDebugScreenSetXY(36, 8);
            pspDebugScreenPrintf("Branch: --            ");
            pspDebugScreenSetXY(36, 10);
            pspDebugScreenPrintf("Modif:  -- files      ");
            pspDebugScreenSetXY(36, 12);
            pspDebugScreenPrintf("Untrk:  -- files      ");
            pspDebugScreenSetXY(36, 14);
            pspDebugScreenPrintf("Tree:   INACTIVE      ");
        }
    }

    /* ===== FOOTER ===== */
    pspDebugScreenSetTextColor(0xFF888888);
    pspDebugScreenSetXY(0, 18);
    pspDebugScreenPrintf("+---------------------------------------------------------------+");
    pspDebugScreenSetXY(0, 19);
    pspDebugScreenPrintf("| Press ");
    pspDebugScreenSetTextColor(0xFFFFFF00);
    pspDebugScreenPrintf("START");
    pspDebugScreenSetTextColor(0xFF888888);
    pspDebugScreenPrintf(" to exit client application                          |");
    pspDebugScreenSetXY(0, 20);
    pspDebugScreenPrintf("+---------------------------------------------------------------+");

    /* ===== NOTIFICATION TICKER ===== */
    pspDebugScreenSetXY(1, 22);
    int cols_printed = 0;
    
    if (conn_state == UI_CONN_CONNECTED && notif != NULL && notif->app_name[0] != '\0')
    {
        // Print "[NOTIF] " (8 chars)
        pspDebugScreenSetTextColor(0xFF00FFFF); // Yellow
        pspDebugScreenPrintf("[NOTIF] ");
        cols_printed += 8;

        // Print app name (app_name + ": " chars)
        pspDebugScreenSetTextColor(0xFFFFFF00); // Cyan
        char app_str[32];
        snprintf(app_str, sizeof(app_str), "%s: ", notif->app_name);
        pspDebugScreenPrintf("%s", app_str);
        cols_printed += strlen(app_str);

        // Print combined summary and body, truncated to fit on a single line
        pspDebugScreenSetTextColor(0xFFFFFFFF); // White
        char msg_str[128];
        snprintf(msg_str, sizeof(msg_str), "%s - %s", notif->summary, notif->body);
        
        // We have (66 - cols_printed) columns left on this row
        int max_msg_len = 66 - cols_printed;
        if (max_msg_len > 0)
        {
            char msg_truncated[128];
            strncpy(msg_truncated, msg_str, max_msg_len);
            msg_truncated[max_msg_len] = '\0';
            
            // If it was truncated, add a premium "..." trailing ellipsis
            int msg_len = strlen(msg_truncated);
            if (strlen(msg_str) > (size_t)max_msg_len && msg_len >= 3)
            {
                msg_truncated[msg_len - 1] = '.';
                msg_truncated[msg_len - 2] = '.';
                msg_truncated[msg_len - 3] = '.';
            }
            
            pspDebugScreenPrintf("%s", msg_truncated);
            cols_printed += strlen(msg_truncated);
        }
    }
    else
    {
        pspDebugScreenSetTextColor(0xFF555555); // Grey placeholder
        pspDebugScreenPrintf("[NOTIF] No notifications received");
        cols_printed += 33;
    }

    // Overwrite the rest of row 22 with spaces up to column 67
    pspDebugScreenSetTextColor(0xFFFFFFFF);
    int remaining = 67 - cols_printed;
    for (int i = 0; i < remaining; i++)
    {
        pspDebugScreenPrintf(" ");
    }

    // Clear row 23 entirely to remove any old multi-line wrapping leftovers
    pspDebugScreenSetXY(1, 23);
    for (int i = 0; i < 66; i++)
    {
        pspDebugScreenPrintf(" ");
    }

    // Draw popup overlay if active
    if (s_popup_active)
    {
        ui_draw_popup();
        
        if (s_popup_ticks > 0)
        {
            s_popup_ticks--;
        }
        else
        {
            s_popup_active = 0;
        }
    }

    /* ---- Step 3+4: wait for vblank then flip buffers ---- */
    sceDisplayWaitVblankStart();

    // Show the back buffer on screen
    sceDisplaySetFrameBuf(g_draw_buf, DISPLAY_STRIDE, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);

    // Swap: what was the draw buffer becomes the display buffer next frame
    void *tmp   = g_disp_buf;
    g_disp_buf  = g_draw_buf;
    g_draw_buf  = tmp;
}
