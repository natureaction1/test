/*
****************************************************************************
* FILE        :
*                                 jrd_usim_simlock.c
* DESCRIPTION :
*               JRD USIM SIMLOCK MODULE -- usim module feature of simlock.
****************************************************************************
*    Copyright (c) 2015 by JRDCOM Incorporated.  All Rights Reserved.
****************************************************************************
*/
/*
**********************************EditHistory*******************************
* This section contains comments describing changes made to the module.
* Notice that changes are listed in reverse chronological order.
*
*    when               who                      what, where, why
* ------------    ---------------    ---------------------------------------
*  2015/11/18      Randy.Xi           Create the initial version.
*
****************************************************************************
*/

#include "jrd_oem.h"
#include "jrd_oem_common.h"

#include "jrd_usim_al.h"
#include "jrd_usim_simlock.h"
#include <ctype.h>


/*===========================================================================
  FUNCTION  jrd_usim_sscanf
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
static int jrd_usim_sscanf(char *src_str, char dest_str[][7], int *str_num)
{
    int index = 0;
    int offset = 0;
    int rc = JRD_NO_ERR;

    while (1) {
        rc = JRD_SSCANF(src_str, "%[^',']%n", dest_str[index], &offset);
        if (rc == 0) {
            *str_num = ++index;
            src_str+=offset;
        } else if (rc == 1) {
            src_str++;
        } else {
            break;
        }
    }
}

static int jrd_usim_split_simlock_code(char *src_str, char dest_str[][JRD_USIM_SIMLOCK_CODE_MAX], int *str_num)
{
  int index = 0;
  char *ph = src_str;
  char *p  = src_str;

  if(!src_str)
    return JRD_FAIL;

  while(*p && isspace(*p) == 0)
  {
    if(index >= 32)
      return JRD_FAIL;

    if(*(++p) == ',' || *p == '\0' || isspace(*p))
    {
      if ((p - ph) >= JRD_USIM_SIMLOCK_CODE_MAX)
        return JRD_FAIL;

      JRD_MEMCPY(dest_str[index], ph,(p - ph));
      index++;
      ph = p+1;
      (*str_num)++;

      JRD_OEM_LOG_INFO(JRD_OEM_LOG_LOW,"jrd_usim_split_simlock_code dest_str[%d]=%s.\n",index-1,dest_str[index-1]);
    }
  }
  return JRD_NO_ERR;
}

/*===========================================================================
  FUNCTION  jrd_usim_simlock_flag_handle
===========================================================================*/
/*!
@brief


@return
  error

*/
/*=========================================================================*/
int jrd_usim_simlock_flag_handle(e_jrd_usim_simlock_flag simlock_flag)
{
	e_jrd_usim_simlock_flag simlock_flag_tmp = E_JRD_USIM_SIMUNLOCK_FLAG_INVALID;
	int rc = JRD_NO_ERR;
	int rc_remove = -1;
	FILE *fp = NULL;
	simlock_flag_tmp = simlock_flag;
	
	if (simlock_flag_tmp == E_JRD_USIM_SIMUNLOCK_FLAG_ACTIVE)
	{
    	fp = JRD_FOPEN("/cache/simlock_active_flag", "w");
    	if (NULL == fp)
    	{
        	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"file open Fail!\n");
        	rc = JRD_FAIL;
    	}
    	else
    	{
			JRD_FCLOSE(fp);
    	}
	}
	else if (simlock_flag_tmp == E_JRD_USIM_SIMUNLOCK_FLAG_DEACTIVE)
	{
    	rc_remove = JRD_REMOVE("/cache/simlock_active_flag");
    	if (-1 == rc_remove)
    	{
        	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"file remove Fail!\n");
        	rc = JRD_FAIL;
    	}
	}
	else if (simlock_flag_tmp == E_JRD_USIM_SIMUNLOCK_FLAG_SUCESS)
	{
    	fp = JRD_FOPEN("/jrd-resource/resource/simlock_active_sucess_flag", "w");
    	if (NULL == fp)
    	{
        	JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"file open Fail!\n");
        	rc = JRD_FAIL;
    	}
    	else
    	{
			JRD_FCLOSE(fp);
    	}
	}
	else if (simlock_flag_tmp == E_JRD_USIM_SIMUNLOCK_FLAG_SUCESS_REMOVE)
	{
        rc_remove = JRD_REMOVE("/jrd-resource/resource/simlock_active_sucess_flag");
        if (-1 == rc_remove)
        {
           JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"file remove Fail!\n");
           rc = JRD_FAIL;
        }

	}
	else
	{
		JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"not a valid pararam,do not handle!\n");
	}
	
	return rc;

}

