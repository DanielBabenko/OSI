g++ -c -Wall -Wextra -fPIC cache_lib.cpp -o cache_lib.o
g++ -shared -o cache_lib.dll cache_lib.o -Wl,--out-implib,libcache_lib.dll.a
g++ test.cpp -o main.exe -L. -lcache_lib
g++ test2.cpp -o main2.exe -L. -lcache_lib
