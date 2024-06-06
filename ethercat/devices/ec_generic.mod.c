#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x4a4cf5e2, "module_layout" },
	{ 0x57f660fc, "kmalloc_caches" },
	{ 0x44120af3, "ecdev_open" },
	{ 0x8b4596ee, "ecdev_withdraw" },
	{ 0x51d3864a, "kernel_sendmsg" },
	{ 0x7f3eefac, "sock_release" },
	{ 0x3e95ffc1, "dev_base_lock" },
	{ 0xf50f4bbc, "rt_read_unlock" },
	{ 0xa4b9631d, "sock_create_kern" },
	{ 0x3ea5fb10, "ecdev_offer" },
	{ 0x78f63cff, "rt_read_lock" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x6afc6ada, "free_netdev" },
	{ 0x9166fada, "strncpy" },
	{ 0x983a9834, "init_net" },
	{ 0xb87160bc, "ecdev_receive" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0xfbcc5908, "ecdev_set_link" },
	{ 0x92997ed8, "_printk" },
	{ 0xbf60457c, "alloc_netdev_mqs" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x2290e39e, "ether_setup" },
	{ 0x34d1f098, "kmem_cache_alloc_trace" },
	{ 0x80c97952, "kernel_recvmsg" },
	{ 0x37a0cba, "kfree" },
	{ 0x89791643, "kernel_bind" },
	{ 0x4f214220, "ecdev_close" },
};

MODULE_INFO(depends, "ec_master");


MODULE_INFO(srcversion, "89221CF9FB56F73155E7B93");
