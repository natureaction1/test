/*
****************************************************************************
* FILE        :
*                                 jrd_thread.c
* DESCRIPTION :
*                          jrd common module's thread
****************************************************************************
*    Copyright (c) 2015 by JRDCOM Incorporated.  All Rights Reserved.
****************************************************************************
*/
/*
**********************************EditHistory*******************************
* This section contains comments describing changes made to the module.
* Notice that changes are listed in reverse chronological order.
*
*    when                 who                      what, where, why
* ---------------------------    ---------------------------------------
*  2015/11/08   Randy.Xi               Create the initial version
*
****************************************************************************
*/

#include <stdio.h>
#include <pthread.h>

#include "jrd_oem_common.h"
#include "jrd_thread.h"

#define JRD_THREAD_DEFAULT_STACK_SIZE    (128 * 1024)

int jrd_thread_create(jrd_thread_data_t *thread_data)
{
	pthread_attr_t attr;
    size_t stack_size = 0;
    int rc = JRD_NO_ERR;

	if (!thread_data) {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, param pointer is NULL.\n");
        return JRD_FAIL;
    }

    rc = pthread_attr_init(&attr);
    if (rc) {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, init thread attribute failed(rc = %d).\n", rc);
        return JRD_FAIL;
    }

	if (thread_data->stacksize == 0) {
		stack_size = JRD_OEM_THREAD_STACK_SIZE;
    } else {
		stack_size = thread_data->stacksize;
    }

	rc = pthread_attr_setstacksize(&attr, stack_size);
    if (rc) {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, set stack size failed(rc = %d).\n", rc);
        return rc;
    }

    rc = pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    if (rc) {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, set setdetachstate failed(rc = %d).\n", rc);
    }

    rc = pthread_create(&(thread_data->thread_id), &attr, thread_data->user_func, thread_data->user_data);
    if (rc) {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, create thread failed(rc = %d).\n", rc);
        return rc;
    }
    return JRD_NO_ERR;
}
