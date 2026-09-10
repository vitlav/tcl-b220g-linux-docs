#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/workqueue.h>
#include "overlay.inc"
static struct regulator *supply;
static bool enabled;
static int id;
static void release(struct work_struct *w) {
 int r;
 if (!enabled)return;
 r=regulator_disable(supply);
 if(!r)enabled=false;
 pr_info("TCL_BOB_CONSUMER disable=%d enabled=%d\n",r,enabled);
}
static DECLARE_DELAYED_WORK(off,release);
static int probe(struct platform_device *p) {
 int r;
 supply=devm_regulator_get(&p->dev,"power");
 if(IS_ERR(supply))return dev_err_probe(&p->dev,PTR_ERR(supply),"supply unavailable\n");
 r=regulator_set_voltage(supply,3300000,3300000);if(r)return r;
 r=regulator_set_mode(supply,REGULATOR_MODE_NORMAL);if(r)return r;
 r=regulator_enable(supply);if(r)return r;
 enabled=true;
 schedule_delayed_work(&off,msecs_to_jiffies(5000));
 pr_info("TCL_BOB_CONSUMER enabled voltage=%d mode=%u; disable in5s\n",regulator_get_voltage(supply),regulator_get_mode(supply));
 return 0;
}
static void remove_consumer(struct platform_device *p) {
 cancel_delayed_work_sync(&off);release(NULL);
}
static const struct of_device_id ids[]={{.compatible="tcl,bob-consumer-test"},{}};
static struct platform_driver drv={.probe=probe,.remove=remove_consumer,.driver={.name="tcl-bob-consumer-test",.of_match_table=ids}};
static int __init start(void) {
 struct device_node *n;
 int r;
 if(!of_machine_is_compatible("tcl,book14"))return -ENODEV;
 n=of_find_node_by_path("/soc@0/rsc@18200000/regulators-1/bob");
 if(n){of_node_put(n);return -EBUSY;}
 n=of_find_node_by_path("/soc@0/rsc@18200000/tcl-audio-bob-test");
 if(n){of_node_put(n);return -EEXIST;}
 r=platform_driver_register(&drv);if(r)return r;
 r=of_overlay_fdt_apply(blob,sizeof(blob),&id,NULL);
 if(r){platform_driver_unregister(&drv);return r;}
 pr_info("TCL_BOB_TEST overlay=%d\n",id);
 return 0;
}
module_init(start);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCL isolated BOB regulator-framework five-second test");
