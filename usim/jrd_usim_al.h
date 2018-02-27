/*
****************************************************************************
* FILE        :
*                                 jrd_usim_al.c
* DESCRIPTION :
*           JRD USIM ABSTRACTION LAYER -- usim module's abstraction layer.
****************************************************************************
*    Copyright (c) 2015 by JRDCOM Incorporated.  All Rights Reserved.
****************************************************************************
*/

#ifndef _JRD_USIM_AL_H_
#define _JRD_USIM_AL_H_

#include "utils/list.h"
#include "user_identity_module_v01.h"
#include "device_management_service_v01.h"


#define JRD_USIM_MCC_LEN      3
#define JRD_USIM_MNC_LEN_2    2
#define JRD_USIM_MNC_LEN_3    3
#define JRD_USIM_MSIN_LEN    10

#define JRD_USIM_NETWORK_CODE_LEN_5    5
#define JRD_USIM_NETWORK_CODE_LEN_6    6
#define JRD_USIM_NETWORK_SUBSET_CODE_LEN_7    7
#define JRD_USIM_NETWORK_SUBSET_CODE_LEN_8    8
#define JRD_USIM_SIM_BASED_CODE_LEN_15    15
#define JRD_USIM_SIM_BASED_CODE_LEN_16    16

#define JRD_USIM_DIRMAIN_MAX_PATH_ELEMENTS    5
#define JRD_USIM_SIMLOCK_NETWORK_MAX           16    /* Max number of sim-lock network, 1 ~ 85. */
#define JRD_USIM_SIMLOCK_NETWORK_SUBSET_MAX    16    /* Max number of sim-lock network subset, 1 ~ 64. */
#define JRD_USIM_SIMLOCK_SP_MAX                16    /* Max number of sim-lock service provider, 1 ~ 64. */
#define JRD_USIM_SIMLOCK_CP_MAX                16    /* Max number of sim-lock corporate provider, 1 ~ 51. */
#define JRD_USIM_SIMLOCK_SIM_BASED_MAX         16    /* Max number of sim-lock sim based, 1 ~ 32. */

#define JRD_USIM_SIMLOCK_CODE_MAX (JRD_USIM_SIMLOCK_SIM_BASED_MAX + 1)

typedef enum {
    E_JRD_USIM_EVENT_INVALID = -1,
    E_JRD_USIM_EVENT_UNKNOWN,
    E_JRD_USIM_EVENT_USIM_STATUS_CHANGE,
    E_JRD_USIM_EVENT_MAX,
} e_jrd_usim_event_t;

typedef enum {
    E_JRD_USIM_MMGSDI_INVALID_EF_START = -1,
    E_JRD_USIM_MMGSDI_EF_UNDER_MF_START,    /* ICCID */
    E_JRD_USIM_MMGSDI_GSM_EF_START,    /* SIM card */
    E_JRD_USIM_MMGSDI_USIM_EF_START,    /* USIM card */
    E_JRD_USIM_MMGSDI_MAX,
} e_jrd_usim_mmgsid_ef_t;

typedef enum {
    E_JRD_USIM_CARD_DEFAULT = -1,
    E_JRD_USIM_CARD_0,
    E_JRD_USIM_CARD_1,
} e_jrd_usim_card_slot_t;

typedef enum {
    E_JRD_USIM_CARD_TYPE_INVALID = -1,
    E_JRD_USIM_CARD_TYPE_UNKNOWN,
    E_JRD_USIM_CARD_TYPE_SIM,
    E_JRD_USIM_CARD_TYPE_USIM,
    E_JRD_USIM_CARD_TYPE_RUIM,
    E_JRD_USIM_CARD_TYPE_CSIM,
    E_JRD_USIM_CARD_TYPE_ISIM,
    E_JRD_USIM_CARD_TYPE_MAX,
} e_jrd_usim_card_type_t;

