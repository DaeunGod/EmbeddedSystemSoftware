#include "stdafx.h"

int __init iom_fpga_init(void)
{
    int result;
    
    result = register_chrdev(IOM_FPGA_DOT_MAJOR, IOM_FPGA_DOT_NAME, &iom_fpga_dot_fops);
    if(result < 0) {
        printk(KERN_WARNING"Can't get any major\n");
        return result;
    }
    
    iom_fpga_dot_addr = ioremap(IOM_FPGA_DOT_ADDRESS, 0x10);
    
    printk("init module, %s major number : %d\n", IOM_FPGA_DOT_NAME, IOM_FPGA_DOT_MAJOR);
    
    result = register_chrdev(IOM_FND_MAJOR, IOM_FND_NAME, &iom_fpga_fnd_fops);
    if(result < 0) {
        printk(KERN_WARNING"Can't get any major\n");
        return result;
    }
    
    iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
    
    printk("init module, %s major number : %d\n", IOM_FND_NAME, IOM_FND_MAJOR);
    
    result = register_chrdev(IOM_LED_MAJOR, IOM_LED_NAME, &iom_led_fops);
    if(result < 0) {
        printk(KERN_WARNING"Can't get any major\n");
        return result;
    }
    
    iom_fpga_led_addr = ioremap(IOM_LED_ADDRESS, 0x1);
    
    printk("init module, %s major number : %d\n", IOM_LED_NAME, IOM_LED_MAJOR);

    
    return 0;
}

void __exit iom_fpga_exit(void)
{
    iounmap(iom_fpga_dot_addr);
    unregister_chrdev(IOM_FPGA_DOT_MAJOR, IOM_FPGA_DOT_NAME);
    
    iounmap(iom_fpga_fnd_addr);
    unregister_chrdev(IOM_FND_MAJOR, IOM_FND_NAME);
    
    iounmap(iom_fpga_led_addr);
    unregister_chrdev(IOM_LED_MAJOR, IOM_LED_NAME);
}

module_init(iom_fpga_init);
module_exit(iom_fpga_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huins");
