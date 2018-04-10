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

#define MAX_MODE 5

typedef struct{
	int curMode;
	struct tm *initTime;
	time_t elapsedTime;
	unsigned char fndData[4];
	unsigned char string[33];
	int strIndex;
	int ledValue;
  int	dotIndex;
	int count;
}commonVari;

typedef struct{
		unsigned char table[10];
		unsigned char tableCheck[10];
		int dotRow;
		int dotCol;
		int isVisible;
		int count;
}drawDotVari;

void commomVariInit( commonVari *cv, int mode ){
	time_t t = time(NULL);

	cv->curMode = mode;
	cv->initTime = localtime(&t);
	time(&(cv->elapsedTime));
	memset(cv->fndData, 0, sizeof(cv->fndData));
	strcpy(cv->string, "Hello           World         \0");
	cv->strIndex = -1;
	cv->ledValue = 0;
	cv->dotIndex = 0;
	cv->count = 0;

	if( mode == 2 ){
		cv->ledValue = 10;
	}
	else if( mode == 3 ){
		cv->dotIndex = 10;
		memset(cv->string, 0, sizeof(cv->string));
		memset(cv->string, ' ', sizeof(cv->string)-1);
	}
	else if( mode == 5 ){
		cv->dotIndex = 11;
		memset(cv->string, 0, sizeof(cv->string));
		memset(cv->string, ' ', sizeof(cv->string)-1);
	}
}

void drawDotVariInit( drawDotVari *ddv ){
	memset(ddv->table, 0, sizeof(ddv->table));
	memset(ddv->tableCheck, 0, sizeof(ddv->tableCheck));
	ddv->dotRow=1;
	ddv->dotCol=0;
	ddv->isVisible = 1;
	ddv->count = 0;
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
void mode2swbutton(int* swbutton, commonVari *cv);
void mode3swbutton(int* swbutton, commonVari *cv);
void mode4swbutton(int* swbutton, drawDotVari *ddv, commonVari *cv);
void mode5swbutton(int* swbutton, commonVari *cv);
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
		struct timeval startTime, endTime;
		commonVari cv;
		drawDotVari ddv;
		
		//time(&(elapsedTime));
		gettimeofday(&startTime, NULL);
		commomVariInit( &cv, mode );
		drawDotVariInit( &ddv );

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
			int swbutton[9]={0};
			int i;
			long msec = 0;
			gettimeofday(&endTime, NULL);
			
			msec = (endTime.tv_sec-startTime.tv_sec)*1000+(endTime.tv_usec-startTime.tv_usec)/1000;
			if( msec >= 400 )
				startTime = endTime;
				//continue;
			else
				continue;
			//time_t currentTime;
			//time(&(currentTime));
			//if( currentTime - elapsedTime > 
      //usleep(400000);
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
				mode = (mode)%MAX_MODE+1;
				commomVariInit( &cv, mode );
				drawDotVariInit( &ddv );
      }
      else if( pressedKey == 114 ){
        /* function button(vol-) pressed */
				/* Mode Change - */
				mode--;
				if( mode < 1 )
					mode =  MAX_MODE;
				commomVariInit( &cv, mode );
				drawDotVariInit( &ddv );
      }

      /* calculate something according to the mode*/
			if( mode == 1 ){
				mode1swbutton(swbutton, &cv);
			}
			else if( mode == 2 ){
				mode2swbutton(swbutton, &cv);
			}
			else if( mode == 3 ){
				mode3swbutton(swbutton, &cv);
			}
			else if( mode == 4 ){
				mode4swbutton( swbutton, &ddv, &cv );
			}
			else if( mode == 5 ){
				mode5swbutton( swbutton, &cv );
			}

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
			for(i=0; i<10; i++){
				shmaddr[DOTTABLE_START+i] = ddv.table[i];
			}

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

int mode1swbutton(int* swbutton, commonVari *cv){
	static unsigned char deltaTime[2] = {0};
	time_t t = time(NULL);
	unsigned char hour, min;
	if( swbutton[0] == KEY_PRESS ){
		cv->ledValue = !(cv->ledValue);
	}
	cv->initTime = localtime(&t);
	if( cv->ledValue == 1 ){
		if( swbutton[1] == KEY_PRESS ){
			//commomVariInit( cv, 1 );
			cv->initTime = localtime(&t);
			deltaTime[0] = deltaTime[1] = 0;
		}
		if( swbutton[2] == KEY_PRESS ){
			deltaTime[0]++;
		}
		if( swbutton[3] == KEY_PRESS ){
			deltaTime[1]++;
		}
	}
	hour = cv->initTime->tm_hour + deltaTime[0];
	min = cv->initTime->tm_min + deltaTime[1];
	if( min >= 60 ){
		hour += min/60;
		min = min%60;
	}
	if( hour >= 24 ){
		hour = hour%24;
	}

	cv->fndData[0] = hour/10; 
	cv->fndData[1] = hour%10;
	cv->fndData[2] = min/10;
 	cv->fndData[3] = min%10;

	return 0;
}

