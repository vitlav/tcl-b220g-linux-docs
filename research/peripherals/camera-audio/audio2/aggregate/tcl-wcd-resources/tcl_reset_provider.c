#include <linux/module.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/gpio/driver.h>
static void __iomem *regs;
static struct gpio_chip chip;
static int set(struct gpio_chip *c,unsigned int n,int v){writel(v?2:0,regs+4);readl(regs+4);return 0;}
static int get(struct gpio_chip *c,unsigned int n){return !!(readl(regs+4)&1);}
static int dir(struct gpio_chip *c,unsigned int n){return !(readl(regs)&0x200);}
static int output(struct gpio_chip *c,unsigned int n,int v){set(c,n,v);writel(0x200,regs);readl(regs);return 0;}
static void free_pin(struct gpio_chip *c,unsigned int n){writel(0,regs+4);readl(regs+4);writel(1,regs);readl(regs);pr_info("TCL_RESET_PROVIDER restored cfg=%x io=%x\n",readl(regs),readl(regs+4));}
static int probe(struct platform_device *p){int r;
 if(!of_machine_is_compatible("tcl,book14"))return -ENODEV;
 regs=ioremap(0x0353a000,0x14);if(!regs)return -ENOMEM;
 if(readl(regs+0x10)!=1||readl(regs)!=1||readl(regs+4)!=0){iounmap(regs);return -EBUSY;}
 chip=(struct gpio_chip){.label="tcl-wcd-reset",.owner=THIS_MODULE,.parent=&p->dev,.fwnode=dev_fwnode(&p->dev),.base=-1,.ngpio=1,.get=get,.set=set,.get_direction=dir,.direction_output=output,.free=free_pin};
 r=gpiochip_add_data(&chip,NULL);if(r)iounmap(regs);return r;
}
static void remove_provider(struct platform_device *p){gpiochip_remove(&chip);iounmap(regs);}
static const struct of_device_id ids[]={{.compatible="tcl,book14-reset-test"},{}};
static struct platform_driver drv={.probe=probe,.remove=remove_provider,.driver={.name="tcl-wcd-reset",.of_match_table=ids}};
module_platform_driver(drv);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Temporary WCD GPIO58-only MMIO descriptor provider; release restores initial state");
