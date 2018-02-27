/*
****************************************************************************
* FILE        :
*                                 jrd_usim.h
* DESCRIPTION :
*                     JRD USIM MODULE -- usim card module.
****************************************************************************
*    Copyright (c) 2015 by JRDCOM Incorporated.  All Rights Reserved.
****************************************************************************
*/

#ifndef _JRD_USIM_H_
#define _JRD_USIM_H_

#include "jrd_ussd_al.h"
#include "jrd_usim_al.h"
#include "jrd_usim_simlock.h"


#define JRD_OEM_USIM_STACK_SIZE    128*1024
#define JRD_USIM_SIMLOCK_FLAG_PATH    "/jrd-resource/resource/simlock_active_flag"

/* 20401x ~ 20419x */
#define JRD_USIM_ERR_CODE_CARD_SLOT                       204011
#define JRD_USIM_ERR_CODE_CARD_STATE                      204021
#define JRD_USIM_ERR_CODE_SIM_LOCK_STATE                  204031
#define JRD_USIM_ERR_CODE_SIM_UNLOCK_REMAIN_TIMES         204041
#define JRD_USIM_ERR_CODE_AUTO_PIN_STATE                  204051
#define JRD_USIM_ERR_CODE_PIN_STATE                       204061
#define JRD_USIM_ERR_CODE_PIN_UNLOCK_REMAIN_TIMES         204071
#define JRD_USIM_ERR_CODE_PUK_UNLOCK_REMAIN_TIMES         204081
/* 20420x ~ 20439x */
#define JRD_USIM_ERR_CODE_SIMINFO_PLMN                    204201
#define JRD_USIM_ERR_CODE_SIMINFO_IMSI                    204211
#define JRD_USIM_ERR_CODE_SIMINFO_ICCID                   204221
#define JRD_USIM_ERR_CODE_SIMINFO_MSISDN                  204231
#define JRD_USIM_ERR_CODE_SIMINFO_SPN                     204241
#define JRD_USIM_ERR_CODE_SIMINFO_UST                     204251
#define JRD_USIM_ERR_CODE_SIMINFO_LANGUAGE                204261
/* 20440x ~ 20459x */
#define JRD_USIM_ERR_CODE_OPS_PIN_OPERATION               204401
#define JRD_USIM_ERR_CODE_NOT_SUPPORT_ACTIVE_PIN          204402
#define JRD_USIM_ERR_CODE_OPS_VERIFY_PIN                  204411
#define JRD_USIM_ERR_CODE_OPS_CHANGE_PIN                  204421
#define JRD_USIM_ERR_CODE_OPS_UNBLOCK_PIN                 204431
#define JRD_USIM_ERR_CODE_OPS_AUTO_SIM_LOCK               204441
#define JRD_USIM_ERR_CODE_OPS_ACTIVE_SIM_LOCK             204451
#define JRD_USIM_ERR_CODE_OPS_DEACTIVE_SIM_LOCK           204461
#define JRD_USIM_ERR_CODE_OPS_CANNEL_USSD                 204471
#define JRD_USIM_ERR_CODE_OPS_ACTIVE_USSD_ANSWER          204481
#define JRD_USIM_ERR_CODE_OPS_ACTIVE_USSD_ORIG_NO_WAIT    204482
/* 20460x ~ 20479x */
#define JRD_USIM_ERR_CODE_AUTO_UNLOCK_PIN                 204601
#define JRD_USIM_ERR_CODE_NOT_SUPPORT_AUTO_UNLOCK_PIN     204602
#define JRD_USIM_ERR_CODE_AUTO_UNLOCK_PIN_ERR_PIN_CODE    204603
#define JRD_USIM_ERR_CODE_AUTO_UNLOCK_PIN_INVALID_CARD    204604
#define JRD_USIM_ERR_CODE_AUTO_UNLOCK_PIN_REMAIN_TIMES    204605
#define JRD_USIM_ERR_CODE_SET_MSISDN                      132501

typedef enum
{
    E_JRD_USIM_MSISDN_INVALID = -1,
    E_JRD_USIM_MSISDN_USIM,
    E_JRD_USIM_MSISDN_UI,
    E_JRD_USIM_MSISDN_MAX,
} e_jrd_usim_msisdn_t;


