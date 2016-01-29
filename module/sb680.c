#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Christian Sattler");
MODULE_DESCRIPTION("A skeleton device driver for the SMART interactive whiteboard SB680");
MODULE_VERSION("0.1");

static int __init sb680_init(void) {
  printk(KERN_INFO "sb680: initialized");
  return 0;
}

static void __exit sb680_exit(void) {
  printk(KERN_INFO "sb680: exited");
}

module_init(sb680_init);
module_exit(sb680_exit);
