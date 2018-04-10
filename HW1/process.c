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

#define MAX_BUTTON 9
#define KEY_PRESS 1
#define KEY_RELEASE 0

#define FND_START 12
#define STR_START 16
#define DOTTABLE_START 49

typedef struct{
	int curMode;
	struct tm *initTime;
	struct tm *curTime;
	unsigned char fndData[4];
	unsigned char string[32];
	int strIndex;
	int ledValue;
  int	dotIndex;
	int count;
}commonVari;

typedef struct{
		unsigned char dotTable[10];
		unsigned char dotTableFlag[10];
		int dotRow;
		int dotCol;
		int isVisible;
		int count;
}drawDotVari;

void commomVariInit( commonVari* cv, int mode ){
	time_t t = time(NULL);

	cv->curMode = mode;
	cv->initTime = localtime(&t);
	cv->curTime = localtime(&t);
	memset(cv->fndData, 0, sizeof(unsigned char)*4);
	strcpy(cv->string, "Hello           World         \0");
	cv->strIndex = 0;
	cv->ledValue = 0;
	cv->dotIndex = 0;
	cv->count = 0;

	if( mode == 2 ){
		cv->ledValue = 10;
	}
	else if( mode == 3 ){
		cv->dotIndex = 10;
		memset(cv->string, ' ', sizeof(unsigned char) * 32);
	}
}

/* name: mode1swbutton
 * return: time editing flag, false or true
 * param
 *  swbutton: information of sw button pressed 
 *  tm_ptr: structure that contains current time information 
 *  fndData: data that show on the FND
 * desc
 *  if swbutton[0] == 1: flip time editing flag
 *  if time editing flag == 1:
 *    if swbutton[1] == 1: reset time that local
 *    if swbutton[2] == 1: increase 1 hour
 *    if swbutton[3] == 1: increase 1 min
 *                                                */
//int mode1swbutton(int* swbutton, struct tm *tm_ptr, unsigned char* fndData, int isChange);
int mode1swbutton(int* swbutton, commonVari *cv);

/* name: mode2swbutton
 * return: none
 * param
 *  swbutton: information of sw button pressed 
 *  fndData: data that show on the FND
 *  number: one of 10, 8, 4, 2
 * desc
 *  if swbutton[0] == 1: change a number into decimal, octal, quaternion, binary
 *  if swbutton[1] == 1: increase hundred digits
 *  if swbutton[2] == 1: increase ten digits
 *  if swbutton[3] == 1: increase one digits
 *                                            */
void mode2swbutton(int* swbutton, unsigned char* fndData, int *count, int *number);

/* name: modesInit
 * return: none
 * param
 *  mode: current mode 
 *  others: variables that need to initialize
 * desc: initialize the variables according to the mode
 *                                                    */
void modesInit(int mode, struct tm *tm_ptr, unsigned char* fndData, int *ledstat, int *dotMatrix, unsigned char* string);

/* name: mode1Init
 * return: none
 * param
 *  tm_ptr: curent time 
 * desc: initialize the tm_ptr into local time
 *                                            */
void mode1Init(struct tm **tm_ptr);

/* name: mode2Init
 * return: none
 * param
 *  fndData:  
 * desc: initialize the fndData into 0
 *                                            */
void mode2Init(unsigned char *fndData);
void mode3swbutton(int* swbutton, unsigned char *string, int *strIndex, int* dotMatrix, int *cnt);
void mode4swbutton(int* swbutton, unsigned char* table, unsigned char* tableCheck, int* row, int* col, int* isVisible, int *cnt);
void setBit(unsigned char *data, int n, int setOrClear);
void flipBit(unsigned char *data);

