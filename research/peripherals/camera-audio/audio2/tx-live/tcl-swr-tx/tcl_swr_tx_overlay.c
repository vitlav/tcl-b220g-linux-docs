#include <linux/module.h>
#include <linux/of.h>
#include "overlay.inc"
static int id;
static int __init start(void) {
 struct device_node *n=of_find_node_by_phandle(0xe3);
 int r;
 if(!n || !of_device_is_compatible(n,"qcom,sm8250-lpass-tx-macro")) {of_node_put(n); return -EINVAL;}
 of_node_put(n);
 n=of_find_node_by_path("/soc@0/soundwire@62630000");
 if(n) {of_node_put(n); return -EEXIST;}
 r=of_overlay_fdt_apply(blob,sizeof(blob),&id,NULL);
 pr_info("TCL_SWR_TX_OVERLAY ret=%d id=%d TX SPI296\n",r,id);
 return r;
}
module_init(start);
MODULE_LICENSE("GPL");
