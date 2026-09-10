// SPDX-License-Identifier: GPL-2.0
// Copyright (c) 2020, Linaro Limited

#include <dt-bindings/sound/qcom,q6afe.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <linux/soundwire/sdw.h>
#include <sound/jack.h>
#include <linux/input-event-codes.h>
#include "qdsp6/q6afe.h"
#include "common.h"
#include "usb_offload_utils.h"
#include "sdw.h"

#define MI2S_BCLK_RATE		1536000

struct sm8250_snd_data {
	bool stream_prepared[AFE_PORT_MAX];
	struct snd_soc_card *card;
	struct sdw_stream_runtime *sruntime[AFE_PORT_MAX];
	struct snd_soc_jack jack;
	struct snd_soc_jack usb_offload_jack;
	bool usb_offload_jack_setup;
	struct snd_soc_jack dp_jack;
	bool jack_setup;
};

static int sm8250_snd_init(struct snd_soc_pcm_runtime *rtd)
{
	struct sm8250_snd_data *data = snd_soc_card_get_drvdata(rtd->card);
	struct snd_soc_dai *cpu_dai = snd_soc_rtd_to_cpu(rtd, 0);

	switch (cpu_dai->id) {
	case DISPLAY_PORT_RX:
		return qcom_snd_dp_jack_setup(rtd, &data->dp_jack, 0);
	case USB_RX:
		return qcom_snd_usb_offload_jack_setup(rtd, &data->usb_offload_jack,
						       &data->usb_offload_jack_setup);
	default:
		return qcom_snd_wcd_jack_setup(rtd, &data->jack, &data->jack_setup);
	}
}

static void sm8250_snd_exit(struct snd_soc_pcm_runtime *rtd)
{
	struct sm8250_snd_data *data = snd_soc_card_get_drvdata(rtd->card);
	struct snd_soc_dai *cpu_dai = snd_soc_rtd_to_cpu(rtd, 0);

	if (cpu_dai->id == USB_RX)
		qcom_snd_usb_offload_jack_remove(rtd,
						 &data->usb_offload_jack_setup);

}

static int sm8250_be_hw_params_fixup(struct snd_soc_pcm_runtime *rtd,
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

static int sm8250_snd_startup(struct snd_pcm_substream *substream)
{
	unsigned int fmt = SND_SOC_DAIFMT_BP_FP;
	unsigned int codec_dai_fmt = SND_SOC_DAIFMT_BC_FC;
	struct snd_soc_pcm_runtime *rtd = snd_soc_substream_to_rtd(substream);
	struct snd_soc_dai *cpu_dai = snd_soc_rtd_to_cpu(rtd, 0);
	struct snd_soc_dai *codec_dai = snd_soc_rtd_to_codec(rtd, 0);

	switch (cpu_dai->id) {
	case PRIMARY_MI2S_RX:
		codec_dai_fmt |= SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_I2S;
		snd_soc_dai_set_sysclk(cpu_dai,
			Q6AFE_LPASS_CLK_ID_PRI_MI2S_IBIT,
			MI2S_BCLK_RATE, SNDRV_PCM_STREAM_PLAYBACK);
		snd_soc_dai_set_fmt(cpu_dai, fmt);
		snd_soc_dai_set_fmt(codec_dai, codec_dai_fmt);
		break;
	case SECONDARY_MI2S_RX:
		codec_dai_fmt |= SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_I2S;
		snd_soc_dai_set_sysclk(cpu_dai,
			Q6AFE_LPASS_CLK_ID_SEC_MI2S_IBIT,
			MI2S_BCLK_RATE, SNDRV_PCM_STREAM_PLAYBACK);
		snd_soc_dai_set_fmt(cpu_dai, fmt);
		snd_soc_dai_set_fmt(codec_dai, codec_dai_fmt);
		break;
	case TERTIARY_MI2S_RX:
		codec_dai_fmt |= SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_I2S;
		snd_soc_dai_set_sysclk(cpu_dai,
			Q6AFE_LPASS_CLK_ID_TER_MI2S_IBIT,
			MI2S_BCLK_RATE, SNDRV_PCM_STREAM_PLAYBACK);
		snd_soc_dai_set_fmt(cpu_dai, fmt);
		snd_soc_dai_set_fmt(codec_dai, codec_dai_fmt);
		break;
	default:
		break;
	}

	return qcom_snd_sdw_startup(substream);
}

static void sm8250_snd_shutdown(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = snd_soc_substream_to_rtd(substream);
	struct snd_soc_dai *cpu_dai = snd_soc_rtd_to_cpu(rtd, 0);
	struct sm8250_snd_data *data = snd_soc_card_get_drvdata(rtd->card);
	struct sdw_stream_runtime *sruntime = qcom_snd_sdw_get_stream(substream);

	data->sruntime[cpu_dai->id] = NULL;
	sdw_release_stream(sruntime);
}

static int sm8250_snd_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = snd_soc_substream_to_rtd(substream);
	struct snd_soc_dai *cpu_dai = snd_soc_rtd_to_cpu(rtd, 0);
	struct sm8250_snd_data *pdata = snd_soc_card_get_drvdata(rtd->card);

	return qcom_snd_sdw_hw_params(substream, params, &pdata->sruntime[cpu_dai->id]);
}

