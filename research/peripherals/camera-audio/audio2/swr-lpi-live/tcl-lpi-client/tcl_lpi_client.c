#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include "overlay.inc"
static int probe(struct platform_device *p) {dev_info(&p->dev,"RX pinctrl client active\n"); return 0;}
static const struct of_device_id ids[]={{.compatible="tcl,rx-pin-test"},{}};
static struct platform_driver drv={.probe=probe,.driver={.name="tcl-rx-pin-test",.of_match_table=ids}};
static int __init start(void) {
 int old=3,id,r;
 r=of_overlay_remove(&old); if(r) return r;
 r=platform_driver_register(&drv); if(r) return r;
 r=of_overlay_fdt_apply(blob,sizeof(blob),&id,NULL);
 pr_info("TCL_LPI_CLIENT overlay=%d ret=%d\n",id,r);
 return r;
}
module_init(start);
MODULE_LICENSE("GPL");
