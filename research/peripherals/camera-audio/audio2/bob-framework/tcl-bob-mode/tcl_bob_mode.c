#include <linux/module.h>
#include <linux/of.h>
#include "overlay.inc"
static int id;
static int __init start(void){int r=of_overlay_fdt_apply(blob,sizeof(blob),&id,NULL);pr_info("TCL_BOB_MODE id=%d ret=%d\n",id,r);return r;}
module_init(start);
MODULE_LICENSE("GPL");
