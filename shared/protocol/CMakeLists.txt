add_library(pspdevlink_protocol STATIC
    src/transport_stub.c
)

target_include_directories(pspdevlink_protocol
    PUBLIC
        include
)
