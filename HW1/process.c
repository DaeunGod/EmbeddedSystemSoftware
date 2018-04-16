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
#include <limits.h>

#define MAX_BUTTON 9
#define KEY_PRESS 1
#define KEY_RELEASE 0

#define FND_START 12
#define STR_START 16
#define DOTTABLE_START 49

#define MAX_MODE 5

/* 
 * struct commonVari
 * dsec: Variables that used every modes
 *
 * curMode: what is current mode
 * initTime: initialize the time into system time
 * elapsedTime: for calculate the elapsed time from initTime
 * fndData: data that show on fnd
 * string: data that show on lcd
 * strIndex: string index
 * ledValue: led data, [0-255]
 * dotIndex: data that show on dot matrix, [0-9]:numbers, [10]:'A', [11~14]:'+', '-', '/', '*'
 * count: how many times the switch button pressed
 */
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

/*
 * struct drawDotVari
 * desc: Variables that need to mode4
 *
 * table: data that show on dot matrix
 * tableCheck: actual data that user draws
 * dotRow: current cursor row
 * dotCol: current cursor col
 * isVisible: 1 cursor is visible
							0 cursor is not visible
 * count: how many times the switch button pressed
 */
typedef struct{
		unsigned char table[10];
		unsigned char tableCheck[10];
		int dotRow;
		int dotCol;
		int isVisible;
		int count;
}drawDotVari;

/*
 * struct extraVari
 * desc: Variables that need to mode5
 * 
 * operand: data that current operand
 * tempStr: data that will write on second line lcd
 * operator: current operator
 * result: save a calculated data
 * isOperator: 1 when user put the operators '+', '-', '/', '*'
								0 initial value or when user put the operator '='
 */
typedef struct{
	unsigned char operand[16];
	unsigned char tempStr[16];
	unsigned char operator;
	int result;
	int isOperator;
}extraVari;

/* name: commonVariInit
 * desc: Initialize commonVari variable according to current mode
 *
 * return: none
 * param
 *  cv: variable that is going to be initialize
 *  mode: current mode
 */
void commomVariInit( commonVari *cv, int mode );

/* name: drawDotVariInit
 * desc: Initialize drawDotVari variable
 *
 * return: none
 * param
 *  ddv: variable that is going to be initialize
 */
void drawDotVariInit( drawDotVari *ddv );

/* name: extraVariInit
 * desc: Initialize extraVari variable
 *
 * return: none
 * param
 *  ev: variable that is going to be initialize
 */
void extraVariInit( extraVari *ev );

/* name: mode1swbutton
 * return: none
 * param
 *  swbutton: information of sw button pressed 
 *  cv: common variables
 * desc
 *  if swbutton[0] == 1: flip time editing flag
 *  if time editing flag == 1:
 *    if swbutton[1] == 1: reset the time into local time
 *    if swbutton[2] == 1: increase 1 hour
 *    if swbutton[3] == 1: increase 1 min
 */
void mode1swbutton(int* swbutton, commonVari *cv);

/* name: mode2swbutton
 * return: none
 * param
 *  swbutton: information of sw button pressed 
 *  cv: common variables
 * desc
 *  if swbutton[0] == 1: change a number into decimal, octal, quaternion, binary
 *  if swbutton[1] == 1: increase hundred digits
 *  if swbutton[2] == 1: increase ten digits
 *  if swbutton[3] == 1: increase one digits
 */
void mode2swbutton(int* swbutton, commonVari *cv);

/* name: mode3swbutton
 * return: none
 * param
 *  swbutton: information of sw button pressed 
 *  cv: common variables
 * desc
 *  if swbutton[0~8] == 1: put the character or number in string
 *  if swbutton[1] == 1 && swbutton[2] == 1: clear the string
 *  if swbutton[4] == 1 && swbutton[5] == 1: Change input mode, character or number
 *  if swbutton[7] == 1 && swbutton[8] == 1: put a space in string
 */
void mode3swbutton(int* swbutton, commonVari *cv);

/* name: mode4swbutton
 * return: none
 * param
 *  swbutton: information of sw button pressed 
 *  ddv: draw Dot matrix variables
 *  cv: common variables
 * desc
 *  if swbutton[1, 3, 5, 7] == 1: move the cursor up, left, right, down
 *  if swbutton[4] == 1: Mark the position of cursor(set)
 *  if swbutton[0] == 1: clear and reset
 *  if swbutton[2] == 1: set the visibility of cursor [0, 1]
 *  if swbutton[6] == 1: clear the data but cursor position
 *  if swbutton[8] == 1: reverse every data on the dot matrix
 */
void mode4swbutton(int* swbutton, drawDotVari *ddv, commonVari *cv);

/* name: mode5swbutton
 * return: none
 * param
 *  swbutton: information of sw button pressed 
 *  cv: common variables
 *  ev: extra variables
 * desc
 *  if swbutton[0~8] == 1: put the numbers in operand
 *  if swbutton[0] == 1 && swbutton[1] == 1: clear
 *  if swbutton[3] == 1 && swbutton[4] == 1: choose the current operator
 *  if swbutton[4] == 1 && swbutton[5] == 1: put the zero in operand
 *  if swbutton[6] == 1 && swbutton[7] == 1: change the operator
 *  if swbutton[7] == 1 && swbutton[8] == 1: get the answer
 */
