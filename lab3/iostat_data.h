#ifndef IOSTAT_DATA_H
#define IOSTAT_DATA_H

#define BDEVNAME_SIZE 256

struct iostat_data {
    unsigned long long read_sectors;
    unsigned long long write_sectors;
    unsigned long long read_ios;
    unsigned long long write_ios;
    char path[BDEVNAME_SIZE];
};

#endif
