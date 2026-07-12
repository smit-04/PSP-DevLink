#ifndef PSPDL_DRIVER_IOCTL_H
#define PSPDL_DRIVER_IOCTL_H

#define PSPDL_IOCTL_INIT     0x1000
#define PSPDL_IOCTL_SEND     0x1001
#define PSPDL_IOCTL_RECV     0x1002
#define PSPDL_IOCTL_SHUTDOWN 0x1003

struct pspdl_send_args {
    const void *data;
    int size;
};

struct pspdl_recv_args {
    void *buf;
    int size;
    int *received;
};

#endif
