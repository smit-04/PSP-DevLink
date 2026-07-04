#ifndef PSPDL_SYSTEM_SERVICE_H
#define PSPDL_SYSTEM_SERVICE_H

#include <stdint.h>

struct SystemMetrics
{
    uint8_t cpu_usage;    // 0-100 %
    uint8_t ram_usage;    // 0-100 %
    uint16_t cpu_temp;    // tenths of a degree C (e.g. 450 = 45.0 C)
    uint64_t ram_total;   // bytes
    uint64_t ram_free;    // bytes
};

class SystemService
{
public:
    SystemService();
    SystemMetrics get_metrics();

private:
    unsigned long long last_user;
    unsigned long long last_user_low;
    unsigned long long last_sys;
    unsigned long long last_idle;
};

#endif // PSPDL_SYSTEM_SERVICE_H
