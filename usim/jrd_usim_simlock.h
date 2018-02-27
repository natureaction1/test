/*
****************************************************************************
* FILE        :
*                                 jrd_usim_simlock.h
* DESCRIPTION :
*               JRD USIM SIMLOCK MODULE -- usim module feature of simlock.
****************************************************************************
*    Copyright (c) 2015 by JRDCOM Incorporated.  All Rights Reserved.
****************************************************************************
*/

#ifndef _JRD_USIM_SIMLOCK_H_
#define _JRD_USIM_SIMLOCK_H_


typedef enum {
    E_JRD_USIM_SIMLOCK_MODE_INVALID = -1,
    E_JRD_USIM_SIMLOCK_MODE_DISABLE,
    E_JRD_USIM_SIMLOCK_MODE_NETWORK,
    E_JRD_USIM_SIMLOCK_MODE_NETWORK_SUBSET,
    E_JRD_USIM_SIMLOCK_MODE_SERVICE_PROVIDER,
    E_JRD_USIM_SIMLOCK_MODE_CORPORATE_PROVIDER,
    E_JRD_USIM_SIMLOCK_MODE_SIM_BASED_LOCK,
    E_JRD_USIM_SIMLOCK_MODE_MAX,
} e_jrd_usim_simlock_mode_t;

typedef enum {
    E_JRD_USIM_SIMUNLOCK_MODE_INVALID = -1,
    E_JRD_USIM_SIMUNLOCK_MODE_ANY_CARD_INSET,
    E_JRD_USIM_SIMUNLOCK_MODE_SPECIFIED_CARD,
    E_JRD_USIM_SIMUNLOCK_MODE_IGNORE_INSERT_OR_NOT,
    E_JRD_USIM_SIMUNLOCK_MODE_MAX,
} e_jrd_usim_simunlock_mode_t;
typedef enum {
    E_JRD_USIM_SIMUNLOCK_FLAG_INVALID = -1,
    E_JRD_USIM_SIMUNLOCK_FLAG_ACTIVE = 0,
    E_JRD_USIM_SIMUNLOCK_FLAG_DEACTIVE = 1,
    E_JRD_USIM_SIMUNLOCK_FLAG_SUCESS = 2,
    E_JRD_USIM_SIMUNLOCK_FLAG_SUCESS_REMOVE = 3,
    E_JRD_USIM_SIMUNLOCK_FLAG_MAX,
} e_jrd_usim_simlock_flag;


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* _JRD_USIM_SIMLOCK_H_ */
