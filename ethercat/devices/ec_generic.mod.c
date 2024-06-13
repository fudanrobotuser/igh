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
	{ 0x1bcdef37, "module_layout" },
	{ 0xdc504605, "kmalloc_caches" },
	{ 0xbf0cfa73, "ecdev_open" },
	{ 0xedd1a1d1, "ecdev_withdraw" },
	{ 0xaa0b7fe5, "kernel_sendmsg" },
	{ 0xaeceade8, "sock_release" },
	{ 0xd50c0fb7, "dev_base_lock" },
	{ 0x8d97b58d, "rt_read_unlock" },
	{ 0x57a67eb7, "sock_create_kern" },
	{ 0x4e4e8cd0, "ecdev_offer" },
	{ 0x4636ff43, "rt_read_lock" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x408c32f5, "free_netdev" },
	{ 0x9166fada, "strncpy" },
	{ 0xb95737fb, "init_net" },
	{ 0x9bde47df, "ecdev_receive" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x5c5e432d, "ecdev_set_link" },
	{ 0x92997ed8, "_printk" },
	{ 0xa0f56184, "alloc_netdev_mqs" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x9e40ebf2, "ether_setup" },
	{ 0xb0f1603a, "kmem_cache_alloc_trace" },
	{ 0xa885d61c, "kernel_recvmsg" },
	{ 0x37a0cba, "kfree" },
	{ 0x40ac10ec, "kernel_bind" },
	{ 0x272501f8, "ecdev_close" },
};

MODULE_INFO(depends, "ec_master");


MODULE_INFO(srcversion, "848BB80F1C588A2FDA42EDB");
