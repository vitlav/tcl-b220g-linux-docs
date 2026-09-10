#include <linux/module.h>
#include <linux/init.h>
#include "/tmp/tcl-ice1-linux/sound/soc/qcom/qdsp6/q6core.h"
static int __init tcl_query_init(void)
{
 int ids[] = {3,4,7,8};
 int i;
 pr_info("TCL_ADSP_QUERY ready_api=%d (may include unsupported-command fallback)\n", q6core_is_adsp_ready());
 for (i=0;i<ARRAY_SIZE(ids);i++) {
  struct q6core_svc_api_info a = { .service_id=ids[i], .api_version=0xffffffff, .api_branch_version=0xffffffff };
  int r=q6core_get_svc_api_info(ids[i], &a);
  pr_info("TCL_ADSP_QUERY service=%d ret=%d api=%#x branch=%#x\n",ids[i],r,a.api_version,a.api_branch_version);
 }
 return 0;
}
static void __exit tcl_query_exit(void) {}
module_init(tcl_query_init);
module_exit(tcl_query_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("One-shot TCL ADSP readiness and service-version query; no audio routes");
