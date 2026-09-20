// SPDX-License-Identifier: GPL-2.0-only
/* TCL SC7180 ACPI audio DSP wiring. DSP startup is a separate userspace step. */
#include <linux/acpi.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/efi.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/platform_data/qcom-tcl-acpi.h>
#include <linux/pm_domain.h>
#include <linux/pm_runtime.h>
#include <linux/property.h>
#include <dt-bindings/power/qcom-rpmpd.h>
#include <dt-bindings/sound/qcom,q6asm.h>

static struct acpi_device *adsp, *glnk;
static struct device *pd, *cc, *apcs, *qmp;
static struct platform_device *smp, *pas, *proxy[2];
static struct qcom_tcl_pas_data pas_data;
static struct clk *xo;
static struct clk_lookup *xo_lookup;
static unsigned int virqs[4];
static unsigned int swr_irqs[2];
static const unsigned int swr_gsis[] = { 329, 328 };
static bool nodes_registered;
static bool ready;
module_param(ready, bool, 0444);
/* Validate only is the default: no resource ownership or DSP startup. */
static bool register_devices;
module_param(register_devices, bool, 0444);

static struct software_node_ref_args smp_mbox[1], edge_mbox[1], qmp_ref[1];
static const u32 smem_ids[] = { 443, 429 };
static const struct property_entry smp_props[] = {
	PROPERTY_ENTRY_U32_ARRAY("qcom,smem", smem_ids),
	PROPERTY_ENTRY_U32("qcom,local-pid", 0),
	PROPERTY_ENTRY_U32("qcom,remote-pid", 2),
	PROPERTY_ENTRY_REF_ARRAY("mboxes", smp_mbox), { }
};
static const struct software_node smp_node = {
	.name = "tcl-adsp-smp2p", .properties = smp_props,
};
static const struct property_entry in_props[] = {
	PROPERTY_ENTRY_STRING("qcom,entry-name", "slave-kernel"),
	PROPERTY_ENTRY_BOOL("interrupt-controller"),
	PROPERTY_ENTRY_U32("#interrupt-cells", 2), { }
};
static const struct software_node in_node = {
	.name = "slave-kernel", .parent = &smp_node, .properties = in_props,
};
static const struct property_entry out_props[] = {
	PROPERTY_ENTRY_STRING("qcom,entry-name", "master-kernel"),
	PROPERTY_ENTRY_U32("#qcom,smem-state-cells", 1), { }
};
static const struct software_node out_node = {
	.name = "master-kernel", .parent = &smp_node, .properties = out_props,
};
static const struct property_entry edge_props[] = {
	PROPERTY_ENTRY_STRING("label", "lpass"),
	PROPERTY_ENTRY_U32("qcom,remote-pid", 2),
	PROPERTY_ENTRY_REF_ARRAY("mboxes", edge_mbox), { }
};
static const struct software_node edge_node = {
	.name = "tcl-adsp-glink", .properties = edge_props,
};
static const struct property_entry apr_props[] = {
	PROPERTY_ENTRY_STRING("compatible", "qcom,apr-v2"),
	PROPERTY_ENTRY_STRING("qcom,glink-channels", "apr_audio_svc"),
	PROPERTY_ENTRY_U32("qcom,domain", 4), { }
};
static const struct software_node apr_node = {
	.name = "apr", .parent = &edge_node, .properties = apr_props,
};
static const char * const audio_pd[] = { "avs/audio", "msm/adsp/audio_pd" };
#define APR_SERVICE(_name, _id) \
static const struct property_entry _name##_props[] = { \
	PROPERTY_ENTRY_STRING("compatible", "qcom," #_name), \
	PROPERTY_ENTRY_U32("reg", _id), \
	PROPERTY_ENTRY_STRING_ARRAY("qcom,protection-domain", audio_pd), { } \
}; \
static const struct software_node _name##_node = { \
	.name = #_name, .parent = &apr_node, .properties = _name##_props, \
}
APR_SERVICE(q6core, 3);
APR_SERVICE(q6afe, 4);
APR_SERVICE(q6asm, 7);
APR_SERVICE(q6adm, 8);