int main(){
  int inputProcId, outputProcId;
  key_t key;
  int shmid;
  int *shmaddr = NULL;
	char shmidChar[256];

  /* make a key to create shared memory */
  key = ftok("/data", 1);
  shmid = shmget(key, 1024, IPC_CREAT|0644);
	sprintf(shmidChar, "%d", shmid);
  if( shmid == -1 ){
    perror("shmget");
    exit(1);
  }

  if( (inputProcId = fork()) < 0 ){
    perror("fork error");
    exit(1);
  }
  else if( inputProcId == 0 ){ 
    /* -MARK: input Process */
    /* pass the key to child process */
		char *argv[] = {"./reader", shmidChar, NULL};
		execv(argv[0], argv);
  }
  else{
    /* variables
     *  processingChild, status: when parents process die, check the child process still running
     *  mode: current mode
     *  tm_ptr: current time
     *  fndData: Data that show on FND
     *  string: string that show on LCD
     *  strIndex: index for string. use this variable in mode3
     *  ledstat: data for led value
     *  dotMatrix: data for dotMatrix value
     * */
    int processingChild, status;
		int mode = 1;
		//struct tm *tm_ptr;

		//unsigned char fndData[4]={0};
		//unsigned char string[32] = {"Hello           World          "};
		//int strIndex = -1;
		int i;
		//int ledstat=0, dotMatrix=0;
		//int count=0;
		commonVari cv;

		unsigned char dotTable[10] = {0};
		unsigned char dotTableFlag[10] = {0};
		int dotRow=1, dotCol=0;
		int isVisible = 1;
		
		commomVariInit( &cv, 1 );
		//mode1Init(&tm_ptr);

    /* mapping the shared memory address */
    shmaddr = (int*)shmat(shmid, (int*)NULL, 0);

    if( (outputProcId = fork()) < 0 ){
      perror("fork error");
      exit(1);
    }
    else if( outputProcId == 0 ){ 
      /* -MARK: output Process */
      /* pass the key to child process */
			char *argv[] = {"./writer", shmidChar, NULL};
			execv(argv[0], argv);
    }

		/* -MARK: main Process */
    while(1){
      int pressedKey = shmaddr[0];
			int swbutton[9];
      usleep(400000);
      /* get the information about sw button pressed */
			for(i=0; i<MAX_BUTTON; i++)
				swbutton[i] = shmaddr[1+i];

			printf("mode: %d\n ", cv.curMode);
      if( pressedKey == 158 ){
        /* function button(back) pressed */
				/* Quit */
        break;
      }
      else if( pressedKey == 116 ){
        /* function button(proc) pressed */
      }
      else if( pressedKey == 115 ){
        /* function button(vol+) pressed */
				/* Mode Change + */
				mode = (mode+1)%4+1;
				commomVariInit( &cv, mode );
				//modesInit(mode, tm_ptr, fndData, &ledstat, &dotMatrix, string);
				//strIndex = -1;
				//memset(dotTable, 0, sizeof(unsigned char)*10);
				//memset(dotTableFlag, 0, sizeof(unsigned char)*10);
				//dotRow=1;
			 	//dotCol=0;
				//isVisible = 1;
				//count = 0;
      }
      else if( pressedKey == 114 ){
        /* function button(vol-) pressed */
				/* Mode Change - */
				mode--;
				if( mode < 1 )
					mode =  4;
				commomVariInit( &cv, mode );
				//modesInit(mode, tm_ptr, fndData, &ledstat, &dotMatrix, string);
				//strIndex = -1;
				//memset(dotTable, 0, sizeof(unsigned char)*10);
				//memset(dotTableFlag, 0, sizeof(unsigned char)*10);
				//dotRow=1;
			 	//dotCol=0;
				//isVisible = 1;
				//count = 0;
      }

      /* calculate something according to the mode*/
			if( mode == 1 ){
				//ledstat = mode1swbutton(swbutton, tm_ptr, fndData, ledstat);
				mode1swbutton(swbutton, &cv);
			}
			/*else if( mode == 2 ){
				mode2swbutton(swbutton, fndData, &count, &ledstat);
			}
			else if( mode == 3 ){
				mode3swbutton(swbutton, string, &strIndex, &dotMatrix, &ledstat);
				fndData[0] = ledstat/1000;
				ledstat = ledstat%1000;	
				fndData[1] = (ledstat)/100;
				ledstat = ledstat%100;
				fndData[2] = ledstat/10;
				fndData[3] = (ledstat)%10;
			}
			else if( mode == 4 ){
				mode4swbutton(swbutton, dotTable, dotTableFlag, &dotRow, &dotCol, &isVisible, &ledstat);
				fndData[0] = ledstat/1000;
				ledstat = ledstat%1000;	
				fndData[1] = (ledstat)/100;
				ledstat = ledstat%100;
				fndData[2] = ledstat/10;
				fndData[3] = (ledstat)%10;
			}*/

      /* write the data on shared memory for output */
			shmaddr[10] = cv.curMode;
			shmaddr[11] = cv.ledValue;
			for(i=0; i<4; i++){
				shmaddr[ FND_START +i] = cv.fndData[i];
			}
			for(i=0; i<32; i++){
				shmaddr[ STR_START +i] = cv.string[i];
			}
			shmaddr[48] = cv.dotIndex;
			//for(i=0; i<10; i++){
				//shmaddr[DOTTABLE_START+i] = dotTable[i];
			//}

    }

    //prctl(PR_SET_PDEATHSIG, SIGTERM);
		kill(inputProcId, SIGTERM);
		kill(outputProcId, SIGTERM);

    /* waiting for running child*/
    while( (processingChild = wait(&status)) > 0 );
  }
	shmdt((int*)shmaddr);
	shmctl(shmid, IPC_RMID, (struct shmid_ds*)NULL);
	return 0;
}

