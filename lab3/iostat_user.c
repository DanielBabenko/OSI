#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/fs.h>
#include <sys/stat.h>

#define IOCTL_GET_IOSTAT _IOWR('k', 0, struct iostat_data)
#define BDEVNAME_SIZE 256

int main(int argc, char *argv[]) {
    int fd;
    struct iostat_data data;
    struct stat st;

    if(argc != 2){
        fprintf(stderr, "Usage: %s <device_node>\n", argv[0]);
        return 1;
    }
    
    if (stat(argv[1], &st) < 0) {
        fprintf(stderr, "iostat_user: invalid device path\n");
        return 1;
    }
    
    strncpy(data.path, argv[1], sizeof(data.path)-1);
    data.path[sizeof(data.path) - 1] = '\0';

    fd = open("/dev/iostat_device", O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Failed to open device: %s\n", strerror(errno));
        return 1;
    } else {
        printf("Success in opening device\n");
    }
    
    if (ioctl(fd, IOCTL_GET_IOSTAT, &data) < 0) {
        fprintf(stderr, "ioctl failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    printf("I/O Statistics:\n");
    printf("  Read Sectors: %llu\n", data.read_sectors);
    printf("  Write Sectors: %llu\n", data.write_sectors);
    printf("  Read IOs:     %llu\n", data.read_ios);
    printf("  Write IOs:    %llu\n", data.write_ios);
    
    close(fd);
    return 0;
}