static const char * const afe_clock_aliases[] = {
	"LPASS_CLK_ID_VA_CORE_MCLK", "tcl-sc7280-va", "mclk",
	"LPASS_HW_MACRO", "tcl-sc7280-va", "macro",
	"LPASS_HW_DCODEC", "tcl-sc7280-va", "dcodec",
	"LPASS_CLK_ID_RX_CORE_MCLK", "tcl-sc7280-rx", "mclk",
	"LPASS_CLK_ID_RX_CORE_NPL_MCLK", "tcl-sc7280-rx", "npl",
	"LPASS_HW_MACRO", "tcl-sc7280-rx", "macro",
	"LPASS_HW_DCODEC", "tcl-sc7280-rx", "dcodec",
	"LPASS_CLK_ID_TX_CORE_MCLK", "tcl-sc7280-tx", "mclk",
	"LPASS_CLK_ID_TX_CORE_NPL_MCLK", "tcl-sc7280-tx", "npl",
	"LPASS_HW_MACRO", "tcl-sc7280-tx", "macro",
	"LPASS_HW_DCODEC", "tcl-sc7280-tx", "dcodec",
};
static const struct property_entry afe_clock_props[] = {
	PROPERTY_ENTRY_STRING("compatible", "qcom,q6afe-clocks"),
	PROPERTY_ENTRY_U32("qcom,clock-attribute", 1),
	PROPERTY_ENTRY_STRING_ARRAY("qcom,clock-aliases", afe_clock_aliases), { }
};
static const struct software_node afe_clock_node = {
	.name = "clocks", .parent = &q6afe_node, .properties = afe_clock_props,
};
static const struct property_entry afe_dai_props[] = {
	PROPERTY_ENTRY_STRING("compatible", "qcom,q6afe-dais"), { }
};
static const struct software_node afe_dai_node = {
	.name = "dais", .parent = &q6afe_node, .properties = afe_dai_props,
};

#define LPASS_MACRO_NODE(_name, _compat, _base) \
static const struct property_entry _name##_props[] = { \
	PROPERTY_ENTRY_STRING("compatible", _compat), \
	PROPERTY_ENTRY_U64("reg-base", _base), \
	PROPERTY_ENTRY_U64("reg-size", 0x1000), { } \
}; \
static const struct software_node _name##_node = { \
	.name = #_name, .parent = &q6afe_node, .properties = _name##_props, \
}
static const struct property_entry va_macro_hw_props[] = {
	PROPERTY_ENTRY_STRING("compatible", "qcom,sc7280-lpass-va-macro"),
	PROPERTY_ENTRY_U64("reg-base", 0x62770000),
	PROPERTY_ENTRY_U64("reg-size", 0x1000),
	PROPERTY_ENTRY_U32("qcom,dmic-sample-rate", 4800000),
	PROPERTY_ENTRY_STRING("clock-output-names", "fsgen"), { }
};
static const struct software_node va_macro_hw_node = {
	.name = "va_macro_hw", .parent = &q6afe_node,
	.properties = va_macro_hw_props,
};
LPASS_MACRO_NODE(rx_macro_hw, "qcom,sc7280-lpass-rx-macro", 0x62600000);
LPASS_MACRO_NODE(tx_macro_hw, "qcom,sc7280-lpass-tx-macro", 0x62620000);
#undef LPASS_MACRO_NODE

static const struct property_entry asm_dai_props[] = {
	PROPERTY_ENTRY_STRING("compatible", "qcom,q6asm-dais"),
	PROPERTY_ENTRY_BOOL("qcom,tcl-acpi-audio"), { }
};
static const struct software_node asm_dai_node = {
	.name = "dais", .parent = &q6asm_node, .properties = asm_dai_props,
};
static const struct property_entry asm_pcm_props[] = {
	PROPERTY_ENTRY_U32("reg", 0),
	PROPERTY_ENTRY_U32("direction", Q6ASM_DAI_TX_RX), { }
};
static const struct software_node asm_pcm_node = {
	.name = "dai-0", .parent = &asm_dai_node, .properties = asm_pcm_props,
};

