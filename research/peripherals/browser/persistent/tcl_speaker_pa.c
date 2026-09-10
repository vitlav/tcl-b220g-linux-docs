// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/gpio/driver.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio/machine.h>
#include <linux/of.h>
#include <linux/workqueue.h>
#include <linux/string.h>
static struct gpio_desc *pins[2];
static unsigned int hold_seconds = 5;
module_param(hold_seconds, uint, 0444);
static struct gpio_device *gdev;
static void off(struct work_struct *work)
{
 int i;
 for (i = 0; i < 2; i++)
  if (pins[i]) gpiod_set_value_cansleep(pins[i], 0);
 pr_info("TCL_PA GPIO46/47 low\n");
}
static DECLARE_DELAYED_WORK(timer, off);
static void release(void)
{
 int i;
 off(NULL);
 for (i = 0; i < 2; i++) {
  if (pins[i]) gpiochip_free_own_desc(pins[i]);
  pins[i] = NULL;
 }
 if (gdev) gpio_device_put(gdev);
 gdev = NULL;
}
static int __init start(void)
{
 struct gpio_chip *chip;
 struct gpio_desc *desc;
 int i, ret;
 if (!of_machine_is_compatible("tcl,book14")) return -ENODEV;
 if (hold_seconds > 1200) return -EINVAL;
 gdev = gpio_device_find_by_label("3500000.pinctrl");
 if (!gdev) return -ENODEV;
 chip = gpio_device_get_chip(gdev);
 if (!chip) { release(); return -ENODEV; }
 for (i = 0; i < 2; i++) {
  desc = gpiochip_request_own_desc(chip, 46 + i, "tcl-speaker-pa-test", GPIO_LOOKUP_FLAGS_DEFAULT, GPIOD_ASIS);
  if (IS_ERR(desc)) { ret = PTR_ERR(desc); release(); return ret; }
  if (gpiod_get_direction(desc) != 0 || gpiod_get_raw_value_cansleep(desc) != 0) {
   gpiochip_free_own_desc(desc); release(); return -EBUSY;
  }
  pins[i] = desc;
 }
 if (hold_seconds)
  schedule_delayed_work(&timer, msecs_to_jiffies(hold_seconds * 1000));
 gpiod_set_value_cansleep(pins[0], 1);
 gpiod_set_value_cansleep(pins[1], 1);
 if (hold_seconds)
  pr_info("TCL_PA GPIO46/47 high, automatic off in %u seconds\n", hold_seconds);
 else
  pr_info("TCL_PA GPIO46/47 high, managed until module unload\n");
 return 0;
}
static void __exit stop(void)
{
 cancel_delayed_work_sync(&timer);
 release();
}
module_init(start);
module_exit(stop);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCL speaker PA GPIO46/47; hold_seconds=0 for service-managed lifetime");
