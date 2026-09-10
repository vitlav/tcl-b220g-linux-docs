#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/workqueue.h>


static unsigned int hold_seconds = 30;
module_param(hold_seconds, uint, 0444);
static void detach(struct work_struct *w){
 struct device_node *n=of_find_node_by_path("/soc@0/tcl-wcd9385");
 struct platform_device *p;
 if(!n)return;
 p=of_find_device_by_node(n);of_node_put(n);
 if(p){device_release_driver(&p->dev);put_device(&p->dev);}
 pr_info("TCL_WCD_AGGREGATE watchdog detached codec\n");
}
static DECLARE_DELAYED_WORK(off,detach);
static int __init start(void){if (hold_seconds > 1800) return -EINVAL; if (hold_seconds) schedule_delayed_work(&off,msecs_to_jiffies(hold_seconds * 1000));return 0;}
static void __exit stop(void){cancel_delayed_work_sync(&off);detach(NULL);}
module_exit(stop);
module_init(start);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("WCD9385 detach on unload; hold_seconds=0 disables timed detach");