//int mode1swbutton(int* swbutton, struct tm *tm_ptr, unsigned char* fndData, int isChange){
int mode1swbutton(int* swbutton, commonVari *cv){
	static unsigned char deltaTime[2] = {0};
	time_t t = time(NULL);
	if( swbutton[0] == KEY_PRESS ){
		//isChange = !isChange;
		cv->ledValue = !(cv->ledValue);
	}
	cv->initTime = localtime(&t);
	if( cv->ledValue == 1 ){
		if( swbutton[1] == KEY_PRESS ){
			//mode1Init(&tm_ptr);
			commomVariInit( cv, 1 );
		}
		if( swbutton[2] == KEY_PRESS ){
			//tm_ptr->tm_hour++;
			deltaTime[0]++;
		}
		if( swbutton[3] == KEY_PRESS ){
			//tm_ptr->tm_min++;
			deltaTime[1]++;
		}
		/*if(tm_ptr->tm_min >= 60 )
			tm_ptr->tm_hour++;
		if( tm_ptr->tm_hour >= 24 )
			tm_ptr->tm_hour = tm_ptr->tm_hour % 24;	*/
		if( deltaTime[1] >= 60 ){
			deltaTime[0]++;
			deltaTime[1] = deltaTime[1]%60;
		}
		if( deltaTime[0] >= 24 ){
			deltaTime[0] = deltaTime[0]%24;
		}
	}

	cv->fndData[0] = cv->initTime->tm_hour/10 + deltaTime[0]/10; 
	cv->fndData[1] = (cv->initTime->tm_hour)%10 + deltaTime[0]%10;
	cv->fndData[2] = cv->initTime->tm_min/10 + deltaTime[1]/10;
 	cv->fndData[3] = (cv->initTime->tm_min)%10 + deltaTime[1]%10;

	return 0;
}

void mode1Init(struct tm **tm_ptr){
	time_t t = time(NULL);
	*tm_ptr = localtime(&t);
}

void mode2swbutton(int* swbutton, unsigned char* fndData, int *count, int *number){
	int temp=0;
	if(*number == 0 ){
		printf("divider is zeor\n");
		return ;
	}

	if( swbutton[0] == KEY_PRESS ){
		//int origin=0;

		//origin += fndData[1] * divider * divider;
		//origin += fndData[2] * divider;
		//origin += fndData[3];

		if( *number == 10 )
			*number = 8;
		else if( *number == 8 )
			*number = 4;
		else if( *number == 4)
			*number = 2;
		else
			*number = 10;

		/*divider = *number;
		origin %= (divider*divider*divider);
		fndData[1] = origin/(divider*divider);
		origin %= (divider*divider);
		fndData[2] = origin/divider;
		origin %= divider;
		fndData[3] = origin;*/
	}
	if( swbutton[1] == KEY_PRESS ){
		//fndData[1] = fndData[1]%(*number);
		(*count) += (*number)*(*number);
	}
	if( swbutton[2] == KEY_PRESS ){
		//fndData[2] = fndData[2]+1;
		//fndData[1] += fndData[2]/(*number);
		//fndData[1] = fndData[1]%(*number);
		//fndData[2] = fndData[2]%(*number);
		(*count) += (*number);
	}
	if( swbutton[3] == KEY_PRESS ){
		(*count) += 1;
		//fndData[3]++;
		//fndData[2] += fndData[3]/(*number);
		//fndData[1] += fndData[2]/(*number);
		//fndData[1] = fndData[1]%(*number);
		//fndData[2] = fndData[2]%(*number);
		//fndData[3] = fndData[3]%(*number);
	}
	temp = (*count);
	fndData[3] = temp%10;
	temp /= 10;
	fndData[2] = temp%10;
	temp /= 10;
	fndData[1] = temp%10;
}

void mode2Init(unsigned char *fndData){
	memset(fndData, 0, sizeof(unsigned char)*4);
}

void modesInit(int mode, struct tm *tm_ptr, unsigned char* fndData, int *ledstat, int *dotMatrix, unsigned char* string){
	*ledstat = 0;
	memset(fndData, 0, sizeof(unsigned char)*4);
	strcpy(string, "Hello           World         \0");
	if( mode == 1)
		mode1Init(&tm_ptr);
	else if( mode == 2 ){
		*ledstat = 10;
	}
	else if( mode == 3 ){
		*dotMatrix = 10;
		memset(string, ' ', sizeof(unsigned char) * 32);
	}
	else if( mode == 4 ){
	}
}