/*===========================================================================
  FUNCTION  jrd_usim_simlock_active
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
int jrd_usim_simlock_active(e_jrd_usim_simlock_mode_t simlock_mode,jrd_usim_simlock_config_data_t *simlock_config)
{
    char src_data[512] = {0};
    int rc = JRD_NO_ERR;

    rc = jrd_usim_simlock_flag_handle(E_JRD_USIM_SIMUNLOCK_FLAG_ACTIVE);
  	if (JRD_FAIL == rc)
  	{
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, touch simlock flag fail\n");
  	}

    switch (simlock_mode)
    {
        case E_JRD_USIM_SIMLOCK_MODE_NETWORK:
        {
        		rc = jrd_db_get_value("sim_config", "NetworkCode", src_data, E_PARAM_STR, sizeof(src_data));
        		if (rc != JRD_NO_ERR) {
      		      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, read NetworkCode from sim_config table.\n");
    	    	    return JRD_FAIL;
        	  }

     		    jrd_usim_split_simlock_code(src_data, simlock_config->simlock_code, &simlock_config->simlock_code_num);
            simlock_config->simlock_type = E_JRD_USIM_SIMLOCK_TYPE_NETWORK;
            break;
        }
        /*
        case E_JRD_USIM_SIMLOCK_MODE_NETWORK_SUBSET:
        {
      		  rc = jrd_db_get_value("sim_config", "NetworkSubsetCode", src_data, E_PARAM_STR, sizeof(simlock_config->simlock_code));
           	if (rc != JRD_NO_ERR) {
		    	      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, read NetworkSubsetCode from sim_config table.\n");
			          return JRD_FAIL;
   			    }
    		    jrd_usim_sscanf(src_data, simlock_config->simlock_code, &simlock_config->simlock_code_num);

            simlock_config->simlock_type = E_JRD_USIM_SIMLOCK_TYPE_NETWORK_SUBSET;
            break;
        }
        case E_JRD_USIM_SIMLOCK_MODE_SERVICE_PROVIDER:
        {
    	      rc = jrd_db_get_value("sim_config", "SPCode", src_data, E_PARAM_STR, sizeof(simlock_config->simlock_code));
      		  if (rc != JRD_NO_ERR) {
       			    JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, read SPCode from sim_config table.\n");
	    		      return JRD_FAIL;
		        }
        	  jrd_usim_sscanf(src_data, simlock_config->simlock_code, &simlock_config->simlock_code_num);

            simlock_config->simlock_type = E_JRD_USIM_SIMLOCK_TYPE_SERVICE_PROVIDER;
            break;
        }
        case E_JRD_USIM_SIMLOCK_MODE_CORPORATE_PROVIDER:
        {
        	  rc = jrd_db_get_value("sim_config", "CorporateCode", src_data, E_PARAM_STR, sizeof(simlock_config->simlock_code));
            if (rc != JRD_NO_ERR) {
		            JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, read CorporateCode from sim_config table.\n");
  				      return JRD_FAIL;
    		    }
	    	    jrd_usim_sscanf(src_data, simlock_config->simlock_code, &simlock_config->simlock_code_num);
            simlock_config->simlock_type = E_JRD_USIM_SIMLOCK_TYPE_CORPORATE_PROVIDER;
            break;
        }
        */
        case E_JRD_USIM_SIMLOCK_MODE_SIM_BASED_LOCK:
        {
          	rc = jrd_db_get_value("sim_config", "SimBasedCode", src_data, E_PARAM_STR, sizeof(src_data));
      	    if (rc != JRD_NO_ERR) {
  			      JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, read SimBasedCode from sim_config table.\n");
      			  return JRD_FAIL;
	   		    }
	    	    jrd_usim_split_simlock_code(src_data, simlock_config->simlock_code, &simlock_config->simlock_code_num);

            simlock_config->simlock_type = E_JRD_USIM_SIMLOCK_TYPE_SIM_BASED_LOCK;
            break;
        }
        default:
        {
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Warning, unknown sim-lock mode(mode = %d).\n", simlock_mode);
            return JRD_NO_ERR;
        }
    }

	  rc = jrd_usim_al_active_simlock(simlock_config);
    if (rc != JRD_NO_ERR) {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, active sim-lock..\n");
        return JRD_FAIL;
    }

    rc = jrd_usim_simlock_flag_handle(E_JRD_USIM_SIMUNLOCK_FLAG_SUCESS);
  	if (JRD_FAIL == rc)
  	{
          JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, touch simlock flag fail\n");
  	}

    return JRD_NO_ERR;
}

