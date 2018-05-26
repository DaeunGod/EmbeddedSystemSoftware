#include <linux/kernel.h>



asmlinkage int sys_makestream(char startIndex, char startValue, 
		char timeInterval, char times){

	int value = (startIndex<<24) | (startValue<<16) | (timeInterval<<8) | (times);
	return value;
}

