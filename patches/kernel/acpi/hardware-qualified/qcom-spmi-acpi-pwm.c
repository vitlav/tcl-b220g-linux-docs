// SPDX-License-Identifier: GPL-2.0
/* ACPI resource adapter. The normal PMIC arbiter owns the transport. */
#include <linux/acpi.h>
#include <linux/component.h>
#include <linux/dmi.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/regmap.h>
#include <linux/pwm.h>
#include <linux/spmi.h>
#include <soc/qcom/qcom-spmi-pmic.h>
#include <linux/soc/qcom/irq.h>

static bool enable;
module_param(enable, bool, 0400);
MODULE_PARM_DESC(enable, "Enable TCL SPMI resource adapter (requires platform-aware arbiter)");

struct owner_test {
	bool master;
	struct acpi_device *pm01;
	int irq;
	struct device *pdc;
	struct irq_domain *domain;
	struct irq_data original;
	u32 pin_config;
	bool pushed;
	struct fwnode_handle *fwnode;
	struct platform_device *child;
	struct spmi_controller *ctrl;
	struct spmi_device *pmic[2];
	struct fwnode_handle *pmic_node[2];
	struct platform_device *lpg;
	struct platform_device *pwm_consumer;
	struct fwnode_handle *pwm_consumer_node;
	struct pwm_device *pwm;
	struct fwnode_handle *lpg_node;
};

static int owner_component_bind(struct device *dev, struct device *master,
				void *data)
{
	struct owner_test *state = data;
	struct resource *res;
	struct irq_data *irqd;

	res = platform_get_resource(to_platform_device(dev), IORESOURCE_IRQ, 0);
	if (!res || res->start != res->end || res->start > INT_MAX)
		return -EINVAL;
	/* Inspect the already allocated mapping; do not create or request one. */
	irqd = irq_get_irq_data(res->start);
	if (!irqd || irqd->hwirq != 513 || !irqd->domain ||
	    !is_fwnode_irqchip(irqd->domain->fwnode))
		return -EINVAL;

	state->irq = res->start;
	dev_info(master, "owner-test: PM01 IRQ %d INTID %lu type %#x; unchanged\n",
		 state->irq, irqd->hwirq, irq_get_trigger_type(state->irq));
	return 0;
}

static void owner_component_unbind(struct device *dev, struct device *master,
				  void *data)
{
	struct owner_test *state = data;

	state->irq = 0;
	dev_info(master, "owner-test: component released; IRQ unchanged\n");
}

static const struct component_ops owner_component_ops = {
	.bind = owner_component_bind,
	.unbind = owner_component_unbind,
};

static void owner_release_pmic(struct owner_test *state)
{
	int i;

	if (state->pwm) {
		pwm_put(state->pwm);
		state->pwm = NULL;
	}
	if (state->pwm_consumer) {
		platform_device_unregister(state->pwm_consumer);
		state->pwm_consumer = NULL;
	}
	if (state->pwm_consumer_node) {
		fwnode_remove_software_node(state->pwm_consumer_node);
		state->pwm_consumer_node = NULL;
	}

	if (state->lpg) {
		platform_device_unregister(state->lpg);
		state->lpg = NULL;
	}
	if (state->lpg_node) {
		fwnode_remove_software_node(state->lpg_node);
		state->lpg_node = NULL;
	}
	for (i = 1; i >= 0; i--) {
		if (state->pmic[i]) {
			spmi_device_remove(state->pmic[i]);
			state->pmic[i] = NULL;
		}
		if (state->pmic_node[i]) {
			fwnode_remove_software_node(state->pmic_node[i]);
			state->pmic_node[i] = NULL;
		}
	}
	spmi_controller_put(state->ctrl);
	state->ctrl = NULL;
}

