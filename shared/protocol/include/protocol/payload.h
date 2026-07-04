#ifndef PSPDL_PROTOCOL_PAYLOAD_H
#define PSPDL_PROTOCOL_PAYLOAD_H

#include <stdint.h>
#include <stddef.h>

#define PSPDL_PAYLOAD_SYSTEM_STATS_SIZE 20
#define PSPDL_PAYLOAD_GIT_STATUS_SIZE 40
#define PSPDL_PAYLOAD_NOTIFICATION_SIZE 128
#define PSPDL_PAYLOAD_CONTROL_SIZE 1

typedef struct
{
    uint8_t cpu_usage;
    uint8_t ram_usage;
    uint16_t cpu_temp;
    uint64_t ram_total;
    uint64_t ram_free;
} PSPDL_SystemStatsPayload;

typedef struct
{
    uint32_t modified_files;
    uint32_t untracked_files;
    char branch_name[32];
} PSPDL_GitStatusPayload;

typedef struct
{
    char app_name[24];
    char summary[52];
    char body[52];
} PSPDL_NotificationPayload;

typedef struct
{
    uint8_t command_id;
} PSPDL_ControlPayload;

#ifdef __cplusplus
extern "C" {
#endif

int pspl_serialize_system_stats(
    const PSPDL_SystemStatsPayload *stats,
    uint8_t *buffer,
    size_t size);

int pspl_deserialize_system_stats(
    const uint8_t *buffer,
    size_t size,
    PSPDL_SystemStatsPayload *stats);

int pspl_serialize_git_status(
    const PSPDL_GitStatusPayload *git,
    uint8_t *buffer,
    size_t size);

int pspl_deserialize_git_status(
    const uint8_t *buffer,
    size_t size,
    PSPDL_GitStatusPayload *git);

int pspl_serialize_notification(
    const PSPDL_NotificationPayload *notif,
    uint8_t *buffer,
    size_t size);

int pspl_deserialize_notification(
    const uint8_t *buffer,
    size_t size,
    PSPDL_NotificationPayload *notif);

int pspl_serialize_control(
    const PSPDL_ControlPayload *ctrl,
    uint8_t *buffer,
    size_t size);

int pspl_deserialize_control(
    const uint8_t *buffer,
    size_t size,
    PSPDL_ControlPayload *ctrl);

#ifdef __cplusplus
}
#endif

#endif /* PSPDL_PROTOCOL_PAYLOAD_H */
