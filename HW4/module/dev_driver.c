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
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <asm/irq.h>
#include <mach/gpio.h>
#include <asm/gpio.h>
#include <linux/interrupt.h>

#include "fpga_dot_font.h"

#define IOM_FPGA_MAJOR 242
#define IOM_FPGA_NAME "dev_driver"

#define IOM_FND_ADDRESS 0x08000004
#define IOM_DOT_ADDRESS 0x08000210

unsigned char *iom_fpga_fnd_addr;
unsigned char *iom_fpga_dot_addr;

int iom_fpga_open(struct inode *minode, struct file *mfile) ;
int iom_fpga_release(struct inode *minode, struct file *mfile); 
ssize_t iom_fpga_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) ;
ssize_t iom_fpga_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);

ssize_t fpga_fnd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t fpga_dot_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) ;

irqreturn_t inter_HomeButton(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_VolumePButton(int irq, void* dev_id, struct pt_regs* reg);
irqreturn_t inter_VolumeMButton(int irq, void* dev_id, struct pt_regs* reg);

int inter_register_cdev(void);
void timer_write(void);
void kernel_timer_function(unsigned long timerout);

wait_queue_head_t wq_write;
DECLARE_WAIT_QUEUE_HEAD(wq_write);

int dev_driver_usage = 0;
dev_t inter_dev;
struct cdev inter_cdev;

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
	char fndData[4];
	int dotIndex;	
	int mode;
	int isTimerPause;
	int timerTime;
	int isDone;
	char storedFndData[4];
}userData;

// when fpga_dot device open ,call this function
int iom_fpga_open(struct inode *minode, struct file *mfile) 
{	
  int ret = 0, irq = 0;
	if(dev_driver_usage != 0) return -EBUSY;
  dev_driver_usage = 1;

  gpio_direction_input(IMX_GPIO_NR(1,11));
  irq = gpio_to_irq(IMX_GPIO_NR(1,11));
  ret = request_irq(irq, (void*)inter_HomeButton, IRQF_TRIGGER_FALLING, "home", 0);

  gpio_direction_input(IMX_GPIO_NR(2,15));
  irq = gpio_to_irq(IMX_GPIO_NR(2,15));
  ret = request_irq(irq, (void*)inter_VolumePButton, IRQF_TRIGGER_FALLING, "volume-up", 0);


  gpio_direction_input(IMX_GPIO_NR(5,14));
  irq = gpio_to_irq(IMX_GPIO_NR(5,14));
  ret = request_irq(irq, (void*)inter_VolumeMButton, IRQF_TRIGGER_FALLING, "volume-down", 0);

	userData.mode = 0;
	userData.timerTime = 0;
	userData.isDone = 0;
	userData.isTimerPause = 1;
	return 0;
}

// when fpga_dot device close ,call this function
int iom_fpga_release(struct inode *minode, struct file *mfile) 
{
	dev_driver_usage = 0;

  free_irq(gpio_to_irq(IMX_GPIO_NR(1,11)), NULL);
  free_irq(gpio_to_irq(IMX_GPIO_NR(2,15)), NULL);
  free_irq(gpio_to_irq(IMX_GPIO_NR(5,14)), NULL);

	return 0;
}

// when write to fpga_dot device  ,call this function
ssize_t iom_fpga_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
	int i;
	userData.inode = inode;
	memset(userData.fndData, 0, sizeof(userData.fndData));

	for(i=0; i<length; i++){
			userData.storedFndData[i] = gdata[i];
	}

	if( userData.mode == 0 ){
		for(i=0; i < length; i++){
			userData.fndData[i] = gdata[i];
		}
		userData.dotIndex = gdata[3];

		fpga_fnd_write(inode, userData.fndData, length, off_what);
 		fpga_dot_write(inode, fpga_number[userData.dotIndex], 10, off_what);	
	}
	return 0;
}

