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
	{ 0x7f43ce8a, "ecrt_master_receive" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0xbcf269a7, "ecrt_master_create_domain" },
	{ 0xbcb172a2, "ecrt_master_send" },
	{ 0x8ed27e38, "ecrt_domain_queue" },
	{ 0x63dae992, "ecrt_master_send_ext" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0xe1a3f32f, "ecrt_domain_reg_pdo_entry_list" },
	{ 0x15ba50a6, "jiffies" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xcb06e09a, "ecrt_domain_process" },
	{ 0xe9c132db, "ecrt_domain_state" },
	{ 0x478366da, "ecrt_master_callbacks" },
	{ 0x3e1e5323, "ecrt_master_state" },
	{ 0x4e98f0c6, "ecrt_slave_config_state" },
	{ 0xabbcecc8, "ecrt_master_slave_config" },
	{ 0x6626afca, "down" },
	{ 0x24d273d1, "add_timer" },
	{ 0x64281078, "ecrt_slave_config_pdos" },
	{ 0x3e115413, "ecrt_domain_external_memory" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0xc7e307ed, "ecrt_domain_size" },
	{ 0x92997ed8, "_printk" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x82ee90dc, "timer_delete_sync" },
	{ 0x37a0cba, "kfree" },
	{ 0x699657b7, "ecrt_master_activate" },
	{ 0x693dce80, "ecrt_release_master" },
	{ 0xcf2a6966, "up" },
	{ 0x59b7a070, "ecrt_request_master" },
};

MODULE_INFO(depends, "ec_master");


MODULE_INFO(srcversion, "959AA38D411A96A439764C4");
