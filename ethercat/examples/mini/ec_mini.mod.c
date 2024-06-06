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
	{ 0x69d6eb86, "ecrt_master_receive" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0xda6ccd6f, "ecrt_master_create_domain" },
	{ 0xeb4adb2a, "ecrt_master_send" },
	{ 0x614a3153, "ecrt_domain_queue" },
	{ 0xd52c8aff, "ecrt_master_send_ext" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0xc9cc3e82, "ecrt_domain_reg_pdo_entry_list" },
	{ 0x15ba50a6, "jiffies" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x8f1a196b, "ecrt_domain_process" },
	{ 0xc6f99885, "ecrt_domain_state" },
	{ 0x9c41fcb5, "ecrt_master_callbacks" },
	{ 0x4d92366a, "ecrt_master_state" },
	{ 0x34406f99, "ecrt_slave_config_state" },
	{ 0x4651527b, "ecrt_master_slave_config" },
	{ 0x6626afca, "down" },
	{ 0x24d273d1, "add_timer" },
	{ 0xf508c4c1, "ecrt_slave_config_pdos" },
	{ 0x1393fd07, "ecrt_domain_external_memory" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x39002fe8, "ecrt_domain_size" },
	{ 0x92997ed8, "_printk" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x82ee90dc, "timer_delete_sync" },
	{ 0x37a0cba, "kfree" },
	{ 0xf90f07af, "ecrt_master_activate" },
	{ 0x7277d7c9, "ecrt_release_master" },
	{ 0xcf2a6966, "up" },
	{ 0xf289f38e, "ecrt_request_master" },
};

MODULE_INFO(depends, "ec_master");


MODULE_INFO(srcversion, "959AA38D411A96A439764C4");