void mode2swbutton(int* swbutton, commonVari *cv){
	int temp=0;
	if(cv->ledValue == 0 ){
		printf("divider is zeor\n");
		return ;
	}

	if( swbutton[0] == KEY_PRESS ){
		if( cv->ledValue == 10 )
			cv->ledValue = 8;
		else if( cv->ledValue == 8 )
			cv->ledValue = 4;
		else if( cv->ledValue == 4)
			cv->ledValue = 2;
		else
			cv->ledValue = 10;
	}
	if( swbutton[1] == KEY_PRESS )
		cv->count += (cv->ledValue)*(cv->ledValue);
	
	if( swbutton[2] == KEY_PRESS )
		cv->count += (cv->ledValue);
	
	if( swbutton[3] == KEY_PRESS )
		cv->count += 1;
	
	temp = cv->count;
	cv->fndData[3] = temp%(cv->ledValue);
	temp /= (cv->ledValue);
	cv->fndData[2] = temp%(cv->ledValue);
	temp /= (cv->ledValue);
	cv->fndData[1] = temp%(cv->ledValue);
}

void mode3swbutton(int* swbutton, commonVari *cv){
	int index1=-1, index2=-1;
	int i;
	static int clickCnt = 0;
	static int clickedBefore = -1;
	static char c;
	int temp;

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
		memset(cv->string, ' ', sizeof(unsigned char) * 32);
		cv->count ++;
		cv->strIndex = -1;
	}
	else if( index1 == 4 && index2 == 5 ){
		/* pressed sw(5), sw(6) */
		/* change input mode, character or number */
		if( cv->dotIndex == 1 )
			cv->dotIndex = 10;
		else
			cv->dotIndex = 1;
		cv->count ++;
	}
	else if( index1 == 7 && index2 == 8 ){
		/* pressed sw(8), sw(9) */
		/* make space 					*/
		clickedBefore = -1;
		clickCnt = 0;
		cv->strIndex++;
		c = ' ';
		cv->count++;
	}
	else{
		if( index1 != -1 ){
			if( cv->dotIndex == 10 ){
				if( index1 != clickedBefore ){
					clickedBefore = index1;
					clickCnt = 0;
					cv->strIndex++;
				}
				else
					clickCnt = (clickCnt+1)%3;

				c = charTable[index1][clickCnt];
			}
			else{
				c = (index1+1)+'0';
				cv->strIndex++;
			}

			cv->count++;
		}
	}
	temp = cv->count;
	temp = (temp)%1000;
	cv->fndData[1] = (temp)/100;
	temp = temp%100;
	cv->fndData[2] = temp/10;
	cv->fndData[3] = (temp)%10;

	if( cv->strIndex > 31 ){
		for( i=0; i<31; i++)
			cv->string[i] = cv->string[i+1];
		cv->strIndex = 31;
	}
	if( cv->strIndex != -1 )
		cv->string[ cv->strIndex ] = c;

	//printf("str %s ind %d\n", cv->string, cv->strIndex);
}

