// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/workqueue.h>
#include <soc/qcom/cmd-db.h>
#include <soc/qcom/rpmh.h>
static struct platform_device *banks[2];
static bool touched;
static int vote(int bank, u32 addr, u32 value)
{
 struct tcs_cmd cmd = { .addr = addr, .data = value };
 int ret = rpmh_write(&banks[bank]->dev, RPMH_ACTIVE_ONLY_STATE, &cmd, 1);
 pr_info("TCL_AUDIO_POWER_TEST addr=%08x value=%u ret=%d\n", addr, value, ret);
 return ret;
}
static void release_votes(void)
{
 int a,b;
 if (!touched) return;
 a = vote(1, 0x40404, 0);
 b = vote(0, 0x42204, 0);
 if (!a && !b) touched = false;
 pr_info("TCL_AUDIO_POWER_TEST disable_complete=%d\n", !touched);
}
static void timeout_off(struct work_struct *work) { release_votes(); }
static DECLARE_DELAYED_WORK(off_work, timeout_off);
static int __init start(void)
{
 static const char * const paths[] = {
  "/soc@0/rsc@18200000/regulators-0", "/soc@0/rsc@18200000/regulators-1" };
 static const char * const compatibles[] = {
  "qcom,pm6150-rpmh-regulators", "qcom,pm6150l-rpmh-regulators" };
 static const char * const childnames[] = {"ldo15", "bob"};
 int i,ret = -ENODEV;
 if (cmd_db_read_addr("ldoa15") != 0x42200 || cmd_db_read_addr("bobc1") != 0x40400)
  return -ENODEV;
 for (i=0;i<2;i++) {
  struct device_node *np = of_find_node_by_path(paths[i]), *child;
  if (!np) goto out;
  if (!of_device_is_compatible(np, compatibles[i])) { of_node_put(np); goto out; }
  child=of_get_child_by_name(np,childnames[i]);
  if (child) {of_node_put(child);of_node_put(np);ret=-EBUSY;goto out;}
  banks[i]=of_find_device_by_node(np); of_node_put(np);
  if (!banks[i] || !banks[i]->dev.driver || !banks[i]->dev.parent ||
      !dev_get_drvdata(banks[i]->dev.parent)) goto out;
 }
 /* Board APCC: LDO15_A 1800mV/HPM7; BOB_C 3300mV/AUTO6.
  * Only active votes; neither GPIO nor sleep/wake votes are touched.
  * Stop automatically after 20 seconds, also on a partial failure. */
 touched=true;
 ret=vote(0,0x42200,1800);if(ret)goto fail;
 ret=vote(0,0x42208,7);if(ret)goto fail;
 ret=vote(0,0x42204,1);if(ret)goto fail;
 ret=vote(1,0x40400,3300);if(ret)goto fail;
 ret=vote(1,0x40408,6);if(ret)goto fail;
 ret=vote(1,0x40404,1);if(ret)goto fail;
 pr_info("TCL_AUDIO_POWER_TEST enabled; automatic disable in20s; no GPIO access\n");
 schedule_delayed_work(&off_work,msecs_to_jiffies(20000));
 return 0;
fail:
 release_votes();
 /* Keep the module available to retry cleanup if a disable failed. */
 if(touched) {schedule_delayed_work(&off_work,msecs_to_jiffies(1000));return 0;}
out:
 for(i=0;i<2;i++) if(banks[i]) {put_device(&banks[i]->dev);banks[i]=NULL;}
 return ret;
}
static void __exit stop(void)
{
 int i;
 cancel_delayed_work_sync(&off_work);
 release_votes();
 for(i=0;i<2;i++) if(banks[i]) put_device(&banks[i]->dev);
}
module_init(start);module_exit(stop);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Bounded TCL APCC audio supply test with exact OEM millivolt requests");
