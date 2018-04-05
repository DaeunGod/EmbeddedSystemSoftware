#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "reader.h"

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
  else if( inputProcId == 0 ){
    shmaddr = (int*)shmat(shmid, (int*)NULL, 0);
    readFromDevice(shmaddr);
  }
  else{
    int processingChild, status;

    shmaddr = (int*)shmat(shmid, (int*)NULL, 0);

    while(1){
      usleep(400000);
      int pressedKey = shmaddr[0];
      if( pressedKey == 123 ){
        break;
      }
      else if( pressedKey == 456 ){
      }
      else if( pressedKey == 789 ){
      }
      else if( pressedKey == 101112 ){
      }
    }

    if( (outputProcId = fork()) < 0 ){
      perror("fork error");
      exit(1);
    }
    else if( outputProcId == 0 ){
    }

    /* waiting for running child*/
    while( (processingChild = wait(&status)) > 0 );
  } 
  return 0;
}
