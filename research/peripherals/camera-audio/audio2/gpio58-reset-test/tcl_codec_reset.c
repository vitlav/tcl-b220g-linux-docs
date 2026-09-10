#include <linux/module.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
static void __iomem *regs;
static bool changed;
static void restore(void)
{
 if (!changed) return;
 writel(0,regs+4);
 readl(regs+4);
 writel(1,regs);
 readl(regs);
 changed=false;
 pr_info("TCL_CODEC_RESET restored cfg=%08x io=%08x\n",readl(regs),readl(regs+4));
}
static void auto_restore(struct work_struct *w) {restore();}
static DECLARE_DELAYED_WORK(restore_work,auto_restore);
static int __init start(void)
{
 u32 present,cfg,io;
 if (!of_machine_is_compatible("tcl,book14")) return -ENODEV;
 regs=ioremap(0x0353a000,0x14);
 if(!regs)return -ENOMEM;
 present=readl(regs+0x10);cfg=readl(regs);io=readl(regs+4);
 if(present!=1||cfg!=1||io!=0){
  pr_err("TCL_CODEC_RESET unexpected initial state present=%x cfg=%x io=%x; no writes\n",present,cfg,io);
  iounmap(regs);regs=NULL;return -EBUSY;
 }
 /* Exact bounded experiment: same mux0/drive0, output enable, no pull,
  * corresponding to OEM output-configuration path. Do not touch59..62. */
 changed=true;
 writel(0,regs+4);
 writel(0x200,regs);
 readl(regs);
 msleep(5);
 writel(2,regs+4);
 readl(regs+4);
 msleep(2);
 pr_info("TCL_CODEC_RESET released cfg=%08x io=%08x; restore in15s\n",readl(regs),readl(regs+4));
 schedule_delayed_work(&restore_work,msecs_to_jiffies(15000));
 return 0;
}
static void __exit stop(void)
{
 cancel_delayed_work_sync(&restore_work);
 restore();
 if(regs)iounmap(regs);
}
module_init(start);module_exit(stop);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Bounded OEM TCL GPIO58 codec reset test; strict initial-state guard");
