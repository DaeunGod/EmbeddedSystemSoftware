//#include "stdafx.h"
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>

#include "fpga_dot_font.h"

#define IOM_FPGA_MAJOR 242
#define IOM_FPGA_NAME "dev_driver"

#define IOM_FND_ADDRESS 0x08000004
#define IOM_FPGA_DOT_ADDRESS 0x08000210 // pysical address
#define IOM_LED_ADDRESS 0x08000016 // pysical address
#define IOM_TEXT_LCD_ADDRESS 0x08000090 // pysical address

#define LINE_BUFF 16
#define MAX_BUFF 32

static unsigned char *iom_fpga_dot_addr;
static unsigned char *iom_fpga_fnd_addr;
static unsigned char *iom_fpga_led_addr;
static unsigned char *iom_fpga_text_lcd_addr;

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
ssize_t fpga_text_lcd_write(struct file *inode, const char *gdata, size_t length, loff_t* off_what);

ssize_t timer_write(struct file *inode, const char *gdata, size_t length, loff_t* off_what);
void kernel_timer_function(unsigned long timerout);

void fpga_update_dataNdisplay(void);
void fpga_construct_fullString(void);
void fpga_update_string(void);
void init_user_dataNstring(void);

struct file_operations iom_fpga_fops = {
	.owner = THIS_MODULE,
	.open = iom_fpga_open,
	.write = iom_fpga_write,
	.read = iom_fpga_read,
	.release = iom_fpga_release,
};

struct struct_timer{
	struct timer_list timer;
	int count;
};

struct struct_timer timerInfo;

struct {
	struct file *inode;
	int fndOriginValue;
	int fndIndex;
	int fndValue;
	char fndData[4];
	char ledValue;
	int timeInterval;
	int times;
}userData;

struct{
	int dir1;
	int dir2;
	int space1;
	int space2;
	char str1[8];
	char str2[9];
	char fullString[33];
}userString;

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
	userData.inode = inode;
  userData.fndIndex = (int)gdata[0];
  userData.fndOriginValue = userData.fndValue = (int)gdata[1];
	userData.ledValue = 1 << (8-userData.fndValue);
	userData.timeInterval = (int)gdata[2];
	userData.times = (int)gdata[3];
	memset(userData.fndData, 0, sizeof(userData.fndData));
  userData.fndData[userData.fndIndex] = userData.fndValue;

	userString.dir1 = userString.dir2 = userString.space1 = userString.space2 = 0;
	strncpy(userString.str1, "20131612", 8);
	strncpy(userString.str2, "ChoeDaeun", 9);
	fpga_construct_fullString();

	printk("%s\n", userString.str1);
	printk("%d %d %d %d\n", (int)gdata[0], userData.fndValue, userData.timeInterval, userData.times);

	fpga_dot_write(inode, fpga_number[userData.fndValue], 10, off_what);
	fpga_fnd_write(inode, userData.fndData, length, off_what); 
	led_write(inode, &(userData.ledValue), 1, off_what);
	fpga_text_lcd_write(inode, userString.fullString, MAX_BUFF, off_what);
	timer_write(inode, 0, 1, off_what);
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

  for(i=0; i<10; i++)
    value[i] = gdata[i];

	for(i=0;i<length;i++)
    {
        _s_value = value[i] & 0x7F;
		outw(_s_value,(unsigned int)iom_fpga_dot_addr+i*2);
    }
	
	return length;
}

ssize_t fpga_dot_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) {

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
	int i;
	unsigned char value[4];
	unsigned short int value_short = 0;

  for(i=0; i<4; i++)
    value[i] = gdata[i];

    value_short = value[0] << 12 | value[1] << 8 |value[2] << 4 |value[3];
    outw(value_short,(unsigned int)iom_fpga_fnd_addr);

	return length;
}