void mode5swbutton(int* swbutton, commonVari *cv, extraVari *ev);

/* name: setBit
 * return: none
 * param
 *  data: data that set or clear a bit, 
 *  n: n-th position
 *  setOrClear: 1 set
								0 clear
 * desc
 *  set or clear a n-th bit
 */
void setBit(unsigned char *data, int n, int setOrClear);

/* name: flipBit
 * return: none
 * param
 *  data: origin data
 * desc
 *  every bits in data will be flipped
 */
void flipBit(unsigned char *data);

/* name: calculate
 * return: none
 * param
 *  operator: current operator, '+', '-', '/', '*'
 *  result: operand of left side and result of evaluation
 *  operand: operand of right side
 * desc
 *  evaluate the result according to operator
 */
int calculate(unsigned char operator, int* result, int operand);

/* name: setOperator
 * return: none
 * param
 *  dotIndex: dot Matrix index, [11~14]
 *  operator: '+', '-', '/', '*'
 * desc
 *  set the operator according to dotIndex
 */
void setOperator(int dotIndex, unsigned char *operator);

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
		 *  startTime, endTiem: to calculate input time
		 *  cv: common variables for every modes
		 *  ddv: draw dot matrix variables for mode4
		 *  ev: extra variables for mode5
     */
    int processingChild, status;
		int mode = 1;
		struct timeval startTime, endTime;
		commonVari cv;
		drawDotVari ddv;
		extraVari ev;
		
		gettimeofday(&startTime, NULL);
		commomVariInit( &cv, mode );
		drawDotVariInit( &ddv );
		extraVariInit( &ev );

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
			
			/* this is for delay */
			msec = (endTime.tv_sec-startTime.tv_sec)*1000+(endTime.tv_usec-startTime.tv_usec)/1000;
			if( msec >= 200 )
				startTime = endTime;
			else
				continue;

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
				/* Mode Change +, Initialize every variables */
				mode = (mode)%MAX_MODE+1;
				commomVariInit( &cv, mode );
				drawDotVariInit( &ddv );
				extraVariInit( &ev );
      }
      else if( pressedKey == 114 ){
        /* function button(vol-) pressed */
				/* Mode Change -, Initialize every variables */
				mode--;
				if( mode < 1 )
					mode =  MAX_MODE;
				commomVariInit( &cv, mode );
				drawDotVariInit( &ddv );
				extraVariInit( &ev );
      }

      /* calculate something according to the mode*/
			if( mode == 1 )
				mode1swbutton(swbutton, &cv);
			else if( mode == 2 )
				mode2swbutton(swbutton, &cv);
			else if( mode == 3 )
				mode3swbutton(swbutton, &cv);
			else if( mode == 4 )
				mode4swbutton( swbutton, &ddv, &cv );
			else if( mode == 5 )
				mode5swbutton( swbutton, &cv, &ev );
			

      /* write the data on shared memory for output */
			shmaddr[10] = cv.curMode;
			shmaddr[11] = cv.ledValue;
			for(i=0; i<4; i++)
				shmaddr[ FND_START +i] = cv.fndData[i];
			for(i=0; i<32; i++)
				shmaddr[ STR_START +i] = cv.string[i];
			shmaddr[48] = cv.dotIndex;
			for(i=0; i<10; i++)
				shmaddr[DOTTABLE_START+i] = ddv.table[i];
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

