#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};


MODULE_INFO(depends, "snd-soc-wcd-common,snd-soc-wcd-mbhc,mux-core,soundwire-bus,snd-soc-wcd938x-sdw,snd-soc-wcd-classh");

MODULE_ALIAS("of:N*T*Cqcom,wcd9380-codec");
MODULE_ALIAS("of:N*T*Cqcom,wcd9380-codecC*");
MODULE_ALIAS("of:N*T*Cqcom,wcd9385-codec");
MODULE_ALIAS("of:N*T*Cqcom,wcd9385-codecC*");