typedef enum {
    E_JRD_USIM_CARD_STATE_INVALID = -1,
    E_JRD_USIM_CARD_STATE_ABSENT,
    E_JRD_USIM_CARD_STATE_DETECTED,
    E_JRD_USIM_CARD_STATE_PINLOCK,
    E_JRD_USIM_CARD_STATE_PUKLOCK,
    E_JRD_USIM_CARD_STATE_SIMLOCK,
    E_JRD_USIM_CARD_STATE_PINLOCK_BLOCK,    /* 5 */
    E_JRD_USIM_CARD_STATE_ILLEGAL,
    E_JRD_USIM_CARD_STATE_READY,
    E_JRD_USIM_CARD_STATE_INITING = 11,    /* Only this value to 11 */
    E_JRD_USIM_CARD_STATE_MAX,
} e_jrd_usim_card_state_t;

typedef enum {
    E_JRD_USIM_SIMINFO_TYPE_INVALID = -1,
    E_JRD_USIM_SIMINFO_TYPE_UNKNOWN,
    E_JRD_USIM_SIMINFO_TYPE_ICCID,
    E_JRD_USIM_SIMINFO_TYPE_IMSI,
    E_JRD_USIM_SIMINFO_TYPE_MSISDN,
    E_JRD_USIM_SIMINFO_TYPE_SPN,
    E_JRD_USIM_SIMINFO_TYPE_LI,
    E_JRD_USIM_SIMINFO_TYPE_AD,
    E_JRD_USIM_SIMINFO_TYPE_UST,
    /*add new match profile by wei.huang.sz*/
    E_JRD_USIM_SIMINFO_TYPE_GID1,/*ADD GID1 wei.huang.sz*/
    E_JRD_USIM_SIMINFO_TYPE_GID2,/*ADD GID2 wei.huang.sz*/
    /*add new match profile by wei.huang.sz*/
	E_JRD_USIM_SIMINFO_TYPE_PNN,
    E_JRD_USIM_SIMINFO_TYPE_ALL,
    E_JRD_USIM_SIMINFO_TYPE_MAX,
} e_jrd_usim_siminfo_type_t;


typedef enum {
    E_JRD_USIM_CK_TYPE_INVALID = -1,
    E_JRD_USIM_CK_TYPE_UNKNOWN,
    E_JRD_USIM_CK_TYPE_RCK,    /* 1 */
    E_JRD_USIM_CK_TYPE_NCK,    /* 2 */
    E_JRD_USIM_CK_TYPE_MAX,
} e_jrd_usim_ck_type_t;

typedef enum {
    E_JRD_USIM_SIMLOCK_STATE_INVALID = -1,
    E_JRD_USIM_SIMLOCK_STATE_UNKNOWN,
    E_JRD_USIM_SIMLOCK_STATE_CODE_LOCK,    /* Personalization code is required. */
    E_JRD_USIM_SIMLOCK_STATE_PUK_LOCK,    /* PUK for personalization code is required. */
    E_JRD_USIM_SIMLOCK_STATE_UNLOCKED,
    E_JRD_USIM_SIMLOCK_STATE_BLOCKED,    /* Permanently blocked. */
    E_JRD_USIM_SIMLOCK_STATE_MAX,
} e_jrd_usim_simlock_state_t;

typedef enum {
    E_JRD_USIM_SIMLOCK_TYPE_INVALID = -1,
    E_JRD_USIM_SIMLOCK_TYPE_NETWORK,
    E_JRD_USIM_SIMLOCK_TYPE_NETWORK_SUBSET,
    E_JRD_USIM_SIMLOCK_TYPE_SERVICE_PROVIDER,
    E_JRD_USIM_SIMLOCK_TYPE_CORPORATE_PROVIDER,
    E_JRD_USIM_SIMLOCK_TYPE_SIM_BASED_LOCK,
    E_JRD_USIM_SIMLOCK_TYPE_JRD_RCK,
    E_JRD_USIM_SIMLOCK_TYPE_JRD_RCK_FORBID,
    E_JRD_USIM_SIMLOCK_TYPE_MAX,
} e_jrd_usim_simlock_type_t;


typedef enum {
    E_JRD_USIM_PINLOCK_STATE_INVALID = -1,
    E_JRD_USIM_PINLOCK_STATE_UNKNOWN,
    E_JRD_USIM_PINLOCK_STATE_ENABLED_NOT_VERIFIED,
    E_JRD_USIM_PINLOCK_STATE_ENABLED_VERIFIED,
    E_JRD_USIM_PINLOCK_STATE_DISABLED,
    E_JRD_USIM_PINLOCK_STATE_BLOCKED,    /* Need PUK Code to Verified. */
    E_JRD_USIM_PINLOCK_STATE_PERMANENTLY_BLOCKED,
    E_JRD_USIM_PINLOCK_STATE_MAX,
} e_jrd_usim_pinlock_state_t;

