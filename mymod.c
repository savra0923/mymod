#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define B_SIZE 4096

static dev_t dev;
static int units = 1;
static int open_count = 0;
static int release_count = 0;

static struct mod_private{
    struct cdev mod_cdev;
    struct class *mod_cl;
    struct device *mod_device;

    char buffer[B_SIZE];
    size_t head;
    size_t tail;
    size_t count;
}mod_private;
static struct mod_private mod_private;

static int my_open(struct inode *ino, struct file *flip)
{
    flip->private_data = container_of(ino->i_cdev, struct mod_private, mod_cdev);
    open_count++;

    printk(KERN_INFO "OPENING A FILE!\n");
    printk(KERN_INFO "OPEN called %d times\n", open_count);

    return 0;
}

static int my_release(struct inode *ino, struct file *file)
{
    printk(KERN_INFO "CLOSING A FILE!\n");
    release_count++;
    printk(KERN_INFO "RELEASE called %d times\n", release_count);

    return 0;
}

static ssize_t my_read(struct file *flip, char __user *buff, size_t count, loff_t *offp)
{
    struct mod_private *currDev = flip->private_data;
    size_t avail;
    size_t to_read;
    size_t first_part;
    size_t second_part;

    avail = B_SIZE - currDev->count;
    if (avail == 0)
    {
        printk(KERN_ERR "NO SPACE LEFT TO READ\n");
        return -ENOSPC;
    }

    to_read = avail;
    if(to_read > count)
        to_read = count;

    first_part = to_read;
    if(first_part > (B_SIZE - currDev->head))
        first_part = (B_SIZE - currDev->head);

    second_part = to_read - first_part;

    if (copy_to_user(&currDev->buffer[currDev->head], buff, first_part))
    {
        printk(KERN_ERR "user-space pointer could NOT be accessed\n");
        return -EFAULT;
    }

    if (second_part > 0 && copy_to_user(&currDev->buffer[0], buff + first_part, second_part))
    {
        printk(KERN_ERR "user-space pointer could NOT be accessed\n");
        return -EFAULT;
    }

    currDev->head = (currDev->head + to_read) % B_SIZE;
    currDev->count -= to_read;

    printk(KERN_INFO "mymod: wrote %zu bytes, buffer contains %zu bytes\n", to_read, currDev->count);

    return to_read;
}

static ssize_t my_write(struct file *flip, const char __user *buff, size_t count, loff_t *offp)
{
    struct mod_private *currDev = flip->private_data;
    size_t avail;
    size_t to_write;
    size_t first_part;
    size_t second_part;

    avail = B_SIZE - currDev->count;
    if (avail == 0)
    {
        printk(KERN_ERR "NO SPACE LEFT\n");
        return -ENOSPC;
    }

    to_write = avail;
    if(to_write > count)
        to_write = count;

    first_part = to_write;
    if(first_part > (B_SIZE - currDev->head))
        first_part = (B_SIZE - currDev->head);

    second_part = to_write - first_part;

    if (copy_from_user(&currDev->buffer[currDev->head], buff, first_part))
    {
        printk(KERN_ERR "user-space pointer could NOT be accessed\n");
        return -EFAULT;
    }

    if (second_part > 0 && copy_from_user(&currDev->buffer[0], buff + first_part, second_part))
    {
        printk(KERN_ERR "user-space pointer could NOT be accessed\n");
        return -EFAULT;
    }

    currDev->head = (currDev->head + to_write) % B_SIZE;
    currDev->count += to_write;

    printk(KERN_INFO "mymod: wrote %zu bytes, buffer contains %zu bytes\n", to_write, currDev->count);

    return to_write;

}

static const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
};

static int __init mymod_init(void)
{
    int err;
    mod_private.head = 0;
    mod_private.tail = 0;
    mod_private.count = 0;
    
    printk(KERN_INFO "Hello, kernel!\n");

    err = alloc_chrdev_region(&dev, 0, units, "CourseDriver");

    if (err < 0)
    {
        printk(KERN_ERR "failed to allocate device number\n");
        return err;
    }

    printk(KERN_INFO "allocated major=%d minor=%d\n", MAJOR(dev), MINOR(dev));

    cdev_init(&mod_private.mod_cdev, &my_fops);
    err = cdev_add(&mod_private.mod_cdev, dev, units);

    if (err < 0) 
    {
        printk(KERN_ERR "failed to add cdev\n");
        unregister_chrdev_region(dev, units);
        return err;
    }

    printk(KERN_INFO "cdev registered successfully\n");

    mod_private.mod_cl = class_create("Saps_class");

    if(mod_private.mod_cl == 0)
    {
        printk(KERN_ERR "failed to create a class\n");
        return err;
    }

    printk(KERN_INFO "class by the name of Saps_class was created\n");

    mod_private.mod_device = device_create(mod_private.mod_cl, NULL, dev, NULL, "Saps_dev");

    if(mod_private.mod_device == 0)
    {
        printk(KERN_ERR "failed to create a device\n");
        return err;
    }

    printk(KERN_INFO "device by the name of Saps_dev was created\n");

    return 0;
}

static void __exit mymod_exit(void)
{
    device_destroy(mod_private.mod_cl, dev);
    class_destroy(mod_private.mod_cl);

    cdev_del(&mod_private.mod_cdev);

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