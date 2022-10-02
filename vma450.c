/* 
 *
 *  vma450.c - short description
 *
 *
 */ 

#include <linux/init.h> /* Needed for the macros */ 

#include <linux/kernel.h> /* Needed for pr_info() */ 

#include <linux/module.h> /* Needed by all modules */ 


MODULE_AUTHOR("Luca Baldesi <luca@baldesi.ovh>"); 
MODULE_DESCRIPTION("Driver for I2C device Whadda VMA450"); 
 

static int __init vma450_init(void) 
{ 
        pr_info("Hello, world 2\n"); 
        return 0; 
} 


static void __exit vma450_exit(void) 
{ 
        pr_info("Goodbye, world 2\n"); 
} 

module_init(vma450_init); 
module_exit(vma450_exit); 

 
MODULE_LICENSE("GPLv2");
