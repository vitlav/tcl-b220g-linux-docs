// SPDX-License-Identifier: GPL-2.0-only
#include <linux/module.h>
#include <linux/platform_device.h>
#include <sound/soc.h>
#include <dt-bindings/sound/qcom,q6afe.h>
#define ASM "62400000.remoteproc:glink-edge:apr:service@7:dais"
#define AFE "62400000.remoteproc:glink-edge:apr:service@4:dais"
#define ROUTING "62400000.remoteproc:glink-edge:apr:service@8:routing"
static unsigned int channel_mask=3;
module_param(channel_mask,uint,0444);
MODULE_PARM_DESC(channel_mask,"Diagnostic DMA channel mask, initial hypothesis 3; not OEM-confirmed");
static struct snd_soc_dai_link_component fe_cpu[]={{.name=ASM}};
static struct snd_soc_dai_link_component fe_platform[]={{.name=ASM}};
static struct snd_soc_dai_link_component be_cpu[]={{.name=AFE,.dai_name="RX_CODEC_DMA_RX_0"}};
static struct snd_soc_dai_link_component be_platform[]={{.name=ROUTING}};
static int be_init(struct snd_soc_pcm_runtime *rtd)
{
 return snd_soc_dai_set_channel_map(snd_soc_rtd_to_cpu(rtd,0),0,NULL,2,&channel_mask);
}
static struct snd_soc_dai_link links[]={
 {.name="TCL PCM Diagnostic",.stream_name="TCL PCM Diagnostic",.id=0,
  .cpus=fe_cpu,.num_cpus=1,.platforms=fe_platform,.num_platforms=1,
  .codecs=&snd_soc_dummy_dlc,.num_codecs=1,.dynamic=1,.nonatomic=1,.playback_only=1},
 {.name="TCL B030 Diagnostic",.stream_name="RX_CODEC_DMA_RX_0 Playback",.id=RX_CODEC_DMA_RX_0,
  .cpus=be_cpu,.num_cpus=1,.platforms=be_platform,.num_platforms=1,
  .codecs=&snd_soc_dummy_dlc,.num_codecs=1,.no_pcm=1,.nonatomic=1,.playback_only=1,.init=be_init}
};
static const struct snd_soc_dapm_widget widgets[]={SND_SOC_DAPM_OUTPUT("DSP Diagnostic Sink")};
static const struct snd_soc_dapm_route routes[]={{"DSP Diagnostic Sink",NULL,"RX_CODEC_DMA_RX_0 Playback"}};
static struct snd_soc_card card={.name="TCL-ADSP-Diagnostic",.owner=THIS_MODULE,
 .dai_link=links,.num_links=ARRAY_SIZE(links),.dapm_widgets=widgets,.num_dapm_widgets=ARRAY_SIZE(widgets),
 .dapm_routes=routes,.num_dapm_routes=ARRAY_SIZE(routes)};
static struct platform_device *pdev;
static int __init card_init(void)
{
 int r;
 pdev=platform_device_register_simple("tcl-adsp-diagnostic",-1,NULL,0);
 if (IS_ERR(pdev)) return PTR_ERR(pdev);
 card.dev=&pdev->dev;r=snd_soc_register_card(&card);
 if (r) {platform_device_unregister(pdev);return r;}
 pr_info("TCL_ADSP_CARD: registered digital-path diagnostic with dummy codec; physical speaker not configured\n");
 return 0;
}
static void __exit card_exit(void) {snd_soc_unregister_card(&card);platform_device_unregister(pdev);}
module_init(card_init);module_exit(card_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCL Q6ASM to OEM AFE b030 digital diagnostic; no physical codec or PA setup");