/* Verified RX/TX port layouts; hardware devices are registered in a later stage. */
static const u8 swr_rx_offset1[] = { 0x0, 0x0, 0xb, 0x1, 0x0 };
static const u8 swr_rx_offset2[] = { 0x0, 0x0, 0xb, 0x0, 0x0 };
static const u8 swr_rx_sinterval_low[] = { 0x3, 0x1f, 0x1f, 0x7, 0x0 };
static const u8 swr_rx_block_pack_mode[] = { 0xff, 0x0, 0x1, 0xff, 0xff };
static const u8 swr_rx_hstart[] = { 0xff, 0x3, 0xff, 0xff, 0xff };
static const u8 swr_rx_hstop[] = { 0xff, 0x6, 0xff, 0xff, 0xff };
static const u8 swr_rx_word_length[] = { 0x1, 0x7, 0x4, 0xff, 0xff };
static const u8 swr_rx_block_group_count[] = { 0xff, 0xff, 0xff, 0xff, 0x0 };
static const u8 swr_rx_lane_control[] = { 0x1, 0x0, 0x0, 0x0, 0x0 };
static struct property_entry swr_rx_props[] = {
	PROPERTY_ENTRY_STRING("compatible", "qcom,soundwire-v1.5.1"),
	PROPERTY_ENTRY_U64("reg-base", 0x62610000),
	PROPERTY_ENTRY_U64("reg-size", 0x2000),
	PROPERTY_ENTRY_U32("irq", 329),
	PROPERTY_ENTRY_STRING("label", "RX"),
	PROPERTY_ENTRY_U32("qcom,din-ports", 0),
	PROPERTY_ENTRY_U32("qcom,dout-ports", 5),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-offset1", swr_rx_offset1),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-offset2", swr_rx_offset2),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-sinterval-low", swr_rx_sinterval_low),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-block-pack-mode", swr_rx_block_pack_mode),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-hstart", swr_rx_hstart),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-hstop", swr_rx_hstop),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-word-length", swr_rx_word_length),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-block-group-count", swr_rx_block_group_count),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-lane-control", swr_rx_lane_control),
	{ }
};
static const struct software_node swr_rx_node = {
	.name = "tcl-swr-rx", .parent = &q6afe_node, .properties = swr_rx_props,
};
static const u32 wcd_rx_map[] = { 1, 2, 3, 4, 5 };
static const struct property_entry wcd_rx_props[] = {
	PROPERTY_ENTRY_U64("mipi-sdw-address", 0x240217010d00ULL),
	PROPERTY_ENTRY_U32_ARRAY("qcom,rx-port-mapping", wcd_rx_map), { }
};
static const struct software_node wcd_rx_node = {
	.name = "wcd-rx", .parent = &swr_rx_node, .properties = wcd_rx_props,
};
static const u8 swr_tx_offset1[] = { 0xff, 0x1, 0x0, 0x2, 0x0 };
static const u8 swr_tx_offset2[] = { 0xff, 0x0, 0x0, 0x0, 0x0 };
static const u8 swr_tx_sinterval_low[] = { 0xff, 0x1, 0x1, 0x3, 0x3 };
static const u8 swr_tx_block_pack_mode[] = { 0xff, 0xff, 0xff, 0xff, 0xff };
static const u8 swr_tx_hstart[] = { 0xff, 0xff, 0xff, 0xff, 0xff };
static const u8 swr_tx_hstop[] = { 0xff, 0xff, 0xff, 0xff, 0xff };
static const u8 swr_tx_word_length[] = { 0xff, 0xff, 0xff, 0xff, 0xff };
static const u8 swr_tx_block_group_count[] = { 0xff, 0xff, 0xff, 0xff, 0xff };
static const u8 swr_tx_lane_control[] = { 0xff, 0x0, 0x1, 0x0, 0x1 };
static struct property_entry swr_tx_props[] = {
	PROPERTY_ENTRY_STRING("compatible", "qcom,soundwire-v1.5.1"),
	PROPERTY_ENTRY_U64("reg-base", 0x62630000),
	PROPERTY_ENTRY_U64("reg-size", 0x2000),
	PROPERTY_ENTRY_U32("irq", 328),
	PROPERTY_ENTRY_STRING("label", "TX"),
	PROPERTY_ENTRY_U32("qcom,din-ports", 5),
	PROPERTY_ENTRY_U32("qcom,dout-ports", 0),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-offset1", swr_tx_offset1),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-offset2", swr_tx_offset2),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-sinterval-low", swr_tx_sinterval_low),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-block-pack-mode", swr_tx_block_pack_mode),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-hstart", swr_tx_hstart),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-hstop", swr_tx_hstop),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-word-length", swr_tx_word_length),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-block-group-count", swr_tx_block_group_count),
	PROPERTY_ENTRY_U8_ARRAY("qcom,ports-lane-control", swr_tx_lane_control),
	{ }
};
static const struct software_node swr_tx_node = {
	.name = "tcl-swr-tx", .parent = &q6afe_node, .properties = swr_tx_props,
};
static const u32 wcd_tx_map[] = { 2, 3, 4, 5 };
static const struct property_entry wcd_tx_props[] = {
	PROPERTY_ENTRY_U64("mipi-sdw-address", 0x230217010d00ULL),
	PROPERTY_ENTRY_U32_ARRAY("qcom,tx-port-mapping", wcd_tx_map), { }
};
static const struct software_node wcd_tx_node = {
	.name = "wcd-tx", .parent = &swr_tx_node, .properties = wcd_tx_props,
};

