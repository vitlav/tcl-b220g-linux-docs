#include <linux/module.h>
#include <linux/of.h>
#include "overlay.inc"
static int overlay_id;
static struct of_changeset detach;
static struct device_node *oldclk;
static int __init start(void)
{
 int ret;
 oldclk=of_find_node_by_path("/soc@0/remoteproc@62400000/glink-edge/apr/service@4/clock-controller");
 if (!oldclk) return -ENODEV;
 if (oldclk->phandle) { of_node_put(oldclk); return -EEXIST; }
 of_changeset_init(&detach);
 ret=of_changeset_detach_node(&detach,oldclk);
 if (ret) goto fail;
 ret=of_changeset_apply(&detach);
 if (ret) goto fail;
 ret=of_overlay_fdt_apply(overlay_blob,sizeof(overlay_blob),&overlay_id,NULL);
 if (ret) {
  int restore=of_changeset_revert(&detach);
  pr_err("TCL_VA_OVERLAY apply=%d restore=%d\n",ret,restore);
  goto fail;
 }
 pr_info("TCL_VA_OVERLAY id=%d: clock provider replaced with referenced node; added AON/VA, persistent until reboot\n",overlay_id);
 return 0;
fail:
 of_changeset_destroy(&detach);
 of_node_put(oldclk);
 return ret;
}
module_init(start);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCL LPASS AON VA diagnostic overlay; unload q6afe_clocks before loading");
