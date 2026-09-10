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


MODULE_INFO(depends, "soundwire-bus,slimbus");

MODULE_ALIAS("of:N*T*Cqcom,soundwire-v1.3.0");
MODULE_ALIAS("of:N*T*Cqcom,soundwire-v1.3.0C*");
MODULE_ALIAS("of:N*T*Cqcom,soundwire-v1.5.1");
MODULE_ALIAS("of:N*T*Cqcom,soundwire-v1.5.1C*");
MODULE_ALIAS("of:N*T*Cqcom,soundwire-v1.6.0");
MODULE_ALIAS("of:N*T*Cqcom,soundwire-v1.6.0C*");
MODULE_ALIAS("of:N*T*Cqcom,soundwire-v1.7.0");
MODULE_ALIAS("of:N*T*Cqcom,soundwire-v1.7.0C*");
MODULE_ALIAS("of:N*T*Cqcom,soundwire-v2.0.0");
MODULE_ALIAS("of:N*T*Cqcom,soundwire-v2.0.0C*");