static const struct property_entry pas_props[] = {
	PROPERTY_ENTRY_STRING("firmware-name", "qcom/sc7180/tcl/b220g/qcadsp7180.mbn"),
	PROPERTY_ENTRY_REF_ARRAY("qcom,qmp", qmp_ref),
	PROPERTY_ENTRY_REF("qcom,smem-states", &out_node, 0),
	PROPERTY_ENTRY_STRING("qcom,smem-state-names", "stop"),
	PROPERTY_ENTRY_BOOL("qcom,tcl-no-interconnect"), { }
};
static const struct software_node pas_node = {
	.name = "tcl-adsp", .properties = pas_props,
};
static const struct software_node * const nodes[] = {
	&smp_node, &in_node, &out_node, &edge_node, &apr_node,
	&q6core_node, &q6afe_node, &q6asm_node, &q6adm_node,
	&afe_clock_node, &afe_dai_node, &asm_dai_node, &asm_pcm_node,
	&va_macro_hw_node, &rx_macro_hw_node, &tx_macro_hw_node,
	&swr_rx_node, &wcd_rx_node, &swr_tx_node, &wcd_tx_node,
	&pas_node, NULL,
};

static struct acpi_device *find_acpi(const char *path, const char *hid)
{
	struct acpi_device *adev;
	acpi_handle handle;

	if (ACPI_FAILURE(acpi_get_handle(NULL, (acpi_string)path, &handle)))
		return NULL;
	adev = acpi_get_acpi_dev(handle);
	if (adev && strcmp(acpi_device_hid(adev), hid)) {
		acpi_dev_put(adev);
		return NULL;
	}
	return adev;
}

static struct device *provider(const char *name)
{
	struct device *dev = bus_find_device_by_name(&platform_bus_type, NULL, name);
	bool bound;

	if (!dev)
		return NULL;
	device_lock(dev);
	bound = device_is_bound(dev);
	device_unlock(dev);
	if (!bound) {
		put_device(dev);
		return NULL;
	}
	return dev;
}

static int firmware_irq(struct acpi_device *adev, unsigned int index,
			unsigned int gsi, const char *name, struct resource *res)
{
	struct irq_data *data;
	int ret;

	memset(res, 0, sizeof(*res));
	ret = acpi_irq_get(adev->handle, index, res);
	if (ret)
		return ret;
	data = irq_get_irq_data(res->start);
	if (!data || irqd_to_hwirq(data) != gsi ||
	    irq_get_trigger_type(res->start) != IRQ_TYPE_EDGE_RISING)
		return -EINVAL;
	res->name = name;
	return 0;
}

static int proxy_domain(unsigned int slot, unsigned int index, const char *name)
{
	struct genpd_onecell_data *data = dev_get_drvdata(pd);
	int ret;

	if (!data || index >= data->num_domains || !data->domains[index])
		return -EINVAL;
	proxy[slot] = platform_device_register_simple(name, PLATFORM_DEVID_NONE, NULL, 0);
	if (IS_ERR(proxy[slot])) {
		ret = PTR_ERR(proxy[slot]);
		proxy[slot] = NULL;
		return ret;
	}
	ret = pm_genpd_add_device(data->domains[index], &proxy[slot]->dev);
	if (ret) {
		platform_device_unregister(proxy[slot]);
		proxy[slot] = NULL;
		return ret;
	}
	pm_runtime_enable(&proxy[slot]->dev);
	pas_data.proxy_pds[slot] = &proxy[slot]->dev;
	return 0;
}

