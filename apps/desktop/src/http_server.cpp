#include "http_server.h"
#include "index_html.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

HttpServer::HttpServer(ConfigService& config)
    : m_config(config)
    , m_port(8080)
    , m_server_fd(-1)
    , m_running(false)
    , m_connection_state("DISCONNECTED")
    , m_has_telemetry(false)
    , m_has_git(false)
    , m_trigger_exit(false)
    , m_trigger_reboot(false)
{
}

HttpServer::~HttpServer()
{
    stop();
}

bool HttpServer::start(int port)
{
    m_port = port;
    m_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_server_fd < 0)
    {
        std::cerr << "[HTTP] Failed to create socket" << std::endl;
        return false;
    }

    int opt = 1;
    if (setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "[HTTP] setsockopt REUSEADDR failed" << std::endl;
        close(m_server_fd);
        m_server_fd = -1;
        return false;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(m_port);

    if (bind(m_server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        std::cerr << "[HTTP] Bind to port " << m_port << " failed" << std::endl;
        close(m_server_fd);
        m_server_fd = -1;
        return false;
    }

    if (listen(m_server_fd, 10) < 0)
    {
        std::cerr << "[HTTP] Listen failed" << std::endl;
        close(m_server_fd);
        m_server_fd = -1;
        return false;
    }

    m_running = true;
    m_thread = std::thread(&HttpServer::run_listener, this);
    std::cout << "[HTTP] Server started successfully on port " << m_port << std::endl;
    return true;
}

void HttpServer::stop()
{
    if (m_running)
    {
        m_running = false;
        if (m_server_fd >= 0)
        {
            // Connect to our own socket to wake up the accept call
            int dummy_fd = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in address;
            memset(&address, 0, sizeof(address));
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            address.sin_port = htons(m_port);
            connect(dummy_fd, (struct sockaddr*)&address, sizeof(address));
            close(dummy_fd);

            close(m_server_fd);
            m_server_fd = -1;
        }

        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }
}

void HttpServer::update_connection_state(const std::string& state)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connection_state = state;
}

void HttpServer::update_telemetry(const SharedTelemetry& tel)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_telemetry = tel;
    m_has_telemetry = true;
}

void HttpServer::update_git(const SharedGit& git)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_git = git;
    m_has_git = true;
}

void HttpServer::add_log(const std::string& msg, const std::string& type)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logs.push_back({msg, type});
    if (m_logs.size() > 100)
    {
        m_logs.erase(m_logs.begin());
    }
}

bool HttpServer::check_and_clear_exit()
{
    return m_trigger_exit.exchange(false);
}

bool HttpServer::check_and_clear_reboot()
{
    return m_trigger_reboot.exchange(false);
}

void HttpServer::run_listener()
{
    while (m_running)
    {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(m_server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0)
        {
            continue;
        }

        if (!m_running)
        {
            close(client_fd);
            break;
        }

        handle_client(client_fd);
    }
}

static std::string get_query_param(const std::string& path, const std::string& key)
{
    size_t pos = path.find("?" + key + "=");
    if (pos == std::string::npos)
    {
        pos = path.find("&" + key + "=");
    }
    if (pos == std::string::npos)
    {
        return "";
    }
    pos += key.length() + 2;
    size_t end_pos = path.find("&", pos);
    if (end_pos == std::string::npos)
    {
        return path.substr(pos);
    }
    return path.substr(pos, end_pos - pos);
}

static std::string get_post_param(const std::string& body, const std::string& key)
{
    size_t pos = body.find(key + "=");
    if (pos == std::string::npos)
    {
        return "";
    }
    pos += key.length() + 1;
    size_t end_pos = body.find("&", pos);
    if (end_pos == std::string::npos)
    {
        return body.substr(pos);
    }
    return body.substr(pos, end_pos - pos);
}

