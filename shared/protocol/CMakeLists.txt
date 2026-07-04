# Shared protocol module
#
# This module currently exports only the public protocol interfaces.
# Platform-specific transport implementations are compiled by each
# application (Desktop and PSP) rather than by this module.
#
# Shared protocol source files (packet serialization, framing, message
# processing, etc.) will be added in future milestones.

add_library(pspdevlink_protocol STATIC
    src/packet.c
    src/payload.c
)

target_include_directories(pspdevlink_protocol
    PUBLIC
        include
)