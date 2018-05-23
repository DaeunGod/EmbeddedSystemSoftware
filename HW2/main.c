/* FPGA Dot Matrix Ioremap Control
FILE : fpga_dot_driver.c 
AUTH : largest@huins.com*/

/*#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>*/

#include "stdafx.h"


int __init iom_fpga_dot_init(void)
{
	/*int result;

	result = register_chrdev(IOM_FPGA_DOT_MAJOR, IOM_FPGA_DOT_NAME, &iom_fpga_dot_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}

	iom_fpga_dot_addr = ioremap(IOM_FPGA_DOT_ADDRESS, 0x10);
*/
	//printk("init module, %s major number : %d\n", IOM_FPGA_DOT_NAME, IOM_FPGA_DOT_MAJOR);
	printk("init module\n");

	return 0;
}

void __exit iom_fpga_dot_exit(void) 
{
	//iounmap(iom_fpga_dot_addr);
	//unregister_chrdev(IOM_FPGA_DOT_MAJOR, IOM_FPGA_DOT_NAME);
}

module_init(iom_fpga_dot_init);
module_exit(iom_fpga_dot_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huins");
