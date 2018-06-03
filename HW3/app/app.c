#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define FPGA_DEVICE "/dev/stopwatch"

#define IOCTL_FPGA _IOW(242, 0, char*)

int main(int argc, char **argv){

	int dev = 0;
	char buf[2] = {0};

	dev = open(FPGA_DEVICE, O_RDWR);
	if( dev < 0 ){
		perror("device file error");
		exit(-1);
	}
	write(dev, buf, 2);
	
	printf("close application\n");
	close(dev);

	return 0;
}
