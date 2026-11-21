#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

static dev_t dev;
static int units = 1;
static struct cdev my_cdev;
static struct class *cl;
static struct device *device;

static const struct file_operations my_fops = {
    .owner = THIS_MODULE,
};

static int __init mymod_init(void)
{
    int err;
    
    printk(KERN_INFO "Hello, kernel!\n");

    err = alloc_chrdev_region(&dev, 0, units, "CourseDriver");

    if (err < 0)
    {
        printk(KERN_ERR "failed to allocate device number\n");
        return err;
    }

    printk(KERN_INFO "allocated major=%d minor=%d\n", MAJOR(dev), MINOR(dev));

    cdev_init(&my_cdev, &my_fops);
    err = cdev_add(&my_cdev, dev, units);

    if (err < 0) 
    {
        printk(KERN_ERR "failed to add cdev\n");
        unregister_chrdev_region(dev, units);
        return err;
    }

    printk(KERN_INFO "cdev registered successfully\n");

    cl = class_create("Saps_class");

    if(cl == 0)
    {
        printk(KERN_ERR "failed to create a class\n");
        return err;
    }

    printk(KERN_INFO "class by the name of Saps_class was created\n",);

    device = device_create(cl, NULL, dev, NULL, "Saps_dev");

    if(device == 0)
    {
        printk(KERN_ERR "failed to create a device\n");
        return err;
    }

    printk(KERN_INFO "device by the name of Saps_dev was created\n");

    return 0;
}

static void __exit mymod_exit(void)
{
    device_destroy(cl, dev);
    class_destroy(cl);


    cdev_del(&my_cdev);

    unregister_chrdev_region(dev, units);
    printk(KERN_INFO "Goodbye, kernel!\n");
    printk(KERN_INFO "mymod unloaded\n");

    return;
}

module_init(mymod_init);
module_exit(mymod_exit);

MODULE_LICENSE("GPL");
MODULE_VERSION("2.0");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("My first Linux kernel module");