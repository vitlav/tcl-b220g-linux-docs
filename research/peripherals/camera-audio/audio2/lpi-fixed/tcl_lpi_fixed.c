#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include "overlay.inc"
static int probe(struct platform_device *p) {dev_info(&p->dev,"RX pinctrl client active\n"); return 0;}
static const struct of_device_id ids[]={{.compatible="tcl,rx-pin-test"},{}};
static struct platform_driver drv={.probe=probe,.driver={.name="tcl-rx-pin-test",.of_match_table=ids}};
static int __init start(void) {
 int id=0,r;
 struct device_node *n=of_find_node_by_path("/soc@0/pinctrl@627c0000");
 if(n) {of_node_put(n); return -EEXIST;}
 n=of_find_node_by_phandle(0xda);
 if(!n || !of_device_is_compatible(n,"qcom,q6afe-clocks")) {of_node_put(n); return -EINVAL;}
 of_node_put(n);
 r=platform_driver_register(&drv); if(r) return r;
 r=of_overlay_fdt_apply(blob,sizeof(blob),&id,NULL);
 pr_info("TCL_LPI_FIXED overlay=%d ret=%d\n",id,r);
 if(r) platform_driver_unregister(&drv);
 return r;
}
module_init(start);
MODULE_LICENSE("GPL");