static int sm8250_snd_prepare(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = snd_soc_substream_to_rtd(substream);
	struct snd_soc_dai *cpu_dai = snd_soc_rtd_to_cpu(rtd, 0);
	struct sm8250_snd_data *data = snd_soc_card_get_drvdata(rtd->card);
	struct sdw_stream_runtime *sruntime = data->sruntime[cpu_dai->id];

	return qcom_snd_sdw_prepare(substream, sruntime,
				    &data->stream_prepared[cpu_dai->id]);
}

static int sm8250_snd_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = snd_soc_substream_to_rtd(substream);
	struct sm8250_snd_data *data = snd_soc_card_get_drvdata(rtd->card);
	struct snd_soc_dai *cpu_dai = snd_soc_rtd_to_cpu(rtd, 0);
	struct sdw_stream_runtime *sruntime = data->sruntime[cpu_dai->id];

	return qcom_snd_sdw_hw_free(substream, sruntime,
				    &data->stream_prepared[cpu_dai->id]);
}

static const struct snd_soc_ops sm8250_be_ops = {
	.startup = sm8250_snd_startup,
	.shutdown = sm8250_snd_shutdown,
	.hw_params = sm8250_snd_hw_params,
	.hw_free = sm8250_snd_hw_free,
	.prepare = sm8250_snd_prepare,
};

static void sm8250_add_be_ops(struct snd_soc_card *card)
{
	struct snd_soc_dai_link *link;
	int i;

	for_each_card_prelinks(card, i, link) {
		if (link->no_pcm == 1) {
			link->init = sm8250_snd_init;
			link->exit = sm8250_snd_exit;
			link->be_hw_params_fixup = sm8250_be_hw_params_fixup;
			link->ops = &sm8250_be_ops;
		}
	}
}


/* Diagnostic runtime card: resolve existing nodes without modifying phandles. */
static void put_node(void *node)
{
 of_node_put(node);
}

static int get_dlc(struct device *dev, struct snd_soc_dai_link_component *dlc,
                   const char *path, int id)
{
 struct of_phandle_args args = {};
 int ret;
 args.np = of_find_node_by_path(path);
 if (!args.np)
  return -ENODEV;
 ret = devm_add_action_or_reset(dev, put_node, args.np);
 if (ret)
  return ret;
 args.args_count = 1;
 args.args[0] = id;
 dlc->of_node = args.np;
 ret = snd_soc_get_dai_name(&args, &dlc->dai_name);
 dev_info(dev, "TCL card resolve %s id=%d dai=%s ret=%d\n",
          path, id, dlc->dai_name ?: "none", ret);
 return ret;
}

static const struct snd_soc_dapm_widget test_widgets[] = {
 SND_SOC_DAPM_HP("Headphone Jack", NULL),
 SND_SOC_DAPM_MIC("Mic Jack", NULL),
};
static const struct snd_soc_dapm_route test_routes[] = {
 { "IN1_HPHL", NULL, "HPHL_OUT" },
 { "IN2_HPHR", NULL, "HPHR_OUT" },
};

