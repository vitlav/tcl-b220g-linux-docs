// SPDX-License-Identifier: GPL-2.0-only
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/property.h>

static const struct software_node *rx_node, *tx_node;
static const struct software_node *glink_node, *apr_node, *q6afe_node;
static const struct software_node *rx_master_node, *tx_master_node;
static struct platform_device *codec;
static struct property_entry codec_props[4];

static void tcl_acpi_wcd_put_lookup_nodes(void)
{
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

	rx_node = NULL;
	tx_node = NULL;
	rx_master_node = NULL;
	tx_master_node = NULL;
	q6afe_node = NULL;
	apr_node = NULL;
	glink_node = NULL;
}

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
	/*
	 * Let the platform device own the aggregate software node.  The
	 * referenced RX/TX nodes are owned by tcl_acpi_audio; properties only
	 * need their stable software_node identities, not our lookup references.
	 */
	info.properties = codec_props;
	codec = platform_device_register_full(&info);
	if (IS_ERR(codec)) {
		ret = PTR_ERR(codec);
		codec = NULL;
		goto no_nodes;
	}
	tcl_acpi_wcd_put_lookup_nodes();
	pr_info("TCL ACPI WCD9385 aggregate test device registered\n");
	return 0;

no_nodes:
	tcl_acpi_wcd_put_lookup_nodes();
	return ret ?: -ENODEV;
}

static void __exit tcl_acpi_wcd_exit(void)
{
	if (codec)
		platform_device_unregister(codec);
}

module_init(tcl_acpi_wcd_init);
module_exit(tcl_acpi_wcd_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Temporary ACPI WCD9385 software-node aggregate test");
