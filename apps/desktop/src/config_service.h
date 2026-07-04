#ifndef PSPDL_DESKTOP_CONFIG_SERVICE_H
#define PSPDL_DESKTOP_CONFIG_SERVICE_H

#include <string>

class ConfigService
{
public:
    ConfigService(const std::string &filepath);
    void load();
    void save();

    int telemetry_interval_ms;
    int git_interval_ms;
    int notif_interval_ms;
    bool enable_git;
    bool enable_notif;

private:
    std::string m_filepath;
};

#endif // PSPDL_DESKTOP_CONFIG_SERVICE_H
