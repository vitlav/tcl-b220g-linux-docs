// SPDX-License-Identifier: GPL-2.0-only
#include <dt-bindings/sound/qcom,q6afe.h>
#include <dt-bindings/sound/qcom,q6dsp-lpass-ports.h>
#include <linux/delay.h>
#include <linux/bitmap.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio/machine.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/soc/qcom/apr.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/soc-dapm.h>
#include "sdw.h"

struct tcl_card_data {
	bool stream_prepared[256];
	struct gpio_descs *speaker_pa;
};

static int tcl_be_startup(struct snd_pcm_substream *substream)
{
	return qcom_snd_sdw_startup(substream);
}

static int tcl_be_prepare(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = snd_soc_substream_to_rtd(substream);
	struct tcl_card_data *data = snd_soc_card_get_drvdata(rtd->card);
	struct snd_soc_dai *cpu_dai = snd_soc_rtd_to_cpu(rtd, 0);

	return qcom_snd_sdw_prepare(substream,
				    &data->stream_prepared[cpu_dai->id]);
}

static int tcl_be_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = snd_soc_substream_to_rtd(substream);
	struct tcl_card_data *data = snd_soc_card_get_drvdata(rtd->card);
	struct snd_soc_dai *cpu_dai = snd_soc_rtd_to_cpu(rtd, 0);

	return qcom_snd_sdw_hw_free(substream,
				    &data->stream_prepared[cpu_dai->id]);
}

static const struct snd_soc_ops tcl_be_ops = {
	.startup = tcl_be_startup,
	.shutdown = qcom_snd_sdw_shutdown,
	.prepare = tcl_be_prepare,
	.hw_free = tcl_be_hw_free,
};

static int tcl_be_fixup(struct snd_soc_pcm_runtime *rtd,
			struct snd_pcm_hw_params *params)
{
	struct snd_interval *rate = hw_param_interval(params,
						      SNDRV_PCM_HW_PARAM_RATE);
	struct snd_interval *channels = hw_param_interval(params,
							  SNDRV_PCM_HW_PARAM_CHANNELS);
	struct snd_mask *fmt = hw_param_mask(params, SNDRV_PCM_HW_PARAM_FORMAT);

	rate->min = rate->max = 48000;
	channels->min = channels->max = 2;
	snd_mask_set_format(fmt, SNDRV_PCM_FORMAT_S16_LE);
	return 0;
}

static int tcl_speaker_pa_event(struct snd_soc_dapm_widget *widget,
				struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_card *card = snd_soc_dapm_to_card(widget->dapm);
	struct tcl_card_data *data = snd_soc_card_get_drvdata(card);
	DECLARE_BITMAP(values, 2);
	int value, ret;

	if (event == SND_SOC_DAPM_POST_PMU)
		value = 1;
	else if (event == SND_SOC_DAPM_PRE_PMD)
		value = 0;
	else
		return 0;

	if (!data->speaker_pa || data->speaker_pa->ndescs != 2)
		return -ENODEV;

	if (value)
		bitmap_fill(values, data->speaker_pa->ndescs);
	else
		bitmap_zero(values, data->speaker_pa->ndescs);
	ret = gpiod_multi_set_value_cansleep(data->speaker_pa, values);
	if (ret)
		return ret;
	dev_info(card->dev, "TCL_DAPM_PA GPIO46/47=%s event=%x\n",
		 value ? "high" : "low", event);
	usleep_range(5000, 6000);
	return 0;
}

static const struct snd_soc_dapm_widget tcl_widgets[] = {
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_SPK("Internal Speaker", tcl_speaker_pa_event),
};

static const struct snd_kcontrol_new tcl_controls[] = {
	SOC_DAPM_PIN_SWITCH("Internal Speaker"),
};

static const struct snd_soc_dapm_route tcl_routes[] = {
	{ "IN1_HPHL", NULL, "HPHL_OUT" },
	{ "IN2_HPHR", NULL, "HPHR_OUT" },
	{ "Internal Speaker", NULL, "IN1_HPHL" },
	{ "Internal Speaker", NULL, "IN2_HPHR" },
	{ "AMIC1", NULL, "MIC BIAS1" },
	{ "TX SWR_ADC0", NULL, "ADC1_OUTPUT" },
};

static struct gpiod_lookup_table tcl_pa_gpiod_table = {
	.dev_id = "tcl-acpi-card",
	.table = {
		GPIO_LOOKUP_IDX("QCOM080D:00", 46, "speaker-pa", 0,
				GPIO_ACTIVE_HIGH),
		GPIO_LOOKUP_IDX("QCOM080D:00", 47, "speaker-pa", 1,
				GPIO_ACTIVE_HIGH),
		{},
	},
};

static struct snd_soc_dai_link_component fe_cpu = {
	.name = "tcl-q6asm-dai",
	.dai_name = "MultiMedia1",
};
static struct snd_soc_dai_link_component fe_platform = {
	.name = "tcl-q6asm-dai",
};
static struct snd_soc_dai_link_component be_cpu = {
	.name = "q6afe-dai.6.auto",
	.dai_name = "RX_CODEC_DMA_RX_0",
};
static struct snd_soc_dai_link_component be_tx_cpu = {
	.name = "q6afe-dai.6.auto",
	.dai_name = "TX_CODEC_DMA_TX_3",
};
static struct snd_soc_dai_link_component be_platform = {
	.name = "q6routing",
};
static struct snd_soc_dai_link_component be_codecs[] = {
	{ .name = "wcd938x_codec", .dai_name = "wcd938x-sdw-rx" },
	{ .name = "tcl-swr-rx", .dai_name = "SDW Pin0" },
	{ .name = "tcl-sc7280-rx", .dai_name = "rx_macro_rx1" },
};
static struct snd_soc_dai_link_component be_tx_codecs[] = {
	{ .name = "wcd938x_codec", .dai_name = "wcd938x-sdw-tx" },
	{ .name = "tcl-swr-tx", .dai_name = "SDW Pin0" },
	{ .name = "tcl-sc7280-tx", .dai_name = "tx_macro_tx3" },
};

