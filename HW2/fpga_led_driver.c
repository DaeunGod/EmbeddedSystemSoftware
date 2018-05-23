/* FPGA LED Ioremap Control
FILE : fpga_led_driver.c 
AUTH : largest@huins.com */

#include <linux/module.h>
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
#include <linux/version.h>

#include "stdafx.h"

/*#define IOM_LED_MAJOR 260		// ioboard led device major number
#define IOM_LED_NAME "fpga_led"		// ioboard led device name

#define IOM_LED_ADDRESS 0x08000016 // pysical address


//Global variable
static int ledport_usage = 0;
static unsigned char *iom_fpga_led_addr;

// define functions...
ssize_t iom_led_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t iom_led_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
int iom_led_open(struct inode *minode, struct file *mfile);
int iom_led_release(struct inode *minode, struct file *mfile);*/

// define file_operations structure 
struct file_operations iom_led_fops =
{
	.owner		=	THIS_MODULE,
	.open		=	iom_led_open,
	.write		=	iom_led_write,	
	.read		=	iom_led_read,	
	.release	=	iom_led_release,
};

// when led device open ,call this function
int iom_led_open(struct inode *minode, struct file *mfile) 
{	
	if(ledport_usage != 0) return -EBUSY;

	ledport_usage = 1;

	return 0;
}

// when led device close ,call this function
int iom_led_release(struct inode *minode, struct file *mfile) 
{
	ledport_usage = 0;

	return 0;
}

// when write to led device  ,call this function
ssize_t iom_led_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	unsigned char value;
	unsigned short _s_value;
	const char *tmp = gdata;

	if (copy_from_user(&value, tmp, 1))
		return -EFAULT;

    _s_value = (unsigned short)value;
    outw(_s_value, (unsigned int)iom_fpga_led_addr);

	
	
	return length;
}

// when read to led device  ,call this function
ssize_t iom_led_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) 
{
	unsigned char value;
	unsigned short _s_value;
	char *tmp = gdata;

	_s_value = inw((unsigned int)iom_fpga_led_addr);	    

    value = _s_value & 0xF;

	if (copy_to_user(tmp, &value, 1))
		return -EFAULT;

	return length;
}

/*int __init iom_led_init(void)
{
	int result;

	result = register_chrdev(IOM_LED_MAJOR, IOM_LED_NAME, &iom_led_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}

	iom_fpga_led_addr = ioremap(IOM_LED_ADDRESS, 0x1);

	printk("init module, %s major number : %d\n", IOM_LED_NAME, IOM_LED_MAJOR);

	return 0;
}

void __exit iom_led_exit(void) 
{
	iounmap(iom_fpga_led_addr);
	unregister_chrdev(IOM_LED_MAJOR, IOM_LED_NAME);
}

module_init(iom_led_init);
module_exit(iom_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huins");*/
