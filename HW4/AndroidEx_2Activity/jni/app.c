#include <jni.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "android/log.h"

#define LOG_TAG "MyTag"
#define LOGV(...)   __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

#define FPGA_DEVICE "/dev/dev_driver"

jint JNICALL Java_com_example_androidex_MainActivity2_00024BackThread_isDone(JNIEnv *env, jobject this, jint fd)
{
	return read(fd, 0, 0);
}

jint JNICALL Java_com_example_androidex_FpgaControl_open(JNIEnv *env, jobject this)
{
	int dev = open(FPGA_DEVICE, O_RDWR);
	if( dev < 0 ){
		perror("device file error");
		return -1;
	}

	return dev;
}

void JNICALL Java_com_example_androidex_FpgaControl_close(JNIEnv *env, jobject this, jint fd)
{
	char _value[4]={0};
	write(fd, _value, 4);
	close(fd);
}

void JNICALL Java_com_example_androidex_FpgaControl_write(JNIEnv *env, jobject this, jint fd, jint value)
{
	char _value[4] = {0};
	_value[3] = value%10;
	value = value /10;
	_value[2] = value%10;


	write(fd, _value, 4);
}
