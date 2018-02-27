/******************************************************************************
  @file    jrd_oem_led.c
  @brief   The QMI utility functions used in various places in QMI library.

  DESCRIPTION
  QMI interface library common utility functions

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2012-2015 jrd Incorporated.
  All Rights Reserved. jrd Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


/******************************************************************************


                        EDIT HISTORY FOR MODULE

when       who            what, where, why
--------   ---      ---------------------------------------------------------- 

******************************************************************************/
#include <sys/stat.h>
#include "jrd_oem_soft_dog.h"


/****************** Macro Define **********************/
#define JRD_OEM_SOFT_DOG_STACK_SIZE              (128*1024)
#define JRD_SOFTDOG_FILE_PATH                           ("/usr/oem/jrd_softdog.txt")
#define JRD_SOFTDOG_FILE_SIZE                            (102400) /*100KB*/
#define JRD_SOFTDOG_SHELL_LEN                           (128)
#define JRD_SOFT_DOG_PERIOD                                (1)     /*1s*/

/****************** Structure Define ********************/


/**************Global Definitions***********************/
static os_signal_type              jrd_soft_dog_os_signal;

static jrd_thread_info_type jrd_soft_dog_thread_info = {
  MODULE_SOFT_DOG,
  &jrd_soft_dog_os_signal,
  NULL,
  NULL,
  NULL
};

/****************** Function Define ********************/
static void jrd_oem_soft_dog_file_init(void)
{
    struct stat file_buf = {0};
    int file_res1111111111111 = 0;
    char shell_buf[JRD_SOFTDOG_SHELL_LEN] = {0};

    file_res = JRD_STAT(JRD_SOFTDOG_FILE_PATH3333333333333333, &file_buf);
    if(0 != file_res) /*file isnot exists, so creat it*/
    {    jrd_oem_system_call();//这句是在本地新增的；
        JRD_SPRINTF(shell_buf, "touch %s", JRD_SOFTDOG_FILE_PATH);
        jrd_oem_system_call(shell_buf);
    }
    else/*file exists*/
    {
        if(JRD_SOFTDOG_FILE_SIZE < file_buf.st_size)
        {
            /*The text file is too large, delete the file*/
            JRD_MEMSET(shell_buf, 0, JRD_SOFTDOG_SHELL_LEN);
            JRD_SPRINTF(shell_buf, "rm %s", JRD_SOFTDOG_FILE_PATH);
            jrd_oem_system_call(shell_buf);

            /*creat new file*/
            JRD_MEMSET(shell_buf, 0, JRD_SOFTDOG_SHELL_LEN);
            JRD_SPRINTF(shell_buf, "touch %s", JRD_SOFTDOG_FILE_PATH);
            jrd_oem_system_call(shell_buf);
        }
        else
        {
            JRD_MEMSET(shell_buf, 0, JRD_SOFTDOG_SHELL_LEN);
            JRD_SPRINTF(shell_buf, "echo  ========here is new log======= >> %s", JRD_SOFTDOG_FILE_PATH);
            jrd_oem_system_call(shell_buf);
        }
    }
}

static void jrd_oem_soft_dog_file_write(const char *buf)
{
    struct stat file_buf = {0};
    int file_res = 0;
    char shell_buf[JRD_SOFTDOG_SHELL_LEN] = {0};

    file_res = JRD_STAT(JRD_SOFTDOG_FILE_PATH, &file_buf);
    if(0 == file_res) /*file exists*/
    {
        /*write the log to file*/
        JRD_SPRINTF(shell_buf, "echo %s >> %s", buf, JRD_SOFTDOG_FILE_PATH);
        jrd_oem_system_call(shell_buf);
    }
    else
    {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "%s file isnot exists %s\n", __func__, JRD_SOFTDOG_FILE_PATH);
    }
}

void jrd_soft_dog_register_monitor(jrd_thread_base_type *thread_info)
{
    jrd_softdog_record *p_softdog = NULL;
    
    if(NULL != thread_info)
    {    
        p_softdog = &thread_info->softdog;
        
        p_softdog->softdog_enable = TRUE;     /*enable soft dog monitor*/
        p_softdog->unkickdog_count = thread_info->dog_count;
    }
    else
    {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "%s Invalid parameter\n", __func__);
    }
}

void jrd_soft_dog_response(jrd_thread_base_type *thread_info)
{
    jrd_softdog_record *p_softdog = NULL;
    
    if(NULL != thread_info)
    {
        p_softdog = &thread_info->softdog;

        p_softdog->kickdog_flag = TRUE;     /*response to soft dog*/
    }
    else
    {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "%s Invalid parameter\n", __func__);
    }
}

