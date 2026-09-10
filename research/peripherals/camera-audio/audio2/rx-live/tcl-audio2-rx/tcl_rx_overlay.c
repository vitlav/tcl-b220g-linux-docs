#include <linux/module.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include "lpass-macro-common.h"
#include "overlay.inc"
static struct clk *fsgen;
static struct clk_lookup *lookup;
static int overlay_id;
static int __init start(void)
{
 struct device_node *np;
 struct of_phandle_args args = {};
 int ret;
 np=of_find_node_by_phandle(0xda);
 if (!np || !of_device_is_compatible(np,"qcom,q6afe-clocks")) {
  of_node_put(np); return -EINVAL;
 }
 of_node_put(np);
 np=of_find_node_by_path("/soc@0/rxmacro@62600000");
 if (np) { of_node_put(np); return -EEXIST; }
 args.np=of_find_node_by_path("/soc@0/codec@62770000");
 if (!args.np) return -ENODEV;
 fsgen=of_clk_get_from_provider(&args);
 of_node_put(args.np);
 if (IS_ERR(fsgen)) return PTR_ERR(fsgen);
 lookup=clkdev_create(fsgen,"fsgen","62600000.rxmacro");
 if (!lookup) { clk_put(fsgen); return -ENOMEM; }
 pr_info("TCL_RX_OVERLAY LPASS codec=%s, VA fsgen provider acquired\n",lpass_macro_get_codec_version_string(lpass_macro_get_codec_version()));
 ret=of_overlay_fdt_apply(overlay_blob,sizeof(overlay_blob),&overlay_id,NULL);
 if (ret) { clkdev_drop(lookup); clk_put(fsgen); return ret; }
 pr_info("TCL_RX_OVERLAY id=%d: RX platform node and fsgen alias, persistent until reboot\n",overlay_id);
 return 0;
}
module_init(start);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCL RX macro test; SoC platform node, preserves APR clock provider");
