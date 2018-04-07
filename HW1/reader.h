#ifndef __READER_H_
#define __READER_H_


#define MAX_BUTTON 9
#define KEY_PRESS 1
#define KEY_RELEASE 0

void user_signal1(int sig);

/* name: readFromDevie
 * return: none
 * param
 *  shmaddr: shared memory address 
 * desc: get data from device and write on the shared memory 
 *       devices are SW buttons and function key(bakc, proc, vol+, vol-)*/
void readFromDevice(int* shmaddr);

#endif
