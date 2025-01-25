#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/string.h>
#include <linux/seq_file.h>
#include <linux/kdev_t.h>
#include <linux/stat.h>

#define IOCTL_GET_IOSTAT _IOWR('k', 0, struct iostat_data)
#define SYSFS_PATH_MAX 256
#define BDEVNAME_SIZE 256

struct iostat_data {
    unsigned long long read_sectors;
    unsigned long long write_sectors;
    unsigned long long read_ios;
    unsigned long long write_ios;
    char path[BDEVNAME_SIZE];
};

static dev_t dev_num;
static struct class* dev_class;
static struct cdev cdev;
static struct device* device;


static int read_sysfs_file(const char* path, unsigned long long* value) {
    struct file* f = filp_open(path, O_RDONLY, 0);
    char buf[32];
    int ret = 0;
    loff_t pos = 0;
    if (IS_ERR(f)) {
        return -PTR_ERR(f);
    }

    ret = kernel_read(f, buf, sizeof(buf)-1, &pos);
    printk(KERN_INFO "OK\n");
    printk(KERN_INFO "iRet: %d\n", ret);
    if (ret < 0) {
        filp_close(f, NULL);
        return ret;
    }
    buf[ret] = '\0';
    *value = simple_strtoull(buf, NULL, 10);
    filp_close(f, NULL);
    return 0;
}

static long device_ioctl(struct file* file, unsigned int cmd, unsigned long arg) {
    struct iostat_data data;
    struct block_device *bdev = NULL;
    char sysfs_path[SYSFS_PATH_MAX];
    int ret = 0;

     if (_IOC_TYPE(cmd) != 'k' || _IOC_NR(cmd) != 0) {
        return -ENOTTY;
    }

     if (cmd == IOCTL_GET_IOSTAT) {
        if (copy_from_user(&data, (struct iostat_data __user *)arg, sizeof(data))) {
             return -EFAULT;
        }

        bdev = blkdev_get_by_path(data.path, FMODE_READ | FMODE_WRITE, NULL);
        if (IS_ERR(bdev)) {
           return PTR_ERR(bdev);
        }

        printk(KERN_INFO "iostat_module: block device name: %s\n", bdev->bd_disk->disk_name);
        snprintf(sysfs_path, SYSFS_PATH_MAX, "/sys/block/%s/stat", bdev->bd_disk->disk_name);
         
        unsigned long long values[11];

         ret = read_sysfs_file(sysfs_path, &values[0]);
         if(ret < 0) {
              goto out;
         }

          data.read_ios = values[0];
          data.read_sectors = values[2];
          data.write_ios = values[4];
          data.write_sectors = values[6];


         if (copy_to_user((struct iostat_data __user *)arg, &data, sizeof(data))) {
              ret = -EFAULT;
              goto out;
         }


      out:
           blkdev_put(bdev, FMODE_READ);
           return ret;

    }
    return -ENOTTY;
}
 
static int device_open(struct inode* inode, struct file* file) { 
    printk(KERN_INFO "Device opened.\n"); 
    return 0; 
} 
 
static int device_release(struct inode* inode, struct file* file) { 
    printk(KERN_INFO "Device closed.\n"); 
    return 0; 
} 

static const struct file_operations fops = { 
    .owner = THIS_MODULE, 
    .open = device_open, 
    .release = device_release, 
    .unlocked_ioctl = device_ioctl, 
}; 
  
static int __init my_module_init(void) { 
    if (alloc_chrdev_region(&dev_num, 0, 1, "iostat_device") < 0) { 
        return -1; 
    } 
 
    if ((dev_class = class_create(THIS_MODULE, "iostat_device_class")) == NULL) { 
        unregister_chrdev_region(dev_num, 1); 
        return -1; 
    } 
 
    if ((device = device_create(dev_class, NULL, dev_num, NULL, "iostat_device")) == NULL) { 
        class_destroy(dev_class); 
        unregister_chrdev_region(dev_num, 1); 
        return -1; 
    } 
 
    cdev_init(&cdev, &fops); 
    if (cdev_add(&cdev, dev_num, 1) < 0) { 
        device_destroy(dev_class, dev_num); 
        class_destroy(dev_class); 
        unregister_chrdev_region(dev_num, 1); 
        return -1; 
    } 
 
    printk(KERN_INFO "Module loaded!\n"); 
    return 0; 
} 
  
static void __exit my_module_exit(void) { 
    cdev_del(&cdev); 
    device_destroy(dev_class, dev_num); 
    class_destroy(dev_class); 
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "Module unloaded!\n"); 
} 
 
module_init(my_module_init); 
module_exit(my_module_exit); 
 
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Daniel"); 
MODULE_DESCRIPTION("IOStat Kernel Module");
