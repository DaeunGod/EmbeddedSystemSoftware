#ifndef __WRITER_H_
#define __WRITER_H_


void readFromSM(int* shmaddr, int *mode, int *ledstat, int *dotMat);
void FNDmode1();
void LCD();
void LED(int n);
void Dot(int dotMatrix);

#endif
