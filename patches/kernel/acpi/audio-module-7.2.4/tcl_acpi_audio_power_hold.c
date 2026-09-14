// SPDX-License-Identifier: GPL-2.0-only
#include <linux/dmi.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <soc/qcom/cmd-db.h>
#include <soc/qcom/rpmh.h>

static struct platform_device *ldo_bank;
static struct platform_device *bob_bank;
static bool ldo_on;
static bool bob_on;

static const struct dmi_system_id tcl_board[] = {
	{
		.matches = {
			DMI_MATCH(DMI_PRODUCT_NAME, "B220G"),
			DMI_MATCH(DMI_BOARD_VENDOR, "TCL"),
		},
	},
	{}
};

static struct platform_device *find_rpmh_client(const char *name)
{
	struct device *dev;

	dev = bus_find_device_by_name(&platform_bus_type, NULL, name);
	return dev ? to_platform_device(dev) : NULL;
}

static int rpmh_vote(struct platform_device *bank, u32 addr, u32 value)
{
	struct tcs_cmd cmd = { .addr = addr, .data = value };
	int ret;

	ret = rpmh_write(&bank->dev, RPMH_ACTIVE_ONLY_STATE, &cmd, 1);
	dev_info(&bank->dev, "TCL ACPI audio vote %#x=%u: %d\n",
		 addr, value, ret);
	return ret;
}

static int disable_rails(void)
{
	int ret = 0;
	int err;

	if (bob_on) {
		err = rpmh_vote(bob_bank, 0x40404, 0);
		if (!err)
			bob_on = false;
		else
			ret = err;
	}
	if (ldo_on) {
		err = rpmh_vote(ldo_bank, 0x42204, 0);
		if (!err)
			ldo_on = false;
		else if (!ret)
			ret = err;
	}

	return ret;
}

static void put_banks(void)
{
	if (bob_bank) {
		put_device(&bob_bank->dev);
		bob_bank = NULL;
	}
	if (ldo_bank) {
		put_device(&ldo_bank->dev);
		ldo_bank = NULL;
	}
}

static int __init tcl_audio_power_hold_init(void)
{
	int ret;

	if (!dmi_check_system(tcl_board))
		return -ENODEV;
	if (cmd_db_read_addr("ldoa15") != 0x42200 ||
	    cmd_db_read_addr("bobc1") != 0x40400)
		return -ENODEV;

	ldo_bank = find_rpmh_client("tcl-pm6150-reg");
	bob_bank = find_rpmh_client("tcl-pm6150l-reg");
	if (!ldo_bank || !bob_bank || !ldo_bank->dev.driver ||
	    !bob_bank->dev.driver ||
	    strcmp(ldo_bank->dev.driver->name, "qcom-rpmh-regulator") ||
	    strcmp(bob_bank->dev.driver->name, "qcom-rpmh-regulator") ||
	    !ldo_bank->dev.parent || !bob_bank->dev.parent ||
	    !dev_get_drvdata(ldo_bank->dev.parent) ||
	    !dev_get_drvdata(bob_bank->dev.parent)) {
		ret = -ENODEV;
		goto err_put;
	}

	/* OEM AUDD requests: LDO15_A=1.8 V/HPM and BOB_C=3.3 V/AUTO.
	 * Votes remain active for the module lifetime and are removed on exit.
	 */
	ret = rpmh_vote(ldo_bank, 0x42200, 1800);
	if (ret)
		goto err_put;
	ret = rpmh_vote(ldo_bank, 0x42208, 7);
	if (ret)
		goto err_disable;
	ret = rpmh_vote(ldo_bank, 0x42204, 1);
	if (ret)
		goto err_disable;
	ldo_on = true;

	ret = rpmh_vote(bob_bank, 0x40400, 3300);
	if (ret)
		goto err_disable;
	ret = rpmh_vote(bob_bank, 0x40408, 6);
	if (ret)
		goto err_disable;
	ret = rpmh_vote(bob_bank, 0x40404, 1);
	if (ret)
		goto err_disable;
	bob_on = true;

	pr_info("TCL ACPI audio OEM rails held until module removal\n");
	return 0;

err_disable:
	if (disable_rails())
		pr_err("failed to remove one or more partial audio rail votes\n");
err_put:
	put_banks();
	return ret;
}

static void __exit tcl_audio_power_hold_exit(void)
{
	if (disable_rails())
		pr_err("failed to release one or more audio rail votes\n");
	put_banks();
}

module_init(tcl_audio_power_hold_init);
module_exit(tcl_audio_power_hold_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCL B220G ACPI audio APCC votes held for module lifetime");