static int owner_register_pmic(struct device *dev)
{
	static const struct property_entry pmic_props[] = {
		PROPERTY_ENTRY_STRING("compatible", "qcom,pm8150l"),
		{ }
	};
	static const struct property_entry lpg_props[] = {
		PROPERTY_ENTRY_U32("#pwm-cells", 3),
		{ }
	};
	struct software_node_ref_args pwm_ref = {
		.swnode = NULL,
		.args = { 3, 853333, 0 },
		.nargs = 3,
	};
	struct property_entry consumer_props[2];
	struct pwm_state pwm_state;
	static const char * const polarity[] = { "normal", "inversed" };
	struct owner_test *state = dev_get_drvdata(dev);
	struct platform_device_info info = {
		.name = "pm8150l-lpg",
		.id = PLATFORM_DEVID_NONE,
	};
	const struct qcom_spmi_pmic *revision;
	struct spmi_device *sdev;
	struct regmap *map;
	u8 pwm[7];
	int i, ret;

	/* Explicit loading also supports configurations with modular drivers. */
	if (IS_MODULE(CONFIG_MFD_SPMI_PMIC)) {
		ret = request_module("spmi:spmi-pmic");
		if (ret)
			return ret;
	}
	ret = request_module("platform:pm8150l-lpg");
	if (ret)
		return ret;
	state->ctrl = spmi_find_controller_by_parent(&state->child->dev);
	if (!state->ctrl)
		return -EPROBE_DEFER;
	for (i = 0; i < 2; i++) {
		state->pmic_node[i] = fwnode_create_software_node(pmic_props, NULL);
		if (IS_ERR(state->pmic_node[i])) {
			ret = PTR_ERR(state->pmic_node[i]);
			state->pmic_node[i] = NULL;
			return ret;
		}
		sdev = spmi_device_alloc(state->ctrl);
		if (!sdev)
			return -ENOMEM;
		sdev->usid = 4 + i;
		device_set_node(&sdev->dev, state->pmic_node[i]);
		ret = spmi_device_add(sdev);
		if (ret) {
			spmi_device_put(sdev);
			return ret;
		}
		state->pmic[i] = sdev;
		if (!device_is_bound(&sdev->dev) || !dev_get_regmap(&sdev->dev, NULL))
			return -EPROBE_DEFER;
	}
	state->lpg_node = fwnode_create_software_node(lpg_props, NULL);
	if (IS_ERR(state->lpg_node)) {
		ret = PTR_ERR(state->lpg_node);
		state->lpg_node = NULL;
		return ret;
	}
	info.parent = &state->pmic[1]->dev;
	info.fwnode = state->lpg_node;
	state->lpg = platform_device_register_full(&info);
	if (IS_ERR(state->lpg)) {
		ret = PTR_ERR(state->lpg);
		state->lpg = NULL;
		return ret;
	}
	if (!device_is_bound(&state->lpg->dev))
		return -EPROBE_DEFER;
	revision = qcom_pmic_get(&state->lpg->dev);
	if (IS_ERR(revision))
		return PTR_ERR(revision);
	if (revision->subtype != PM8150L_SUBTYPE)
		return -ENODEV;
	map = dev_get_regmap(info.parent, NULL);
	if (!map)
		return -ENXIO;
	ret = regmap_bulk_read(map, 0xbc40, pwm, sizeof(pwm));
	if (ret)
		return ret;
	pwm_ref.swnode = to_software_node(state->lpg_node);
	consumer_props[0] = (struct property_entry)PROPERTY_ENTRY_REF("pwms", &pwm_ref);
	consumer_props[1] = (struct property_entry) { };
	state->pwm_consumer_node = fwnode_create_software_node(consumer_props, NULL);
	if (IS_ERR(state->pwm_consumer_node)) {
		ret = PTR_ERR(state->pwm_consumer_node);
		state->pwm_consumer_node = NULL;
		return ret;
	}
	info.parent = &state->lpg->dev;
	info.name = "tcl-lpg-pwm-readback";
	info.fwnode = state->pwm_consumer_node;
	info.num_res = 0;
	info.res = NULL;
	state->pwm_consumer = platform_device_register_full(&info);
	if (IS_ERR(state->pwm_consumer)) {
		ret = PTR_ERR(state->pwm_consumer);
		state->pwm_consumer = NULL;
		return ret;
	}
	state->pwm = pwm_get(&state->pwm_consumer->dev, NULL);
	if (IS_ERR(state->pwm)) {
		ret = PTR_ERR(state->pwm);
		state->pwm = NULL;
		return ret;
	}
	ret = pwm_get_state_hw(state->pwm, &pwm_state);
	if (ret)
		return ret;
	dev_info(dev, "PMIC %s revision %u.%u SID4/5; LPG PWM raw=%*ph; API period=%llu duty=%llu enabled=%u polarity=%s (read only)\n",
		 revision->name, revision->major, revision->minor,
		 (int)sizeof(pwm), pwm, pwm_state.period, pwm_state.duty_cycle,
		 pwm_state.enabled, polarity[pwm_state.polarity]);
	return 0;
}

