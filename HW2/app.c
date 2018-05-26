#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "./fpga_dot_font.h"

#define FPGA_DEVICE "/dev/dev_driver"

int main(int argc, char **argv){

	char timeInterval=0;
	char times=0;
	char startIndex=0;
	int startValue=0;

	int dev=0;
	char data[4]={0};

	int i = 0;
	int ret = 0;
	char mask = 0xFF;

	if( argc != 4 ){
		printf("please input the 3 parameters! [time interval] [times] [start option]\n");
		printf("ex) ./app [1-100] [1-100] [0001-8000]\n");
		return -1;	
	}

	timeInterval = atoi(argv[1]);
	times = atoi(argv[2]);
	for(i=0; i<strlen(argv[3]); i++){
		if( argv[3][i] != '0' ){
			startIndex = i;
			startValue = atoi(argv[3]);
			break;
		}
	}

	for(i=0; i<4-startIndex-1; i++){
		startValue /= 10;
	}
	startValue %= 10;

	dev = open(FPGA_DEVICE, O_WRONLY);
	if( dev < 0 ){
		printf("Device open error : %s\n", FPGA_DEVICE);
		close(dev);
		exit(1);
	}

	ret = syscall(376, startIndex, (char)startValue, timeInterval, times);

	for(i=3; i>=0; i--){
		data[i] = (char)(ret&mask);
		ret = ret >> 8;
		printf("input data: %d\n", data[i]);
	}

	write(dev, data, 4);

	close(dev);
	return 0;
}
