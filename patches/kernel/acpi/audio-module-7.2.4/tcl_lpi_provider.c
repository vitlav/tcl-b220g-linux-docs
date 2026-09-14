// SPDX-License-Identifier: GPL-2.0-only
#include <linux/clk.h>
#include <linux/dmi.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/property.h>

static const struct dmi_system_id tcl_board[] = {
	{
		.matches = {
			DMI_MATCH(DMI_PRODUCT_NAME, "B220G"),
			DMI_MATCH(DMI_BOARD_VENDOR, "TCL"),
		},
	},
	{}
};

static const struct property_entry lpi_props[] = {
	PROPERTY_ENTRY_STRING("compatible", "qcom,sc7280-lpass-lpi-pinctrl"),
	PROPERTY_ENTRY_BOOL("gpio-controller"),
	PROPERTY_ENTRY_U32("#gpio-cells", 2),
	PROPERTY_ENTRY_BOOL("qcom,tcl-acpi-lpi-soundwire"),
	{ }
};

static const struct software_node lpi_node = {
	.name = "tcl-acpi-lpi-pinctrl",
	.properties = lpi_props,
};

static const struct resource lpi_resources[] = {
	DEFINE_RES_MEM(0x627c0000, 0x10000),
	/* The LPI driver adds 0xa000 to this base for its slew register. */
	DEFINE_RES_MEM(0x62950000, 0x10000),
};

static struct platform_device *lpi_pdev;
static struct clk *lpi_core_clk;
static struct clk *lpi_audio_clk;
static bool lpi_core_enabled;
static bool lpi_audio_enabled;

static void lpi_clks_disable(void)
{
	if (lpi_audio_enabled) {
		clk_disable_unprepare(lpi_audio_clk);
		lpi_audio_enabled = false;
	}
	if (lpi_core_enabled) {
		clk_disable_unprepare(lpi_core_clk);
		lpi_core_enabled = false;
	}
}

static void lpi_clks_put(void)
{
	if (lpi_audio_clk)
		clk_put(lpi_audio_clk);
	if (lpi_core_clk)
		clk_put(lpi_core_clk);
	lpi_audio_clk = NULL;
	lpi_core_clk = NULL;
}

static int __init tcl_lpi_provider_init(void)
{
	struct platform_device_info info = {
		.name = "tcl-lpi-provider",
		.id = 0,
		.res = lpi_resources,
		.num_res = ARRAY_SIZE(lpi_resources),
	};
	int ret;

	if (!dmi_check_system(tcl_board))
		return -ENODEV;

	/* These aliases are registered by tcl_acpi_audio. */
	lpi_core_clk = clk_get_sys("tcl-sc7280-rx", "mclk");
	if (IS_ERR(lpi_core_clk)) {
		ret = PTR_ERR(lpi_core_clk);
		lpi_core_clk = NULL;
		return ret;
	}
	lpi_audio_clk = clk_get_sys("tcl-sc7280-rx", "npl");
	if (IS_ERR(lpi_audio_clk)) {
		ret = PTR_ERR(lpi_audio_clk);
		lpi_audio_clk = NULL;
		clk_put(lpi_core_clk);
		lpi_core_clk = NULL;
		return ret;
	}
	ret = clk_prepare_enable(lpi_core_clk);
	if (ret)
		goto err_clks;
	lpi_core_enabled = true;
	ret = clk_prepare_enable(lpi_audio_clk);
	if (ret)
		goto err_clks;
	lpi_audio_enabled = true;

	ret = software_node_register(&lpi_node);
	if (ret)
		goto err_enabled_clks;
	info.fwnode = software_node_fwnode(&lpi_node);
	lpi_pdev = platform_device_register_full(&info);
	if (IS_ERR(lpi_pdev)) {
		ret = PTR_ERR(lpi_pdev);
		lpi_pdev = NULL;
		software_node_unregister(&lpi_node);
		goto err_enabled_clks;
	}

	lpi_clks_disable();
	pr_info("TCL B220G ACPI LPASS LPI pinctrl provider registered\n");
	return 0;

err_enabled_clks:
	lpi_clks_disable();
err_clks:
	lpi_clks_put();
	return ret;
}

static void __exit tcl_lpi_provider_exit(void)
{
	platform_device_unregister(lpi_pdev);
	software_node_unregister(&lpi_node);
	lpi_clks_put();
}

module_init(tcl_lpi_provider_init);
module_exit(tcl_lpi_provider_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCL B220G ACPI LPASS LPI SoundWire pinctrl provider");
