#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/cdev.h>

static dev_t dev;
static int units = 1;
static struct cdev my_cdev;

static const struct file_operations my_fops = {
    .owner = THIS_MODULE,
};

static int __init mymod_init(void)
{
    int err;
    
    printk(KERN_INFO "Hello, kernel!\n");
    printk(KERN_INFO "mymod has loaded\n");

    err = alloc_chrdev_region(&dev, 0, units, "CourseDriver");

    cdev_init(&my_cdev, &my_fops);
    cdev_add(&my_cdev, dev, units);

    return 0;
}

static void __exit mymod_exit(void)
{
    cdev_del(&my_cdev);

    unregister_chrdev_region(dev, units);
    printk(KERN_INFO "Goodbye, kernel!\n");

    return;
}

module_init(mymod_init);
module_exit(mymod_exit);

MODULE_LICENSE("GPL");
MODULE_VERSION("2.0");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("My first Linux kernel module");