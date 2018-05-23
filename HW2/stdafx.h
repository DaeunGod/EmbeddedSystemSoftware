#ifndef __FPGA_DOT_DRIVER__
#define __FPGA_DOOT_DRIVER__

///----------------------------------DOT-------------------------------------///
#define IOM_FPGA_DOT_MAJOR 262        // ioboard led device major number
#define IOM_FPGA_DOT_NAME "fpga_dot"        // ioboard led device name

#define IOM_FPGA_DOT_ADDRESS 0x08000210 // pysical address

//Global variable
static int fpga_dot_port_usage = 0;
static unsigned char *iom_fpga_dot_addr;

ssize_t iom_fpga_dot_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
int iom_fpga_dot_open(struct inode *minode, struct file *mfile);
int iom_fpga_dot_release(struct inode *minode, struct file *mfile);
///----------------------------------FND-------------------------------------///
#define IOM_FND_MAJOR 261        // ioboard fpga device major number
#define IOM_FND_NAME "fpga_fnd"        // ioboard fpga device name

#define IOM_FND_ADDRESS 0x08000004 // pysical address

//Global variable
static int fpga_fnd_port_usage = 0;
static unsigned char *iom_fpga_fnd_addr;

// define functions...
ssize_t iom_fpga_fnd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t iom_fpga_fnd_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
int iom_fpga_fnd_open(struct inode *minode, struct file *mfile);
int iom_fpga_fnd_release(struct inode *minode, struct file *mfile);

///------------------------------------LED---------------------------------------///
#define IOM_LED_MAJOR 260        // ioboard led device major number
#define IOM_LED_NAME "fpga_led"        // ioboard led device name

#define IOM_LED_ADDRESS 0x08000016 // pysical address
//Global variable
static int ledport_usage = 0;
static unsigned char *iom_fpga_led_addr;

// define functions...
ssize_t iom_led_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t iom_led_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
int iom_led_open(struct inode *minode, struct file *mfile);
int iom_led_release(struct inode *minode, struct file *mfile);

///------------------------------------TEXT---------------------------------------///
#define MAX_BUFF 32
#define LINE_BUFF 16
#define FPGA_TEXT_LCD_DEVICE "/dev/fpga_text_lcd"

#endif
