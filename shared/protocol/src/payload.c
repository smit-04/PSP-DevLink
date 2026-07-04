#include "protocol/payload.h"
#include <string.h>

int pspl_serialize_system_stats(
    const PSPDL_SystemStatsPayload *stats,
    uint8_t *buffer,
    size_t size)
{
    if (stats == NULL || buffer == NULL || size < PSPDL_PAYLOAD_SYSTEM_STATS_SIZE)
    {
        return -1;
    }

    buffer[0] = stats->cpu_usage;
    buffer[1] = stats->ram_usage;

    buffer[2] = (uint8_t)(stats->cpu_temp & 0xFF);
    buffer[3] = (uint8_t)((stats->cpu_temp >> 8) & 0xFF);

    // Pack 64-bit ram_total (Little-Endian)
    buffer[4] = (uint8_t)(stats->ram_total & 0xFF);
    buffer[5] = (uint8_t)((stats->ram_total >> 8) & 0xFF);
    buffer[6] = (uint8_t)((stats->ram_total >> 16) & 0xFF);
    buffer[7] = (uint8_t)((stats->ram_total >> 24) & 0xFF);
    buffer[8] = (uint8_t)((stats->ram_total >> 32) & 0xFF);
    buffer[9] = (uint8_t)((stats->ram_total >> 40) & 0xFF);
    buffer[10] = (uint8_t)((stats->ram_total >> 48) & 0xFF);
    buffer[11] = (uint8_t)((stats->ram_total >> 56) & 0xFF);

    // Pack 64-bit ram_free (Little-Endian)
    buffer[12] = (uint8_t)(stats->ram_free & 0xFF);
    buffer[13] = (uint8_t)((stats->ram_free >> 8) & 0xFF);
    buffer[14] = (uint8_t)((stats->ram_free >> 16) & 0xFF);
    buffer[15] = (uint8_t)((stats->ram_free >> 24) & 0xFF);
    buffer[16] = (uint8_t)((stats->ram_free >> 32) & 0xFF);
    buffer[17] = (uint8_t)((stats->ram_free >> 40) & 0xFF);
    buffer[18] = (uint8_t)((stats->ram_free >> 48) & 0xFF);
    buffer[19] = (uint8_t)((stats->ram_free >> 56) & 0xFF);

    return 0;
}

int pspl_deserialize_system_stats(
    const uint8_t *buffer,
    size_t size,
    PSPDL_SystemStatsPayload *stats)
{
    if (buffer == NULL || stats == NULL || size < PSPDL_PAYLOAD_SYSTEM_STATS_SIZE)
    {
        return -1;
    }

    stats->cpu_usage = buffer[0];
    stats->ram_usage = buffer[1];

    stats->cpu_temp = (uint16_t)buffer[2] | ((uint16_t)buffer[3] << 8);

    stats->ram_total = (uint64_t)buffer[4] |
                       ((uint64_t)buffer[5] << 8) |
                       ((uint64_t)buffer[6] << 16) |
                       ((uint64_t)buffer[7] << 24) |
                       ((uint64_t)buffer[8] << 32) |
                       ((uint64_t)buffer[9] << 40) |
                       ((uint64_t)buffer[10] << 48) |
                       ((uint64_t)buffer[11] << 56);

    stats->ram_free = (uint64_t)buffer[12] |
                      ((uint64_t)buffer[13] << 8) |
                      ((uint64_t)buffer[14] << 16) |
                      ((uint64_t)buffer[15] << 24) |
                      ((uint64_t)buffer[16] << 32) |
                      ((uint64_t)buffer[17] << 40) |
                      ((uint64_t)buffer[18] << 48) |
                      ((uint64_t)buffer[19] << 56);

    return 0;
}

int pspl_serialize_git_status(
    const PSPDL_GitStatusPayload *git,
    uint8_t *buffer,
    size_t size)
{
    if (git == NULL || buffer == NULL || size < PSPDL_PAYLOAD_GIT_STATUS_SIZE)
    {
        return -1;
    }

    // Pack 32-bit modified_files
    buffer[0] = (uint8_t)(git->modified_files & 0xFF);
    buffer[1] = (uint8_t)((git->modified_files >> 8) & 0xFF);
    buffer[2] = (uint8_t)((git->modified_files >> 16) & 0xFF);
    buffer[3] = (uint8_t)((git->modified_files >> 24) & 0xFF);

    // Pack 32-bit untracked_files
    buffer[4] = (uint8_t)(git->untracked_files & 0xFF);
    buffer[5] = (uint8_t)((git->untracked_files >> 8) & 0xFF);
    buffer[6] = (uint8_t)((git->untracked_files >> 16) & 0xFF);
    buffer[7] = (uint8_t)((git->untracked_files >> 24) & 0xFF);

    // Pack branch name string (32 bytes) safely, warning-free
    memset(buffer + 8, 0, 32);
    for (int i = 0; i < 31 && git->branch_name[i] != '\0'; i++)
    {
        buffer[8 + i] = (uint8_t)git->branch_name[i];
    }

    return 0;
}

int pspl_deserialize_git_status(
    const uint8_t *buffer,
    size_t size,
    PSPDL_GitStatusPayload *git)
{
    if (buffer == NULL || git == NULL || size < PSPDL_PAYLOAD_GIT_STATUS_SIZE)
    {
        return -1;
    }

    git->modified_files = (uint32_t)buffer[0] |
                          ((uint32_t)buffer[1] << 8) |
                          ((uint32_t)buffer[2] << 16) |
                          ((uint32_t)buffer[3] << 24);

    git->untracked_files = (uint32_t)buffer[4] |
                           ((uint32_t)buffer[5] << 8) |
                           ((uint32_t)buffer[6] << 16) |
                           ((uint32_t)buffer[7] << 24);

    memset(git->branch_name, 0, sizeof(git->branch_name));
    for (int i = 0; i < 31; i++)
    {
        git->branch_name[i] = (char)buffer[8 + i];
    }
    git->branch_name[31] = '\0';

    return 0;
}
