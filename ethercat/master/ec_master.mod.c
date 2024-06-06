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
	{ 0xa26c1342, "cdev_del" },
	{ 0x57f660fc, "kmalloc_caches" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0xb515e09, "cdev_init" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x754d539c, "strlen" },
	{ 0x20000329, "simple_strtoul" },
	{ 0x6bd0e573, "down_interruptible" },
	{ 0x6729d3df, "__get_user_4" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x999e8297, "vfree" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x21271fd0, "copy_user_enhanced_fast_string" },
	{ 0xfb8b4183, "kthread_create_on_node" },
	{ 0x15ba50a6, "jiffies" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x2ecbfbbb, "kthread_bind" },
	{ 0x715f9174, "__netdev_alloc_skb" },
	{ 0x59843e77, "__init_waitqueue_head" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xf1cbc5ea, "param_ops_charp" },
	{ 0xfb578fc5, "memset" },
	{ 0xbfbaf5f0, "current_task" },
	{ 0xc94654a7, "kthread_stop" },
	{ 0x1f199d24, "copy_user_generic_string" },
	{ 0x9166fada, "strncpy" },
	{ 0xc594a129, "skb_push" },
	{ 0x6626afca, "down" },
	{ 0x18769c57, "device_create" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0xd9ddd5e, "cdev_add" },
	{ 0xecdcabd2, "copy_user_generic_unrolled" },
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0x793bc6bb, "module_put" },
	{ 0x6383b27c, "__x86_indirect_thunk_rdx" },
	{ 0xb2fd5ceb, "__put_user_4" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x1000e51, "schedule" },
	{ 0x8ddd8aad, "schedule_timeout" },
	{ 0x92997ed8, "_printk" },
	{ 0x65487097, "__x86_indirect_thunk_rax" },
	{ 0x1bbf3178, "wake_up_process" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x34d1f098, "kmem_cache_alloc_trace" },
	{ 0xc9eb0768, "__wake_up" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0x781a4928, "prepare_to_wait_event" },
	{ 0x37a0cba, "kfree" },
	{ 0x69acdf38, "memcpy" },
	{ 0x8c38b6bb, "param_array_ops" },
	{ 0xcf2a6966, "up" },
	{ 0x10a27d0a, "class_destroy" },
	{ 0x5a4896a8, "__put_user_2" },
	{ 0xfea64d7d, "finish_wait" },
	{ 0x76293dbb, "device_unregister" },
	{ 0x8f9c199c, "__get_user_2" },
	{ 0x656e4a6e, "snprintf" },
	{ 0x8d7850ff, "consume_skb" },
	{ 0xbd3945ad, "vmalloc_to_page" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x713c6cdf, "param_ops_uint" },
	{ 0x946830d1, "__class_create" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x6ef21a7f, "try_module_get" },
	{ 0xe914e41e, "strcpy" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "0F537F12448F07EF7A45465");
