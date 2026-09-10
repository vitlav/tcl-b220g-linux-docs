#include <linux/module.h>
#include <linux/of.h>
#include <linux/soundwire/sdw.h>
#include <linux/soundwire/sdw_type.h>
static int __init start(void) {
 struct device_node *np;
 struct device *dev;
 struct sdw_slave *s;
 unsigned int regs[]={0x3401,0x3402,0x3403,0x3404,0x34b0};
 int i,v,ret=0;
 if(!of_machine_is_compatible("tcl,book14"))return -ENODEV;
 np=of_find_node_by_path("/soc@0/soundwire@62630000/codec@0,3");
 if(!np)return -ENODEV;
 dev=of_sdw_find_device_by_node(np);of_node_put(np);
 if(!dev)return -ENODEV;
 s=dev_to_sdw_dev(dev);
 if(s->status!=SDW_SLAVE_ATTACHED || s->dev_num!=1 || !dev->driver) {ret=-ENODEV;goto out;}
 /* Test runner holds both macro/controller clocks and supplies active.
  * Direct bus reads avoid reading cached regmap defaults. No codec writes. */
 for(i=0;i<ARRAY_SIZE(regs);i++) {
  v=sdw_read_no_pm(s,regs[i]);
  pr_info("TCL_WCD_ID reg=%04x value=%d hex=%02x\n",regs[i],v,v);
  if(v<0){ret=v;break;}
 }
 out:put_device(dev);return ret;
}
static void __exit stop(void){}
module_init(start);module_exit(stop);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Bounded direct SoundWire read of WCD938x identification registers");
