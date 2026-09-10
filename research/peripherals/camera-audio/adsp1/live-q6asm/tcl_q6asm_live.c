// SPDX-License-Identifier: GPL-2.0-only
#include <linux/module.h>
#include <linux/of.h>
#include <linux/init.h>
static struct of_changeset changes;
static struct device_node *parent, *child;
static int __init tcl_q6asm_live_init(void)
{
 int ret;
 struct device_node *existing;
 parent=of_find_node_by_path("/soc@0/remoteproc@62400000/glink-edge/apr/service@7/dais");
 if (!parent) return -ENODEV;
 existing=of_get_child_by_name(parent,"dai@0");
 if (existing) { of_node_put(existing); of_node_put(parent); return -EEXIST; }
 of_changeset_init(&changes);
 child=of_changeset_create_node(&changes,parent,"dai@0");
 if (!child) { ret=-ENOMEM; goto fail; }
 ret=of_changeset_add_prop_u32(&changes,child,"reg",0);
 if (ret) goto fail;
 ret=of_changeset_apply(&changes);
 if (ret) goto fail;
 pr_info("TCL_Q6ASM_LIVE: added generic frontend dai@0; rebind q6asm-dai to register it. Persistent until reboot.\n");
 return 0;
fail:
 of_changeset_destroy(&changes);
 of_node_put(child);
 of_node_put(parent);
 return ret;
}
module_init(tcl_q6asm_live_init);
/* No module_exit: keep node and changeset alive for later ASoC consumers. */
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCL diagnostic: add one Q6ASM frontend; lifetime until reboot");
