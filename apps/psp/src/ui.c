#include "ui.h"
#include <pspdebug.h>
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

static void draw_progress_bar(int x, int y, int val, int max_val, uint32_t bar_color)
{
    pspDebugScreenSetXY(x, y);
    pspDebugScreenSetTextColor(0xFFCCCCCC);
    pspDebugScreenPrintf("[");
    
    int bar_width = 10;
    int filled = (val * bar_width) / max_val;
    if (filled > bar_width) filled = bar_width;
    if (filled < 0) filled = 0;
    
    pspDebugScreenSetTextColor(bar_color);
    for (int i = 0; i < filled; i++)
    {
        pspDebugScreenPrintf("#");
    }
    pspDebugScreenSetTextColor(0xFF444444);
    for (int i = filled; i < bar_width; i++)
    {
        pspDebugScreenPrintf("-");
    }
    
    pspDebugScreenSetTextColor(0xFFCCCCCC);
    pspDebugScreenPrintf("]");
}

void ui_render(
    UIConnectionState conn_state,
    const PSPDL_SystemStatsPayload *stats,
    const PSPDL_GitStatusPayload *git)
{
    // Move to (0, 0) and redraw everything. Overwriting avoids flickering!
    pspDebugScreenSetXY(0, 0);
    
    // Draw main header border
    pspDebugScreenSetTextColor(0xFF888888);
    pspDebugScreenPrintf("+-----------------------------------------------------------------+\n");
    pspDebugScreenPrintf("|                 ");
    pspDebugScreenSetTextColor(0xFFFFFF00); // Cyan title
    pspDebugScreenPrintf("PSP DEVLINK COMPANION DASHBOARD");
    pspDebugScreenSetTextColor(0xFF888888);
    pspDebugScreenPrintf("                 |\n");
    pspDebugScreenPrintf("+-----------------------------------------------------------------+\n\n");

    // Draw connection status line
    pspDebugScreenSetXY(2, 4);
    pspDebugScreenSetTextColor(0xFFFFFFFF);
    pspDebugScreenPrintf("Link Status: ");
    if (conn_state == UI_CONN_CONNECTED)
    {
        pspDebugScreenSetTextColor(0xFF00FF00); // Green
        pspDebugScreenPrintf("[ CONNECTED ]               ");
    }
    else
    {
        pspDebugScreenSetTextColor(0xFF00FFFF); // Yellow
        pspDebugScreenPrintf("[ SEARCHING FOR HOST... ]  ");
    }

    // Left Card: System Performance
    pspDebugScreenSetTextColor(0xFF888888);
    pspDebugScreenSetXY(2, 6);
    pspDebugScreenPrintf("+--------- SYSTEM TELEMETRY -------+");
    for (int j = 1; j <= 9; j++)
    {
        pspDebugScreenSetXY(2, 6 + j);
        pspDebugScreenPrintf("|                                  |");
    }
    pspDebugScreenSetXY(2, 16);
    pspDebugScreenPrintf("+----------------------------------+");

    // Right Card: Git Status
    pspDebugScreenSetTextColor(0xFF888888);
    pspDebugScreenSetXY(38, 6);
    pspDebugScreenPrintf("+------- GIT WORKSPACE STATUS -----+");
    for (int j = 1; j <= 9; j++)
    {
        pspDebugScreenSetXY(38, 6 + j);
        pspDebugScreenPrintf("|                                  |");
    }
    pspDebugScreenSetXY(38, 16);
    pspDebugScreenPrintf("+----------------------------------+");

    // Fill System Card Content
    if (conn_state == UI_CONN_CONNECTED && stats != NULL)
    {
        // CPU Usage
        pspDebugScreenSetXY(4, 8);
        pspDebugScreenSetTextColor(0xFFCCCCCC);
        pspDebugScreenPrintf("CPU Load: ");
        draw_progress_bar(15, 8, stats->cpu_usage, 100, 0xFF00FF00); // Green bar
        pspDebugScreenSetXY(28, 8);
        pspDebugScreenSetTextColor(0xFFFFFFFF);
        pspDebugScreenPrintf("%3d%%", stats->cpu_usage);

        // RAM Usage
        pspDebugScreenSetXY(4, 10);
        pspDebugScreenSetTextColor(0xFFCCCCCC);
        pspDebugScreenPrintf("RAM Load: ");
        draw_progress_bar(15, 10, stats->ram_usage, 100, 0xFF00FF00);
        pspDebugScreenSetXY(28, 10);
        pspDebugScreenSetTextColor(0xFFFFFFFF);
        pspDebugScreenPrintf("%3d%%", stats->ram_usage);

        // CPU Temperature
        pspDebugScreenSetXY(4, 12);
        pspDebugScreenSetTextColor(0xFFCCCCCC);
        pspDebugScreenPrintf("CPU Temp:  ");
        pspDebugScreenSetTextColor(0xFFFFFFFF);
        pspDebugScreenPrintf("%4d.%d C", stats->cpu_temp / 10, stats->cpu_temp % 10);

        // Memory Sizes
        double total_gb = (double)stats->ram_total / (1024.0 * 1024.0 * 1024.0);
        double free_gb = (double)stats->ram_free / (1024.0 * 1024.0 * 1024.0);
        pspDebugScreenSetXY(4, 14);
        pspDebugScreenSetTextColor(0xFFCCCCCC);
        pspDebugScreenPrintf("Free Mem:  ");
        pspDebugScreenSetTextColor(0xFFFFFFFF);
        pspDebugScreenPrintf("%5.2f / %5.2f GB", free_gb, total_gb);
    }
    else
    {
        // Default Empty State
        pspDebugScreenSetTextColor(0xFF666666);
        pspDebugScreenSetXY(4, 8);
        pspDebugScreenPrintf("CPU Load:  [----------]   --%%");
        pspDebugScreenSetXY(4, 10);
        pspDebugScreenPrintf("RAM Load:  [----------]   --%%");
        pspDebugScreenSetXY(4, 12);
        pspDebugScreenPrintf("CPU Temp:  --.- C");
        pspDebugScreenSetXY(4, 14);
        pspDebugScreenPrintf("Free Mem:  --.-- / --.-- GB");
    }

    // Fill Git Card Content
    if (conn_state == UI_CONN_CONNECTED && git != NULL)
    {
        // Branch
        pspDebugScreenSetXY(40, 8);
        pspDebugScreenSetTextColor(0xFFCCCCCC);
        pspDebugScreenPrintf("Branch:    ");
        pspDebugScreenSetTextColor(0xFFFFFF00); // Cyan branch
        pspDebugScreenPrintf("%-20s", git->branch_name);

        // Modified Files
        pspDebugScreenSetXY(40, 10);
        pspDebugScreenSetTextColor(0xFFCCCCCC);
        pspDebugScreenPrintf("Modified:  ");
        if (git->modified_files > 0)
        {
            pspDebugScreenSetTextColor(0xFF0000FF); // Red if dirty
        }
        else
        {
            pspDebugScreenSetTextColor(0xFF00FF00); // Green if clean
        }
        pspDebugScreenPrintf("%-4u files", (unsigned int)git->modified_files);

        // Untracked Files
        pspDebugScreenSetXY(40, 12);
        pspDebugScreenSetTextColor(0xFFCCCCCC);
        pspDebugScreenPrintf("Untracked: ");
        if (git->untracked_files > 0)
        {
            pspDebugScreenSetTextColor(0xFF00FFFF); // Yellow if untracked
        }
        else
        {
            pspDebugScreenSetTextColor(0xFF00FF00); // Green if clean
        }
        pspDebugScreenPrintf("%-4u files", (unsigned int)git->untracked_files);

        // Worktree
        pspDebugScreenSetXY(40, 14);
        pspDebugScreenSetTextColor(0xFFCCCCCC);
        pspDebugScreenPrintf("Worktree:  ");
        pspDebugScreenSetTextColor(0xFF00FF00);
        pspDebugScreenPrintf("ACTIVE");
    }
    else
    {
        // Default Empty State
        pspDebugScreenSetTextColor(0xFF666666);
        pspDebugScreenSetXY(40, 8);
        pspDebugScreenPrintf("Branch:    --");
        pspDebugScreenSetXY(40, 10);
        pspDebugScreenPrintf("Modified:  -- files");
        pspDebugScreenSetXY(40, 12);
        pspDebugScreenPrintf("Untracked: -- files");
        pspDebugScreenSetXY(40, 14);
        pspDebugScreenPrintf("Worktree:  INACTIVE");
    }

    // Bottom info block
    pspDebugScreenSetTextColor(0xFF888888);
    pspDebugScreenSetXY(0, 18);
    pspDebugScreenPrintf("\n+-----------------------------------------------------------------+\n");
    pspDebugScreenPrintf("| Press ");
    pspDebugScreenSetTextColor(0xFFFFFF00);
    pspDebugScreenPrintf("START");
    pspDebugScreenSetTextColor(0xFF888888);
    pspDebugScreenPrintf(" to exit client application                              |\n");
    pspDebugScreenPrintf("+-----------------------------------------------------------------+\n");
}
