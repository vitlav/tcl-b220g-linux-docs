// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <soc/qcom/cmd-db.h>
static int __init start(void)
{
 static const char * const ids[] = { "ldoa15", "bobc1", "ldoa10", "ldoc10" };
 int i, ret = cmd_db_ready();
 if (ret) return ret;
 for (i = 0; i < ARRAY_SIZE(ids); i++) {
  size_t len = 0;
  const void *aux = cmd_db_read_aux_data(ids[i], &len);
  pr_info("TCL_AUDIO_POWER_DB %s addr=%08x type=%u aux_len=%zu aux_err=%ld\n",
   ids[i], cmd_db_read_addr(ids[i]), cmd_db_read_slave_id(ids[i]), len,
   IS_ERR(aux) ? PTR_ERR(aux) : 0L);
 }
 return 0;
}
static void __exit stop(void) {}
module_init(start);
module_exit(stop);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Read cached command DB audio resource metadata only; no RPMh votes or GPIO access");