typedef enum {
    E_JRD_USIM_DIAG_STATUS_NO_SIM_CARD = 0,
    E_JRD_USIM_DIAG_STATUS_INVALID_SIM_CARD,
    E_JRD_USIM_DIAG_STATUS_PIN1,
    E_JRD_USIM_DIAG_STATUS_PUK1,
    E_JRD_USIM_DIAG_STATUS_READY,
    E_JRD_USIM_DIAG_STATUS_SIMLOCK_NETWORK_PIN,
    E_JRD_USIM_DIAG_STATUS_SIMLOCK_NETWORK_SUBSET_PIN,
    E_JRD_USIM_DIAG_STATUS_SIMLOCK_SERVICE_PROVIDER_PIN,
    E_JRD_USIM_DIAG_STATUS_SIMLOCK_CORPORATE_PROVIDER_PIN,
    E_JRD_USIM_DIAG_STATUS_SIMLOCK_SIM_BASED_LOCK_PIN,
    E_JRD_USIM_DIAG_STATUS_SIMLOCK_NETWORK_PUK,
    E_JRD_USIM_DIAG_STATUS_SIMLOCK_NETWORK_SUBSET_PUK,
    E_JRD_USIM_DIAG_STATUS_SIMLOCK_SERVICE_PROVIDER_PUK,
    E_JRD_USIM_DIAG_STATUS_SIMLOCK_CORPORATE_PROVIDER_PUK,
    E_JRD_USIM_DIAG_STATUS_SIMLOCK_SIM_BASED_LOCK_PUK,
    E_JRD_USIM_DIAG_STATUS_SIMLOCK_RCK_FORBID,
    E_JRD_USIM_DIAG_STATUS_PIN_PERM_BLOCKED,
    E_JRD_USIM_DIAG_STATUS_SIM_BUSY,
    E_JRD_USIM_DIAG_STATUS_MAX,
} e_jrd_usim_diag_status_t;


typedef struct jrd_usim_mmgsdi_ef_path_struct {
    uint32 len;
    uint16 path[JRD_USIM_DIRMAIN_MAX_PATH_ELEMENTS];
} jrd_usim_mmgsdi_ef_path_t;

typedef int (*jrd_usim_event_cb)(e_jrd_usim_event_t event, void *msg_data, int msg_data_len, void *cb_param);
typedef struct jrd_usim_msg_element_struct {
    jrd_usim_event_cb callback;
    void *cb_param;
} jrd_usim_event_element_t;

typedef struct jrd_usim_event_list_node_struct {
    list_link_type *link;
    jrd_usim_event_element_t event_element;
} jrd_usim_event_list_node_t;

/**
 * struct jrd_usim_cardinfo_struct - The attribute of the card status infomartion.
 *
 * @card_slot: Uesed card slot.
 * @card_type: The type of sim card, about SIM/USIM/RUIM/CSIM/ISIM.
 * @card_state: Indicates the state of the card.
 */
typedef struct jrd_usim_card_info_struct {
    e_jrd_usim_card_slot_t card_slot;
    e_jrd_usim_card_type_t card_type;
    e_jrd_usim_card_state_t card_state;
} jrd_usim_card_info_t;

/**
 * struct jrd_usim_pinlock_struct - The attribute of the sim-info.
 *
 * @plmn:        The PLMN of the card.
 * @imsi:        The IMSI of the card.
 * @iccid:       The ICCID of the card.
 * @msisdn:      The MSISDN of the card.
 * @spn:         The SPN of the card.
 * @ust:         The UST of the card.
 * @language:    The LANGUAGE of the card.
 */
typedef struct jrd_usim_siminfo_struct {
    char plmn[16];
    char imsi[32];
    char iccid[32];
    char msisdn[128];
    char spn[64];
    char ad[64];
    uint8 ust[16];    /* [0]:ust lenght. */
    char language[16];
    /*add new match profile by wei.huang.sz*/
    char gid1[8];
    char gid2[8];
    /*add new match profile by wei.huang.sz*/
	char pnn[64];
} jrd_usim_siminfo_t;