void commomVariInit( commonVari *cv, int mode ){
	time_t t = time(NULL);

	cv->curMode = mode;
	cv->initTime = localtime(&t);
	time(&(cv->elapsedTime));
	memset(cv->fndData, 0, sizeof(cv->fndData));
	strcpy(cv->string, "Hello           World           \0");
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

void extraVariInit( extraVari *ev ){
	memset(ev->operand, 0, sizeof(ev->operand));
	memset(ev->tempStr, 0, sizeof(ev->tempStr));
	ev->operator = '+';
	ev->result = 0;
	ev->isOperator = 0;
}

void mode1swbutton(int* swbutton, commonVari *cv){
	static unsigned char deltaTime[2] = {0};
	time_t t = time(NULL);
	unsigned char hour, min;
	if( swbutton[0] == KEY_PRESS ){
		cv->ledValue = !(cv->ledValue);
	}
	cv->initTime = localtime(&t);

	if( cv->ledValue == 1 ){
		/* when user enter the time change mode */
		if( swbutton[1] == KEY_PRESS ){
			/* reset into local time */
			cv->initTime = localtime(&t);
			deltaTime[0] = deltaTime[1] = 0;
		}
		if( swbutton[2] == KEY_PRESS ){
			/* increase 1 hour */
			deltaTime[0]++;
		}
		if( swbutton[3] == KEY_PRESS ){
			/* increase 1 min */
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
}

void mode2swbutton(int* swbutton, commonVari *cv){
	int temp=0;
	if(cv->ledValue == 0 ){
		printf("divider is zero\n");
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
		clickCnt = 0;
		clickedBefore = -1;
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
	temp = (temp)%10000;
  cv->fndData[0] = (temp)/1000;
  temp = temp%1000;
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
		//ddv->count = 0;
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
	//else
		//setBit( &(ddv->table[ddv->dotCol]), 7-ddv->dotRow, 0 );

	temp = ddv->count;
	temp = (temp)%10000;
  cv->fndData[0] = (temp)/1000;
  temp = temp%1000;
	cv->fndData[1] = (temp)/100;
	temp = temp%100;
	cv->fndData[2] = temp/10;
	cv->fndData[3] = (temp)%10;
}

void setBit(unsigned char *data, int n, int setOrClear){
	if( setOrClear )
		(*data) ^= (1 << (n));
	else
		(*data) &= ~(1 << (n));
}

void flipBit(unsigned char *data){
	(*data) ^= 0xff;
}

void mode5swbutton(int* swbutton, commonVari *cv, extraVari *ev){
	int index1=-1, index2=-1;
  int i;
  static char c = ' ';
  int temp;

  for(i=0; i<9; i++){
		if( swbutton[i] != 0 ){
			if( index1 == -1 )
				index1 = i;
			else
			  index2 = i;
		}
	}

	memset(cv->string, ' ', sizeof(cv->string));
  if( index1 == 1 && index2 == 2 ){
		/* pressed sw(2), sw(3) */
	  /* clear string         */
	  memset(cv->string, ' ', sizeof(cv->string));
		extraVariInit( ev );
		cv->count ++;
	  cv->strIndex = -1;
		//cv->dotIndex = 11;
		setOperator(cv->dotIndex, &(ev->operator));
		c = ' ';
	}
	else if( index1 == 3 && index2 == 4 ){
	  /* pressed sw(5), sw(6) */
	  /* put operator					*/
		if( ev->isOperator == 0 ){
			ev->isOperator = 1;
			sscanf(ev->operand, "%d", &(ev->result));
		}
		else{
			int operand2=0, res = 0;
			sscanf(ev->operand, "%d", &operand2);
			res = calculate(ev->operator, &ev->result, operand2);
			if( res == -1 ){
				sprintf(ev->tempStr, "Dvide by zero");
				c = -1;
			}
			else if( res == -2 ){
				sprintf(ev->tempStr, "Overflow");
				c = -1;
			}
		}

	  memset(ev->operand, 0, sizeof(ev->operand));
	  cv->strIndex = -1;
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

		setOperator(cv->dotIndex, &(ev->operator));
		cv->count++;
	}
  else if( index1 == 7 && index2 == 8 ){
	  /* pressed sw(8), sw(9) */
	  /* get answer           */
		int operand2=0, res=0;
		sscanf(ev->operand, "%d", &operand2);
		res = calculate(ev->operator, &ev->result, operand2);
		if( res == -1 ){
			sprintf(ev->tempStr, "Dvide by zero");
			c = -1;
		}
		else if( res == -2 ){
			sprintf(ev->tempStr, "Overflow");
			c = -1;
		}

	  memset(ev->operand, 0, sizeof(ev->operand));
	  cv->strIndex = -1;
		ev->operator = ' ';
		ev->isOperator = 0;

		cv->count++;
	}
	else{
		if( index1 != -1 ){
			c = (index1+1)+'0';
			if( ev->operator == ' ' ){
				/* after you get answer make result zero and show operator again */
				setOperator(cv->dotIndex, &(ev->operator));
				ev->result = 0;
			}
			cv->strIndex++;
			cv->count++;
		}
	}

  temp = cv->count;
	temp = (temp)%10000;
  cv->fndData[0] = (temp)/1000;
  temp = temp%1000;
  cv->fndData[1] = (temp)/100;
  temp = temp%100;
  cv->fndData[2] = temp/10;
  cv->fndData[3] = (temp)%10;

	if( c != 255 ){
		ev->operand[ cv->strIndex ] = c;
		sprintf(ev->tempStr, "%d%c", ev->result, ev->isOperator ? (ev->operator) : ' ');
	}
	memcpy( cv->string, ev->operand, strlen(ev->operand) );
	memcpy( cv->string+16, ev->tempStr, strlen(ev->tempStr) );
}

int calculate(unsigned char operator, int* result, int operand){
	double temp = *(result);
	if( operator == '+' )
		if( temp + operand > INT_MAX )
			return -2;
		else
			*(result) += operand;
	else if( operator == '-' )
		if( temp - operand < INT_MIN )
			return -2;
		else
			*(result) -= operand;
	else if( operator == '/' ){
		if( operand != 0 )
			*(result) /= operand;
		else{
			return -1;//Invalid Expression
		}
	}
	else if( operator == '*' ){
		if( temp * operand > INT_MAX )
			return -2;
		else
			*(result) *= operand;
	}

	return 0;
}

void setOperator(int dotIndex, unsigned char *operator){
		if( dotIndex == 11 )
			*operator = '+'	;
		else if( dotIndex == 12 )
			*operator = '-';
		else if( dotIndex == 13 )
			*operator = '/';
		else if( dotIndex == 14 )
			*operator = '*';
}