static void owner_release_transport(struct device *dev)
{
	struct owner_test *state = dev_get_drvdata(dev);
	struct irq_data *d;
	bool enabled;
	u32 config;
	int ret;

	owner_release_pmic(state);
	/* The arbiter removes children, synchronizes its IRQ, then the domain. */
	if (state->child) {
		platform_device_unregister(state->child);
		state->child = NULL;
	}
	if (state->fwnode) {
		fwnode_remove_software_node(state->fwnode);
		state->fwnode = NULL;
	}
	if (state->pushed) {
		d = irq_get_irq_data(state->irq);
		ret = qcom_pdc_acpi_spmi_pin_state(state->pdc, &config, &enabled);
		if (!d || irqd_is_activated(d) || ret || enabled ||
		    config != state->pin_config) {
			dev_err(dev, "SPMI teardown incomplete; preserving core-owned PDC mapping\n");
			goto put_pdc;
		}
		ret = irq_domain_pop_irq(state->domain, state->irq);
		if (ret) {
			dev_err(dev, "cannot detach PDC mapping: %d\n", ret);
			goto put_pdc;
		}
		state->pushed = false;
		d = irq_get_irq_data(state->irq);
		if (!d || d->domain != state->original.domain ||
		    d->hwirq != state->original.hwirq ||
		    d->chip != state->original.chip ||
		    d->chip_data != state->original.chip_data ||
		    d->parent_data != state->original.parent_data ||
		    d->mask != state->original.mask)
			dev_err(dev, "SPMI parent IRQ changed during teardown\n");
		else
			dev_info(dev, "SPMI transport removed; PDC/GIC state restored\n");
	}
put_pdc:
	if (state->pdc) {
		put_device(state->pdc);
		state->pdc = NULL;
	}
}

static int owner_register_transport(struct device *dev)
{
	static const struct property_entry properties[] = {
		PROPERTY_ENTRY_U32("qcom,channel", 0),
		PROPERTY_ENTRY_U32("qcom,ee", 0),
		{ }
	};
	struct owner_test *state = dev_get_drvdata(dev);
	struct resource resources[] = {
		DEFINE_RES_MEM_NAMED(0x0c440000, 0x1100, "core"),
		DEFINE_RES_MEM_NAMED(0x0c600000, 0x2000000, "chnls"),
		DEFINE_RES_MEM_NAMED(0x0e600000, 0x100000, "obsrvr"),
		DEFINE_RES_MEM_NAMED(0x0e700000, 0xa0000, "intr"),
		DEFINE_RES_MEM_NAMED(0x0c40a000, 0x26000, "cnfg"),
		DEFINE_RES_IRQ_NAMED(state->irq, "periph_irq"),
	};
	struct platform_device_info info = {
		.parent = dev,
		.name = "spmi_pmic_arb",
		.id = PLATFORM_DEVID_NONE,
		.res = resources,
		.num_res = ARRAY_SIZE(resources),
	};
	struct irq_fwspec spec;
	struct irq_data *d;
	bool enabled, masked, active, pending;
	int ret;

	state->pdc = bus_find_device_by_name(&platform_bus_type, NULL, "sc7180-usb-pdc");
	if (!state->pdc)
		return -EPROBE_DEFER;
	state->domain = qcom_pdc_acpi_spmi_domain(state->pdc);
	if (IS_ERR(state->domain)) {
		ret = PTR_ERR(state->domain);
		goto release;
	}
	d = irq_get_irq_data(state->irq);
	ret = -EBUSY;
	if (!d || d->hwirq != 513 || d->domain != state->domain->parent ||
	    d->parent_data || irqd_is_activated(d))
		goto release;
	state->original = *d;
	ret = irq_get_irqchip_state(state->irq, IRQCHIP_STATE_MASKED, &masked);
	if (ret)
		goto release;
	ret = irq_get_irqchip_state(state->irq, IRQCHIP_STATE_ACTIVE, &active);
	if (ret)
		goto release;
	ret = irq_get_irqchip_state(state->irq, IRQCHIP_STATE_PENDING, &pending);
	if (ret)
		goto release;
	ret = -EBUSY;
	if (!masked || active || pending)
		goto release;
	ret = qcom_pdc_acpi_spmi_pin_state(state->pdc, &state->pin_config, &enabled);
	if (ret)
		goto release;
	if (enabled || state->pin_config != 4) {
		ret = -EBUSY;
		goto release;
	}
	spec = (struct irq_fwspec) {
		.fwnode = state->domain->fwnode,
		.param_count = 2,
		.param = { 1, IRQ_TYPE_LEVEL_HIGH },
	};
	ret = irq_domain_push_irq(state->domain, state->irq, &spec);
	if (ret)
		goto release;
	state->pushed = true;
	state->fwnode = fwnode_create_software_node(properties, NULL);
	if (IS_ERR(state->fwnode)) {
		ret = PTR_ERR(state->fwnode);
		state->fwnode = NULL;
		goto release;
	}
	info.fwnode = state->fwnode;
	state->child = platform_device_register_full(&info);
	if (IS_ERR(state->child)) {
		ret = PTR_ERR(state->child);
		state->child = NULL;
		goto release;
	}
	if (!device_is_bound(&state->child->dev)) {
		ret = -EPROBE_DEFER;
		goto release;
	}
	ret = owner_register_pmic(dev);
	if (ret)
		goto release;
	dev_info(dev, "SPMI transport registered through standard platform driver\n");
	return 0;
release:
	owner_release_transport(dev);
	return ret;
}

