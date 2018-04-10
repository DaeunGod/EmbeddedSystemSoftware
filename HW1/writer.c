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
#include <time.h>
#include "fpga_dot_font.h"

#define FPGA_BASE_ADDRESS 0x08000000
#define LED_ADDR 0x16

unsigned char fndData[4];
unsigned char string[32];
unsigned char dotTable[10];

void readFromSM(int* shmaddr, int *mode, int *ledstat, int *dotMat){
	int i;
	*mode = shmaddr[10];
	*ledstat = shmaddr[11];
	for(i=0; i<4; i++){
		fndData[i] = shmaddr[12+i];
	}
	for(i=0; i<32; i++){
		string[i] = shmaddr[16+i];
	}
	*dotMat = shmaddr[48];
	for(i=0; i<10; i++){
		dotTable[i] = shmaddr[49+i];
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

	retval = write(dev, &fndData, 4);
	if( retval < 0 ){
		perror("FND write error'");
		exit(1);
	}

	close(dev);
}

void LCD(){
	char *lcdFile = "/dev/fpga_text_lcd";
	int dev = open(lcdFile, O_WRONLY);

  if( dev < 0 ){
    perror("LCD open error");
    close(dev);
    exit(1);
  }
	write(dev, string, 32);
	close(dev);
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

	//sleep(1);
	munmap(led_addr, 4096);
	close(fd);
}

void Dot(int dotMatrix){
	int dev = open("/dev/fpga_dot", O_WRONLY);
	int size;
	if( dev < 0 ){
		perror("Dot Matrix");
		close(dev);
		return ;
	}
	

	if( dotMatrix == -1 )
		write(dev, fpga_set_blank, sizeof(fpga_set_blank));
	else if( dotMatrix == -2){
		size = sizeof(dotTable);
		write(dev, dotTable, size);
	}
	else {
		size = sizeof(fpga_number[dotMatrix]);
		write(dev, fpga_number[dotMatrix], size);
	}

	close(dev);
}

void Dot2(){
	int dev = open("/dev/fpga_dot", O_WRONLY);
	int size;
	int i;
	if( dev < 0 ){
		perror("Dot Matrix");
		close(dev);
		return ;
	}

	size = sizeof(dotTable);
	for(i=0; i<10; i++)
		printf("dot %d: %x\n", i, dotTable[i]);
	write(dev, dotTable, size);

	//sleep(1);
	close(dev);
}


int main(int argc, char* argv[]){
  key_t key;
  int shmid = atoi(argv[1]);
  int *shmaddr = NULL;
	int mode=0, ledstat=0, dotMatrix;
	time_t initTime, currentTime;
	time(&initTime);
	time(&currentTime);

  shmaddr = (int*)shmat(shmid, (int*)NULL, 0);
	readFromSM(shmaddr, &mode, &ledstat, &dotMatrix); // Initialize
	LCD();
	LED(128);
	Dot(-1);
	while(1){
		readFromSM(shmaddr, &mode, &ledstat, &dotMatrix);
		FNDmode1();
		//Dot(-1);
		if( mode == 1 ){
			time(&currentTime);
			Dot(-1);
			if( currentTime - initTime >= 1 ){
				initTime = currentTime;
				if( ledstat == 1 ){
					static int check = 0;
					LED( (check ? 32 : 16) );
					check = !check;
				}
				else
					LED(128);
			}
		}
		else if( mode == 2 ){
			time(&currentTime);
			Dot(-1);
			if( currentTime - initTime >= 1 ){
				initTime = currentTime;
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
		else if( mode == 3 ){
			time(&currentTime);
			if( currentTime - initTime >= 1 ){
				initTime = currentTime;
				Dot(dotMatrix);
				LED(0);
			}
		}
		else if( mode == 4 ){
			Dot(-2);
		}
		else if( mode == 5 ){
			//printf("dot mat : %d\n", dotMatrix);
			Dot(dotMatrix);
		}

		LCD();
	}
	return 0;
}
