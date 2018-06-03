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
#include <linux/cdev.h>
#include <linux/wait.h>
#include <asm/irq.h>

#define IOM_FPGA_MAJOR 242
#define IOM_FPGA_NAME "stopwatch"

#define IOCTL_FPGA _IOW(242, 0, char*)

#define IOM_FND_ADDRESS 0x08000004

static unsigned char *iom_fpga_fnd_addr;

int iom_fpga_open(struct inode *minode, struct file *mfile) ;
int iom_fpga_release(struct inode *minode, struct file *mfile); 
ssize_t iom_fpga_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) ;
ssize_t iom_fpga_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) ;
long iom_fpga_ioctl(struct file *inode, unsigned int ioctl_num, unsigned long ioctl_param); 

ssize_t fpga_fnd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t fpga_fnd_read(struct file *inode, char *gdata, size_t length, loff_t *off_what); 

ssize_t timer_write(struct file *inode, const char *gdata, size_t length, loff_t* off_what);
void kernel_timer_function(unsigned long timerout);

irqreturn_t inter_HomeButton(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_BackButton(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_VolumePButton(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_VolumeMButton(int irq, void* dev_id, struct pt_regs* reg);

wait_queue_head_t wq_write;
DECLARE_WAIT_QUEUE_HEAD(wq_write);

int dev_driver_usage = 0;

struct file_operations iom_fpga_fops = {
	.owner = THIS_MODULE,
	.open = iom_fpga_open,
	.write = iom_fpga_write,
	.unlocked_ioctl = iom_fpga_ioctl,
	.release = iom_fpga_release,
};

struct struct_timer{
	struct timer_list timer;
	int count;
};

struct struct_timer timerInfo;

struct {
	struct file *inode;
	int fndMinValue;
  int fndSecValue;
	char fndData[4];
	int timeInterval;
	int times;
}userData;

// when fpga_dot device open ,call this function
int iom_fpga_open(struct inode *minode, struct file *mfile) 
{	
  int ret = 0, irq = 0;
	if(dev_driver_usage != 0) return -EBUSY;
  dev_driver_usage = 1;

//irqreturn_t inter_HomeButton(int irq, void* dev_id, struct pt_regs* reg);
//irqreturn_t inter_BackButton(int irq, void* dev_id, struct pt_regs* reg);
//irqreturn_t inter_VolumePButton(int irq, void* dev_id, struct pt_regs* reg);
//irqreturn_t inter_VolumeMButton(int irq, void* dev_id, struct pt_regs* reg);
  gpio_direction_input(IMX_GPIO_NR(1,11));
  irq = gpio_to_irq(IMX_GPIO_NR(1,11));
  ret = request_irq(irq, inter_HomeButton, IRQG_TRIGGER_FALLING, "home", 0);


  gpio_direction_input(IMX_GPIO_NR(1,12));
  irq = gpio_to_irq(IMX_GPIO_NR(1,12));
  ret = request_irq(irq, inter_BackButton, IRQG_TRIGGER_FALLING, "back", 0);


  gpio_direction_input(IMX_GPIO_NR(2,15));
  irq = gpio_to_irq(IMX_GPIO_NR(2,15));
  ret = request_irq(irq, inter_VolumePButton, IRQG_TRIGGER_FALLING, "volup", 0);


  gpio_direction_input(IMX_GPIO_NR(5,14));
  irq = gpio_to_irq(IMX_GPIO_NR(5,14));
  ret = request_irq(irq, inter_VolumePButton, IRQG_TRIGGER_FALLING, "voldown", 0);


	return 0;
}

// when fpga_dot device close ,call this function
int iom_fpga_release(struct inode *minode, struct file *mfile) 
{
	dev_driver_usage = 0;

  free_irq(gpio_to_irq(IMX_GPIO_NR(1,11), NULL);
  free_irq(gpio_to_irq(IMX_GPIO_NR(1,12), NULL);
  free_irq(gpio_to_irq(IMX_GPIO_NR(2,15), NULL);
  free_irq(gpio_to_irq(IMX_GPIO_NR(5,14), NULL);

	return 0;
}

// when write to fpga_dot device  ,call this function
ssize_t iom_fpga_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	userData.inode = inode;
  //userData.fndIndex = (int)gdata[0];
  //userData.fndOriginValue = userData.fndValue = (int)gdata[1];
  userData.fndMinValue = 0;
  userData.fndSecValue = 0;
	userData.timeInterval = 10;
	userData.times = (int)gdata[3];
	memset(userData.fndData, 0, sizeof(userData.fndData));

	//printk("%s\n", userString.str1);
	//printk("%d %d %d %d\n", (int)gdata[0], userData.fndValue, userData.timeInterval, userData.times);

	fpga_fnd_write(inode, userData.fndData, length, off_what); 
	timer_write(inode, 0, 1, off_what);

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

long iom_fpga_ioctl(struct file *inode, unsigned int ioctl_num, unsigned long ioctl_param){
	char* gdata = (char*)ioctl_param;
	
	switch (ioctl_num){
		case IOCTL_FPGA:
			/*userData.inode = inode;
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

			fpga_dot_write(inode, fpga_number[userData.fndValue], 10, 0);
			fpga_fnd_write(inode, userData.fndData, 4, 0); 
			led_write(inode, &(userData.ledValue), 1, 0);
			fpga_text_lcd_write(inode, userString.fullString, MAX_BUFF, 0);
			timer_write(inode, 0, 1, 0);*/
			break;
	}

	return 0;
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
	if( p_timer->count <= 0 ){
		return ;
	}

	timerInfo.timer.expires = get_jiffies_64() + (userData.timeInterval*HZ/10);
	timerInfo.timer.data = (unsigned long)&timerInfo;
	timerInfo.timer.function = kernel_timer_function;

	add_timer(&timerInfo.timer);
}

irqreturn_t inter_HomeButton(int irq, void* dev_id, struct pt_regs* reg){

  
  return IRQ_HANDLED;
}


irqreturn_t inter_BackButton(int irq, void* dev_id, struct pt_regs* reg){

  return IRQ_HANDLED;
}

irqreturn_t inter_VolumePButton(int irq, void* dev_id, struct pt_regs* reg){

  return IRQ_HANDLED;
}

irqreturn_t inter_VolumeMButton(int irq, void* dev_id, struct pt_regs* reg){

  return IRQ_HANDLED;
}

int __init iom_fpga_init(void)
{
	int result;

	result = register_chrdev(IOM_FPGA_MAJOR, IOM_FPGA_NAME, &iom_fpga_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}

	iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);

	init_timer(&(timerInfo.timer));

	printk("init module, %s major number : %d\n", IOM_FPGA_NAME, IOM_FPGA_MAJOR);
	
	return 0;
}

void __exit iom_fpga_exit(void) 
{
	printk("exit module\n");
	del_timer_sync(&timerInfo.timer);
	iounmap(iom_fpga_fnd_addr);

	unregister_chrdev(IOM_FPGA_MAJOR, IOM_FPGA_NAME);
}

module_init(iom_fpga_init);
module_exit(iom_fpga_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huins");
