#include "notification_service.h"
#include <iostream>
#include <cstdio>
#include <memory>
#include <array>
#include <chrono>
#include <vector>
#include <sys/select.h>
#include <unistd.h>

NotificationService::NotificationService() : m_running(true) {
    m_thread = std::thread(&NotificationService::run_listener, this);
}

NotificationService::~NotificationService() {
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

NotificationMetrics NotificationService::get_latest() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) {
        return { "", "", "", false };
    }
    NotificationMetrics next = m_queue.front();
    m_queue.pop();
    return next;
}

void NotificationService::push_mock_notification(const std::string& app, const std::string& title, const std::string& msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    NotificationMetrics metric = { app, title, msg, true };
    m_queue.push(metric);
}

void NotificationService::run_listener() {
    std::string cmd = "dbus-monitor --session \"type='method_call',interface='org.freedesktop.Notifications',member='Notify'\" 2>/dev/null";
    FILE* pipe = popen(cmd.c_str(), "r");
    
    bool use_fallback = false;
    if (!pipe) {
        use_fallback = true;
    }

    auto last_mock_time = std::chrono::steady_clock::now();
    size_t mock_index = 0;
    std::vector<std::pair<std::string, std::pair<std::string, std::string>>> mocks = {
        {"Slack", {"New message from @alex", "Hey, did you finish compiling the PSP EBOOT?"}},
        {"GitHub", {"PR #42 Approved", "User approved Milestone 8 merge request"}},
        {"System", {"Low battery warning", "Battery level is currently at 12%"}},
        {"Terminal", {"Build Success", "Target EBOOT.PBP compiled in 4.2 seconds"}}
    };

    char line_buf[512];
    std::string app_name, summary, body;
    int string_index = -1;
    bool in_notify = false;

    while (m_running) {
        if (use_fallback) {
            // Simulated notification generation every 10 seconds
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_mock_time).count() >= 10) {
                auto& mock = mocks[mock_index];
                push_mock_notification(mock.first, mock.second.first, mock.second.second);
                mock_index = (mock_index + 1) % mocks.size();
                last_mock_time = now;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        int fd = fileno(pipe);
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100ms timeout
        
        int retval = select(fd + 1, &fds, NULL, NULL, &tv);
        if (retval == -1) {
            use_fallback = true;
            pclose(pipe);
            pipe = NULL;
            continue;
        } else if (retval == 0) {
            // Timeout, loop back to check m_running
            continue;
        }

        if (fgets(line_buf, sizeof(line_buf), pipe) == NULL) {
            use_fallback = true;
            pclose(pipe);
            pipe = NULL;
            continue;
        }

        std::string line(line_buf);
        if (line.find("member=Notify") != std::string::npos) {
            in_notify = true;
            string_index = 0;
            app_name = "";
            summary = "";
            body = "";
        } else if (in_notify) {
            size_t str_pos = line.find("string \"");
            if (str_pos != std::string::npos) {
                size_t end_pos = line.rfind("\"");
                if (end_pos != std::string::npos && end_pos > str_pos + 8) {
                    std::string value = line.substr(str_pos + 8, end_pos - (str_pos + 8));
                    if (string_index == 0) {
                        app_name = value;
                    } else if (string_index == 1) {
                        // Skip icon
                    } else if (string_index == 2) {
                        summary = value;
                    } else if (string_index == 3) {
                        body = value;
                        push_mock_notification(app_name, summary, body);
                        in_notify = false;
                    }
                    string_index++;
                }
            } else if (line.find("method call") != std::string::npos || line.find("signal") != std::string::npos) {
                in_notify = false;
            }
        }
    }
    
    if (pipe) {
        pclose(pipe);
    }
}
