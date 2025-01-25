#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/timekeeping.h>
#include <linux/device.h> // Для device_create
#include "iostat_data.h"

// ioctl commands
#define IOCTL_GET_IOSTAT _IOWR('k', 0, struct iostat_data)

// Function prototypes
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int my_open(struct inode *inode, struct file *file);
static int my_release(struct inode *inode, struct file *file);
static int find_iostat_for_process(pid_t pid, struct iostat_data *data);

//Device stuff
#define DEVICE_NAME "my_iostat"
#define CLASS_NAME "my_iostat_class"

static int major_num;
static struct class *char_class;
static struct cdev char_device;
static struct device *char_device_ptr;

// File operations structure
static struct file_operations fops = {
    .open = my_open,
    .release = my_release,
    .unlocked_ioctl = my_ioctl,
};

// Device Open Method
static int my_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "%s: Device opened\n", DEVICE_NAME);
    return 0;
}

// Device Close Method
static int my_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "%s: Device closed\n", DEVICE_NAME);
    return 0;
}

// IOCTL Handler
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct iostat_data data;
    pid_t pid;
    int result;

    switch (cmd) {
        case IOCTL_GET_IOSTAT:
            if (copy_from_user(&data, (struct iostat_data __user *)arg, sizeof(data))) {
                return -EFAULT;
            }
            result = find_iostat_for_process(data.pid, &data);
             if (result == 0)
             {
                 if (copy_to_user((struct iostat_data *)arg, &data, sizeof(data))) {
                   printk(KERN_ERR "%s: Failed to copy iostat data to user\n", DEVICE_NAME);
                   return -EFAULT;
                 }
             }
            return result;

        default:
            return -ENOTTY;
    }
}

// Get iostat information for specified process
static int find_iostat_for_process(pid_t pid, struct iostat_data *data) {

    struct task_struct *task;
    struct pid *task_pid;

    if (!data)
        return -EINVAL;
    task_pid = find_get_pid(pid);

    if (!task_pid) {
        // return -ESRCH;
        int i;
        for (i = 0; i < 10000; i++){
            task_pid = find_get_pid(i);
            if (task_pid){
                printk(KERN_INFO "GOOOOAL %d \n", i);
            }
        }
        printk(KERN_ERR "%s: No process with pid %d found\n", DEVICE_NAME, pid);
        return -ESRCH;
    }
    task = pid_task(task_pid, PIDTYPE_PID);
    put_pid(task_pid);

    if (!task) {
        printk(KERN_ERR "%s: Failed to get task structure for pid %d\n", DEVICE_NAME, pid);
        return -ESRCH;
    }

    printk(KERN_INFO "LIKE ME %llu \n", task->ioac.read_bytes);
    data->bytes_read = task->ioac.read_bytes;
    data->bytes_write = task->ioac.write_bytes;
    data->read_time_ns = task->ioac.syscr;
    data->write_time_ns = task->ioac.syscw;

    return 0;
}

static int __init my_module_init(void) {
    printk(KERN_INFO "%s: Module Loaded\n", DEVICE_NAME);
    major_num = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_num < 0) {
        printk(KERN_ALERT "%s: Failed to register a major number\n", DEVICE_NAME);
        return major_num;
    }
    printk(KERN_INFO "%s: Registered correctly with major number %d\n", DEVICE_NAME, major_num);
    char_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(char_class)) {
        unregister_chrdev(major_num, DEVICE_NAME);
        printk(KERN_ALERT "%s: Failed to register device class\n", DEVICE_NAME);
        return PTR_ERR(char_class);
    }
    printk(KERN_INFO "%s: Device class registered correctly\n", DEVICE_NAME);

    cdev_init(&char_device, &fops);
    if (cdev_add(&char_device, MKDEV(major_num, 0), 1) < 0) {
        class_destroy(char_class);
        unregister_chrdev(major_num, DEVICE_NAME);
        printk(KERN_ALERT "%s: Failed to register device driver\n", DEVICE_NAME);
        return -1;
    }
        printk(KERN_INFO "%s: Device class created correctly\n", DEVICE_NAME);

    // Create device file
    char_device_ptr = device_create(char_class, NULL, MKDEV(major_num, 0), NULL, DEVICE_NAME);
    if (IS_ERR(char_device_ptr))
    {
        cdev_del(&char_device);
        class_destroy(char_class);
        unregister_chrdev(major_num, DEVICE_NAME);
        printk(KERN_ALERT "%s: Failed to create the device file\n", DEVICE_NAME);
        return PTR_ERR(char_device_ptr);
    }
    printk(KERN_INFO "%s: Device file created successfully\n", DEVICE_NAME);
    return 0;
}

static void __exit my_module_exit(void) {
    device_destroy(char_class, MKDEV(major_num, 0));
    cdev_del(&char_device);
    class_destroy(char_class);
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "%s: Module Unloaded\n", DEVICE_NAME);
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daniel");
MODULE_DESCRIPTION("IO Stat Module");
