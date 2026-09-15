// SPDX-License-Identifier: GPL-2.0-only
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/property.h>

static const struct software_node *rx_node, *tx_node;
static const struct software_node *glink_node, *apr_node, *q6afe_node;
static const struct software_node *rx_master_node, *tx_master_node;
static struct platform_device *codec;
static struct property_entry codec_props[4];
static struct software_node codec_node = {
	.name = "tcl-acpi-wcd9385-test",
	.properties = codec_props,
};

static int __init tcl_acpi_wcd_init(void)
{
	struct platform_device_info info = {
		.name = "wcd938x_codec",
		.id = PLATFORM_DEVID_NONE,
	};
	int ret = -ENODEV;

	glink_node = software_node_find_by_name(NULL, "tcl-adsp-glink");
	if (glink_node)
		apr_node = software_node_find_by_name(glink_node, "apr");
	if (apr_node)
		q6afe_node = software_node_find_by_name(apr_node, "q6afe");
	if (q6afe_node) {
		rx_master_node = software_node_find_by_name(q6afe_node, "tcl-swr-rx");
		tx_master_node = software_node_find_by_name(q6afe_node, "tcl-swr-tx");
	}
	if (rx_master_node)
		rx_node = software_node_find_by_name(rx_master_node, "wcd-rx");
	if (tx_master_node)
		tx_node = software_node_find_by_name(tx_master_node, "wcd-tx");
	if (!rx_node || !tx_node)
		goto no_nodes;
	codec_props[0] = PROPERTY_ENTRY_BOOL("qcom,tcl-acpi-audio-test");
	codec_props[1] = PROPERTY_ENTRY_REF("qcom,rx-device", rx_node);
	codec_props[2] = PROPERTY_ENTRY_REF("qcom,tx-device", tx_node);
	codec_props[3] = (struct property_entry) { };
	ret = software_node_register(&codec_node);
	if (ret)
		goto no_nodes;
	info.fwnode = software_node_fwnode(&codec_node);
	codec = platform_device_register_full(&info);
	if (IS_ERR(codec)) {
		ret = PTR_ERR(codec);
		codec = NULL;
		software_node_unregister(&codec_node);
		goto no_nodes;
	}
	fwnode_handle_put(software_node_fwnode(glink_node));
	fwnode_handle_put(software_node_fwnode(apr_node));
	fwnode_handle_put(software_node_fwnode(q6afe_node));
	fwnode_handle_put(software_node_fwnode(rx_master_node));
	fwnode_handle_put(software_node_fwnode(tx_master_node));
	pr_info("TCL ACPI WCD9385 aggregate test device registered\n");
	return 0;

no_nodes:
	if (tx_node)
		fwnode_handle_put(software_node_fwnode(tx_node));
	if (rx_node)
		fwnode_handle_put(software_node_fwnode(rx_node));
	if (tx_master_node)
		fwnode_handle_put(software_node_fwnode(tx_master_node));
	if (rx_master_node)
		fwnode_handle_put(software_node_fwnode(rx_master_node));
	if (q6afe_node)
		fwnode_handle_put(software_node_fwnode(q6afe_node));
	if (apr_node)
		fwnode_handle_put(software_node_fwnode(apr_node));
	if (glink_node)
		fwnode_handle_put(software_node_fwnode(glink_node));
	return ret ?: -ENODEV;
}

static void __exit tcl_acpi_wcd_exit(void)
{
	if (codec)
		platform_device_unregister(codec);
	software_node_unregister(&codec_node);
	fwnode_handle_put(software_node_fwnode(tx_node));
	fwnode_handle_put(software_node_fwnode(rx_node));
}

module_init(tcl_acpi_wcd_init);
module_exit(tcl_acpi_wcd_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Temporary ACPI WCD9385 software-node aggregate test");