void HttpServer::handle_client(int client_fd)
{
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    ssize_t received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0)
    {
        close(client_fd);
        return;
    }

    std::string request(buffer);
    std::istringstream req_stream(request);
    std::string method, path, protocol;
    req_stream >> method >> path >> protocol;

    std::string body = "";
    size_t dbl_newline = request.find("\r\n\r\n");
    if (dbl_newline != std::string::npos)
    {
        body = request.substr(dbl_newline + 4);
    }

    std::ostringstream response;

    if (method == "GET" && (path == "/" || path == "/index.html"))
    {
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: text/html\r\n"
                 << "Content-Length: " << strlen(INDEX_HTML_CONTENT) << "\r\n"
                 << "Connection: close\r\n\r\n"
                 << INDEX_HTML_CONTENT;
    }
    else if (method == "GET" && path.rfind("/api/status", 0) == 0)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::ostringstream json;
        json << "{\n"
             << "  \"connection_state\": \"" << m_connection_state << "\",\n"
             << "  \"config\": {\n"
             << "    \"telemetry_interval\": " << m_config.telemetry_interval_ms << ",\n"
             << "    \"git_interval\": " << m_config.git_interval_ms << ",\n"
             << "    \"enable_git\": " << (m_config.enable_git ? "true" : "false") << ",\n"
             << "    \"enable_notif\": " << (m_config.enable_notif ? "true" : "false") << "\n"
             << "  }";

        if (m_connection_state == "CONNECTED" && m_has_telemetry)
        {
            json << ",\n  \"telemetry\": {\n"
                 << "    \"cpu_usage\": " << m_telemetry.cpu_usage << ",\n"
                 << "    \"ram_usage\": " << m_telemetry.ram_usage << ",\n"
                 << "    \"cpu_temp\": " << m_telemetry.cpu_temp << ",\n"
                 << "    \"ram_total\": " << m_telemetry.ram_total << ",\n"
                 << "    \"ram_free\": " << m_telemetry.ram_free << "\n"
                 << "  }";
        }

        if (m_connection_state == "CONNECTED" && m_has_git)
        {
            json << ",\n  \"git\": {\n"
                 << "    \"branch\": \"" << m_git.branch << "\",\n"
                 << "    \"modified\": " << m_git.modified << ",\n"
                 << "    \"untracked\": " << m_git.untracked << "\n"
                 << "  }";
        }

        json << ",\n  \"logs\": [\n";
        for (size_t i = 0; i < m_logs.size(); i++)
        {
            json << "    { \"message\": \"" << m_logs[i].message << "\", \"type\": \"" << m_logs[i].type << "\" }";
            if (i + 1 < m_logs.size()) json << ",";
            json << "\n";
        }
        json << "  ]\n";
        json << "}";

        std::string json_str = json.str();
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: application/json\r\n"
                 << "Content-Length: " << json_str.length() << "\r\n"
                 << "Connection: close\r\n\r\n"
                 << json_str;
    }
    else if (method == "POST" && path.rfind("/api/config", 0) == 0)
    {
        std::string tel_str = get_post_param(body, "telemetry_interval");
        std::string git_str = get_post_param(body, "git_interval");
        std::string e_git = get_post_param(body, "enable_git");
        std::string e_notif = get_post_param(body, "enable_notif");

        if (!tel_str.empty()) m_config.telemetry_interval_ms = std::stoi(tel_str);
        if (!git_str.empty()) m_config.git_interval_ms = std::stoi(git_str);
        if (!e_git.empty()) m_config.enable_git = (e_git == "1");
        if (!e_notif.empty()) m_config.enable_notif = (e_notif == "1");

        m_config.save();

        std::string resp_json = "{ \"result\": \"success\" }";
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: application/json\r\n"
                 << "Content-Length: " << resp_json.length() << "\r\n"
                 << "Connection: close\r\n\r\n"
                 << resp_json;
    }
    else if (method == "POST" && path.rfind("/api/control", 0) == 0)
    {
        std::string action = get_query_param(path, "action");
        std::string result = "unknown_action";

        if (action == "exit")
        {
            m_trigger_exit = true;
            result = "success";
        }
        else if (action == "reboot")
        {
            m_trigger_reboot = true;
            result = "success";
        }

        std::ostringstream resp_json;
        resp_json << "{ \"result\": \"" << result << "\" }";
        std::string resp_str = resp_json.str();

        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: application/json\r\n"
                 << "Content-Length: " << resp_str.length() << "\r\n"
                 << "Connection: close\r\n\r\n"
                 << resp_str;
    }
    else
    {
        std::string not_found = "404 Not Found";
        response << "HTTP/1.1 404 Not Found\r\n"
                 << "Content-Type: text/plain\r\n"
                 << "Content-Length: " << not_found.length() << "\r\n"
                 << "Connection: close\r\n\r\n"
                 << not_found;
    }

    std::string response_str = response.str();
    send(client_fd, response_str.c_str(), response_str.length(), 0);
    close(client_fd);
}
