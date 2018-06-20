#include <jni.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "android/log.h"

#define LOG_TAG "MyTag"
#define LOGV(...)   __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

#define FPGA_DEVICE "/dev/dev_driver"

jint JNICALL Java_com_example_androidex_FpgaControl_open(JNIEnv *env, jobject this)
{
	int dev = open(FPGA_DEVICE, O_RDWR);
	//if( dev < 0 ){
	//	perror("device file error");
	//	return -1;
	//}

	return dev;
}

void JNICALL Java_com_example_androidex_FpgaControl_close(JNIEnv *env, jobject this, jint fd)
{
	close(fd);
}

void JNICALL Java_com_example_androidex_FpgaControl_write(JNIEnv *env, jobject this, jint fd, jint value)
{
	char _value[4] = {0};
	sprintf(_value, "%d", value);
	write(fd, _value, 4);
}
