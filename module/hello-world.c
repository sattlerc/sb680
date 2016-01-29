#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("Dual\ MIT/GPL");
MODULE_AUTHOR("Christian Sattler");
MODULE_DESCRIPTION("A skeleton device driver for the SMART interactive whiteboard SB680");
MODULE_VERSION("0.1");

static int __init init() {
  return 0;
}

static void __exit exit() {
}

module_init(init);
module_exit(exit);