static void cleanup(void)
{
	int i;

	if (pas)
		platform_device_unregister(pas);
	for (i = 0; i < ARRAY_SIZE(virqs); i++)
		if (virqs[i])
			irq_dispose_mapping(virqs[i]);
	if (smp)
		platform_device_unregister(smp);
	if (xo_lookup)
		clkdev_drop(xo_lookup);
	if (xo)
		clk_put(xo);
	for (i = 1; i >= 0; i--)
		if (proxy[i]) {
			pm_runtime_disable(&proxy[i]->dev);
			pm_genpd_remove_device(&proxy[i]->dev);
			platform_device_unregister(proxy[i]);
		}
	if (nodes_registered)
		software_node_unregister_node_group(nodes);
	for (i = 0; i < ARRAY_SIZE(swr_irqs); i++)
		if (swr_irqs[i]) {
			acpi_unregister_gsi(swr_gsis[i]);
			swr_irqs[i] = 0;
		}
	if (qmp)
		put_device(qmp);
	if (apcs)
		put_device(apcs);
	if (cc)
		put_device(cc);
	if (pd)
		put_device(pd);
	if (glnk)
		acpi_dev_put(glnk);
	if (adsp)
		acpi_dev_put(adsp);
}

static int __init tcl_audio_init(void)
{
	const struct software_node *apcs_node, *qmp_node;
	struct acpi_table_header *table;
	struct resource smp_irq, edge_irq;
	struct resource res[2] = {
		DEFINE_RES_MEM_NAMED(0x90b00000, 0x2800000, "firmware"),
	};
	static const char * const names[] = { "fatal", "ready", "handover", "stop-ack" };
	struct resource pas_res[6];
	struct platform_device_info info = { };
	struct irq_fwspec spec = { .param_count = 2 };
	efi_memory_desc_t md;
	bool match;
	int ret, i;

	if (acpi_disabled || !efi_enabled(EFI_BOOT) ||
	    ACPI_FAILURE(acpi_get_table(ACPI_SIG_IORT, 0, &table)))
		return -ENODEV;
	match = !memcmp(table->oem_id, "QCOM  ", ACPI_OEM_ID_SIZE) &&
		!memcmp(table->oem_table_id, "QCOMEDK2", ACPI_OEM_TABLE_ID_SIZE) &&
		table->oem_revision == 0x7180;
	acpi_put_table(table);
	if (!match || efi_mem_desc_lookup(0x90b00000, &md) ||
	    md.type != EFI_RESERVED_TYPE || md.phys_addr > 0x90b00000 ||
	    md.num_pages > (U64_MAX >> EFI_PAGE_SHIFT) ||
	    (md.num_pages << EFI_PAGE_SHIFT) < 0x93300000 - md.phys_addr)
		return -EINVAL;
	adsp = find_acpi("\\_SB.ADSP", "QCOM081D");
	glnk = find_acpi("\\_SB.GLNK", "QCOM088D");
	ret = -ENODEV;
	if (!adsp || !glnk)
		goto fail;
	ret = firmware_irq(adsp, 0, 194, "wdog", &res[1]);
	if (!ret)
		ret = firmware_irq(glnk, 1, 190, "smp2p", &smp_irq);
	if (!ret)
		ret = firmware_irq(glnk, 4, 188, "glink", &edge_irq);
	if (ret)
		goto fail;
	pd = provider("qcom-sc7180-rpmhpd");
	cc = provider("qcom-sc7180-rpmhcc");
	apcs = provider("qcom-apss-shared");
	qmp = provider("qcom_aoss_qmp");
	apcs_node = software_node_find_by_name(NULL, "tcl-apss");
	qmp_node = software_node_find_by_name(NULL, "tcl-aoss");
	ret = -ENODEV;
	if (!pd || !cc || !apcs || !qmp || !apcs_node || !qmp_node)
		goto fail;
	pr_info("TCL ACPI audio: EFI ADSP reservation and IRQs 194/190/188 verified; providers bound\n");
	if (!register_devices)
		return 0;
	ret = acpi_register_gsi(NULL, swr_gsis[0], ACPI_EDGE_SENSITIVE,
				ACPI_ACTIVE_HIGH);
	if (ret < 0)
		goto fail;
	swr_irqs[0] = ret;
	ret = acpi_register_gsi(NULL, swr_gsis[1], ACPI_EDGE_SENSITIVE,
				ACPI_ACTIVE_HIGH);
	if (ret < 0)
		goto fail;
	swr_irqs[1] = ret;
	swr_rx_props[3].value.u32_data[0] = swr_irqs[0];
	swr_tx_props[3].value.u32_data[0] = swr_irqs[1];
	pr_info("TCL ACPI audio: SoundWire GSI %u/%u mapped to IRQ %u/%u\n",
		swr_gsis[0], swr_gsis[1], swr_irqs[0], swr_irqs[1]);
	smp_mbox[0] = SOFTWARE_NODE_REFERENCE(apcs_node, 10);
	edge_mbox[0] = SOFTWARE_NODE_REFERENCE(apcs_node, 8);
	qmp_ref[0] = SOFTWARE_NODE_REFERENCE(qmp_node);
	ret = software_node_register_node_group(nodes);
	if (ret)
		goto fail;
	nodes_registered = true;
	ret = proxy_domain(0, SC7180_LCX, "tcl-adsp-lcx");
	if (!ret)
		ret = proxy_domain(1, SC7180_LMX, "tcl-adsp-lmx");
	if (ret)
		goto fail;
	xo = clk_get_sys(dev_name(cc), "bi_tcxo");
	if (IS_ERR(xo)) {
		ret = PTR_ERR(xo);
		xo = NULL;
		goto fail;
	}
	xo_lookup = clkdev_create(xo, "xo", "tcl-sc7180-adsp");
	if (!xo_lookup) {
		ret = -ENOMEM;
		goto fail;
	}
	info.name = "qcom_smp2p";
	info.id = PLATFORM_DEVID_AUTO;
	info.parent = &glnk->dev;
	info.fwnode = software_node_fwnode(&smp_node);
	info.res = &smp_irq;
	info.num_res = 1;
	smp = platform_device_register_full(&info);
	if (IS_ERR(smp)) {
		ret = PTR_ERR(smp);
		smp = NULL;
		goto fail;
	}
	device_lock(&smp->dev);
	match = device_is_bound(&smp->dev);
	device_unlock(&smp->dev);
	if (!match) {
		ret = -EPROBE_DEFER;
		goto fail;
	}
	pas_res[0] = res[0];
	pas_res[1] = res[1];
	spec.fwnode = software_node_fwnode(&in_node);
	for (i = 0; i < ARRAY_SIZE(virqs); i++) {
		spec.param[0] = i;
		spec.param[1] = IRQ_TYPE_EDGE_RISING;
		virqs[i] = irq_create_fwspec_mapping(&spec);
		if (!virqs[i]) {
			ret = -ENODEV;
			goto fail;
		}
		pas_res[i + 2] = (struct resource)DEFINE_RES_IRQ_NAMED(virqs[i], names[i]);
	}
	pas_data.glink_node = software_node_fwnode(&edge_node);
	pas_data.glink_irq = edge_irq.start;
	info = (struct platform_device_info) {
		.name = "tcl-sc7180-adsp", .id = PLATFORM_DEVID_NONE,
		.parent = &adsp->dev, .fwnode = software_node_fwnode(&pas_node),
		.res = pas_res, .num_res = ARRAY_SIZE(pas_res),
		.data = &pas_data, .size_data = sizeof(pas_data),
	};
	pas = platform_device_register_full(&info);
	if (IS_ERR(pas)) {
		ret = PTR_ERR(pas);
		pas = NULL;
		goto fail;
	}
	/* The exit path unregisters PAS first, stopping ADSP before its fwnodes go. */
	device_lock(&pas->dev);
	ready = device_is_bound(&pas->dev);
	device_unlock(&pas->dev);
	pr_info("TCL ACPI audio: ADSP resources registered, bound=%u; explicit remoteproc start required\n", ready);
	return 0;
fail:
	pr_err("TCL ACPI audio: preparation failed: %d; unwinding\n", ret);
	cleanup();
	return ret;
}
module_init(tcl_audio_init);
module_exit(cleanup);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCL ACPI ADSP resource preparation");