void mode3swbutton(int* swbutton, unsigned char *string, int *strIndex, int* dotMatrix, int *cnt){
	int index1=-1, index2=-1;
	int i;
	static int clickCnt = 0;
	static int clickedBefore = -1;

	char charTable[9][3] = 
	{{'.','Q','Z'}, {'A', 'B', 'C'}, {'D', 'E', 'F'},
		{'G','H','I'}, {'J','K','L'}, {'M','N','O'},
		{'P','R','S'}, {'T','U','V'}, {'W','X','Y'}};

	for(i=0; i<9; i++){
		if( swbutton[i] != 0 ){
			if( index1 == -1 )
				index1 = i;
			else
				index2 = i;
		}
	}

	if( index1 == 1 && index2 == 2 ){
		/* pressed sw(2), sw(3) */
		/* clear string					*/
		memset(string, ' ', sizeof(unsigned char) * 32);
		(*cnt) += 2;
	}
	else if( index1 == 4 && index2 == 5 ){
		/* pressed sw(5), sw(6) */
		/* change input mode, character or number */
		if( *dotMatrix == 1 )
		 	*dotMatrix = 10;
		else
			*dotMatrix = 1;
		(*cnt) += 2;
	}
	else if( index1 == 7 && index2 == 8 ){
		/* pressed sw(8), sw(9) */
		/* make space 					*/
		(*strIndex)++;
		string[(*strIndex)] = ' ';
		clickedBefore = -1;
		clickCnt = 0;
		(*cnt) += 2;
	}
	else{
		if( index1 != -1 ){
			char c;
			
			if( (*dotMatrix) == 10 ){
				if( index1 != clickedBefore ){
					clickedBefore = index1;
					clickCnt = 0;
					(*strIndex)++;
				}
				else {
					clickCnt++;
					clickCnt = clickCnt % 3;
				}
				c	= charTable[index1][clickCnt];
			}
			else{
				c = (index1+1)+'0';
				(*strIndex)++;
			}

			(*cnt)++;
			printf("str : %c %d\n", c, *strIndex);
			string[(*strIndex)] = c;
		}
	}
	(*cnt) = (*cnt)%10000;
	(*strIndex) = (*strIndex)%32;
}

void mode4swbutton(int* swbutton, unsigned char* table, unsigned char* tableCheck, int* row, int* col, int* isVisible, int *cnt){
	static int isblink = 0;
	int i,j;
	isblink = !isblink;

	if( swbutton[1] == KEY_PRESS ){
		(*col)--;
		if( (*col) < 0 )
			(*col) = 0;
		isblink = 1;
		(*cnt)++;
	}
	if( swbutton[3] == KEY_PRESS ){
		(*row)--;
		if( (*row) < 1 )
			(*row) = 1;
		isblink = 1;
		(*cnt)++;
	}
	if( swbutton[5] == KEY_PRESS ){
		(*row)++;
		if( (*row) > 7 )
			(*row) = 7;
		isblink = 1;
		(*cnt)++;
	}
	if( swbutton[7] == KEY_PRESS ){
		(*col)++;
		if( (*col) > 9 )
			(*col) = 9;
		isblink = 1;
		(*cnt)++;
	}

	if( swbutton[0] == KEY_PRESS ){
		(*row) = 1;
		(*col) = 0;
		memset(tableCheck, 0, sizeof(unsigned char)*10);
		(*cnt) = 0;
	}

	if( swbutton[2] == KEY_PRESS ){
		(*isVisible) = !(*isVisible);
		(*cnt)++;
	}

	if( swbutton[4] == KEY_PRESS ){
		setBit( &(tableCheck[(*col)]), 7-(*row), 1 );
		(*cnt)++;
	}
	if( swbutton[6] == KEY_PRESS ){
		memset(tableCheck, 0, sizeof(unsigned char)*10);
		(*cnt)++;
	}
	if( swbutton[8] == KEY_PRESS ){
		int i;
		for(i=0; i<10; i++){
			flipBit( &(tableCheck[i]) );
		}
		(*cnt)++;
	}
	//if( (*cnt) > 9999 )
		//(*cnt) = 0;
	//printf("row %d, col %d\n", *row, *col);

	memcpy( table, tableCheck, sizeof(unsigned char)*10 );
	if( (*isVisible) == 1)
		setBit( &(table[(*col)]), 7-(*row), isblink); 
}

void setBit(unsigned char *data, int n, int setOrClear){
	if( setOrClear )
		(*data) |= (1 << (n));
	else
		(*data) &= ~(1 << (n));
}

void flipBit(unsigned char *data){
	(*data) ^= 0xff;
}