static struct snd_soc_dai_link tcl_links[] = {
	{
		.name = "MultiMedia1",
		.stream_name = "MultiMedia1",
		.id = 0,
		.cpus = &fe_cpu,
		.num_cpus = 1,
		.platforms = &fe_platform,
		.num_platforms = 1,
		.codecs = &snd_soc_dummy_dlc,
		.num_codecs = 1,
		.dynamic = 1,
		.ignore_suspend = 1,
		.nonatomic = 1,
	},
	{
		.name = "WCD Playback",
		.stream_name = "WCD Playback",
		.id = RX_CODEC_DMA_RX_0,
		.cpus = &be_cpu,
		.num_cpus = 1,
		.platforms = &be_platform,
		.num_platforms = 1,
		.codecs = be_codecs,
		.num_codecs = ARRAY_SIZE(be_codecs),
		.no_pcm = 1,
		.playback_only = 1,
		.ignore_suspend = 1,
		.ignore_pmdown_time = 1,
		.nonatomic = 1,
		.ops = &tcl_be_ops,
		.be_hw_params_fixup = tcl_be_fixup,
	},
	{
		.name = "WCD Capture",
		.stream_name = "WCD Capture",
		.id = TX_CODEC_DMA_TX_3,
		.cpus = &be_tx_cpu,
		.num_cpus = 1,
		.platforms = &be_platform,
		.num_platforms = 1,
		.codecs = be_tx_codecs,
		.num_codecs = ARRAY_SIZE(be_tx_codecs),
		.no_pcm = 1,
		.capture_only = 1,
		.ignore_suspend = 1,
		.nonatomic = 1,
		.ops = &tcl_be_ops,
		.be_hw_params_fixup = tcl_be_fixup,
	},
};

static int tcl_card_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card;
	struct tcl_card_data *data;

	card = devm_kzalloc(&pdev->dev, sizeof(*card), GFP_KERNEL);
	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!card || !data)
		return -ENOMEM;

	data->speaker_pa = devm_gpiod_get_array(&pdev->dev, "speaker-pa",
						GPIOD_OUT_LOW);
	if (IS_ERR(data->speaker_pa))
		return dev_err_probe(&pdev->dev, PTR_ERR(data->speaker_pa),
				      "speaker PA GPIOs unavailable\n");
	if (data->speaker_pa->ndescs != 2)
		return dev_err_probe(&pdev->dev, -EINVAL,
				      "expected two speaker PA GPIOs\n");

	card->owner = THIS_MODULE;
	card->dev = &pdev->dev;
	card->name = "TCL B220G ACPI Audio";
	card->driver_name = "tcl-b220g-audio";
	card->dai_link = tcl_links;
	card->num_links = ARRAY_SIZE(tcl_links);
	card->dapm_widgets = tcl_widgets;
	card->num_dapm_widgets = ARRAY_SIZE(tcl_widgets);
	card->controls = tcl_controls;
	card->num_controls = ARRAY_SIZE(tcl_controls);
	card->dapm_routes = tcl_routes;
	card->num_dapm_routes = ARRAY_SIZE(tcl_routes);
	snd_soc_card_set_drvdata(card, data);
	platform_set_drvdata(pdev, card);
	return devm_snd_soc_register_card(&pdev->dev, card);
}

static struct platform_driver tcl_card_driver = {
	.probe = tcl_card_probe,
	.driver = { .name = "tcl-acpi-card" },
};
static struct platform_device *tcl_route_device, *tcl_card_device;

static int __init tcl_card_init(void)
{
	struct platform_device_info route_info = {
		.name = "q6routing",
		.id = PLATFORM_DEVID_NONE,
	};
	struct device *q6adm_dev;
	int ret;

	gpiod_add_lookup_table(&tcl_pa_gpiod_table);

	/* q6adm_open() gets its service state from q6routing's parent. */
	q6adm_dev = bus_find_device_by_name(&aprbus, NULL,
						    "aprsvc:q6adm:4:8");
	if (!q6adm_dev) {
		ret = -EPROBE_DEFER;
		goto err_lookup;
	}
	route_info.parent = q6adm_dev;
	tcl_route_device = platform_device_register_full(&route_info);
	put_device(q6adm_dev);
	if (IS_ERR(tcl_route_device)) {
		ret = PTR_ERR(tcl_route_device);
		goto err_lookup;
	}
	ret = platform_driver_register(&tcl_card_driver);
	if (ret)
		goto err_route;
	tcl_card_device = platform_device_register_simple("tcl-acpi-card",
							   PLATFORM_DEVID_NONE,
							   NULL, 0);
	if (IS_ERR(tcl_card_device)) {
		ret = PTR_ERR(tcl_card_device);
		goto err_driver;
	}
	return 0;

err_driver:
	platform_driver_unregister(&tcl_card_driver);
err_route:
	platform_device_unregister(tcl_route_device);
err_lookup:
	gpiod_remove_lookup_table(&tcl_pa_gpiod_table);
	return ret;
}

static void __exit tcl_card_exit(void)
{
	platform_device_unregister(tcl_card_device);
	platform_driver_unregister(&tcl_card_driver);
	platform_device_unregister(tcl_route_device);
	gpiod_remove_lookup_table(&tcl_pa_gpiod_table);
}

module_init(tcl_card_init);
module_exit(tcl_card_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCL B220G ACPI WCD9385 audio card with DAPM-controlled speaker PA");
