// SPDX-License-Identifier: GPL-2.0-only
#include <linux/delay.h>
#include <linux/dmi.h>
#include <linux/io.h>
#include <linux/module.h>

#define TCL_GPIO58_BASE 0x0353a000
#define TCL_GPIO58_SIZE 0x14

static const struct dmi_system_id tcl_board[] = {
	{
		.matches = {
			DMI_MATCH(DMI_PRODUCT_NAME, "B220G"),
			DMI_MATCH(DMI_BOARD_VENDOR, "TCL"),
		},
	},
	{}
};

static void __iomem *gpio58;
static bool active;

static void restore_gpio58(void)
{
	if (!active)
		return;

	/* Restore the guarded pre-test input/pulldown state. */
	writel(0, gpio58 + 4);
	readl(gpio58 + 4);
	writel(1, gpio58);
	readl(gpio58);
	active = false;
	pr_info("TCL GPIO58 restored cfg=%#x io=%#x\n",
		readl(gpio58), readl(gpio58 + 4));
}

static int __init tcl_codec_reset_hold_init(void)
{
	u32 present, cfg, io;

	if (!dmi_check_system(tcl_board))
		return -ENODEV;

	gpio58 = ioremap(TCL_GPIO58_BASE, TCL_GPIO58_SIZE);
	if (!gpio58)
		return -ENOMEM;

	present = readl(gpio58 + 0x10);
	cfg = readl(gpio58);
	io = readl(gpio58 + 4);
	if (!(present & 1) || cfg != 1 || io != 0) {
		pr_err("GPIO58 guard failed present=%#x cfg=%#x io=%#x; no writes\n",
		       present, cfg, io);
		iounmap(gpio58);
		gpio58 = NULL;
		return -EBUSY;
	}

	/* OEM reset: assert low for 5 ms, release high, wait 2 ms.
	 * Keep the line muxed as an output until module removal.
	 */
	writel(0, gpio58 + 4);
	writel(0x200, gpio58);
	readl(gpio58);
	active = true;
	msleep(5);
	writel(2, gpio58 + 4);
	readl(gpio58 + 4);
	msleep(2);
	pr_info("TCL GPIO58 codec reset released cfg=%#x io=%#x\n",
		readl(gpio58), readl(gpio58 + 4));
	return 0;
}

static void __exit tcl_codec_reset_hold_exit(void)
{
	restore_gpio58();
	if (gpio58)
		iounmap(gpio58);
}

module_init(tcl_codec_reset_hold_init);
module_exit(tcl_codec_reset_hold_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCL B220G guarded GPIO58 reset held until module removal");
