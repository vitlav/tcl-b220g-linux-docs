// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/gpio/driver.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio/machine.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/machine.h>
#include <linux/pinctrl/pinconf-generic.h>
static unsigned long configs[] = { PIN_CONF_PACKED(PIN_CONFIG_BIAS_PULL_UP, 1) };
static const struct pinctrl_map maps[] = {
 PIN_MAP_CONFIGS_GROUP("tcl-sar-pins", "default", "3500000.pinctrl", "gpio34", configs),
};
static int __init start(void)
{
 struct platform_device *pdev;
 struct pinctrl *pctl;
 struct device_node *expected;
 struct gpio_device *gdev;
 struct gpio_chip *chip;
 struct gpio_desc *pin;
 struct i2c_adapter *adap;
 struct i2c_client *client;
 struct i2c_board_info info = { I2C_BOARD_INFO("aw96105", 0x12) };
 int ret;
 if (!of_machine_is_compatible("tcl,book14")) return -ENODEV;
 adap = i2c_get_adapter(4);
 if (!adap) return -ENODEV;
 expected = of_find_node_by_path("/soc@0/geniqup@8c0000/i2c@890000");
 ret = expected && adap->dev.of_node == expected;
 of_node_put(expected);
 if (!ret) {
  ret = -EINVAL; goto put_adapter;
 }
 gdev = gpio_device_find_by_label("3500000.pinctrl");
 if (!gdev) { ret = -ENODEV; goto put_adapter; }
 chip = gpio_device_get_chip(gdev);
 if (!chip) { ret = -ENODEV; goto put_gpio; }
 pin = gpiochip_request_own_desc(chip, 34, "tcl-sar-irq", GPIO_LOOKUP_FLAGS_DEFAULT, GPIOD_IN);
 if (IS_ERR(pin)) { ret = PTR_ERR(pin); goto put_gpio; }
 ret = pinctrl_register_mappings(maps, ARRAY_SIZE(maps));
 if (ret) goto free_pin;
 pdev = platform_device_register_simple("tcl-sar-pins", -1, NULL, 0);
 if (IS_ERR(pdev)) { ret = PTR_ERR(pdev); goto unmap; }
 pctl = pinctrl_get_select_default(&pdev->dev);
 if (IS_ERR(pctl)) { ret = PTR_ERR(pctl); goto unregister; }
 ret = gpiod_to_irq(pin);
 if (ret < 0) goto put_pinctrl;
 info.irq = ret;
 ret = irq_set_irq_type(info.irq, IRQ_TYPE_EDGE_BOTH);
 if (ret) goto put_pinctrl;
 client = i2c_new_client_device(adap, &info);
 if (IS_ERR(client)) { ret = PTR_ERR(client); goto put_pinctrl; }
 pr_info("TCL_SAR registered %s IRQ%d; existing powered rail retained; diagnostic client has no unload\n", dev_name(&client->dev), info.irq);
 i2c_put_adapter(adap);
 return 0;
put_pinctrl:
 pinctrl_put(pctl);
unregister:
 platform_device_unregister(pdev);
unmap:
 pinctrl_unregister_mappings(maps);
free_pin:
 gpiochip_free_own_desc(pin);
put_gpio:
 gpio_device_put(gdev);
put_adapter:
 i2c_put_adapter(adap);
 return ret;
}
module_init(start);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCL AW96105 diagnostic I2C client, GPIO34 from OEM ACPI; no power switching");
