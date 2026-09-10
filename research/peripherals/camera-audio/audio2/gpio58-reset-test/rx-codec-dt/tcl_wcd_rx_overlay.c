#include <linux/module.h>
#include <linux/of.h>
#include "overlay.inc"
static int id;
static int __init start(void) {
 struct device_node *n;
 int r;
 if (!of_machine_is_compatible("tcl,book14")) return -ENODEV;
 n=of_find_node_by_path("/soc@0/soundwire@62610000");
 if (!n) return -ENODEV;
 if (!of_device_is_compatible(n,"qcom,soundwire-v1.5.1")) { of_node_put(n); return -EINVAL; }
 of_node_put(n);
 n=of_find_node_by_path("/soc@0/soundwire@62610000/codec@0,4");
 if (n) { of_node_put(n); return -EEXIST; }
 r=of_overlay_fdt_apply(blob,sizeof(blob),&id,NULL);
 pr_info("TCL_WCD_RX_OVERLAY ret=%d id=%d UID4 RX mapping1..5\n",r,id);
 return r;
}
module_init(start);
MODULE_LICENSE("GPL");
