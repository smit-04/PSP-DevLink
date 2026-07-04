#ifndef PSPDL_DESKTOP_NOTIFICATION_SERVICE_H
#define PSPDL_DESKTOP_NOTIFICATION_SERVICE_H

#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>

struct NotificationMetrics {
    std::string app_name;
    std::string summary;
    std::string body;
    bool is_new;
};

class NotificationService {
public:
    NotificationService();
    ~NotificationService();
    NotificationMetrics get_latest();

private:
    void run_listener();
    void push_mock_notification(const std::string& app, const std::string& title, const std::string& msg);

    std::thread m_thread;
    std::atomic<bool> m_running;
    std::queue<NotificationMetrics> m_queue;
    std::mutex m_mutex;
};

#endif // PSPDL_DESKTOP_NOTIFICATION_SERVICE_H