void mode4swbutton(int* swbutton, drawDotVari *ddv, commonVari *cv){
	static int isblink = 0;
	int i,j;
	int temp;
	time_t currentTime;
	
	time(&currentTime);

	if( currentTime - cv->elapsedTime >= 1 ){
		cv->elapsedTime = currentTime;
		isblink = !isblink;
	}

	if( swbutton[1] == KEY_PRESS ){
		ddv->dotCol--;
		if( ddv->dotCol < 0 )
			ddv->dotCol = 0;
		isblink = 1;
		cv->elapsedTime = currentTime;
		ddv->count++;
	}
	if( swbutton[3] == KEY_PRESS ){
		ddv->dotRow--;
		if( ddv->dotRow < 1 )
			ddv->dotRow = 1;
		isblink = 1;
		cv->elapsedTime = currentTime;
		ddv->count++;
	}
	if( swbutton[5] == KEY_PRESS ){
		ddv->dotRow++;
		if( ddv->dotRow > 7 )
			ddv->dotRow = 7;
		isblink = 1;
		cv->elapsedTime = currentTime;
		ddv->count++;
	}
	if( swbutton[7] == KEY_PRESS ){
		ddv->dotCol++;
		if( ddv->dotCol > 9 )
			ddv->dotCol = 9;
		isblink = 1;
		cv->elapsedTime = currentTime;
		ddv->count++;
	}

	if( swbutton[0] == KEY_PRESS ){
		ddv->dotRow = 1;
		ddv->dotCol = 0;
		memset(ddv->tableCheck, 0, sizeof(ddv->tableCheck));
		ddv->count = 0;
	}

	if( swbutton[2] == KEY_PRESS ){
		ddv->isVisible = !(ddv->isVisible);
		ddv->count++;
	}

	if( swbutton[4] == KEY_PRESS ){
		setBit( &(ddv->tableCheck[ddv->dotCol]), 7-ddv->dotRow, 1 );
		ddv->count++;
	}
	if( swbutton[6] == KEY_PRESS ){
		memset(ddv->tableCheck, 0, sizeof(ddv->tableCheck));
		ddv->count++;
	}
	if( swbutton[8] == KEY_PRESS ){
		int i;
		for(i=0; i<10; i++){
			flipBit( &(ddv->tableCheck[i]) );
		}
		ddv->count++;
	}
	memcpy( ddv->table, ddv->tableCheck, sizeof(ddv->table) );
	if( ddv->isVisible == 1 )
		setBit( &(ddv->table[ddv->dotCol]), 7-ddv->dotRow, isblink );
	else
		setBit( &(ddv->table[ddv->dotCol]), 7-ddv->dotRow, 0 );

	temp = ddv->count;
	temp = (temp)%1000;
	cv->fndData[1] = (temp)/100;
	temp = temp%100;
	cv->fndData[2] = temp/10;
	cv->fndData[3] = (temp)%10;
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

void mode5swbutton(int* swbutton, commonVari *cv){
	int index1=-1, index2=-1;
  int i;
  static char c;
  int temp;

  for(i=0; i<9; i++){
		if( swbutton[i] != 0 ){
			if( index1 == -1 )
				index1 = i;
			else
			  index2 = i;
		}
	}

	printf("%d %d\n", index1, index2);
  if( index1 == 1 && index2 == 2 ){
		/* pressed sw(2), sw(3) */
	  /* clear string         */
	  memset(cv->string, ' ', sizeof(unsigned char) * 32);
	  cv->count ++;
	  cv->strIndex = -1;
	}
	else if( index1 == 3 && index2 == 4 ){
	  /* pressed sw(5), sw(6) */
	  /* put operator					*/
		if( cv->dotIndex == 11 ){
			c = '+'	;
		}
		else if( cv->dotIndex == 12 ){
			c = '-';
		}
		else if( cv->dotIndex == 13 ){
			c = '/';
		}
		else if( cv->dotIndex == 14 ){
			c = '*';
		}
		cv->strIndex++;
		cv->count++;
	}
  else if( index1 == 4 && index2 == 5 ){
	  /* pressed sw(5), sw(6) */
	  /* put zero 						*/
	  c = 0+'0';
		cv->strIndex++;
		cv->count++;
	}
	else if( index1 == 6 && index2 == 7 ){
	  /* pressed sw(7), sw(8) */
	  /* Change Operator      */
		cv->dotIndex++;
		if( cv->dotIndex > 14 )
			cv->dotIndex = 11;
		cv->count++;
	}
  else if( index1 == 7 && index2 == 8 ){
	  /* pressed sw(8), sw(9) */
	  /* get answer           */
		int i=0;
		for(i=0; i<32; i++){

		}
		cv->count++;
	}
	else{
		if( index1 != -1 ){
			c = (index1+1)+'0';
			cv->strIndex++;
			cv->count++;
		}
	}

  temp = cv->count;
  temp = (temp)%1000;
  cv->fndData[1] = (temp)/100;
  temp = temp%100;
  cv->fndData[2] = temp/10;
  cv->fndData[3] = (temp)%10;

  //if( cv->strIndex > 31 ){
	    //for( i=0; i<31; i++)
	      //cv->string[i] = cv->string[i+1];
	    //cv->strIndex = 31;
	  

	if( cv->strIndex != -1 )
		cv->string[ cv->strIndex ] = c;
}

int vaildCheck(unsigned char* string){
	int isOperator=0;
	char operand[32]={0};
	char operators[16]={0};
	char operands[16]={0};
	int operatorCount=0;
	int operandCount=0;
	int cnt = 0;
	int i, j;
	int result = 0;

	if( (string[0] < '0' || string[0] > '9') )
		return 0;

	for(i=0; i<32; i++){
		if( string[i] == ' ' || string[i] == '\0' )
			break;

		if( string[i] >= '0' && string[i] <= '9' ){
			operand[cnt++] = string[i];
			isOperator = 0;
		}
		else{
			if( isOperator == 1 )
				return 0;

			if( string[i] != '+' || string[i] != '-' ||
					string[i] != '/' || string[i] != '*' )
				return 0;
			
			operators[operatorCount++] = string[i];
			sscanf(operand, "%d", &operands[operandCount++]);
			isOperator = 1;
			cnt = 0;

			memset(operand, 0, sizeof(operand));
		}
	}


	result = operands[j++];
	for(i=0; i<operatorCount; i++){
		if( operators[i] == '+' ){
			result += operands[j+1];
		}
		else if( operators[i] == '-' ){
			result += operands[j] + operands[j+1];
		}
		else if( operators[i] == '/' ){
			result += operands[j] + operands[j+1];
		}
		else if( operators[i] == '*' ){
			result += operands[j] + operands[j+1];
		}
	}
}
