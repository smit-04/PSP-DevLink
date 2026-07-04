#include "git_service.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <algorithm>
#include <cctype>

GitService::GitService() {}

static std::string execute_command(const std::string &cmd)
{
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        return "";
    }
    while (fgets(buffer, sizeof(buffer), pipe) != NULL)
    {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

static std::string trim_spaces(std::string str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), str.end());
    return str;
}

GitMetrics GitService::get_metrics()
{
    GitMetrics metrics;
    metrics.branch_name = "None";
    metrics.modified_files = 0;
    metrics.untracked_files = 0;
    metrics.is_repository = false;

    // Check if we are inside a Git repository
    std::string check_repo = execute_command("git rev-parse --is-inside-work-tree 2>/dev/null");
    check_repo = trim_spaces(check_repo);
    if (check_repo != "true")
    {
        return metrics;
    }

    metrics.is_repository = true;

    // Get active branch name
    std::string branch = execute_command("git rev-parse --abbrev-ref HEAD 2>/dev/null");
    branch = trim_spaces(branch);
    if (!branch.empty())
    {
        metrics.branch_name = branch;
    }

    // Get status modifications counts
    std::string status = execute_command("git status --porcelain 2>/dev/null");
    std::stringstream ss(status);
    std::string line;
    while (std::getline(ss, line))
    {
        if (line.size() >= 2)
        {
            if (line[0] == '?' && line[1] == '?')
            {
                metrics.untracked_files++;
            }
            else
            {
                metrics.modified_files++;
            }
        }
    }

    return metrics;
}