static int card_probe(struct platform_device *pdev)
{
 struct device *dev = &pdev->dev;
 struct sm8250_snd_data *data;
 struct snd_soc_card *card;
 struct snd_soc_dai_link *links;
 struct snd_soc_dai_link_component *c;
 int ret;
 data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
 card = devm_kzalloc(dev, sizeof(*card), GFP_KERNEL);
 links = devm_kcalloc(dev, 2, sizeof(*links), GFP_KERNEL);
 c = devm_kcalloc(dev, 7, sizeof(*c), GFP_KERNEL);
 if (!data || !card || !links || !c)
  return -ENOMEM;
 card->owner = THIS_MODULE;
 card->dev = dev;
 card->name = "TCL WCD9385 Test";
 card->driver_name = "tcl-audio-test";
 card->dai_link = links;
 card->num_links = 2;
 card->dapm_widgets = test_widgets;
 card->num_dapm_widgets = ARRAY_SIZE(test_widgets);
 card->dapm_routes = test_routes;
 card->num_dapm_routes = ARRAY_SIZE(test_routes);
 snd_soc_card_set_drvdata(card, data);
 data->card = card;
 links[0].name = links[0].stream_name = "MultiMedia1";
 links[0].id = 0;
 links[0].cpus = &c[0]; links[0].num_cpus = 1;
 links[0].platforms = &c[1]; links[0].num_platforms = 1;
 links[0].codecs = &snd_soc_dummy_dlc; links[0].num_codecs = 1;
 links[0].dynamic = 1; links[0].nonatomic = 1;
 links[0].ignore_suspend = 1;
 ret = get_dlc(dev, &c[0], "/soc@0/remoteproc@62400000/glink-edge/apr/service@7/dais", 0);
 if (ret) return ret;
 c[1].of_node = c[0].of_node;
 links[1].name = links[1].stream_name = "WCD Playback";
 links[1].id = RX_CODEC_DMA_RX_0;
 links[1].cpus = &c[2]; links[1].num_cpus = 1;
 links[1].platforms = &c[3]; links[1].num_platforms = 1;
 links[1].codecs = &c[4]; links[1].num_codecs = 3;
 links[1].no_pcm = 1; links[1].nonatomic = 1;
 links[1].ignore_suspend = 1; links[1].ignore_pmdown_time = 1;
 ret = get_dlc(dev, &c[2], "/soc@0/remoteproc@62400000/glink-edge/apr/service@4/dais", RX_CODEC_DMA_RX_0);
 if (ret) return ret;
 c[3].of_node = of_find_node_by_path("/soc@0/remoteproc@62400000/glink-edge/apr/service@8/routing");
 if (!c[3].of_node) return -ENODEV;
 ret = devm_add_action_or_reset(dev, put_node, c[3].of_node);
 if (ret) return ret;
 ret = get_dlc(dev, &c[4], "/soc@0/tcl-wcd9385", 0);
 if (ret) return ret;
 ret = get_dlc(dev, &c[5], "/soc@0/soundwire@62610000", 0);
 if (ret) return ret;
 ret = get_dlc(dev, &c[6], "/soc@0/rxmacro@62600000", 0);
 if (ret) return ret;
 sm8250_add_be_ops(card);
 ret = devm_snd_soc_register_card(dev, card);
 dev_info(dev, "TCL card register ret=%d\n", ret);
 return ret;
}

static struct platform_driver card_driver = {
 .probe = card_probe,
 .driver = { .name = "tcl-audio-card-test" },
};
static struct platform_device *card_device;
static int __init card_start(void)
{
 int ret;
 if (!of_machine_is_compatible("tcl,book14")) return -ENODEV;
 ret = platform_driver_register(&card_driver);
 if (ret) return ret;
 card_device = platform_device_register_simple("tcl-audio-card-test", -1, NULL, 0);
 if (IS_ERR(card_device)) {
  ret = PTR_ERR(card_device);
  platform_driver_unregister(&card_driver);
  return ret;
 }
 return 0;
}
static void __exit card_stop(void)
{
 platform_device_unregister(card_device);
 platform_driver_unregister(&card_driver);
}
module_init(card_start);
module_exit(card_stop);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCL diagnostic playback card, based on sm8250 SoundWire callbacks");
