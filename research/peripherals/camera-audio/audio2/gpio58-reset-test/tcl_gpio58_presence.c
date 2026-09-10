#include <linux/module.h>
#include <linux/io.h>
#include <linux/of.h>
static int __init probe(void)
{
 void __iomem *p;
 u32 value, cfg, io;
 if (!of_machine_is_compatible("tcl,book14")) {
  pr_err("TCL_GPIO58_PRESENCE wrong machine; no access\n");
  return -ENODEV;
 }
 p=ioremap(0x0353a000,0x14);
 if (!p) return -ENOMEM;
 pr_info("TCL_GPIO58_PRESENCE before verified presence/config/io read; no writes\n");
 value=readl(p+0x10);
 if (!(value&1)) {iounmap(p);return -ENODEV;}
 cfg=readl(p);
 io=readl(p+4);
 pr_info("TCL_GPIO58_SNAPSHOT cfg=%08x io=%08x function=%u oe=%u pull=%u drive=%u\n",cfg,io,(cfg>>2)&15,(cfg>>9)&1,cfg&3,(cfg>>6)&7);
 pr_info("TCL_GPIO58_PRESENCE value=%08x present_bit=%u\n",value,value&1);
 iounmap(p);
 return 0;
}
static void __exit done(void) {}
module_init(probe);module_exit(done);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("OEM-mapped GPIO58 presence/config/io snapshot; no writes");
