#ifndef __WRITER_H_
#define __WRITER_H_


/* name: readFromSM
 * return: none
 * param
 *  shmaddr: shared memory address
 *  mode: current mode
 *  ledstat: value of led[0-255]
 *  dotMat: value of dotMat[0-10]
 * desc: get the data from shared memory and update FND data, String, other status
 *                                                                                */
void readFromSM(int* shmaddr, int *mode, int *ledstat, int *dotMat);

/* name: FNDmode1
 * return: none
 * param: none
 * desc: control FND using device file, data is fndData
 *                                                      */
void FNDmode1();

/* name: LCD
 * return: none
 * param: none
 * desc: control LCD using device file, data is string
 *                                                    */
void LCD();

/* name: LED
 * return: none
 * param
 *  n: value of LED[0-255]
 * desc: on or off the led to show a number in 8bit binary 
 *                                                        */
void LED(int n);

/* name: Dot
 * return: none
 * param
 *  dotMatrix: value of dotMatrix index[0-10] 
 * desc: show the symbol that written in fpga_dot_font.h
 *       available index is 0 to 10
 *                                                      */
void Dot(int dotMatrix);

#endif
