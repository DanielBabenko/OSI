#ifndef CACHELIB_H
#define CACHELIB_H

#ifdef __cplusplus
extern "C" {
#endif

//__declspec(dllexport) int add(int a, int b);
//__declspec(dllexport) void greet(const char* name);
__declspec(dllexport) bool cache_init(size_t capacity, const char* disk_file);
__declspec(dllexport) bool cache_read(int block_id, void* buffer);
__declspec(dllexport) bool cache_write(int block_id, const void* buffer);
__declspec(dllexport) void cache_destroy();
__declspec(dllexport) int lab2_open(const char* path);
__declspec(dllexport) int lab2_close(int fd);
__declspec(dllexport) ssize_t lab2_read(int fd, void* buf, size_t count);
__declspec(dllexport) ssize_t lab2_write(int fd, const void* buf, size_t count);

#ifdef __cplusplus
}
#endif

#endif
