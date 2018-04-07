#ifndef __READER_H_
#define __READER_H_


#define MAX_BUTTON 9
#define KEY_PRESS 1
#define KEY_RELEASE 0
void user_signal1(int sig);
void readFromDevice(int* shmaddr);

#endif
