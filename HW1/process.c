#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <signal.h>

#include <time.h>
#include "reader.h"
#include "writer.h"

/*void FNDmode1(unsigned char *data){
	int dev = open(
	//tm_ptr->tm_hour, tm_ptr->tm_min
}*/

void mode1swbutton(int* swbutton, struct tm *tm_ptr, unsigned char* fndData);
void mode2swbutton(int* swbutton, unsigned char* fndData, int number);
void modesInit(int mode, struct tm *tm_ptr, unsigned char* fndData);
void mode1Init(struct tm **tm_ptr);
void mode2Init(unsigned char *fndData);

int main(){
  int inputProcId, outputProcId;
  key_t key;
  int shmid;
  int *shmaddr = NULL;

  key = ftok("/data", 1);
  shmid = shmget(key, 1024, IPC_CREAT|0644);
  if( shmid == -1 ){
    perror("shmget");
    exit(1);
  }

  if( (inputProcId = fork()) < 0 ){
    perror("fork error");
    exit(1);
  }
  else if( inputProcId == 0 ){ /* -MARK: input Process */
    shmaddr = (int*)shmat(shmid, (int*)NULL, 0);
    readFromDevice(shmaddr);
  }
  else{
    int processingChild, status;
		int mode = 1;
		struct tm *tm_ptr;

		unsigned char fndData[4];
		unsigned char string[32] = {"Hello           World          "};
		int i;

		mode1Init(&tm_ptr);
		fndData[0] = tm_ptr->tm_hour/10; fndData[1] = (tm_ptr->tm_hour)%10;
		fndData[2] = tm_ptr->tm_min/10; fndData[3] = (tm_ptr->tm_min)%10;

		printf("%d %d %d %d\n", fndData[0], fndData[1], fndData[2], fndData[3]);

    shmaddr = (int*)shmat(shmid, (int*)NULL, 0);
		for(i=0; i<4; i++){
			shmaddr[11+i] = fndData[i];
		}
		for(i=0; i<32; i++){
			shmaddr[15+i] = string[i];
		}


    if( (outputProcId = fork()) < 0 ){
      perror("fork error");
      exit(1);
    }
    else if( outputProcId == 0 ){ /* -MARK: output Process */
			if( mode == 1 ){
    		shmaddr = (int*)shmat(shmid, (int*)NULL, 0);
				readFromSM(shmaddr);
				LCD();
				while(1){
      		//usleep(400000);
					FNDmode1();
					readFromSM(shmaddr);
				}
			}
    }

		/* -MARK: main Process */
    while(1){
      int pressedKey = shmaddr[0];
			int swbutton[9];
			int i;
      usleep(400000);
			for(i=0; i<MAX_BUTTON; i++)
				swbutton[i] = shmaddr[1+i];

      if( pressedKey == 158 ){
				/* Quit */
        break;
      }
      else if( pressedKey == 116 ){
      }
      else if( pressedKey == 115 ){
				/* Mode Change + */
				mode = (mode+1)%4;
				modesInit(mode, tm_ptr, fndData);
      }
      else if( pressedKey == 114 ){
				/* Mode Change - */
				mode--;
				if( mode < 0 )
					mode =  4;
				modesInit(mode, tm_ptr, fndData);
      }

			if( mode == 1 ){
				mode1swbutton(swbutton, tm_ptr, fndData);
				for(i=0; i<4; i++){
					shmaddr[11+i] = fndData[i];
				}
			}
			else if( mode == 2 ){
				mode2swbutton(swbutton, fndData, 10);
			}
    }

    //prctl(PR_SET_PDEATHSIG, SIGTERM);
		//kill(inputProcId, SIGTERM);
		//kill(outputProcId, SIGTERM);

    /* waiting for running child*/
    while( (processingChild = wait(&status)) > 0 );
  } 
  return 0;
}


void mode1swbutton(int* swbutton, struct tm *tm_ptr, unsigned char* fndData){
	static int isChange = 0;
	if( swbutton[0] == KEY_PRESS ){
		isChange = !isChange;
	}

	if( isChange == 1 ){
		if( swbutton[1] == KEY_PRESS ){
			//time_t the_time = time(NULL);
			//tm_ptr = localtime(&the_time);
			mode1Init(&tm_ptr);
		}
		if( swbutton[2] == KEY_PRESS ){
			tm_ptr->tm_hour++;
		}
		if( swbutton[3] == KEY_PRESS ){
			tm_ptr->tm_min++;
		}
	}
	fndData[0] = tm_ptr->tm_hour/10; fndData[1] = (tm_ptr->tm_hour)%10;
	fndData[2] = tm_ptr->tm_min/10; fndData[3] = (tm_ptr->tm_min)%10;
}

void mode1Init(struct tm **tm_ptr){
	time_t t = time(NULL);
	*tm_ptr = localtime(&t);
	//printf("t: %d %d\n", tm_ptr->tm_hour, tm_ptr->tm_min);
}

void mode2swbutton(int* swbutton, unsigned char* fndData, int number){
	if( swbutton[0] == KEY_PRESS ){
	}
	if( swbutton[1] == KEY_PRESS ){
		fndData[1] = (fndData[1]+1)%number;
	}
	if( swbutton[2] == KEY_PRESS ){
		fndData[2] = fndData[2]+1;
		fndData[1] += fndData[2]/number;
		fndData[1] = fndData[1]%number;
		fndData[2] = fndData[2]%number;
	}
	if( swbutton[3] == KEY_PRESS ){
		fndData[3]++;
		fndData[2] += fndData[3]/number;
		fndData[1] += fndData[2]/number;
		fndData[1] = fndData[1]%number;
		fndData[2] = fndData[2]%number;
		fndData[3] = fndData[3]%number;
	}
}

void mode2Init(unsigned char *fndData){
	memset(fndData, 0, sizeof(unsigned char)*4);
}

void modesInit(int mode, struct tm *tm_ptr, unsigned char* fndData){
	if( mode == 1)
		mode1Init(&tm_ptr);
	else if( mode == 2 )
		mode2Init(fndData);
}