typedef enum
{
    E_JRD_USIM_CONFIG_ID_INVALID = -1,
    E_JRD_USIM_CONFIG_ID_SIM_HOT_SWAP,
    E_JRD_USIM_CONFIG_ID_AUTO_SIMLOCK,
    E_JRD_USIM_CONFIG_ID_SIMLOCK_MODE,
    E_JRD_USIM_CONFIG_ID_SIMUNLOCK_MODE,
    E_JRD_USIM_CONFIG_ID_NCK_UNLOCK_TIMES,
    E_JRD_USIM_CONFIG_ID_RCK_UNLOCK_TIMES,
    E_JRD_USIM_CONFIG_ID_AUTO_PINUNLOCK,
    E_JRD_USIM_CONFIG_ID_DEAFAULT_PIN_CODE,
    E_JRD_USIM_CONFIG_MSISDN_UI,
    E_JRD_USIM_CONFIG_MSISDN_ORIGIN,
    E_JRD_USIM_CONFIG_MSISDN_IMSI,
    E_JRD_USIM_CONFIG_ID_SIMLOCK_CK_LENGTH,
    E_JRD_USIM_CONFIG_ID_SIMLOCK_RCK_SWITCH_ENABLE,
    E_JRD_USIM_CONFIG_ID_SIMLOCK_RCK_STATUS,
    E_JRD_USIM_CONFIG_ID_MAX,
} e_jrd_usim_config_id_t;

typedef enum {
    E_JRD_USIM_IND_INVALID = -1,
    E_JRD_USIM_IND_SELF_CARD_STATUS_CHANGE,    /* Indication self. */
    E_JRD_USIM_IND_CARD_STATUS_CHANGE,
    E_JRD_USIM_IND_ICCID_CHG_LANG_ID,
    E_JRD_USIM_IND_MAX,
} e_jrd_usim_ind_t;

typedef enum {
    E_JRD_USIM_ACT_ID_GET_CARD_INFO = 0,
    E_JRD_USIM_ACT_ID_GET_SIMINFO,
    E_JRD_USIM_ACT_ID_GET_USSD_INFO,
    E_JRD_USIM_ACT_ID_OPS_PIN_OPERATION,
    E_JRD_USIM_ACT_ID_OPS_VERIFY_PIN,
    E_JRD_USIM_ACT_ID_OPS_CHANGE_PIN,    /* 5 */
    E_JRD_USIM_ACT_ID_OPS_UNBLOCK_PIN,
    E_JRD_USIM_ACT_ID_OPS_ACTIVE_SIMLOCK,
    E_JRD_USIM_ACT_ID_OPS_DEACTIVE_SIMLOCK,
    E_JRD_USIM_ACT_ID_OPS_ACTIVE_USSD,
    E_JRD_USIM_ACT_ID_OPS_CANNEL_USSD,    /* 10 */
    E_JRD_USIM_ACT_ID_OPS_SET_AUTO_VALIDATE_PIN,
    E_JRD_USIM_ACT_ID_OPS_SET_MSISDB_UI,
    E_JRD_USIM_ACT_ID_MAX,
} e_jrd_usim_act_id_t;

typedef enum {
    E_JRD_USIM_PARAM_SIM_LOCK_CODE = 0,
    E_JRD_USIM_PARAM_CARD_SLOT,
    E_JRD_USIM_PARAM_CARD_STATE,
    E_JRD_USIM_PARAM_CARD_TYPE,
    E_JRD_USIM_PARAM_STATE,
    E_JRD_USIM_PARAM_SIM_LOCK_STATE,    /* 5 */
    E_JRD_USIM_PARAM_SIM_LOCK_TYPE,
    E_JRD_USIM_PARAM_SIM_UNLOCK_REMAIN_TIMES,
    E_JRD_USIM_PARAM_PIN_LOCK_STATE,
    E_JRD_USIM_PARAM_PIN_UNLOCK_REMAIN_TIMES,
    E_JRD_USIM_PARAM_PUK_UNLOCK_REMAIN_TIMES,    /* 10 */
    E_JRD_USIM_PARAM_PLMN,
    E_JRD_USIM_PARAM_IMSI,
    E_JRD_USIM_PARAM_ICCID,
    E_JRD_USIM_PARAM_MSISDN,
    E_JRD_USIM_PARAM_SPN,    /* 15 */
    E_JRD_USIM_PARAM_UST,
    E_JRD_USIM_PARAM_AD,
    E_JRD_USIM_PARAM_LANGUAGE,
    E_JRD_USIM_PARAM_PIN,
    E_JRD_USIM_PARAM_NEW_PIN,    /* 20 */
    E_JRD_USIM_PARAM_OLD_PIN,
    E_JRD_USIM_PARAM_PUK,
    E_JRD_USIM_PARAM_USSD_STATE,
    E_JRD_USIM_PARAM_USSD_TYPE,
    E_JRD_USIM_PARAM_USSD_CONTENT,     /* 25 */
    E_JRD_USIM_PARAM_USSD_CONTENT_LEN,
    E_JRD_USIM_PARAM_MSISDN_MARK,  
} e_jrd_usim_param_t;


