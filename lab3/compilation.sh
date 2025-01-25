sudo rmmod iostat_module.ko
echo "Module unloaded"
make
echo "Compiled"
g++ iostat_user.c -o iostat_user
echo "User compiled"
sudo insmod iostat_module.ko
echo "Module loaded"
lsmod | grep iostat_module
sudo ./iostat_user /dev/sda
echo "End"
