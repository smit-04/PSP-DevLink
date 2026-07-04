#ifndef PSPDL_HTTP_SERVER_H
#define PSPDL_HTTP_SERVER_H

#include "config_service.h"
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>

struct SharedLogEntry
{
    std::string message;
    std::string type; // "info", "success", "warn", "error"
};

struct SharedTelemetry
{
    int cpu_usage = 0;
    int ram_usage = 0;
    int cpu_temp = 0;
    long long ram_total = 0;
    long long ram_free = 0;
};

struct SharedGit
{
    std::string branch = "--";
    int modified = 0;
    int untracked = 0;
};

class HttpServer
{
public:
    HttpServer(ConfigService& config);
    ~HttpServer();

    bool start(int port = 8080);
    void stop();

    // State setters called by main loop
    void update_connection_state(const std::string& state);
    void update_telemetry(const SharedTelemetry& tel);
    void update_git(const SharedGit& git);
    void add_log(const std::string& msg, const std::string& type = "info");

    // Command triggers read by main loop
    bool check_and_clear_exit();
    bool check_and_clear_reboot();

private:
    void run_listener();
    void handle_client(int client_fd);

    ConfigService& m_config;
    int m_port;
    int m_server_fd;
    std::atomic<bool> m_running;
    std::thread m_thread;

    // Shared status caches
    std::string m_connection_state;
    SharedTelemetry m_telemetry;
    bool m_has_telemetry;
    SharedGit m_git;
    bool m_has_git;

    std::vector<SharedLogEntry> m_logs;
    std::mutex m_mutex;

    // Control triggers
    std::atomic<bool> m_trigger_exit;
    std::atomic<bool> m_trigger_reboot;
};

#endif // PSPDL_HTTP_SERVER_H