static int owner_master_bind(struct device *dev)
{
	struct owner_test *state = dev_get_drvdata(dev);
	int ret;

	ret = component_bind_all(dev, state);
	if (ret)
		return ret;
	ret = owner_register_transport(dev);
	if (ret)
		component_unbind_all(dev, state);
	return ret;
}

static void owner_master_unbind(struct device *dev)
{
	owner_release_transport(dev);
	component_unbind_all(dev, dev_get_drvdata(dev));
}

static const struct component_master_ops owner_master_ops = {
	.bind = owner_master_bind,
	.unbind = owner_master_unbind,
};

static int owner_compare(struct device *dev, void *data)
{
	return ACPI_COMPANION(dev) == data;
}

static void owner_put_acpi(void *data)
{
	acpi_dev_put(data);
}

static int owner_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct acpi_device *adev = ACPI_COMPANION(dev);
	struct component_match *match = NULL;
	struct owner_test *state;
	struct resource *res;
	acpi_handle handle;
	int ret;

	if (!enable || !dmi_match(DMI_SYS_VENDOR, "TCL Communication Ltd.") ||
	    !dmi_match(DMI_PRODUCT_NAME, "B220G") || !adev)
		return -ENODEV;

	state = devm_kzalloc(dev, sizeof(*state), GFP_KERNEL);
	if (!state)
		return -ENOMEM;
	platform_set_drvdata(pdev, state);

	if (!strcmp(acpi_device_hid(adev), "QCOM0830")) {
		if (ACPI_FAILURE(acpi_get_handle(NULL, "\\_SB.PM01", &handle)) ||
		    handle != adev->handle)
			return -ENODEV;
		return component_add(dev, &owner_component_ops);
	}

	if (ACPI_FAILURE(acpi_get_handle(NULL, "\\_SB.SPMI", &handle)) ||
	    handle != adev->handle)
		return -ENODEV;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res || res->start != 0x0c400000 ||
	    resource_size(res) != 0x02800000 ||
	    platform_get_resource(pdev, IORESOURCE_MEM, 1))
		return -EINVAL;
	if (ACPI_FAILURE(acpi_get_handle(NULL, "\\_SB.PM01", &handle)))
		return -ENODEV;
	state->pm01 = acpi_get_acpi_dev(handle);
	if (!state->pm01)
		return -ENODEV;
	ret = devm_add_action_or_reset(dev, owner_put_acpi, state->pm01);
	if (ret)
		return ret;

	state->master = true;
	component_match_add(dev, &match, owner_compare, state->pm01);
	return component_master_add_with_match(dev, &owner_master_ops, match);
}

static void owner_remove(struct platform_device *pdev)
{
	struct owner_test *state = platform_get_drvdata(pdev);

	if (state->master)
		component_master_del(&pdev->dev, &owner_master_ops);
	else
		component_del(&pdev->dev, &owner_component_ops);
}

static const struct acpi_device_id owner_ids[] = {
	{ "QCOM080C" },
	{ "QCOM0830" },
	{ }
};
MODULE_DEVICE_TABLE(acpi, owner_ids);

static struct platform_driver owner_driver = {
	.probe = owner_probe,
	.remove = owner_remove,
	.driver = {
		.name = "qcom-spmi-acpi",
		.acpi_match_table = owner_ids,
	},
};
module_platform_driver(owner_driver);

MODULE_DESCRIPTION("TCL ACPI resource adapter for the Qualcomm SPMI platform driver");
MODULE_LICENSE("GPL");
