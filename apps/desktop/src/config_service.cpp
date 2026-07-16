#include "config_service.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cctype>

ConfigService::ConfigService(const std::string &filepath)
    : m_filepath(filepath),
      telemetry_interval_ms(1000),
      git_interval_ms(3000),
      notif_interval_ms(500),
      enable_git(true),
      enable_notif(true)
{
}

void ConfigService::load()
{
    std::ifstream file(m_filepath);
    if (!file.is_open())
    {
        save();
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos)
        {
            line = line.substr(0, comment_pos);
        }

        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos)
        {
            continue;
        }

        std::string key = line.substr(0, eq_pos);
        std::string val = line.substr(eq_pos + 1);

        auto trim = [](std::string &s) {
            while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
            while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
        };
        trim(key);
        trim(val);

        if (val.empty()) continue;

        try {
            if (key == "telemetry_interval_ms")
            {
                telemetry_interval_ms = std::stoi(val);
            }
            else if (key == "git_interval_ms")
            {
                git_interval_ms = std::stoi(val);
            }
            else if (key == "notif_interval_ms")
            {
                notif_interval_ms = std::stoi(val);
            }
            else if (key == "enable_git")
            {
                enable_git = (std::stoi(val) != 0);
            }
            else if (key == "enable_notif")
            {
                enable_notif = (std::stoi(val) != 0);
            }
        }
        catch (const std::exception& e) {
            std::cerr << "[WARN] Invalid configuration value for " << key << ": " << val << std::endl;
        }
    }
}

void ConfigService::save()
{
    std::ofstream file(m_filepath);
    if (!file.is_open())
    {
        std::cerr << "[ERROR] Could not open config file for writing: " << m_filepath << std::endl;
        return;
    }

    file << "# PSP DevLink Desktop Companion Configuration\n";
    file << "telemetry_interval_ms = " << telemetry_interval_ms << "\n";
    file << "git_interval_ms = " << git_interval_ms << "\n";
    file << "notif_interval_ms = " << notif_interval_ms << "\n";
    file << "enable_git = " << (enable_git ? 1 : 0) << "\n";
    file << "enable_notif = " << (enable_notif ? 1 : 0) << "\n";
}
