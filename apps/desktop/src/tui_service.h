#ifndef PSPDL_DESKTOP_TUI_SERVICE_H
#define PSPDL_DESKTOP_TUI_SERVICE_H

#include "config_service.h"
#include <termios.h>

class TuiService
{
public:
    TuiService(ConfigService &config);
    ~TuiService();

    void initialize();
    void shutdown();

    /* Non-blocking key check. Returns char read or 0 if no key pressed */
    char check_input();

    /* Render the interactive TUI layout */
    void render(const std::string &conn_status);

private:
    ConfigService &m_config;
    struct termios m_orig_termios;
    bool m_raw_mode;
};

#endif // PSPDL_DESKTOP_TUI_SERVICE_H
