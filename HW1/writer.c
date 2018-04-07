#include "writer.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/mman.h>

#define FPGA_BASE_ADDRESS 0x08000000
#define LED_ADDR 0x16

unsigned char fndData[4];
unsigned char string[32];

void readFromSM(int* shmaddr, int *mode, int* isChange){
	int i;
	*mode = shmaddr[10];
	*isChange = shmaddr[11];
	for(i=0; i<4; i++){
		fndData[i] = shmaddr[12+i];
	}
	for(i=0; i<32; i++){
		string[i] = shmaddr[16+i];
	}
}

void FNDmode1(){
	char *fndFile = "/dev/fpga_fnd";
	unsigned char retval;

	int dev = open(fndFile, O_RDWR);
	if( dev < 0 ){
		perror("FND open error");
		close(dev);
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

void LED(int n){
	int fd;

	unsigned long *fpga_addr = 0;
	unsigned char *led_addr = 0;
	
	if( n < 0 || n > 255 ){
		printf("LED data error\n");
		return ;
	}

	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if( fd < 0 ){
		perror("/dev/mem open error");
		close(fd);
		return ;
	}

	fpga_addr = (unsigned long*)mmap(NULL, 4096, 
			PROT_READ | PROT_WRITE, MAP_SHARED, fd, FPGA_BASE_ADDRESS);
	if( fpga_addr == MAP_FAILED ){
		printf("mmap error\n");
		close(fd);
		return ;
	}

	led_addr = (unsigned char*)((void*)fpga_addr + LED_ADDR);

	*led_addr = n;

	sleep(1);
	munmap(led_addr, 4096);
	close(fd);
}

int main(int argc, char* argv[]){
  key_t key;
  int shmid = atoi(argv[1]);
  int *shmaddr = NULL;
	int mode=0, ledstat=0;

  shmaddr = (int*)shmat(shmid, (int*)NULL, 0);
	//if( mode == 1 ){
	readFromSM(shmaddr, &mode, &ledstat); // Initialize
	LCD();
	LED(128);
	while(1){
   	//usleep(400000);
		FNDmode1();
		readFromSM(shmaddr, &mode, &ledstat);

		if( mode == 1 ){
			if( ledstat == 1 ){
				static int check = 0;
				LED( (check ? 32 : 16) );
				check = !check;
			}
			else
				LED(128);
			
			//printf("mode : %d, %d\n", mode, isChange);
		}
		else if( mode == 2 ){
			if( ledstat == 10 )
				LED(64);
			else if( ledstat == 8 )
				LED(32);
			else if( ledstat == 4 )
				LED(16);
			else
				LED(128);
		}
	}
	//}
	return 0;
}
