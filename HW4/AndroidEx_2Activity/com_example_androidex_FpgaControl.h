/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_example_androidex_FpgaControl */

#ifndef _Included_com_example_androidex_FpgaControl
#define _Included_com_example_androidex_FpgaControl
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_example_androidex_FpgaControl
 * Method:    open
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_example_androidex_FpgaControl_open
  (JNIEnv *, jobject);

/*
 * Class:     com_example_androidex_FpgaControl
 * Method:    close
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_example_androidex_FpgaControl_close
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_example_androidex_FpgaControl
 * Method:    write
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_example_androidex_FpgaControl_write
  (JNIEnv *, jobject, jint, jint);

#ifdef __cplusplus
}
#endif
#endif
