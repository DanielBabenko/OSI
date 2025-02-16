#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include "iostat_data.h"

// ioctl commands
#define IOCTL_GET_IOSTAT _IOWR('k', 0, struct iostat_data)

// Структура для передачи данных пользователю
struct iostat_data {
    pid_t pid;
    unsigned long long bytes_read;
    unsigned long long bytes_write;
    unsigned long long read_time_ns;
    unsigned long long write_time_ns;
};

int main(int argc, char *argv[]) {
    int fd;
    struct iostat_data data;
    pid_t pid;
    char *endptr;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PID>\n", argv[0]);
        return 1;
    }

    pid = strtol(argv[1], &endptr, 10);

    if (*endptr != '\0' || errno == ERANGE) {
         fprintf(stderr, "Invalid PID: %s\n", argv[1]);
         return 1;
    }

    data.pid = pid;

    // Open the device
    fd = open("/dev/my_iostat", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    // // Send the ioctl to kernel module
    // if (ioctl(fd, IOCTL_GET_IOSTAT, &pid) < 0) {
    //     printf("Ola");
    //     perror("Failed to send ioctl");
    //     close(fd);
    //     return 1;
    // }

    if (ioctl(fd, IOCTL_GET_IOSTAT, &data) < 0) {
        printf("Aloha!");
        perror("Failed to receive ioctl");
        close(fd);
        return 1;
    }

    printf("Process: (PID: %d)\n", pid);
    printf("Bytes read: %llu\n", data.bytes_read);
    printf("Bytes written: %llu\n", data.bytes_write);
    printf("Read time (ns): %llu\n", data.read_time_ns);
    printf("Write time (ns): %llu\n", data.write_time_ns);
    
    close(fd);
    return 0;
}
