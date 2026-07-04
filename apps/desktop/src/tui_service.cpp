#include "tui_service.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

TuiService::TuiService(ConfigService &config)
    : m_config(config), m_raw_mode(false)
{
    memset(&m_orig_termios, 0, sizeof(m_orig_termios));
}

TuiService::~TuiService()
{
    shutdown();
}

void TuiService::initialize()
{
    if (m_raw_mode) return;

    if (tcgetattr(STDIN_FILENO, &m_orig_termios) < 0)
    {
        return;
    }

    struct termios raw = m_orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag &= ~(CSIZE | PARENB);
    raw.c_cflag |= CS8;
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) < 0)
    {
        return;
    }

    m_raw_mode = true;

    // Clear terminal screen and hide cursor
    std::cout << "\033[2J\033[H\033[?25l" << std::flush;
}

void TuiService::shutdown()
{
    if (!m_raw_mode) return;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &m_orig_termios);
    m_raw_mode = false;

    // Show cursor and clean up terminal line
    std::cout << "\033[?25h\n" << std::flush;
}

char TuiService::check_input()
{
    if (!m_raw_mode) return 0;

    char c = 0;
    int n = read(STDIN_FILENO, &c, 1);
    if (n == 1)
    {
        // Process toggles and config adjustments directly
        if (c == '1')
        {
            if (m_config.telemetry_interval_ms == 500) m_config.telemetry_interval_ms = 1000;
            else if (m_config.telemetry_interval_ms == 1000) m_config.telemetry_interval_ms = 2000;
            else if (m_config.telemetry_interval_ms == 2000) m_config.telemetry_interval_ms = 5000;
            else m_config.telemetry_interval_ms = 500;
            m_config.save();
        }
        else if (c == '2')
        {
            if (m_config.git_interval_ms == 1000) m_config.git_interval_ms = 3000;
            else if (m_config.git_interval_ms == 3000) m_config.git_interval_ms = 5000;
            else if (m_config.git_interval_ms == 5000) m_config.git_interval_ms = 10000;
            else m_config.git_interval_ms = 1000;
            m_config.save();
        }
        else if (c == '3')
        {
            if (m_config.notif_interval_ms == 250) m_config.notif_interval_ms = 500;
            else if (m_config.notif_interval_ms == 500) m_config.notif_interval_ms = 1000;
            else if (m_config.notif_interval_ms == 1000) m_config.notif_interval_ms = 2000;
            else m_config.notif_interval_ms = 250;
            m_config.save();
        }
        else if (c == '4')
        {
            m_config.enable_git = !m_config.enable_git;
            m_config.save();
        }
        else if (c == '5')
        {
            m_config.enable_notif = !m_config.enable_notif;
            m_config.save();
        }
        return c;
    }
    return 0;
}

void TuiService::render(const std::string &conn_status)
{
    // Write cursor position back to 0, 0
    std::cout << "\033[H";
    std::cout << "======================================================\n";
    std::cout << "         PSP DEVLINK - DESKTOP COMPANION MENU         \n";
    std::cout << "======================================================\n";
    std::cout << " Link Status: " << conn_status << "                      \n\n";

    std::cout << " [SETTINGS]\n";
    std::cout << "  [1] Telemetry Interval  : " << m_config.telemetry_interval_ms << " ms      \n";
    std::cout << "  [2] Git Sync Interval   : " << m_config.git_interval_ms << " ms      \n";
    std::cout << "  [3] Notif Check Interval: " << m_config.notif_interval_ms << " ms      \n";
    std::cout << "  [4] Enable Git Sync     : " << (m_config.enable_git ? "ENABLED " : "DISABLED") << "\n";
    std::cout << "  [5] Enable Notifications: " << (m_config.enable_notif ? "ENABLED " : "DISABLED") << "\n\n";

    std::cout << " [REMOTE COMMANDS]\n";
    std::cout << "  [x] Send 'Exit to XMB' Command to PSP\n";
    std::cout << "  [r] Send 'Reboot Console' Command to PSP\n\n";

    std::cout << " [CONTROLS]\n";
    std::cout << "  Press [1-5] to change configuration values\n";
    std::cout << "  Press [x] to Exit Client, [r] to Reboot Client\n";
    std::cout << "  Press [q] to Quit Companion Application\n";
    std::cout << "======================================================\n";
    std::cout << std::flush;
}
