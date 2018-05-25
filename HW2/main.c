//#include "stdafx.h"
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

#define IOM_FPGA_MAJOR 242
#define IOM_FPGA_NAME "dev_drive"

#define IOM_FND_ADDRESS 0x08000004
#define IOM_FPGA_DOT_ADDRESS 0x08000210 // pysical address
#define IOM_LED_ADDRESS 0x08000016 // pysical address
static unsigned char *iom_fpga_dot_addr;
static unsigned char *iom_fpga_fnd_addr;
static unsigned char *iom_fpga_led_addr;

int iom_fpga_open(struct inode *minode, struct file *mfile) ;
int iom_fpga_release(struct inode *minode, struct file *mfile); 
ssize_t iom_fpga_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) ;
ssize_t iom_fpga_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) ;

ssize_t fpga_dot_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) ;
ssize_t fpga_dot_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) ;
ssize_t fpga_fnd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t fpga_fnd_read(struct file *inode, char *gdata, size_t length, loff_t *off_what); 
ssize_t led_read(struct file *inode, char *gdata, size_t length, loff_t *off_what); 
ssize_t led_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);

struct file_operations iom_fpga_fops = {
	.owner = THIS_MODULE,
	.open = iom_fpga_open,
	.write = iom_fpga_write,
	.read = iom_fpga_read,
	.release = iom_fpga_release,
};


// when fpga_dot device open ,call this function
int iom_fpga_open(struct inode *minode, struct file *mfile) 
{	
	//if(fpga_dot_port_usage != 0) return -EBUSY;

	//fpga_dot_port_usage = 1;


	return 0;
}

// when fpga_dot device close ,call this function
int iom_fpga_release(struct inode *minode, struct file *mfile) 
{
	//fpga_dot_port_usage = 0;

	return 0;
}

// when write to fpga_dot device  ,call this function
ssize_t iom_fpga_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	
	fpga_dot_write(inode, gdata, length, off_what);
	fpga_fnd_write(inode, gdata, length, off_what); 
	led_write(inode, gdata, length, off_what);
	//return length;
	return 0;
}

ssize_t iom_fpga_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) 
{
	fpga_dot_read(inode, gdata, length, off_what);
	fpga_fnd_read(inode, gdata, length, off_what);
	led_read(inode, gdata, length, off_what) ;	

	//return length;
	return 0;
}

ssize_t fpga_dot_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	int i;

	unsigned char value[10];
	unsigned short int _s_value;
	const char *tmp = gdata;

	if (copy_from_user(&value, tmp, length))
		return -EFAULT;

	for(i=0;i<length;i++)
    {
        _s_value = value[i] & 0x7F;
		outw(_s_value,(unsigned int)iom_fpga_dot_addr+i*2);
    }
	
	return length;
}

ssize_t fpga_dot_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) {

	//int i;
	unsigned char value[4];
	unsigned short int value_short = 0;
	char *tmp = gdata;

    value_short = inw((unsigned int)iom_fpga_fnd_addr);
    value[0] =(value_short >> 12) & 0xF;
    value[1] =(value_short >> 8) & 0xF;
    value[2] =(value_short >> 4) & 0xF;
    value[3] = value_short & 0xF;

    if (copy_to_user(tmp, value, 4))
        return -EFAULT;

	return length;
}


ssize_t fpga_fnd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	//int i;
	unsigned char value[4];
	unsigned short int value_short = 0;
	const char *tmp = gdata;

	if (copy_from_user(&value, tmp, 4))
		return -EFAULT;

    value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(value_short,(unsigned int)iom_fpga_fnd_addr);

	return length;
}

int __init iom_fpga_init(void)
{
	int result;

	result = register_chrdev(IOM_FPGA_MAJOR, IOM_FPGA_NAME, &iom_fpga_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}

	iom_fpga_dot_addr = ioremap(IOM_FPGA_DOT_ADDRESS, 0x10);
	iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
	iom_fpga_led_addr = ioremap(IOM_LED_ADDRESS, 0x1);

	printk("init module, %s major number : %d\n", IOM_FPGA_NAME, IOM_FPGA_MAJOR);
	
	return 0;
}

ssize_t fpga_fnd_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) 
{
	//int i;
	unsigned char value[4];
	unsigned short int value_short = 0;
	char *tmp = gdata;

    value_short = inw((unsigned int)iom_fpga_fnd_addr);	    
    value[0] =(value_short >> 12) & 0xF;
    value[1] =(value_short >> 8) & 0xF;
    value[2] =(value_short >> 4) & 0xF;
    value[3] = value_short & 0xF;

    if (copy_to_user(tmp, value, 4))
        return -EFAULT;

	return length;
}

ssize_t led_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
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

ssize_t led_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) 
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

void __exit iom_fpga_exit(void) 
{
	iounmap(iom_fpga_dot_addr);
	unregister_chrdev(IOM_FPGA_MAJOR, IOM_FPGA_NAME);
	iounmap(iom_fpga_fnd_addr);
	iounmap(iom_fpga_led_addr);
}

module_init(iom_fpga_init);
module_exit(iom_fpga_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huins");
