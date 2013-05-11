#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define DEVICE_NAME "gpio"

static const struct file_operations gpio_fops = {
  .llseek = memory_lseek,
  .read = read_mem,
  .write = write_mem,
  .mmap = mmap_mem,
  .open = open_mem,
  .get_unmapped_area = get_unmapped_area_mem,
};

 static const struct memdev {
         umode_t mode;
         const struct file_operations *fops;
         struct backing_dev_info *dev_info;
 } devlist = { 0, &gpio_fops, &directly_mappable_cdev_bdi };

static int major;
static struct class * class;
static dev_t devt;

static int __init init_gpio(void)
{
  major = register_chrdev(0, DEVICE_NAME, &gpio_fops);
  if (major < 0) {
    printk(KERN_ERR DEVICE_NAME "unable to get a major dev number: %d\n", major);
    return major;
  }

  class = class_create(THIS_MODULE, "mem");
  if (IS_ERR(class)) {
    printk(KERN_ERR DEVICE_NAME "unable to class_create: %d\n", PTR_ERR(class));
    return PTR_ERR(class);
  }

  int minor = 1;
  devt = MVDEV(major, minor);

  struct device *parent = NULL;
  void *drvdata = NULL;
  const char *fmt = DEVICE_NAME;
  struct device * device;
  device = device_create(class, parent, devt, drvdata, fmt);
  if (IS_ERR(device)) {
    printk(KERN_ERR DEVICE_NAME "unable to device_create: %d\n", PTR_ERR(device));
    return PTR_ERR(device);
  }

  device_create(class, NULL, MKDEV(MEM_MAJOR, minor), NULL, "gpio");

  printk(KERN_INFO DEVICE_NAME "module loaded\n");

  return 0;
}

static void __exit cleanup_gpio(void)
{
  device_destroy(class, devt);
  class_destroy(class);

  int ret = unregister_chrdev(Major, DEVICE_NAME);
  if (ret < 0)
    printk(KERN_ALERT DEVICE_NAME "unable to unregister_chrdev: %d\n", ret);

  printk(KERN_INFO DEVICE_NAME "module unloaded\n");
}

module_init(init_gpio);
module_exit(cleanup_gpio);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Luís Reis <luis.m.reis@gmail.com>");
MODULE_AUTHOR("Igor Soarez <igorsoarez@gmail.com>");
MODULE_DESCRIPTION("GPIO mmap'able module");
MODULE_SUPPORTED_DEVICE(DEVICE_NAME);
MODULE_VERSION("0.0.0");
