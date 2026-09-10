#include <linux/module.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/io.h>
static int __init start(void)
{
 struct of_phandle_args a = {};
 struct clk *c;
 void __iomem *b;
 int r;
 a.np=of_find_node_by_path("/soc@0/rxmacro@62600000");
 if (!a.np) return -ENODEV;
 c=of_clk_get_from_provider(&a);
 of_node_put(a.np);
 if(IS_ERR(c)) return PTR_ERR(c);
 r=clk_prepare_enable(c);
 if(r) {clk_put(c); return r;}
 b=ioremap(0x62610000,0x1000);
 if(!b) {r=-ENOMEM; goto out;}
 pr_info("TCL_SWR_IDENT RX version=%08x params=%08x master=%08x cfg=%08x status=%08x\n",readl(b),readl(b+0x100),readl(b+0x104),readl(b+4),readl(b+0x14));
 iounmap(b);
 out: clk_disable_unprepare(c); clk_put(c); return r;
}
static void __exit stop(void) {}
module_init(start);
module_exit(stop);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Read RX SoundWire identification with RX clock enabled");