ssize_t iom_fpga_read(struct file *inode, char *gdata, size_t length, loff_t *off_what){
	return userData.isDone;
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

void fpag_update_data(void){
}

irqreturn_t inter_HomeButton(int irq, void* dev_id, struct pt_regs* reg){
	printk(KERN_ALERT "Home button %x\n", gpio_get_value(IMX_GPIO_NR(1, 11)));
  
	if( userData.mode == 0 ){
		char fnd[4] = {0};
		userData.mode = 1;
		userData.isTimerPause = 1;
		userData.timerTime = 0;

		fpga_fnd_write(userData.inode, fnd, 4, 0);
 		fpga_dot_write(userData.inode, fpga_set_blank, 10, 0);	
	}
	else{
		int i=0;
		userData.mode = 0;
		for(i=0; i<4; i++){
			userData.fndData[i] = userData.storedFndData[i];
		}
		userData.dotIndex = userData.fndData[3];
		fpga_fnd_write(userData.inode, userData.fndData, 4, 0);
 		fpga_dot_write(userData.inode, fpga_number[userData.dotIndex], 10, 0);	
	}
  return IRQ_HANDLED;
}

irqreturn_t inter_VolumePButton(int irq, void* dev_id, struct pt_regs* reg){
	char fnd[4] = {0};
	int time = 0;
	printk(KERN_ALERT "V+ button %x\n", gpio_get_value(IMX_GPIO_NR(2, 15)));
	
	if( userData.mode == 1 ){
		if( userData.isTimerPause == 1 ){
			userData.timerTime++;
			if( userData.timerTime > 60 )
				userData.timerTime = 0;

			time = userData.timerTime;
			fnd[3] = time%10;
			time = time/10;
			fnd[2] = time%10;
			fpga_fnd_write(userData.inode, fnd, 4, 0);
		}
	}

  return IRQ_HANDLED;
}

irqreturn_t inter_VolumeMButton(int irq, void* dev_id, struct pt_regs* reg){
	//int input = (int)gpio_get_value(IMX_GPIO_NR(5,14));
	printk(KERN_ALERT "V- button %x\n", gpio_get_value(IMX_GPIO_NR(5, 14)));

	if( userData.mode == 1){
		if( userData.isTimerPause == 1 ){
			userData.isTimerPause = 0;

			timer_write();
		}
	}
  return IRQ_HANDLED;
}

void timer_write(void){
	char fnd[4]={0};
	int time=0;
	del_timer_sync( &timerInfo.timer );

	time = userData.timerTime;
	fnd[3] = time%10;
	time = time/10;
	fnd[2] = time%10;
	fpga_fnd_write(userData.inode, fnd, 4, 0);
	if( userData.timerTime < 10 ){
		userData.dotIndex = userData.timerTime;
 		fpga_dot_write(userData.inode, fpga_number[userData.dotIndex], 10, 0);	
	}

	timerInfo.timer.expires = get_jiffies_64() + HZ;
	timerInfo.timer.data = (unsigned long)&timerInfo;
	timerInfo.timer.function = kernel_timer_function;

	add_timer(&timerInfo.timer);
}

void kernel_timer_function(unsigned long timerout){
	char fnd[4]={0};
	int time=0;
	if( userData.timerTime <= 0 || userData.isTimerPause ){
 		fpga_dot_write(userData.inode, fpga_set_blank, 10, 0);
		userData.isDone = 1;	
		return ;
	}

	userData.timerTime--;

	if( userData.mode == 1 ){
		time = userData.timerTime;
		fnd[3] = time%10;
		time = time/10;
		fnd[2] = time%10;
		fpga_fnd_write(userData.inode, fnd, 4, 0);
		if( userData.timerTime < 10 ){
			userData.dotIndex = userData.timerTime;
 			fpga_dot_write(userData.inode, fpga_number[userData.dotIndex], 10, 0);	
		}
	}

	timerInfo.timer.expires = get_jiffies_64() + HZ;
	timerInfo.timer.data = (unsigned long)&timerInfo;
	timerInfo.timer.function = kernel_timer_function;

	add_timer(&timerInfo.timer);
}

int inter_register_cdev(void){
	int error=0;
	inter_dev = MKDEV(IOM_FPGA_MAJOR, 0);
	error = register_chrdev_region(inter_dev, 1, "inter");

	if( error < 0 ){
		printk(KERN_WARNING"Can't get any major\n");
		return error;
	}

	printk("init module, %s major number : %d\n", IOM_FPGA_NAME, IOM_FPGA_MAJOR);
	cdev_init(&inter_cdev, &iom_fpga_fops);
	inter_cdev.owner = THIS_MODULE;
	inter_cdev.ops = &iom_fpga_fops;

	error = cdev_add(&inter_cdev, inter_dev, 1);

	if( error )
		printk(KERN_NOTICE "inter Register Error %d\n", error);

	return 0;
}

int __init iom_fpga_init(void)
{
	int result;
	if( (result = inter_register_cdev()) < 0){
		return result;
	}

	iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
	iom_fpga_dot_addr = ioremap(IOM_DOT_ADDRESS, 0x10);

	init_timer(&(timerInfo.timer));
	
	return 0;
}

void __exit iom_fpga_exit(void) 
{
	printk("exit module\n");
	del_timer_sync(&timerInfo.timer);
	iounmap(iom_fpga_fnd_addr);
	iounmap(iom_fpga_dot_addr);
	cdev_del(&inter_cdev);

	unregister_chrdev_region(inter_dev, 1);
	//unregister_chrdev(IOM_FPGA_MAJOR, IOM_FPGA_NAME);
}

module_init(iom_fpga_init);
module_exit(iom_fpga_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huins");
