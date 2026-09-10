#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/pm_runtime.h>
#include <linux/io.h>
static int __init start(void) {
 struct device_node *np=of_find_node_by_path("/soc@0/soundwire@62610000");
 struct platform_device *p;
 void __iomem *b;
 int r;
 if(!np) return -ENODEV;
 p=of_find_device_by_node(np); of_node_put(np);
 if(!p) return -ENODEV;
 if(!p->dev.driver) {put_device(&p->dev); return -ENODEV;}
 r=pm_runtime_resume_and_get(&p->dev);
 if(r<0) {put_device(&p->dev);return r;}
 b=ioremap(0x62610000,0x2000);
 if(!b) {r=-ENOMEM; goto out;}
 pr_info("TCL_SWR_STATUS cfg=%08x comp_status=%08x irq=%08x enum=%08x bus=%08x mcp=%08x slaves=%08x id1=%08x id2=%08x\n",readl(b+4),readl(b+0x14),readl(b+0x200),readl(b+0x500),readl(b+0x1044),readl(b+0x104c),readl(b+0x1090),readl(b+0x538),readl(b+0x53c));
 iounmap(b); r=0;
 out: pm_runtime_mark_last_busy(&p->dev);pm_runtime_put_autosuspend(&p->dev);put_device(&p->dev);return r;
}
static void __exit stop(void) {}
module_init(start);module_exit(stop);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Read only selected SoundWire status registers, excluding FIFO");
