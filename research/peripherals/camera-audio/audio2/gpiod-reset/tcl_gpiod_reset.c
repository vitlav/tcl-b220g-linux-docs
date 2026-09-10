#include <linux/module.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/gpio/driver.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio/machine.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
static void __iomem *regs;
static struct gpio_desc *desc;
static bool changed;
static int set(struct gpio_chip *gc,unsigned int n,int v){writel(v?2:0,regs+4);readl(regs+4);return 0;}
static int get(struct gpio_chip *gc,unsigned int n){return !!(readl(regs+4)&1);}
static int direction(struct gpio_chip *gc,unsigned int n){return !(readl(regs)&0x200);}
static int output(struct gpio_chip *gc,unsigned int n,int v){changed=true;set(gc,n,v);writel(0x200,regs);readl(regs);return 0;}
static struct gpio_chip chip={.label="tcl-codec-reset-test",.owner=THIS_MODULE,.base=-1,.ngpio=1,.can_sleep=true,.get=get,.set=set,.get_direction=direction,.direction_output=output};
static void restore(struct work_struct *w){
 if(!changed)return;
 gpiod_set_value_cansleep(desc,1);
 writel(1,regs);readl(regs);changed=false;
 pr_info("TCL_GPIOD_RESET restored cfg=%x io=%x\n",readl(regs),readl(regs+4));
}
static DECLARE_DELAYED_WORK(off,restore);
static int __init start(void){
 int r;
 if(!of_machine_is_compatible("tcl,book14"))return -ENODEV;
 regs=ioremap(0x0353a000,0x14);if(!regs)return -ENOMEM;
 if(readl(regs+0x10)!=1||readl(regs)!=1||readl(regs+4)!=0){r=-EBUSY;goto unmap;}
 r=gpiochip_add_data(&chip,NULL);if(r)goto unmap;
 desc=gpiochip_request_own_desc(&chip,0,"tcl-reset-validation",GPIO_ACTIVE_LOW,GPIOD_OUT_HIGH);
 if(IS_ERR(desc)){r=PTR_ERR(desc);gpiochip_remove(&chip);goto unmap;}
 msleep(5);
 gpiod_set_value_cansleep(desc,0);
 msleep(2);
 pr_info("TCL_GPIOD_RESET released logical=%d cfg=%x io=%x; restore15s\n",gpiod_get_value_cansleep(desc),readl(regs),readl(regs+4));
 schedule_delayed_work(&off,msecs_to_jiffies(15000));
 return 0;
 unmap:iounmap(regs);return r;
}
static void __exit stop(void){cancel_delayed_work_sync(&off);restore(NULL);gpiochip_free_own_desc(desc);gpiochip_remove(&chip);iounmap(regs);}
module_init(start);module_exit(stop);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Temporary single-pin GPIO58 provider and bounded active-low gpiod reset test");