/**
 * struct jrd_usim_simlock_struct - The attribute of the sim-lock.
 *
 * @simlock_state:
 * @simlock_type:
 * @simunlock_remain_times:
 */
typedef struct jrd_usim_simlock_struct {
    e_jrd_usim_simlock_state_t simlock_state;
    e_jrd_usim_simlock_type_t simlock_type;
    int simunlock_remain_times;
} jrd_usim_simlock_t;

/**
 * struct jrd_usim_pinlock_struct - The attribute of the pin-lock.
 *
 * @pinlock_state:
 * @pinunlock_remain_times:
 * @pukunlock_remain_times:
 * @pin_code:
 */
typedef struct jrd_usim_pinlock_struct {
    e_jrd_usim_pinlock_state_t pinlock_state;
    int pinunlock_remain_times;
    int pukunlock_remain_times;
    //char pin_code[16];    /* Save the PIN Code. */
} jrd_usim_pinlock_t;

typedef struct jrd_usim_simlock_config {
  int simlock_type;
  uint8 rck_num_retries_max;
  uint8 nck_num_retries_max;
  uint8 ck_length;
  uint8 rck_switch_enable;
  uint8 rck_status;
  char simlock_code[32][JRD_USIM_SIMLOCK_CODE_MAX];
  int simlock_code_num;
  uint8 auto_lock;
} jrd_usim_simlock_config_data_t;

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
  FUNCTION  jrd_usim_al_get_card_info
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_al_get_card_info(
  jrd_usim_card_info_t *card_info,
  jrd_usim_pinlock_t *pinlock_info,
  jrd_usim_simlock_t *simlock_info,
  e_jrd_usim_diag_status_t *usim_diag_status
);


/*===========================================================================
  FUNCTION  jrd_usim_al_get_card_status
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_al_get_card_status(jrd_usim_card_info_t *card_info);


/*===========================================================================
  FUNCTION  jrd_usim_al_get_plmn_from_imsi
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_al_get_plmn_from_imsi(
  jrd_usim_siminfo_t *siminfo_data
);

/*===========================================================================
  FUNCTION  jrd_usim_al_get_card_siminfo
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_al_get_card_siminfo(
  jrd_usim_siminfo_t *siminfo_data,
  e_jrd_usim_siminfo_type_t siminfo_type,
  e_jrd_usim_card_type_t card_type
);


/*===========================================================================
  FUNCTION  jrd_usim_al_get_simlock_info
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_al_get_simlock_info(jrd_usim_simlock_t *simlock_info);


/*===========================================================================
  FUNCTION  jrd_usim_al_get_pinlock_info
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_al_get_pinlock_info(jrd_usim_pinlock_t *pinlock_info);


/*===========================================================================
  FUNCTION  jrd_usim_al_set_pin_protection
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_al_set_pin_protection(int operation, char *pin_value, jrd_usim_pinlock_t *pinlock_info);


/*===========================================================================
  FUNCTION  jrd_usim_al_verify_pin
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_al_verify_pin(char *pin_value);


/*===========================================================================
  FUNCTION  jrd_usim_al_unblock_pin
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_al_unblock_pin(char *pin_value, char *puk_value, jrd_usim_pinlock_t *pinlock_info);


/*===========================================================================
  FUNCTION  jrd_usim_al_simlock
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_al_active_simlock(
  jrd_usim_simlock_config_data_t *simlock_config
);


/*===========================================================================
  FUNCTION  jrd_usim_al_deactive_simlock
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_al_deactive_simlock(
  jrd_usim_simlock_config_data_t *simlock_config,
  e_jrd_usim_card_state_t card_state,
  char *ck_value,
  int ck_value_len,
  int *retries_left_valid
);


/*===========================================================================
  FUNCTION  jrd_usim_al_event_register
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_al_event_register(void *callback, void *cb_param);


/*===========================================================================
  FUNCTION  jrd_usim_al_event_unregister
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_al_event_unregister(void *callback);


/*===========================================================================
  FUNCTION  jrd_usim_al_init
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_al_init(void);


/*===========================================================================
  FUNCTION  jrd_usim_al_exit
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
extern int jrd_usim_al_exit(void);


#ifdef __cplusplus
}
#endif

#endif /* _JRD_USIM_AL_H_ */