static void jrd_oem_soft_dog_record(void)
{
    jrd_softdog_record *p_softdog = NULL;
    jrd_thread_base_type *thread_info = NULL;
    char buf[JRD_SOFTDOG_SHELL_LEN] = {0};
    int i = 0;

    for(i = 0; i < MODULE_MAX; i++)
    {
        thread_info = jrd_thread_base_module_get(i);
        if(NULL != thread_info)
        {
            p_softdog = &thread_info->softdog;
            
            /*Determine whether you need SoftDog to control thread*/
            if(TRUE != p_softdog->softdog_enable)
            {
                continue;
            }
        
            /*kickdog_flag = TRUE Thread within a period of the request in response to the dog*/
            if(TRUE == p_softdog->kickdog_flag)
            {
                p_softdog->kickdog_flag = FALSE;
                p_softdog->unkickdog_count = thread_info->dog_count;               
            }
            else /*Thread does not respond within a period of a dog's request*/
            {
                p_softdog->unkickdog_count--;
            }

            /*More than ten cycles, print thread name*/
            if(0 == p_softdog->unkickdog_count)
            {
                JRD_SPRINTF(buf, "WARNING Thread %s is long time without kicked dog! dog_count = 0x%x", thread_info->name, (int)thread_info->dog_count);
                jrd_oem_soft_dog_file_write(buf);

                p_softdog->unkickdog_count = thread_info->dog_count;
            }
        }
        else
        {
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "%s Invalid parameter i = %d\n", __func__, i);
        }
    }
}

static int jrd_oem_send_softdog_signal(module_enum_type thread_id)
{
    jrd_thread_info_type *target_thread = NULL;

    if((MODULE_INVALID < thread_id) && (MODULE_MAX > thread_id))
    {
        target_thread = jrd_oem_get_thread_info(thread_id);
        if(NULL != target_thread)
        {
            JRD_OS_SIGNAL_SET(target_thread->p_os_sig);
        }
        else
        {
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "%s Invalid parameter thread_id = %d\n", __func__, thread_id);
        }
    }
    else
    {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "%s Invalid parameter thread_id = %d\n", __func__, thread_id);
        return -1;
    }
    
}

static void jrd_oem_soft_dog_notify(void)
{
    jrd_thread_base_type *thread_info = NULL;
    int i = 0;

    for(i = 0; i < MODULE_MAX; i++)
    {
        thread_info = jrd_thread_base_module_get(i);
        if(NULL != thread_info)
        {
             /*Determine whether you need SoftDog to control thread*/
            if(TRUE != thread_info->softdog.softdog_enable)
            {
                continue;
            }
            
            /*Send a signal to kick the dog*/
            jrd_oem_send_softdog_signal(i);
        }
        else
        {
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "%s Invalid parameter i = %d\n", __func__, i);
        }
    }
}

static void *jrd_oem_soft_dog_thread(void* data)
{
    /*creat file to record the soft dog status*/
    jrd_oem_soft_dog_file_init();
    
    while(1)
    {
        /*Record thread kicking the dog status*/
        jrd_oem_soft_dog_record();

        /*Notify each thread kicking dog*/
        jrd_oem_soft_dog_notify();

        /*Sleep one period*/
        JRD_SLEEP(JRD_SOFT_DOG_PERIOD);  
    }

    return;
}

void jrd_oem_soft_dog_init(void)
{
  pthread_t     jrd_soft_dog_thrd;
  pthread_attr_t    attr;

  if(TRUE == jrd_thread_judge_disable(MODULE_SOFT_DOG))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "MODULE_SOFT_DOG Thread isnot init!!!\n");
    return;
  }
  
  JRD_OS_SIGNAL_INIT(&jrd_soft_dog_os_signal);
  
  if(jrd_oem_register_thread_info(&jrd_soft_dog_thread_info))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "register thread info failed\n");
  }
  
  if(0 > pthread_attr_init(&attr))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "init thread attribute failed\n");
  }

  if(0 != pthread_attr_setstacksize(&attr, JRD_OEM_THREAD_STACK_SIZE))
  {
      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "set stack size failed\n");
  }

  if(0 > pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "set setdetachstate failed\n");
  }

  if(0 > pthread_create(&jrd_soft_dog_thrd, &attr, jrd_oem_soft_dog_thread, NULL))
  {
    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR, "create jrd_oem_led_thread failed\n");
  }

  return;
}