ssize_t fpga_fnd_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) 
{
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

  value = *gdata;

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

ssize_t fpga_text_lcd_write(struct file *inode, const char *gdata, size_t length, loff_t* off_what){
	int i;
	unsigned short int _s_value =0;
	unsigned char value[33];

	strncpy(value, gdata, MAX_BUFF);
	value[length] = 0;
	for(i=0; i<length; i++){
		_s_value = ((value[i] & 0xFF) << 8) | (value[i+1] & 0xFF);
		outw( _s_value, (unsigned int)iom_fpga_text_lcd_addr+i);
		i++;
	}

	return length;
}

ssize_t timer_write(struct file *inode, const char *gdata, size_t length, loff_t* off_what){
	timerInfo.count = userData.times;

	del_timer_sync( &timerInfo.timer );

	timerInfo.timer.expires = get_jiffies_64() + (userData.timeInterval * HZ/10);
	timerInfo.timer.data = (unsigned long)&timerInfo;
	timerInfo.timer.function = kernel_timer_function;

	add_timer(&timerInfo.timer);

	return 1;
}

void kernel_timer_function(unsigned long timerout){
	struct struct_timer *p_timer = (struct struct_timer*)timerout;

	printk("timer data: %d\n", p_timer->count);
	p_timer->count--;
	if( !p_timer->count ){
		init_user_dataNstring();
		return ;
	}

	fpga_update_string();
	fpga_construct_fullString();
 	fpga_update_dataNdisplay();

	timerInfo.timer.expires = get_jiffies_64() + (userData.timeInterval*HZ/10);
	timerInfo.timer.data = (unsigned long)&timerInfo;
	timerInfo.timer.function = kernel_timer_function;

	add_timer(&timerInfo.timer);
}

void fpga_update_dataNdisplay(void){
	struct file *inode = userData.inode;

	userData.fndValue++;
	if(userData.fndValue > 8 ){
		userData.fndValue = 1;
	}
	if( userData.fndValue == userData.fndOriginValue ){
		userData.fndIndex++;
		if( userData.fndIndex > 3 ){
			userData.fndIndex = 0;
		}
	}

	memset(userData.fndData, 0, sizeof(userData.fndData));
	userData.fndData[userData.fndIndex] = userData.fndValue;
	userData.ledValue = 1 << (8-userData.fndValue);


	fpga_dot_write(inode, fpga_number[userData.fndValue], 10, 0);
	fpga_fnd_write(inode, userData.fndData, 4, 0); 
	led_write(inode, &(userData.ledValue), 1, 0);
	fpga_text_lcd_write(inode, userString.fullString, MAX_BUFF, 0);
}

void fpga_construct_fullString(void){
	memset(userString.fullString, 0, sizeof(userString.fullString));

	memset(userString.fullString, ' ', userString.space1);
	strncat(userString.fullString, userString.str1, 8);
	memset(userString.fullString+8+userString.space1, ' ', LINE_BUFF-8-userString.space1);

	memset(userString.fullString+LINE_BUFF, ' ', userString.space2);
	strncat(userString.fullString, userString.str2, 9);
	memset(userString.fullString+LINE_BUFF+9+userString.space2, ' ', LINE_BUFF-9-userString.space2);
}

void fpga_update_string(void){
	if( userString.dir1 == 0 ){
		userString.space1++;
		if( userString.space1 + 9 > LINE_BUFF )
			userString.dir1 = 1;
	}
	else{
		userString.space1--;
		if( userString.space1 < 1 )
			userString.dir1 = 0;
	}

	if( userString.dir2 == 0 ){
		userString.space2++;
		if( userString.space2 + 10 > LINE_BUFF )
			userString.dir2 = 1;
	}
	else{
		userString.space2--;
		if( userString.space2 < 1 )
			userString.dir2 = 0;
	}
}

void init_user_dataNstring(void){
	struct file *inode = userData.inode;
	userData.fndOriginValue = 0;
	userData.fndIndex = 0;
	userData.fndValue = 0;
	memset(userData.fndData, 0, sizeof(userData.fndData));
	userData.ledValue = 0;

	memset(userString.fullString, ' ', sizeof(userString.fullString));	

	fpga_dot_write(inode, fpga_set_blank, 10, 0);
	fpga_fnd_write(inode, userData.fndData, 4, 0); 
	led_write(inode, &(userData.ledValue), 1, 0);
	fpga_text_lcd_write(inode, userString.fullString, MAX_BUFF, 0);
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
	iom_fpga_text_lcd_addr = ioremap(IOM_TEXT_LCD_ADDRESS, 0x32);

	init_timer(&(timerInfo.timer));

	printk("init module, %s major number : %d\n", IOM_FPGA_NAME, IOM_FPGA_MAJOR);
	
	return 0;
}

void __exit iom_fpga_exit(void) 
{
	printk("exit module\n");
	del_timer_sync(&timerInfo.timer);
	iounmap(iom_fpga_dot_addr);
	iounmap(iom_fpga_fnd_addr);
	iounmap(iom_fpga_led_addr);
	iounmap(iom_fpga_text_lcd_addr);

	unregister_chrdev(IOM_FPGA_MAJOR, IOM_FPGA_NAME);
}

module_init(iom_fpga_init);
module_exit(iom_fpga_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huins");
