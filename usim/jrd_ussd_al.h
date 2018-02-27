/*
****************************************************************************
* FILE        :
*                                 jrd_ussd_al.h
* DESCRIPTION :
*       JRD USSD ABSTRACTION LAYER -- usim module's ussd abstraction layer.
****************************************************************************
*    Copyright (c) 2015 by JRDCOM Incorporated.  All Rights Reserved.
****************************************************************************
*/


#ifndef _JRD_USSD_AL_H_
#define _JRD_USSD_AL_H_

#include "utils/list.h"

#define JRD_CM_USSD_DCS_7_BIT          0x00
#define JRD_CM_USSD_ASCII              0x02
#define JRD_CM_USSD_DCS_8_BIT          0x04
#define JRD_CM_USSD_DCS_UCS2           0x08
#define JRD_CM_USSD_DCS_UNSPECIFIED    0x0F

#define JRD_USSD_CONTENT_MAX_LEN    182    /* Max num of char is USS data*/


typedef enum {
    E_JRD_USIM_USSD_ACTION_TYPE_INVALID = -1,
    E_JRD_USIM_USSD_ACTION_TYPE_NOTHING,
    E_JRD_USIM_USSD_ACTION_TYPE_REQUEST,
    E_JRD_USIM_USSD_ACTION_TYPE_ANSWER,
    E_JRD_USIM_USSD_ACTION_TYPE_CANCEL,
    E_JRD_USIM_USSD_ACTION_TYPE_MAX,
} e_jrd_usim_ussd_action_type_t;

typedef enum {
    E_JRD_USIM_USSD_STATE_INVALID = -1,
    E_JRD_USIM_USSD_STATE_SEND_NONE,
    E_JRD_USIM_USSD_STATE_SENDING,
    E_JRD_USIM_USSD_STATE_SEND_COMPLETE,
    E_JRD_USIM_USSD_STATE_SEND_ERROR,
    E_JRD_USIM_USSD_STATE_MAX,
} e_jrd_usim_ussd_state_t;

typedef enum {
    E_JRD_USIM_USSD_TYPE_INVALID = -1,
    E_JRD_USIM_USSD_TYPE_NONE,
    E_JRD_USIM_USSD_TYPE_DONE,    /* No further action required. */
    E_JRD_USIM_USSD_TYPE_MORE,    /* Further user action required. */
    E_JRD_USIM_USSD_TYPE_ABORT,    /* USSD terminated by network. */
    E_JRD_USIM_USSD_TYPE_OTHER,    /* Other local client responded. */
    E_JRD_USIM_USSD_TYPE_NOSUP,    /* Operation not supported. */
    E_JRD_USIM_USSD_TYPE_TIMEOUT,    /* Network time out. */
    E_JRD_USIM_USSD_TYPE_MAX,
} e_jrd_usim_ussd_type_t;

/**
 * struct jrd_usim_ussd_resp_struct - The response data of ussd.
 *
 * @ussd_state:
 * @ussd_type:
 * @ussd_content:
 * @ussd_content_len:
 */
typedef struct jrd_usim_ussd_resp_struct {
    e_jrd_usim_ussd_state_t ussd_state;
    e_jrd_usim_ussd_type_t ussd_type;
    char ussd_content[JRD_USSD_CONTENT_MAX_LEN];
    uint32 ussd_content_len;
} jrd_usim_ussd_resp_t;

/**
 * struct jrd_usim_ussd_resp_struct - The response data of ussd.
 *
 * @ussd_state:
 * @ussd_type:
 * @ussd_content:
 * @ussd_content_len:
 */
typedef struct jrd_usim_ussd_data_struct {
    e_jrd_usim_ussd_action_type_t action_type;
    jrd_usim_ussd_resp_t ussd_resp;
} jrd_usim_ussd_data_t;


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* _JRD_USSD_AL_H_ */
