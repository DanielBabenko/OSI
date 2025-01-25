#ifndef IOSTAT_DATA_H
#define IOSTAT_DATA_H

// Структура для передачи данных пользователю
struct iostat_data {
    pid_t pid;
    unsigned long long bytes_read;
    unsigned long long bytes_write;
    unsigned long long read_time_ns;
    unsigned long long write_time_ns;
};

#endif
