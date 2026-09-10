#include <assert.h>
#include <stdint.h>
#define AFE_API_VERSION_CODEC_DMA_CONFIG 1
struct afe_param_id_cdc_dma_cfg { unsigned cdc_dma_cfg_minor_version,sample_rate,bit_width,data_format,num_channels,active_channels_mask; };
union afe_port_config { struct afe_param_id_cdc_dma_cfg dma_cfg; };
struct q6afe_port { union afe_port_config port_cfg; };
struct q6afe_cdc_dma_cfg { unsigned sample_rate,bit_width,data_format,num_channels,active_channels_mask; };
void q6afe_cdc_dma_port_prepare(struct q6afe_port *port,
				struct q6afe_cdc_dma_cfg *cfg)
{
	union afe_port_config *pcfg = &port->port_cfg;
	struct afe_param_id_cdc_dma_cfg *dma_cfg = &pcfg->dma_cfg;

	dma_cfg->cdc_dma_cfg_minor_version = AFE_API_VERSION_CODEC_DMA_CONFIG;
	dma_cfg->sample_rate = cfg->sample_rate;
	dma_cfg->bit_width = cfg->bit_width;
	dma_cfg->data_format = cfg->data_format;
	dma_cfg->num_channels = cfg->num_channels;
	dma_cfg->active_channels_mask = cfg->active_channels_mask;
	if (!cfg->active_channels_mask)
		dma_cfg->active_channels_mask = (1 << cfg->num_channels) - 1;
}

int main(void) {
 struct q6afe_port p={0}; struct q6afe_cdc_dma_cfg c={48000,16,0,2,3};
 q6afe_cdc_dma_port_prepare(&p,&c); assert(p.port_cfg.dma_cfg.active_channels_mask==3);
 c.active_channels_mask=5; q6afe_cdc_dma_port_prepare(&p,&c); assert(p.port_cfg.dma_cfg.active_channels_mask==5);
 c.active_channels_mask=0; q6afe_cdc_dma_port_prepare(&p,&c); assert(p.port_cfg.dma_cfg.active_channels_mask==3);
 c.num_channels=1; q6afe_cdc_dma_port_prepare(&p,&c); assert(p.port_cfg.dma_cfg.active_channels_mask==1);
 return 0;
}
