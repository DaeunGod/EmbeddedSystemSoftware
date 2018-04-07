#include "writer.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

unsigned char fndData[4];
unsigned char string[32];

void readFromSM(int* shmaddr){
	int i;
	for(i=0; i<4; i++){
		fndData[i] = shmaddr[11+i];
	}
	for(i=0; i<32; i++){
		string[i] = shmaddr[15+i];
	}
}

void FNDmode1(){
	char *fndFile = "/dev/fpga_fnd";
	unsigned char retval;

	int dev = open(fndFile, O_RDWR);
	if( dev < 0 ){
		perror("FND open error");
		exit(1);
	}

	//printf("%d %d %d %d\n", fndData[0], fndData[1], fndData[2], fndData[3]);
	retval = write(dev, &fndData, 4);
	if( retval < 0 ){
		perror("FND write error'");
		exit(1);
	}

	close(dev);
	//tm_ptr->tm_hour, tm_ptr->tm_min
}

void LCD(){
	char *lcdFile = "/dev/fpga_text_lcd";
	int dev = open(lcdFile, O_WRONLY);

	printf("%s\n", string);
	write(dev, string, 32);
	close(dev);
	memset(string, 0, sizeof(string));
}
