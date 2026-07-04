#include "ui.h"
#include <pspdebug.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <string.h>

void ui_init(void)
{
    // Initialize debug screen system
    pspDebugScreenInit();

    // Set standard colors
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
    const PSPDL_GitStatusPayload *git)
{
    /* ---- vsync: wait for vertical blank so we draw during the off-screen
            period — this is the standard fix for PSP debug-screen flicker ---- */
    sceDisplayWaitVblankStart();

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
}
