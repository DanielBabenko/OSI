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

#ifdef __cplusplus
}
#endif

#endif