/**
 * struct jrd_usim_simcard_data_struct - The data of sim card.
 *
 * @card_info:
 * @simlock:
 * @pinlock:
 * @sim_info:
 */
typedef struct jrd_usim_simcard_data_struct {
    jrd_usim_card_info_t card_info;
    jrd_usim_siminfo_t sim_info;
    jrd_usim_simlock_t simlock;
    jrd_usim_pinlock_t pinlock;
} jrd_usim_simcard_data_t;

/**
 * struct jrd_usim_simlock_config_struct - The config attribute of the sim-lock.
 *
 * @auto_simlock: Auto active sim-lock when the sim card is first inserted.
 * @simlock_mode: Active or disable sim-lock. When active sim-lock, 
 *     there has five sim-lock mode, such as:
 *     Network(MCC/MNC), Network Subset(IMSI digits 6 and 7),
 *     SP(Service Provider), Corporate(Corporate Provider), 
 *     SIM/USIM or Sim-Based lock(IMSI digits 8 to 15).
 * @simunlock_mode: The mode of deactive sim-lock, there has three 
 *     sim-unlock mode, such as:
 *     Unlock with any sim card insert,
 *     Unlock with specified sim card,
 *     Unlock ignore sim card insert or not.
 * @nck_unlock_times: Max times to unlock NCK.
 * @nck_unlock_times: Max times to unlock RCK.
 */
typedef struct jrd_usim_simlock_config_struct {
    boolean auto_simlock;
    e_jrd_usim_simlock_mode_t simlock_mode;
    e_jrd_usim_simunlock_mode_t simunlock_mode;

    unsigned int nck_unlock_times;
    unsigned int rck_unlock_times;
    uint8 ck_length;
    uint8 rck_switch_enable;
    uint8 rck_status;
} jrd_usim_simlock_config_t;

/**
 * struct jrd_usim_pinlock_config_struct - The config attribute of the pin-lock.
 *
 * @auto_pinunlock: Auto unlock pin-lock when unlock the pin-lock success first.
 * @pin_code: When auto_pinunlock had actived, must have default unlock pin code.
 */
typedef struct jrd_usim_pinlock_config_struct {
    boolean auto_pinunlock;
    //char default_pin_code[16];
} jrd_usim_pinlock_config_t;
typedef struct jrd_usim_msisdn_config_struct {
    uint8 msisdn_origin;
    uint8 msisdn_mark;
    char msisdn_ui[64];
    char imsi[32];
} jrd_usim_msisdn_config_t;

/**
 * struct jrd_usim_config_data_struct - The config data of usim module.
 *
 * @sim_hot_swap: Sim card hot swap.
 * @simlock: The simlock config data of usim form custome.
 * @pinlock: The pinlock config data of usim form custome.
 */
typedef struct jrd_usim_config_data_struct {
    boolean sim_hot_swap;

    jrd_usim_simlock_config_t simlock;
    jrd_usim_pinlock_config_t pinlock;
    jrd_usim_msisdn_config_t msisdn;
} jrd_usim_config_data_t;

typedef struct jrd_usim_ind_data_struct {
    e_jrd_usim_card_state_t card_state;
    char plmn[16];
    char imsi[32];
    char spn[64];
    char language[16];
    /*add new match profile by wei.huang.sz*/
    char gid1[8];
    char gid2[8];
    /*add new match profile by wei.huang.sz*/
	char pnn[32];
} jrd_usim_ind_data_t;

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
  FUNCTION  jrd_usim_init
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _JRD_USIM_H_ */