/*===========================================================================
  FUNCTION  jrd_usim_simlock_deactive
===========================================================================*/
/*!
@brief


@return
  None

*/
/*=========================================================================*/
int jrd_usim_simlock_deactive(
  e_jrd_usim_simlock_mode_t simlock_mode,
  jrd_usim_simlock_config_data_t *simlock_config,
  e_jrd_usim_card_state_t card_state, 
  char *ck_value,
  int *retries_left_valid
)
{
    simlock_config->simlock_type = E_JRD_USIM_SIMLOCK_TYPE_INVALID;
  	int rc = JRD_NO_ERR;

    switch (simlock_mode)
    {
        case E_JRD_USIM_SIMLOCK_MODE_NETWORK:
        {
            simlock_config->simlock_type = E_JRD_USIM_SIMLOCK_TYPE_NETWORK;
            break;
        }
        case E_JRD_USIM_SIMLOCK_MODE_NETWORK_SUBSET:
        {
            simlock_config->simlock_type = E_JRD_USIM_SIMLOCK_TYPE_NETWORK_SUBSET;
            break;
        }
        case E_JRD_USIM_SIMLOCK_MODE_SERVICE_PROVIDER:
        {
            simlock_config->simlock_type = E_JRD_USIM_SIMLOCK_TYPE_SERVICE_PROVIDER;
            break;
        }
        case E_JRD_USIM_SIMLOCK_MODE_CORPORATE_PROVIDER:
        {
            simlock_config->simlock_type = E_JRD_USIM_SIMLOCK_TYPE_CORPORATE_PROVIDER;
            break;
        }
        case E_JRD_USIM_SIMLOCK_MODE_SIM_BASED_LOCK:
        {
            simlock_config->simlock_type = E_JRD_USIM_SIMLOCK_TYPE_SIM_BASED_LOCK;
            break;
        }
        default:
        {
            JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, unknown sim-lock mode(mode = %d).\n", simlock_mode);
            return JRD_FAIL;
        }
    }

  	rc = jrd_usim_simlock_flag_handle(E_JRD_USIM_SIMUNLOCK_FLAG_DEACTIVE);
  	if (JRD_FAIL == rc) {
          JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, remove simlock flag fail\n");
  	}

	  rc = jrd_usim_al_deactive_simlock(simlock_config, card_state, ck_value, JRD_STRLEN(ck_value),retries_left_valid);
    if (rc != JRD_NO_ERR) {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, deactive sim-lock..\n");
        return JRD_FAIL;
    }
    
    rc = jrd_usim_simlock_flag_handle(E_JRD_USIM_SIMUNLOCK_FLAG_SUCESS_REMOVE);
    if (JRD_FAIL == rc)
    {
        JRD_OEM_LOG_INFO(JRD_OEM_LOG_ERROR,"Error, remove simlock flag fail\n");
    }

    return JRD_NO_ERR;
}
