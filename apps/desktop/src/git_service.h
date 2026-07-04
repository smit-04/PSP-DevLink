#ifndef PSPDL_GIT_SERVICE_H
#define PSPDL_GIT_SERVICE_H

#include <string>
#include <stdint.h>

struct GitMetrics
{
    std::string branch_name;
    uint32_t modified_files;
    uint32_t untracked_files;
    bool is_repository;
};

class GitService
{
public:
    GitService();
    GitMetrics get_metrics();
};

#endif // PSPDL_GIT_SERVICE_H
