#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/workqueue.h>
#include "overlay.inc"
static int id;
static void detach(struct work_struct *w){
 struct device_node *n=of_find_node_by_path("/soc@0/tcl-wcd9385");
 struct platform_device *p;
 if(!n)return;
 p=of_find_device_by_node(n);of_node_put(n);
 if(p){device_release_driver(&p->dev);put_device(&p->dev);}
 pr_info("TCL_WCD_AGGREGATE watchdog detached codec\n");
}
static DECLARE_DELAYED_WORK(off,detach);
static int __init start(void){int r;
 if(!of_machine_is_compatible("tcl,book14"))return -ENODEV;
 r=of_overlay_fdt_apply(blob,sizeof(blob),&id,NULL);
 pr_info("TCL_WCD_AGGREGATE overlay=%d ret=%d\n",id,r);
 if(!r)schedule_delayed_work(&off,msecs_to_jiffies(30000));
 return r;
}
module_init(start);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("WCD9385 aggregate test with automatic detach after30s");
